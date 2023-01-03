#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#include <stdint.h>

#define SOLO ADC0

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
    DDRB |= (1<<PB5);

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