//
//  pci.c
//
//  Created by Matt Parsons on 13/08/2021.
//  Copyright © 2021 Matt Parsons. All rights reserved.
//

#include "pci.h"
#include "memory.h"
#include "SystemLog.h"
#include "x86cpu_ports.h"

pci_t pci;

void InitPCI(library_t* lib){
    
    InitList(&pci.device.unitList);
    lib->node.name = "pci.device";
    
    //Just create a unit to start
    pciUnit_t* unit = (pciUnit_t*) executive->Alloc(sizeof(pciUnit_t));
    executive->AddHead(&pci.device.unitList,(node_t*)unit);
    
    debug_write_string("PCI Device: Init called\n");
    
    
    
}


library_t* PCIOpen(library_t* lib){
    lib->openCount += 1;
    //debug_write_string("PCI Device:Opened!\n");
    return lib;
}

void PCIClose(library_t* lib){
    lib->openCount -= 1;
    //debug_write_string("PCI Device:Closed!\n");
}

void PCIBeginIO(ioRequest_t* req){
    req->error = IO_IN_PROGRESS;
    
    if(req->flags == IOF_QUICK){
        debug_write_string("PCI Device: quick IO not supported.\n");
        req->flags = 0;
    }
    
}


//Too be called only by the Kernel_main as it just sets up the device memory.
void LoadPCIDevice(){
    
    pci.device.library.node.type    = NODE_DEVICE;  // Devices can't be accessed by the file system, handlers can though.
    pci.device.library.Open             = PCIOpen;
    pci.device.library.Close            = PCIClose;
    pci.device.library.Init             = InitPCI;
    pci.device.library.baseLibrary      = &pci.device.library;
    pci.device.BeginIO                  = PCIBeginIO;
}
