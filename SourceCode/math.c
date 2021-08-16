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

float sin(float x){
    float res=0;
    float pow=x;
    float fact=1.0;
    
    for(int i=0; i<6; ++i){
        res+=pow/fact;
        pow*=-1*x*x;
        fact*=(2*(i+1))*(2*(i+1)+1);
    }

    return res;
}

float cos(float x){
    
    x =1.570796327 - x;
    
    float res=0;
    float pow=x;
    float fact=1.0;
    
    for(int i=0; i<6; ++i){
        res+=pow/fact;
        pow*=-1*x*x;
        fact*=(2*(i+1))*(2*(i+1)+1);
    }

    return res;
}

/*
double sin(double x){
    int i = 1;
    double cur = x;
    double acc = 1;
    double fact= 1;
    double pow = x;
    while (fabs(acc) > .00000001 &&   i < 100){
        fact *= ((2*i)*(2*i+1));
        pow *= -1 * x*x;
        acc =  pow / fact;
        cur += acc;
        i++;
    }
    return cur;

}
*/
