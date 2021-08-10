//
//  task.c
//
//  Created by Matt Parsons on 02/11/2020.
//  Copyright © 2020 Matt Parsons. All rights reserved.
//

#include "task.h"
#include "memory.h"
#include "interrupts.h"
#include "descriptor_tables.h"

#include "input.h"

extern tss_entry_t tss_entry;


#include "SystemLog.h"
#include "intuition.h"
#include "ports.h"
#include "timer.h"


//#include "cli.h"

int IdleTask(){
    
    uint32_t time = 0;
    
    while(1){
       // debug_write_dec(time);
        for(int i=0;i<1000;++i){
            asm("hlt");
        }
        DefragMem();
        //debug_write_string("s");

        time +=1;
    }
    
    return 0;
}







//*******************************************************************************************************************************************************************



 
void SetTaskPri(task_t* task,int32_t pri){
    
    task->node.priority = pri;
    
}

task_t* AddTask(void* entry,uint32_t stackSize,int32_t pri){

    
    uint32_t supervisorStackSize = 4096; //16384;
    
    //minium stack is 4k
    if(stackSize<4096){
        stackSize = 4096;
    }
    
    //allocate enough memory for the stack and the task structure
    uint32_t size = supervisorStackSize + stackSize + sizeof(task_t);
    task_t* task = (task_t*)executive->Alloc(size);
    task->node.nodeType = NODE_TASK;
   
    //set up kernel stack
    uint32_t frameSize = sizeof(registers_t);
    
    task->ssp_top  = (uint32_t)task;
    task->ssp_top += size - stackSize;
    task->ssp_top &= 0xFFFFFFF0;            // 16byte alignment
    task->ssp = task->ssp_top - frameSize;  // 16 registers at 4 bytes each... this will need to change for 64bit.
    
    task->usp_top = (uint32_t)task;
    task->usp_top += size;
    task->usp_top &= 0xFFFFFFF0;            // 16byte alignment
    task->usp_top -= 16; //leave 16 bytes free at the top of the stack}

    registers_t* frame =(registers_t*)task->ssp;
    
    frame->ss = 0x23;               // 0x23 is the user mode data segment, where the stack is
    frame->useresp = task->usp_top;
    frame->eflags = 0x3246;         //or 582?
    frame->cs = 0x1B;                //0x8 is supervisor mode code segment, 0x1b is user mode code segment
    frame->eip = (uint32_t)entry;
    frame->ds = 0x23;
    
    //frame->esp = task->ssp + 0x24; //just for completeness
    task->ssp +=4;  // initial esp needs to point to 4 bytes higher in mem than the stack frame.
    frame->link = task->ssp;
    frame->eax = (void_ptr)executive; //upon startup every task has a pointer to the kernel interface for now.


    task->parent = executive->thisTask;
    task->signalAlloc = 65535;// first 16 signals are for Operating system use;
    task->signalWait = 0;
    task->signalReceived = 0;
    task->forbidCount = 0;
    task->state = TASK_READY;
    
    //set up message port
    
    
    SetTaskPri(task,pri);
    
    executive->Forbid();
    Enqueue(&executive->taskReady,(node_t*)task);
    executive->Permit();
    
    return task;
}

void FreeSignal(uint32_t sigNum){

    if(sigNum < 16){
        return;
    }
    
    task_t* task = executive->thisTask;
    
    task->signalAlloc ^= (1 << sigNum);
    
}

int32_t AllocSignal(int32_t sigNum){
    
    task_t* task = executive->thisTask;
    
    int32_t sig = 0;
    
    if(sigNum == -1){
        
        while (task->signalAlloc & (1 << sig) ){
            sig +=1;
            
            if(sig == 64){
                return -1;
            }
        }
        
        task->signalAlloc |= (1 << sig);
        return sig;
        
    }
    
    if( task->signalAlloc & (1 << sigNum) ){
        return -1;
    }
    
    task->signalAlloc |= (1 << sigNum);
    return sigNum;
    
}

void Signal(task_t* task,uint64_t signals){   //signal a task
    
    
    task->signalReceived |= signals;    //save signals
    executive->Reschedule();

     
}



uint64_t Wait(uint64_t signal){               //wait for a signal
 
    task_t* task = executive->thisTask;
    task->signalWait |= signal;
    
    //only wait if no signals are present
    if( (task->signalReceived & task->signalWait) == 0){
        //wait trap - saving a task state can only happen in an interrupt, so some of the code must be executed there.
        asm volatile("int $49");
    }
    
    //returning from a signal
    
    uint64_t sig = task->signalReceived & task->signalWait; // which signals woke the task?
    task->signalReceived ^= sig; // clear the waking signals
    
    
    return sig;
}

bool schedulerLock = false;

