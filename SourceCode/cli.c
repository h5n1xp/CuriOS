//
//  cli.c
//
//  Created by Matt Parsons on 10/11/2020.
//  Copyright © 2020 Matt Parsons. All rights reserved.
//

#include "cli.h"

#include "string.h"
#include "list.h"
#include "memory.h"
#include "intuition.h"

#include "dos.h"

#include "SystemLog.h"

#include "timer.h"

#include "math.h"



intuition_t* intuibase;



typedef struct {
    window_t* window;
    
    int32_t width;
    int32_t height;
    int32_t x;
    int32_t y;

    uint32_t conCurX;
    uint32_t conCurY;

    intuition_t* intuibase;
    
    uint32_t bufferIndex;
    char consoleBuffer[4096];
    
    bool windowClear;
    
    
    //Command interpretor data
    dos_t* dosbase;
    uint32_t commandBufferIndex;
    char commandBuffer[512];
    char lastCommand[512];
    
}console_t;






void ConsoleClearCursor(console_t* console){
    
    window_t* window = console->window;
    
    intuibase->DrawRectangle(window,console->conCurX, console->conCurY, 8, 16, window->backgroundColour);
}



void ConsoleDrawCursor(console_t* console, uint32_t x, uint32_t y){
    
    window_t* window = console->window;
    
    console->conCurX = (x*8)+4;
    console->conCurY = (y*16)+22;
    intuibase->DrawRectangle(window,console->conCurX, console->conCurY, 8, 16, window->highlightColour);
}

void ConsoleRedraw(console_t* console);


void ConsoleScroll(console_t* console){
    
   // window_t* window = console->window;
    
    if(console->bufferIndex >= 4095){
        
        for(int i = 0 ; i< 2048;++i){
            
            console->consoleBuffer[i] = console->consoleBuffer[i+2048];
            
        }
        
        console->bufferIndex = 2047;
        
    }
    
    ConsoleRedraw(console);
}



void ConsolePutCharPrivate(console_t* console, char character){
    
    window_t* window = console->window;
    
    //Doesn't record the character placement in the console buffer
    
    ConsoleClearCursor(console);
    
    if(character=='\n'){
        ++console->y;
        console->x=0;
        
        if(console->y >= console->height){
               ConsoleScroll(console);
               //y -= 1;
           }
        
    }else if(character=='\t'){
        
        //x = (x + 7) & 0xFFF8;
        if(console->x%4){
            console->x=console->x-1;
            console->x=console->x/4;
            console->x=console->x*4;
            console->x=console->x+4;
        }
        
    }else{
    
        intuibase->PutChar(window,(console->x*8)+4,(console->y*16)+22,character,window->foregroundColour,window->backgroundColour);
        executive->Reschedule();//***************************************************************************************************SLOW THIS RIGHT DOWN!!
        console->x++;
        if(console->x>console->width){
            console->y++;
            
            if(console->y >= console->height){
                ConsoleScroll(console);
                //y -= 1;
            }
            
            console->x=0;
        }
    
    }
    ConsoleDrawCursor(console,console->x,console->y);
}

void ConsolePutChar(console_t* console, char character){
    
   // window_t* window = console->window;
    
    console->consoleBuffer[console->bufferIndex++] = character;
    if(console->bufferIndex >= 4095){
        
        for(int i = 0 ; i < 2048;++i){
            
            console->consoleBuffer[i] = console->consoleBuffer[i+2048];
            
        }
        
        console->bufferIndex = 2048;
        
    }
    
    
    ConsolePutCharPrivate(console, character);

    
}



void ConsoleRedraw(console_t* console){
 
    window_t* window = console->window;
    
    //Count lines
    int32_t lines = 0;
    uint32_t i = 0;
    char c = 0;
    
    int32_t lineLength = 0;
    
    do{
        c = console->consoleBuffer[i];
        lineLength += 1;
        
        if(c =='\n'){
            lines += 1;
            lineLength = 0;
        }
        
        if(lineLength >= console->width){
            lines += 1;
            lineLength = 0;
        }
        
        i += 1;
    }while(i<console->bufferIndex);
    
    if(lines >= console->height){
    
        uint32_t startline = lines - console->height;

        lines = 0;
        i = 0;
        c = 0;
    
        uint32_t lineLength = 0;
    
        do{
            c = console->consoleBuffer[i];
            lineLength += 1;
        
            if(c =='\n'){
                lines += 1;
                lineLength = 0;
            }
        
            if((uint32_t)lineLength >= (uint32_t)console->width){
                lines += 1;
                lineLength = 0;
            }
        
            i += 1;
        }while((uint32_t)lines <= startline);
    
    }else{
        i = 0;
    }
    
    console->x = 0;
    console->y = 0;
    
    if(console->windowClear == false){
        executive->Forbid(); //intuition doesn't lock the buffers properly for the clear window function
        intuibase->ClearWindow(window);
        executive->Permit();
    }
    for(; i<console->bufferIndex;++i){
        ConsolePutCharPrivate(console,console->consoleBuffer[i]);
    }


    
    console-> windowClear = false;
}

