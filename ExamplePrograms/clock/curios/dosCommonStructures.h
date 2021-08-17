//
//  dosCommonStructures.h
//
//  Created by Matt Parsons on 10/07/2021.
//  Copyright © 2021 Matt Parsons. All rights reserved.
//

#ifndef dosCommonStructures_h
#define dosCommonStructures_h

#include "stdheaders.h"

typedef struct {
    char name[64];
    uint32_t cluster;
    uint32_t flags;
    uint32_t fileSize;
    bool isDir;
    bool isHidden;
} directoryEntry_t;

typedef struct{
    uint32_t size;
    directoryEntry_t* entry;
    directoryEntry_t data;      //start of the data array in memory
} directoryStruct_t;

#endif /* dosCommonStructures_h */
