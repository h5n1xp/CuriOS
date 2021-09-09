//
//  memory.h
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

#define EXEC_REQUEST_NOP      0
#define EXEC_REQUEST_ADD_TASK 1
#define EXEC_REQUEST_REM_TASK 2
#define EXEC_REQUEST_SHUTDOWN 3

typedef struct{
    message_t message;
    uint32_t request;
    task_t* caller;
    void* data;
    uint32_t param1;
    uint32_t param2;
} executiveRequest_t;

typedef struct{
    list_t freeList;
    uint64_t memSize;
    uint64_t ticks;
    uint64_t quantum;
    volatile int64_t elapsed;
    
    uint64_t allocationTotal;   // it might make sense to audit the total number of allocations vs deallocations at some point
    uint64_t deallocationTotal;
    
    //Debugging output functions
    void (*debug_show)(void);
    void (*debug_hide)(void);
    void (*debug_write_string)(char* str);
    void (*debug_write_hex)(uint32_t n);
    void (*debug_write_dec)(uint32_t n);
    void (*debug_backspace)(void);
    void (*debug_putchar)(char c);
    
    //List functions
    node_t* (*Alloc)(size_t, uint64_t attributes);       //For allocating node_t types, it is the responsibility of the calling task to keep a record of the allocation
    void (*Dealloc)(node_t*);       //for deallocating node_t types, it is the responsibility of the calling task to keep a record of the deallocation
    
    void* (*AllocMem)(size_t size, uint64_t type); // the allocation is recorded in the task structure of the allocating context.
    void (*FreeMem)(void* pointer); // memory is returned to the free list.
    
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
    
    void (*Lock)(lock_t* lock);      // will block the task until the lock is free
    void (*FreeLock)(lock_t* lock);  // will relelase a lock.
    bool (*TestLock)(lock_t* lock);  // test but not set a lock, returns true if set
    
    //Libraries
    void (*AddLibrary)(library_t* library);
    library_t* (*OpenLibrary)(char* name, uint64_t version);
    void (*CloseLibrary)(library_t* library);
    
    //Tasks
    task_t* (*CreateTask)(char* name, int32_t pri, void* entry, uint32_t stackSize);
    void (*AddTask)(task_t* task);
    void (*RemTask)(task_t* task);
    task_t* (*FindTask)(char* name);
    uint64_t (*Wait)(uint64_t signal);
    void (*Signal)(task_t* task, uint64_t signals);
    int32_t (*AllocSignal)(int32_t sigNum);
    void (*FreeSignal)(uint32_t sigNum);
    void (*SetTaskPri)(task_t* task, int32_t priority);
    void (*Reschedule)(void);
    void (*Forbid)(void);
    void (*Permit)(void);
    
    //Message Ports
    messagePort_t* (*CreatePort)(char* name);
    void (*DeletePort)(messagePort_t* port);
    messagePort_t* (*FindPort)(char* name);
    void* (*CreateMessage)(uint64_t size, messagePort_t* replyPort);
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
    void (*CloseHandler)(handler_t* handler);
    uint64_t (*DeviceUnitCount)(char* name);
    ioRequest_t* (*CreateIORequest)(messagePort_t* replyPort, uint64_t size);
    void (*SendIO)(ioRequest_t* req);
    void (*DoIO)(ioRequest_t* req);
    void (*CheckIO)(ioRequest_t* req);
    
    //Interrupts
    void (*SetIntVector)(uint8_t interruptNumber, isr_t handler); //set the interrupt handler callback code.
    
    
    
    //Private Area, these do not need to be in the public interface strucutre.
    void (*SetTaskPriPrivate)(task_t* task, int32_t priority);
    void (*AddTaskPrivate)(task_t* task);
    void (*ReschedulePrivate)(void_ptr* link);
    
    //Data Area - This can change, so must only be accessed via accessor functions by user tasks
    
    task_t* kernelTask;
    task_t* thisTask;
    list_t deviceList;
    list_t libraryList;
    list_t portList;
    list_t taskReady;
    list_t taskWait;
    list_t taskSuspended;
    list_t taskList;        //For Task/thread auditing
    messagePort_t* executivePort;
} executive_t;

extern executive_t* executive;


extern uint8_t kickStartBootImage[];



void Lock(lock_t* lock);      // will block the task until the lock is free
void FreeLock(lock_t* lock);  // will relelase a lock.
bool TestLock(lock_t* lock);  // test but not set a lock, returns true if set
void DefragMem(void);


void InitMemory(void* startAddress, uint64_t size);




#endif /* memory_h */
