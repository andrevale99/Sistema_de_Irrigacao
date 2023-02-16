#ifndef UTILITARIOS_C
#define UTILITARIOS_C

#define SetBit(port, pin) (port |= (1 << pin))
#define ClrBit(port, pin) (port &= ~(1 << pin))
#define ToggleBit(port, pin) (port ^= (1 << pin))
#define TestBit(port, pin) (port & (1 << pin))

#endif