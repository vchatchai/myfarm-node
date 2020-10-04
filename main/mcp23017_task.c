#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "mcp23017_task.h"

#include "cJSON.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_err.h"
#include "mqtt_task.h"
#include "driver/i2c.h"

static const char *TAG = "MCP23017_TASK";

/**
 * TEST CODE BRIEF
 *
 * This example will show you how to use I2C module by running two tasks on i2c bus:
 *
 * - read external i2c sensor, here we use a MPU6050 sensor for instance.
 * - Use one I2C port(master mode) to read or write the other I2C port(slave mode) on one ESP8266 chip.
 *
 * Pin assignment:
 *
 * - master:
 *    GPIO14 is assigned as the data signal of i2c master port
 *    GPIO2 is assigned as the clock signal of i2c master port
 *
 * Connection:
 *
 * - connect sda/scl of sensor with GPIO14/GPIO2
 * - no need to add external pull-up resistors, driver will enable internal pull-up resistors.
 *
 * Test items:
 *
 * - read the sensor data, if connected.
 */

#define I2C_EXAMPLE_MASTER_SCL_IO           05                /*!< gpio number for I2C master clock */
#define I2C_EXAMPLE_MASTER_SDA_IO           04              /*!< gpio number for I2C master data  */
#define I2C_EXAMPLE_MASTER_NUM              I2C_NUM_0        /*!< I2C port number for master dev */
#define I2C_EXAMPLE_MASTER_TX_BUF_DISABLE   0                /*!< I2C master do not need buffer */
#define I2C_EXAMPLE_MASTER_RX_BUF_DISABLE   0                /*!< I2C master do not need buffer */

#define MCP23017_ADDR                 0x20             /*!< slave address for MPU6050 sensor */
#define WRITE_BIT                           I2C_MASTER_WRITE /*!< I2C master write */
#define READ_BIT                            I2C_MASTER_READ  /*!< I2C master read */
#define ACK_CHECK_EN                        0x1              /*!< I2C master will check ack from slave*/
#define ACK_CHECK_DIS                       0x0              /*!< I2C master will not check ack from slave */
#define ACK_VAL                             0x0              /*!< I2C ack value */
#define NACK_VAL                            0x1              /*!< I2C nack value */
#define LAST_NACK_VAL                       0x2              /*!< I2C last_nack value */

void smartconfig_example_task(void *parm);

/**
 * @brief i2c master initialization
 */
static esp_err_t i2c_master_init() {
	int i2c_master_port = I2C_EXAMPLE_MASTER_NUM;
	i2c_config_t conf;
	conf.mode = I2C_MODE_MASTER;
	conf.sda_io_num = I2C_EXAMPLE_MASTER_SDA_IO;
	conf.sda_pullup_en = 1;
	conf.scl_io_num = I2C_EXAMPLE_MASTER_SCL_IO;
	conf.scl_pullup_en = 1;
	conf.clk_stretch_tick = 100000; // 300 ticks, Clock stretch is about 210us, you can make changes according to the actual situation.
	ESP_ERROR_CHECK(i2c_driver_install(i2c_master_port, conf.mode));
	ESP_ERROR_CHECK(i2c_param_config(i2c_master_port, &conf));
	return ESP_OK;
}

