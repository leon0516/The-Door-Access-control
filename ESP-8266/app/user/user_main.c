/******************************************************************************
 * Copyright 2013-2014 Espressif Systems (Wuxi)
 *
 * FileName: user_main.c
 *
 * Description: entry file of user application
 *
 * Modification history:
 *     2014/1/1, v1.0 create this file.
 *******************************************************************************/
#include "ets_sys.h"
#include "osapi.h"
#include "os_type.h"
#include "ip_addr.h"
#include "espconn.h"
#include "mem.h"
#include "../driver/uart.h"
#include "user_interface.h"
#include "smartconfig.h"
#include "../easygpio/easygpio.h"
#include "../mqtt/mqtt.h"
#include "../mqtt/debug.h"
#include "../httpclient/httpclient.h"
#include "../key/peri_key.h"
#include "gpio.h"
#include "../driver/i2c_oled.h"
#include "../driver/i2c.h"
#include "../driver/dht22.h"
#include "../pn532/pn532.h"
#include "user_json.h"
#include "user_config.h"
#define MENJINDEBUG
#define PN532DEBUG
#define ESP8266DEBUG
#define DEBUG_MSG
#define DEFAULT_LAN_PORT 	12476
#define DELAY 60000 /* milliseconds */
#define CHECK_DOOR_DELAY 5000
#define CLEAN_OLED_DELAY 6000
#define PN532_SEND_READ_CMD_DELAY 1000
LOCAL os_timer_t get_environment_info_timer;
LOCAL os_timer_t check_door_status_timer;
LOCAL os_timer_t clean_oled_timer;
LOCAL os_timer_t pn532readID;
uint8_t pin_num[13];
DHT_Sensor sensor;
bool isFirstConMQTT = true;
uint8_t humidity[5];
uint8_t temperature[5];
float lasthumidity = 0;
float lasttemperature = 0;
void get_environment_info_cb(void *arg);
char * combine_env_strings(char *temp, char * humi);
void task_send_environment_info_init();
uint8 num[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B',
		'C', 'D', 'E', 'F' };
bool isNeedReadID = false;
bool isNeedSendCMD = true;
bool isNeedSendENV = true;
uint8 uidstr[9];
uint8_t lan_buf[200];
uint16_t lan_buf_len;
uint8 udp_sent_cnt = 0;
void peri_key_long_press(void);
void pn532_send_read_cmd_cb();
void peri_key_short_press(void);
void mqttstart();
LOCAL int ICACHE_FLASH_ATTR
json_get(struct jsontree_context *js_ctx) {
	const char *path = jsontree_path_name(js_ctx, js_ctx->depth - 1);
	if (os_strncmp(path, "humidity", 8) == 0) {
		jsontree_write_string(js_ctx, humidity);
	}
	return 0;
}
LOCAL int ICACHE_FLASH_ATTR
temp_json_get(struct jsontree_context *js_ctx) {
	const char *path = jsontree_path_name(js_ctx, js_ctx->depth - 1);
	if (os_strncmp(path, "temperature", 11) == 0) {
		jsontree_write_string(js_ctx, temperature);
	}
	return 0;
}

LOCAL int ICACHE_FLASH_ATTR
open_door_cmd_json_set(struct jsontree_context *js_ctx,
		struct jsonparse_state *parser) {
	int type;
	while ((type = jsonparse_next(parser)) != 0) {
		if (type == JSON_TYPE_PAIR_NAME) {
			char buffer[64];
			os_bzero(buffer, 64);
			if (jsonparse_strcmp_value(parser, "cmd") == 0) {
				jsonparse_next(parser);
				jsonparse_next(parser);
				jsonparse_copy_value(parser, buffer, sizeof(buffer));
				if (!strcoll(buffer, "on")) {
#ifdef MENJINDEBUG
					os_printf("JSON: door open \n");
#endif
					easygpio_outputSet(JIDIANQI_IO, 0);
				} else if (!strcoll(buffer, "off")) {
#ifdef MENJINDEBUG
					os_printf("JSON: door clease \n");
#endif
//					OLED_Print(0, 2, "              ", 2);
					OLED_Print(0, 2, "    Permission  ", 2);
//					OLED_Print(0, 4, "              ", 2);
					OLED_Print(0, 4, "      Denied    ", 2);
					easygpio_outputSet(JIDIANQI_IO, 1);
				}
			}
		}
	}
	return 0;
}

