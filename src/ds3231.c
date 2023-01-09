#include "ds3231.h"

struct _ds3231
{
    uint8_t horas[3];
}ds3231;

volatile uint8_t addr = 0x00;
volatile bool opr_complete = false;

volatile uint8_t idx = 0;

void ler_DS3231()
{
    for(idx = 0; idx < 0x03; ++idx)
    {
        i2c_start_bit();
        while(!opr_complete)
            ;
        opr_complete = false;
        addr += idx;
    }
    PORTB |= (1<<PB0);
}

uint8_t get_seconds()
{
    return ds3231.horas[0];
}

uint8_t get_minutes()
{
    return ds3231.horas[1];
}

uint8_t get_hours()
{
    return ds3231.horas[2];
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
            ds3231.horas[idx] = TWDR;
            opr_complete = true;
            break;

        default:
            break;
    }

    TWCR |= (1<<TWINT);
}