void ReschedulePrivate(void_ptr* link){

    if(__atomic_test_and_set(&schedulerLock, __ATOMIC_ACQUIRE)){
        return;
    }
    
    executive->elapsed = executive->quantum;
    
    //check all waiting tasks to confirm if they have recieved a waking signal
    node_t* node = executive->taskWait.head;
    
    while(node->next != NULL){
        
        task_t* check = (task_t*)node;
        
        if(check->signalReceived & check->signalWait){
            Remove(&executive->taskWait,(node_t*)check);
            Enqueue(&executive->taskReady,(node_t*)check);
            check->state = TASK_READY;
        }
        
        node = node->next;
    }
    
    
    
    if(executive->thisTask==NULL){
        task_t* firstTask = (task_t*)RemHead(&executive->taskReady);
        executive->thisTask = firstTask;
        executive->thisTask->state = TASK_RUNNING;
        *link = firstTask->ssp;
        tss_entry.esp0 = firstTask->ssp_top;

        __atomic_clear(&schedulerLock, __ATOMIC_RELEASE);
        return;
    }
    

    
    if(executive->taskReady.count > 0 && executive->thisTask->forbidCount < 1){ //
        
        //need to check head node priority here... and only continue if it is higher or equal
        
        task_t* t = (task_t*)executive->taskReady.head;
        
        if(t->node.priority < executive->thisTask->node.priority){
            
            __atomic_clear(&schedulerLock, __ATOMIC_RELEASE);
            return;
        }
        
        task_t* nextTask = (task_t*)RemHead(&executive->taskReady);
        
        
        //save current task state... just the stack for now
        task_t* task = executive->thisTask;
        task->ssp = *link;
        task->state = TASK_READY;
        
        //retire current task
        Enqueue(&executive->taskReady,(node_t*)task);
        
        //context switch
        executive->thisTask = nextTask;
        nextTask->state = TASK_RUNNING;
        
        tss_entry.esp0 = nextTask->ssp_top;
        *link = nextTask->ssp;
        
    }

    __atomic_clear(&schedulerLock, __ATOMIC_RELEASE);
}


void Reschedule(){
    asm volatile("int $48");
}


void SignalPrivate(registers_t* regs, task_t* task,uint64_t signals){   //signal a task
    
    task->signalReceived |= signals;    //save signals
    ReschedulePrivate(&regs->link);

}

void Forbid(void){
        //asm volatile("cli");
    executive->thisTask->forbidCount += 1;
}

void Permit(void){
    executive->thisTask->forbidCount -= 1;
       // asm volatile("sti");
}

static void signal_trap(registers_t* regs);
static void wait_trap(registers_t* regs);

void InitMultitasking(){
    //**************************************************************
    // Write the address of the executive to address: 0x100000 (to top of the first megabyte)
    // This address isn't used for anything once the multiboot header has been read
    // So we are using it as the sysbase
    // proper executables with a .executive section don't need this
    
    //uint32_t* sysbase  = (uint32_t*)0x100000;
    //*sysbase = (uint32_t)executive;
    
    //debug_write_hex((uint32_t)executive);debug_putchar('\n');

    
    register_interrupt_handler(48, signal_trap); //  fire int48 for immediate reschedule
    register_interrupt_handler(49, wait_trap);   //  int49 interrupt to wait a task.
    
    
    
    //Idle task is special it runs in ring 0, so it can halt the CPU
    task_t* task = AddTask(IdleTask,1024,-128); //really this should be the lowest possible priority
    task->node.name = "Idle Task";
    
    registers_t* regs = (registers_t*)(task->ssp - 4);
    regs->cs = 0x8;
    regs->ds = 0x10;
    regs->ss = 0x10;
    
    
    inputStruct.inputTask = AddTask(InputTaskEntry,4096,20);
    
    
    executive->thisTask          = NULL;
    
    executive->AddTask           = AddTask;
    executive->Wait              = Wait;
    executive->Signal            = Signal;
    executive->AllocSignal       = AllocSignal;
    executive->FreeSignal        = FreeSignal;
    executive->SetTaskPri        = SetTaskPri;
    executive->Reschedule        = Reschedule;
    executive->ReschedulePrivate = ReschedulePrivate;
    executive->SignalPrivate     = SignalPrivate;
    executive->Forbid            = Forbid;
    executive->Permit            = Permit;
}







//Signal trap, this interupt causes an immediate reschedule
static void signal_trap(registers_t* regs){
    ReschedulePrivate(&regs->link);
}


//wait trap, this interupt saves task state, and then reschedules
static void wait_trap(registers_t* regs){
    
    if(__atomic_test_and_set(&schedulerLock, __ATOMIC_ACQUIRE)){
        return;
    }
    
    //add running task to waiting list
    AddHead(&executive->taskWait,(node_t*)executive->thisTask);
    
    //save current task state... just the stack for now
    task_t* task = executive->thisTask;
    task->ssp = regs->link;
    task->state = TASK_WAITING;
    
    //Clear the Forbid count
    executive->thisTask->forbidCount = 0;
    
    __atomic_clear(&schedulerLock, __ATOMIC_RELEASE);
    
    executive->thisTask = NULL;
    ReschedulePrivate(&regs->link);
    
}
