#include "ds3231.h"

struct _ds3231
{
    volatile uint8_t s;
    volatile uint8_t min;
    volatile uint8_t h;
}ds3231;

volatile uint8_t addr = 0x00;

void i2c_init()
{
    I2C_DDR  &= ~((1<<SCL) | (1<<SDA));
    //Ajuste da frequência de trabalho - SCL = F_CPU/(16+2.TWBR.Prescaler)
    TWBR = 18;
    TWSR |= 0x01;//prescaler = 4;
    TWCR |= (1<<TWINT) | (1<<TWEN) | (1<<TWIE); //habilita o TWI com interrupcao
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
    TWCR |= ((1<<TWINT) | (1<<TWSTO) | (1<<TWEN)) & ~(1<<TWSTA);
}

void ler_DS3231()
{
    i2c_start_bit();
    addr =0x00;
}

uint8_t get_seconds()
{
    return ds3231.s;
}

uint8_t get_minutes()
{
    return ds3231.min;
}

uint8_t get_hours()
{
    return ds3231.h;
}

ISR(TWI_vect)
{

    switch(TWSR & TW_STATUS_MASK)
    {
        case TW_START:

            TWDR = DS3231_ADDRESS | TW_WRITE;
            TWCR &= ~(1<<TWSTA);
            break;

        case TW_MT_SLA_ACK:
            TWDR = addr;
            break;

        case TW_MT_DATA_ACK:
            TWCR |= (1<<TWSTA);
            break;

        case TW_REP_START:
            TWDR = DS3231_ADDRESS | TW_READ;
            TWCR &= ~(1<<TWSTA);
            break;

        case TW_MR_SLA_ACK:
            TWCR &=~(1<<TWEA);
            break;

        case TW_MR_DATA_NACK:
            i2c_stop_bit();
            if(addr == 0x00)
                ds3231.s = TWDR;
            if(addr == 0x01)
                ds3231.min = TWDR;
            break;

        default:
            break;
    }

    TWCR |= (1<<TWINT);
}