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

#include <stdint.h>
#include <stdbool.h>

#define I2C_DDR DDRC
#define I2C_PORT PORTC

#define SCL PC5
#define SDA PC4

#define DS3231_ADDRESS 0xD0

void i2c_init();
void i2c_init_pullup();
void i2c_unit();

void i2c_start_bit();
void i2c_stop_bit();

void ler_DS3231();
uint8_t get_seconds();
uint8_t get_minutes();
uint8_t get_hours();

ISR(TWI_vect);


#endif