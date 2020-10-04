/* I2C example

 This example code is in the Public Domain (or CC0 licensed, at your option.)

 Unless required by applicable law or agreed to in writing, this
 software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
 CONDITIONS OF ANY KIND, either express or implied.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "esp_log.h"
#include "esp_system.h"
#include "esp_err.h"

#include "mcp23017.h"
#include "driver/i2c.h"

static const char *TAG = "main";



uint8_t reg_to_uint8(reg_t value) {
	uint8_t data = value.port7;
	data = data << 1;
	data = data + value.port6;
	data = data << 1;
	data = data + value.port5;
	data = data << 1;
	data = data + value.port4;
	data = data << 1;
	data = data + value.port3;
	data = data << 1;
	data = data + value.port2;
	data = data << 1;
	data = data + value.port1;
	data = data << 1;
	data = data + value.port0;
	return data;
}




/**
 * @brief test code to write mpu6050
 *
 * 1. send data
 * ___________________________________________________________________________________________________
 * | start | slave_addr + wr_bit + ack | write reg_address + ack | write data_len byte + ack  | stop |
 * --------|---------------------------|-------------------------|----------------------------|------|
 *
 * @param i2c_num I2C port number
 * @param reg_address slave reg address
 * @param data data to send
 * @param data_len data length
 *
 * @return
 *     - ESP_OK Success
 *     - ESP_ERR_INVALID_ARG Parameter error
 *     - ESP_FAIL Sending command error, slave doesn't ACK the transfer.
 *     - ESP_ERR_INVALID_STATE I2C driver not installed or not in master mode.
 *     - ESP_ERR_TIMEOUT Operation timeout because the bus is busy.
 */
esp_err_t i2c_master_mcp23017_write(i2c_port_t i2c_num,	uint8_t reg_address, uint8_t *data, size_t data_len) {
	int ret;
	i2c_cmd_handle_t cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, MCP23017_ADDR << 1 | WRITE_BIT,
			ACK_CHECK_EN);
	i2c_master_write_byte(cmd, reg_address, ACK_CHECK_EN);
	i2c_master_write(cmd, data, data_len, ACK_CHECK_EN);
	i2c_master_stop(cmd);
	ret = i2c_master_cmd_begin(i2c_num, cmd, 1000 / portTICK_RATE_MS);
	i2c_cmd_link_delete(cmd);

//    esp_err_t ret;
//    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
//    i2c_master_start(cmd);
//    i2c_master_write_byte(cmd, (device->dev_addr << 1) | WRITE_BIT, ACK_CHECK_EN);
//    i2c_master_write_byte(cmd, reg_addr, ACK_CHECK_EN);
//    i2c_master_write_byte(cmd, data, ACK_CHECK_EN);
//    ret = iot_i2c_bus_cmd_begin(device->bus, cmd, 1000 / portTICK_RATE_MS);
//    i2c_cmd_link_delete(cmd);

	return ret;
}

/**
 * @brief test code to read mpu6050
 *
 * 1. send reg address
 * ______________________________________________________________________
 * | start | slave_addr + wr_bit + ack | write reg_address + ack | stop |
 * --------|---------------------------|-------------------------|------|
 *
 * 2. read data
 * ___________________________________________________________________________________
 * | start | slave_addr + wr_bit + ack | read data_len byte + ack(last nack)  | stop |
 * --------|---------------------------|--------------------------------------|------|
 *
 * @param i2c_num I2C port number
 * @param reg_address slave reg address
 * @param data data to read
 * @param data_len data length
 *
 * @return
 *     - ESP_OK Success
 *     - ESP_ERR_INVALID_ARG Parameter error
 *     - ESP_FAIL Sending command error, slave doesn't ACK the transfer.
 *     - ESP_ERR_INVALID_STATE I2C driver not installed or not in master mode.
 *     - ESP_ERR_TIMEOUT Operation timeout because the bus is busy.
 */
esp_err_t i2c_master_mcp23017_read(i2c_port_t i2c_num, uint8_t reg_address, uint8_t *data, size_t data_len) {
	int ret;
	i2c_cmd_handle_t cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, MCP23017_ADDR << 1 | WRITE_BIT,
			ACK_CHECK_EN);
	i2c_master_write_byte(cmd, reg_address, ACK_CHECK_EN);
	i2c_master_stop(cmd);
	ret = i2c_master_cmd_begin(i2c_num, cmd, 1000 / portTICK_RATE_MS);
	i2c_cmd_link_delete(cmd);

	if (ret != ESP_OK) {
		return ret;
	}

	cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, MCP23017_ADDR << 1 | READ_BIT,
			ACK_CHECK_EN);
	i2c_master_read(cmd, data, data_len, LAST_NACK_VAL);
	i2c_master_stop(cmd);
	ret = i2c_master_cmd_begin(i2c_num, cmd, 1000 / portTICK_RATE_MS);
	i2c_cmd_link_delete(cmd);

	return ret;
}

