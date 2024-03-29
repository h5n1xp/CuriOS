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
        
        graphics.DrawRect(&graphics.frameBuffer, graphics.frameBuffer.width-25,5,10,10,graphics.Colour(0,0,0,0xFF));
        graphics.DrawRect(&graphics.frameBuffer, graphics.frameBuffer.width-24,6,8,8,graphics.Colour(0,0,255,0xFF));
        
       // debug_write_dec(time);
        for(int i=0;i<1000;++i){
            asm("hlt");
        }
        DefragMem();
        //debug_write_string("s");
        time +=1;
        //graphics.DrawRect(&graphics.frameBuffer, 800,0,50,50,graphics.Colour(0,0,0,0xFF));

        
        
        
        graphics.DrawRect(&graphics.frameBuffer, graphics.frameBuffer.width-25,5,10,10,graphics.Colour(0,0,0,0xFF));
        graphics.DrawRect(&graphics.frameBuffer, graphics.frameBuffer.width-24,6,8,8,graphics.Colour(255,255,255,0xFF));
        
        // debug_write_dec(time);
         for(int i=0;i<1000;++i){
             asm("hlt");
         }
         DefragMem();
         //debug_write_string("s");
         time +=1;
         //graphics.DrawRect(&graphics.frameBuffer, 800,0,50,50,graphics.Colour(0,0,0,0xFF));
    }
    
    return 0;
}







void HiPriTask(){
    
    while(1){
        

        graphics.DrawRect(&graphics.frameBuffer, graphics.frameBuffer.width-15,5,10,10,graphics.Colour(0,0,0,0xFF));
        graphics.DrawRect(&graphics.frameBuffer, graphics.frameBuffer.width-14,6,8,8,graphics.Colour(255,127,0,0xFF));
        WaitMS(500);

        graphics.DrawRect(&graphics.frameBuffer, graphics.frameBuffer.width-15,5,10,10,graphics.Colour(0,0,0,0xFF));
        graphics.DrawRect(&graphics.frameBuffer, graphics.frameBuffer.width-14,6,8,8,graphics.Colour(0,0,0,0xFF));
        WaitMS(500);
    }
    
    
}




//*******************************************************************************************************************************************************************



 
void SetTaskPri(task_t* task,int32_t pri){
    
    if(pri > 0){pri = 0;}   //no user task may be above 0
    task->node.priority = pri;
}

void SetTaskPriPrivate(task_t* task,int32_t pri){
    
    task->node.priority = pri;
    
}

void AddTask(task_t* task){
   // executive->Forbid();
   // Enqueue(&executive->taskReady,(node_t*)task);
   // executive->Permit();
    
    executiveRequest_t* req = (executiveRequest_t*)executive->Alloc(sizeof(executiveRequest_t),0);
    
    req->message.replyPort = NULL;
    req->caller = executive->thisTask;
    req->request = EXEC_REQUEST_ADD_TASK;
    req->data = task;
    
    executive->PutMessage(executive->executivePort,(message_t*)req);
    
}

void AddTaskPrivate(task_t* task){
    
    taskListNode_t* tNode = NULL;
    
    executive->Enqueue(&executive->taskReady,(node_t*)task);
    
    //Keep a record every task added to the system;
    tNode = (taskListNode_t*)executive->Alloc(sizeof(taskListNode_t),0);
    tNode->task = task;
    executive->AddTail(&executive->taskList,(node_t*)tNode);
    
}


task_t* CreateTask(char* name, int32_t pri, void* entry, uint32_t stackSize){
   
    uint32_t supervisorStackSize = 4096;
    
    //minium stack is 4k
    if(stackSize<4096){
        stackSize = 4096;
    }
    
    //allocate enough memory for the stack and the task structure
    uint32_t size = supervisorStackSize + stackSize + sizeof(task_t);
    uint8_t* buffer = (uint8_t*)executive->Alloc(size,0);
    
    //Clear the buffer(starting at the bottom of the node
    //for(int i=sizeof(node_t); i<size; ++i){
    //    buffer[i] = 0;
    //}
    
    task_t* task = (task_t*)buffer;
    
    task->node.name = name;
    task->node.type = NODE_TASK;
   
    //debug_write_string("Exec - CreateTask:"); debug_write_string(task->node.name);debug_putchar('\n');
    
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
    
    task->ssp +=4;  // initial esp needs to point to 4 bytes higher in mem than the stack frame.
    frame->link = task->ssp;


    task->affinity = -1;        // Any CPU will do
    task->type = 0;             // No CPU types yet
    task->state = TASK_READY;
    task->guru = GURU_MEDITATION_NO_ERROR;  // when something goes wrong an error code can be saved in the task structure.
    task->parent = executive->thisTask;
    
    messagePort_t* port = (messagePort_t*) executive->Alloc(sizeof(messagePort_t),0);
    port->node.name = "Rendezvous";
    port->node.type = NODE_MSGPORT;
    port->owner = task;
    port->sigNum = 15;  //uses one of the Operating System signals
    executive->InitList(&port->messageList);
    task->rendezvousPort = port;
    
    task->dosPort = NULL;       // Noy all task need disk access
    task->dosError = 0;
    task->progdir = NULL;
    executive->InitList(&task->memoryList);
    task->forbidCount = 0;
    task->signalAlloc = 65535;// first 16 signals are for Operating system use;
    task->signalWait = 0;
    task->signalReceived = 0;



    
    //set up message port
    
    
    SetTaskPri(task,pri);
    
    return task;
    
}

