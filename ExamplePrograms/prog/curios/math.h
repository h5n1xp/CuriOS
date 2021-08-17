//
//  math.h
//
//  Created by Matt Parsons on 13/11/2020.
//  Copyright © 2020 Matt Parsons. All rights reserved.
//

#ifndef math_h
#define math_h

#include "stdheaders.h"

#define PI 3.141592654

int32_t abs(int32_t value);

float hermite(float x, float y0, float y1, float y2, float y3); //simple bicubic interpolation

float sin(float x);
float cos(float x);
#endif /* math_h */