LOCAL int ICACHE_FLASH_ATTR
user_info_json_set(struct jsontree_context *js_ctx,
		struct jsonparse_state *parser) {
	int type;
	while ((type = jsonparse_next(parser)) != 0) {
		if (type == JSON_TYPE_PAIR_NAME) {
			char buffer[64];
			os_bzero(buffer, 64);
			if (jsonparse_strcmp_value(parser, "studentID") == 0) {
				jsonparse_next(parser);
				jsonparse_next(parser);
				jsonparse_copy_value(parser, buffer, sizeof(buffer));
#ifdef ESP8266OLED
				if (!strcoll(buffer, "0000000000")) {

				} else {
					//FIXME 添加OLED显示学号
//					OLED_Print(0, 2, "                ", 2);
					OLED_Print(0, 2, "     Welcome    ", 2);
					OLED_Print(0, 4, "    ", 2);
					OLED_Print(3, 4, buffer, 2);
				}
#endif
#ifdef MENJINDEBUG
				os_printf("\nstudent ID :%s\n", buffer);
#endif
			}
		}
	}
	return 0;
}
LOCAL int ICACHE_FLASH_ATTR
card_info_json_get(struct jsontree_context *js_ctx) {
	const char *path = jsontree_path_name(js_ctx, js_ctx->depth - 1);
	jsontree_write_string(js_ctx, uidstr);
	return 0;
}
LOCAL struct ICACHE_FLASH_ATTR jsontree_callback humidity_callback =
JSONTREE_CALLBACK(json_get, NULL);
LOCAL struct ICACHE_FLASH_ATTR jsontree_callback temperature_callback =
JSONTREE_CALLBACK(temp_json_get, NULL);
LOCAL struct ICACHE_FLASH_ATTR jsontree_callback cardid_info_tree_cb =
JSONTREE_CALLBACK(card_info_json_get, NULL);
LOCAL struct ICACHE_FLASH_ATTR jsontree_callback userid_info_tree_cb =
JSONTREE_CALLBACK(NULL, user_info_json_set);
LOCAL struct ICACHE_FLASH_ATTR jsontree_callback open_door_cmd_tree_cb =
JSONTREE_CALLBACK(NULL, open_door_cmd_json_set);
JSONTREE_OBJECT(open_door_cmd_tree,
		JSONTREE_PAIR("cmd",&open_door_cmd_tree_cb));
JSONTREE_OBJECT(environment_tree, JSONTREE_PAIR("humidity", &humidity_callback),
		JSONTREE_PAIR("temperature", &temperature_callback));
JSONTREE_OBJECT(cardid_tree, JSONTREE_PAIR("cardID",&cardid_info_tree_cb));
JSONTREE_OBJECT(userid_tree, JSONTREE_PAIR("studentID",&userid_info_tree_cb));
JSONTREE_OBJECT(cardid_info_tree, JSONTREE_PAIR("cardID",&cardid_tree));
JSONTREE_OBJECT(open_door_info_tree, JSONTREE_PAIR("cmd",&open_door_cmd_tree),
		JSONTREE_PAIR("studentID",&userid_tree));

