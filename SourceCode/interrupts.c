//
//  interrupts.c
//  
//
//  Created by Matt Parsons on 22/09/2020.
//
//

#include "x86cpu_ports.h"
#include "interrupts.h"

#include "memory.h"
#include "task.h"
#include "SystemLog.h"
#include "intuition.h"

#include "string.h"


static char* trap_strs[] = {
    "Divide by zero error",
    "Debug/Fault",
    "Non-Maskable Interrupt",
    "Breakpoint",
    "Overflow",
    "BOUND range exceeded",
    "Invalid opcode",
    "Device not availible (Coprocessor Error)",
    "Double Fault",
    "Coprocessor segment overrun",
    "Bad TSS",
    "Segment not present",
    "Stack-segment fault",
    "General protection fault",
    "Page fault",
    "Out of Memory",    //Offically Intel call this "Reserved 15", I'm using it for Out of Memory
    "x87 FPU floating-point error",
    "Alignment check exception",
    "Machine check exception",
    "SIMD floating-point exception"
};

//Interrupts dispatching and handling code

isr_t interrupt_handlers[256];

void register_interrupt_handler(uint8_t n, isr_t handler){
    interrupt_handlers[n] = handler;
}

void isr_handler(registers_t regs){

    uint8_t int_no = regs.int_no & 0xFF;

    if(int_no<20){
        executive->elapsed = 200;   //Set to some stupidly high value to inhibit rescheduilng until the rescheduler is called in this interrupt
        task_t* task = executive->thisTask;
        executive->AddHead(&executive->taskSuspended,(node_t*)task); // move to the suspended list
        executive->thisTask = NULL;
        executive->ReschedulePrivate(&regs.link);   //schedule in next task
             
        //Task didn't crash it ended
        if(task->state == TASK_ENDED){
            
            //need to clean up the ended tasks from the suspended list somehow.
            return;
        }
        
        
        //If the window server has crashed we need a non GUI Guru
        //if(strcmp("Input Task",task->node.name)){
           
            graphics.DrawRect(&graphics.frameBuffer,0, 0, graphics.frameBuffer.width,graphics.frameBuffer.height, graphics.Colour(0,0,0,0xFF));
            graphics.DrawRect(&graphics.frameBuffer,0, 0, graphics.frameBuffer.width,100, graphics.Colour(255,0,0,0xFF));
            graphics.DrawRect(&graphics.frameBuffer,5, 5, graphics.frameBuffer.width-10,90, graphics.Colour(0,0,0,0xFF));
            
            graphics.RenderString(&graphics.frameBuffer,intuition.defaultFont,30,20,"Guru Meditiation",graphics.Colour(255,0,0,0xFF),graphics.Colour(0,0,0,0xFF));
            graphics.RenderString(&graphics.frameBuffer,intuition.defaultFont,300,20,trap_strs[int_no],graphics.Colour(255,0,0,0xFF),graphics.Colour(0,0,0,0xFF));
            graphics.RenderString(&graphics.frameBuffer,intuition.defaultFont,30,40,"Task:",graphics.Colour(255,0,0,0xFF),graphics.Colour(0,0,0,0xFF));
            graphics.RenderString(&graphics.frameBuffer,intuition.defaultFont,300,40,task->node.name,graphics.Colour(255,0,0,0xFF),graphics.Colour(0,0,0,0xFF));
        //}
        
                           
        //The task definitly crashed :(
        task->state = TASK_SUSPENDED;
                           
        debug_write_string("\nGuru Meditation! ");
        debug_write_string("Task: ");
        

        debug_write_string(task->node.name);
        
        debug_write_string(" - ");
        debug_write_string(trap_strs[int_no]);
        debug_write_string(" \n");
        



        debug_write_string(" Task Suspended!\n");
        intuition_t* intuibase =(intuition_t*)executive->OpenLibrary("intuition.library",0);
        window_t* req = intuibase->Request("Software Failure");
        intuibase->DrawString(req,18,30,"Guru Meditation:",intuibase->blue,intuibase->white);
        intuibase->DrawString(req,18,30+20,task->node.name,intuibase->blue,intuibase->white);
        intuibase->DrawString(req,18,30+40,trap_strs[int_no],intuibase->blue,intuibase->white);
        intuibase->Update();
        return;
    }

    
    //Custom ISR handler...
    if (interrupt_handlers[int_no] != 0){
        isr_t handler = interrupt_handlers[int_no];
        handler(&regs);
        return;
    }

    //terminal_writestring("\nUnhandled Interrupt: ");
    //terminal_write_dec(int_no);
    //terminal_writestring(" \n1>");
}

void irq_handler(registers_t regs){
    // Send an EOI (end of interrupt) signal to the PICs.
    // If this interrupt involved the slave.
    if (regs.int_no >= 40){
        // Send reset signal to slave.
        outb(0xA0, 0x20);
    }
    
    // Send reset signal to master. (As well as slave, if necessary).
    outb(0x20, 0x20);
    
    /*
     terminal_writestring("IRQ: ");
     terminal_write_dec(regs.int_no);
     terminal_writestring(" \n");
     */
    
    //Custom IRQ handler...
    if (interrupt_handlers[regs.int_no] != 0){
        isr_t handler = interrupt_handlers[regs.int_no];
        handler(&regs);
    }
    
    
}


