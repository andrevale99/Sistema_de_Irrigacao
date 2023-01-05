/**
 * @brief Biblioteca baseada nos códigos de 
*/

#ifndef I2C_LIB
#define I2C_LIB

#include <avr/io.h>
#include <util/twi.h> 

#define I2C_DDR DDRC
#define I2C_PORT PORTC
#define SCL PC5
#define SDA PC4

void i2c_init();
void i2c_init_pullup();
void i2c_unit();


#endif