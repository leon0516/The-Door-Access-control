/*******************************************************
 * File name: peri_key.c
 * Author: Chongguang Li
 * Versions:1.0
 * Description:This is the key function module.

 * History:
 *   1.Date:
 *     Author:
 *     Modification:
 *********************************************************/

#include "peri_key.h"
#include "user_interface.h"
#include "eagle_soc.h"
#include "key.h"
#include "os_type.h"
#include "mem.h"
#include "tisan_gpio_intr.h"
#include "../driver/uart.h"



void ICACHE_FLASH_ATTR
peri_config_key_init(uint8 gpio_id,key_function long_press, key_function short_press)
{
	struct base_key_param *key = (struct base_key_param *)os_zalloc(sizeof(struct base_key_param));

	key->gpio_id = gpio_id;
	key->gpio_name = tisan_get_gpio_name(gpio_id);
	key->gpio_func = tisan_get_gpio_general_func(gpio_id);
	key->level = 1;
	key->k_function1 = long_press;
	key->k_function2 = short_press;

	os_printf("\r\nkey->gpio_id:%d\r\n", key->gpio_id);
	key_single_init(key);
}