LOCAL int ICACHE_FLASH_ATTR
date_info_json_set(struct jsontree_context *js_ctx,
		struct jsonparse_state *parser) {
	int type;
	while ((type = jsonparse_next(parser)) != 0) {
		if (type == JSON_TYPE_PAIR_NAME) {
			char buffer[64];
			os_bzero(buffer, 64);
			if (jsonparse_strcmp_value(parser, "date") == 0) {
				jsonparse_next(parser);
				jsonparse_next(parser);
				jsonparse_copy_value(parser, buffer, sizeof(buffer));
				//TODO 添加显示日期!
				OLED_Print(0, 4, "                        ", 1);
				OLED_Print(0, 5, "     ", 1);
				OLED_Print(5, 5, buffer, 1);
				OLED_Print(15, 5, "    ", 1);
			}
		}
	}
	return 0;
}
LOCAL int ICACHE_FLASH_ATTR
time_info_json_set(struct jsontree_context *js_ctx,
		struct jsonparse_state *parser) {
	int type;
	while ((type = jsonparse_next(parser)) != 0) {
		if (type == JSON_TYPE_PAIR_NAME) {
			char buffer[64];
			os_bzero(buffer, 64);
			if (jsonparse_strcmp_value(parser, "time") == 0) {
				jsonparse_next(parser);
				jsonparse_next(parser);
				jsonparse_copy_value(parser, buffer, sizeof(buffer));
				//TODO 添加显示时间!
				OLED_Print(0, 2, "      ", 2);
				OLED_Print(5, 2, buffer, 2);
				OLED_Print(10, 2, "     ", 2);

			}
		}
	}
	return 0;
}
LOCAL struct ICACHE_FLASH_ATTR jsontree_callback date_tree_cb =
JSONTREE_CALLBACK(NULL, date_info_json_set);
LOCAL struct ICACHE_FLASH_ATTR jsontree_callback time_tree_cb =
JSONTREE_CALLBACK(NULL, time_info_json_set);
JSONTREE_OBJECT(date_tree, JSONTREE_PAIR("date",&date_tree_cb));
JSONTREE_OBJECT(time_tree, JSONTREE_PAIR("time",&time_tree_cb));
JSONTREE_OBJECT(time_info_tree, JSONTREE_PAIR("date",&date_tree),
		JSONTREE_PAIR("time",&time_tree));
void smartconfig_done(sc_status status, void *pdata) {
	switch (status) {
	case SC_STATUS_WAIT:
		os_printf("SC_STATUS_WAIT\n");
		break;
	case SC_STATUS_FIND_CHANNEL:
		os_printf("SC_STATUS_FIND_CHANNEL\n");
		break;
	case SC_STATUS_GETTING_SSID_PSWD:
		os_printf("SC_STATUS_GETTING_SSID_PSWD\n");
		sc_type *type = pdata;
		if (*type == SC_TYPE_ESPTOUCH) {
			os_printf("SC_TYPE:SC_TYPE_ESPTOUCH\n");
		} else {
			os_printf("SC_TYPE:SC_TYPE_AIRKISS\n");
		}
		break;
	case SC_STATUS_LINK:
		os_printf("SC_STATUS_LINK\n");
		struct station_config *sta_conf = pdata;
		wifi_station_set_config(sta_conf);
		wifi_station_disconnect();
		wifi_station_connect();
		break;
	case SC_STATUS_LINK_OVER:
		os_printf("SC_STATUS_LINK_OVER\n");
		if (pdata != NULL) {
			//SC_TYPE_ESPTOUCH
			uint8 phone_ip[4] = { 0 };
			os_memcpy(phone_ip, (uint8*) pdata, 4);
			os_printf("Phone ip: %d.%d.%d.%d\n", phone_ip[0], phone_ip[1],
					phone_ip[2], phone_ip[3]);
		}
		smartconfig_stop();
		break;
	}
}

MQTT_Client mqttClient;

void ICACHE_FLASH_ATTR mqttConnectedCb(uint32_t *args) {
	MQTT_Client* client = (MQTT_Client*) args;
	os_printf("MQTT: Connected\r\n");
#ifdef ESP8266DOOR
	MQTT_Subscribe(client, "/tc503/door", 0);

#endif
	MQTT_Subscribe(client, "/tc503/reset", 0);
//#ifdef ESP8266OLED
//	MQTT_Subscribe(client, "/tc503/time", 0);
//#endif
}

void ICACHE_FLASH_ATTR mqttDisconnectedCb(uint32_t *args) {
	MQTT_Client* client = (MQTT_Client*) args;
	os_printf("MQTT: Disconnected\r\n");
}

void ICACHE_FLASH_ATTR mqttPublishedCb(uint32_t *args) {
	MQTT_Client* client = (MQTT_Client*) args;
#ifdef ESP8266DEBUG
	os_printf("MQTT: Published\r\n");
#endif
}

