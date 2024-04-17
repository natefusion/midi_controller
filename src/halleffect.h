#ifndef HALLEFFECT_H
#define HALLEFFECT_H

#include "misc.h"

/*
  0: q=2.595, s=1.700
  1: q=2.525, s=1.765
  2: q=2.570, s=1.775
  3: q=2.525, s=1.760
  4: q=2.615, s=1.685
 */

/* typedef enum Hall_Effect_Orientation { */
/*     Positive_Gauss, Negative_Gauss, */
/* } Hall_Effect_Orientation; */

/* typedef struct Hall_Effect { */
/*     u8 port; */
/*     Hall_Effect_Orientation orientation; */
/*     u16 max_gauss_value; */
/*     u16 min_gauss_value; */
/* } Hall_Effect; */

float halleffect_get_value(u8 index, u16 rawADC);

#endif
