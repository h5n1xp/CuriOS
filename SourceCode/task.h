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

#define GURU_MEDITATION_NO_ERROR        0
#define GURU_MEDITATION_OUT_OF_MEMORY   1
#define GURU_RECOVERABLE_FLAG          -1 //not really thought out yet, but some errors should not lock up the task...


#define TASK_SUSPENDED 0
#define TASK_WAITING 1
#define TASK_READY 2
#define TASK_RUNNING 3
#define TASK_ENDED 4


typedef struct task_t task_t;
struct task_t{
    node_t node;
    int32_t affinity;   // CPU number which this task must be scheduled to run on, or -1 for any CPU.
    uint32_t type;      // CPU type, FPU flag etc, also useful for hetrogenius cores, some Tasks may need high perforamce cores
    uint64_t state;     // i.e. TASK_READY
    uint64_t guru;      // Kernel Error Code, i.e. 1. Out Of Memory
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



void InitMultitasking(void);

#endif /* task_h */
