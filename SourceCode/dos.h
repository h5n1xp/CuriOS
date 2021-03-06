//
//  dos.h
//
//  Created by Matt Parsons on 20/11/2020.
//  Copyright © 2020 Matt Parsons. All rights reserved.
//

#ifndef dos_h
#define dos_h

#include "stdheaders.h"
#include "library.h"
#include "device.h"
#include "list.h"

#include "dosCommonStructures.h"


#define DOS_ERROR_NO_ERROR 0
#define DOS_ERROR_UNKNOWN_COMMAND 100
#define DOS_ERROR_UNKNOWN_DEVICE 101
#define DOS_ERROR_DEVICE_NOT_SPECIFIED 102
#define DOS_ERROR_UNABLE_TO_OPEN_DEVICE 103
#define DOS_ERROR_NOT_A_DIRECTORY 104
#define DOS_ERROR_OBJECT_NOT_FOUND 105
#define DOS_ERROR_OBJECT_NOT_OF_REQUIRED_TYPE 106   // trying to perform file system action on a device which doesn't support them.

//The dosEntry is handler context, this defines what the device looks like to DOS
//this allows a file system to be set up on a device.


typedef struct{
    node_t node;    // The node name is the device name exposed to DOS, i.e. DH0: RAM: etc...
    
    //Dos entries can be a simple Alias to another Dos entry, with no underlying handler or device
    bool isAlias;
    char* assignPath;           // if this not NULL, then this is what the Aliased device is substitued with... possibly this should be a "mountpoint"?
    uint32_t dosType;           // information for the handler about the file system type.
    
    library_t* handler;         // This is the handler which DOS uses to communicate with the underlying device
    uint32_t handlerNumber;     // usually a partition number
    
    char* deviceName;           // this is the name of the underlying hardware device, the device can also be its own handler.
    uint32_t unitNumber;
    
    
    //Most, if not all, of this should be moved into the Handler's library structure
    uint32_t startBlock;        //LBA offset to partition
    uint32_t totalBlocks;
    uint32_t rootBlock;
    uint32_t sectorSize;        // normal size is 512bytes
    
    uint32_t FAT;               // Location of the File Allocation Table
    uint32_t sectorsPerCluster; // Only Valid on FAT partitions
    uint32_t sectorsPerFAT;     // Only Valid on FAT Partitions
    uint32_t dataArea;          // Only Valid on FAT Partitions
    

    
}dosEntry_t;


//This is the context which all DOS operations occur.
typedef struct{
    node_t node;

    char* pathName;
    char* fileName;
    bool isDIR;
    uint64_t size;
    uint64_t numberOfBlocks;    // <---- possibly this is meaningless...
    uint64_t startBlock;    // or in FAT language Cluster number
    dosEntry_t* entry;      // All the DOS device infomation is stored in the entry
    ioRequest_t* request;
    uint32_t position;      //current read position
}file_t;


typedef struct{
    library_t library;
    list_t dosList;
    void (*AddDosEntry)(dosEntry_t* entry);
    file_t* (*Open)(char* fileName, uint64_t attributes);
    void (*Close)(file_t* file);
    
    int (*Read)(file_t* file, void* buffer, uint32_t count);
    
    directoryStruct_t* (*Examine)(file_t* dir);
    uint8_t* (*LoadFile)(file_t* file);
    void (*LoadELF)(file_t* file);
}dos_t;


extern dos_t dos;


void LoadDOSLibrary(void);

#endif /* dos_h */
