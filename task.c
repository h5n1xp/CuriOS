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


#include "cli.h"

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



int timerTask(){
    
    while(1){
       
        executive->Reschedule();
    }
    
    return 0;
}

void PrintFreeList(){

    executive->Forbid();
    
    node_t* node = executive->freeList.head;
    debug_write_string("\n--------------------------------------\n");
    while(node->next != NULL){
        
        debug_write_hex((uint32_t)node);debug_write_string(": ");
        debug_write_dec(node->size);debug_write_string(" bytes\n ");
        
        node = node->next;
    }
    
    
    executive->Permit();
    
}






int under(){
    

    
    int ballX = 40;
    int ballY = 40;
    int ballVX = 1;
    int ballVY = 1;
    

    
    OpenWindow(NULL,500,550,300,100,WINDOW_TITLEBAR | WINDOW_DRAGGABLE | WINDOW_DEPTH_GADGET," GFX Test");
    

    
    window_t* under = OpenWindow(NULL, 0, 400, 500, 120,WINDOW_TITLEBAR | WINDOW_DRAGGABLE | WINDOW_DEPTH_GADGET, "Under");
    Focus(under);
    
    SetScreenTitle(under,"A Random Test Program");
    

    
    while(1) {
        
            //animate one window
            DrawRectangle(under, ballX, ballY, 8, 8, under->backgroundColour);
            ballX += ballVX;
            ballY += ballVY;
        
            if(ballX>186 || ballX<5){ballVX = -ballVX;}
            if(ballY>107 || ballY<23){ballVY = -ballVY;}
        
            DrawRectangle(under, ballX, ballY, 8, 8, intuition.orange);

        
        WaitMS(20);
    }
    
    return 0;
}


int over(){
    
    int ballX = 40;
    int ballY = 40;
    int ballVX = 1;
    int ballVY = 1;
    
    
    intuition_t* intuibase =  (intuition_t*)executive->OpenLibrary("intuition.library",0);
    
    window_t* gfxTest = OpenWindow(NULL,650,550,320,200,WINDOW_TITLEBAR | WINDOW_DRAGGABLE | WINDOW_DEPTH_GADGET | WINDOW_CLOSE_GADGET,"GFX Drawing Test");
    
    SetScreenTitle(gfxTest,"Program draws Amiga Vector Images!");
    gfxTest->isBusy = true;
    
    intuibase->DrawRectangle(gfxTest,30,30,200,160,intuibase->white);
    intuibase->DrawLine(gfxTest,5,25,50,90,intuition.black);
    intuibase->DrawCircle(gfxTest,50,55,32,intuition.black, false);
    intuibase->FloodFill(gfxTest,50,55,intuition.orange);
    
    intuibase->DrawVectorImage(gfxTest,70,40,kickStartBootImage);
    
    
    window_t* under = OpenWindow(NULL, 600, 420, 200, 120,WINDOW_TITLEBAR | WINDOW_DRAGGABLE | WINDOW_DEPTH_GADGET, "Over");
    Focus(under);
    
    SetScreenTitle(under,"Yet another Random Test Program");

    
    
    while(1) {
        
            //animate one window
            DrawRectangle(under, ballX, ballY, 9, 9, under->backgroundColour);
            ballX += ballVX;
            ballY += ballVY;
        
            if(ballX>186 || ballX<6){ballVX = -ballVX;}
            if(ballY>106 || ballY<24){ballVY = -ballVY;}
        
            //DrawRectangle(under, ballX, ballY, 8, 8, intuition.orange);
            intuibase->DrawCircle(under,ballX+4,ballY+4,4,intuition.black, false);
            intuibase->FloodFill(under,ballX+4,ballY+4,intuition.orange);
        
        
        WaitMS(1);
    }
    
    return 0;
}



