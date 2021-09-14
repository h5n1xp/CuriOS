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

void InputTaskEntry(){
    
    intuition_t* intuibase =  (intuition_t*)executive->OpenLibrary("intuition.library",0);
    
    intuibase->intuiPort = executive->CreatePort("intuiPort");
    
    //The input task can have the responsibility of maintaining the title bar.
    inputStruct.screenTitle = intuibase->OpenWindowPrivate(NULL,0,0,graphics.frameBuffer.width,graphics.frameBuffer.height,WINDOW_BORDERLESS,"Screen Title Bar");
    inputStruct.screenTitle->noRise = true;
    graphics.DrawRect(inputStruct.screenTitle->bitmap, 0,0,graphics.frameBuffer.width,graphics.frameBuffer.height,intuition.backgroundColour);
    graphics.DrawRect(inputStruct.screenTitle->bitmap, 0,0,graphics.frameBuffer.width,20,intuition.white);
    graphics.RenderString(inputStruct.screenTitle->bitmap,intuition.defaultFont, 4,2,"System Screen",intuition.blue,intuition.white);
    
    inputStruct.screenTitle->node.priority = 100;
    intuibase->PriorityOrderPrivate(inputStruct.screenTitle);
    
    executive->thisTask->node.name = inputTaskName;
    uint32_t keyboardBufferReadPosition = 0;
    
    while(1) {
        
        uint64_t signals = executive->Wait(2 | 1 << intuibase->intuiPort->sigNum); // signal 2 being the interrupt signal
        

        
        //message received
        
        if(signals & (1 << intuibase->intuiPort->sigNum ) ){ // && 0 means this will never execute
            
            intuitionEvent_t* event = (intuitionEvent_t*) GetMessage(intuibase->intuiPort);
            
            while(event != NULL){
                
                if(event->flags & WINDOW_EVENT_REQUEST_OPEN_WINDOW){

                    executive->Enqueue( intuibase->windowList, (node_t*)event->window);
                    //event->window->needsRedraw = true;
                    intuibase->Redraw(event->window);
                    intuibase->updateLayers(event->window);
                    
                }
                
                if(event->flags & WINDOW_EVENT_REQUEST_CLOSE_WINDOW){

                    executive->Remove( intuibase->windowList, (node_t*)event->window);
                    window_t* newFront = (window_t*)intuibase->windowList->pred;
                    //newFront->needsRedraw = true;
                    intuibase->Redraw(newFront);
                    intuibase->updateLayers(NULL);
            
                }
                
                
                if(event->flags & WINDOW_EVENT_REQUEST_RESIZE_WINDOW){
                    
                    event->window->w = event->mouseX;
                    event->window->h = event->mouseY;
                    intuibase->GimmeZeroZero(event->window);
                    event->window->bitmap = event->data;
                    
                    intuibase->updateLayers(event->window);
                
                    //I would prefer these two functions to be on the task context
                    graphics.ClearBitmap(event->window->bitmap,event->window->backgroundColour); // This should use the proper window clear function....
                    intuition.DrawDecoration(event->window);
                    
                    //It's a bit hacky to just signal without sending a message, but this is all internal to intution
                    executive->Signal(event->message.replyPort->owner,1 << event->message.replyPort->sigNum); //signal task resize is complete
                    event->message.replyPort = NULL;    // don't sent message back when replying
                }
                
                if(event->flags & WINDOW_EVENT_REQUEST_CHANGE_VISIBILITY){
                 
                    if(event->rawKey==1){
                        event->window->isVisible = true;
                    }else{
                        event->window->isVisible = false;;
                    }
                    
                    intuibase->updateLayers(event->window);
                }
                
                
                if(event->flags & WINDOW_EVENT_REQUEST_DRAW_TO_WINDOW){
                    debug_write_string("Intution Draw Request received: ");
                
                    bitmap_t* bm    = event->window->bitmap;
                    uint32_t colour = (uint32_t)event->data;
                    uint8_t command = event->rawKey;
                   // uint8_t flags   = event->scancode;
                    uint32_t x1     = event->mouseX;
                    uint32_t y1     = event->mouseY;
                    //uint32_t x2     = event->mouseXrel;
                    //uint32_t y2     = event->mouseYrel;
                    
                    switch (command) {
                        case WINDOW_DRAW_COMMAND_PLOT:
                            graphics.PutPixel(bm, x1, y1, colour);
                            break;
 
                        case WINDOW_DRAW_COMMAND_CLEAR:
                            break;
                            
                        case WINDOW_DRAW_COMMAND_PUTCHAR:
                            break;
                            
                        case WINDOW_DRAW_COMMAND_STRING:
                            break;
                            
                        case WINDOW_DRAW_COMMAND_RECTANGLE:
                            break;
                            
                        case WINDOW_DRAW_COMMAND_LINE:
                            break;
                            
                        case WINDOW_DRAW_COMMAND_FILL:
                            break;
                            
                        case WINDOW_DRAW_COMMAND_CIRCLE:
                            break;
                            
                        case WINDOW_DRAW_COMMAND_TRIANGLE:
                            break;
                            
                        case WINDOW_DRAW_COMMAND_VECTOR:
                            break;
                            
                        default:
                            break;
                    }
                    
                    
                    //debug_putchar('\n');
                   
                }
                
                executive->ReplyMessage((message_t*)event);
                event = (intuitionEvent_t*) GetMessage(intuibase->intuiPort);
            }
           
        }
        
        //interrupt received
        if(signals & 2){
        
            while( keyboardBufferReadPosition != inputStruct.keyboardBufferPosition){
                uint8_t val = inputStruct.keyboardBuffer[keyboardBufferReadPosition];
                keyboardBufferReadPosition += 1;
                if(keyboardBufferReadPosition == 32){keyboardBufferReadPosition = 0;}
                processKeyboardBuffer(val);
            }
        
            //Generate Intuition Events
            intuibase->Update();
            
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
               
                character = scancode[val+shift];
                
               
                if(inputStruct.focused->eventPort != NULL && (inputStruct.focused->flags & WINDOW_KEYBOARD) ){
                    
                    //intuitionEvent_t* event = (intuitionEvent_t*) executive->Alloc(sizeof(intuitionEvent_t),0);
                    //event->message.replyPort = NULL;
                    //event->message.node.type = NODE_MESSAGE;
                    
                    intuitionEvent_t* event = executive->CreateMessage(sizeof(intuitionEvent_t), NULL);
                    
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
