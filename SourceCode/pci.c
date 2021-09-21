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


char* PCIClassName[] = {"Unclassified","Mass Storage Controller","Network Controller","Display Controller","Multimedia Controller","Memory Controller","Bridge","Simple Communication Controller","Base System Peripheral","Input Device Controller","Docking Station","Processor","Serial Bus Controller","Wireless Controller", "Intelligent Controller","Satellite Communication Controller","Encryption Controller","Signal Processing Controller"};

pci_t pci;

#define CONFIG_ADDRESS  0xCF8
#define CONFIG_DATA     0xCFC

uint16_t pciConfigReadWord(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset){
    uint32_t address;
    uint32_t lbus  = (uint32_t)bus;
    uint32_t lslot = (uint32_t)slot;
    uint32_t lfunc = (uint32_t)func;
    uint16_t tmp = 0;
 
    /* create configuration address as per Figure 1 */
    address = (uint32_t)((lbus << 16) | (lslot << 11) | (lfunc << 8) | (offset & 0xfc) | ((uint32_t)0x80000000));
 
    /* write out the address */
    outl(CONFIG_ADDRESS, address);
    /* read in the data */
    /* (offset & 2) * 8) = 0 will choose the first word of the 32 bits register */
    tmp = (uint16_t)((inl(CONFIG_DATA) >> ((offset & 2) * 8)) & 0xffff);
    return (tmp);
}



uint32_t InitPCI(library_t* lib){
    
    InitList(&pci.device.unitList);
    InitList(&pci.PCIDeviceList);
    lib->node.name = "pci.device";
    
    
    
    //Just create a unit to start
    pciUnit_t* unit = (pciUnit_t*) executive->Alloc(sizeof(pciUnit_t),0);
    executive->AddHead(&pci.device.unitList,(node_t*)unit);
    
    debug_write_string("PCI Device: Init called\n");
    
    for(uint8_t bus = 0; bus < 4; ++bus){
        
        for(uint8_t slot = 0; slot < 32; ++slot){
                 
                    uint16_t vendor = pciConfigReadWord(bus, slot, 0, 0); // get vendor ID
                    
                    if(vendor != 0xFFFF){
    
                        //uint16_t device = pciConfigReadWord(bus, slot, 0, 2);  // get Device ID
                        

                        
                        uint16_t func = 0;
                        int f = 0;
                        
                        do{
                            
                            func = pciConfigReadWord(bus, slot, f, 2);  // get Function ID
                            if(func == 0xFFFF){break;}
                            
                            uint16_t progrev = pciConfigReadWord(bus, slot, f, 8);  // get class/subclass ID
                            uint8_t prog = progrev >> 8;
                            uint8_t rev = (uint8_t)(progrev & 255);
                            
                            uint16_t classword = pciConfigReadWord(bus, slot, f, 10);  // get class/subclass ID
                            
                            uint8_t class = classword >> 8;
                            uint8_t sub = (uint8_t)(classword & 255);
    
                            
                            //Save PCI information.
                            PCINode_t* pcinode = (PCINode_t*)executive->Alloc(sizeof(PCINode_t),0);
                            
                            for(int i=0; i<6;++i){
                                uint16_t addr = (i*4) + 16;
                                uint16_t lowBAR = pciConfigReadWord(bus, slot, f, addr);  // get lower BAR
                                uint16_t hiBAR = pciConfigReadWord(bus, slot, f, addr + 2);  // get higher BAR
                                pcinode->BAR[i] = (hiBAR << 16) | lowBAR;
                                
                            }
                            
                            pcinode->vendor = vendor;
                            pcinode->bus = bus;
                            pcinode->slot = slot;
                            pcinode->device = func; // multifunction devices have show up as multiple devices
                            pcinode->pClass = class;
                            pcinode->subClass = sub;
                            pcinode->progIF = prog;
                            pcinode->revisionID = rev;
                            executive->AddTail(&pci.PCIDeviceList,(node_t*)pcinode);
                            
                            debug_write_hex(slot);debug_putchar(':');
                            debug_write_string(PCIClassName[class]);debug_putchar(',');
                            debug_write_string(" Vendor: ");debug_write_hex(vendor);debug_putchar(' ');
                            //debug_write_string(" Bus: ");debug_write_hex(bus);debug_putchar(',');
                            
                            debug_write_string(" Device: ");debug_write_hex(func);debug_putchar(',');
                            debug_write_string(" Sub: ");debug_write_hex(sub);debug_putchar(',');
                            debug_write_string(" ProgIF: ");debug_write_hex(prog);debug_putchar(',');
                            //debug_write_string(" RevisionID: ");debug_write_hex(rev);debug_putchar(',');
                            debug_write_string(" BAR0: ");debug_write_hex(pcinode->BAR[0]);debug_putchar(',');
                            debug_putchar('\n');
                            f += 1;
                            
                        }while (func != 0xFFFF && f < 8);   // not sure why I've limited the function search to 8?
                            
                        
                        


                }
    
        }
    }
    
    
    return LIBRARY_INIT_SUCCESS;
    
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
    
    pci.PCIClassName = PCIClassName;
}