esp_err_t i2c_master_mcp23017_read_byte(i2c_port_t i2c_num, uint8_t reg_address, uint8_t *data) {
	return i2c_master_mcp23017_read(i2c_num, reg_address, data, 1);
}


esp_err_t write_registor_mcp23017(mcp23017_reg reg, reg_t value) {

	uint8_t data = reg_to_uint8(value);

	return i2c_master_mcp23017_write(I2C_EXAMPLE_MASTER_NUM, reg, &data, 1);
}

esp_err_t write_registor_mcp23017_bit(mcp23017_reg reg, uint8_t pin, uint8_t value) {

	uint8_t data = 0; // value.port7;

	i2c_master_mcp23017_read_byte(I2C_EXAMPLE_MASTER_NUM,  reg, &data);

	reg_t reg_data;
	switch (pin) {
		case 0:
			reg_data.port0 = value;
			break;
		case 1:
			reg_data.port1 = value;
			break;
		case 2:
			reg_data.port2 = value;
			break;
		case 3:
			reg_data.port3 = value;
			break;
		case 4:
			reg_data.port4 = value;
			break;
		case 5:
			reg_data.port5 = value;
			break;
		case 6:
			reg_data.port6 = value;
			break;
		case 7:
			reg_data.port7 = value;
			break;
		default:
			break;
	}
	convert_int8_to_reg(data,&reg_data);


	return i2c_master_mcp23017_write(I2C_EXAMPLE_MASTER_NUM, reg, &data, 1);
}

void printBinary(reg_t value) {
	printf("%d", value.port7);
	printf("%d", value.port6);
	printf("%d", value.port5);
	printf("%d", value.port4);
	printf("%d", value.port3);
	printf("%d", value.port2);
	printf("%d", value.port1);
	printf("%d", value.port0);

}


void convert_int8_to_reg(uint8_t ret,reg_t* result) {


	result->port0 = ret;
	ret = ret >> 1;
	result->port1 = ret;
	ret = ret >> 1;
	result->port2 = ret;
	ret = ret >> 1;
	result->port3 = ret;
	ret = ret >> 1;
	result->port4 = ret;
	ret = ret >> 1;
	result->port5 = ret;
	ret = ret >> 1;
	result->port6 = ret;
	ret = ret >> 1;
	result->port7 = ret;
	ret = ret >> 1;
}



