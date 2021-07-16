//
//  fat_handler.h
//
//  Created by Matt Parsons on 27/11/2020.
//  Copyright © 2020 Matt Parsons. All rights reserved.
//

//Handlers are actually device structures, as devices can also be their own handlers
//The FAT handler is a file system which needs to be mounted on top of a block device (specified in the dosEntry_t)
//as such it doesn't have any of it's own device features, it is essentially just a library.
//
//
//Handlers are a superset of the device type, if a device can be accessed by DOS it needs to be a handler. if a
//device does not need to be exposed to the file system it doesn't need to be a handler.

#ifndef fat_handler_h
#define fat_handler_h

#include "handler.h"



typedef struct{
    handler_t handler;
} fatHandler_t;





extern fatHandler_t fatHandler;
void LoadFATHandler(void);


//Internal functions exposed while the hander features aren't used yet.
//void getPartitionData(file_t* file, int partitionNumber);
//void getVolumeBootRecord(file_t* file);
//void readBlock(file_t* file, uint32_t block);



//directoryStruct_t* readDir(file_t* file,uint32_t block);
//uint8_t* LoadFileAtCluster(file_t* file,uint32_t cluster);  //loads the complete file into memory



//void SetHandler(dosEntry_t*);
//uint32_t FATGetPartitionLBA(int partitionNumber);



#endif /* fat_handler_h */
