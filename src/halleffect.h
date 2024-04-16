#ifndef HALLEFFECT_H
#define HALLEFFECT_H

#include "misc.h"

/* typedef enum Hall_Effect_Orientation { */
/*     Positive_Gauss, Negative_Gauss, */
/* } Hall_Effect_Orientation; */

/* typedef struct Hall_Effect { */
/*     u8 port; */
/*     Hall_Effect_Orientation orientation; */
/*     u16 max_gauss_value; */
/*     u16 min_gauss_value; */
/* } Hall_Effect; */

float halleffect_get_value(u16 rawADC);

#endif
