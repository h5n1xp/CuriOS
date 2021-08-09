//
//  memory.c
//
//  Created by Matt Parsons on 25/10/2020.
//  Copyright © 2020 Matt Parsons. All rights reserved.
//

#include "memory.h"
#include "SystemLog.h"

#include "timer.h"

executive_t* executive;

//Debugging function to be removed once the ELF/Binary executable Loader works
void Test(){
    debug_write_string("Executive Test Function Called by task ");
    debug_write_string(executive->thisTask->node.name);
    debug_putchar('\n');
    WaitMS(1);
}

//Classic Amiga boot image 
uint8_t kickStartBootImage[] = {255,1,35,11,58,11,58,33,113,33,113,11,125,11,136,22,136,94,127,94,127,56,64,56,62,54,53,54,52,56,45,56,45,65,35,72,35,11,254,2,37,69,255,1,33,72,33,10,126,10,138,22,138,95,86,95,86,100,82,108,78,113,74,116,68,125,60,129,60,140,10,140,10,109,9,109,9,81,13,75,20,69,21,65,25,58,30,55,33,54,33,54,30,56,26,58,22,65,21,69,14,75,10,81,10,108,11,109,11,139,40,139,40,118,48,118,52,114,52,95,50,92,50,82,65,69,65,57,62,55,59,55,62,58,62,65,61,66,54,66,51,63,42,70,30,76,18,85,18,84,30,75,26,74,23,71,26,73,30,74,33,72,255,1,50,61,52,54,60,55,61,58,61,65,54,65,50,61,255,1,51,92,51,82,66,69,66,57,125,57,125,94,52,94,51,90,255,1,60,11,111,11,111,32,60,32,60,11,255,1,96,14,107,14,107,28,96,28,96,14,254,3,62,31,255,1,98,15,105,15,105,27,98,27,98,15,254,2,99,26,255,1,47,57,50,57,50,59,47,63,47,57,255,1,41,139,41,119,48,119,53,114,53,105,57,107,65,107,65,109,69,114,73,114,73,116,67,125,59,128,59,139,41,139,255,1,53,95,53,100,58,97,53,95,255,1,57,98,53,100,53,95,74,95,64,105,63,105,65,103,60,98,57,98,255,1,78,95,85,95,85,100,81,108,78,112,73,113,70,113,67,109,67,106,78,95,255,1,68,106,68,109,70,112,72,112,76,111,77,108,73,105,68,106,255,1,54,104,62,106,64,103,60,99,57,99,54,101,54,104,255,1,126,11,137,22,137,94,254,1,34,11,254,1,59,11,254,1,97,15,254,1,106,27,254,1,112,15,254,1,126,94,254,1,75,96,254,1,46,57,255,255};

// a lock is a primative type, so needs to be included for anything that accesses memory


// This lock lowers the resting task to match the locking task - which seems more stable at the moment
inline void Lock(lock_t* lock){
    
    //debug_write_string("\nWants Lock ");
    //debug_write_string(executive->thisTask->node.name);

    if(__atomic_test_and_set(&lock->isLocked, __ATOMIC_ACQUIRE)){
        
        //to avoid a  priority inversion match the waiting task priority with the locking task.
        int64_t taskPri = executive->thisTask->node.priority;
        
       /*
        if(lock->lockingTask == executive->thisTask){
            return;
        }
        */
        
        if(lock->lockingTaskPri<taskPri){  //only match if the locking task is lower priority;
                executive->thisTask->node.priority = lock->lockingTaskPri;
        }


        while (__atomic_test_and_set(&lock->isLocked, __ATOMIC_ACQUIRE)) {
            //debug_write_string(" Waiting Lock: ");
            //task_t* temp = (task_t*)lock->lockingTask;
            //debug_write_string(temp->node.name);
            executive->Reschedule();
        }

        
        executive->thisTask->node.priority = taskPri;
    
    }
    
 
    lock->lockingTaskPri = executive->thisTask->node.priority;
    lock->lockingTask = executive->thisTask;
}
 
 inline void FreeLock(lock_t* lock){
 
 __atomic_clear(&lock->isLocked, __ATOMIC_RELEASE);
 
 }
 

