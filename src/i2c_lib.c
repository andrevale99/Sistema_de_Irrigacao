#include "i2c_lib.h"

void i2c_init()
{
    //Ajuste da frequência de trabalho - SCL = F_CPU/(16+2.TWBR.Prescaler)
    TWBR = 18;
    TWSR |= 0x01;//prescaler = 4;
    TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWIE); //habilita o TWI e a interrupção
}