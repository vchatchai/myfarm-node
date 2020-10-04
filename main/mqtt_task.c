#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "freertos/timers.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"
#include "sdkconfig.h"
#include "mqtt_client.h"
#include "sensors_task.h"
#include "mcp23017_task.h"
#include "mqtt_task.h"
#include "cJSON.h"

static const char *TAG = "MQTT_TASK";

//CONFIG_LOG_DEFAULT_LEVEL = 4;

//char* create_objects();
esp_err_t mqtt_event_handler(esp_mqtt_event_handle_t event) {
	esp_mqtt_client_handle_t client = event->client;
	int msg_id;
	// your_context_t *context = event->context;
	switch (event->event_id) {
	case MQTT_EVENT_CONNECTED:
		ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");

//		msg_id = esp_mqtt_client_publish(client, "/topic/qos0", "data", 0, 1,
//				0);
		char topic[] = "/myfarm/node/";
		strcat(topic,str_chipid);
		ESP_LOGI(TAG, "esp_mqtt_client_subscribe=%s", topic);
		msg_id = esp_mqtt_client_subscribe(client, topic, 0);
		ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

//		msg_id = esp_mqtt_client_subscribe(client, "/topic/qos1", 1);
//		ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);
//
//		msg_id = esp_mqtt_client_unsubscribe(client, "/topic/qos1");
//		ESP_LOGI(TAG, "sent unsubscribe successful, msg_id=%d", msg_id);
		break;
	case MQTT_EVENT_DISCONNECTED:
		ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
		break;

	case MQTT_EVENT_SUBSCRIBED:
		ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
//		msg_id = esp_mqtt_client_publish(client, "/topic/qos0", "data", 0, 0,
//				0);
//		ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
		break;
	case MQTT_EVENT_UNSUBSCRIBED:
		ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
		break;
	case MQTT_EVENT_PUBLISHED:
		ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
		break;
	case MQTT_EVENT_DATA:
		ESP_LOGI(TAG, "MQTT_EVENT_DATA");
		printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
		printf("DATA=%.*s\r\n", event->data_len, event->data);


		cJSON *json = cJSON_Parse(event->data);
		char *string = cJSON_Print(json);
		printf(string);

	    if (json == NULL)
	    {
	        const char *error_ptr = cJSON_GetErrorPtr();
	        if (error_ptr != NULL)
	        {
	            fprintf(stderr, "Error before: %s\n", error_ptr);
	        }
	    }else {

	    	const cJSON *swtichStatus = cJSON_GetObjectItemCaseSensitive(json, "SWITCH");
	        if (cJSON_IsNumber(swtichStatus)  )
	         {
	             int status = swtichStatus->valueint;

	             ESP_LOGI(TAG, "status=%d", status);
	         }
	    }


		break;
	case MQTT_EVENT_ERROR:
		ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
		break;
	}
	return ESP_OK;
}


TaskHandle_t xTask2Handle = NULL;
static void prvMqttTimerCallback(TimerHandle_t xTimer) {
	TickType_t xTimeNow;
	/* Obtain the current tick count */
	xTimeNow = xTaskGetTickCount();

	cJSON *root;
	root = cJSON_CreateObject();
	char str[80];
	sprintf(str, "%.1f",temperature);
	temperature = atof(str);
	sprintf(str,"%.1f",humidity);
	humidity = atof(str);


	cJSON_AddStringToObject(root, "NodeID", str_chipid);
	cJSON_AddNumberToObject(root, "Temperature", temperature);
	cJSON_AddNumberToObject(root, "Humidity", humidity);
	cJSON_AddStringToObject(root, "Name", "Motor2");
	cJSON_AddNumberToObject(root, "SWITCH", 0);
	cJSON_AddNumberToObject(root, "FLOAT_SWITCH_IN", read_io_port_b.port5);
	cJSON_AddNumberToObject(root, "FLOAT_SWITCH_OUT", read_io_port_b.port7);
	char *out = cJSON_Print(root);

	int msg_id = esp_mqtt_client_publish(client, "/myfarm/node/status", out, 0, 1, 0);
	ESP_LOGI(TAG, "msg id: %d xTaskGetTickCount %d : %s", msg_id, xTimeNow,out);

	if(msg_id < 0) {
		if(xTask2Handle == NULL) {
			xTask2Handle = xTaskCreate(mcp24017_task_led_alert_for_mqtt_connect,
				"mcp24017_task_led_alert_for_mqtt_connect", 1024, NULL, 3, &xTask2Handle);
		}
	}else {
		if(xTask2Handle != NULL) {
			vTaskDelete(xTask2Handle);
			xTask2Handle = NULL;

		}
	}




	cJSON_Delete(root);
	free(out);

}



