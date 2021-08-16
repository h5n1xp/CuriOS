#include "stdheaders.h"
#include "multiboot.h"


/* Check if the compiler thinks you are targeting the wrong operating system. */
#if defined(__linux__)
#error "You are not using a cross-compiler, you will most certainly run into trouble"
#endif
 
/* This tutorial will only work for the 32-bit ix86 targets. */
#if !defined(__i386__)
//#error "This tutorial needs to be compiled with a ix86-elf compiler"
#endif


const char VERSTAG[] = "\0$VER: CuriOS 0.46a (10/08/2021) by Matt Parsons";



#include "descriptor_tables.h"
#include "timer.h"
#include "task.h"
#include "memory.h"
#include "graphics.h"
#include "intuition.h"


#include "x86cpu_ports.h"
#include "SystemLog.h"

#include "pci.h"
#include "ps2.h"
#include "cli.h"    //This is, for now, the boot task...

#include "math.h"

void KernelTaskEntry(){
    

}



void kernel_main(multiboot_info_t* mbd, unsigned int magic) {
    
    // Initialise the descriptor tables, and prepare the interrupts.
    init_descriptor_tables();
    
    //set up hardware from multiboot info
    if(magic != MULTIBOOT_BOOTLOADER_MAGIC){
        //terminal_writestring("Not booted by a MultiBoot compatible loader\n");
        return;
    }else{
        init_from_multiboot(mbd);
    }
    
    //Setup multitasking system
    InitMultitasking();
    
    //Here we should create a high priority "Kernel task" (I'll probably call it Executive), which can set everything up and then remain alive to handle kernel related functions. we can then hand off everything currently dont below to that task.
    
    
    //Initilise the graphics library.
    LoadGraphicsLibrary(mbd);
    executive->AddLibrary((library_t*)&graphics);
    
    //Initilise the graphical user interface.
    LoadIntuitionLibrary();
    executive->AddLibrary((library_t*)&intuition);
    
    
    //Initilise the System debugging log... to be replaced with a proper console when the console.device works
    InitSystemLog(0,graphics.frameBuffer.height/2,graphics.frameBuffer.width,graphics.frameBuffer.height/2);
    debug_write_string("Build Date: ");
    debug_write_string(__DATE__);
    debug_write_string("\nMultiboot Flags: ");
    
    debug_write_hex(mbd->flags);
    debug_write_string(" \nRAM:");
    debug_write_dec((executive->memSize/1024)/1024);debug_write_string(" Mb\n");
    debug_write_string("Framebuffer address: ");
    debug_write_hex(mbd->framebuffer_addr); debug_write_string(" \nwidth: ");   //pitch
    debug_write_dec(mbd->framebuffer_width); debug_write_string(" \nheight: ");
    debug_write_dec(mbd->framebuffer_height); debug_write_string(" \nbit depth: ");
    debug_write_dec(mbd->framebuffer_bpp); debug_write_string(" \n");
    
    //get CMOS TIME
    //debug_write_dec()
        
    
    
   // debug_write_string("Boot Task Currently Disabled in kernel.c");
    //Add the boot task, this task should be responsible for setting up the system.
    task_t* task = executive->AddTask(CliEntry,4096*2,0);  //double stack for Boot task... it has a lot to do...
    task->node.name = "BootShell";
    
    
    //Initilise the system timer to 1000Hz
    InitTimer(1000);
    
    //Load and initilise the pci.device
    LoadPCIDevice();
    executive->AddDevice((device_t*)&pci);
    
    //Initilise the PS2 port, for mouse and keyboard... We should only do this if PCI->USB fails
    InitPS2();

    
    //Start the multitasking, but starting the interrupts, the timer will fire and immediately schedule in any ready tasks
    asm volatile("sti"); //<--- Once multitasking is started, this thread can never be scheduled back in.
    
    
    
    
    //This WILL never be executed
	while(1){
        asm volatile ("hlt"); //Just let the CPU wait for an interrupt, in an infinite loop
    }
 

	
}


