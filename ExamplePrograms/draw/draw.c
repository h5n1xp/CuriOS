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


intuition_t* intuibase = (intuition_t*) 1; //setting the value to 1 so the compiler doesn't try to put it in the BSS section, becasue my loader don't make one yet.

//Classic Amiga boot image
uint8_t kickStartBootImage[] = {255,1,35,11,58,11,58,33,113,33,113,11,125,11,136,22,136,94,127,94,127,56,64,56,62,54,53,54,52,56,45,56,45,65,35,72,35,11,254,2,37,69,255,1,33,72,33,10,126,10,138,22,138,95,86,95,86,100,82,108,78,113,74,116,68,125,60,129,60,140,10,140,10,109,9,109,9,81,13,75,20,69,21,65,25,58,30,55,33,54,33,54,30,56,26,58,22,65,21,69,14,75,10,81,10,108,11,109,11,139,40,139,40,118,48,118,52,114,52,95,50,92,50,82,65,69,65,57,62,55,59,55,62,58,62,65,61,66,54,66,51,63,42,70,30,76,18,85,18,84,30,75,26,74,23,71,26,73,30,74,33,72,255,1,50,61,52,54,60,55,61,58,61,65,54,65,50,61,255,1,51,92,51,82,66,69,66,57,125,57,125,94,52,94,51,90,255,1,60,11,111,11,111,32,60,32,60,11,255,1,96,14,107,14,107,28,96,28,96,14,254,3,62,31,255,1,98,15,105,15,105,27,98,27,98,15,254,2,99,26,255,1,47,57,50,57,50,59,47,63,47,57,255,1,41,139,41,119,48,119,53,114,53,105,57,107,65,107,65,109,69,114,73,114,73,116,67,125,59,128,59,139,41,139,255,1,53,95,53,100,58,97,53,95,255,1,57,98,53,100,53,95,74,95,64,105,63,105,65,103,60,98,57,98,255,1,78,95,85,95,85,100,81,108,78,112,73,113,70,113,67,109,67,106,78,95,255,1,68,106,68,109,70,112,72,112,76,111,77,108,73,105,68,106,255,1,54,104,62,106,64,103,60,99,57,99,54,101,54,104,255,1,126,11,137,22,137,94,254,1,34,11,254,1,59,11,254,1,97,15,254,1,106,27,254,1,112,15,254,1,126,94,254,1,75,96,254,1,46,57,255,255};


void main(){
    

    
    intuibase =  (intuition_t*)executive->OpenLibrary("intuition.library",0);
    
    if(intuibase == NULL){
        return;
    }
    
    window_t* under = intuibase->OpenWindow(NULL,650,550,320,200,WINDOW_TITLEBAR | WINDOW_DRAGGABLE | WINDOW_DEPTH_GADGET | WINDOW_CLOSE_GADGET,"GFX Drawing Test");
    
    if(under == NULL){
        return;
    }
    
    intuibase->Focus(under);
    
    intuibase->SetScreenTitle(under,"Program draws Amiga Vector Images!");
    under->isBusy = true;
    
    
    under->eventPort = executive->CreatePort("eventPort");
    
    intuibase->DrawRectangle(under,30,30,200,160,intuibase->white);
    intuibase->DrawLine(under,5,25,50,90,intuibase->black);
    intuibase->DrawCircle(under,50,55,32,intuibase->black, false);
    intuibase->FloodFill(under,50,55,intuibase->orange);
    
    intuibase->DrawVectorImage(under,70,40,kickStartBootImage);
    
    


    int running = 1;

    while(running) {
        
        uint64_t sigRec = executive->Wait(1 << under->eventPort->sigNum);
        
        
        //resize window
        if(sigRec & 1 << under->eventPort->sigNum){

            intuitionEvent_t* event =(intuitionEvent_t*) executive->GetMessage(under->eventPort);
            
            while(event != NULL){
                
                
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

    executive->RemTask(NULL); //remove this task
    
}