/*
// This lock raises the locking task to match the requesting task - seems to cause random lock ups...
inline void Lock(lock_t* lock){
    
    
    if(__atomic_test_and_set(&lock->isLocked, __ATOMIC_ACQUIRE)){
        
        //to avoid a  priority inversion match the waiting task priority with the locking task.
        int64_t taskPri = executive->thisTask->node.priority;
        task_t* lockingTask = lock->lockingTask;
        
        //only match if the locking task is lower priority;
        if(taskPri > lockingTask->node.priority){

            executive->Forbid();
            Remove(&executive->taskReady,(node_t*)lockingTask);
            lockingTask->node.priority = taskPri;
            Enqueue(&executive->taskReady,(node_t*)lockingTask);
            executive->Permit();
            
        }
        
        while (__atomic_test_and_set(&lock->isLocked, __ATOMIC_ACQUIRE)) {
            executive->Reschedule();
        }
        
    }
    
    lock->lockingTask = executive->thisTask;
    lock->lockingTaskPri = executive->thisTask->node.priority;  // save locking task's normal priority
    
}




inline void FreeLock(lock_t* lock){

    executive->thisTask->node.priority = lock->lockingTaskPri; // restore locking task's normal priority
    
    __atomic_clear(&lock->isLocked, __ATOMIC_RELEASE);

}
*/


inline bool TestLock(lock_t* lock){
    
    if(__atomic_test_and_set(&lock->isLocked, __ATOMIC_ACQUIRE)){
        return true;
    }
    
    __atomic_clear(&lock->isLocked, __ATOMIC_RELEASE);
    return false;
}




//lock_t memLock;



node_t* KAlloc(size_t size){
    
    lock_t* memLock = &executive->freeList.lock;
    
    Lock(memLock);
    
    //add on memory header size;
    //size +=sizeof(node_t);        // removed since alloc should only be use to allocate OS objects
    
    //need to align on 8 byte boundary
    size += 7;
    size &= 0xFFFFFFFFFFFFFFF8;
    
    list_t* freelist = &executive->freeList;
    
    node_t* freeblock = freelist->head;

    
    do{
        //exact match
        if(freeblock->size == size){
            Remove(freelist, freeblock);
            freeblock->priority = 0;
            freeblock->flags = 0;
            freeblock->type = MEM_TYPE_ALLOC;
            freeblock->nodeType = NODE_UNKNOWN;
            freeblock->next = NULL;
            freeblock->prev = NULL;
            
            FreeLock(memLock);
            return freeblock;
        }
        
        if(freeblock->size > size + (sizeof(node_t) * 2 ) ){
            break;
        }
        
        freeblock = freeblock->next;
    }while(freeblock->next != NULL);
    

    //No block big enough
    if(freeblock->next == NULL){
        FreeLock(memLock);
        return NULL;
    }

           
           
    uint8_t* temp = (uint8_t*)freeblock;
    temp += freeblock->size;
    
    temp -= size;
    
    node_t* newBlock = (node_t*)temp;
    
    newBlock->size = size;
    newBlock->priority = 0;
    newBlock->flags = 0;
    newBlock->type = MEM_TYPE_ALLOC;
    newBlock->nodeType = NODE_UNKNOWN;
    newBlock->next = NULL;
    newBlock->prev = NULL;
    newBlock->nextContigious = freeblock->nextContigious;
    

    
    freeblock->size -= size;
    freeblock->nextContigious = newBlock;

    
    FreeLock(memLock);
    return newBlock;
    
}


void Coalesce(node_t* node){
    
    node_t* contig = node->nextContigious;
    
    if(contig == NULL){
        return;
    }
    
    if( (contig->type & MEM_TYPE_ALLOC) == MEM_TYPE_ALLOC){
        return;
    }
    
    //debug_write_string("!");
    
    Coalesce(contig);
    
    

    
    //lock_t* memLock = &executive->freeList.lock;
    //if(TestLock(memLock)){
    //    return;
    //}
    
    //Lock(memLock);
    
    //need a better way to lock memory
    executive->Forbid();
    Remove(&executive->freeList,contig);
    node->nextContigious = contig->nextContigious;
    node->size +=contig->size;
    executive->Permit();
    
    //FreeLock(memLock);
    
}


void DefragMem(){
    
     lock_t* memLock = &executive->freeList.lock;
     Lock(memLock);
    
    node_t* node = executive->freeList.head;
    
    while (node->next != NULL) {
        Coalesce(node);
        node = node->next;
    }
    
    FreeLock(memLock);
}