static esp_err_t i2c_master_mcp23017_init(i2c_port_t i2c_num) {
//	uint8_t cmd_data;
	vTaskDelay(100 / portTICK_RATE_MS);
	i2c_master_init();
//    cmd_data = 0x00;    // reset mpu6050
//    ESP_ERROR_CHECK(i2c_master_mcp23017_write(i2c_num, MCP23017_REG_IODIRA, &cmd_data, 1));

	reg_t port_io_a;
	port_io_a.port0 = 1;
	port_io_a.port1 = 1;
	port_io_a.port2 = 1;
	port_io_a.port3 = 1;
	port_io_a.port4 = 1;
	port_io_a.port5 = 1;
	port_io_a.port6 = 1;
	port_io_a.port7 = 1;

	ESP_ERROR_CHECK(write_registor_mcp23017(MCP23017_REG_IODIRA, port_io_a));

	reg_t port_io_b;
	port_io_b.port0 = 0; //LED status
	port_io_b.port1 = 1;
	port_io_b.port2 = 1; //button switch
	port_io_b.port3 = 0; //LED button switch
	port_io_b.port4 = 1;
	port_io_b.port5 = 1;
	port_io_b.port6 = 1;
	port_io_b.port7 = 1;

	ESP_ERROR_CHECK(write_registor_mcp23017(MCP23017_REG_IODIRB, port_io_b));

//
//    cmd_data = 0xff;    // Set the SMPRT_DIV
//    ESP_ERROR_CHECK(i2c_master_mcp23017_write(i2c_num, MCP23017_REG_IODIRB, &cmd_data, 1));
//    cmd_data = 0xff;    // Set the Low Pass Filter
//    ESP_ERROR_CHECK(i2c_master_mcp23017_write(i2c_num, MCP23017_REG_GPPUA, &cmd_data, 1));
//    cmd_data = 0xff;    // Set the GYRO range
//    ESP_ERROR_CHECK(i2c_master_mcp23017_write(i2c_num, MCP23017_REG_GPPUB, &cmd_data, 1));

	reg_t pullup_a;
	pullup_a.port0 = 1;
	pullup_a.port1 = 1;
	pullup_a.port2 = 1;
	pullup_a.port3 = 1;
	pullup_a.port4 = 1;
	pullup_a.port5 = 1;
	pullup_a.port6 = 1;
	pullup_a.port7 = 1;

	ESP_ERROR_CHECK(write_registor_mcp23017(MCP23017_REG_GPPUA, pullup_a));

	reg_t pullup_b;
	pullup_b.port0 = 1;
	pullup_b.port1 = 1;
	pullup_b.port2 = 1;
	pullup_b.port3 = 1;
	pullup_b.port4 = 1;
	pullup_b.port5 = 1;
	pullup_b.port6 = 1;
	pullup_b.port7 = 1;

	ESP_ERROR_CHECK(write_registor_mcp23017(MCP23017_REG_GPPUB, pullup_b));

//    cmd_data = 0x01;    // Set the ACCEL range
//    ESP_ERROR_CHECK(i2c_example_master_mpu6050_write(i2c_num, ACCEL_CONFIG, &cmd_data, 1));
	return ESP_OK;
}
void mcp24017_task_led_alert_for_select_wifi(void *arg) {
	int value = 0;
		printf("mcp24017_task_led_alert_for_select_wifi");
	while (1) {
		uint8_t retb = 0;
		i2c_master_mcp23017_read(I2C_EXAMPLE_MASTER_NUM, MCP23017_REG_GPIOB,
				&retb, 1);

		reg_t io_port_b;
		convert_int8_to_reg(retb, &read_io_port_b);
		io_port_b.port0 = value;
		io_port_b.port3 = value;

		value = !value;
		write_registor_mcp23017(MCP23017_REG_GPIOB, io_port_b);
		vTaskDelay(1000 / portTICK_RATE_MS);

	}

}
void mcp24017_task_led_alert_for_mqtt_connect(void *arg) {
	int value = 0;
		printf("mcp24017_task_led_alert_for_mqtt_connect");
	while (1) {
		uint8_t retb = 0;
		i2c_master_mcp23017_read(I2C_EXAMPLE_MASTER_NUM, MCP23017_REG_GPIOB,
				&retb, 1);

		reg_t io_port_b;
		convert_int8_to_reg(retb, &read_io_port_b);
		io_port_b.port0 = value;
		io_port_b.port3 = value;

		value = !value;
		write_registor_mcp23017(MCP23017_REG_GPIOB, io_port_b);
		vTaskDelay(2000 / portTICK_RATE_MS);

	}

}
void mcp24017_task_led_alert(void *arg) {
	int value = 0;
	printf("mcp24017_task_led_alert");
	while (1) {
		uint8_t retb = 0;
		i2c_master_mcp23017_read(I2C_EXAMPLE_MASTER_NUM, MCP23017_REG_GPIOB,
				&retb, 1);

		reg_t io_port_b;
		convert_int8_to_reg(retb, &read_io_port_b);
		io_port_b.port0 = value;
		io_port_b.port3 = value;

		value = !value;
		write_registor_mcp23017(MCP23017_REG_GPIOB, io_port_b);
		vTaskDelay(3000 / portTICK_RATE_MS);

	}

}
void switch_status_task(void *arg) { 


	int status = 0;
	while(1) {

		read_io_port_b.port3 = status;
		write_registor_mcp23017(MCP23017_REG_GPIOB, read_io_port_b);

	}

}

