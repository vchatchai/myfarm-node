#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>


#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "driver/gpio.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"
#include "dht.h"
#include "mqtt_client.h"
#include "esp_log.h"
#include "mcp23017_task.h"
#include "sensors_task.h"
#include "mqtt_task.h"

static const char *TAG = "SENSOR_TASK";

int pin_dht22 = 14;



static void prvDHTCallback(TimerHandle_t xTimer) {
	TickType_t xTimeNow;
	/* Obtain the current tick count */
	xTimeNow = xTaskGetTickCount();

	bool result = dht_read_float_data(DHT_TYPE_DHT22, pin_dht22, &humidity,
			&temperature);
	if (result) {

		printf("%d Hum %.1f\n", result, humidity);
		printf("%d Tmp %.1f\n", result, temperature);


//		cJSON *root;
//		root = cJSON_CreateObject();
//		char str[80];
//		sprintf(str, "%.1f",temperature);
//		temperature = atof(str);
//		sprintf(str,"%.1f",humidity);
//		humidity = atof(str);
//
//
//		char str_chipid[6];
//		sprintf(str_chipid,"%d%d%d%d%d%d",base_mac_addr[0],base_mac_addr[1],base_mac_addr[2],base_mac_addr[3],base_mac_addr[4],base_mac_addr[5]);
//
//		cJSON_AddStringToObject(root, "NodeID", str_chipid);
//		cJSON_AddNumberToObject(root, "Temperature", temperature);
//		cJSON_AddNumberToObject(root, "Humidity", humidity);
//		cJSON_AddStringToObject(root, "Name", "Motor2");
//		cJSON_AddNumberToObject(root, "SWITCH", 0);
//		cJSON_AddNumberToObject(root, "FLOAT_SWITCH_IN", read_io_port_b.port5);
//		cJSON_AddNumberToObject(root, "FLOAT_SWITCH_OUT", read_io_port_b.port7);
//		char *out = cJSON_Print(root);
//
//		int msg_id = esp_mqtt_client_publish(client, "/farm_home/node/status", out, 0, 1,
//				0);
//
//		ESP_LOGI(TAG, "msg id: %d xTaskGetTickCount %d : %s", msg_id, xTimeNow,out);
//		cJSON_Delete(root);
//		free(out);
	}

}

#define mainAUTO_RELOAD_TIMER_PERIOD pdMS_TO_TICKS( 3000 )
TimerHandle_t xAutoReloadTimer;

void sensor_setup() {


//	ESP_ERROR_CHECK(


	gpio_config_t io_conf;
	io_conf.pin_bit_mask = GPIO_Pin_14;
	io_conf.intr_type = GPIO_INTR_DISABLE;
	gpio_config(&io_conf);

	/* Create the auto-reload timer, storing the handle to the created timer in xAutoReloadTimer. */
	xAutoReloadTimer = xTimerCreate(
	/* Text name for the software timer - not used by FreeRTOS. */
	"Sensor"
	""
	"",
	/* The software timer's period in ticks. */
	mainAUTO_RELOAD_TIMER_PERIOD,
	/* Setting uxAutoRealod to pdTRUE creates an auto-reload timer. */
	pdTRUE,
	/* This example does not use the timer id. */
	0,
	/* The callback function to be used by the software timer being created. */
	prvDHTCallback);

	xTimerStart(xAutoReloadTimer, 0);

}

void sensor_stop()  {

	xTimerStop(xAutoReloadTimer,0);
}

