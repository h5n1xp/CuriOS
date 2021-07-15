//
//  ps2.c
//  
//
//  Created by Matt Parsons on 22/09/2020.
//
//

#include "ps2.h"
#include "x86cpu_ports.h"
#include "interrupts.h"
#include "memory.h"
#include "task.h"
#include "intuition.h"
#include "input.h"

//Keyboard *****************************************


void keyboard_callback(registers_t* regs){
    
    regs->ds +=0; //suppress compiler warning about unused variables;
    
    uint8_t val = inb(0x60); // get the last pressed key
    
    if(val==0x0 || val == 0xFF){
        return;
    }
    

    inputStruct.keyboardBuffer[inputStruct.keyboardBufferPosition] = val;
    
    inputStruct.keyboardBufferPosition += 1;
    if(inputStruct.keyboardBufferPosition == 32){inputStruct.keyboardBufferPosition = 0;}

}

int8_t seq = 0;

void mouse_callback(registers_t* regs){
    
    
    regs->ds +=0; //suppress compiler warning about unused variables;
    
    switch(seq){
        case 0:
            inputStruct.rawMouse[0]  =  inb(0x60);
            
            if( !(inputStruct.rawMouse[0] & 0x8) ){
                
                seq = -1;
                
            }
            
            break;
        case 1:
            inputStruct.rawMouse[1] +=  inb(0x60);
            break;
        case 2:
            inputStruct.rawMouse[2] +=  inb(0x60);
            inputStruct.rawMouse[3]  = 1; //let intuition know to update the mouse
            seq = -1;
            break;
    }
    
   
    seq +=1;
    
    


}

void InitPS2(){
    
    //disable the controller while we set it up.
    outb(0x64,0xAD); //first channel
    outb(0x64,0xA7); //second channel
    
    //flush buffer, by reading it... lots of times
    for(int i=0;i<1024;i++){
        inb(0x60);
    }
    //Send a POST command to the controller
    outb(0x64,0xAA);
    
    if(inb(0x60) != 0x55){
        //terminal_writestring("Keyboard/Mouse controller Error!\n");
    }
    
    //Send a POST command to 1st PS/2 port
    outb(0x64,0xAB);
    
    if(inb(0x60) != 0x00){
        //terminal_writestring("Keyboard Error!\n");
    }
    
    //Send a POST command to 2nd PS/2 port
    outb(0x64,0xA9);
    
    if(inb(0x60) != 0x00){
        //terminal_writestring("Mouse Error!\n");
    }
    

    
    
    
    //Activate packet transmission on the mouse
    outb(0x64,0xD4); // inform next byte is a command for the mouse
    outb(0x60,0xF4); // transmission on
    
    //wait for mouse to be ready
    while(!(inb(0x64) & 1)){
        
    };

    inb(0x60); // read back the acknowledge byte (should be 0xFA, not bothering to check here)

    
    //Enable controller
    outb(0x64,0xAE); //first channel
    outb(0x64,0xA8); //second channel
    
    //enable interrupts
    outb(0x64,0x60);
    outb(0x60,0x43);
    
    //outb(0x64,0x20); //request config
    //terminal_writestring("Mouse Active\n");
    //terminal_write_hex(inb(0x60));
    
    
    
    // Register our keyboard callback.
    register_interrupt_handler(IRQ1, keyboard_callback);
    register_interrupt_handler(IRQ12, mouse_callback);

}




/*
 //if the mouse suffers a sync error... just reset it.
 if( (rawMouse[0] & 0x8) != 0x8){
 terminal_writestring("!sync error!");
 //Send a Reset command to 2nd PS/2 port
 outb(0x64,0xD4); // inform next byte is a command for the mouse
 outb(0x64,0xFF);
 
 //wait for mouse to be ready
 while(!(inb(0x64) & 1)){}
 inb(0x60); // read back the acknowledge byte (should be 0xFA, not bothering to check here)
 
 //Activate packet transmission on the mouse
 outb(0x64,0xD4); // inform next byte is a command for the mouse
 outb(0x60,0xF4); // transmission on
 
 //wait for mouse to be ready
 while(!(inb(0x64) & 1)){
 }
 inb(0x60); // read back the acknowledge byte (should be 0xFA, not bothering to check here)
 
 seq=0;
 return;
 }
 */
