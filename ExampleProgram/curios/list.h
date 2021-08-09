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
#define MEM_TYPE_FREE   0   // Free, ready to be allocated
#define MEM_TYPE_ALLOC  1   // Allocated and in use
#define MEM_TYPE_PUBLIC 2   // Public/Shared between tasks

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

typedef struct {
    volatile bool isLocked;
    int64_t lockingTaskPri;
    void* lockingTask;
}lock_t;

typedef struct node_t node_t;

struct node_t{
    node_t* next;
    node_t* prev;
    node_t* nextContigious;
    int32_t priority;
    uint32_t flags;
    uint32_t type;          //memory type
    uint32_t nodeType;      //node type
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