void RemTask(task_t* task){
    
    //debug_write_string("Need to implement the RemTask function!\n");
    //debug_write_string(task->node.name);debug_write_string(" wants to end\n");
    
    executiveRequest_t* req = (executiveRequest_t*)executive->Alloc(sizeof(executiveRequest_t),0);
    
    req->message.replyPort = NULL;
    req->caller = executive->thisTask;
    req->request = EXEC_REQUEST_REM_TASK;
    
    if(task == NULL){
        
        task = executive->thisTask;
        
        req->data = task;
        executive->PutMessage(executive->executivePort,(message_t*)req);
        executive->Wait(1<<15); //Put task to sleep, this should never return as the executive will kill it
        
    }else{
        
        req->data = task;
        executive->PutMessage(executive->executivePort,(message_t*)req);
        
    }
    
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
        
        //debug_write_string(task->node.name);debug_write_string(" Signal: "); debug_write_dec(sig);debug_putchar('\n');
        
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
 
    uint64_t temp = executive->elapsed;
    executive->elapsed = 200;    //force the rescheduler to not reschedule but putting some stupid high value
    
    task_t* task = executive->thisTask;
    task->signalWait |= signal;
    
    //only wait if no signals are present
    if( (task->signalReceived & task->signalWait) == 0){
        //wait trap - saving a task state can only happen in an interrupt, so some of the code must be executed there.
        asm volatile("int $49");
    }

    //returning from a signal
    executive->elapsed = temp;    //continue the task's time slice
    uint64_t sig = task->signalReceived & task->signalWait; // which signals woke the task?
    task->signalReceived ^= sig; // clear the waking signals
    
    
    return sig;
}



void ReschedulePrivate(void_ptr* link){
    
    //check all waiting tasks to confirm if they have recieved a waking signal
    node_t* node = executive->taskWait.head;
    
    while(node->next != NULL){

        task_t* check = (task_t*)node;
        
        //Here the reserved task signals will be checked for things like ABORT, so we can kill a task etc
        
        if(check->signalReceived & check->signalWait){
            
            node = node->next;
            
            Remove(&executive->taskWait,(node_t*)check);
            Enqueue(&executive->taskReady,(node_t*)check);
            check->state = TASK_READY;
        }else{
            node = node->next;
        }
    }
    
    
    if(executive->thisTask==NULL){
        task_t* firstTask = (task_t*)RemHead(&executive->taskReady);
        executive->thisTask = firstTask;
        executive->thisTask->state = TASK_RUNNING;
        *link = firstTask->ssp;
        tss_entry.esp0 = firstTask->ssp_top;

        executive->elapsed = executive->quantum;
        return;
    }

    
   // graphics.RenderString(&graphics.frameBuffer,topazOld_font,800,80,"                ",graphics.Colour(255,0,0,255),graphics.Colour(0,0,0,255));
   // graphics.RenderString(&graphics.frameBuffer,topazOld_font,800,80,"-> to reschedule",graphics.Colour(0,0,255,255),graphics.Colour(0,0,0,255));

    
    if(executive->taskReady.count > 0 && executive->thisTask->forbidCount < 1){
        

        //need to check head node priority here... and only continue if it is higher or equal
        task_t* t = (task_t*)executive->taskReady.head;
        
        if(t->node.priority < executive->thisTask->node.priority){
            executive->elapsed = executive->quantum;
            return;
        }
        
        task_t* nextTask = (task_t*)RemHead(&executive->taskReady);
        
        //Ready list becomes corrupted do to this function being called twice in a row?! - NO LONGER NEEDED AS THE WAITING -> READY CODE HAS BEEN FIXED
        //if(nextTask->node.type != NODE_TASK){
        //    //graphics.RenderString(&graphics.frameBuffer,topazOld_font,600,60,"READY LIST CORRUPT!",graphics.Colour(255,255,255,255),graphics.Colour(65,65,65,255));
        //    debug_write_string("Ready list corrupt! ->");
        //    executive->elapsed = executive->quantum;
        //    return;
        //}
        
        //save current task state... just the stack for now
        task_t* task = executive->thisTask;
        task->ssp = *link;
        task->state = TASK_READY;
        
        //retire current task
        Enqueue(&executive->taskReady,(node_t*)task);
        //graphics.RenderString(&graphics.frameBuffer,topazOld_font,500,60,task->node.name,graphics.Colour(255,255,255,255),graphics.Colour(65,65,65,255));
        
        //context switch
        executive->thisTask = nextTask;
        nextTask->state = TASK_RUNNING;
        
        tss_entry.esp0 = nextTask->ssp_top;
        *link = nextTask->ssp;
        
    }
    
    executive->elapsed = executive->quantum;

}


