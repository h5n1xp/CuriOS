//
//  bouncy.c
//
//  Created by Matt Parsons on 06/08/2021.
//  Copyright © 2021 Matt Parsons. All rights reserved.
//


#include "curios.h"
#include "curios/math.h"

//The ELF Loader will find this and fix with correct addresses
executive_t* executive __attribute__ ((section (".executive"))) = {0};

const char VERSTAG[] = "\0$VER: Bouncy 0.1a (06/08/2021) by Matt Parsons";


intuition_t* intuibase = (intuition_t*) 1; //setting the value to 1 so the compiler doesn't try to put it in the BSS section, becasue my loader don't make one yet.


float* x;
float* y;

void clockDrawFace(window_t* win,int ticks){

    intuibase->DrawCircle(win,100,125,90,intuibase->black,false);
    intuibase->DrawCircle(win,100,125,89,intuibase->black,false);
    intuibase->FloodFill(win,100,125,intuibase->white);
    
    
   // while(1){
   // }
    
    int minuteMarker = ticks / 60;
    int hourMarker = ticks / 12;
    
    int tx1 = 0;
    int ty1 = 0;

    int tx2 = 0;
    int ty2 = 0;

    int tx3 = 0;
    int ty3 = 0;

    int tx4 = 0;
    int ty4 = 0;
    
    for(int i = 0;i<ticks;++i){
        
        if(i%minuteMarker == 0){
            
            if(i%hourMarker == 0){
                

                
                if(i==0){
                    tx1 = (x[ticks-4]*83)+100;
                    ty1 = (y[ticks-4]*83)+125;
                }else{
                    tx1 = (x[i-3]*83)+100;
                    ty1 = (y[i-3]*83)+125;
                }
                
                tx2 = (x[i]*88)+100;
                ty2 = (y[i]*88)+125;
                
                tx3 = (x[i+3]*83)+100;
                ty3 = (y[i+3]*83)+125;
                
                tx4 = (x[i]*78)+100;
                ty4 = (y[i]*78)+125;
                
                intuibase->DrawLine(win,tx1,ty1,tx2,ty2,intuibase->black);
                intuibase->DrawLine(win,tx2,ty2,tx3,ty3,intuibase->black);
                intuibase->DrawLine(win,tx3,ty3,tx4,ty4,intuibase->black);
                intuibase->DrawLine(win,tx4,ty4,tx1,ty1,intuibase->black);
                
                int ax = (tx1 + tx3)/2;
                int ay = (ty2 + ty4)/2;
                intuibase->FloodFill(win,ax,ay,intuibase->black);
                
                
                
                
            }else{
                intuibase->DrawLine(win,(x[i]*80)+100,(y[i]*80)+125,(x[i]*90)+100,(y[i]*90)+125,intuibase->black);
            }
            
        }
        
    }
    
}
 

void clockDrawHand(window_t* win,int ticks,int pos,int len, int wid, uint32_t colour){
    
    
    int tx1 = 0;
    int ty1 = 0;

    int tx2 = 0;
    int ty2 = 0;

    int tx3 = 0;
    int ty3 = 0;

    int tx4 = 0;
    int ty4 = 0;
    
    int sho = len - 16;
    
    if(pos==0){
        tx1 = (x[(ticks-wid)-1]*sho)+100;
        ty1 = (y[(ticks-wid)-1]*sho)+125;
    }else{
        tx1 = (x[pos-4]*sho)+100;
        ty1 = (y[pos-4]*sho)+125;
    }
    
    tx2 = (x[pos]*len)+100;
    ty2 = (y[pos]*len)+125;
    
    tx3 = (x[pos+wid]*sho)+100;
    ty3 = (y[pos+wid]*sho)+125;
    
    tx4 = 100;
    ty4 = 125;
    
    
    
    intuibase->DrawLine(win,tx1,ty1,tx2,ty2,colour);
    intuibase->DrawLine(win,tx2,ty2,tx3,ty3,colour);
    intuibase->DrawLine(win,tx3,ty3,tx4,ty4,colour);
    intuibase->DrawLine(win,tx4,ty4,tx1,ty1,colour);
    
    int ax = (x[pos]*sho)+100;
    int ay = (y[pos]*sho)+125;
    intuibase->FloodFill(win,ax,ay,intuibase->black);
    
}