void KDealloc(node_t* node){
    
    lock_t* memLock = &executive->freeList.lock;
    
    Lock(memLock);
    
    node->type = MEM_TYPE_FREE;
    node->nodeType = NODE_MEMORY;
    node->priority = 0;
    node->flags = 0;
    
    Coalesce(node);
    
    list_t* freelist = &executive->freeList;
    
    node_t* pred = freelist->pred;
        

    if(pred->prev==NULL){
        AddHead(freelist, node);
    }
    
    while (pred->prev != NULL) {
        
        
        
        if(pred->size < node->size){
            
            Insert(freelist, node, pred);
            
            FreeLock(memLock);
            return;
            
        }
        
        
        
        pred = pred->prev;
    }
    
    AddHead(freelist, node);
    FreeLock(memLock);
}

void* AllocMem(size_t size, uint64_t type){

    // random check, to suppress compiler warning while we only have one type of ram.
    if(type == 68000){
        return NULL;
    }
    
    //add on memory header size;
    size +=sizeof(node_t);
    
    node_t* node = KAlloc(size);
    node->nodeType = NODE_DATA_BLOCK;
    
    //add this memory allocation to the calling task's memory list
    if(executive->thisTask != NULL){
        AddTail(&executive->thisTask->memoryList,node);
    }
    
    node +=1;
    
    
    return (void*)node;
    
}

void FreeMem(void* pointer){
    
    node_t* node = (node_t*)pointer;
    
    node -= 1;
    
    //remove this memory allocation from the calling task's memory list
    if(executive->thisTask != NULL){
        Remove(&executive->thisTask->memoryList,node);
    }
    
    KDealloc(node);
    
}

library_t* OpenLibrary(char* name,uint64_t version){

    library_t* library = (library_t*) FindName(&executive->libraryList,name);
    
    if(library == NULL){
        //debug_write_string("Can't find library\n");
        return NULL;
    }
    

    
    
    //need to check the verion number before returning;
    if(library->version < version){
        //versions don't matter at this time....
        //return NULL;
    }
    
    // Each Library's Open() function is allowed to return a new instance if it wants to do so
    // but it is up to that library to manage separate instances.
    //
    library_t* libInstance = library->Open(library);
    
    //debug_write_string("OpenLibrary Called\n");
    //debug_write_string(libInstance->node.name);
    //executive->debug_putchar(' ');
    //debug_write_hex((uint32_t)libInstance);
    //executive->debug_putchar('\n');
    
    return libInstance;
    
}


void AddLibrary(library_t* library){
    
    library->Init(library);
    
    AddTail(&executive->libraryList,(node_t*)library);
    
}

void AddDevice(device_t* device){
    
    device->library.Init(&device->library);
    
    /* // now isn't the correct time to check to see what units the device may have.
    if(device->unitList.count < 1 ){
        debug_write_string("Device Open failure: No units!!");
    }
    */
    
    AddTail(&executive->deviceList,(node_t*)device);
    
}



uint64_t OpenDevice(char* name,uint32_t unitNumber,ioRequest_t* ioRequest,uint64_t flags){
    
    node_t* node = FindName(&executive->deviceList,name);
    
    if(node==NULL){
        return DEVICE_ERROR_DEVICE_NOT_FOUND;
    }
    
    device_t* device = (device_t*)node;
    device->library.Open(&device->library);
    ioRequest->device = device;
    
    if(node==NULL){
        ioRequest->error = -1;
        return DEVICE_ERROR_DEVICE_NOT_FOUND;
    }

    
    if(device->unitList.count < (unitNumber+1) ){
        ioRequest->error = -1;
        return DEVICE_ERROR_UNIT_NOT_FOUND;
    }
    
    uint32_t index = 0;
    node = device->unitList.head;
    
    while(index < unitNumber){
        node = node->next;
        index +=1;
    }
    
    unit_t* unit =(unit_t*)node;
    unit->openCount += 1;
    
    if(flags == 0){
        //do nothing
    }
    

    ioRequest->unit = (unit_t*)node;
    ioRequest->error = 0;
    
    return DEVICE_NO_ERROR;
    
}

