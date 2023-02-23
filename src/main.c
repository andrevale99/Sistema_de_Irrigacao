/**
 * @author Andre Menezes de Freitas Vale
 * @date 03/01/2023
 *
 * @version 1.0.1
 *
 * @brief Programa para realizar a irrigacao automatica
 * de um jardim utlizando a programacao em C e o microcontrolador
 * AVR Atmega328p
 *
 * @note A parte da programacao do RTC e do LCD foram baseados no livro
 * "AVR e Arduino: Tecnicas de Projeto" de Charles Borges de Lima
 *  e Marco V. M. Villaca
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

#include "Utilitarios.c"

#define RELE PD0
#define BT PD1
#define POT PC0
#define SOLO PC1

#define SOLO_SECO 500

//===============================================
//  VARIAVEIS
//===============================================
volatile uint8_t ovf_secs = 0x00;
volatile uint8_t cnt = 0x00;

volatile uint8_t horas[3] = {0, 0, 0};
volatile bool refresh_horas = false;

char str_adc[] = "0000";
char str_horas[] = "000000";

void (*function_ptr)(); // Ponteiro de função, quando haver troca de rotina
//===============================================
//  PROTOTIPOS
//===============================================
void setup();

void gpio_setup();

void adc_setup();
uint16_t adc_read(uint8_t pino);

void timer1_setup();

void convert_to_str(uint32_t value, char str[], uint8_t len);
void reset_string(char str[]);

void rotina_principal();

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

    function_ptr = rotina_principal;

    for (;;)
    {
        (*function_ptr)();
        _delay_ms(250);
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
    // Desativa os Pinos RX e TX para serem usados
    // como pinos normais, caso precise
    UCSR0B |= 0x00;

    gpio_setup();

    adc_setup();

    i2c_init();

    timer1_setup();

    inic_LCD_4bits();
}

/**
 * @brief Inicializacao dos pinos
 */
void gpio_setup()
{
    SetBit(DDRB, PB5);
    SetBit(DDRB, PB0);
    SetBit(DDRB, PB1);

    SetBit(DDRD, RELE);

    ClrBit(DDRD, BT);
    SetBit(PORTD, BT);

    // Inicialização da via de dados do LCD
    DDRD |= 0xF0;
}

/**
 * @brief Inicializacao do ADC
 *
 * @note Ativa o ADC com o prescale de 16e6/128 e
 * referencia no AVCC/AREF. Desativa as entradas
 * analogicas A2 e A3 para consumir menos energia
 */
void adc_setup()
{
    SetBit(ADCSRA, ADEN);
    SetBit(ADCSRA, ADPS2);
    SetBit(ADCSRA, ADPS1);
    SetBit(ADCSRA, ADPS0);

    SetBit(ADMUX, REFS0);

    SetBit(DIDR0, ADC2D);
    SetBit(DIDR0, ADC3D);
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
 * ja que a resolucao do arduino e de 10 bits. NAO
 * USA INTERRUPCAO
 */
uint16_t adc_read(uint8_t pino)
{
    static uint8_t adc_LSB;
    static uint8_t adc_MSB;

    ADMUX |= pino;

    SetBit(ADCSRA, ADSC);

    while (!(ADCSRA &= ~(1 << ADIF)))
        ;

    SetBit(ADCSRA, ADIF);

    adc_LSB = ADCL;
    adc_MSB = ADCH;

    SetBit(ADCSRA, ADSC);

    return (adc_MSB << 8) | adc_LSB;
}

/**
 * @brief Converte o valor adc em um
 * vetor de char
 *
 * @param adc valor do adc
 */
void convert_to_str(uint32_t value, char str[], uint8_t len)
{
    uint16_t i = 10;
    uint8_t idx = len;
    while (value != 0)
    {
        str[idx--] = (value % i) + '0';
        value = value / i;
    }
}

/**
 * @brief Reseta a string da conversao
 * do adc
 *
 * @param str vetor de char para resetar
 */
void reset_string(char str[])
{
    memcpy(str, "0000", 4);
}

/**
 * @brief Rotina principal, somente atualiza o tempo e
 * ativa o rele
 */
void rotina_principal()
{
    escreve_LCD("PRINCIPAL");
    
    cmd_LCD(PULAR_LINHA, 0);

    convert_to_str(horas[2], str_horas, 6);
    escreve_LCD(str_horas);
    cmd_LCD(RETURN_HOME, 0);


    if (refresh_horas)
    {
        ClrBit(TIMSK1, OCIE1A);

        horas[0] = ler_DS3231(0x02);
        horas[1] = ler_DS3231(0x01);
        horas[2] = ler_DS3231(0x00);

        refresh_horas = false;

        TCNT1 = 0;
        ToggleBit(TIMSK1, OCIE1A);
    }

    if (horas[0] == 6 || horas[0] == 18)
    {
        if (adc_read(SOLO) <= SOLO_SECO)
            SetBit(PORTD, RELE);
        else
            ClrBit(PORTD, RELE);
    }
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

    if (ovf_secs == 5)
    {
        refresh_horas = true;
        ovf_secs = 0;
    }
}