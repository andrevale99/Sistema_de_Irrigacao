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

#include "i2c_lib.h"

#define DS3231_ADDRESS 0xD0

void ler_DS3231(uint8_t _addr);
uint8_t get_seconds();
uint8_t get_minutes();
uint8_t get_hours();

ISR(TWI_vect);


#endif