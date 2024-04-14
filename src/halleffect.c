#include "halleffect.h"
#include "adc.h"

u16 halleffect_get_value(Hall_Effect he) {
    u16 rawADC = adc_read_port(he.port);
    return rawADC;
}
