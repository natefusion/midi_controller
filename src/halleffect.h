#ifndef HALLEFFECT_H
#define HALLEFFECT_H

#include "misc.h"

typedef enum Hall_Effect_Orentation {
    Positive_Gauss, Negative_Gauss,
}

typedef struct Hall_Effect {
    u8 port;
    Orientation orientation;
    u16 max_gauss_value;
    u16 min_gauss_value;
} Hall_Effect;

u16 halleffect_get_value_normalized(Hall_Effect he);

#endif
