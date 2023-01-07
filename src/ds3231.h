/**
 * @brief Biblioteca baseada nos códigos de 
*/

#ifndef DS3231_H
#define DS3231_H

#ifndef F_CPU
    #define F_CPU 16000000UL
#endif

#include <avr/io.h>
#include <util/twi.h> 
#include <avr/interrupt.h>

#define I2C_DDR DDRC
#define I2C_PORT PORTC

#define SCL PC5
#define SDA PC4

#define READ 0x01
#define WRITE 0x00

#define DS3231_ADDRESS 0xD0

void i2c_init();
void i2c_init_pullup();
void i2c_unit();

void i2c_start_bit();
void i2c_stop_bit();

ISR(TWI_vect);


#endif