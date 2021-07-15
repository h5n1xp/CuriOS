//
//  input.c
//  PrepStep8
//
//  Created by Matt Parsons on 03/11/2020.
//  Copyright © 2020 Matt Parsons. All rights reserved.
//

#include "input.h"
#include "memory.h"

#include "SystemLog.h"
#include "graphics.h"
#include "intuition.h"

inputStruct_t inputStruct;

char inputTaskName[] = "Input Task";


#define SHIFT_VALUE 56
uint8_t scancode[] ={' ',' ','1','2','3','4','5','6','7','8','9','0','-','=',' ',
    '\t','q','w','e','r','t','y','u','i','o','p','[',']','\n',
    ' ','a','s','d','f','g','h','j','k','l',';','\'','\\',
    ' ','`','z','x','c','v','b','n','m',',','.','/','B',' ',
    
    ' ',' ','!','@','#','$','%','^','&','*','(',')','_','+',' ',
    '\t','Q','W','E','R','T','Y','U','I','O','P','{','}','\n',
    ' ','A','S','D','F','G','H','J','K','L',':','\"','|',
    ' ','~','Z','X','C','V','B','N','M','<','>','?','B',' '
};


void processKeyboardBuffer(uint8_t);

int InputTaskEntry(){
    
    //The input task can have the responsibility of maintaining the title bar.
    inputStruct.screenTitle = OpenWindow(NULL,0,0,graphics.frameBuffer.width,graphics.frameBuffer.height,WINDOW_BORDERLESS,"Screen Title Bar");
    inputStruct.screenTitle->noRise = true;
    graphics.DrawRect(inputStruct.screenTitle->bitmap, 0,0,graphics.frameBuffer.width,graphics.frameBuffer.height,intuition.backgroundColour);
    graphics.DrawRect(inputStruct.screenTitle->bitmap, 0,0,graphics.frameBuffer.width,20,intuition.white);
    graphics.RenderString(inputStruct.screenTitle->bitmap,intuition.defaultFont, 4,2,"System Screen",intuition.blue,intuition.white);
    
    inputStruct.screenTitle->node.priority = 100;
    PriorityOrderPrivate(inputStruct.screenTitle);
    
    executive->thisTask->node.name = inputTaskName;
    uint32_t keyboardBufferReadPosition = 0;
    
    while(1) {
        
        uint64_t signals = executive->Wait(2);
        
        if(signals == 2){
        
            while( keyboardBufferReadPosition != inputStruct.keyboardBufferPosition){
                uint8_t val = inputStruct.keyboardBuffer[keyboardBufferReadPosition];
                keyboardBufferReadPosition += 1;
                if(keyboardBufferReadPosition == 32){keyboardBufferReadPosition = 0;}
                processKeyboardBuffer(val);
            }

            
            //Generate Intuition Events
            IntuitionUpdate();
        }
    }
    
}


void processKeyboardBuffer(uint8_t val){
    
    static uint8_t shift = 0;
    static bool caps = false;
    
    // key release
    if( (val & 128) == 128){
        
        //Shift keys
        if((caps == false) && (val == 170 || val == 182) ){
            shift = 0;
        }
        
        
    }else{   //key down
        
        
        
        //backspace = 14
        //ctrl is 29
        //alt is 56;
        //left command is 91
        
        
        //key down
        uint8_t character = 0;
        
        switch(val){
                
            case 1: break; // escape key
                
            case 29: break; // left ctrl
                
            case 42:
                shift = SHIFT_VALUE;
                break;
                
            case 54:
                shift = SHIFT_VALUE;
                break;
                
            case 56: break; // alt
                
            case 58: //break;
                if(caps == false){
                    caps = true;
                    shift = SHIFT_VALUE;
                }else{
                    caps = false;
                    shift = 0;
                }
                break;
                
            case 59:break; //F1
            case 60:break; //F2
            case 61:break;
            case 62:break;
            case 63:break;
            case 64:break;
            case 65:break;
            case 66:break;
            case 67:break;
            case 68:break; // F10
                
            case 75:break; //left cursor
                           //case 72:break; //up cursor
            case 77:break; //right cursor
            case 80:break; //down cursor
            default:
                //terminal_write_dec(val);
                character = scancode[val+shift];
                
                //commandBufferInsert(character,val);
                if(inputStruct.focused->eventPort != NULL){
                    
                    intuitionEvent_t* event = (intuitionEvent_t*) executive->Alloc(sizeof(intuitionEvent_t));
                    event->message.replyPort = NULL;
                    event->flags = WINDOW_EVENT_KEYDOWN;
                    event->window = inputStruct.focused;
                    event->mouseX = 0;
                    event->mouseY = 0;
                    event->mouseXrel = 0;
                    event->mouseYrel = 0;
                    event->rawKey = val;
                    event->scancode = character;
                    executive->PutMessage(inputStruct.focused->eventPort ,(message_t*)event);
                    
                }
                
                break;
        }
    }
}
