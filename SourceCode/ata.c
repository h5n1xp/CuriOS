
//
//  ata.c
//
//  Created by Matt Parsons on 16/11/2020.
//  Copyright © 2020 Matt Parsons. All rights reserved.
//

#include "ata.h"
#include "memory.h"
#include "SystemLog.h"
#include "x86cpu_ports.h"

#define ATA_PRIMARY 0x1F0

#define ATA_REG_DRIVE_SELECT 0x06
#define ATA_REG_SECTOR_COUNT 0x02
#define ATA_REG_LBA_LO  0x03
#define ATA_REG_LBA_MID 0x04
#define ATA_REG_LBA_HI  0x05

#define ATA_REG_COMMAND 0x07
#define ATA_REG_STATUS  0x07

#define ATA_READ_SECTORS 0x20

#define ATA_SELECT_MASTER 0xA0
#define ATA_SELECT_SLAVE 0xB0

#define ATA_IDENTIFY 0xEC
#define ATA_STATUS_BUSY 0x80

#define ATA_SELECT_LBA_MODE 0x40

ata_t ata;

uint8_t numberOfDrives = 0;

messagePort_t* ATAPort;

//************************************Low level functions

uint8_t busy_wait_for_drive(){  //only to be used when no interrupts are available.
    uint8_t status = 0;
    
    int countdown = 1000;
    
    //debug_write_string(" Wait for Drive - >");
    
    do{
        status = inb(ATA_PRIMARY+ATA_REG_STATUS);
       // --countdown;
    }while( (status & ATA_STATUS_BUSY) == ATA_STATUS_BUSY && countdown > 0 );
    
    //debug_write_string(" Ok\n");
    
    return status;
}



void lowLevelRead(uint16_t disk, int8_t unit, uint8_t* sector, uint8_t* buffer,uint32_t bufferOffset){
    
    //Need to scan the cache first then only read the disk if the sector is not cached
    
    //uint16_t one = 1;
    //uint8_t* count = (uint8_t*)&one;
    
    //debug_write_string("Low Level Read\n");
    
    outb(disk + ATA_REG_DRIVE_SELECT, unit); // Set the primaryto LBA mode
    
    outb(disk + ATA_REG_SECTOR_COUNT, 0);                   // write the high byte of the sector count
    outb(disk + ATA_REG_LBA_LO,sector[3]);
    outb(disk + ATA_REG_LBA_MID,sector[4]);
    outb(disk + ATA_REG_LBA_HI,sector[5]);
    
    outb(disk + ATA_REG_SECTOR_COUNT, 1);                    // write the low byte of the sector count
    outb(disk + ATA_REG_LBA_LO,sector[0]);
    outb(disk + ATA_REG_LBA_MID,sector[1]);
    outb(disk + ATA_REG_LBA_HI,sector[2]);
    
    outb(disk  + ATA_REG_COMMAND, ATA_READ_SECTORS);
    
    //Wait for drive
    busy_wait_for_drive();
    

    
    //transfer data
    for(uint32_t i=(bufferOffset*512); i<((bufferOffset*512)+511); i+=2 ){
        uint16_t data= inw(disk);
        
        buffer[i] = data & 255;
        buffer[i+1] = (data >> 8) & 255;
    }
    

}

void read_blocks(uint16_t disk,uint64_t start,uint16_t sectorCount, uint8_t* buffer){
    
    uint8_t unit = 0;
    
    if(disk == 0){
        disk = ATA_PRIMARY;
        unit = 0x40;    //master
    }else if(disk == 1){
        disk = ATA_PRIMARY;
        unit = 0x50;    //slave
    }else{
        return;
    }
    
    if(sectorCount <1){
        sectorCount = 1;
    }
    

    
    for(uint32_t j = 0; j<sectorCount;++j){
    
        //debug_write_string("Low Revel Read started\n");
        lowLevelRead(disk, unit, (uint8_t*)&start, buffer, j);
        start++;
        //debug_write_string("Low Revel Read Stopped\n");
    }
    
    
    
    return;
    

}




