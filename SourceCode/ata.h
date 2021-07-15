//
//  ata.h
//
//  Created by Matt Parsons on 16/11/2020.
//  Copyright © 2020 Matt Parsons. All rights reserved.
//

#ifndef ata_h
#define ata_h

#include "stdheaders.h"
#include "device.h"

typedef struct{
    device_t device;

}ata_t;

typedef struct{
    unit_t unit;
    uint8_t  primary;
    uint32_t driveNumber;
    uint64_t sectors;
}ataUnit_t;

extern ata_t ata;
void LoadATADevice(void);

#endif /* ata_h */
