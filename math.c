//
//  math.c
//
//  Created by Matt Parsons on 13/11/2020.
//  Copyright © 2020 Matt Parsons. All rights reserved.
//

#include "math.h"

int32_t abs(int32_t value){
    
    if ( value < 0){value = -value;}
    return value;
}

//float version of my bicubic interpolation function
float hermite(float x, float y0, float y1, float y2, float y3){
    // 4-point, 3rd-order Hermite (x-form)
    float c0 = y1;
    float c1 = 0.5f * (y2 - y0);
    float c3 = 1.5f * (y1 - y2) + 0.5f * (y3 - y0);
    float c2 = y0 - y1 + c1 - c3;
    
    return ((c3 * x + c2) * x + c1) * x + c0;
}