void clockEraseHand(window_t* win,int ticks,int pos,int len, int wid, uint32_t colour){

    int tx1 = 0;
    int ty1 = 0;

    int tx2 = 0;
    int ty2 = 0;

    int tx3 = 0;
    int ty3 = 0;

    int tx4 = 0;
    int ty4 = 0;
    
    int sho = len - 16;
    
    if(pos==0){
        tx1 = (x[(ticks-wid)-1]*sho)+100;
        ty1 = (y[(ticks-wid)-1]*sho)+125;
    }else{
        tx1 = (x[pos-4]*sho)+100;
        ty1 = (y[pos-4]*sho)+125;
    }
    
    tx2 = (x[pos]*len)+100;
    ty2 = (y[pos]*len)+125;
    
    tx3 = (x[pos+wid]*sho)+100;
    ty3 = (y[pos+wid]*sho)+125;
    
    tx4 = 100;
    ty4 = 125;
    
    
    int sx = tx1 < tx2 ? tx1 : tx2;
    sx = sx  < tx3 ? sx  : tx3;
    sx = sx  < tx4 ? sx  : tx4;
    
    int sy = ty1 < ty2 ? ty1 : ty2;
    sy = sy  < ty3 ? sy  : ty3;
    sy = sy  < ty4 ? sy  : ty4;
    
    
    int mx = tx1 > tx2 ? tx1 : tx2;
    mx = mx  > tx3 ? mx  : tx3;
    mx = mx  > tx4 ? mx  : tx4;
    
    int my = ty1 > ty2 ? ty1 : ty2;
    my = my  > ty3 ? my  : ty3;
    my = my  > ty4 ? my  : ty4;
    
    intuibase->DrawRectangle(win,sx-1,sy-1,(mx-sx)+2,(my-sy)+2,colour);
    
}

void main(){
    

    
    intuibase =  (intuition_t*)executive->OpenLibrary("intuition.library",0);
    
    if(intuibase == NULL){
        return;
    }
    
    window_t* under = intuibase->OpenWindow(NULL,650,500,200,250,WINDOW_TITLEBAR | WINDOW_DRAGGABLE | WINDOW_DEPTH_GADGET | WINDOW_CLOSE_GADGET| WINDOW_VSYNC,"Clock");
    window_t* win = under;
    
    if(under == NULL){
        return;
    }
    
    intuibase->Focus(win);
    
    intuibase->SetScreenTitle(win,"Amiga Clock!");
    under->eventPort = executive->CreatePort("eventPort");
    
    int ticks = 480;
    float count = -1.570796327;
    float inc = (2*PI) / (float)ticks;
    
    x = (float*) executive->AllocMem(sizeof(float)*ticks,0);
    y = (float*) executive->AllocMem(sizeof(float)*ticks,0);
    
    
    for(int i = 0;i<ticks;++i){
        y[i] = sin(count);
        x[i] = cos(count);
        count += inc;
    }
    
    clockDrawFace(win,ticks);
    

    
    
    
    
    int hour = 0;
    int minute = 0;
    int minuteMarker = ticks / 60;
    int hourMarker = ticks / 12;

    //int waitTime = (int)(1000.0 *(60.0/(float)ticks));
    int vwaits = 7; //approximately 125ms, which is the wait time betwwen ticks
    
    int i = 0;
    
    int y2 = (int)(y[i]*74.0);
    int x2 = (int)(x[i]*74.0);
    

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
                
                if(event->flags & WINDOW_EVENT_VSYNC){
                    
                    vwaits -= 1;
                    
                    if(vwaits==0){
                        vwaits = 7;
                        intuibase->DrawLine(win,100,125,x2+100,y2+125,intuibase->white);
                        clockEraseHand(win,ticks,hour,55,6,intuibase->white);
                        clockEraseHand(win,ticks,minute,74,4,intuibase->white);
                    
                        executive->Forbid();    //we don't save the FPU registers yet... so we can't context switch in the middle of this
                        y2 = (int)(y[i]*74.0);
                        x2 = (int)(x[i]*74.0);
                        executive->Permit();
                

                
                        i++;
                        if(i>(ticks-1)){
                    
                            minute += minuteMarker;
                    
                            if(minute>(ticks - minuteMarker)){
                                minute = 0;
                                hour +=hourMarker;
                        
                                if(hour>(ticks - hourMarker)){
                                    hour = 0;
                                }
                            }
                    
                            i=0;
                        }
                

                        clockDrawHand(win,ticks,minute,74,4,intuibase->black);
                        clockDrawHand(win,ticks,hour,55,6,intuibase->black);
                        intuibase->DrawLine(win,100,125,x2+100,y2+125,intuibase->orange);
                    }
                    //WaitMS(waitTime);
                    //WaitMS(1);
                
                }
                
                
                executive->ReplyMessage((message_t*)event);
                event = (intuitionEvent_t*) executive->GetMessage(under->eventPort);
            }
            
        }
        

    }
    
    //return 0;

    executive->RemTask(NULL);   //remove self
    
}
