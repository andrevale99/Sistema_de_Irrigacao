#include "ds3231.h"

volatile uint8_t data = 0x00;

volatile uint8_t addr = 0x00;
volatile bool opr_complete = false;

uint8_t ler_DS3231(uint8_t _addr)
{
    opr_complete = false;
    addr = _addr;

    i2c_start_bit();
    while(!opr_complete)
        ;
    
    opr_complete = false;
    
    return data;
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
            data = TWDR;
            opr_complete = true;
            break;

        default:
            break;
    }

    TWCR |= (1<<TWINT);
}