//*********************************************************
// The ATA device task which processes the device messages
int ATATaskEntry(){
    
    //debug_write_string("ATA Device: Started\n");
    
    while(1){
        //debug_write_string("ATA Device: Wait!\n");
        executive->WaitPort(ATAPort);

        ioRequest_t* req =(ioRequest_t*)executive->GetMessage(ATAPort);
        ataUnit_t* unit = (ataUnit_t*)req->unit;
        

        //debug_write_string("ATA Device: Message Received!\n");
        switch(req->command){
            case CMD_READ:
               //debug_write_string("ATA Device: Read!\n");

                read_blocks(unit->driveNumber,req->offset/512,req->length/512,req->data);
                
                break;
            default:
                req->error = IO_ERROR;
                break;
                
        }

        executive->ReplyMessage((message_t*)req);
        //debug_write_string("ATA Device: Done!\n");
    }
    
}

uint32_t InitATA(library_t* lib){
    
    InitList(&ata.device.unitList);
    
    ata.device.task = executive->CreateTask("ata.device",10,ATATaskEntry,4096);
    executive->SetTaskPriPrivate(ata.device.task,10);
    executive->AddTaskPrivate(ata.device.task);  // too early for executive messages, so have to use the private function... the ATA device should be started later in the boot process really so it can use the non private function.
    lib->node.name = "ata.device";
    
    
    //just have a single messagePort for all the ATA device units for now...
    //This setup needs to be done in the actual device task!!!
    ATAPort = executive->CreatePort("ATA Unit"); // WARNING THIS SETS THE OWNING TASK TO THE InitATA() CALLER, NOT THE DEVICE TASK!!!
    ATAPort->owner = ata.device.task;   //need to set the correct owning task of this message port.
    
    // identify the primary drive
    
    uint16_t driveStatusData[256];
    driveStatusData[100] = 0;       //ensuring 0 to suppress compiler warning
    
    outb(ATA_PRIMARY + ATA_REG_DRIVE_SELECT,ATA_SELECT_MASTER);
    
    outb(ATA_PRIMARY + ATA_REG_SECTOR_COUNT, 0);
    outb(ATA_PRIMARY + ATA_REG_LBA_LO, 0);
    outb(ATA_PRIMARY + ATA_REG_LBA_MID,0);
    outb(ATA_PRIMARY + ATA_REG_LBA_HI, 0);
    
    outb(ATA_PRIMARY + ATA_REG_COMMAND, ATA_IDENTIFY);
    
    if(inb(ATA_PRIMARY+ATA_REG_STATUS)==0){
        debug_write_string("ATA Device: Master - No Hard disks\n");
    }else{
        
        busy_wait_for_drive();
        
        for(int i =0; i<256;++i ){
            driveStatusData[i] = inw(ATA_PRIMARY);
        }
        
        if(driveStatusData[83] & 0x400){
           //debug_write_string("ATA Device: Master - LBA48 Drive - ");
            
            uint64_t* lba = 0;
            lba = (uint64_t*)&driveStatusData[100];
            //debug_write_hex(*lba);debug_write_string(" sectors in size\n");
            
            ataUnit_t* ataUnit = (ataUnit_t*)executive->Alloc(sizeof(ataUnit_t),0);
            ataUnit->unit.node.name = "DH0:";
            ataUnit->unit.messagePort = ATAPort;
            ataUnit->unit.openCount = 0;
            ataUnit->primary = (uint8_t)ATA_PRIMARY;
            ataUnit->driveNumber = numberOfDrives; numberOfDrives += 1;
            ataUnit->sectors = *lba;
            
            AddTail(&ata.device.unitList,(node_t*)ataUnit);
            
        }else{
            
           debug_write_string("ATA Device: Master - LBA28 Drive\n");
            uint64_t* lba = 0;
            lba = (uint64_t*)&driveStatusData[100];
           debug_write_hex(*lba);debug_write_string(" sectors in size (not accurate for LBA 28 drives)\n");
            
            ataUnit_t* ataUnit = (ataUnit_t*)executive->Alloc(sizeof(ataUnit_t),0);
            ataUnit->unit.node.name = "DH0:";
            ataUnit->unit.messagePort = ATAPort;
            ataUnit->unit.openCount = 0;
            ataUnit->primary = (uint8_t)ATA_PRIMARY;
            ataUnit->driveNumber = numberOfDrives; numberOfDrives += 1;
            ataUnit->sectors = *lba;
            
            AddTail(&ata.device.unitList,(node_t*)ataUnit);
        }
        
    }
    

    
    // identify the secondary drive
    
    outb(ATA_PRIMARY + ATA_REG_DRIVE_SELECT,ATA_SELECT_SLAVE);
    
    outb(ATA_PRIMARY + ATA_REG_SECTOR_COUNT, 0);
    outb(ATA_PRIMARY + ATA_REG_LBA_LO, 0);
    outb(ATA_PRIMARY + ATA_REG_LBA_MID,0);
    outb(ATA_PRIMARY + ATA_REG_LBA_HI, 0);
    
    outb(ATA_PRIMARY + ATA_REG_COMMAND, ATA_IDENTIFY);
    

    
    if(inb(ATA_PRIMARY+ATA_REG_STATUS)==0){
       //debug_write_string("ATA Device: Slave - No disk.\n");
    }else{
        
        busy_wait_for_drive();
        
        for(int i =0; i<256;++i ){
            driveStatusData[i] = inw(ATA_PRIMARY);
        }
        
        if(driveStatusData[83] & 0x400){
            
            uint64_t* lba = 0;
            lba = (uint64_t*)&driveStatusData[100];
            //debug_write_hex(*lba);debug_write_string(" \n");
            
            ataUnit_t* ataUnit = (ataUnit_t*)executive->Alloc(sizeof(ataUnit_t),0);
            ataUnit->unit.messagePort = ATAPort;
            ataUnit->unit.node.name = "DH1:";
            ataUnit->unit.openCount = 0;
            ataUnit->primary = (uint8_t)ATA_PRIMARY;
            ataUnit->driveNumber = numberOfDrives; numberOfDrives += 1;
            ataUnit->sectors = *lba;
            
            AddTail(&ata.device.unitList,(node_t*)ataUnit);
            
        }else{
            
            //debug_write_string("ATA Device: Slave - LBA28 Drive\n");
            uint64_t* lba = 0;
            lba = (uint64_t*)&driveStatusData[100];
            debug_write_hex(*lba);debug_write_string(" sectors in size\n");
            
            ataUnit_t* ataUnit = (ataUnit_t*)executive->Alloc(sizeof(ataUnit_t),0);
            ataUnit->unit.node.name = "DH1:";
            ataUnit->unit.messagePort = ATAPort;
            ataUnit->unit.openCount = 0;
            ataUnit->primary = (uint8_t)ATA_PRIMARY;
            ataUnit->driveNumber = numberOfDrives; numberOfDrives += 1;
            ataUnit->sectors = *lba;
            
            AddTail(&ata.device.unitList,(node_t*)ataUnit);

        }
        
    }
    
    //debug_write_string("-->");
    //debug_write_string("ata.device: ");
    //debug_write_dec(ata.device.unitList.count);
    //debug_write_string(" unit on signal: ");
    //debug_write_dec(ATAPort->sigNum);debug_write_string(" \n");
    
    return LIBRARY_INIT_SUCCESS;
    
}


