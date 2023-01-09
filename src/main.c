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
 *  1.Criar logica do LCD
 *  3.Criar logica do rele para ativas as 6 e 18 horas 
 *  4.Criar logica do ultrassom
 * 
*/
#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <util/twi.h> 

#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#include "i2c_lib.h"
#include "ds3231.h"

#define SOLO PC0
#define RELE PD7

#define BAUD 9600
#define MYUBRR F_CPU/16/BAUD-1

//===============================================
//  VARIAVEIS
//===============================================
volatile uint8_t ovf_secs = 0x00;

volatile uint8_t horas[3] = {0,0,0};
volatile bool refresh_horas = false;
//===============================================
//  PROTOTIPOS
//===============================================
void setup();

void gpio_setup();

void adc_setup();
uint16_t adc_read(uint8_t pino);

void USART_Init(unsigned int ubrr);
void USART_Transmit(unsigned char data);

void timer1_setup();

ISR(TIMER1_COMPA_vect);
//===============================================
//  MAIN
//===============================================
int main()
{
    setup();
    sei();

    for(;;)
    {
        if(refresh_horas)
        {
            TIMSK1 &= ~(1<<OCIE1A);

            horas[0] = ler_DS3231(0x02);
            horas[1] = ler_DS3231(0x01);
            horas[2] = ler_DS3231(0x00);

            refresh_horas = false;
            TIMSK1 ^= (1<<OCIE1A);
        }

        if(adc_read(0x01)>900)
            PORTD |= (1<<RELE);
        else
            PORTD &= ~(1<<RELE);
    }

    cli();

    return 0;
}

//===============================================
//  FUNCOES
//===============================================
/**
 * @brief Inicializacao dos pinos e protocolos
*/
void setup()
{
    gpio_setup();

    adc_setup();

    i2c_init();

    timer1_setup();

    USART_Init(MYUBRR);
}

/**
 * @brief Inicializacao dos pinos
 * 
 * @note Coloca os pinos PB5 e RELE(PD7) como OUTPUTs
*/
void gpio_setup()
{
    DDRB |= (1<<PB5);
    DDRD |= (1<<RELE);
}

/**
 * @brief Inicializacao do ADC
 * 
 * @note Ativa o ADC com o prescale de 16e6/128 e
 * referencia no AVCC/AREF 
*/
void adc_setup()
{
    ADCSRA |= (1<<ADEN) | (1<<ADPS2) 
              | (1<<ADPS1) | (1<<ADPS0);
        
    ADMUX |= (1<<REFS0);
}

/**
 * @brief Inicializa o Timer de 16 bits
*/
void timer1_setup()
{
    TCCR1B |= (1<<WGM12) | (1<<CS12) | (1<<CS10);
    TIMSK1 |= (1<<OCIE1A);

    OCR1A = 15625;
}

/**
 * @brief Funcao para pegar o valor do adc
 * @param pino Pino que deseja ler
 * @return Valor da conversao ADC em um valor de 16 bits
 * 
 * @note Retorna um valor de 16 bits pela facilidade,
 * ja que a resolucao do arduino e de 10 bits
*/
uint16_t adc_read(uint8_t pino)
{
    static uint8_t adc_LSB; 
    static uint8_t adc_MSB;

    ADMUX |= pino;

    ADCSRA |= (1<<ADSC);

    while(!(ADCSRA &= ~(1<<ADIF)));
    ADCSRA |= (1<<ADIF); 

    adc_LSB = ADCL;
    adc_MSB = ADCH;

    ADCSRA |= (1<<ADSC);

    return (adc_MSB<<8) | adc_LSB;
}

/**
 * @brief Inicializacao do USART
 * @param ubrr Valor calculado do baud rate
 * 
 * @note A definicao MYUBRR ja faz o calculo (datasheet)
*/
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

/**
 * @brief Transmissao de um byte pelo USART0
 * @param data caractere
*/
void USART_Transmit(unsigned char data)
{
    /* Wait for empty transmit buffer */
    while (!(UCSR0A & (1<<UDRE0))) 
        ;
    /* Put data into buffer, sends the data */
    UDR0 = data;
}

/**
 * @brief Vetor de interrupcao para o estouro
 * do valor MAX do time1 (16 bits)
 * 
 * @note A cada 5 segundos, ele libera a atualizacao
 * das horas na main trocando a flag "refresh_horas"
*/
ISR(TIMER1_COMPA_vect)
{
    ++ovf_secs;
    
    if(ovf_secs==30)
    {
        refresh_horas = true;
        ovf_secs = 0;
    }
}