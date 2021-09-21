//
//  console_device.h
//
//  Created by Matt Parsons on 17/07/2021.
//  Copyright © 2021 Matt Parsons. All rights reserved.
//



#ifndef console_device_h
#define console_device_h

#include "device.h"

typedef struct{
    device_t device;
    
} console_device_t;

typedef struct{
    unit_t unit;
    unit_t consoleNumber;
} console_unit_t;

extern console_device_t console;

void LoadConsoleDevice(void);

#endif /* console_device_h */
