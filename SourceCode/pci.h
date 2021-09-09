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
    node_t node;
    uint16_t vendor;
    uint16_t device;
    uint16_t bus;
    uint16_t slot;
    uint8_t pClass;
    uint8_t subClass;
    uint8_t revisionID;
    uint8_t progIF;
    uint32_t BAR[6];

} PCINode_t;

typedef struct{
    device_t device;
    list_t PCIDeviceList;
    char** PCIClassName;
}pci_t;

typedef struct{
    unit_t unit;
}pciUnit_t;

extern pci_t pci;
void LoadPCIDevice(void);

#endif /* pci_h */
