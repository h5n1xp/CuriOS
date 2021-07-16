//
//  memory.h
//  GFXConvert
//
//  Created by Matt Parsons on 25/10/2020.
//  Copyright © 2020 Matt Parsons. All rights reserved.
//

#ifndef memory_h
#define memory_h

#include "stdheaders.h"
#include "list.h"
#include "task.h"
#include "ports.h"
#include "library.h"
#include "device.h"
#include "handler.h"

#include "interrupts.h"

typedef struct{
    list_t freeList;
    uint64_t memSize;
    uint64_t ticks;
    uint64_t quantum;
    int64_t elapsed;
    
    uint64_t allocationTotal;   // it might make sense to audit the total number of allocations vs deallocations at some point
    uint64_t deallocationTotal;
    
    node_t* (*Alloc)(size_t);       //For allocating node_t types
    void (*Dealloc)(node_t*);       //for deallocating node_t types
    
    void* (*AllocMem)(size_t size, uint64_t type);
    void (*FreeMem)(void* pointer);
    
    void (*InitList)(list_t* list);
    
    void (*AddHead)( list_t* list, node_t* node);
    void (*AddTail)(list_t* list, node_t* node);
    
    void (*Enqueue)(list_t* list,node_t* node);
    void (*EnqueueSize)(list_t* list,node_t* node);
    
    node_t* (*FindName)(list_t* list, char *name );
    node_t* (*ItemAtIndex)(list_t* list, uint64_t index);
    void (*Insert)(list_t* list, node_t* node, node_t* pred);
    void (*Remove)(list_t* list,node_t* node);
    
    node_t* (*RemHead)(list_t* list);
    node_t* (*RemTail)(list_t* list);
    
    void (*AddLibrary)(library_t* library);
    library_t* (*OpenLibrary)(char* name, uint64_t version);
    
    task_t* (*AddTask)(void* entry,uint32_t stackSize, int32_t priority);
    uint64_t (*Wait)(uint64_t signal);
    void (*Signal)(task_t* task, uint64_t signals);
    int32_t (*AllocSignal)(int32_t sigNum);
    void (*FreeSignal)(uint32_t sigNum);
    void (*SetTaskPri)(task_t* task, int32_t priority);
    void (*Reschedule)(void);
    void (*ReschedulePrivate)(void_ptr* link);
    void (*SignalPrivate)(registers_t* regs, task_t* task,uint64_t signals);
    void (*Forbid)(void);
    void (*Permit)(void);
    
    messagePort_t* (*CreatePort)(char* name);
    void (*DeletePort)(messagePort_t* port);
    messagePort_t* (*FindPort)(char* name);
    message_t* (*GetMessage)(messagePort_t* port);
    void (*PutMessage)(messagePort_t* port, message_t* message);
    void (*ReplyMessage)(message_t* message);
    void (*AddPort)(messagePort_t* port);
    void (*RemovePort)(messagePort_t* port);
    message_t* (*WaitPort)(messagePort_t* port);
    
    //Devices
    void (*AddDevice)(device_t* device);
    uint64_t (*OpenDevice)(char* name,uint32_t unitNumber,ioRequest_t* ioRequest,uint64_t flags);
    handler_t* (*OpenHandler)(char* name, uint64_t version);
    void (*CloseDevice)(ioRequest_t* ioRequest);
    uint64_t (*DeviceUnitCount)(char* name);
    ioRequest_t* (*CreateIORequest)(messagePort_t* replyPort, uint64_t size);
    void (*SendIO)(ioRequest_t* req);
    void (*DoIO)(ioRequest_t* req);
    void (*CheckIO)(ioRequest_t* req);
    
    task_t* thisTask;
    list_t deviceList;
    list_t libraryList;
    list_t portList;
    list_t taskReady;
    list_t taskWait;
    list_t taskSuspended;
    
} executive_t;

extern executive_t* executive;


extern uint8_t kickStartBootImage[];



void Lock(lock_t* lock);      // will block the task until the lock is free
void FreeLock(lock_t* lock);  // will relelase a lock.
bool TestLock(lock_t* lock);    // test but not set a lock, returns true if set
void DefragMem(void);


void InitMemory(void* startAddress, uint64_t size);




#endif /* memory_h */
