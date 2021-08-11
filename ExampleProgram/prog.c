//
//  bouncy.c
//
//  Created by Matt Parsons on 06/08/2021.
//  Copyright © 2021 Matt Parsons. All rights reserved.
//


#include "curios.h"


//The ELF Loader will find this and fix with correct addresses
executive_t* executive __attribute__ ((section (".executive"))) = {0};

const char VERSTAG[] = "\0$VER: Bouncy 0.1a (06/08/2021) by Matt Parsons";

int ballX = 40;
int ballY = 40;
int ballVX = 1;
int ballVY = 1;
intuition_t* intuibase = (intuition_t*) 1; //setting the value to 1 so the compiler doesn't try to put it in the BSS section, becasue my loader don't make one yet.

void updateBall(window_t* under){
    
    //animate
    intuibase->DrawRectangle(under, ballX, ballY, 9, 9, under->backgroundColour);
 
    
    ballX += ballVX;
    ballY += ballVY;
            
    if((uint32_t)ballX >= under->innerW-10 || (uint32_t)ballX <= under->innerX+2){ballVX = -ballVX;}
    if((uint32_t)ballY >= under->innerH-10 || (uint32_t)ballY <= under->innerY+2){ballVY = -ballVY;}
            

    intuibase->DrawCircle(under,ballX+4,ballY+4,4,intuibase->black, false);
    intuibase->FloodFill(under,ballX+4,ballY+4,intuibase->orange);
    
}


void main(){
    

    
    intuibase =  (intuition_t*)executive->OpenLibrary("intuition.library",0);
    
    if(intuibase == NULL){
        return;
    }
    
    window_t* under = intuibase->OpenWindow(NULL, 0, 0, 200, 120,WINDOW_TITLEBAR | WINDOW_DRAGGABLE | WINDOW_DEPTH_GADGET | WINDOW_RESIZABLE | WINDOW_CLOSE_GADGET | WINDOW_VSYNC, "Bouncy!");
    
    if(under == NULL){
        return;
    }
    
    intuibase->Focus(under);
    
    intuibase->SetScreenTitle(under,"A Real Program!!!");
    
    under->eventPort = executive->CreatePort("eventPort");
    
    
    
    
    


    int running = 1;

    while(running) {
        
        uint64_t sigRec = executive->Wait(1 << under->eventPort->sigNum);
        
        
        //resize window
        if(sigRec & 1 << under->eventPort->sigNum){

            intuitionEvent_t* event =(intuitionEvent_t*) executive->GetMessage(under->eventPort);
            
            while(event != NULL){
                
                if(event->flags & WINDOW_EVENT_RESIZE){
                    intuibase->ResizeWindow(under, event->window->w - event->mouseXrel, event->window->h - event->mouseYrel);

                    if((uint32_t)ballX > (under->innerW-14)){ballX = 10;}
                    if((uint32_t)ballY > (under->innerH-12)){ballY = 24;}
                    
                }
                
                if(event->flags & WINDOW_EVENT_VSYNC){
                    updateBall(under);
                }
                
                if(event->flags & WINDOW_EVENT_CLOSE){
                    running = 0;
                    executive->ReplyMessage((message_t*)event);
                    intuibase->CloseWindow(under);
                    break;
                }
                
                executive->ReplyMessage((message_t*)event);
                event = (intuitionEvent_t*) executive->GetMessage(under->eventPort);
            }
            
        }
        

    }
    
    //return 0;

    executive->thisTask->state = TASK_ENDED;
    running = running / 0;  // pursposely crash the task.
    
}