void mcp24017_task_read_io(void *arg) {

	while (1) {

		uint8_t retb = 0;
		uint8_t reta = 0;
		i2c_master_mcp23017_read(I2C_EXAMPLE_MASTER_NUM, MCP23017_REG_GPIOB,&retb, 1);
		i2c_master_mcp23017_read(I2C_EXAMPLE_MASTER_NUM, MCP23017_REG_GPIOA,&reta, 1);

		convert_int8_to_reg(reta, &read_io_port_a);
		convert_int8_to_reg(retb, &read_io_port_b);
		vTaskDelay(100 / portTICK_RATE_MS);
	}

	i2c_driver_delete(I2C_EXAMPLE_MASTER_NUM);
}

void mcp24017_task(void *arg) {

	int smart_config = 0;

	int previousStatus = read_io_port_b.port2;
	char topic[] = "/myfarm/node/";
	strcat(topic,str_chipid);
	TaskHandle_t xTask2Handle = NULL;
	while (1) {

//		printBinary(read_io_port_a);
//		printBinary(read_io_port_b);

//		printf("\nport A: %d%d%d%d%d%d%d%d", read_io_port_a.port7, read_io_port_a.port6, read_io_port_a.port5, read_io_port_a.port4, read_io_port_a.port3, read_io_port_a.port2, read_io_port_a.port1, read_io_port_a.port0);
//		printf("\nport B: %d%d%d%d%d%d%d%d", read_io_port_b.port7, read_io_port_b.port6, read_io_port_b.port5, read_io_port_b.port4, read_io_port_b.port3, read_io_port_b.port2, read_io_port_b.port1, read_io_port_b.port0);

		/*
		if (!read_io_port_b.port2) {
			smart_config++;
			if(smart_config == 1) {

			}
		} else {
			smart_config = 0;
		}


		if (smart_config == 5) {
			xTaskCreate(smartconfig_example_task, "smartconfig_example_task", 4096, NULL, 3, NULL);
		}
		vTaskDelay(1000 / portTICK_RATE_MS);
		*/

		int status = read_io_port_b.port2;

		if(status == 1) {
			if(status == previousStatus) {
				smart_config++;
				if(smart_config == 5) {
					TickType_t xTimeNow;
					/* Obtain the current tick count */
					xTimeNow = xTaskGetTickCount();

					cJSON *root;
					root = cJSON_CreateObject();
					char str[80];


					cJSON_AddStringToObject(root, "NodeID", str_chipid);
					cJSON_AddStringToObject(root, "Name", "Motor2");
					cJSON_AddNumberToObject(root, "SWITCH", 1);
					char *out = cJSON_Print(root);
					printf(out);

//					int msg_id = esp_mqtt_client_publish(client, topic, out, 0, 1, 0);
//					ESP_LOGI(TAG, "msg id: %d mcp24017_task %d : %s", msg_id, xTimeNow,out);
//
//					if(msg_id < 0) {
//						if(xTask2Handle == NULL) {
//							xTask2Handle = xTaskCreate(mcp24017_task_led_alert_for_mqtt_connect,
//								"mcp24017_task_led_alert_for_mqtt_connect", 1024, NULL, 3, &xTask2Handle);
//						}
//					}else {
//						if(xTask2Handle != NULL) {
//							vTaskDelete(xTask2Handle);
//							xTask2Handle = NULL;
//
//						}
//					}




					cJSON_Delete(root);
					free(out);

				}
			}
		}else {
			smart_config = 0;
		}

//		if (smart_config == 50) {
//			xTaskCreate(smartconfig_example_task, "smartconfig_example_task", 4096, NULL, 3, NULL);
//		}
		previousStatus = status;
		vTaskDelay(100 / portTICK_RATE_MS);
	}

}

void mcp24017_setup() {

	i2c_master_mcp23017_init(I2C_EXAMPLE_MASTER_NUM);

	xTaskCreate(mcp24017_task_read_io, "mcp24017_task_read_io", 2048, NULL, 10,	NULL);
	xTaskCreate(mcp24017_task, "mcp24017_task", 2048, NULL, 10, NULL);
}