int receiverT(){
    
    int ballX = 40;
    int ballY = 40;
    int ballVX = 2;
    int ballVY = 2;
    
    intuition_t* intuibase =  (intuition_t*)executive->OpenLibrary("intuition.library",0);
    
    window_t* under = OpenWindow(NULL, 810, 420, 200, 120,WINDOW_TITLEBAR | WINDOW_DRAGGABLE | WINDOW_DEPTH_GADGET | WINDOW_RESIZABLE, "Message Receiver");
    WindowToFront(under);
    
    SetScreenTitle(under,"Test Program, which only updates when it receives a message from a counting task!");
    
    under->eventPort = executive->CreatePort("eventPort");
    
    messagePort_t* testPort = executive->CreatePort("Monkey");
    
    AddPort(testPort);
    

    
    while(1) {
        

        uint64_t sigRec = executive->Wait( 1 << testPort->sigNum | 1 << under->eventPort->sigNum);
        
        
        //resize window
        if(sigRec & 1 << under->eventPort->sigNum){

            intuitionEvent_t* event =(intuitionEvent_t*) GetMessage(under->eventPort);
            
            while(event != NULL){
                
                if(event->flags & WINDOW_EVENT_RESIZE){
                    ResizeWindow(under, event->window->w - event->mouseXrel, event->window->h - event->mouseYrel);

                    if((uint32_t)ballX > (under->innerW-14)){ballX = 10;}
                    if((uint32_t)ballY > (under->innerH-12)){ballY = 24;}
                    
                }
                
                executive->ReplyMessage((message_t*)event);
                event = (intuitionEvent_t*) GetMessage(under->eventPort);
            }
            
        }
        
        
        //animate one window
        DrawRectangle(under, ballX, ballY, 9, 9, under->backgroundColour);
        if(sigRec & 1 << testPort->sigNum){
            
            message_t* message = GetMessage(testPort);
            
            // debug_write_string("->");
            
            while(message != NULL){
            
                ballX += ballVX;
                ballY += ballVY;
                
                if((uint32_t)ballX >= under->innerW-10 || (uint32_t)ballX <= under->innerX+2){ballVX = -ballVX;}
                if((uint32_t)ballY >= under->innerH-10 || (uint32_t)ballY <= under->innerY+2){ballVY = -ballVY;}
                
                //executive->Dealloc((node_t*)message);
                executive->ReplyMessage(message);
                message = GetMessage(testPort);
                //debug_write_string("!");
            }
            //debug_write_string("<-\n");
        }
        //DrawRectangle(under, ballX, ballY, 8, 8, intuition.orange);
        intuibase->DrawCircle(under,ballX+4,ballY+4,4,intuition.black, false);
        intuibase->FloodFill(under,ballX+4,ballY+4,intuition.orange);
       
    }
    
    return 0;
}


int outputee(){
    
    //executive->thisTask->node.name = "No window";
     
    messagePort_t* rep = CreatePort("Reply here!");
    
    message_t* mess = NULL;
    
    
    messagePort_t* testPort = NULL;
    
    while(1){
        
                
        //debug_write_hex((uint32_t)&mess);debug_write_string(" -> ");
            
                if(testPort != NULL){
                    
                    mess = (message_t*)executive->Alloc(sizeof(message_t));
                    
                    if(mess != NULL){
                        
                        mess->replyPort = rep;
                        PutMessage(testPort,mess);
                        
                        
                    }else{
                        debug_write_string("No memory for message!\n");
                    }
                    
                }else{
                    testPort = FindPort("Monkey");
                    
                    if(testPort==NULL){
                        debug_write_string("Sender Task: Can't find port\n");
                    }
                }
                
       WaitMS(64);
    }
    
    return 0;
}


//*******************************************************************************************************************************************************************

//Should be the first function your task will call
void* TaskStartup(){
   
    void_ptr kernel;
    asm volatile("mov %%eax,%0" : "=r" ((void_ptr)kernel)::);
    
    return (void*)kernel;
}

void SetTaskPri(task_t* task,int32_t pri){
    
    task->node.priority = pri;
    
}

task_t* AddTask(void* entry,uint32_t stackSize,int32_t pri){

    
    uint32_t supervisorStackSize = 16384;
    
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
    frame->eflags = 0x3246; //or 582?
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
        asm volatile("cli");
    executive->thisTask->forbidCount += 1;
}

void Permit(void){
    executive->thisTask->forbidCount -= 1;
        asm volatile("sti");
}

static void signal_trap(registers_t* regs);
static void wait_trap(registers_t* regs);

void InitMultitasking(){
    
    register_interrupt_handler(48, signal_trap); //  fire int48 for immediate reschedule
    register_interrupt_handler(49, wait_trap);   //  int49 interrupt to wait a task.
    
    
    
    //Idle task is special it runs in ring 0, so it can halt the CPU
    task_t* task = AddTask(IdleTask,1024,-128); //really this should be the lowest possible priority
    task->node.name = "Idle Task";
    
    registers_t* regs = (registers_t*)(task->ssp - 4);
    regs->cs = 0x8;
    regs->ds = 0x10;
    regs->ss = 0x10;
    
    
    inputStruct.inputTask = AddTask(InputTaskEntry,65536,20);

    
    task = AddTask(CliEntry,4096*2,0);  //double stack for Boot task... it has a lot to do...
    task->node.name = "BootShell";

    task = AddTask(outputee,4096,0);
    task->node.name = "Message Sender";

     task = AddTask(over,4096,0);
     task->node.name = "Over Ball bounce";
    
    task = AddTask(receiverT,4096,10);
    task->node.name = "Message Receiver";
     
     //task = AddTask(under,4096,0);
     //task->node.name = "Under Ball Bounce";
     
     
     /*
     //Don't bother with these....

    

    
     task = AddTask(tockTask,4096,0);
     task->node.name = "cheese muffin";
    
     publicTask = AddTask(tockTask1,4096,0);
     publicTask->node.name = "Smoople";
  

     //
     task = addTask(tockTask6,4096,0);
     task->name = tockTaskName6;
     
     task = addTask(tockTask7,4096,0);
     task->name = tockTaskName7;
     */

    
    executive->thisTask=NULL;
    
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