#define MQTT_AUTO_RELOAD_TIMER_PERIOD pdMS_TO_TICKS( 3000 )

TimerHandle_t xAutoReloadTimer;

esp_err_t mqtt_setup(void) {

	esp_efuse_mac_get_default(base_mac_addr);


	sprintf(str_chipid,"%d%d%d%d%d%d",base_mac_addr[0],base_mac_addr[1],base_mac_addr[2],base_mac_addr[3],base_mac_addr[4],base_mac_addr[5]);

	esp_mqtt_client_config_t mqtt_cfg = { .host = "192.168.1.109",
			.event_handle = mqtt_event_handler, .port = 1883,
			.client_id="motor1"

	};
//    esp_mqtt_client_config_t mqtt_cfg = {
//        .uri = "tcp://192.168.0.10:1883",
//        .event_handle = mqtt_event_handler,
//        // .user_context = (void *)your_context
//    };

	client = esp_mqtt_client_init(&mqtt_cfg);
	esp_err_t err = esp_mqtt_client_start(client);

	printf((err != ESP_OK) ? "mqtt_start Failed!\n" : "mqtt_start Done\n");


//    xTaskCreate(mqtt_task, "mqtt_task", 4096, NULL, 3, NULL);
//	sensor_setup();


//	xTimerCreate("MQTT PUBLISH TASK", MQTT_AUTO_RELOAD_TIMER_PERIOD, true, 0, prvMqttTimerCallback);

	/* Create the auto-reload timer, storing the handle to the created timer in xAutoReloadTimer. */
	xAutoReloadTimer = xTimerCreate(
	/* Text name for the software timer - not used by FreeRTOS. */
	"MQTT publish",
	/* The software timer's period in ticks. */
	MQTT_AUTO_RELOAD_TIMER_PERIOD,
	/* Setting uxAutoRealod to pdTRUE creates an auto-reload timer. */
	pdTRUE,
	/* This example does not use the timer id. */
	0,
	/* The callback function to be used by the software timer being created. */
	prvMqttTimerCallback);

	xTimerStart(xAutoReloadTimer, 0);

	return err;
}

//void app_main()
//{
//    ESP_ERROR_CHECK(nvs_flash_init());
//    ESP_ERROR_CHECK(esp_netif_init());
//    ESP_ERROR_CHECK(esp_event_loop_create_default());
//
//    ESP_ERROR_CHECK(example_connect());
//
//    ESP_LOGI(TAG, "[APP] Startup..");
//    ESP_LOGI(TAG, "[APP] Free memory: %d bytes", esp_get_free_heap_size());
//    ESP_LOGI(TAG, "[APP] IDF version: %s", esp_get_idf_version());
//
//    esp_log_level_set("*", ESP_LOG_INFO);
//    esp_log_level_set("MQTT_CLIENT", ESP_LOG_VERBOSE);
//    esp_log_level_set("TRANSPORT_TCP", ESP_LOG_VERBOSE);
//    esp_log_level_set("TRANSPORT_SSL", ESP_LOG_VERBOSE);
//    esp_log_level_set("TRANSPORT", ESP_LOG_VERBOSE);
//    esp_log_level_set("OUTBOX", ESP_LOG_VERBOSE);
//
//    mqtt_app_start();
//}
