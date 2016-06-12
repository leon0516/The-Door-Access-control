/*
 * pn532test.c
 *
 *  Created on: 2016年3月24日
 *      Author: Leon
 */
#include "../pn532/pn532.h"
#include "../easygpio/easygpio.h"
#include "../driver/i2c.h"

#define RESET 14
#define HIGH 1
#define LOW 0
//#define PN532DEBUG
uint8 pn532ack[] = { 0x00, 0x00, 0xFF, 0x00, 0xFF, 0x00 };
uint8 pn532response_firmwarevers[] = { 0x00, 0xFF, 0x06, 0xFA, 0xD5, 0x03 };
/**************************************************************************/
/*!
 @brief  Sends a single byte via I2C

 @param  x    The byte to send
 */
/**************************************************************************/
static inline void i2c_send(uint8_t x) {
	i2c_writeByte(x);
	if (!i2c_check_ack()) {
#ifdef PN532DEBUG
		os_printf(
				"\n[ERROR]:No ACK received When Write CMD! in file:%s func:%s\n",
				__FILE__, __FUNCTION__);
#endif
		i2c_stop();
		return;
	}
}
void pn532_init() {
	easygpio_pinMode(IRQ, EASYGPIO_NOPULL, EASYGPIO_INPUT);
	uint8 cmd[2]={0x14,0x01};
    sendCommandCheckAck(cmd,2,1000);
}


/**************************************************************************/
/*!
 @brief  Tries to read the SPI or I2C ACK signal
 */
/**************************************************************************/
bool ICACHE_FLASH_ATTR readack() {
	uint8_t ackbuff[6];

	readdata(ackbuff, 6);

	return (0 == os_strncmp((char *) ackbuff, (char *) pn532ack, 6));
}

bool isready() {
	// I2C check if status is ready by IRQ line being pulled low.
	uint8_t x = easygpio_inputGet(IRQ);
	return x == 0;
}
/**************************************************************************/
/*!
 @brief  Waits until the PN532 is ready.

 @param  timeout   Timeout before giving up
 */
/**************************************************************************/
bool ICACHE_FLASH_ATTR waitready(uint16_t timeout) {
	uint16_t timer = 0;
	while (!isready()) {
		system_soft_wdt_feed();
		if (timeout != 0) {
			timer++;
			if (timer > timeout) {
#ifdef PN432DEBUG
				os_printf("\n[ERROR]:Timeout When wait ready! in file:%s func:%s\n", __FILE__,
						__FUNCTION__);
#endif
				return false;
			}
		}
		delay(1);
	}
	return true;
}
void delay(uint16 ms) {
	for (; ms > 0; ms--) {
		os_delay_us(1000);
	}
}
/**************************************************************************/
/*!
 @brief  Reads n bytes of data from the PN532 via SPI or I2C.

 @param  buff      Pointer to the buffer where data will be written
 @param  n         Number of bytes to be read
 */
/**************************************************************************/
void ICACHE_FLASH_ATTR readdata(uint8_t* buff, uint8_t n) {
#ifdef PN532DEBUG
	os_printf("Reading: ");
#endif
	uint8 ack;
	uint16 i;
	i2c_start();
	i2c_writeByte(PN532_I2C_ADDRESS + 1);
	if (!i2c_check_ack()) {
		os_printf(
				"\n[ERROR]:No ACK received When Read Date! in file:%s func:%s\n",
				__FILE__, __FUNCTION__);
		i2c_stop();
		return;
	}
#ifdef PN532DEBUG
	os_printf("\n[INFO]:read date result:\n");
#endif
	i2c_readByte();
	i2c_send_ack(1);
	for (i = 0; i < n; i++) {
		buff[i] = i2c_readByte();
#ifdef PN532DEBUG
		os_printf("0x%x  ", buff[i]);
#endif
		i2c_send_ack((i == (n - 1)) ? 0 : 1);
	}
	i2c_stop();

#ifdef PN532DEBUG
	os_printf("\n[INFO]:read date ok");
	os_printf("\n");
#endif
}
/**************************************************************************/
/*!
 @brief  Checks the firmware version of the PN5xx chip

 @returns  The chip's firmware version and ID
 */
