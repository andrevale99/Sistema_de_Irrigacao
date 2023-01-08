#include "ds3231.h"

struct _ds3231
{
    volatile uint8_t s;
    volatile uint8_t min;
    volatile uint8_t h;
}ds3231;

volatile uint8_t addr = 0x00;
volatile bool opr_complete = false;

void ler_DS3231(uint8_t _addr)
{
    i2c_start_bit();
    addr = _addr;
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
            ds3231.min = TWDR;
            PORTB |= (1<<PB5);
            break;

        default:
            break;
    }

    TWCR |= (1<<TWINT);
}