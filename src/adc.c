#include <avr/io.h>
#include "adc.h"

void adc_init(void) {
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
    ADMUX = (1 << REFS0) ;
}

u10 adc_read_port(u3 port) {
    ADMUX = (ADMUX & 0xF8) | port;
    ADCSRA |= 1 << ADSC;
    while ((ADCSRA & (1 << ADIF)) == 0);
    ADCSRA |= 1 << ADIF;

    return ADC;
}
