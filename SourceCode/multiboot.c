//
//  multiboot.c
//  
//
//  Created by Matt Parsons on 05/10/2020.
//
//


#include "multiboot.h"
#include "string.h"
#include "memory.h"

//#include "graphics.h"
//#include "MPKernel.h"

void RamTest(void* addr,uint32_t len){
    
    volatile uint8_t* d = addr;
    uint8_t t = 0;
    
    while(len>0){
        
        t = (uint8_t)len;
        d[len] = t;
        if(d[len] != t){
            //terminal_writestring("RAM Error!");
        }
        len -= 1;
    }
}




void init_from_multiboot(multiboot_info_t* mbd){
    
    //Set up memory from the bootloader information
    
    if( (mbd->flags & 2048) == 2048){
        //terminal_writestring("VBE GFX Available\n");
    }
    
    if( (mbd->flags & 4096) == 4096){
        //terminal_writestring("Framebuffer GFX Available\n");
        
    }
    

    if( (mbd->flags & 1) == 0){
        //terminal_writestring("Memory info invalid\n");
        return;
    }
    
    if( (mbd->flags & 64) != 64){
        //terminal_writestring("No Memory Map!\n");
        return;
    }
    
    /*
     terminal_writestring("\nmmap length: ");
     terminal_write_dec((uint32_t)mbd->mmap_length);
     terminal_writestring(" \nmmap address: ");
     terminal_write_dec((uint32_t)mbd->mmap_addr);
     terminal_writestring(" \n");
     */
    
    
    multiboot_memory_map_t* mmap = (multiboot_memory_map_t*) mbd->mmap_addr;
    
    uint32_t entries = mbd->mmap_length / sizeof(multiboot_memory_map_t);
    
    uint32_t largestChunk = 0;
    uint32_t addr = 0;
    
    for(uint32_t i = 0; i < entries; ++i){
        
        if(mmap[i].len>largestChunk && mmap[i].type == 1){
            largestChunk = mmap[i].len;
            addr = mmap[i].addr;
        }
        
        /*
         terminal_writestring("\nSize: ");
         terminal_write_hex(mmap[i].size);
         terminal_writestring(" || Base: ");
         terminal_write_hex(mmap[i].addr);
         terminal_writestring(" || Length: ");
         terminal_write_dec(mmap[i].len);
         terminal_writestring(" || Type: ");
         terminal_write_dec(mmap[i].type);
         */
    }
    
    //let's clear the extended ram
    uint32_t kernelArea = 524288;   //give the kernel 512k for code and data
    addr            += kernelArea;
    largestChunk    -= kernelArea;
    
    memset((void*)(addr),0,largestChunk); //clear the ram...
    InitMemory((void*)addr,largestChunk);
    
    //RamTest((void*)(addr),largestChunk);
    
    /* // report available ram to the terminal
     terminal_writestring("\nRam Size: ");
     terminal_write_hex(largestChunk);
     terminal_writestring(" \n");
     */
    
    /*
     terminal_writestring("\nConventional Memory: ");
     terminal_write_dec((uint32_t)mbd->mem_lower);
     terminal_writestring("k \nExtended Memory: ");
     terminal_write_dec((uint32_t)mbd->mem_upper/1024);
     terminal_writestring("Mb \n");
     */
    
}
