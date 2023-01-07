#include "ds3231.h"

unsigned char dia;

void i2c_init()
{
    //Ajuste da frequência de trabalho - SCL = F_CPU/(16+2.TWBR.Prescaler)
    TWBR = 18;
    TWSR |= 0x01;//prescaler = 4;
    TWCR |= (1<<TWINT) | (1<<TWEN) | (1<<TWIE); //habilita o TWI
}

void i2c_init_pullup()
{
    I2C_DDR  &= ~((1<<SCL) | (1<<SDA));
    I2C_PORT |= (1<<SCL) | (1<<SDA);

    i2c_init(); 
}

void i2c_unit()
{
    TWCR &= ~(1<<TWEN);
}

void i2c_start_bit()
{
    TWCR |= (1<<TWINT) | (1<<TWSTA) | (1<<TWEN);
}

void i2c_stop_bit()
{
    TWCR |= (1<<TWINT) | (1<<TWSTO) | (1<<TWEN);
}

ISR(TWI_vect)
{

    switch(TWSR & 0xF8)
    {
        case 0x08:

            TWDR = DS3231_ADDRESS | WRITE;
            TWCR &= ~(1<<TWSTA);
            break;

        case 0x18:
            TWDR = 0x00;
            break;

        case 0x28:
            TWCR |= (1<<TWSTA);
            break;

        case 0x10:
            TWDR = DS3231_ADDRESS | READ;
            TWCR &= ~(1<<TWSTA);
            break;

        case 0x40:
            TWCR &=~(1<<TWEA);
            PORTB |= (1<<PB5);
            break;

        case 0x58:
            TWCR |= (1<<TWSTO);
            PORTB ^= (1<<PB0);
            dia = TWDR;
            break;

        default:
            break;
    }

    TWCR |= (1<<TWINT);
}