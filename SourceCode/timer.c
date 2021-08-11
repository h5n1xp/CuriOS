//
//  timer.c
//  
//
//  Created by Matt Parsons on 22/09/2020.
//
//

#include "timer.h"
#include "x86cpu_ports.h"
#include "interrupts.h"
#include "memory.h"

//#include "task.h"
#include "SystemLog.h"
#include "input.h"


uint32_t fakeVBLCountDown = 1;



typedef struct{
    node_t node;
    uint64_t timeOut;
    task_t* task;
    uint32_t sigNum;
}timerReq_t;


typedef struct{
    node_t node;
    task_t* task;
    uint64_t timeOut;
} simpleTimerNode_t;

list_t simpleTimer;


static void timer_callback(registers_t* regs){
    executive->ticks++;
    fakeVBLCountDown -=1;
    
    
    executive->elapsed -=1;
    

    
    /* ******************
       if(!TestLock(&simpleTimerLock)){
           for(int i=0; i<64;++i){
               //Process simple timer
               if(simpleTimers[i].task != NULL){
                
                   if(executive->ticks >= simpleTimers[i].timeOut){
            
                  
                       simpleTimers[i].timeOut += 0xFFFFF;
                       task_t* task = simpleTimers[i].task;
                       uint64_t sig = 1 << simpleTimers[i].sigNum;
        
                       //signal to run but do NOT reschedule.
                       task->signalReceived |= sig;
                   }
                
               }
    
        
           }
           
       }
    */
    
    
    
    
    if(fakeVBLCountDown == 0){
        fakeVBLCountDown = 15;  //approximately 60Hz which would be 16.67ms
        executive->SignalPrivate(regs, inputStruct.inputTask,2); //will cause an immediate reschedule
        //inputStruct.inputTask->signalReceived = 2; // will reschedule next quantum
    }else if(executive->elapsed<=0){
        
            executive->ReschedulePrivate(&regs->link);
    
    }else if(!TestLock(&simpleTimer.lock)){
        
        node_t* node = simpleTimer.head;
        
        
        //This SHOULD NOT BE IN THE INTERRUPT! But we dont have timer task at this time :-(
        while (node->next != NULL) {
        
            simpleTimerNode_t* t = (simpleTimerNode_t*) node;
        
            if(t->timeOut <= executive->ticks){
                t->timeOut += 0xFFFFF;
                t->task->signalReceived |= (1 << 4);    // will wake the task, the next OS Quantum.
            }
        
            node = node->next;
        }
    }
    
}

void InitTimer(uint32_t frequency){
    // Firstly, register our timer callback.
    register_interrupt_handler(IRQ0, timer_callback);
    
    // The value we send to the PIT is the value to divide it's input clock
    // (1193180 Hz) by, to get our required frequency. Important to note is
    // that the divisor must be small enough to fit into 16-bits.
    uint32_t divisor = 1193180 / frequency;
    
    // Send the command byte.
    outb(0x43, 0x36);
    
    // Divisor has to be sent byte-wise, so split here into upper/lower bytes.
    uint8_t l = (uint8_t)(divisor & 0xFF);
    uint8_t h = (uint8_t)( (divisor>>8) & 0xFF );
    
    // Send the frequency divisor.
    outb(0x40, l);
    outb(0x40, h);
    
    InitList(&simpleTimer);
    
}




void WaitMS(uint64_t time){
    
    if(TestLock(&simpleTimer.lock)){
      //  return;
    }
    
    
    //return;
    simpleTimerNode_t* node = (simpleTimerNode_t*)executive->Alloc(sizeof(simpleTimerNode_t));
    if(node == NULL){
        return;
    }
    node->task = executive->thisTask;
    
    Lock(&simpleTimer.lock);
    AddTail(&simpleTimer,(node_t*)node);
    node->timeOut = executive->ticks + time;
    FreeLock(&simpleTimer.lock);
    
    executive->Wait(1 << 4);
    
    Lock(&simpleTimer.lock);
    Remove(&simpleTimer,(node_t*)node);
    FreeLock(&simpleTimer.lock);
    
    executive->Dealloc((node_t*)node);
    
    return;
    
    /*
    Lock(&simpleTimerLock);

    for(int i =0;i<64;++i){
        
        if(simpleTimers[i].task == NULL){
            
            simpleTimers[i].task  = executive->thisTask;
            simpleTimers[i].sigNum = 4;//executive->AllocSignal(-1);
            simpleTimers[i].timeOut = executive->ticks + time;
            
            FreeLock(&simpleTimerLock);
            executive->Wait(1 << simpleTimers[i].sigNum);
            
            Lock(&simpleTimerLock);
            simpleTimers[i].task = NULL;    //timer fired,so invalidate it;
            FreeLock(&simpleTimerLock);
            
            return;
            
        }
        
    }
    FreeLock(&simpleTimerLock);
     */
}