handler_t* OpenHandler(char* name, uint64_t version){
    
    //this is just te open library function which seacher the device list.
    handler_t* handler = (handler_t*) FindName(&executive->deviceList,name);
    
    
    if(handler == NULL){
        //debug_write_string("Can't find handler\n");
        return NULL;
    }
    //debug_write_string("found handler\n");

     
    //need to check the verion number before returning;
    if(handler->device.library.version < version){
        //versions don't matter at this time....
        //return NULL;
    }
    
    
    handler->device.library.Open(&handler->device.library);
    return handler;
    
}

void CloseDevice(ioRequest_t* ioRequest){
    
    device_t* device = ioRequest->device;
    device->library.Close((library_t*)device);
    
}

uint64_t DeviceUnitCount(char* name){
    
    node_t* node = FindName(&executive->deviceList,name);
    
    //Device not found
    if(node==NULL){
        return 0;
    }
    
    device_t* device = (device_t*)node;
    
    //No available units
    if(device->unitList.count < 1 ){
        return 0;
    }
    
    return device->unitList.count;
}

ioRequest_t* CreateIORequest(messagePort_t* replyPort,uint64_t size){
    ioRequest_t* req = (ioRequest_t*)executive->Alloc(size);
    req->message.replyPort = replyPort;
    return req;
}


void SendIO(ioRequest_t* req){
    req->device->BeginIO(req);
    
}

void DoIO(ioRequest_t* req){
    
    req->flags |= IOF_QUICK;
    req->device->BeginIO(req);
    
}




void dummyStub(void){
    //debug_write_string("Unimpelmented Function\n");
    return;
}

void InitMemory(void* startAddress, uint64_t size){
    
    executive = (executive_t*)startAddress;
    
    InitList(startAddress);
    executive_t* executive = startAddress;
    executive->memSize = size;
    

    node_t* freeblock = startAddress + sizeof(executive_t);
    freeblock->size = size - sizeof(executive_t);
    freeblock->type = MEM_TYPE_FREE;
    freeblock->nodeType = NODE_MEMORY;
    freeblock->nextContigious = NULL;
    AddHead(startAddress, freeblock);
    
    executive->quantum = 2;
    executive->elapsed = executive->quantum;
    
    //Debugging output functions
    executive->debug_write_string   = debug_write_string;
    executive->debug_write_hex      = debug_write_hex;
    executive->debug_write_dec      = debug_write_dec;
    executive->debug_backspace      = debug_backspace;
    executive->debug_putchar        = debug_putchar;
    
    executive->Alloc            = KAlloc;
    executive->Dealloc          = KDealloc;
    
    executive->AllocMem         = AllocMem;
    executive->FreeMem          = FreeMem;
    
    executive->InitList         = InitList;
    
    executive->AddHead          = AddHead;
    executive->AddTail          = AddTail;
    
    executive->Enqueue          = Enqueue;
    executive->EnqueueSize      = EnqueueSize;
    
    executive->FindName         = FindName;
    executive->ItemAtIndex      = ItemAtIndex;
    executive->Insert           = Insert;
    executive->Remove           = Remove;
    
    executive->RemHead          = RemHead;
    executive->RemTail          = RemTail;
    
    executive->AddLibrary       = AddLibrary;
    executive->OpenLibrary      = OpenLibrary;
    
    executive->CreatePort       = CreatePort;
    executive->DeletePort       = DeletePort;
    executive->FindPort         = FindPort;

    executive->GetMessage       = GetMessage;
    executive->PutMessage       = PutMessage;
    executive->ReplyMessage     = ReplyMessage;

    executive->AddPort          = AddPort;
    executive->RemovePort       = RemovePort;

    executive->WaitPort         = WaitPort;
    
    executive->Forbid           = dummyStub;
    executive->Permit           = dummyStub;
    
    //devices
    executive->AddDevice        = AddDevice;
    executive->OpenDevice       = OpenDevice;
    executive->OpenHandler      = OpenHandler;
    executive->CloseDevice      = CloseDevice;
    executive->DeviceUnitCount  = DeviceUnitCount;
    executive->SendIO           = SendIO;
    executive->DoIO             = DoIO;
    executive->CreateIORequest  = CreateIORequest;
    
    executive->SetIntVector     = register_interrupt_handler; //for system use only.
    
    executive->thisTask = NULL;
    InitList(&executive->deviceList);
    InitList(&executive->libraryList);
    InitList(&executive->portList);
    InitList(&executive->taskReady);
    InitList(&executive->taskWait);
    InitList(&executive->taskSuspended);

    return;
}