void ICACHE_FLASH_ATTR mqttDataCb(uint32_t *args, const char* topic,
		uint32_t topic_len, const char *data, uint32_t data_len) {
	char *topicBuf = (char*) os_zalloc(topic_len + 1), *dataBuf =
			(char*) os_zalloc(data_len + 1);

	MQTT_Client* client = (MQTT_Client*) args;

	os_memcpy(topicBuf, topic, topic_len);
	topicBuf[topic_len] = 0;

	os_memcpy(dataBuf, data, data_len);
	dataBuf[data_len] = 0;
#ifdef MENJINDEBUG
	os_printf("Receive topic: %s, data: %s \r\n", topicBuf, dataBuf);
#endif
#ifdef ESP8266DOOR
	if (!strcoll(topicBuf, "/tc503/door")) {
		struct jsontree_context js;
		jsontree_setup(&js, (struct jsontree_value *) &open_door_info_tree,
				json_putchar);
		json_parse(&js, dataBuf);
	}
#endif
	if (!strcoll(topicBuf, "/tc503/reset")) {
		if (!strcoll(dataBuf, "1")){
			system_restart();
		}
	}
	os_free(topicBuf);
	os_free(dataBuf);
//#ifdef ESP8266OLED
//	if (!strcoll(topicBuf, "/tc503/time")) {
//		struct jsontree_context js;
//		jsontree_setup(&js, (struct jsontree_value *) &time_info_tree,
//				json_putchar);
//		json_parse(&js, dataBuf);
//	}
//	os_free(topicBuf);
//	os_free(dataBuf);
//#endif
}

void ICACHE_FLASH_ATTR user_rf_pre_init(void) {
}
void ICACHE_FLASH_ATTR pn532_cb_event_init() {
	os_timer_disarm(&pn532readID);
	os_timer_setfn(&pn532readID, (os_timer_func_t *) pn532_send_read_cmd_cb,
			(void *) 0);
	os_timer_arm(&pn532readID, PN532_SEND_READ_CMD_DELAY, 1);

}
void ICACHE_FLASH_ATTR key_and_gpio_init() {

	peri_config_key_init(4, peri_key_long_press, peri_key_short_press);
	base_keys_init();
}
void ICACHE_FLASH_ATTR wifi_handle_event_cb(System_Event_t *evt) {
	os_printf("event %x\n", evt->event);
	switch (evt->event) {
	case EVENT_STAMODE_CONNECTED:
		os_printf("connect to ssid %s, channel %d\n",
				evt->event_info.connected.ssid,
				evt->event_info.connected.channel);
		MQTT_Disconnect(&mqttClient);
		break;
	case EVENT_STAMODE_DISCONNECTED:
		os_printf("disconnect from ssid %s, reason %d\n",
				evt->event_info.disconnected.ssid,
				evt->event_info.disconnected.reason);
		MQTT_Disconnect(&mqttClient); //断开mqtt连接
		break;
	case EVENT_STAMODE_AUTHMODE_CHANGE:
		os_printf("mode: %d -> %d\n", evt->event_info.auth_change.old_mode,
				evt->event_info.auth_change.new_mode);
		MQTT_Disconnect(&mqttClient);
		break;
	case EVENT_STAMODE_GOT_IP:
		os_printf("ip:" IPSTR ",mask:" IPSTR ",gw:" IPSTR,
				IP2STR(&evt->event_info.got_ip.ip),
				IP2STR(&evt->event_info.got_ip.mask),
				IP2STR(&evt->event_info.got_ip.gw));
		os_printf("\n");
		os_printf("mqtt start connect!\n");
		MQTT_Connect(&mqttClient);
		break;
	case EVENT_SOFTAPMODE_STACONNECTED:
		os_printf("station: " MACSTR "join, AID = %d\n",
				MAC2STR(evt->event_info.sta_connected.mac),
				evt->event_info.sta_connected.aid);
		MQTT_Disconnect(&mqttClient);
		break;
	case EVENT_SOFTAPMODE_STADISCONNECTED:
		os_printf("station: " MACSTR "leave, AID = %d\n",
				MAC2STR(evt->event_info.sta_disconnected.mac),
				evt->event_info.sta_disconnected.aid);
		MQTT_Disconnect(&mqttClient);
		break;
	default:
		break;
	}
}
void ICACHE_FLASH_ATTR check_door_status_cb(void *arg) {
	os_timer_disarm(&check_door_status_timer);
	if (!easygpio_inputGet(DOOR_STATUS_IO)) {
		easygpio_outputSet(JIDIANQI_IO, 1);
	}
	os_timer_arm(&check_door_status_timer, CHECK_DOOR_DELAY, 1);
}
void ICACHE_FLASH_ATTR clean_oled_cb(void *arg) {
	os_timer_disarm(&clean_oled_timer);
	OLED_Print(0, 2, "              ", 2);
	OLED_Print(0, 4, "              ", 2);
	os_timer_arm(&clean_oled_timer, CLEAN_OLED_DELAY, 1);
}
void ICACHE_FLASH_ATTR door_switch_init() {
	easygpio_pinMode(JIDIANQI_IO, EASYGPIO_NOPULL, EASYGPIO_OUTPUT);
	easygpio_pinMode(DOOR_STATUS_IO, EASYGPIO_NOPULL, EASYGPIO_INPUT);
	easygpio_outputSet(JIDIANQI_IO, 1);
	os_timer_disarm(&check_door_status_timer);
	os_timer_setfn(&check_door_status_timer,
			(os_timer_func_t *) check_door_status_cb, (void *) 0);
	os_timer_arm(&check_door_status_timer, CHECK_DOOR_DELAY, 1);
}