static void i2c_task_example(void *arg) {

//	i2c_master_mcp23017_init(I2C_EXAMPLE_MASTER_NUM);


	while (1) {
//		uint8_t reta = 0;
//		i2c_master_mcp23017_read(I2C_EXAMPLE_MASTER_NUM, MCP23017_REG_GPIOA,
//				&reta, 1);
//		ESP_LOGI(TAG, "MCP23017_REG_GPIOA: %x  ", reta);
//		printBinary(convert_int8_to_reg(reta));




		uint8_t retb = 0;
		i2c_master_mcp23017_read(I2C_EXAMPLE_MASTER_NUM, MCP23017_REG_GPIOB,
				&retb, 1);
		ESP_LOGI(TAG, "MCP23017_REG_GPIOB: %x\n", retb);
		reg_t reg ;
		convert_int8_to_reg(retb,&reg);
		printBinary(reg);

		if(reg.port2) {
			reg.port0 = 1;
			reg.port3 = 1;
			ESP_ERROR_CHECK(write_registor_mcp23017(MCP23017_REG_GPIOB, reg));
		}else {
			reg.port0 = 0;
			reg.port3 = 0;
			ESP_ERROR_CHECK(write_registor_mcp23017(MCP23017_REG_GPIOB, reg));
		}






//    	i2c_master_mcp23017_read_byte(I2C_EXAMPLE_MASTER_NUM, MCP23017_REG_IODIRA , &data);
//    		printf("MCP23017_REG_IODIRA %x %x\n",MCP23017_REG_IODIRA, data);
//    		i2c_master_mcp23017_read_byte(I2C_EXAMPLE_MASTER_NUM, MCP23017_REG_IODIRB, &data);
//    		printf("MCP23017_REG_IODIRB %x %x\n",MCP23017_REG_IODIRB, data);
//    		i2c_master_mcp23017_read_byte(I2C_EXAMPLE_MASTER_NUM, MCP23017_REG_IPOLA, &data);
//    		printf("MCP23017_REG_IPOLA %x %x\n",MCP23017_REG_IPOLA, data);
//    		i2c_master_mcp23017_read_byte(I2C_EXAMPLE_MASTER_NUM, MCP23017_REG_IPOLB, &data);
//    		printf("MCP23017_REG_IPOLB %x %x\n",MCP23017_REG_IPOLB, data);
//    		i2c_master_mcp23017_read_byte(I2C_EXAMPLE_MASTER_NUM, MCP23017_REG_GPINTENA, &data);
//    		printf("MCP23017_REG_GPINTENA %x %x\n",MCP23017_REG_GPINTENA, data);
//    		i2c_master_mcp23017_read_byte(I2C_EXAMPLE_MASTER_NUM, MCP23017_REG_GPINTENB, &data);
//    		printf("MCP23017_REG_GPINTENB %x %x\n",MCP23017_REG_GPINTENB ,data);
//    		i2c_master_mcp23017_read_byte(I2C_EXAMPLE_MASTER_NUM, MCP23017_REG_DEFVALA, &data);
//    		printf("MCP23017_REG_DEFVALA %x %x\n",MCP23017_REG_DEFVALA,data);
//    		i2c_master_mcp23017_read_byte(I2C_EXAMPLE_MASTER_NUM, MCP23017_REG_DEFVALB, &data);
//    		printf("MCP23017_REG_DEFVALB %x %x\n",MCP23017_REG_DEFVALB,data);
//    		i2c_master_mcp23017_read_byte(I2C_EXAMPLE_MASTER_NUM, MCP23017_REG_INTCONA, &data);
//    		printf("MCP23017_REG_INTCONA %x %x\n",MCP23017_REG_INTCONA,data);
//    		i2c_master_mcp23017_read_byte(I2C_EXAMPLE_MASTER_NUM, MCP23017_REG_INTCONB, &data);
//    		printf("MCP23017_REG_INTCONB %x %x\n",MCP23017_REG_INTCONB,data);
//    		i2c_master_mcp23017_read_byte(I2C_EXAMPLE_MASTER_NUM, MCP23017_REG_IOCONA, &data);
//    		printf("MCP23017_REG_IOCONA %x %x\n",MCP23017_REG_IOCONA,data);
//    		i2c_master_mcp23017_read_byte(I2C_EXAMPLE_MASTER_NUM, MCP23017_REG_IOCONB, &data);
//    		printf("MCP23017_REG_IOCONB %x %x\n",MCP23017_REG_IOCONB,data);
//    		i2c_master_mcp23017_read_byte(I2C_EXAMPLE_MASTER_NUM, MCP23017_REG_GPPUA, &data);
//    		printf("MCP23017_REG_GPPUA %x %x\n",MCP23017_REG_GPPUA,data);
//    		i2c_master_mcp23017_read_byte(I2C_EXAMPLE_MASTER_NUM, MCP23017_REG_GPPUB, &data);
//    		printf("MCP23017_REG_GPPUB %x %x\n",MCP23017_REG_GPPUB,data);
//    		i2c_master_mcp23017_read_byte(I2C_EXAMPLE_MASTER_NUM, MCP23017_REG_INTFA, &data);
//    		printf("MCP23017_REG_INTFA %x %x\n",MCP23017_REG_INTFA,data);
//    		i2c_master_mcp23017_read_byte(I2C_EXAMPLE_MASTER_NUM, MCP23017_REG_INTFB, &data);
//    		printf("MCP23017_REG_INTFB %x %x\n",MCP23017_REG_INTFB,data);
//    		i2c_master_mcp23017_read_byte(I2C_EXAMPLE_MASTER_NUM, MCP23017_REG_INTCAPA, &data);
//    		printf("MCP23017_REG_INTCAPA %x %x\n",MCP23017_REG_INTCAPA,data);
//    		i2c_master_mcp23017_read_byte(I2C_EXAMPLE_MASTER_NUM, MCP23017_REG_INTCAPB, &data);
//    		printf("MCP23017_REG_INTCAPB %x %x\n",MCP23017_REG_INTCAPB,data);
//    		i2c_master_mcp23017_read_byte(I2C_EXAMPLE_MASTER_NUM, MCP23017_REG_GPIOA, &data);
//    		printf("MCP23017_REG_GPIOA %x %x\n",MCP23017_REG_GPIOA,data);
//    		i2c_master_mcp23017_read_byte(I2C_EXAMPLE_MASTER_NUM, MCP23017_REG_GPIOB, &data);
//    		printf("MCP23017_REG_GPIOB %x %x\n",MCP23017_REG_GPIOB,data);
//    		i2c_master_mcp23017_read_byte(I2C_EXAMPLE_MASTER_NUM, MCP23017_REG_OLATA, &data);
//    		printf("MCP23017_REG_OLATA %x %x\n",MCP23017_REG_OLATA,data);
//    		i2c_master_mcp23017_read_byte(I2C_EXAMPLE_MASTER_NUM, MCP23017_REG_OLATB, &data);
//    		printf("MCP23017_REG_OLATB %x %x\n\n",MCP23017_REG_OLATB,data);

//		reg_t p = read_registor_mcp23017(MCP23017_REG_GPIOA);
//
//		printf("GPIOA.port0 %d \n",p.port0);
//		printf("GPIOA.port1 %d \n",p.port1);
//		printf("GPIOA.port2 %d \n",p.port2);
//		printf("GPIOA.port3 %d \n",p.port3);
//		printf("GPIOA.port4 %d \n",p.port4);
//		printf("GPIOA.port5 %d \n",p.port5);
//		printf("GPIOA.port6 %d \n",p.port6);
//		printf("GPIOA.port7 %d \n",p.port7);
//		p = read_registor_mcp23017(MCP23017_REG_GPIOB);
//		printf("GPIOB.port0 %d \n",p.port0);
//		printf("GPIOB.port1 %d \n",p.port1);
//		printf("GPIOB.port2 %d \n",p.port2);
//		printf("GPIOB.port3 %d \n",p.port3);
//		printf("GPIOB.port4 %d \n",p.port4);
//		printf("GPIOB.port5 %d \n",p.port5);
//		printf("GPIOB.port6 %d \n",p.port6);
//		printf("GPIOB.port7 %d \n",p.port7);

//        reg_t result;
//
//        uint8_t ret = icount;
//    	result.port0 = ret;
//    	ret = ret >> 1;
//    	result.port1 = ret;
//    	ret = ret >> 1;
//    	result.port2 = ret;
//    	ret = ret >> 1;
//    	result.port3 = ret;
//    	ret = ret >> 1;
//    	result.port4 = ret;
//    	ret = ret >> 1;
//    	result.port5 = ret;
//    	ret = ret >> 1;
//    	result.port6 = ret;
//    	ret = ret >> 1;
//    	result.port7 = ret;
//    	ret = ret >> 1;
//
//        write_registor_mcp23017(MCP23017_REG_GPIOA,result);
//        icount++;

//
//        if (0x68 != who_am_i) {
//            error_count++;
//        }
//
//        memset(sensor_data, 0, 14);
//        ret = i2c_example_master_mpu6050_read(I2C_EXAMPLE_MASTER_NUM, ACCEL_XOUT_H, sensor_data, 14);
//
//        if (ret == ESP_OK) {
//            ESP_LOGI(TAG, "*******************\n");
//            ESP_LOGI(TAG, "WHO_AM_I: 0x%02x\n", who_am_i);
//            Temp = 36.53 + ((double)(int16_t)((sensor_data[6] << 8) | sensor_data[7]) / 340);
//            ESP_LOGI(TAG, "TEMP: %d.%d\n", (uint16_t)Temp, (uint16_t)(Temp * 100) % 100);
//
//            for (i = 0; i < 7; i++) {
//                ESP_LOGI(TAG, "sensor_data[%d]: %d\n", i, (int16_t)((sensor_data[i * 2] << 8) | sensor_data[i * 2 + 1]));
//            }
//
//            ESP_LOGI(TAG, "error_count: %d\n", error_count);
//        } else {
//            ESP_LOGE(TAG, "No ack, sensor not connected...skip...\n");
//        }
//
		vTaskDelay(100 / portTICK_RATE_MS);
	}

	i2c_driver_delete(I2C_EXAMPLE_MASTER_NUM);
}

//void app_main(void) {
//	//start i2c task
//	xTaskCreate(i2c_task_example, "i2c_task_example", 2048, NULL, 10, NULL);
//}
