/*
 * mqtt_task.h
 *
 *  Created on: Jun 6, 2020
 *      Author: chatchai
 */


#include "mqtt_client.h"

esp_mqtt_client_handle_t *client;
esp_err_t mqtt_setup(void) ;
static uint8_t base_mac_addr[6] = { 0 };
char str_chipid[6];
