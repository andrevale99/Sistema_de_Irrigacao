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

#define SOLO PC0
#define POT PC1
#define RELE PB0
#define LED_ERRO PB5

#define BAUD 9600
#define MYUBRR F_CPU/16/BAUD-1

//===============================================
//  VARIAVEIS
//===============================================
uint8_t adc_LSB = 0x00;
uint8_t adc_MSB = 0x00;

//===============================================
//  PROTOTIPOS
//===============================================
void setup();
uint16_t adc_conv(uint8_t pino);

void USART_Init(unsigned int ubrr);
void USART_Transmit(unsigned char data);
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
    DDRB |= (1<<RELE) | (1<<PB5);

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

void USART_Init(unsigned int ubrr)
{
    /*Set baud rate */
    UBRR0H = (unsigned char)(ubrr>>8);
    UBRR0L = (unsigned char)ubrr;
    /*Enable receiver and transmitter */
    UCSR0B = (1<<RXEN0)|(1<<TXEN0);
    /* Set frame format: 8data, 2stop bit */
    UCSR0C = (1<<USBS0)|(3<<UCSZ00);
}

void USART_Transmit(unsigned char data)
{
    /* Wait for empty transmit buffer */
    while (!(UCSR0A & (1<<UDRE0))) 
        ;
    /* Put data into buffer, sends the data */
    UDR0 = data;
}