/**************************************************************************/
uint32_t ICACHE_FLASH_ATTR getFirmwareVersion(void) {
	uint32_t response;
	uint8 pn532_packetbuffer[64];
	pn532_packetbuffer[0] = PN532_COMMAND_GETFIRMWAREVERSION;

	if (!sendCommandCheckAck(pn532_packetbuffer, 1, 1000))
		return 0;
	waitready(1000); // 读取数据前等待
	// read data packet
	readdata(pn532_packetbuffer, 12);
	// check some basic stuff
	if (0
			!= os_strncmp((char *) pn532_packetbuffer,
					(char *) pn532response_firmwarevers, 6)) {
#ifdef PN532DEBUG
		os_printf("\n[ERROR]:Firmware doesn't match! in file:%s func:%s\n",
		__FILE__, __FUNCTION__);

#endif
		return 0;
	}
	int offset = 7; // Skip a response byte when using I2C to ignore extra data.
	response = pn532_packetbuffer[offset++];
	response <<= 8;
	response |= pn532_packetbuffer[offset++];
	response <<= 8;
	response |= pn532_packetbuffer[offset++];
	response <<= 8;
	response |= pn532_packetbuffer[offset++];

	return response;
}
/**************************************************************************/
/*!
 @brief  Writes a command to the PN532, automatically inserting the
 preamble and required frame details (checksum, len, etc.)

 @param  cmd       Pointer to the command buffer
 @param  cmdlen    Command length in bytes
 */
/**************************************************************************/
void ICACHE_FLASH_ATTR writecommand(uint8* cmd, uint8 cmdlen) {
	// I2C command write.
	uint8 checksum;
	uint8 i;
	cmdlen++;

#ifdef PN532DEBUG
	os_printf("\nSending: ");
#endif
	// I2C START
	i2c_start();//FIXME 验证一下，这里可不可以去掉，在正常模式下
	i2c_writeByte(PN532_I2C_ADDRESS);
	delay(2);
	i2c_stop();
	delay(2); //   or whatever the delay is for waking up the board
	i2c_start();
	i2c_send(PN532_I2C_ADDRESS);
	checksum = PN532_PREAMBLE + PN532_PREAMBLE + PN532_STARTCODE2;
	i2c_send(PN532_PREAMBLE);
	i2c_send(PN532_PREAMBLE);
	i2c_send(PN532_STARTCODE2);

	i2c_send((uint8) cmdlen);
	i2c_send((uint8) (~cmdlen + 1));

	i2c_send(PN532_HOSTTOPN532);
	checksum += PN532_HOSTTOPN532;

#ifdef PN532DEBUG
	os_printf(" 0x");
	os_printf("%x", PN532_PREAMBLE);
	os_printf(" 0x");
	os_printf("%x", PN532_PREAMBLE);
	os_printf(" 0x");
	os_printf("%x", PN532_STARTCODE2);
	os_printf(" 0x");
	os_printf("%x", (uint8) cmdlen);
	os_printf(" 0x");
	os_printf("%x", (uint8) (~cmdlen + 1));
	os_printf(" 0x");
	os_printf("%x", PN532_HOSTTOPN532);
#endif
	for (i = 0; i < cmdlen - 1; i++) {
		i2c_send(cmd[i]);
		checksum += cmd[i];
#ifdef PN532DEBUG
		os_printf(" 0x");
		os_printf("%x", cmd[i]);
#endif
	}
	i2c_send((uint8) (~checksum));
	i2c_send(PN532_POSTAMBLE);

	// I2C STOP
	i2c_stop();

#ifdef PN532DEBUG
	os_printf(" 0x");
	os_printf("%x", (uint8) (~checksum));
	os_printf(" 0x");
	os_printf("%x", PN532_POSTAMBLE);
	os_printf("\n");
#endif

}

bool ICACHE_FLASH_ATTR sendCommandCheckAck(uint8_t *cmd, uint8_t cmdlen,
		uint16_t timeout) {
	// write the command
	writecommand(cmd, cmdlen);
	// Wait for chip to say its ready!
	if (!waitready(timeout)) {
		return false;
	}
#ifdef PN532DEBUG
	os_printf("IRQ received\n");
#endif
	// read acknowledgement
	if (!readack()) {
#ifdef PN532DEBUG
		os_printf("No ACK frame received!\n");
#endif
		return false;
	}
	return true; // ack'd command
}/**************************************************************************/
/*!
 @brief  'InLists' a passive target. PN532 acting as reader/initiator,
 peer acting as card/responder.
 */