void ConsoleWriteString(console_t* console, char* str){
    
   // window_t* window = console->window;
    
    uint32_t size = strlen(str);
    
    for (size_t i = 0; i < size; i++){
        ConsolePutChar(console,str[i]);
    }
    
    
}

void ConsoleWriteDec(console_t* console, int32_t n){
    
   // window_t* window = console->window;
    
    if (n == 0){
         ConsolePutChar(console,'0');
         return;
     }
    
    //Handle negative numbers
    bool negative = false;
    if(n<0){negative = true;}
    
    n = abs(n);
     
    int32_t acc = n;
    char c[32];
    
    int i = 0;
        
     while (acc > 0){
         c[i] = '0' + acc%10;
         acc /= 10;
         i++;
     }
     
 

     
    if(negative==true){
        c[i] = '-';
        i += 1;
    }
    
    c[i] = 0;
    
     char c2[32];
     c2[i--] = 0;
     int j = 0;
     while(i >= 0){
         c2[i--] = c[j++];
     }
     
    ConsoleWriteString(console, c2);
    
}


void ConsoleWriteHex(console_t* console, uint32_t n){
    int32_t tmp;
     
   // window_t* window = console->window;
    
    // debug_write_string("0x");
    ConsoleWriteString(console, "0x");
    
     char noZeroes = 1;
     
     int i;
     
     for (i = 28; i > 0; i -= 4){
         tmp = (n >> i) & 0xF;
         if (tmp == 0 && noZeroes != 0){
             continue;
         }
         
         if (tmp >= 0xA){
             noZeroes = 0;
             //debug_putchar(tmp - 0xA + 'a');
             ConsolePutChar(console,tmp - 0xA + 'a');
         }else{
             noZeroes = 0;
             //debug_putchar(tmp + '0');
             ConsolePutChar(console,tmp + '0');
         }
     }
     
     tmp = n & 0xF;
     if (tmp >= 0xA){
         //debug_putchar(tmp - 0xA + 'a');
         ConsolePutChar(console,tmp - 0xA + 'a');
     }else{
         //debug_putchar(tmp + '0');
         ConsolePutChar(console,tmp + '0');
     }
   
}


void ConsoleBackSpace(console_t* console){
    
   // window_t* window = console->window;
    
    ConsoleClearCursor(console);
    console->x -= 1;
    console->bufferIndex -= 1;
    
    if(console->x<0){
        console->x = console->width;
        console->y -= 1;
    }
    ConsoleDrawCursor(console,console->x,console->y);
}


void ConsoleSize(console_t* console){
    
    window_t* window = console->window;
    
    console->width = (window->innerW / 8) - 2;
    console->height = ((window->innerH - 22) / 16) - 1;
}

void ConsoleMoveCursorX(console_t* console, int rx){
    
    while(console->x<rx){
        ConsolePutChar(console,' ');
    }
    
}








char strClockTaskName[] = "clock";


//DEBUGGING TASKS!!!!!
/*
int outputee(){
    
    //executive->thisTask->node.name = "No window";
     
    messagePort_t* rep = CreatePort("Reply here!");
    
    message_t* mess = NULL;
    
    
    messagePort_t* testPort = NULL;
    
    while(1){
        
                
        //debug_write_hex((uint32_t)&mess);debug_write_string(" -> ");
            
                if(testPort != NULL){
                    
                    mess = (message_t*)executive->Alloc(sizeof(message_t));
                    
                    if(mess != NULL){
                        
                        mess->replyPort = rep;
                        PutMessage(testPort,mess);
                        
                        
                    }else{
                        debug_write_string("No memory for message!\n");
                    }
                    
                }else{
                    testPort = FindPort("Monkey");
                    
                    if(testPort==NULL){
                        debug_write_string("Sender Task: Can't find port\n");
                    }
                }
                
       WaitMS(64);
    }
    
    return 0;
}
*/


void clockDrawFace(intuition_t* intuibase, window_t* win,int ticks, float* x, float* y){
        
    int minuteMarker = ticks / 60;
    int hourMarker = ticks / 12;
    
    //DrawFace
    executive->Forbid();    //we don't save the FPU registers yet... so we can't context switch in the middle of this
    intuibase->DrawCircle(win,100,125,90,intuition.black,false);
    intuibase->DrawCircle(win,100,125,89,intuition.black,false);
    intuibase->FloodFill(win,100,125,intuition.white);
        
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
    executive->Permit();
}

