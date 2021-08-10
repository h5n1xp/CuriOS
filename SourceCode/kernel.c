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


const char VERSTAG[] = "\0$VER: CuriOS 0.45a (08/07/2021) by Matt Parsons";



#include "descriptor_tables.h"
//#include "terminal.h"
#include "timer.h"
#include "task.h"
#include "memory.h"
#include "graphics.h"
#include "intuition.h"


#include "x86cpu_ports.h"
#include "SystemLog.h"

#include "ps2.h"

#include "cli.h"    //This is, for now, the boot task...


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
    
    //Initilise the graphics library.
    LoadGraphicsLibrary(mbd);
    executive->AddLibrary((library_t*)&graphics);
    
    //Initilise the graphical user interface.
    LoadIntuitionLibrary();
    executive->AddLibrary((library_t*)&intuition);
    
    //Initilise the System debugging log
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
    
    
    //Setup initial tasks
    InitMultitasking();
    
    
    //Add the boot task, this task should be responsible for setting up the system.
    //task_t* task = executive->AddTask(CliEntry,4096*2,0);  //double stack for Boot task... it has a lot to do...
    //task->node.name = "BootShell";
    
    
    //Initilise the system timer to 1000Hz
    InitTimer(1000);
    
    //Initilise the PS2 port, for mouse and keyboard
    InitPS2();

    //Start the multitasking.
    asm volatile("int $48"); //<--- Once multitasking is started, this thread will never be scheduled back in.
    
    //This WILL never be executed
	while(1){
        asm volatile ("hlt"); //Just let the CPU wait for an interrupt, in an infinite loop
    }
 

	
}


