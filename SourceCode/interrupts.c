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
    "Reserved 15",
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
        
        debug_write_string("\nGuru Meditation: ");
        debug_write_string("Task - ");
        
        task_t* task = executive->thisTask;
        debug_write_string(task->node.name);
        
        //terminal_write_hex((uint32_t)kernel->runningTask);
        debug_write_string(" ");
        debug_write_string(trap_strs[int_no]);
        debug_write_string(" \n");
        
        //suspend faulty task
        //executive->SuspendTask(executive->thisTask);
        executive->AddHead(&executive->taskSuspended,(node_t*)executive->thisTask);
        executive->thisTask = NULL;
        executive->ReschedulePrivate(&regs.link);
        debug_write_string(" Task Suspended!\n");
        intuition_t* intuibase =(intuition_t*)executive->OpenLibrary("intuition.library",0);
        window_t* req = intuibase->Request("Software Failure");
        intuibase->DrawString(req,18,30,"Guru Meditation:",intuibase->blue,intuibase->white);
        intuibase->DrawString(req,18,30+20,task->node.name,intuibase->blue,intuibase->white);
        intuibase->DrawString(req,18,30+40,trap_strs[int_no],intuibase->blue,intuibase->white);
        intuibase->Update();
         
        //Halt the system.
        //terminal_writestring(" !System Halted!\n");
        //asm volatile("cli");
        //asm volatile("hlt");
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