void clockDrawHand(intuition_t* intuibase, window_t* win,int ticks,int pos,int len, int wid, float* x, float* y, uint32_t colour){
    
    
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

void clockEraseHand(intuition_t* intuibase, window_t* win,int ticks,int pos,int len, int wid, float* x, float* y, uint32_t colour){

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



void clock(){
    
    intuition_t* intuibase =  (intuition_t*)executive->OpenLibrary("intuition.library",0);
    
    window_t* clockWin = intuibase->OpenWindow(NULL, 20, 20, 200, 250,WINDOW_TITLEBAR | WINDOW_DRAGGABLE | WINDOW_DEPTH_GADGET | WINDOW_CLOSE_GADGET,"Clock");
    



    executive->Forbid();
    int ticks = 480;
    float count = -1.570796327;
    float inc = (2*PI) / (float)ticks;
    
    
    float* x = executive->AllocMem(sizeof(float)*ticks,0);
    float* y = executive->AllocMem(sizeof(float)*ticks,0);

    for(int i = 0;i<ticks;++i){
        y[i] = sin(count);
        x[i] = cos(count);
        count += inc;
    }
    executive->Permit();
    

    
    clockDrawFace(intuibase,clockWin,ticks,x,y);

    
    int hour = 0;
    int minute = 0;
    int minuteMarker = ticks / 60;
    int hourMarker = ticks / 12;

    int waitTime = (int)(1000.0 *(60.0/(float)ticks));
    int i = 0;
    while(1){
        
        executive->Forbid();    //we don't save the FPU registers yet... so we can't context switch in the middle of this
        int y2 = (int)(y[i]*74.0);
        int x2 = (int)(x[i]*74.0);
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
        

        clockDrawHand(intuibase,clockWin,ticks,minute,74,4,x,y,intuibase->black);
        clockDrawHand(intuibase,clockWin,ticks,hour,55,6,x,y,intuibase->black);
        intuibase->DrawLine(clockWin,100,125,x2+100,y2+125,intuibase->orange);
        
        WaitMS(waitTime);
        //WaitMS(1);
        intuibase->DrawLine(clockWin,100,125,x2+100,y2+125,intuibase->white);
        clockEraseHand(intuibase,clockWin,ticks,hour,55,6,x,y,intuibase->white);
        clockEraseHand(intuibase,clockWin,ticks,minute,74,4,x,y,intuibase->white);

    }
    
}





int over(){
    
    int ballX = 40;
    int ballY = 40;
    int ballVX = 1;
    int ballVY = 1;
    
    intuition_t* intuibase =  (intuition_t*)executive->OpenLibrary("intuition.library",0);
    
    window_t* gfxTest = intuibase->OpenWindowPrivate(NULL,650,550,320,200,WINDOW_TITLEBAR | WINDOW_DRAGGABLE | WINDOW_DEPTH_GADGET,"GFX Drawing Test");
    
    intuibase->SetScreenTitle(gfxTest,"Program draws Amiga Vector Images!");
    gfxTest->isBusy = true;
    
    intuibase->DrawRectangle(gfxTest,30,30,200,160,intuibase->white);
    intuibase->DrawLine(gfxTest,5,25,50,90,intuition.black);
    intuibase->DrawCircle(gfxTest,50,55,32,intuition.black, false);
    intuibase->FloodFill(gfxTest,50,55,intuition.orange);
    
    intuibase->DrawVectorImage(gfxTest,70,40,kickStartBootImage);
    
    
    window_t* under = intuibase->OpenWindowPrivate(NULL, 600, 420, 200, 120,WINDOW_TITLEBAR | WINDOW_DRAGGABLE | WINDOW_DEPTH_GADGET, "Over");
    intuibase->Focus(under);
    
    intuibase->SetScreenTitle(under,"Yet another Random Test Program");

    
    
    while(1) {
        
            //animate one window
            intuibase->DrawRectangle(under, ballX, ballY, 9, 9, under->backgroundColour);
            ballX += ballVX;
            ballY += ballVY;
        
            if(ballX>186 || ballX<6){ballVX = -ballVX;}
            if(ballY>106 || ballY<24){ballVY = -ballVY;}
        
            intuibase->DrawCircle(under,ballX+4,ballY+4,4,intuition.black, false);
            intuibase->FloodFill(under,ballX+4,ballY+4,intuition.orange);
        
        under->needsRedraw = true;
        WaitMS(1);
    }
    
    return 0;
}


/*
int receiverT(){
    
    
    int ballX = 40;
    int ballY = 40;
    int ballVX = 2;
    int ballVY = 2;
    
    intuition_t* intuibase =  (intuition_t*)executive->OpenLibrary("intuition.library",0);
    
    window_t* under = intuibase->OpenWindowPrivate(NULL, 810, 420, 200, 120,WINDOW_TITLEBAR | WINDOW_DRAGGABLE | WINDOW_DEPTH_GADGET | WINDOW_RESIZABLE, "Message Receiver");
    intuibase->WindowToFront(under);
    
    intuibase->SetScreenTitle(under,"Test Program, which only updates when it receives a message from a counting task!");
    
    under->eventPort = executive->CreatePort("eventPort");
    
    messagePort_t* testPort = executive->CreatePort("Monkey");
    
    AddPort(testPort);
    

    
    while(1) {
        

        uint64_t sigRec = executive->Wait( 1 << testPort->sigNum | 1 << under->eventPort->sigNum);
        
        
        //resize window
        if(sigRec & 1 << under->eventPort->sigNum){

            intuitionEvent_t* event =(intuitionEvent_t*) GetMessage(under->eventPort);
            
            while(event != NULL){
                
                if(event->flags & WINDOW_EVENT_RESIZE){
                    intuibase->ResizeWindow(under, event->window->w - event->mouseXrel, event->window->h - event->mouseYrel);

                    if((uint32_t)ballX > (under->innerW-14)){ballX = 10;}
                    if((uint32_t)ballY > (under->innerH-12)){ballY = 24;}
                    
                }
                
                executive->ReplyMessage((message_t*)event);
                event = (intuitionEvent_t*) GetMessage(under->eventPort);
            }
            
        }
        
        
        //animate one window
        intuibase->DrawRectangle(under, ballX, ballY, 9, 9, under->backgroundColour);
        if(sigRec & 1 << testPort->sigNum){
            
            message_t* message = GetMessage(testPort);
            
            // debug_write_string("->");
            
            while(message != NULL){
            
                ballX += ballVX;
                ballY += ballVY;
                
                if((uint32_t)ballX >= under->innerW-10 || (uint32_t)ballX <= under->innerX+2){ballVX = -ballVX;}
                if((uint32_t)ballY >= under->innerH-10 || (uint32_t)ballY <= under->innerY+2){ballVY = -ballVY;}
                
                //executive->Dealloc((node_t*)message);
                executive->ReplyMessage(message);
                message = GetMessage(testPort);
                //debug_write_string("!");
            }
            //debug_write_string("<-\n");
        }
        
        intuibase->DrawCircle(under,ballX+4,ballY+4,4,intuition.black, false);
        intuibase->FloodFill(under,ballX+4,ballY+4,intuition.orange);
       
    }
    
    return 0;
}
*/








void bouncy(){
    
    int ballX = 40;
    int ballY = 40;
    int ballVX = 1;
    int ballVY = 1;

    
    intuibase =  (intuition_t*)executive->OpenLibrary("intuition.library",0);
    
    if(intuibase == NULL){
        return;
    }
    
    window_t* under = intuibase->OpenWindow(NULL, 0, 0, 200, 120,WINDOW_TITLEBAR | WINDOW_DRAGGABLE | WINDOW_DEPTH_GADGET | WINDOW_RESIZABLE | WINDOW_CLOSE_GADGET | WINDOW_VSYNC,"Bouncy!");
    
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
                    
                    //animate
                    intuibase->DrawRectangle(under, ballX, ballY, 9, 9, under->backgroundColour);
                 
                    
                    ballX += ballVX;
                    ballY += ballVY;
                            
                    if((uint32_t)ballX >= under->innerW-10 || (uint32_t)ballX <= under->innerX+2){ballVX = -ballVX;}
                    if((uint32_t)ballY >= under->innerH-10 || (uint32_t)ballY <= under->innerY+2){ballVY = -ballVY;}
                            

                    intuibase->DrawCircle(under,ballX+4,ballY+4,4,intuition.black, false);
                    intuibase->FloodFill(under,ballX+4,ballY+4,intuition.orange);
                    
                    //debug_write_string("Msg! ->");
                    
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

    //remove self
    executive->RemTask(NULL);
    
}









// Command Interpretor ***************************************************************
//This really should be a seprate task from the console, which should be managed by the console device.

void reportError(console_t* console, char** arguments){
    
    //debug_write_string("Reporting Error");
    //window_t* req;
    
    switch(executive->thisTask->dosError){
        case DOS_ERROR_UNKNOWN_DEVICE:
            
            //req = intuibase->Request("System Request");    //Throw a test Requestor,
            /*
            //fake gadgets
            intuibase->DrawRectangle(req,17,98,63,33,intuibase->orange);
            intuibase->DrawRectangle(req,19,100,59,29,intuibase->white);
            intuibase->DrawRectangle(req,19,102,59,25,intuibase->blue);
            intuibase->DrawRectangle(req,21,104,55,21,intuibase->white);
            
            intuibase->DrawRectangle(req,200+17,98,63,33,intuibase->orange);
            intuibase->DrawRectangle(req,200+19,100,59,29,intuibase->white);
            intuibase->DrawRectangle(req,200+19,102,59,25,intuibase->blue);
            intuibase->DrawRectangle(req,200+21,104,55,21,intuibase->white);
            
            intuibase->DrawString(req,10,30," Error validating disk",intuibase->blue,intuibase->white);
            intuibase->DrawString(req,10,30+32," Disk is unreadable",intuibase->blue,intuibase->white);
            
            intuibase->DrawString(req,32,106,"Test",intuibase->blue,intuibase->white);
            intuibase->DrawString(req,232,106,"Test",intuibase->blue,intuibase->white);
            */
            ConsoleWriteString(console,"Unknown Device\n");
            break;
        case DOS_ERROR_DEVICE_NOT_SPECIFIED:
            ConsoleWriteString(console,"Device not specified\n");
            break;
        case DOS_ERROR_UNABLE_TO_OPEN_DEVICE:
            ConsoleWriteString(console,"Can't open device");
            break;
        case DOS_ERROR_OBJECT_NOT_FOUND:
            ConsoleWriteString(console,"File not found!\n");
            break;
        case DOS_ERROR_NOT_A_DIRECTORY:
            ConsoleWriteString(console,"Not a directory!\n");
            break;
        case DOS_ERROR_OBJECT_NOT_OF_REQUIRED_TYPE:
            ConsoleWriteString(console,"Object not of required type! (Not all devices support filesystem actions)\n");
            break;
            
        default:
            ConsoleWriteString(console,"An unknown error occured... Not very helpful.\n");
            ConsoleWriteString(console,arguments[0]);
            break;
    }
}

void processCommand(console_t* console, int commandLength){
    
    dos_t* dosbase = console->dosbase;
    //debug_write_string("process command\n");
    
    //save last command
    strcpy(console->lastCommand, console->commandBuffer);
    
    //debug_write_string("saved buffer\n");
    
    //remove leading spaces
    int start = 0;
    while(console->commandBuffer[start] == ' ' && start < commandLength){
        start++;
    }
    
    //count arguments
    int count = 1;  // there is alwasy one argument if we get to the process buffer stage
    for(int i=start;i<commandLength;++i){
        
        if(console->commandBuffer[i]==' ' && (console->commandBuffer[i+1]!=' ' && console->commandBuffer[i+1]!=0) ){
            count++;
        }
    }
   
    //create an argument pointer array
    char* arguments[count];
    
    arguments[0] = &console->commandBuffer[start];
    int pos = 0;
    for(int i = start;i<commandLength;++i){
        
        if(console->commandBuffer[i]==' ' && (console->commandBuffer[i+1]!=' ' && console->commandBuffer[i+1]!=0) ){
            pos++;
            console->commandBuffer[i]=0;
            arguments[pos] = &console->commandBuffer[i + 1];
        }else if(console->commandBuffer[i]==' '){
            console->commandBuffer[i]=0; // null terminate the argument
        }
        
    }
    
    
    
    
    //disply the available devices, and add assigns
    if(strcmp(arguments[0],"assign") == 0){

        ConsoleWriteString(console,"Devices:\n");
        
        
        list_t* dosList = &dosbase->dosList;
        node_t* dev = dosList->head;
        
        do{
            ConsoleWriteString(console,dev->name);ConsolePutChar(console,'\n');
            dev = dev->next;
        }while(dev->next != NULL);
    
        return;
    }
    
    
    //Need to set the current directory
    if(strcmp(arguments[0],"cd") == 0){



        
        //if no arguments just print the progdir
        if(count < 2){
            if(executive->thisTask->progdir == NULL){
                ConsoleWriteString(console,"No current directory!\n");
            }else{
                //ConsoleWriteString(console," ");
                ConsoleWriteString(console,executive->thisTask->progdir);
                ConsolePutChar(console,'\n');
            }
            return;
        }
        
        //if arguments, then try to open and check the path is vaild
        //if path is valid then save it to the progdir
        
        file_t* file = dosbase->Open(arguments[1],0);

        
        if(file == NULL){
            reportError(console, arguments); //print the Dos Error
            return;
        }
        
        if(file->isDIR){
            
            char temp[512];
            for(int s=0;s<512;++s){
                temp[s] = 0;    //clear string
            }
            strcpy(temp,arguments[1]);
            
            int tmpLen = strlen(temp);
            
            if(temp[tmpLen-1] =='/' || temp[tmpLen-1] ==':'){
                //ConsoleWriteString(console,"Ok!\n");
            }else{
                temp[tmpLen] = '/';
                temp[tmpLen+1] = 0;
                //ConsoleWriteString(console,"Need to add traing slash\n");
            }
            
            
            if(executive->thisTask->progdir != NULL){
                executive->FreeMem(executive->thisTask->progdir);
            }
            
            executive->thisTask->progdir = (char*) executive->AllocMem(strlen(temp),0);
            strcpy(executive->thisTask->progdir,temp);
            
        }else{
            ConsoleWriteString(console,"Path was valid.. but it was a file.\n");
        }

        
        dosbase->Close(file);
        return;
    }
    
    
    if(strcmp(arguments[0],"avail") == 0){
        
        uint64_t a = executive->allocationTotal / 1024;// / 1024;
        uint64_t d = executive->deallocationTotal / 2024;// / 1024;
        uint64_t c = a - d;
        
        ConsoleWriteString(console,"Allocated: "); ConsoleWriteDec(console,a);ConsolePutChar(console,'\n');
        ConsoleWriteString(console,"Deallocated: "); ConsoleWriteDec(console,d);ConsolePutChar(console,'\n');
        ConsoleWriteString(console,"Allocation Delta: "); ConsoleWriteDec(console,c);ConsolePutChar(console,'\n');
        return;
    }
    
    
    if(strcmp(console->commandBuffer,"debug") == 0){
        executive->debug_show();
        return;
    }
    
    //messy way to implment the dir command :-)
    if(strcmp(console->commandBuffer,"dir") == 0){
        //debug_write_string("Dir!\n");
            
        file_t* file;
        
        if(count<2){
            char temp[512];
            temp[0] = 0;
            file = dosbase->Open(temp,0);
        }else{
        
            file = dosbase->Open(arguments[1],0);
        }
        
        //debug_write_string("Opened!\n");
        
        if(file == NULL){
            
            reportError(console, arguments); //print the Dos Error
                
        }else{
                
            //debug_write_string("Prepare to examine... ");
            directoryStruct_t* ds = dosbase->Examine(file);
            //debug_write_string("Examined!\n");
                
            if(ds==NULL){
                    
                switch(executive->thisTask->dosError){
                    case DOS_ERROR_NOT_A_DIRECTORY:
                        ConsoleWriteString(console,"Not a directory!\n");
                        break;
                            
                    default:
                        ConsoleWriteString(console,"Um... An error occured!\n");
                        break;
                            
                }
                return;
                    
            }
                

            
            //Print directory using the returned structure
            directoryEntry_t* dt = ds->entry;

                
            for(uint32_t i = 0;i<ds->size;i++){
                    
                //Hide hidden
                if(dt[i].isHidden){
                    continue;
                }
                
                ConsoleWriteString(console,"   ");
                ConsoleWriteString(console,dt[i].name);
                ConsoleMoveCursorX(console,32);
                    
                    
                if(dt[i].isDir){
                    ConsoleWriteString(console,"(dir)");
                }else{
                    ConsoleWriteDec(console,dt[i].fileSize);
                }
                    
                //ConsoleWriteString(console," | Cluster: ");
                //ConsoleWriteDec(console,dt[i].cluster);
                    
                if(dt[i].isHidden){
                    ConsoleWriteString(console," (Hidden)");
                }
                    
                ConsolePutChar(console,'\n');
                executive->Reschedule();    //slow the update rate down
            }
            executive->FreeMem(ds);//free the directory structure returned by Examine()
                
            dosbase->Close(file);
        }
            
        ConsolePutChar(console,'\n');
            
        return;
        
    }
    
    //An odd way to implment the echo command :-)
    if(strcmp(arguments[0],"echo") == 0){
        
        if(count > 1){
           // ConsoleWriteString(console,"Openning file:\n");
            file_t* file = dosbase->Open(arguments[1],0);
            
            if(file == NULL){
                
                reportError(console, arguments); //print the Dos Error
                
            }else{
            
                if(file->isDIR){
                    ConsoleWriteString(console,"Not a file.\n");
                    dosbase->Close(file);
                    return;
                }

                ConsoleWriteString(console," Echoing file:\n");
                uint8_t* temp = dosbase->LoadFile(file);

                intuibase->SetBusy(console->window, true); //if the window can't process any events let the user know
                for(uint64_t i=0; i < file->size; ++i){
                    ConsolePutChar(console,temp[i]);
                    
                    for(int j=0; j<1000; ++j){
                        executive->Reschedule();    //slow the update rate down
                    }
                  
                }
                intuibase->SetBusy(console->window, false); //user can do things again.
                
                ConsoleWriteString(console,"\n");
                executive->FreeMem(temp);
                dosbase->Close(file);
            }
            
        }else{
            ConsoleWriteString(console,"Error: No file provided.\n");
        }
        
            return;
    }
    
    //A quick way to crash the machine
    if(strcmp(arguments[0],"guru") == 0){
        volatile char i = 0;
        char j = 128;
        char c = j / i;
        ConsolePutChar(console, c);
        return;
    }
    
    //A quick way to crash the machine
    if(strcmp(arguments[0],"help") == 0){
        ConsoleWriteString(console," Supported Commands:\n");
        ConsoleWriteString(console,"   assign - show the currently mounted devices.\n");
        ConsoleWriteString(console,"   avail - show the amount of free RAM... listfree is more useful.\n");
        ConsoleWriteString(console,"   cd   (Usage: cd path) - changes the current path.\n");
        ConsoleWriteString(console,"   cli - opens a new CLI window.\n");
        ConsoleWriteString(console,"   debug - show debugging console.\n");
        ConsoleWriteString(console,"   dir  (Usage: dir path) - lists files at the path.\n");
        ConsoleWriteString(console,"   echo (Usage: echo filename) - echos the contents of a file to the console.\n");
        ConsoleWriteString(console,"   guru (Usage: guru) - purposely crashes this task.\n");
        ConsoleWriteString(console,"   help (Usage: help) - prints this help.\n");
        ConsoleWriteString(console,"   load (Usage: load filename) - loads an ELF executable into memory... and execute it (does not detach task from CLI!)\n");
        ConsoleWriteString(console,"   listfree - list free memory blocks.\n");
        ConsoleWriteString(console,"   listtasks - list tasks currently running on the system.\n");
        ConsoleWriteString(console,"   run (Usage: run filename) - loads an ELF executable into memory... and runs it (detaches task from CLI!)\n");
        return;
    }
    
    //Load an ELF executable :-)
    if(strcmp(arguments[0],"load") == 0){
        
        if(count > 1){
           // ConsoleWriteString(console,"Openning file:\n");
            file_t* file = dosbase->Open(arguments[1],0);
            
            if(file == NULL){
                
                reportError(console, arguments); //print the Dos Error
                
            }else{
            
                if(file->isDIR){
                    ConsoleWriteString(console,"Not a file.\n");
                    dosbase->Close(file);
                    return;
                }

                executable_t tmp = dosbase->LoadELF(file);
                
                if(tmp.type==0){
                    ConsoleWriteString(console,"Not executable.\n");
                    return;
                }
                
                void (*mn)() = tmp.entry;
                mn();
                executive->FreeMem(tmp.segment);
                
                //ConsoleWriteString(console,"\n");
                dosbase->Close(file);
            }
            
        }else{
            ConsoleWriteString(console,"Error: No file provided.\n");
        }
        
            return;
    }
    
    //Load an ELF Relocatable Object :-)
    if(strcmp(arguments[0],"run") == 0){
        
        if(count > 1){
           // ConsoleWriteString(console,"Openning file:\n");
            file_t* file = dosbase->Open(arguments[1],0);
            
            if(file == NULL){
                
                reportError(console, arguments); //print the Dos Error
                
            }else{
            
                if(file->isDIR){
                    ConsoleWriteString(console,"Not a file.\n");
                    dosbase->Close(file);
                    return;
                }

                executable_t tmp = dosbase->LoadELF(file);
                
                if(tmp.type==0){
                    ConsoleWriteString(console,"Not executable.\n");
                    return;
                }
                
                task_t* task =  executive->CreateTask("PlaceHolder",0,tmp.entry,4096);
                executive->AddTask(task);
                //task_t* task =  executive->AddTask(tmp.entry,4096,0);
                
                //remove the memory segment from this task's memory list and add it to the task's memory list
                //All allocations relating to a task must be recorded in the task's own structure
                //This is so they can be cleaned up upon exit.
                //
                //At the moment, LoadELF just returns the whole file, eventually it will return just the program segment
                //The program segment will be type NODE_TASK_SEGMENT so won't need removing from the calling task's memory list
                executive->Remove(&executive->thisTask->memoryList,(node_t*)tmp.segment);
                tmp.segment->type = NODE_TASK_SEGMENT;  //Only needed because dos currently records the allocation on it's own memory list as data.
                executive->AddTail(&task->memoryList,(node_t*)tmp.segment);
                debug_write_string("Check this Segment ownership code in the run function in cli.c\n");
                
                //ConsoleWriteString(console,"\n");
                dosbase->Close(file);
            }
            
        }else{
            ConsoleWriteString(console,"Error: No file provided.\n");
        }
        
            return;
    }
    
    
    // THese three theming commands don't really work as intution isn't set up for runtime theme swapping yet.
    if(strcmp(arguments[0],"themenew") == 0){
        intuibase->SetTheme(1);
        ConsoleWriteString(console,"AmigaOS 3.1 theme\n");
        return;
    }
    
    if(strcmp(arguments[0],"themeold") == 0){
        intuibase->SetTheme(0);
        ConsoleWriteString(console,"AmigaOS 1.3 theme\n");
        return;
    }
    
    if(strcmp(arguments[0],"thememac") == 0){
        intuibase->SetTheme(2);
        ConsoleWriteString(console,"Classic Mac theme\n");
        return;
    }
    
    if(strcmp(arguments[0],"a") == 0){
        
        executiveRequest_t* test = (executiveRequest_t*)executive->Alloc(sizeof(executiveRequest_t),0);
        
        test->message.replyPort = NULL;
        test->caller = executive->thisTask;
        test->request = EXEC_REQUEST_NOP;
        
        executive->PutMessage(executive->executivePort,(message_t*)test);
        
        task_t* t = executive->CreateTask(strClockTaskName,0,clock,4096);
        executive->AddTask(t);
        
        
        
        return;
    }
  
    if(strcmp(arguments[0],"b") == 0){
        task_t* t = executive->CreateTask("Over!",0,over,4096);
        executive->AddTask(t);
        return;
    }
 
    if(strcmp(arguments[0],"c") == 0){
        task_t* t =  executive->CreateTask("Bouncy!",0,bouncy,4096);
        executive->AddTask(t);
        return;
    }
    
    
    if(strcmp(arguments[0],"cli") == 0){
        task_t* t =  executive->CreateTask("CLI",0,CliEntry,4096);
        executive->AddTask(t);
        return;
    }
    
    if(strcmp(arguments[0],"listfree") == 0){
        

        

        ConsoleWriteString(console,"\n--------------------------------------\n");
        
        uint32_t total = 0;
        uint32_t count = 0;
        
        executive->Forbid();
        node_t* node = executive->freeList.head;
        while(node->next != NULL){
            ConsolePutChar(console,' ');
            ConsoleWriteHex(console,(uint32_t)node);ConsoleWriteString(console,": ");
            ConsoleWriteDec(console,node->size);ConsoleWriteString(console," bytes\n");
            
            total +=node->size;
            count += 1;
            node = node->next;
        }
        executive->Permit();
        
        ConsoleWriteString(console,"--------------------------------------\n");
        ConsoleWriteString(console," Free Blocks: ");ConsoleWriteDec(console,count);ConsolePutChar(console,'\n');
        ConsoleWriteString(console," Total Free: ");ConsoleWriteDec(console,total);ConsoleWriteString(console," bytes\n\n");
        
        ConsoleWriteString(console," Total Allocated: ");ConsoleWriteDec(console,executive->memSize - total);ConsoleWriteString(console," bytes\n\n");

        

        
        
        return;
    }
    
    if(strcmp(arguments[0],"listtasks") == 0){
        

        

        ConsoleWriteString(console,"\n--------------------------------------\n");
        
        
        executive->Forbid();
        node_t* node = executive->taskList.head;
        while(node->next != NULL){
            
            taskListNode_t* TLNode = (taskListNode_t*) node;
            
            ConsolePutChar(console,' ');
            ConsoleWriteHex(console,(uint32_t)TLNode->task);ConsoleWriteString(console,": ");
            ConsoleWriteString(console,TLNode->task->node.name);ConsoleWriteString(console,", Priority: ");
            ConsoleWriteDec(console,TLNode->task->node.priority);ConsoleWriteString(console,"\n");
            
            node = node->next;
        }
        executive->Permit();
        
        return;
    }
    
    ConsoleWriteString(console,"Unknown Command ");
    ConsoleWriteString(console,arguments[0]);
    ConsolePutChar(console,'\n');

}



































int CliEntry(void){
    
    executive->thisTask->parent = NULL; //CLI should have no parent task, upon execution a task should check and see if it has a parent, if it doesn't then then that means it hasn't been detached from CLI and may use the CLI for it's I/O. If a task does have a parent then it must set up it's own I/O.
    
    //set the progdir to the boot disk
    executive->thisTask->progdir = executive->AllocMem(5,0); //need 5 bytes, the name plus null
    strcpy(executive->thisTask->progdir,"dh0:");
    
    console_t* console = executive->AllocMem(sizeof(console_t),0);
    console->commandBufferIndex = 0;
    
    for(int i=0;i<4096;++i){
        console->consoleBuffer[i] = 0;
    }
    console->bufferIndex = 0;
    
    console->windowClear = true;
    
    
    
    
    intuibase = (intuition_t*) executive->OpenLibrary("intuition.library",0);
    
    //console = intuibase->OpenWindowPrivate(NULL,0,22,intuibase->screenWidth,(intuibase->screenHeight/2)-24,WINDOW_TITLEBAR | WINDOW_DRAGGABLE | WINDOW_DEPTH_GADGET | WINDOW_RESIZABLE, "BootShell");
    
    console->window = intuibase->OpenWindow(NULL,0,22,intuibase->screenWidth,(intuibase->screenHeight)-24,WINDOW_TITLEBAR | WINDOW_DRAGGABLE | WINDOW_DEPTH_GADGET | WINDOW_RESIZABLE | WINDOW_KEYBOARD, "BootShell");
    intuibase->Focus(console->window);
    
    
    console->window->eventPort = executive->CreatePort("Event Port");

    ConsoleSize(console);
    console->x = 0;
    console->y = 0;
    console->conCurX = (console->x*8)+4;
    console->conCurY = (console->y*16)+22;

     // This bootshell task, this should be responsible for much of the system set up
    
    ConsoleWriteString(console,"Copyright ");
    ConsolePutChar(console,169);
    ConsoleWriteString(console,"2021 Matt Parsons.\nAll rights reserved.\nRelease 0.47a\n\n");
    

 
    
    //Initilise and open DOS library
    console->dosbase = (dos_t*) executive->OpenLibrary("dos.library",0);    // never use a library without opening it first

    
    debug_write_string("CLI MESSAGE: Type help for available commands.\n");
    ConsoleWriteString(console,"1> ");
    while(1){
        
        //debug_write_string("BootShell: Wait Event\n");
        
        executive->WaitPort(console->window->eventPort);
        intuitionEvent_t* event = (intuitionEvent_t*) executive->GetMessage(console->window->eventPort);
    
        while(event != NULL){
        
            if(event->flags & WINDOW_EVENT_RESIZE){
                intuibase->ResizeWindow(console->window, console->window->w - event->mouseXrel, console->window->h - event->mouseYrel);
                console->windowClear = true; //window is alwasy clear after a resize event!
                ConsoleSize(console);
            }
            
            if(event->flags & WINDOW_EVENT_RESIZE_END){
                ConsoleRedraw(console);
            }
            
            
            if(event->flags & WINDOW_EVENT_KEYDOWN){
            

                
                    if(event->rawKey == 0xE){
                        //delete key
                        
                        if(console->commandBufferIndex > 0){
                            console->commandBufferIndex -=1;
                            ConsoleBackSpace(console);
                        }
                        
                    }else if(event->rawKey == 0x1C){
                        // enter key
                        ConsolePutChar(console,'\n');   // newline as enter has been pressed
                        console->commandBuffer[console->commandBufferIndex]= 0; // append a NULL to the command buffer

                        
                        if(console->commandBufferIndex>0){
                            
                           processCommand(console, console->commandBufferIndex);
                
                        }
                        
                        console->commandBufferIndex  = 0;
                        ConsoleWriteString(console,"1> ");
                        
                    }else if(event->rawKey == 0x48){
                        //up cursor
                        
                        //Delete current line
                        while(console->commandBufferIndex>0){
                            console->commandBufferIndex -=1;
                            ConsoleBackSpace(console);
                        }
                        
                        //copy command
                        int i = 0;
                        while(console->lastCommand[i] != 0){
                            console->commandBuffer[console->commandBufferIndex] = console->lastCommand[i];
                            console->commandBufferIndex += 1;
                            ConsolePutChar(console, console->lastCommand[i]);
                            i++;
                        }
                            
                        
                        
                        
                    }else{
                        if(console->commandBufferIndex < 255){
                            console->commandBuffer[console->commandBufferIndex] = event->scancode;
                            console->commandBufferIndex += 1;
                            ConsolePutChar(console, event->scancode);
                        }
                    }
        
                
            }

            
            executive->ReplyMessage((message_t*)event); // Always reply a message as soon as possible
            //get next message
            event = (intuitionEvent_t*) executive->GetMessage(console->window->eventPort);
        }
    
    }
}
