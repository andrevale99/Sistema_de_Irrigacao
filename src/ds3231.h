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

uint8_t ler_DS3231(uint8_t _add);

ISR(TWI_vect);


#endif