void ICACHE_FLASH_ATTR oled_init() {
	OLED_Init();
	OLED_ON();
	OLED_Print(0, 0, " WXIT Smart Lab", 2);
	os_timer_disarm(&clean_oled_timer);
	os_timer_setfn(&clean_oled_timer, (os_timer_func_t *) clean_oled_cb,
			(void *) 0);
	os_timer_arm(&clean_oled_timer, CLEAN_OLED_DELAY, 1);
}
void ICACHE_FLASH_ATTR user_init(void) {
	uart_init(BIT_RATE_115200, BIT_RATE_115200);
	os_printf("SDK version:%s\n", system_get_sdk_version());
	i2c_init();
#ifdef ESP8266OLED
	oled_init();
#endif
	key_and_gpio_init();
#ifdef ESP8266DOOR
	door_switch_init();
#endif
	wifi_set_event_handler_cb(wifi_handle_event_cb);
	mqttstart();
#ifdef ESP8266ENVINFO
	task_send_environment_info_init();
#endif
#ifdef ESP8266CARDREAD
	pn532_init();
	pn532_cb_event_init();
#endif
	struct station_config s_staconf;
	wifi_station_get_config_default(&s_staconf);
	if (os_strlen(s_staconf.ssid) == 0) {
		wifi_set_opmode(STATION_MODE);
		smartconfig_set_type(SC_TYPE_ESPTOUCH);
		smartconfig_start(smartconfig_done);
	}
}
void ICACHE_FLASH_ATTR hextochar(uint8 *numchar, uint8 *hex, uint8 len) {
	uint8 i, high, low;
	for (i = 0; i < len; ++i) {
		high = hex[i] >> 4;
		low = hex[i] & 0xf;
		numchar[i * 2] = num[high];
		numchar[i * 2 + 1] = num[low];
	}
}
char ICACHE_FLASH_ATTR * combine_strings(char *a, char *b) {
	char *ptr;
	int lena = os_strlen(a), lenb = os_strlen(b);
	int i, l = 0;
	ptr = (char *) os_malloc((lena + lenb + 1) * sizeof(char));
	for (i = 0; i < lena; i++)
		ptr[l++] = a[i];
	for (i = 0; i < lenb; i++)
		ptr[l++] = b[i];
	ptr[l] = '\0';
	return (ptr);
}
char ICACHE_FLASH_ATTR * combine_env_strings(char *temp, char * humi) {
	char *ptr;
	ptr = combine_strings("T:", temp);
	ptr = combine_strings(ptr, "*C H:");
	ptr = combine_strings(ptr, humi);
	ptr = combine_strings(ptr, "%");
	return ptr;
}
void ICACHE_FLASH_ATTR pn532_send_read_cmd_cb() {
	os_timer_disarm(&pn532readID);
	uint8 uid[4];
	uint8 *uidlen;
	bool a = readPassiveTargetID(PN532_MIFARE_ISO14443A, &uid, &uidlen, 20);
	uint8 i;
	if (a) {
#ifdef MENJINDEBUG
		os_printf("\nUID:");
		for (i = 0; i < 4; i++) {
			os_printf("0x%x ", uid[i]);
		}
		os_printf("\n");
#endif
		char *id = "ID:";
		hextochar(uidstr, uid, 4);
		uidstr[8] = '\0';
		char *json_buf = NULL;
		json_buf = (char *) os_zalloc(jsonSize);
		json_ws_send((struct jsontree_value *) &cardid_info_tree, "cardID",
				json_buf);
		MQTT_Publish(&mqttClient, "/tc503/onCardRead", json_buf,
				strlen(json_buf), 0, 0);
		os_free(json_buf);
		json_buf = NULL;
	}
	os_timer_arm(&pn532readID, PN532_SEND_READ_CMD_DELAY, 1);
}
void ICACHE_FLASH_ATTR get_dht11(uint8_t *bufftemperature,
		uint8_t *buffhumidity) {
	DHT_Sensor_Data data;
	uint8_t pin;
	pin = pin_num[sensor.pin];
	if (DHTRead(&sensor, &data)) {
#ifdef MENJINDEBUG
		os_printf("GPIO%d\r\n", pin);
		os_printf("Temperature: %s *C\r\n",
				DHTFloat2String(bufftemperature, data.temperature));
#endif
		DHTFloat2String(bufftemperature, data.temperature);
#ifdef MENJINDEBUG
		os_printf("Humidity: %s %%\r\n",
				DHTFloat2String(buffhumidity, data.humidity));
#endif
		DHTFloat2String(buffhumidity, data.humidity);
#ifdef ESP8266OLED
		OLED_Print(2, 7, combine_env_strings(bufftemperature, buffhumidity), 1);
#endif
		if (lasthumidity != data.humidity
				|| lasttemperature != data.temperature)
			isNeedSendENV = true;
		else
			isNeedSendENV = false;
		lasttemperature = data.temperature;
		lasthumidity = data.humidity;
	} else {
#ifdef MENJINDEBUG
		os_printf("Failed to read temperature and humidity sensor on GPIO%d\n",
				pin);
#endif
	}
	os_free(data);
	os_free(pin);
}
void ICACHE_FLASH_ATTR task_send_environment_info_init() {
	sensor.pin = 6;
	sensor.type = DHT11;
	DHTInit(&sensor);
	os_timer_disarm(&get_environment_info_timer);
	os_timer_setfn(&get_environment_info_timer,
			(os_timer_func_t *) get_environment_info_cb, (void *) 0);
	os_timer_arm(&get_environment_info_timer, DELAY, 1);
	get_dht11(temperature, humidity);
}
void ICACHE_FLASH_ATTR mqttstart() {
#ifdef MENJINDEBUG
	os_printf("mqtt client starting....");
#endif
	MQTT_InitConnection(&mqttClient, "192.168.1.1", 1883, 0);
	MQTT_InitClient(&mqttClient, "esp_8266", "", "", 10, 1);
	MQTT_InitLWT(&mqttClient, "/lwt", "offline", 0, 0);
	MQTT_OnConnected(&mqttClient, mqttConnectedCb);
	MQTT_OnDisconnected(&mqttClient, mqttDisconnectedCb);
	MQTT_OnPublished(&mqttClient, mqttPublishedCb);
	MQTT_OnData(&mqttClient, mqttDataCb);
}
/******************************************************************************
 * FunctionName : user_plug_short_press
 * Description  : key's short press function, needed to be installed
 * Parameters   : none
 * Returns      : none
 *******************************************************************************/
void ICACHE_FLASH_ATTR
peri_key_short_press(void) {
	os_printf(" 4 short press..\n");
}
/******************************************************************************
 * FunctionName : user_plug_long_press
 * Description  : key's long press function, needed to be installed, preserved function.
 * Parameters   : none
 * Returns      : none
 *******************************************************************************/
void ICACHE_FLASH_ATTR
peri_key_long_press(void) {
	os_printf("long press..\n");
	os_printf("clean data and restart!");
	system_restore();
	system_restart();
}

void ICACHE_FLASH_ATTR get_environment_info_cb(void *arg) {
	os_timer_disarm(&get_environment_info_timer);
	get_dht11(temperature, humidity);
	// Send data the MQTT broker
	if (isNeedSendENV) {
		char *json_buf = NULL;
		json_buf = (char *) os_zalloc(jsonSize);
		json_ws_send((struct jsontree_value *) &environment_tree, "environment",
				json_buf);
		MQTT_Publish(&mqttClient, "/tc503/environment", json_buf,
				strlen(json_buf), 0, 0);
		os_free(json_buf);
		json_buf = NULL;
	}
	os_timer_arm(&get_environment_info_timer, DELAY, 1);
}