library_t* ATAOpen(library_t* lib){
    lib->openCount += 1;
    //debug_write_string("ATA Device:Opened!\n");
    return lib;
}

unit_t* ATAGetUnit(uint64_t number){
   
    return (unit_t*) executive->ItemAtIndex(&ata.device.unitList,number);

}

uint64_t ATAUnitCount(){
    return ata.device.unitList.count;
}

void ATAClose(library_t* lib){
    lib->openCount -= 1;
    //debug_write_string("ATA Device:Closed!\n");
}

void ATABeginIO(ioRequest_t* req){
    req->error = IO_IN_PROGRESS;
    
    if(req->flags == IOF_QUICK){
        debug_write_string("ATA Device: quick IO not supported.\n");
        req->flags = 0;
    }
    
    executive->PutMessage(req->unit->messagePort,(message_t*)req);
    
}


//Too be called only by the Kernel_main as it just sets up the device memory.
void LoadATADevice(){
    
    ata.device.library.node.name        = "ata.device";
    ata.device.library.node.type        = NODE_DEVICE;  // Devices can't be accessed by the file system, handlers can though.
    ata.device.library.Open             = ATAOpen;
    ata.device.library.Close            = ATAClose;
    ata.device.library.Init             = InitATA;
    ata.device.library.baseLibrary      = &ata.device.library;
    ata.device.GetUnit                  = ATAGetUnit;
    ata.device.UnitCount                = ATAUnitCount;
    ata.device.BeginIO                  = ATABeginIO;
}
