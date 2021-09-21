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


//Only full tasks (Processes?) need a DOS context, a simple threads need not allocate one
typedef struct{
    void* dosPort;      //Rendezvous port for all DOS operations
    uint64_t dosError;  //Value set by the last DOS operation
    void* stdout;       //IORequest for writing to the console
    char* progdir;      //path to which all dos operations are relative
} dosContext_t;


typedef struct task_t task_t;
struct task_t{
    node_t node;
    int32_t affinity;   // CPU number which this task must be scheduled to run on, or -1 for any CPU.
    uint32_t type;      // CPU type, FPU flag etc, also useful for hetrogenius cores, some Tasks may need high perforamce cores
    uint32_t state;     // i.e. TASK_READY
    uint32_t CPU;       // current CPU the task is executing on... only meaningfull when task state = TASK_RUNNING
    uint32_t guru;      // Kernel Error Code, i.e. 1. Out Of Memory
    int32_t forbidCount; //Forbid() shuts down multitasking on all CPUs, it's use is being phased out, and the function will be deprecated.
    task_t* parent;
    void* rendezvousPort;   //Task's primary communication port
    dosContext_t* DOSContext;

    void* dosPort;      //Rendezvous port for all DOS operations
    uint64_t dosError;  //Value set by the last DOS operation
    char* progdir;      //path to which all dos operations are relative
    
    list_t memoryList;
    int (*entry)(void);
    int (*exit)(void);      //To be automatically called when signal 0x1 is recevied;
    uint64_t signalAlloc;
    uint64_t signalReceived;    // received
    uint64_t signalWait;
    void_ptr usp_top;
    void_ptr ssp;
    void_ptr ssp_top;
};


typedef struct{
    node_t node;
    task_t* task;
}taskListNode_t;

void InitMultitasking(void);

#endif /* task_h */
