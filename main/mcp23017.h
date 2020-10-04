
#include "driver/i2c.h"


#ifndef __MCP23017_H__
#define __MCP23017_H__
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



typedef struct {
	unsigned int port0 :1;//LED
	unsigned int port1 :1;
	unsigned int port3 :1;//LED
	unsigned int port2 :1;//switch
	unsigned int port4 :1;
	unsigned int port5 :1;
	unsigned int port6 :1;
	unsigned int port7 :1;

} reg_t;

typedef enum {
	MCP23017_REG_IODIRA = 0,
	MCP23017_REG_IODIRB,
	MCP23017_REG_IPOLA,
	MCP23017_REG_IPOLB,
	MCP23017_REG_GPINTENA,
	MCP23017_REG_GPINTENB,
	MCP23017_REG_DEFVALA,
	MCP23017_REG_DEFVALB,
	MCP23017_REG_INTCONA,
	MCP23017_REG_INTCONB,
	MCP23017_REG_IOCONA,
	MCP23017_REG_IOCONB,
	MCP23017_REG_GPPUA,
	MCP23017_REG_GPPUB,
	MCP23017_REG_INTFA,
	MCP23017_REG_INTFB,
	MCP23017_REG_INTCAPA,
	MCP23017_REG_INTCAPB,
	MCP23017_REG_GPIOA,
	MCP23017_REG_GPIOB,
	MCP23017_REG_OLATA,
	MCP23017_REG_OLATB,

} mcp23017_reg;


//
//typedef enum {
//    I2C_NUM_0 = 0,  /*!< I2C port 0 */
//    I2C_NUM_MAX
//} i2c_port_t;
//esp_err_t i2c_master_mcp23017_init(i2c_port_t i2c_num);

void convert_int8_to_reg(uint8_t ret, reg_t *result) ;
esp_err_t write_registor_mcp23017_bit(mcp23017_reg reg, uint8_t pin,	uint8_t value);

esp_err_t write_registor_mcp23017(mcp23017_reg reg, reg_t value);
esp_err_t i2c_master_mcp23017_read(i2c_port_t i2c_num, uint8_t reg_address, uint8_t *data, size_t data_len);
esp_err_t i2c_master_mcp23017_read_byte(i2c_port_t i2c_num, uint8_t reg_address, uint8_t *data);

void printBinary(reg_t value);

#endif //__MCP23017_H__
