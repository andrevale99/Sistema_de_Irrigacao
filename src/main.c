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
 *  1.Criar logica do LCD (opcional)
 *  4.Criar logica do ultrassom
 * 
*/
#ifndef F_CPU
    #define F_CPU 16000000UL
#endif

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <util/twi.h> 

#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#include "i2c_lib.h"
#include "ds3231.h"
#include "LCD.h"

#define SetBit(port, pin) (port |= (1<<pin))
#define ClrBit(port, pin) (port &= ~(1<<pin))
#define ToggleBit(port, pin) (port ^= (1<<pin))
#define TestBit(port, pin) (port & (1<<pin))

#define SOLO PC0
#define RELE PD7

#define BAUD 9600
#define MYUBRR F_CPU/16/BAUD-1

#define SOLO_SECO 500

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

    horas[0] = ler_DS3231(0x02);
    horas[1] = ler_DS3231(0x01);
    horas[2] = ler_DS3231(0x00);

    for(;;)
    {
        if(refresh_horas)
        {
            ClrBit(TIMSK1, OCIE1A);

            horas[0] = ler_DS3231(0x02);
            horas[1] = ler_DS3231(0x01);
            horas[2] = ler_DS3231(0x00);

            refresh_horas = false;

            TCNT1 = 0;
            ToggleBit(TIMSK1, OCIE1A);
        }

        if(horas[0] == 6 || horas[0] == 18)
        {
            if(adc_read(SOLO) <= SOLO_SECO ) //IFs para verificar a umidade do solo
                SetBit(PORTD, RELE);
            else
                ClrBit(PORTD, RELE);
        }

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

    USART_Init(MYUBRR);

    timer1_setup();

    inic_LCD_4bits();
}

/**
 * @brief Inicializacao dos pinos
 * 
 * @note Coloca os pinos PB5 e RELE(PD7) como OUTPUTs
*/
void gpio_setup()
{
    SetBit(DDRB, PB5);

    SetBit(DDRD, RELE);
}

/**
 * @brief Inicializacao do ADC
 * 
 * @note Ativa o ADC com o prescale de 16e6/128 e
 * referencia no AVCC/AREF 
*/
void adc_setup()
{
    SetBit(ADCSRA, ADEN);
    SetBit(ADCSRA, ADPS2);
    SetBit(ADCSRA, ADPS1);
    SetBit(ADCSRA, ADPS0);
        
    SetBit(ADMUX, REFS0);
}

/**
 * @brief Inicializa o Timer de 16 bits
 * 
 * @note O contador tem a frequencia de 16e6/1024
 * e com o modo de CTC com  interrupcao do 
 * comparador do OCR1A como MAX
*/
void timer1_setup()
{
    SetBit(TCCR1B, WGM12);
    SetBit(TCCR1B, CS12);
    SetBit(TCCR1B, CS10);

    SetBit(TIMSK1, OCIE1A);

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

    SetBit(ADCSRA, ADSC);

    while(!(ADCSRA &= ~(1<<ADIF)));

    SetBit(ADCSRA, ADIF);

    adc_LSB = ADCL;
    adc_MSB = ADCH;

    SetBit(ADCSRA, ADSC);

    return (adc_MSB<<8) | adc_LSB;
}

/**
 * @brief Inicializacao do USART
 * @param ubrr Valor calculado do baud rate
 * 
 * @note A definicao MYUBRR ja faz o calculo (datasheet)
 * e inicializa somento o TX, para o RX (1<<RXEN0) no
 * registrador UCSR0B
*/
void USART_Init(unsigned int ubrr)
{
    /*Set baud rate */
    UBRR0H = (unsigned char)(ubrr>>8);
    UBRR0L = (unsigned char)ubrr;
    /*Enable transmitter */
    SetBit(UCSR0B, TXEN0);
    /* Set frame format: 8data, 2stop bit */
    SetBit(UCSR0C, USBS0);
    SetBit(UCSR0C, UCSZ00);
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
 * @note A cada 30 segundos, ele libera a atualizacao
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