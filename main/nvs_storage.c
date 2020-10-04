/* Non-Volatile Storage (NVS) Read and Write a Value - Example

 For other examples please check:
 https://github.com/espressif/esp-idf/tree/master/examples

 This example code is in the Public Domain (or CC0 licensed, at your option.)

 Unless required by applicable law or agreed to in writing, this
 software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
 CONDITIONS OF ANY KIND, either express or implied.
 */
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "nvs.h"

struct Config {
	char *mqtt_url;
	uint16_t mqtt_port;
	char *device_name;
};

struct Config config = { .mqtt_port = 0 };

void load_config(void) {
//    // Initialize NVS
//    esp_err_t err = nvs_flash_init();
//    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
//        // NVS partition was truncated and needs to be erased
//        // Retry nvs_flash_init
//        ESP_ERROR_CHECK(nvs_flash_erase());
//        err = nvs_flash_init();
//    }
//    ESP_ERROR_CHECK( err );

// Open
	printf("\n");
	printf("Opening Non-Volatile Storage (NVS) handle... ");
	nvs_handle my_handle;
	esp_err_t err = nvs_open("storage", NVS_READWRITE, &my_handle);
	if (err != ESP_OK) {
		printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
	} else {

		// Read
		int32_t restart_counter = 0; // value will default to 0, if not set yet in NVS
		size_t mqtt_url;
		err = nvs_get_str(my_handle, "mqtt_url", NULL, &mqtt_url);
		switch (err) {
		case ESP_OK:
			config.mqtt_url = &mqtt_url;
			printf("\nconfig.mqtt_url: %c\n", mqtt_url);
			break;
		case ESP_ERR_NVS_NOT_FOUND:
			err = nvs_set_str(my_handle, "mqtt_url", "");
			err = nvs_commit(my_handle);

			printf((err != ESP_OK) ? "Failed!\n" : "Done\n");

			printf("config.mqtt_url is not initialized yet!\n");
			break;
		default:
			printf("Error (%s) reading!\n", esp_err_to_name(err));
		}

		err = nvs_get_u16(my_handle, "mqtt_port", &config.mqtt_port);
		switch (err) {
		case ESP_OK:

			printf("mqtt_port: %d\n", config.mqtt_port);
			break;
		case ESP_ERR_NVS_NOT_FOUND:
			err = nvs_set_u16(my_handle, "mqtt_port", 0);
			err = nvs_commit(my_handle);

			printf((err != ESP_OK) ? "Failed!\n" : "Done\n");
			printf("config.mqtt_port is not initialized yet!\n");
			break;
		default:
			printf("Error (%s) reading!\n", esp_err_to_name(err));
		}

		size_t device_name;
		err = nvs_get_str(my_handle, "device_name", NULL, &device_name);
		switch (err) {
		case ESP_OK:
			config.device_name = &device_name;

			printf("config.device_name = %c\n", device_name);
			break;
		case ESP_ERR_NVS_NOT_FOUND:
			err = nvs_set_str(my_handle, "device_name", "");
			err = nvs_commit(my_handle);

			printf((err != ESP_OK) ? "Failed!\n" : "Done\n");
			printf("The config.device_name is not initialized yet!\n");
			break;
		default:
			printf("Error (%s) reading!\n", esp_err_to_name(err));
		}

		// Write
//        printf("Updating restart counter in NVS ... ");
//        restart_counter++;
//        err = nvs_set_i32(my_handle, "restart_counter", restart_counter);
//        printf((err != ESP_OK) ? "Failed!\n" : "Done\n");
//
//        // Commit written value.
//        // After setting any values, nvs_commit() must be called to ensure changes are written
//        // to flash storage. Implementations may write to storage at other times,
//        // but this is not guaranteed.
//        printf("Committing updates in NVS ... ");
//        err = nvs_commit(my_handle);
//        printf((err != ESP_OK) ? "Failed!\n" : "Done\n");

		// Close
		nvs_close(my_handle);
	}

	printf("\n");

	fflush(stdout);
//    esp_restart();
}

void update_config(void) {
//    // Initialize NVS
//    esp_err_t err = nvs_flash_init();
//    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
//        // NVS partition was truncated and needs to be erased
//        // Retry nvs_flash_init
//        ESP_ERROR_CHECK(nvs_flash_erase());
//        err = nvs_flash_init();
//    }
//    ESP_ERROR_CHECK( err );

	// Open
	printf("\n");
	printf("Opening Non-Volatile Storage (NVS) handle... ");
	nvs_handle my_handle;
	esp_err_t err = nvs_open("storage", NVS_READWRITE, &my_handle);
	if (err != ESP_OK) {
		printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
	} else {

		// Write
		err = nvs_set_u16(my_handle, "mqtt_port", config.mqtt_port);
		printf((err != ESP_OK) ? "Failed!\n" : "Done\n");

		err = nvs_set_str(my_handle, "device_name", config.device_name);
		printf((err != ESP_OK) ? "Failed!\n" : "Done\n");

		err = nvs_set_str(my_handle, "mqtt_url", config.mqtt_url);
		printf((err != ESP_OK) ? "Failed!\n" : "Done\n");

//
//        // Commit written value.
//        // After setting any values, nvs_commit() must be called to ensure changes are written
//        // to flash storage. Implementations may write to storage at other times,
//        // but this is not guaranteed.
		printf("Committing updates in NVS ... ");
		err = nvs_commit(my_handle);
		printf((err != ESP_OK) ? "Failed!\n" : "Done\n");

		// Close
		nvs_close(my_handle);
	}

	printf("\n");

	fflush(stdout);
//    esp_restart();
}

