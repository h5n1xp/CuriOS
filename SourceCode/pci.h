//
//  pci.h
//
//  Created by Matt Parsons on 13/08/2021.
//  Copyright © 2021 Matt Parsons. All rights reserved.
//

#ifndef pci_h
#define pci_h

#include "stdheaders.h"
#include "device.h"

typedef struct{
    device_t device;

}pci_t;

typedef struct{
    unit_t unit;
}pciUnit_t;

extern pci_t pci;
void LoadPCIDevice(void);

#endif /* pci_h */
