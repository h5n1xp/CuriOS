//
//  task.h
//
//  Created by Matt Parsons on 02/11/2020.
//  Copyright © 2020 Matt Parsons. All rights reserved.
//

#ifndef task_h
#define task_h

#include "stdheaders.h"
#include "list.h"


#define TASK_SUSPENDED 0
#define TASK_WAITING 1
#define TASK_READY 2
#define TASK_RUNNING 3


typedef struct task_t task_t;
struct task_t{
    node_t node;
    uint64_t state;     // i.e. TASK_READY
    task_t* parent;
    void* dosPort;      //Rendevous port for all DOS operations
    uint64_t dosError;  //Value set by the last DOS operation
    char* progdir;      //path to which all dos operations are relative
    list_t memoryList;
    int32_t forbidCount;
    int (*entry)(void);
    int (*exit)(void);      //To be automatically called when signal 0x1 is recevied;
    uint64_t signalAlloc;
    uint64_t signalReceived;    // received
    uint64_t signalWait;
    void_ptr usp_top;
    void_ptr ssp;
    void_ptr ssp_top;
};

void* TaskStartup(); //first call a task will make, returns pointer to the executive


void InitMultitasking(void);

#endif /* task_h */
