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

#define start_bit() TWCR = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN)
#define stop_bit() TWCR = (1<<TWINT)|(1<<TWEN)| (1<<TWSTO)
#define espera_envio() while (!(TWCR & (1<<TWINT)))
#define envia_byte() TWCR = (1<<TWINT) | (1<<TWEN)
#define recebe_byte() TWCR = (1<<TWINT) | (1<<TWEN)
#define espera_recebimento() while (!(TWCR & (1<<TWINT)))
#define recebe_byte_ret_nack() TWCR = (1<<TWINT) | (1<<TWEN)

//A rotina de tratamento de erro é por conta do programador e da sua lógica de programação
#define teste_envio_start() if((TWSR & 0xF8) != TW_START) PORTB |= (1<<PB5);
#define teste_envio_end_escrita() if((TWSR & 0xF8) != TW_MT_SLA_ACK) PORTB |= (1<<PB5);
#define teste_envio_dado() if((TWSR & 0xF8) != TW_MT_DATA_ACK) PORTB |= (1<<PB5);
#define teste_envio_restart() if((TWSR & 0xF8) != TW_REP_START) PORTB |= (1<<PB5);
#define teste_envio_end_leitura() if((TWSR & 0xF8) != TW_MR_SLA_ACK) PORTB |= (1<<PB5);
#define teste_recebe_byte_ret_nack() if((TWSR & 0xF8) != TW_MR_DATA_NACK) PORTB |= (1<<PB5);

#define SOLO PC0
#define RELE PB0

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

uint8_t ler_RTC(uint8_t endereco);
void escreve_RTC(uint8_t dado, uint8_t endereco);
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

uint8_t ler_RTC(uint8_t endereco)
{
    start_bit();
    espera_envio();
    teste_envio_start();
    
    TWDR = 0xD0; //carrega o endereço para acesso do DS1307 (bit0 = 0, escrita)
                //para outro CI basta trocar este endereço

    envia_byte();
    espera_envio();
    teste_envio_end_escrita();
    
    TWDR = endereco; //ajuste do ponteiro de endereço para a leitura do DS1307
    
    envia_byte();
    espera_envio();
    teste_envio_dado();
    start_bit(); //reinício
    espera_envio();
    teste_envio_restart();
    
    TWDR = 0xD1; //carrega o endereço para acesso do DS1307 (bit0 = 1 é leitura)
                //automaticamente o ATmega chaveia para o estado de recepção
    
    envia_byte();
    espera_envio();
    teste_envio_end_leitura();
    
    recebe_byte_ret_nack(); //só lê um byte, por isso retorna um NACK
    espera_recebimento();
    teste_recebe_byte_ret_nack();
    
    stop_bit();
    
    return TWDR; //retorna byte recebido
}

void escreve_RTC(uint8_t dado, uint8_t endereco)
{
    start_bit();
    espera_envio();
    teste_envio_start();
    
    TWDR = 0xD0; //carrega o endereço para acesso do DS1307 (bit0 = 0 é escrita)
    //para outro CI basta trocar este endereço
    
    envia_byte();
    espera_envio();
    teste_envio_end_escrita();
    
    TWDR = endereco; //carrega o endereço para escrita do dado no DS1307
    
    envia_byte();
    espera_envio();
    teste_envio_dado();
    
    TWDR = dado; //carrega o dado para escrita no endereço especificado
    
    envia_byte();
    espera_envio();
    teste_envio_dado();
    
    stop_bit();
}