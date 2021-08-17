//
//  handler.h
//
//  Created by Matt Parsons on 16/07/2021.
//  Copyright © 2021 Matt Parsons. All rights reserved.
//

//A handler is a superset of the device type, which also exposes a DOS compatible interface.
//Devices and handlers are otherwise indentical and can expose both types of interface if it makes sense to do so.
//Handlers are generally used as a "middle man" for devices and DOS.

#ifndef handler_h
#define handler_h

#include "stdheaders.h"
#include "device.h"
#include "dos.h"
#include "dosCommonStructures.h"
#include "list.h"
#include "library.h"
#include "ports.h"
#include "task.h"

typedef struct{
    device_t device;
    bool isMounted;     //set to true if the handler is mounted
    int (*Mount)(dosEntry_t* entry);       //perform all the handler set up, opening the device etc... return an error code.
    void (*Unmount)(dosEntry_t* entry);
    directoryStruct_t* (*ReadDir)(file_t* file,uint32_t block);
    uint8_t* (*LoadFileAtCluster)(file_t* file,uint32_t cluster);  //loads the complete file into memory
} handler_t;



#endif /* handler_h */
