/**
 * @author Andre Menezes de Freitas Vale
 * @date 03/01/2023
 * 
 * @version 0.0
 * 
 * @brief Programa para realizar a irrigacao automatica
 * de um jardim utlizando a programacao em C e o microcontrolador
 * AVR Atmega328p
 * 
 * @note A parte da programacao do RTC foi baseado no livro
 * "AVR e Arduino: Tecnicas de Projeto" de Charles Borges de Lima
 *  e Marco V. M. Villaca
 * 
*/
#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <util/twi.h> 

#include <stdint.h>

#include "i2c_lib.h"

#define SOLO PC0
#define RELE PB0
#define SCL PC5
#define SDA PC4

//===============================================
//  VARIAVEIS
//===============================================
uint8_t adc_LSB = 0x00;
uint8_t adc_MSB = 0x00;

//===============================================
//  PROTOTIPOS
//===============================================
void setup();

void gpio_setup();

void adc_setup();
uint16_t adc_conv(uint8_t pino);

//===============================================
//  MAIN
//===============================================
int main()
{
    setup();

    for(;;)
    {
  
    }

    return 0;
}

//===============================================
//  FUNCOES
//===============================================
void setup()
{
    gpio_setup();

    adc_setup();

    i2c_init();
}

void gpio_setup()
{
    DDRB |= (1<<RELE) | (1<<PB5);
    DDRC |= (1<<SCL) | (1<<SDA);
}

void adc_setup()
{
    ADCSRA |= (1<<ADEN) | (1<<ADPS2) 
              | (1<<ADPS1) | (1<<ADPS0);
        
    ADMUX |= (1<<REFS0);
}

uint16_t adc_conv(uint8_t pino)
{
    ADMUX |= pino;

    ADCSRA |= (1<<ADSC);

    while(!(ADCSRA &= ~(1<<ADIF)));
    ADCSRA |= (1<<ADIF); 

    adc_LSB = ADCL;
    adc_MSB = ADCH;

    ADCSRA |= (1<<ADSC);

    return (adc_MSB<<8) | adc_LSB;
}

//===============================================
//  INTERRUPCAO
//===============================================
ISR(TWI_vect)
{

}