void Reschedule(){
    executive->elapsed = 0; //trigger a reschedule
    //asm volatile("int $48");
}


/*
//This function should probably be removed
void SignalPrivate(registers_t* regs, task_t* task,uint64_t signals){   //signal a task
    
    task->signalReceived |= signals;    //save signals
    //ReschedulePrivate(&regs->link);
    executive->elapsed = 0;
}
*/

void Forbid(void){
        //asm volatile("cli");
    executive->thisTask->forbidCount += 1;
}

void Permit(void){
    executive->thisTask->forbidCount -= 1;
       // asm volatile("sti");
}

task_t* FindTask(char* name){
    executive->Forbid();
    
    if(name==NULL){
        executive->Permit();
        return executive->thisTask;
    }
    
    task_t* task = (task_t*) FindName(&executive->taskReady,name);
    
    if(task==NULL){
        task = (task_t*) FindName(&executive->taskWait,name);
    }
    
    if(task==NULL){
        task = (task_t*) FindName(&executive->taskSuspended,name);
    }
    
    executive->Permit();
    return task;
}

//static void signal_trap(registers_t* regs); // should probably be removed
static void wait_trap(registers_t* regs);

void InitMultitasking(){
    
    //debug_write_hex((uint32_t)executive);debug_putchar('\n');

    
    //register_interrupt_handler(48, signal_trap); //  fire int48 for immediate reschedule... should probably be removed
    register_interrupt_handler(49, wait_trap);   //  int49 interrupt to wait a task.
    
    
    
    //Idle task is special it runs in ring 0, so it can halt the CPU
    task_t* task = CreateTask("Idle Task",-128,IdleTask,1024); //this should be the lowest possible priority
 
    // This code sets the task to operate in supervisor mode
    registers_t* regs = (registers_t*)(task->ssp - 4);
    regs->cs = 0x8;
    regs->ds = 0x10;
    regs->ss = 0x10;
    
    AddTaskPrivate(task);
    
    
    //This shouldn't be started here! This is the intuition window server task, intuition should start it.
    //inputStruct.inputTask = AddTask(InputTaskEntry,4096,20);
    
    //To Test out task priorities
    task = CreateTask("HiPriTask",30,HiPriTask,4096);
    SetTaskPriPrivate(task,30);
    AddTaskPrivate(task);
    
    
    
    executive->thisTask          = NULL;
    
    executive->CreateTask        = CreateTask;
    executive->AddTask           = AddTask;
    executive->RemTask           = RemTask;
    executive->FindTask          = FindTask;
    executive->Wait              = Wait;
    executive->Signal            = Signal;
    executive->AllocSignal       = AllocSignal;
    executive->FreeSignal        = FreeSignal;
    executive->SetTaskPri        = SetTaskPri;
    executive->Reschedule        = Reschedule;
    executive->Forbid            = Forbid;
    executive->Permit            = Permit;
    
    executive->SetTaskPriPrivate = SetTaskPriPrivate;
    executive->ReschedulePrivate = ReschedulePrivate;
    executive->AddTaskPrivate    = AddTaskPrivate;
    
}






/*
//Signal trap, this interupt causes an immediate reschedule... this should probably be removed...
static void signal_trap(registers_t* regs){
    ReschedulePrivate(&regs->link);
    debug_write_string("Signal Trap!\n");
}
*/

//wait trap, this interupt saves task state, and then reschedules
static void wait_trap(registers_t* regs){


    //save current task state... just the stack for now
    task_t* task = executive->thisTask;
    executive->thisTask = NULL;

    
    task->ssp = regs->link;
    task->state = TASK_WAITING;
    task->forbidCount = 0;     //Clear the Forbid count
    
    //add running task to waiting list
    AddHead(&executive->taskWait,(node_t*)task);
    
    ReschedulePrivate(&regs->link);
    
}
