#include <avr/io.h>
#include "adc.h"

// 5 is good, not too big nor too small
#define WINDOW_SIZE 5

static u16 adc_sum = 0;
static u16 adc_average = 0;
static u16 adc_readings[WINDOW_SIZE] = {0};
static u16 adc_readings_index = 0;

void adc_init(void) {
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
    ADMUX = (1 << REFS0) ;
}

u10 adc_read_port(u3 port) {
    ADMUX = (ADMUX & 0xF8) | port;
    ADCSRA |= 1 << ADSC;
    while ((ADCSRA & (1 << ADIF)) == 0);
    ADCSRA |= 1 << ADIF;

    u16 value = ADC;    
    adc_sum -= adc_readings[adc_readings_index];
    adc_readings[adc_readings_index] = value;
    adc_sum += value;
    adc_readings_index = (adc_readings_index+1) % WINDOW_SIZE;
    adc_average = adc_sum / WINDOW_SIZE;
    
    return adc_average;
}