/**************************************************************************/
bool ICACHE_FLASH_ATTR inListPassiveTarget() {
	uint8 _inListedTag;
	uint8 pn532_packetbuffer[64];
	pn532_packetbuffer[0] = PN532_COMMAND_INLISTPASSIVETARGET;
	pn532_packetbuffer[1] = 1;
	pn532_packetbuffer[2] = 0;

#ifdef PN532DEBUG
	os_printf("[INFO]:inList passive target\n");
#endif

	if (!sendCommandCheckAck(pn532_packetbuffer, 3, 1000)) {
#ifdef PN532DEBUG
		os_printf("Could not send inlist message\n");
#endif
		return false;
	}
	delay(3000);
	if (!waitready(3000)) {
		return false;
	}

	readdata(pn532_packetbuffer, sizeof(pn532_packetbuffer));

	if (pn532_packetbuffer[0] == 0 && pn532_packetbuffer[1] == 0
			&& pn532_packetbuffer[2] == 0xff) {
		uint8_t length = pn532_packetbuffer[3];
		if (pn532_packetbuffer[4] != (uint8_t) (~length + 1)) {
#ifdef PN532DEBUG
			os_printf("Length check invalid\n");
			os_printf("%x", length);
			os_printf("%x", (~length) + 1);
#endif
			return false;
		}
		if (pn532_packetbuffer[5] == PN532_PN532TOHOST
				&& pn532_packetbuffer[6] == PN532_RESPONSE_INLISTPASSIVETARGET) {
			if (pn532_packetbuffer[7] != 1) {
#ifdef PN532DEBUG
				os_printf("Unhandled number of targets inlisted");
#endif
				os_printf("Number of tags inlisted:");
				os_printf(pn532_packetbuffer[7]);
				os_printf("\n");
				return false;
			}

			_inListedTag = pn532_packetbuffer[8];
			os_printf("Tag number: ");
			os_printf(_inListedTag);
			os_printf("\n");
			return true;
		} else {
#ifdef PN532DEBUG
			os_printf("Unexpected response to inlist passive host\n");
#endif
			return false;
		}
	} else {
#ifdef PN532DEBUG
		os_printf("Preamble missing\n");
#endif
		return false;
	}

	return true;
}
/**************************************************************************/
/*!
 Waits for an ISO14443A target to enter the field

 @param  cardBaudRate  Baud rate of the card
 @param  uid           Pointer to the array that will be populated
 with the card's UID (up to 7 bytes)
 @param  uidLength     Pointer to the variable that will hold the
 length of the card's UID.

 @returns 1 if everything executed properly, 0 for an error
 */
/**************************************************************************/
bool ICACHE_FLASH_ATTR readPassiveTargetID(uint8_t cardbaudrate, uint8_t * uid,
		uint8_t * uidLength, uint16_t timeout) {
	uint8_t pn532_packetbuffer_test[64];
	pn532_packetbuffer_test[0] = PN532_COMMAND_INLISTPASSIVETARGET;
	pn532_packetbuffer_test[1] = 1; // max 1 cards at once (we can set this to 2 later)
	pn532_packetbuffer_test[2] = cardbaudrate;

	if (!sendCommandCheckAck(pn532_packetbuffer_test, 3, timeout)) {
#ifdef PN532DEBUG
		os_printf("No card(s) read");
#endif
		return 0x0;  // no cards read
	}

	// wait for a card to enter the field (only possible with I2C)
#ifdef PN532DEBUG
	os_printf("Waiting for IRQ (indicates card presence)");
#endif
	if (!waitready(timeout+30)) {
#ifdef PN532DEBUG
		os_printf("\nIRQ Timeout\n");
#endif
		return 0x0;
	}

	// read data packet
	readdata(pn532_packetbuffer_test, 20);
	// check some basic stuff

	/* ISO14443A card response should be in the following format:

	 byte            Description
	 -------------   ------------------------------------------
	 b0..6           Frame header and preamble
	 b7              Tags Found
	 b8              Tag Number (only one used in this example)
	 b9..10          SENS_RES
	 b11             SEL_RES
	 b12             NFCID Length
	 b13..NFCIDLen   NFCID                                      */

#ifdef PN532DEBUG
	os_printf("Found ");
	os_printf("%d",pn532_packetbuffer_test[7]);
	os_printf(" tags");
#endif
	if (pn532_packetbuffer_test[7] != 1)
		return 0;

	uint16_t sens_res = pn532_packetbuffer_test[9];
	sens_res <<= 8;
	sens_res |= pn532_packetbuffer_test[10];
#ifdef PN532DEBUG
	os_printf("ATQA: 0x");
	os_printf("%x", sens_res);
	os_printf("SAK: 0x");
	os_printf("%x", pn532_packetbuffer_test[11]);
#endif

	/* Card appears to be Mifare Classic */
	*uidLength = pn532_packetbuffer_test[12];
#ifdef PN532DEBUG
	os_printf("UID:");
#endif
	uint8_t i;
	for (i = 0; i < pn532_packetbuffer_test[12]; i++) {
		uid[i] = pn532_packetbuffer_test[13 + i];
#ifdef PN532DEBUG
		os_printf(" 0x");
		os_printf("%x", uid[i]);
#endif
	}
#ifdef PN532DEBUG
	os_printf("\n");
#endif
	return 1;
}

