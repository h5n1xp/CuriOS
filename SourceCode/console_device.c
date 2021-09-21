//
//  console_device.c
//
//  Created by Matt Parsons on 17/07/2020.
//  Copyright © 2021 Matt Parsons. All rights reserved.
//

#include "console_device.h"
#include "intuition.h"

intuition_t* intuiBase;

console_device_t console;
console_unit_t bootConsoleUnit;

uint64_t CDUnitCount(){
    
    // Unit 0 is the system console
    // Unit 1 is a new console
    
    return 2;
}

unit_t* CDGetUnit(uint64_t number){
    
    //This is where the actual console is created
    
    if(number == 0){
            return (unit_t*)&bootConsoleUnit;
    }
    
    return NULL;
}

uint32_t CDInit(library_t* lib){
    
    lib += 0;   //supress compiler warning until we get a proper init
    
    return LIBRARY_INIT_SUCCESS;
}

library_t* CDOpen(library_t* lib){
    lib->openCount += 1;
    //debug_write_string("Console Device: Opened!\n");
    return lib;
}

void CDClose(library_t* lib){
    lib->openCount -= 1;
    //debug_write_string("Console Device: Closed!\n");
}

void CDBeginIO(ioRequest_t* req){
    req->error = IO_IN_PROGRESS;
    
    if(req->flags == IOF_QUICK){
        //debug_write_string("ATA Device: quick IO not supported.\n");
        req->flags = 0;
    }
    
   // executive->PutMessage(req->unit->messagePort,(message_t*)req);
    
}

void LoadConsoleDevice(void){
    
    console.device.library.node.name        = "console.device";
    console.device.library.node.type        = NODE_DEVICE;  // Devices can't be accessed by the file system, handlers can though.
    console.device.library.Open             = CDOpen;
    console.device.library.Close            = CDClose;
    console.device.library.Init             = CDInit;
    console.device.library.baseLibrary      = &console.device.library;
    console.device.GetUnit                  = CDGetUnit;
    console.device.UnitCount                = CDUnitCount;
    console.device.BeginIO                  = CDBeginIO;
}
