/* Esptouch example

 This example code is in the Public Domain (or CC0 licensed, at your option.)

 Unless required by applicable law or agreed to in writing, this
 software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
 CONDITIONS OF ANY KIND, either express or implied.
 */

#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_wpa2.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "tcpip_adapter.h"
#include "esp_smartconfig.h"
#include "mcp23017_task.h"
#include "sensors_task.h"
#include "mqtt_task.h"

/* FreeRTOS event group to signal when we are connected & ready to make a request */
static EventGroupHandle_t wifi_event_group;

/* The event group allows multiple bits for each event,
 but we only care about one event - are we connected
 to the AP with an IP? */
static const int CONNECTED_BIT = BIT0;
static const int ESPTOUCH_DONE_BIT = BIT1;
static const char *TAG = "MAIN";
void smartconfig_example_task(void *parm);
void load_config();

static esp_err_t event_handler(void *ctx, system_event_t *event) {
	switch (event->event_id) {
	case SYSTEM_EVENT_STA_START:
		ESP_LOGI(TAG, "SYSTEM_EVENT_STA_START");

//		ESP_ERROR_CHECK(mqtt_setup());
//        xTaskCreate(smartconfig_example_task, "smartconfig_task", 4096, NULL, 3, NULL);

		break;
	case SYSTEM_EVENT_STA_GOT_IP:
		xEventGroupSetBits(wifi_event_group, CONNECTED_BIT);
		ESP_LOGI(TAG, "SYSTEM_EVENT_STA_GOT_IP");
		ESP_ERROR_CHECK(mqtt_setup());
		break;
	case SYSTEM_EVENT_STA_DISCONNECTED:
		ESP_LOGI(TAG, "SYSTEM_EVENT_STA_DISCONNECTED");
		esp_wifi_connect();
		xEventGroupClearBits(wifi_event_group, CONNECTED_BIT);
		break;

	case SYSTEM_EVENT_STA_CONNECTED:
		ESP_LOGI(TAG, "SYSTEM_EVENT_STA_CONNECTED");
		break;
	default:
		break;
	}
	return ESP_OK;
}

static void initialise_wifi(void) {
	tcpip_adapter_init();
	wifi_event_group = xEventGroupCreate();
	ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL));

	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT()
	;

	if (ESP_OK == esp_wifi_init(&cfg)) {
		if (ESP_OK == esp_wifi_set_mode(WIFI_MODE_STA)) {
			if (ESP_OK == esp_wifi_start()) {
				ESP_ERROR_CHECK(esp_wifi_connect());
				ESP_LOGI(TAG, "ESP_WIFI_CONNECTED");

			} else {
				xTaskCreate(smartconfig_example_task,
						"smartconfig_example_task", 4096, NULL, 3, NULL);
			}
		}
	}

}

static void sc_callback(smartconfig_status_t status, void *pdata) {
	switch (status) {
	case SC_STATUS_WAIT:
		ESP_LOGI(TAG, "SC_STATUS_WAIT");
		break;
	case SC_STATUS_FIND_CHANNEL:
		ESP_LOGI(TAG, "SC_STATUS_FINDING_CHANNEL");
		break;
	case SC_STATUS_GETTING_SSID_PSWD:
		ESP_LOGI(TAG, "SC_STATUS_GETTING_SSID_PSWD");
		break;
	case SC_STATUS_LINK:
		ESP_LOGI(TAG, "SC_STATUS_LINK");
		wifi_config_t *wifi_config = pdata;
		ESP_LOGI(TAG, "SSID:%s", wifi_config->sta.ssid);
		ESP_LOGI(TAG, "PASSWORD:%s", wifi_config->sta.password);

		ESP_ERROR_CHECK(esp_wifi_disconnect());
		ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, wifi_config));
		ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_FLASH));
		ESP_ERROR_CHECK(esp_wifi_connect());
		break;
	case SC_STATUS_LINK_OVER:
		ESP_LOGI(TAG, "SC_STATUS_LINK_OVER");
		if (pdata != NULL) {
			uint8_t phone_ip[4] = { 0 };
			memcpy(phone_ip, (uint8_t*) pdata, 4);
			ESP_LOGI(TAG, "Phone ip: %d.%d.%d.%d\n", phone_ip[0], phone_ip[1],
					phone_ip[2], phone_ip[3]);
		}
		xEventGroupSetBits(wifi_event_group, ESPTOUCH_DONE_BIT);
		break;
	default:
		break;
	}
}




void smartconfig_example_task(void *parm) {
	EventBits_t uxBits;
	TaskHandle_t xTask2Handle = NULL;
	xTaskCreate(mcp24017_task_led_alert_for_select_wifi,
			"mcp24017_task_led_alert_for_select_wifi", 1024, NULL, 3, &xTask2Handle);

	esp_wifi_disconnect();
	ESP_ERROR_CHECK(esp_smartconfig_set_type(SC_TYPE_ESPTOUCH));
	ESP_ERROR_CHECK(esp_smartconfig_start(sc_callback));
	while (1) {
		uxBits = xEventGroupWaitBits(wifi_event_group,
				CONNECTED_BIT | ESPTOUCH_DONE_BIT, true, false, portMAX_DELAY);
		if (uxBits & CONNECTED_BIT) {
			ESP_LOGI(TAG, "WiFi Connected to ap");
		}
		if (uxBits & ESPTOUCH_DONE_BIT) {
			ESP_LOGI(TAG, "smartconfig over");
			esp_smartconfig_stop();
//            config.device_name = "node1";
//            config.mqtt_port = 1883;
//            config.mqtt_url = "192.168.1.107";
			vTaskDelete(xTask2Handle);
			vTaskDelete(NULL);
		}
	}

}

void app_main() {
//    ESP_ERROR_CHECK( nvs_flash_init() );
//    loadConfig();

	esp_err_t err = nvs_flash_init();
	if (err == ESP_ERR_NVS_NO_FREE_PAGES) {
		// NVS partition was truncated and needs to be erased
		// Retry nvs_flash_init
		ESP_ERROR_CHECK(nvs_flash_erase());
		err = nvs_flash_init();
	}
	ESP_ERROR_CHECK(err);
	load_config();

//	    char ptrTaskList[250];
//	    vTaskList(ptrTaskList);
//	    printf("tasks: => %s",ptrTaskList);
	mcp24017_setup();

	sensor_setup();

	initialise_wifi();



}

