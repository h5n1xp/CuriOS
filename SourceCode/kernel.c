#include "stdheaders.h"
#include "multiboot.h"


/* Check if the compiler thinks you are targeting the wrong operating system. */
#if defined(__linux__)
#error "You are not using a cross-compiler, you will most certainly run into trouble"
#endif
 



const char VERSTAG[] = "\0$VER: CuriOS 0.47a (06/09/2021) by Matt Parsons";



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
#include "cli.h"

#include "ata.h"
#include "fat_handler.h"

#include "string.h"





void KernelTaskEntry(void){
 
    executive->executivePort = executive->CreatePort("Executive Port"); // could probably just use the Executive's Rendezvous port?
    
    //setup the ata device as early as possible before the DOS Library
    LoadATADevice();
    executive->AddDevice((device_t*)&ata);
  
    //The boot disk will be FAT32, so load the FAT Handler
    LoadFATHandler();
    executive->AddDevice((device_t*)&fatHandler);
    
    //Initilise and open DOS library
    LoadDOSLibrary();   //Load DOS Library into memory (already done here) and call its "LoadXLibrary()" function
    executive->AddLibrary((library_t*)&dos);    // add DOS to the Exec library list.
    
   // debug_write_string("Boot Shell Currently Disabled in kernel.c");
    //Add the boot shell, this task should be responsible for setting up the user environment
    task_t* task = executive->CreateTask("BootShell",0,CliEntry,4096*4);  //quad stack for Boot shell... it has a lot to do...
    executive->AddTaskPrivate(task);
    
    int running = 1;
    
    do{
        
        uint64_t sigRec = executive->Wait(1 << executive->executivePort->sigNum);
        
        
        //Message received at the executivePort
        if(sigRec & 1 << executive->executivePort->sigNum){
        
            executiveRequest_t* request =(executiveRequest_t*) executive->GetMessage(executive->executivePort);
            
            while(request != NULL){
        
                //Handle requests
                switch(request->request){
                    case EXEC_REQUEST_NOP:
                        debug_write_string("Executive: Test Request Received.\n");
                        break;
                        
                    case EXEC_REQUEST_ADD_TASK:
                        task = request->data;
                        
                        if(task->node.type != NODE_TASK){
                            break;
                        }
                        
                        executive->AddTaskPrivate(task);
                        //debug_write_string("Executive: AddTask Request Received.\n");
                        break;
                        
                    case EXEC_REQUEST_REM_TASK:
                        debug_write_string("Executive: RemTask Request Received: ");
                        task = request->data;
                        
                        debug_write_string(task->node.name);debug_putchar('\n');
                    
                        switch(task->state){
                            case TASK_SUSPENDED: executive->Remove(&executive->taskSuspended,(node_t*)task);
                                break;
                            case TASK_WAITING: executive->Remove(&executive->taskWait,(node_t*)task);
                                break;
                            case TASK_READY: executive->Remove(&executive->taskReady,(node_t*)task);
                                break;
                            case TASK_RUNNING: //On a MultiCPU system the task could also be in a running state!!!
                                break;
                            default:
                                break;
                        }
                        

                        
                        
                        
                        //Need to deallocate the memory on the task's memory list which will include the code/data segments
                        //Don't forget to delete the dos port and the rendezvous ports.
                        //need to deallocate the trask strucuture and then remove the task node from the task list...
                        
                        break;
                        
                    case EXEC_REQUEST_SHUTDOWN:
                        
                        //send all tasks the shutdown signal.
                        
                        running = 0;
                        break;
                        
                    default:
                        break;
                }
                
        
                executive->ReplyMessage((message_t*)request);
                request = (executiveRequest_t*) executive->GetMessage(executive->executivePort);
            }
        
        }
        
        //Handle other signals here
        
        
    }while(running);
        
    
    //Halt the machine
    while(1){
        asm volatile ("hlt"); //Just let the CPU wait for an interrupt, in an infinite loop
    }
    
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
    
    task_t* task = executive->CreateTask("Executive",127,KernelTaskEntry,4096);
    // This code sets the task to operate in supervisor mode
    registers_t* regs = (registers_t*)(task->ssp - 4);
    regs->cs = 0x8;
    regs->ds = 0x10;
    regs->ss = 0x10;
    
    executive->AddTaskPrivate(task);
    
    
    //Initilise the graphics library.
    LoadGraphicsLibrary();
    executive->AddLibrary((library_t*)&graphics);
    
    //The graphics card driver is supposed to set the framebuffer, and override any functions which are hardware accelerated
    //but the system should always set up a temporary framebuffer, if the boot loader doesn't provide one
    void_ptr pointer = (void_ptr)mbd->framebuffer_addr;
    graphics.ChangeFrameBufferPrivate((void*)pointer,mbd->framebuffer_width,mbd->framebuffer_height,mbd->framebuffer_bpp);
    
    //Initilise the Graphical User Interface.
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
    debug_write_hex(mbd->framebuffer_addr);debug_putchar('\n');
    
    
    //Load and initilise the pci.device
    LoadPCIDevice();
    executive->AddDevice((device_t*)&pci);
    
    //get CMOS TIME
    //debug_write_dec()
    
    //Initilise the system timer to 1000Hz... this shoud only be done if we can't find the timer on the PCI
    InitTimer(1000);
    
    //Initilise the PS2 port, for mouse and keyboard... We should only do this if PCI->USB fails
    InitPS2();
    
    //Start the multitasking, by starting the interrupts, the timer will fire and immediately schedule in any ready tasks
    asm volatile("sti"); //<--- Once multitasking is started, this thread can never be scheduled back in.
    

    //This WILL never be executed
	//while(1){
    //    asm volatile ("hlt"); //Just let the CPU wait for an interrupt, in an infinite loop
    //}
}


