//
//  List.h
//  
//
//  Created by Matt Parsons on 25/10/2020.
//  Copyright © 2020 Matt Parsons. All rights reserved.
//

#ifndef list_h
#define list_h

#include "stdheaders.h"



/*----- Mem Types -----*/
#define ALLOC_FLAGS_FREE   0        // Free, ready to be allocated
#define ALLOC_FLAGS_ALLOC  1        // Allocated and in use
#define ALLOC_FLAGS_PUBLIC 2        // Public/Shared between tasks, if this bit is not set, then the node shouldn't be shared between tasks
                                    // Public/shared nodes are usually kept in lists owned by the operating system


/*----- Node Types -----*/
#define NODE_UNKNOWN             0
#define NODE_MEMORY              1
#define NODE_TASK                2
#define NODE_LIBRARY             3
#define NODE_DEVICE              4
#define NODE_MSGPORT             5
#define NODE_MESSAGE             6
#define NODE_BITMAP              7
#define NODE_WINDOW              8
#define NODE_GADGET              9
#define NODE_LIST               10
#define NODE_FONT               11
#define NODE_PALETTE            12
#define NODE_HANDLER            13
#define NODE_FILE_DESCRIPTOR    14
#define NODE_DOS_ENTRY          15
#define NODE_DATA_BLOCK         16
#define NODE_EXECUTABLE_SEGMENT 17
#define NODE_STRING             18

typedef struct {
    volatile bool isLocked;
    int64_t lockingTaskPri;
    void* lockingTask;
}lock_t;

typedef struct node_t node_t;


//
//Would it make sense for nodes to have checksums?
struct node_t{
    node_t* next;
    node_t* prev;
    node_t* nextContigious; // this is just address + size... we can probably deprecate it. it is probably only used by the coellese function
    int32_t priority;
    uint32_t flags;
    uint32_t allocFlags;    // Allocation type. For systems without hardwre MMU this will be important to enforce memory access privilages
    uint32_t type;      //node type 
    uint64_t size;
    char* name;
};

typedef struct{
    node_t node;
    node_t* head;
    node_t* tail;
    node_t* pred;
    uint64_t count;
    lock_t lock;
} list_t;

void InitList(list_t* list);

void AddHead( list_t* list, node_t* node);
void AddTail(list_t* list, node_t* node);

void Enqueue(list_t* list,node_t* node);
void EnqueueSize(list_t* list,node_t* node);

node_t* FindName(list_t* list, char *name );
node_t* ItemAtIndex(list_t* list, uint64_t index);

void Insert(list_t* list, node_t* node, node_t* pred);
void Remove(list_t* list,node_t* node);

node_t* RemHead(list_t* list);
node_t* RemTail(list_t* list);


#endif /* list_h */
