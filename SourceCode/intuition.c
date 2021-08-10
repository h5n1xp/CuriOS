//
//  intuition.c
//
//  Created by Matt Parsons on 26/10/2020.
//  Copyright © 2020 Matt Parsons. All rights reserved.
//

#include "intuition.h"
#include "memory.h"
#include "string.h"
#include "input.h"

#include "SystemLog.h"

#define THEME_OLD 0
#define THEME_NEW 1
#define THEME_MAC 2
#define THEME_GEM 3     //perhaps implement a classic Atari GEM theme too?

int guiTheme = THEME_NEW;

// time to rewrite the pointer code?
//the normal pointers should be 11px by 11px, and should include an indicator to show if they need to be scaled
/*
typedef struct{
    node_t node;
    uint32_t type;
    int width;
    int height;
    int scale;
    uint8_t* image;
} sprite_t;
*/

uint8_t oldMousePointer[] ={
1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,
1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,
1,1,2,2,2,2,2,2,2,2,2,2,1,1,0,0,0,0,0,0,0,0,
1,1,2,2,2,2,2,2,2,2,2,2,1,1,0,0,0,0,0,0,0,0,
1,1,3,3,3,3,3,3,3,3,2,2,1,1,0,0,0,0,0,0,0,0,
1,1,3,3,3,3,3,3,3,3,2,2,1,1,0,0,0,0,0,0,0,0,
1,1,3,3,3,3,3,3,2,2,1,1,0,0,0,0,0,0,0,0,0,0,
1,1,3,3,3,3,3,3,2,2,1,1,0,0,0,0,0,0,0,0,0,0,
1,1,3,3,3,3,3,3,3,3,2,2,1,1,0,0,0,0,0,0,0,0,
1,1,3,3,3,3,3,3,3,3,2,2,1,1,0,0,0,0,0,0,0,0,
1,1,3,3,3,3,1,1,3,3,3,3,2,2,1,1,0,0,0,0,0,0,
1,1,3,3,3,3,1,1,3,3,3,3,2,2,1,1,0,0,0,0,0,0,
0,0,1,1,1,1,0,0,1,1,3,3,3,3,2,2,1,1,0,0,0,0,
0,0,1,1,1,1,0,0,1,1,3,3,3,3,2,2,1,1,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,1,1,3,3,3,3,2,2,1,1,0,0,
0,0,0,0,0,0,0,0,0,0,1,1,3,3,3,3,2,2,1,1,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,1,1,3,3,3,3,2,2,1,1,
0,0,0,0,0,0,0,0,0,0,0,0,1,1,3,3,3,3,2,2,1,1,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,3,3,1,1,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,3,3,1,1,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0};

uint8_t newMousePointer[] ={
3,3,2,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
3,3,2,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
1,1,3,3,2,2,2,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
1,1,3,3,2,2,2,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,1,1,3,3,3,3,2,2,2,2,0,0,0,0,0,0,0,0,0,0,
0,0,1,1,3,3,3,3,2,2,2,2,0,0,0,0,0,0,0,0,0,0,
0,0,1,1,3,3,3,3,3,3,3,3,2,2,2,2,0,0,0,0,0,0,
0,0,1,1,3,3,3,3,3,3,3,3,2,2,2,2,0,0,0,0,0,0,
0,0,0,0,1,1,3,3,3,3,3,3,3,3,3,3,2,2,2,2,0,0,
0,0,0,0,1,1,3,3,3,3,3,3,3,3,3,3,2,2,2,2,0,0,
0,0,0,0,1,1,3,3,3,3,3,3,3,3,3,3,3,3,3,3,0,0,
0,0,0,0,1,1,3,3,3,3,3,3,3,3,3,3,3,3,3,3,0,0,
0,0,0,0,0,0,1,1,3,3,3,3,3,3,2,2,0,0,0,0,0,0,
0,0,0,0,0,0,1,1,3,3,3,3,3,3,2,2,0,0,0,0,0,0,
0,0,0,0,0,0,1,1,3,3,3,3,1,1,3,3,2,2,0,0,0,0,
0,0,0,0,0,0,1,1,3,3,3,3,1,1,3,3,2,2,0,0,0,0,
0,0,0,0,0,0,0,0,1,1,3,3,0,0,1,1,3,3,2,2,0,0,
0,0,0,0,0,0,0,0,1,1,3,3,0,0,1,1,3,3,2,2,0,0,
0,0,0,0,0,0,0,0,1,1,3,3,0,0,0,0,1,1,3,3,2,2,
0,0,0,0,0,0,0,0,1,1,3,3,0,0,0,0,1,1,3,3,2,2,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,3,3,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,3,3};
    
uint8_t macMousePointer[] ={
4,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
4,1,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
4,1,1,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
4,1,1,1,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
4,1,1,1,1,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
4,1,1,1,1,1,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
4,1,1,1,1,1,1,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
4,1,1,1,1,1,1,1,4,0,0,0,0,0,0,0,0,0,0,0,0,0,
4,1,1,1,1,1,1,1,1,4,0,0,0,0,0,0,0,0,0,0,0,0,
4,1,1,1,1,1,4,4,4,4,4,0,0,0,0,0,0,0,0,0,0,0,
4,1,1,4,1,1,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
4,1,4,0,4,1,1,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
4,4,0,0,4,1,1,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
4,0,0,0,0,4,1,1,4,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,4,1,1,4,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,4,4,4,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    

uint8_t oldBusyPointer[] ={
0,0,0,0,0,4,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,4,4,4,4,0,4,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,4,4,4,4,4,4,4,4,4,0,0,0,0,0,0,0,0,0,0,0,
0,4,4,4,4,4,4,4,4,4,4,0,0,0,0,0,0,0,0,0,0,0,
0,4,4,1,1,1,1,4,4,4,4,4,0,0,0,0,0,0,0,0,0,0,
0,4,4,4,4,1,4,4,4,4,4,4,4,0,0,0,0,0,0,0,0,0,
4,4,4,4,1,4,4,4,4,4,4,4,4,0,0,0,0,0,0,0,0,0,
0,4,4,1,1,1,1,4,4,4,4,4,4,4,0,0,0,0,0,0,0,0,
0,4,4,4,4,4,4,4,1,1,1,1,4,4,0,0,0,0,0,0,0,0,
0,0,4,4,4,4,4,4,4,4,1,4,4,4,4,0,0,0,0,0,0,0,
0,4,4,4,4,4,4,4,4,1,4,4,4,4,0,0,0,0,0,0,0,0,
0,0,4,4,4,4,4,4,1,1,1,1,4,4,0,0,0,0,0,0,0,0,
0,0,0,4,4,4,4,4,4,4,4,4,4,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,4,4,4,4,4,4,4,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,4,4,4,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,4,4,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,4,4,4,4,4,4,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,4,4,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,4,4,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,4,4,4,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0};
    
intuition_t intuition;

//list_t windowList;

window_t* windowUnder;
gadget_t* gadgetUnder;
uint32_t mouseOverX;
uint32_t mouseOverY;

window_t* dragging = NULL;
window_t* resizing = NULL;
uint32_t resizeHack = 0;

char* screenTitle;

bool mouseVisible = false;
bool leftMouseHeld = false;

volatile int32_t mouseX=0;
volatile int32_t mouseY=0;
int32_t mouseXold = 0;
int32_t mouseYold = 0;


//forward declare these functions
void drawWindowDecoration(window_t*);
void RedrawWindow(window_t* window);
void WindowToBack(window_t* window);
void WindowToFront(window_t* window);
void Focus(window_t* window);


void updateMouse(){
    // The inaccurately named "UpdateMouse" function, handles all mouse state changes for intution, and dispatches events accordingly
    if(inputStruct.rawMouse[3] == 0){
        return;
    }
    
    //Move mouse
    mouseXold = mouseX;
    mouseYold = mouseY;
    
    mouseX += inputStruct.rawMouse[1];
    mouseY -= inputStruct.rawMouse[2];  // y is negative for some reason
    
    
    inputStruct.rawMouse[1] = 0;
    inputStruct.rawMouse[2] = 0;
    inputStruct.rawMouse[3] = 0;
    
    if(mouseX < 0){
        mouseX = 0;
    }
    
    if((uint32_t)mouseX > (graphics.frameBuffer.width-1)){
        mouseX = graphics.frameBuffer.width-1;
    }
    
    
    if(mouseY < 0){
        mouseY = 0;
    }
    
    if((uint32_t)mouseY > (graphics.frameBuffer.height-1)){
        mouseY = graphics.frameBuffer.height-1;
    }
    
    //unsigned versions of mouse variables for comparisions
    uint32_t mx = (uint32_t) mouseX;
    uint32_t my = (uint32_t) mouseY;
    
    //check if threre is a mouse event in progress...
    
    if(leftMouseHeld == true && ( (inputStruct.rawMouse[0] & 1) == 1) ){
        
        if(dragging != NULL){
            
            intuition.MoveWindow(dragging, mx - dragging->mouseX, my - dragging->mouseY);
            
        }else{
            
            if(gadgetUnder != NULL){
                
                if(gadgetUnder->MouseMoved !=NULL){
                    gadgetUnder->MouseMoved(gadgetUnder);
                }
                
                
                //convert gadget coordinates to screen coordinates
                uint32_t gx1 = gadgetUnder->window->x + gadgetUnder->ix;
                uint32_t gy1 = gadgetUnder->window->y + gadgetUnder->iy;
                uint32_t gx2 = gx1 + gadgetUnder->w;
                uint32_t gy2 = gy1 + gadgetUnder->h;
                
                if(mx>=gx1 && mx<=gx2 && my>=gy1 && my<=gy2){
                    
                    if(gadgetUnder->state != GADGET_STATE_ACTIVE){
                        gadgetUnder->state = GADGET_STATE_ACTIVE;
                        gadgetUnder->Draw(gadgetUnder);
                        gadgetUnder->mouseState = inputStruct.rawMouse[0];
                        RedrawWindow(gadgetUnder->window);
                    }
                }else{
                    gadgetUnder->mouseState = 0;
                    gadgetUnder->state = GADGET_STATE_NORMAL;
                    gadgetUnder->Draw(gadgetUnder);
                    RedrawWindow(gadgetUnder->window);
                }
            }
        }
        
        
        return;
    }else{
        leftMouseHeld = false;
        gadgetUnder=NULL;
        
    }
    
    //Handle mouse position and button events
    Lock(&intuition.windowList->lock);
    
    node_t* node = intuition.windowList->pred;
    
    
    //Check if we are over a window
    while(node->prev != NULL){
        window_t* window = (window_t*)node;
        
        
        if(mx >= window->x && mx <= (window->x + window->w) && my >= window->y && my <= (window->y + window->h) ){
            
            windowUnder = window;
            break;
        }
        
        node = node->prev;
    }

    FreeLock(&intuition.windowList->lock);
    
    windowUnder->mouseX  = mouseX - windowUnder->x;
    windowUnder->mouseY  = mouseY - windowUnder->y;
        
    
    //check if we need to focus and/or Raise the window
    
    if( (inputStruct.rawMouse[0] & 1) == 1){
        
        if( (windowUnder->flags & WINDOW_DEPTH_GADGET) == WINDOW_DEPTH_GADGET){
            if(executive->ticks <= windowUnder->doubleClick){
                WindowToFront(windowUnder);
            }else{
                windowUnder->doubleClick = executive->ticks + intuition.doubleClickTime;
            }
        }
        
        //double clicking a window to front is an Amiga thing, most operating systems autorise active windows
        if(intuition.windowAutorise==true && windowUnder->noRise == false){
            WindowToFront(windowUnder);
            windowUnder->doubleClick = 0;
        }
        
        if(windowUnder->focused == false){
            
            Focus(windowUnder);
            gadgetUnder = NULL;
            dragging = NULL;
        }
        
    }
    

    //Check if the window is busy
    //really this should only check the inner window (window borders shouldn't be busy) and if true then stop processing
    if(windowUnder->isBusy){
        
        //if(windowUnder->mouseX>windowUnder->innerX && windowUnder->mouseX<windowUnder->innerW){
         //   if(windowUnder->mouseY>windowUnder->innerY && windowUnder->mouseX<windowUnder->innerH){
                intuition.mouseImage = intuition.busyMouseImage;
        //    }
       // }
        
        
    }else{
        intuition.mouseImage = intuition.normalMouseImage;
    }
    
    
    //check if we are over a gadget
    node = windowUnder->gadgetList.head;
    gadget_t* gadgetCheck = NULL;
    while(node->next != NULL){
        gadget_t* gadget = (gadget_t*) node;
        
        if(windowUnder->mouseX >= gadget->ix && windowUnder->mouseX <= (gadget->ix + gadget->w) && windowUnder->mouseY >= gadget->iy && windowUnder->mouseY <= (gadget->iy + gadget->h) ){
    
            gadgetCheck = gadget;
            break;
        }
        
        
        node = node->next;
    }
    
    //If no gadget is selected check if we want to drag
    if((windowUnder->flags & WINDOW_DRAGGABLE) == WINDOW_DRAGGABLE){
        
        if(my <= (windowUnder->y + windowUnder->innerY) ){
            dragging = windowUnder;
        }else{
            dragging = NULL;
        }
        
    }

    if(gadgetCheck != NULL){
        dragging = NULL;
        
        if(gadgetCheck != gadgetUnder){

            if(gadgetUnder !=NULL){
                gadgetUnder->mouseState = 0;
                gadgetUnder->state = GADGET_STATE_NORMAL;
                gadgetUnder->Draw(gadgetUnder);
                intuition.RedrawWindow(gadgetUnder->window);
            }
            gadgetUnder = gadgetCheck;
            gadgetUnder->state = GADGET_STATE_NORMAL;
            gadgetUnder->Draw(gadgetUnder);
            intuition.RedrawWindow(gadgetUnder->window);
            
        }
        
        if(inputStruct.rawMouse[0]&1){
            windowUnder->doubleClick = 0; //cancel any chance of an autorise
            gadgetUnder->state = GADGET_STATE_ACTIVE;
            gadgetUnder->mouseState = inputStruct.rawMouse[0];
            gadgetUnder->Draw(gadgetUnder);
            intuition.RedrawWindow(gadgetUnder->window);
            
            if(executive->ticks<=gadgetUnder->doubleClickTimeOut){
                
                if(gadgetUnder->MouseDoubleClick != NULL){
                    gadgetUnder->MouseDoubleClick(gadgetUnder);
                    return;
                  }
                
            }
            
            gadgetUnder->doubleClickTimeOut = executive->ticks + intuition.doubleClickTime;
            
            if(gadgetUnder->MouseDown != NULL){
                gadgetUnder->MouseDown(gadgetUnder);
            }
        }else if(gadgetUnder->mouseState & 1 ){
            gadgetUnder->state = GADGET_STATE_NORMAL;
            gadgetUnder->mouseState = inputStruct.rawMouse[0];
            gadgetUnder->Draw(gadgetUnder);
            intuition.RedrawWindow(gadgetUnder->window);
            if(gadgetUnder->MouseUp != NULL){
                gadgetUnder->MouseUp(gadgetUnder);
            }
            //debug_write_string("up");
            gadgetUnder = NULL;
        }else{
            //No Hover state right now
           // debug_write_string("Hover");
        }
        

        
        
        
    }else{

        if(gadgetUnder !=NULL){
            // Cancel any action
            gadgetUnder->mouseState = 0;
            gadgetUnder->state = GADGET_STATE_NORMAL;
            gadgetUnder->Draw(gadgetUnder);
            intuition.RedrawWindow(gadgetUnder->window);
        }
        
        
    }
    

    if( (inputStruct.rawMouse[0] & 1) == 1){
    
        leftMouseHeld = true;
        
    }
    
    
}
















//*************** Assembly highspeed copy
inline void movsd(void* dst, const void* src, size_t size){
    asm volatile("rep movsl" : "+D"(dst), "+S"(src), "+c"(size) : : "memory");
}

uint32_t mouseBuffer[22*22];

void clearMouse(){
    
    uint32_t index = 0;
    uint32_t* p = graphics.frameBuffer.buffer;
    
    /*
    uint32_t w = (uint32_t)mouseX + 22;
    uint32_t h = (uint32_t)mouseY + 22;
    
    if(w >= graphics.frameBuffer.width){w = graphics.frameBuffer.width;}
    if(h >= graphics.frameBuffer.height){h = graphics.frameBuffer.height;}
    
    for(uint32_t y = (uint32_t)mouseY;y < h;++y){
        for(uint32_t x = (uint32_t)mouseX; x < w;++x){
            
            //if(x<graphics.frameBuffer.width && y<graphics.frameBuffer.height){
                p[(y * graphics.frameBuffer.width) + x] =  mouseBuffer[index];
            //}
            index += 1;
        }
    }
    */

    uint32_t w = 22;
    uint32_t h = 22;
    
    
    if(mouseX + w >= graphics.frameBuffer.width){w = graphics.frameBuffer.width - mouseX;}
    if(mouseY + h >= graphics.frameBuffer.height){h = graphics.frameBuffer.height - mouseY;}
    
    for(uint32_t y = 0; y<h;++y){
        
        movsd(&p[((mouseY+y)*graphics.frameBuffer.width)+mouseX],&mouseBuffer[index],w);
        
        index += 22;
        
    }
    
}



void drawMouse(){
    
    uint32_t index = 0;
    uint32_t* p = graphics.frameBuffer.buffer;
    
    

    
    
    
    //save background
    /*
    for(uint32_t y = (uint32_t)mouseY;y < h;++y){
        for(uint32_t x = (uint32_t)mouseX; x < w;++x){
            
                mouseBuffer[index] = p[(y * graphics.frameBuffer.width) + x];
            
            index += 1;
        }
    }
    */
    
    uint32_t w = 22;
    uint32_t h = 22;
    
    
    if(mouseX + w >= graphics.frameBuffer.width){w = graphics.frameBuffer.width - mouseX;}
    if(mouseY + h >= graphics.frameBuffer.height){h = graphics.frameBuffer.height - mouseY;}
    
    for(uint32_t y = 0; y<h;++y){
        
        movsd(&mouseBuffer[index],&p[((mouseY+y)*graphics.frameBuffer.width)+mouseX],w);
        
        index += 22;
        
    }
    
    
    
    //draw pointer
    w = (uint32_t)mouseX + 22;
    h = (uint32_t)mouseY + 22;
    
    //if(w >= graphics.frameBuffer.width){w = graphics.frameBuffer.width;}
    //if(h >= graphics.frameBuffer.height){h = graphics.frameBuffer.height;}
    
    index = 0;
    for(uint32_t y = (uint32_t)mouseY; y < h; ++y){
        for(uint32_t x = (uint32_t)mouseX; x < w; ++x){
            
            if(x<graphics.frameBuffer.width && y<graphics.frameBuffer.height){
                
                switch(intuition.mouseImage[index]){
                    case 0:
                        break;
                    case 1:
                        p[(y * graphics.frameBuffer.width) + x] =  0;   //black
                        break;
                    case 2:
                        p[(y * graphics.frameBuffer.width) + x] =  intuition.grey;
                        break;
                    case 3:
                        p[(y * graphics.frameBuffer.width) + x] =  intuition.red;
                        break;
                    case 4:
                        p[(y * graphics.frameBuffer.width) + x] =  intuition.white;
                        break;
                    default:
                        break;
                }

                
            }
            index += 1;
        }
    }
    
    return;
    
}

void RedrawWindow(window_t* window){
    
    if(window->needsRedraw == true){
        return;
    }
    
    window->needsRedraw = true;
    
    Lock(&window->clipRectsLock);
    for(uint32_t i = 0;i<window->clipRects;++i){
        window->clipRect[i].needsUpdate = true;
    }
    FreeLock(&window->clipRectsLock);
    
    window->autoRefeshCountdown = 60;
    
}

void drawTitleBar(){
    
    if(inputStruct.screenTitle == NULL){
        return;
    }
    
    if(screenTitle == NULL){
        screenTitle = "System Screen";
    }
    
    graphics.DrawRect(inputStruct.screenTitle->bitmap, 0,0,graphics.frameBuffer.width,20,intuition.white);
    graphics.RenderString(inputStruct.screenTitle->bitmap,intuition.defaultFont, 4,2,screenTitle,intuition.blue,intuition.white);
    RedrawWindow(inputStruct.screenTitle);
}


void RedrawBlitRects(window_t* window){
 
    //if the cliprects are locked then don't bother redrawing.
    if(TestLock(&window->clipRectsLock)){
        return;
    }
    
    Lock(&window->clipRectsLock);
    
    for(uint32_t i = 0; i<window->clipRects; ++i){
        if(window->clipRect[i].isVisible){
            
            if(window->clipRect[i].needsUpdate == true){
                
                //graphics.DrawRect(&graphics.frameBuffer, window->x+window->clipRect[i].x,window-> y+ window->clipRect[i].y, window->clipRect[i].w,  window->clipRect[i].h, intuition.black);
                //graphics.DrawRect(&graphics.frameBuffer, window->x+window->clipRect[i].x+1,window-> y+ window->clipRect[i].y+1, window->clipRect[i].w-2,  window->clipRect[i].h-2, intuition.orange);
                
                graphics.BlitRect(window->bitmap, window->clipRect[i].x, window->clipRect[i].y, window->clipRect[i].w, window->clipRect[i].h, &graphics.frameBuffer, window->x+window->clipRect[i].x, window->y+window->clipRect[i].y);
                
                
                //Draw Cliping Box - Debugging Code
                /*
                graphics.DrawRect(&graphics.frameBuffer,window->x+window->clipRect[i].x,window->y + window->clipRect[i].y,window->clipRect[i].w,1,intuition.black);
                graphics.DrawRect(&graphics.frameBuffer,window->x+window->clipRect[i].x,window->y + window->clipRect[i].y+window->clipRect[i].h,window->clipRect[i].w,1,intuition.black);
                
                graphics.DrawRect(&graphics.frameBuffer,window->x+window->clipRect[i].x,window->y + window->clipRect[i].y,1,window->clipRect[i].h,intuition.orange);
                graphics.DrawRect(&graphics.frameBuffer,window->x+window->clipRect[i].x+window->clipRect[i].w,window->y + window->clipRect[i].y,1,window->clipRect[i].h,intuition.orange);
                
                graphics.RenderString(&graphics.frameBuffer,intuition.defaultFont,window->x + window->clipRect[i].x+(window->clipRect[i].w/2) - 18,window->y + window->clipRect[i].y+(window->clipRect[i].h/2)-8,"Clip",intuition.black,intuition.white);
                */
                
                window->clipRect[i].needsUpdate = false;
            }
        }
    }
    FreeLock(&window->clipRectsLock);
    
    window->needsRedraw = false;
}

void updateLayers(window_t*); //forward declaration...

void IntuitionUpdate(void){
    
    //To be called every Vertical Blanking interrupt.

    clearMouse();
    updateMouse();
    
    drawTitleBar();
    
    //Resizing hack, to display window content when mouse stops moving
    
    if(resizing != NULL){
        if(resizing->eventPort != NULL){
            
            if(resizeHack<2){
            intuitionEvent_t* event = (intuitionEvent_t*) executive->Alloc(sizeof(intuitionEvent_t));
                event->flags = WINDOW_EVENT_RESIZE_END;
                event->message.replyPort = NULL;
                //event->window = resizing;
                //event->mouseX = mouseX;
                //event->mouseY = mouseY;
                //event->mouseXrel = mouseXold - mouseX;
                //event->mouseYrel = mouseYold - mouseY;
                executive->PutMessage(resizing->eventPort,(message_t*)event);
                intuition.needsUpdate = true;
                resizeHack +=1;
            }
        }
    }
    
    //don't update if there are no windows or the window list is locked
    if(intuition.windowList->count == 0 || TestLock(&intuition.windowList->lock)){
        drawMouse();
        return;
    }

    
    if(intuition.needsUpdate){
        updateLayers(NULL);
        intuition.needsUpdate = false;
    }
   
    
    //This is the proper way to draw the window rects, front to back. Any issue with the compositor will show up.
    
    node_t* node = intuition.windowList->pred;
    
    do{
        window_t* window =(window_t*)node;
                
        
        
        if(window->needsRedraw == true){
            RedrawBlitRects(window);
        }else{
            //each window is redrawn automatically after one second
            window->autoRefeshCountdown -=1;
            
            if(window->autoRefeshCountdown <0){
               RedrawWindow(window);
            }
        }
        
        
        node = node->prev;
    }while(node->prev !=NULL);
    
   
    /*
    node_t* node = intuition.windowList->head;

    do{
        window_t* window = (window_t*)node;
        
        
        if(window->needsRedraw == true){
           RedrawBlitRects(window);
        }else{
            //each window is redrawn automatically after one second
            window->autoRefeshCountdown -=1;
            
            if(window->autoRefeshCountdown <0){
               RedrawWindow(window);
            }
        }
        
       
        node = node->next;
    }while(node->next != NULL);
    */
    
    
    drawMouse();
}



gadget_t* CreateGadget(window_t* window, uint32_t flags){
    
    gadget_t* gadget = (gadget_t*)executive->Alloc(sizeof(gadget_t));
    gadget->node.nodeType = NODE_GADGET;
    AddHead(&window->gadgetList,(node_t*)gadget);
    
    gadget->window      = window;
    gadget->isDecoration= false;
    gadget->flags       = flags;
    gadget->state       = GADGET_STATE_NORMAL;
    gadget->MouseDown   = NULL;
    gadget->MouseUp     = NULL;
    gadget->MouseMoved  = NULL;
    return gadget;
}

void RecalculateGadgetPosition(gadget_t* gadget){
    
    if( (gadget->flags & GADGET_FLAG_ORIGIN_RIGHT) == GADGET_FLAG_ORIGIN_RIGHT){
        gadget->ix = gadget->window->w - gadget->x;
    }else{
        gadget->ix = gadget->x;
    }
    
    if( (gadget->flags & GADGET_FLAG_ORIGIN_BOTTOM) == GADGET_FLAG_ORIGIN_BOTTOM){
        gadget->iy = gadget->window->h - gadget->y;
    }else{
        gadget->iy = gadget->y;
    }
    
    
}



//****************************************************************************************************************************************
//
//          GUI THEMES
//
//****************************************************************************************************************************************

void DefaultDepthGadgetRelease(gadget_t* gadget){
    
    window_t* window = gadget->window;
    
    if(window == (window_t*)intuition.windowList->pred){
        WindowToBack(window);
    }else{
        WindowToFront(window);
    }
    
}

void DefaultResizeGadgetMoved(gadget_t* gadget){
    
    if(gadget->window->eventPort != NULL){
        intuitionEvent_t* event = (intuitionEvent_t*) executive->Alloc(sizeof(intuitionEvent_t));
        event->message.replyPort = NULL;
        event->flags = WINDOW_EVENT_RESIZE;
        event->window = gadget->window;
        event->mouseX = mouseX;
        event->mouseY = mouseY;
        event->mouseXrel = mouseXold - mouseX;
        event->mouseYrel = mouseYold - mouseY;
        executive->PutMessage(gadget->window->eventPort,(message_t*)event);
        
        resizing = gadget->window;  //Hack to show window content while resizing
        resizeHack = 0;
        
    }
    
    
}

void DefaultResizeGadgetRelease(gadget_t* gadget){
    
    
    if(gadget->window->eventPort != NULL){
        intuitionEvent_t* event = (intuitionEvent_t*) executive->Alloc(sizeof(intuitionEvent_t));
        event->message.replyPort = NULL;
        event->flags = WINDOW_EVENT_RESIZE_END;
        event->window = gadget->window;
        event->mouseX = mouseX;
        event->mouseY = mouseY;
        event->mouseXrel = mouseXold - mouseX;
        event->mouseYrel = mouseYold - mouseY;
        executive->PutMessage(gadget->window->eventPort,(message_t*)event);
        
        resizing = NULL;  //Hack to show window content while resizing
        
    }
    
    
}

// Old Style
void DrawCloseGadgetOld(gadget_t* gadget){
    
    window_t* window = gadget->window;
    uint32_t state = gadget->state;
    
    switch(state){
    
        case GADGET_STATE_NORMAL:
            graphics.DrawRect(window->bitmap,4,0,24,20,intuition.blue);
            graphics.DrawRect(window->bitmap,6,0,20,20,intuition.white);
            graphics.DrawRect(window->bitmap,8,2,16,16,intuition.blue);
            graphics.DrawRect(window->bitmap,10,4,12,12,intuition.white);
            graphics.DrawRect(window->bitmap,14,8,4,4,intuition.black);
            break;
            
        case GADGET_STATE_HOVER:
            break;
            
        case GADGET_STATE_ACTIVE:
            graphics.DrawRect(window->bitmap,4,0,24,20,intuition.orange);
            graphics.DrawRect(window->bitmap,6,0,20,20,intuition.black);
            graphics.DrawRect(window->bitmap,8,2,16,16,intuition.orange);
            graphics.DrawRect(window->bitmap,10,4,12,12,intuition.black);
            graphics.DrawRect(window->bitmap,14,8,4,4,intuition.white);
            break;
                
    }
}

void DrawDepthGadgetOld(gadget_t* gadget){
    
    window_t* window = gadget->window;
    uint32_t state = gadget->state;
    
    uint32_t x = window->w - 29;
    

    
    switch(state){
        case GADGET_STATE_NORMAL:
            graphics.DrawRect(window->bitmap,x+2,0,24,20,intuition.blue);
            graphics.DrawRect(window->bitmap,x+4,0,20,20,intuition.white);
    
            graphics.DrawRect(window->bitmap,x+6,2,12,12,intuition.black);
            graphics.DrawRect(window->bitmap,x+10,6,12,12,intuition.blue);
            graphics.DrawRect(window->bitmap,x+12,8,8,8,intuition.white);
            break;
        case GADGET_STATE_HOVER:
            
            break;
        case GADGET_STATE_ACTIVE:
            
            graphics.DrawRect(window->bitmap,x+2,0,24,20,intuition.orange);
            graphics.DrawRect(window->bitmap,x+4,0,20,20,intuition.black);
            
            graphics.DrawRect(window->bitmap,x+6,2,12,12,intuition.white);
            graphics.DrawRect(window->bitmap,x+10,6,12,12,intuition.orange);
            graphics.DrawRect(window->bitmap,x+12,8,8,8,intuition.black);
            break;
            
    }
    

}

void DrawSizeGadgetOld(gadget_t* gadget){
    
    
    window_t* window = gadget->window;
    uint32_t state = gadget->state;
    
    
    switch (state) {
        case GADGET_STATE_NORMAL:
            graphics.DrawRect(window->bitmap,window->w - 17, window->h - 18,17,18,intuition.white);
            graphics.DrawRect(window->bitmap,window->w - 17+3, window->h - 18 +3,6,6,intuition.blue);
            
            graphics.DrawRect(window->bitmap,window->w - 17+7, window->h - (12),8,10,intuition.blue);
            
            graphics.DrawRect(window->bitmap,window->w - 12, window->h - 13,2,2,intuition.white);
            graphics.DrawRect(window->bitmap,window->w - 8, window->h - 10,4,6,intuition.white);
            break;
        case GADGET_STATE_HOVER:
            break;
        case GADGET_STATE_ACTIVE:
            graphics.DrawRect(window->bitmap,window->w - 17, window->h - 18,17,18,intuition.black);
            graphics.DrawRect(window->bitmap,window->w - 17+3, window->h - 18 +3,6,6,intuition.orange);
            
            graphics.DrawRect(window->bitmap,window->w - 17+7, window->h - (12),8,10,intuition.orange);
            
            graphics.DrawRect(window->bitmap,window->w - 12, window->h - 13,2,2,intuition.black);
            graphics.DrawRect(window->bitmap,window->w - 8, window->h - 10,4,6,intuition.black);
            break;
    }

}

void drawWindowTitleBarOld(window_t* window){

    graphics.DrawRect(window->bitmap,0,0,window->w,20,intuition.white);
    
    //text start and stop
    uint32_t s = (uint32_t)strlen(window->node.name)*8;
    uint32_t l = 5;
    
    if(window->flags & WINDOW_CLOSE_GADGET){
        l = 31;
    }
    

    graphics.RenderString(window->bitmap,intuition.defaultFont,l,2,window->node.name,intuition.blue,intuition.white);
    
    uint32_t dragBarX2 = 5 + l;
    
    if(window->flags & WINDOW_DEPTH_GADGET){
        dragBarX2 += 28;
    }
    
    if(window->flags & WINDOW_DRAGGABLE){
        graphics.DrawRect(window->bitmap,l+3+s,4,(window->w - s)-dragBarX2,4,intuition.blue);
        graphics.DrawRect(window->bitmap,l+3+s,12,(window->w - s)-dragBarX2,4,intuition.blue);
    }
    
    

    
    
    
    if(window->focused == false){
        int v=0;
        for(uint32_t cx=l; cx<window->w-dragBarX2+6+l; cx++){
            for(uint32_t cy=0; cy<20; cy++){
                
                if(cx<window->w){
                    if(v<3){
                        graphics.PutPixel(window->bitmap,cx,cy,intuition.white);
                    }
                }
                
                v += 1;
                if(v>5){v=0;};
            }
            
        }
    }
    

    
    
}

void drawWindowBorderOld(window_t* window){
    
        graphics.DrawRect(window->bitmap,0,0,window->w,2,intuition.white);
        graphics.DrawRect(window->bitmap,0,0,2,window->h,intuition.white);

    
        graphics.DrawRect(window->bitmap,0,window->h - 2,window->w-2,2,intuition.white);
        graphics.DrawRect(window->bitmap,window->w - 2,0,2,window->h,intuition.white);

}

void GimmeZeroZeroOld(window_t* window){
    
    if(window->flags & WINDOW_BORDERLESS){
        window->innerX = 0;
        window->innerY = 0;
        window->innerW = window->w;
        window->innerH = window->h;
    }else{
        window->innerX = 2;
        window->innerY = 2;
        window->innerW = window->w-4;
        window->innerH = window->h-4;
        
        if(window->flags & WINDOW_TITLEBAR){
            window->innerY = 20;
        }
 
        if(window->flags & WINDOW_RESIZABLE){
            window->innerW = window->w-18;
           // window->innerH = window->h-19;
        }
        
    }
    

    
}

void drawWindowDecorationOld(window_t* window){
    
    if(!(window->flags & WINDOW_BORDERLESS)){
        drawWindowBorderOld(window);
        
        if(window->flags & WINDOW_TITLEBAR){
            drawWindowTitleBarOld(window);
        }
        
        
        
        //Draw Gadgets & and recalculate gadget positions
        node_t* node = window->gadgetList.head;
        while(node->next != NULL){
            
            gadget_t* gadget = (gadget_t*)node;
            RecalculateGadgetPosition(gadget);
            if(gadget->isDecoration){
                gadget->Draw(gadget);
            }
            
            
            node = node->next;
        }
        
    }
        


}


//****************************************************************************
// New Style

void drawFrameNew(bitmap_t* bm,uint32_t x, uint32_t y, uint32_t w, uint32_t h,bool outer){
    
    uint32_t col1 = intuition.black;
    uint32_t col2 = intuition.white;
    
    if(outer){
        col1 = intuition.white;
        col2 = intuition.black;
    }

        graphics.DrawRect(bm,x,y+h - 2,w-1,2,col2);
        graphics.DrawRect(bm,x+w - 1,y,1,h,col2);
        
        graphics.DrawRect(bm,x,y,w,2,col1);
        graphics.DrawRect(bm,x,y,1,h,col1);
}

void DrawCloseGadgetNew(gadget_t* gadget){
    
    window_t* window = gadget->window;
    uint32_t state = gadget->state;
    
    uint32_t col = intuition.backgroundColour;
    
    if(window->focused){
        col = intuition.white;
    }

    
    graphics.DrawRect(window->bitmap,gadget->ix+20,gadget->iy,1,gadget->h-2,intuition.white);
    graphics.DrawRect(window->bitmap,gadget->ix+8,gadget->iy+6,5,10,intuition.black);
    
    switch(state) {
        case GADGET_STATE_NORMAL:
            graphics.DrawRect(window->bitmap,gadget->ix,gadget->iy,gadget->w,2,intuition.white);
            graphics.DrawRect(window->bitmap,gadget->ix,gadget->iy,1,gadget->h,intuition.white);
            graphics.DrawRect(window->bitmap,gadget->ix+19,gadget->iy+2,1,18,intuition.black);
            graphics.DrawRect(window->bitmap,gadget->ix+1,gadget->iy+20,gadget->w,2,intuition.black);
            graphics.DrawRect(window->bitmap,gadget->ix+9,gadget->iy+8,3,6,col);
            break;
        case GADGET_STATE_HOVER:
            break;
        case GADGET_STATE_ACTIVE:
            drawFrameNew(window->bitmap,gadget->ix+0,gadget->iy+0,20,22,false);
            graphics.DrawRect(window->bitmap,gadget->ix+9,gadget->iy+8,3,6,intuition.backgroundColour);
            break;
            
    }
   
}


void DrawDepthGadgetNew(gadget_t* gadget){
    
    window_t* window = gadget->window;
    uint32_t state = gadget->state;
    
    uint32_t x = gadget->ix - 1;
    
    uint32_t col = intuition.backgroundColour;
    
    if(window->focused){
        col = intuition.white;
    }
    
    graphics.DrawRect(window->bitmap,x+6,5,11,10,intuition.black);
    graphics.DrawRect(window->bitmap,x+7,7,9,6,intuition.backgroundColour);
    graphics.DrawRect(window->bitmap,x+10,9,10,9,intuition.black);
    graphics.DrawRect(window->bitmap,x+11,11,8,5,col);
    graphics.DrawRect(window->bitmap,x,2,1,20,intuition.black);
    
    switch(state) {
        case GADGET_STATE_NORMAL:
            
            graphics.DrawRect(window->bitmap,x+1,0,gadget->w,2,intuition.white);
            graphics.DrawRect(window->bitmap,x+1,0,1,20,intuition.white);
            graphics.DrawRect(window->bitmap,x,20,24,2,intuition.black);
            graphics.DrawRect(window->bitmap,x+gadget->w,2,1,gadget->h-2,intuition.black);
            break;
            
        case GADGET_STATE_HOVER:
            
            break;
            
        case GADGET_STATE_ACTIVE:
            drawFrameNew(window->bitmap,x+1, 0,23,22,false);
            graphics.DrawRect(window->bitmap,x+16,5,1,10,intuition.black);
            graphics.DrawRect(window->bitmap,x+10,13,6,2,intuition.black);
            break;
    }
    
}


void DrawSizeGadgetNew(gadget_t* gadget){
    
    window_t* window = gadget->window;
    uint32_t state = gadget->state;
    

    
    if(window->focused){
        graphics.DrawRect(window->bitmap,window->w - 17, window->h - 18,16,16,intuition.blue2);
    }else{
        graphics.DrawRect(window->bitmap,window->w - 17, window->h - 18,16,16,intuition.backgroundColour);
    }
    
    switch(state){
            
        case GADGET_STATE_NORMAL:
            
            graphics.DrawRect(window->bitmap,gadget->ix+1, gadget->iy,16,2,intuition.white);
            graphics.DrawRect(window->bitmap,gadget->ix, gadget->iy,1,gadget->h,intuition.white);
            
            graphics.DrawRect(window->bitmap,gadget->ix+gadget->w, gadget->iy,1,gadget->h,intuition.black);
            graphics.DrawRect(window->bitmap,gadget->ix, gadget->iy+gadget->h-1,gadget->w+1,2,intuition.black);
            
            
            
            graphics.DrawRect(window->bitmap,gadget->ix+3, gadget->iy+4,6,6,intuition.black);
            graphics.DrawRect(window->bitmap,gadget->ix+7, gadget->iy+6,8,10,intuition.black);
            
            if(window->focused){
                graphics.DrawRect(window->bitmap,gadget->ix + 5, gadget->iy + 6,2,2,intuition.white);
                graphics.DrawRect(window->bitmap,gadget->ix + 9, gadget->iy + 8,4,6,intuition.white);
            }else{
                graphics.DrawRect(window->bitmap,gadget->ix + 5, gadget->iy + 6,2,2,intuition.backgroundColour);
                graphics.DrawRect(window->bitmap,gadget->ix + 9, gadget->iy + 8,4,6,intuition.backgroundColour);
            };
            break;
        case GADGET_STATE_HOVER:
            break;
        case GADGET_STATE_ACTIVE:
            drawFrameNew(window->bitmap,window->w - 18, window->h - 19,18,19,false);
            
            graphics.DrawRect(window->bitmap,window->w - 17+3, window->h - 18 +3,6,6,intuition.black);
            graphics.DrawRect(window->bitmap,window->w - 17+7, window->h - (13),8,10,intuition.black);
            graphics.DrawRect(window->bitmap,window->w - 12, window->h - 13,2,2,intuition.backgroundColour);
            graphics.DrawRect(window->bitmap,window->w - 8, window->h - 11,4,6,intuition.backgroundColour);
            
            break;
    }
    
    
}

void drawWindowBorderNew(window_t* window){

    drawFrameNew(window->bitmap,0,0,window->w, window->h, true);
    drawFrameNew(window->bitmap,3,window->innerY,window->innerW-2,window->h - window->innerY - 2,false);
    
    if(window->focused){
        graphics.DrawRect(window->bitmap,1,2,2,window->h-4,intuition.blue2);
        graphics.DrawRect(window->bitmap,window->innerW+1,2,(window->w - window->innerW) - 2,window->h-4,intuition.blue2);
    }else{
        graphics.DrawRect(window->bitmap,1,2,2,window->h-4,intuition.backgroundColour);
        graphics.DrawRect(window->bitmap,window->innerW+1,2,(window->w - window->innerW) - 2,window->h-4,intuition.backgroundColour);
    }

}

void drawWindowTitleBarNew(window_t* window){
    
    uint32_t backCol = intuition.backgroundColour;
    
    
    if(window->focused){
        backCol = intuition.blue2;
    }
    
    graphics.DrawRect(window->bitmap,2,2,window->w-4,20 - 2,backCol);
    
    //debug_write_string("Draw title Bar!\n");

    uint32_t l = 5;
    
    if(window->flags & WINDOW_CLOSE_GADGET){
        l = 31;
    }
    
    
    graphics.RenderString(window->bitmap,intuition.defaultFont,l,2,window->node.name,intuition.black,backCol);
    
}


void drawWindowDecorationNew(window_t* window){
    
    //debug_write_string("Draw Decoration.\n");
    
    if(!(window->flags & WINDOW_BORDERLESS)){
        drawWindowBorderNew(window);

        if(window->flags & WINDOW_TITLEBAR){
            drawWindowTitleBarNew(window);
        }
        
        //Draw Gadgets & and recalculate gadget positions
        node_t* node = window->gadgetList.head;

        while(node->next != NULL){

            gadget_t* gadget = (gadget_t*)node;
            RecalculateGadgetPosition(gadget);
            if(gadget->isDecoration){
                gadget->Draw(gadget);
            }
            

            node = node->next;
        }
        
        
    }
    

    
}





//******************************************************************************
// Mac Style

void DrawCloseGadgetMac(gadget_t* gadget){
    
    window_t* window = gadget->window;
    
    if(!window->focused){
        return;
    }
    
    
    
    graphics.DrawRect(window->bitmap,8,4,13,11,window->backgroundColour);
    graphics.DrawRect(window->bitmap,9,4,11,11,intuition.black);
    graphics.DrawRect(window->bitmap,10,5,9,9,window->backgroundColour);
    
    uint32_t state = gadget->state;
    switch (state) {
        case GADGET_STATE_NORMAL:
            break;
        case GADGET_STATE_HOVER:
            break;
        case GADGET_STATE_ACTIVE:
            
            graphics.DrawRect(window->bitmap,14,5,1,3,intuition.black);
            graphics.DrawRect(window->bitmap,14,11,1,3,intuition.black);
            
            graphics.DrawRect(window->bitmap,10,9,3,1,intuition.black);
            graphics.DrawRect(window->bitmap,16,9,3,1,intuition.black);
            
            graphics.PutPixel(window->bitmap,11,6,intuition.black);
            graphics.PutPixel(window->bitmap,12,7,intuition.black);
            
            graphics.PutPixel(window->bitmap,17,6,intuition.black);
            graphics.PutPixel(window->bitmap,16,7,intuition.black);
            
            graphics.PutPixel(window->bitmap,11,12,intuition.black);
            graphics.PutPixel(window->bitmap,12,11,intuition.black);
            
            graphics.PutPixel(window->bitmap,17,12,intuition.black);
            graphics.PutPixel(window->bitmap,16,11,intuition.black);
            
            break;
    }
    

}

void DrawDepthGadgetMac(gadget_t* gadget){
    
    window_t* window = gadget->window;
    
    if(!window->focused){
        return;
    }
    
    uint32_t x = window->w;
    

    
    uint32_t state = gadget->state;
    switch (state) {
        case GADGET_STATE_NORMAL:
            
            graphics.DrawRect(window->bitmap,x-8-13,4,13,11,window->backgroundColour);
            graphics.DrawRect(window->bitmap,x-9-11,4,11,11,intuition.black);
            graphics.DrawRect(window->bitmap,x-10-9,5,9,9,window->backgroundColour);
            
            break;
        case GADGET_STATE_HOVER:
            break;
        case GADGET_STATE_ACTIVE:
            
            graphics.DrawRect(window->bitmap,x-8-13,4,13,11,window->backgroundColour);
            graphics.DrawRect(window->bitmap,x-9-11,4,11,11,window->backgroundColour);
            graphics.DrawRect(window->bitmap,x-10-9,5,9,9,intuition.black);
            
            break;
    }
    

    
}

void DrawSizeGadgetMac(gadget_t* gadget){
    
    window_t* window = gadget->window;
    
    graphics.DrawRect(window->bitmap,0, window->h - 18,window->w,1,intuition.black);
    graphics.DrawRect(window->bitmap,window->w - 17,18,1,window->h,intuition.black);
    
    graphics.DrawRect(window->bitmap,window->w - 17, window->h - 18,17,18,intuition.black);
    graphics.DrawRect(window->bitmap,window->w - 16, window->h - 17,14,15,window->backgroundColour);
    
    if(!window->focused){
        return;
    }
    
    uint32_t col1 = intuition.black;
    uint32_t col2 = window->backgroundColour;
    
    if(gadget->state == GADGET_STATE_ACTIVE){
        col2 = intuition.black;
        col1 = window->backgroundColour;
    }
    
        graphics.DrawRect(window->bitmap,window->w - 12, window->h - 12,9,9,col1);
        graphics.DrawRect(window->bitmap,window->w - 11, window->h - 11,7,7,col2);
    
        graphics.DrawRect(window->bitmap,window->w - 14, window->h - 15,7,8,col1);
        graphics.DrawRect(window->bitmap,window->w - 13, window->h - 14,5,6,col2);



}

void drawWindowTitleBarMac(window_t* window){
    
    uint32_t s = ((uint32_t)strlen(window->node.name)*8)/2;
    uint32_t titlePos = (window->w/2) - s;
    
    graphics.DrawRect(window->bitmap,1,1,window->w-3,17,window->backgroundColour);
    graphics.DrawRect(window->bitmap,0,18,window->w,1,intuition.black);
    
    if(window->focused){
        graphics.DrawRect(window->bitmap,2,4,window->w-5,1,intuition.black);
        graphics.DrawRect(window->bitmap,2,6,window->w-5,1,intuition.black);
        graphics.DrawRect(window->bitmap,2,8,window->w-5,1,intuition.black);
        graphics.DrawRect(window->bitmap,2,10,window->w-5,1,intuition.black);
        graphics.DrawRect(window->bitmap,2,12,window->w-5,1,intuition.black);
        graphics.DrawRect(window->bitmap,2,14,window->w-5,1,intuition.black);
        
        graphics.DrawRect(window->bitmap,titlePos-7,1,(s*2)+14,16,window->backgroundColour);
        

    }
    
    graphics.RenderString(window->bitmap,intuition.defaultFont,titlePos,2,window->node.name,intuition.black,window->backgroundColour);

    
    return;
}

void drawWindowBorderMac(window_t* window){
    
    graphics.DrawRect(window->bitmap,0,0,window->w,1,intuition.black);
    graphics.DrawRect(window->bitmap,0,0,1,window->h,intuition.black);
    
    //For some reason the classic mac right and bottom borders were 2 pixels
    graphics.DrawRect(window->bitmap,0,window->h - 2,window->w-2,2,intuition.black);
    graphics.DrawRect(window->bitmap,window->w - 2,0,2,window->h,intuition.black);
    
}


void GimmeZeroZeroMac(window_t* window){
    
    if(window->flags & WINDOW_BORDERLESS){
        window->innerX = 0;
        window->innerY = 0;
        window->innerW = window->w;
        window->innerH = window->h;
    }else{
        window->innerX = 2;
        window->innerY = 2;
        window->innerW = window->w-4;
        window->innerH = window->h-4;
        
        if(window->flags & WINDOW_TITLEBAR){
            window->innerY = 20;
        }
        
        if(window->flags & WINDOW_RESIZABLE){
            window->innerW = window->w-18;
            window->innerH = window->h-19;
        }
        
    }
    
    
    
}

void drawWindowDecorationMac(window_t* window){
    
    if(!(window->flags & WINDOW_BORDERLESS)){
        drawWindowBorderMac(window);
        
        if(window->flags & WINDOW_TITLEBAR){
            drawWindowTitleBarMac(window);
            intuition.needsUpdate = true;
        }
        
        
        //Draw Gadgets & and recalculate gadget positions
        node_t* node = window->gadgetList.head;
        while(node->next != NULL){
            
            gadget_t* gadget = (gadget_t*)node;
            RecalculateGadgetPosition(gadget);
            if(gadget->isDecoration){
                gadget->Draw(gadget);
            }
            
            
            node = node->next;
        }
        
    }
    

    
    
}


//*******************************************************************************







//***********************************************



void calculateBlitRects(window_t* window){

    
    //debug_write_string("The Dredded Cliprect function!\n");
    
    window->needsRedraw = true;
    node_t* winNode = (node_t*)window;
    
uint32_t sX1 = window->x;
uint32_t sY1 = window->y;
uint32_t sX2 = window->x+window->w;
uint32_t sY2 = window->y+window->h;
if(sX2 > graphics.frameBuffer.width){sX2=graphics.frameBuffer.width;}
if(sY2 > graphics.frameBuffer.height){sY2=graphics.frameBuffer.height;}
    
Lock(&window->clipRectsLock);
window->clipRect[0].isVisible=true;
window->clipRect[0].x = 0;
window->clipRect[0].y = 0;
window->clipRect[0].w = window->w;
window->clipRect[0].h = window->h;
window->clipRects = 1;
    
    //if there are no wondows above then exit
    node_t* node = window->node.next;
    if(node->next== NULL){
        window->clipRect[0].isVisible=true;
        FreeLock(&window->clipRectsLock);
        return;
    }
    
        node = intuition.windowList->pred;
    
         while (node != winNode) {
             
             window_t* above = (window_t*)node;
             
             uint32_t winX1 = above->x;
             uint32_t winY1 = above->y;
             uint32_t winX2 = above->x + above->w;
             uint32_t winY2 = above->y + above->h;
             if(winX2 > graphics.frameBuffer.width){winX2=graphics.frameBuffer.width;}
             if(winY2 > graphics.frameBuffer.height){winY2=graphics.frameBuffer.height;}
             
          
             //if window is totally covered stop processing
             if( (winX1 <= sX1) && winY1 <= sY1 && winX2 >= sX2 && winY2 >= sY2){
                 window->clipRects = 0;
                 window->clipRect[0].isVisible = false;
                 window->clipRect[0].needsUpdate = false;
                 
                 FreeLock(&window->clipRectsLock);
                 return;
             }
             
                 
             uint32_t clipCount = window->clipRects;
             for(uint32_t i=0;i<clipCount;++i){
             
                 uint32_t clipX1 = window->clipRect[i].x + window->x;
                 uint32_t clipY1 = window->clipRect[i].y + window->y;
                 uint32_t clipX2 = clipX1 + window->clipRect[i].w;
                 uint32_t clipY2 = clipY1 + window->clipRect[i].h;
                 if(clipX2 > graphics.frameBuffer.width){clipX2=graphics.frameBuffer.width;}
                 if(clipY2 > graphics.frameBuffer.height){clipY2=graphics.frameBuffer.height;}
                
                 //check the clip is completely covered
                 if( (winX1 <= clipX1) && winY1 <= clipY1 && winX2 >= clipX2 && winY2 >= clipY2){
                     window->clipRect[i].isVisible = false;
                     window->clipRect[i].needsUpdate = false;


                 }else if (clipX1 < winX2 && clipX2 > winX1 && clipY1 < winY2 && clipY2 > winY1 && window->clipRect[i].isVisible == true){
                        int32_t c = -1;
                        
                        if(winY1>clipY1){
                            //new visible rect to the top
                           
                            if(c==-1){
                                c = i;
                            }else{
                                c =  window->clipRects;
                                window->clipRects +=1;
                            }
                            
                            window->clipRect[c].isVisible = true;
                            window->clipRect[c].needsUpdate = true;
                            window->clipRect[c].x = clipX1 - sX1;
                            window->clipRect[c].w = clipX2 - clipX1;
                            window->clipRect[c].y = clipY1 - sY1;
                            window->clipRect[c].h = winY1 - clipY1;
                
                        }
                        
                        if(winY2<clipY2){
                            //new visible rect to the bottom
                            
                            if(c==-1){
                                c = i;
                            }else{
                                c =  window->clipRects;
                                window->clipRects +=1;
                            }
                            
                            window->clipRect[c].isVisible = true;
                            window->clipRect[c].needsUpdate = true;
                            window->clipRect[c].x = clipX1 - sX1;
                            window->clipRect[c].w = clipX2 - clipX1;
                            window->clipRect[c].y = winY2 - sY1;
                            window->clipRect[c].h = clipY2 - winY2;
                    
                        }
                     
                     
                     
                        if(winX1>clipX1){
                            //new visible rect to the left
                       
                            
                          
                            if(c==-1){
                                c = i;
                            }else{
                                c =  window->clipRects;
                                window->clipRects +=1;
                            }
                            
                            window->clipRect[c].isVisible = true;
                            window->clipRect[c].needsUpdate = true;
                            window->clipRect[c].x = clipX1-sX1;
                            window->clipRect[c].w = winX1 - clipX1;
                        
                            
                            if(winY1<=clipY1){
                                window->clipRect[c].y = (clipY1 - sY1);
                                
                                if(winY2 >= clipY2){
                                    window->clipRect[c].h = clipY2 - clipY1;
                                }else{
                                    window->clipRect[c].h = winY2 - clipY1;
                                }
                                
                            }else{
                                window->clipRect[c].y = (winY1 - sY1);
                                
                                if(winY2 >= clipY2){
                                    window->clipRect[c].h = clipY2 - winY1;
                                }else{
                                    window->clipRect[c].h = winY2 - winY1;
                                }
                
                            }
                            
                            
     
                        }
                    
                     
                     
                        if(winX2<clipX2){
                                //new visible rect to the right
                            
                           
                                if(c==-1){
                                    c = i;
                                }else{
                                    c =  window->clipRects;
                                    window->clipRects +=1;
                                }
                                
                                
                               window->clipRect[c].isVisible = true;
                               window->clipRect[c].needsUpdate = true;
                               window->clipRect[c].x = winX2 - sX1;
                               window->clipRect[c].w = clipX2 - winX2;
                                
                            
                        
                            if(winY1<=clipY1){
                                window->clipRect[c].y = (clipY1 - sY1);
                                
                                if(winY2 >= clipY2){
                                    window->clipRect[c].h = clipY2 - clipY1;
                                }else{
                                    window->clipRect[c].h = winY2 - clipY1;
                                }
                            }else{
                                window->clipRect[c].y = (winY1 - sY1);
                                
                                if(winY2 >= clipY2){
                                    window->clipRect[c].h = clipY2 - winY1;
                                }else{
                                    window->clipRect[c].h = winY2 - winY1;
                                }
                                
                            }
                            
                                 
                            
                        }
                        
                        if(c==-1){
                            window->clipRect[i].isVisible = false;
                            window->clipRect[i].needsUpdate = false;
                        }
                        
                        
                    }
                 
                 
             }
             
             node = node->prev;
         }
    
    if(window->clipRects>255){
        debug_write_string("Too many Clirects!!");
    }
    
    FreeLock(&window->clipRectsLock);
}



























void updateLayers(window_t* window){
    
    //THis function should never be called by a user task!!!
    
    //if window = NULL, update all layers
    
    /*
    node_t* winNode = (node_t*) window;
    
    node_t* node = windowList.head;
    calculateBlitRects((window_t*)node);
    
    if(node == winNode){
        return;
    }
    
    do{
        node = node->next;
        window = (window_t*)node;
        calculateBlitRects(window);
    }while(node != winNode);
    */
    
   
    //Simply Update all layers
    
    //executive->Forbid();
    
    
    //debug_write_string("begin update\n");
    //Lock(&windowList.lock);
    
    node_t* node = intuition.windowList->pred;
    
    do{
        window = (window_t*)node;
        calculateBlitRects(window);
        node = node->prev;
    }while(node->prev != NULL);

    //FreeLock(&windowList.lock);
    
    //executive->Permit();
    
}


void ResizeWindow(window_t* window, uint32_t w, uint32_t h){
    
    
    if(w < window->minWidth){
        
        mouseX = mouseXold;
        
        w = window->minWidth;
        
        if(window->w == window->minWidth && window->h == window->minHeight){

            //return;
        }
        

 
    }
    
    
    if(h < window->minHeight){
        
        mouseY = mouseYold;
        
        h = window->minHeight;
        
        if(window->w == window->minWidth && window->h == window->minHeight){
            return;
        }
        


    }
    
    
    if(w > window->maxWidth){
        w = window->maxWidth;
    }
    
    if(h > window->maxHeight){
        h = window->maxHeight;
    }
    

    


    bitmap_t* bm = graphics.NewBitmap(w, h);
    bitmap_t* old = window->bitmap;
    
    if(bm==NULL){
        return;
    }

    
    
    
    executive->Forbid();
    window->needsRedraw = false;
    window->w = w;
    window->h = h;
    intuition.GimmeZeroZero(window);
    window->bitmap = bm;
    
    graphics.ClearBitmap(window->bitmap,window->backgroundColour); // This should use the proper window clear function....

    graphics.FreeBitmap(old);
    

    
    executive->Permit();
    //debug_write_hex((uint32_t)bm);debug_write_string(" bitmap address\n");
    
 
    intuition.DrawDecoration(window);
    intuition.needsUpdate = true;
    RedrawWindow(window);
}



void MoveWindow(window_t* window,int32_t x, int32_t y){
    
    
    //No off screen moving at the moment, to match original AmigaOS 1.3 behaviour
    if(x+window->w > graphics.frameBuffer.width){
        x = graphics.frameBuffer.width - window->w;
    }
    if(y+window->h > graphics.frameBuffer.height){
        y = graphics.frameBuffer.height - window->h;
    }
    
    
    
    
    if(x<0){
        x=0;
    }
    
    if(y<0){
        y=0;
    }
    
    
    //if the window hasn't moved... don't update.
    if((uint32_t)x==window->x && (uint32_t)y==window->y){
        return;
    }
    
    window->x = x;
    window->y = y;
    
    intuition.needsUpdate = true;
    RedrawWindow(window);
}


void WindowToBack(window_t* window){
    window->node.priority = 1;
    Remove(intuition.windowList,(node_t*)window);
    Enqueue(intuition.windowList, (node_t*)window);
    window->node.priority = 0;
    intuition.needsUpdate = true;
    RedrawWindow(window);
}

void WindowToFront(window_t* window){
    window->node.priority = -1;
    Remove(intuition.windowList,(node_t*)window);
    Enqueue(intuition.windowList, (node_t*)window);
    window->node.priority = 0;
    intuition.needsUpdate = true;
    RedrawWindow(window);
    
}

void PriorityOrderPrivate(window_t* window){
    Remove(intuition.windowList,(node_t*)window);
    Enqueue(intuition.windowList, (node_t*)window);
    //intuition.needsUpdate = true;
    updateLayers(window);
    RedrawWindow(window);
}

void ClearWindow(window_t* window){
    uint32_t* p = window->bitmap->buffer;
    uint32_t i = window->bitmap->width * window->bitmap->height;
    do{
        i -= 1;
        p[i] = window->backgroundColour;
    }while(i > 0);
    intuition.DrawDecoration(window);
}


void Focus(window_t* window){
    
    if(inputStruct.focused == window){
        return;
    }
    
    if(inputStruct.focused != NULL){
        
        inputStruct.focused->focused = false;
        
        if(inputStruct.focused->flags & WINDOW_TITLEBAR){
            intuition.DrawDecoration(inputStruct.focused);
            intuition.RedrawWindow(inputStruct.focused);
        }
        
    }
    
    inputStruct.focused = window;
    window->focused = true;
    screenTitle = window->screenTitle;
    
    if(inputStruct.focused->flags & WINDOW_TITLEBAR){
        intuition.DrawDecoration(inputStruct.focused);
        intuition.RedrawWindow(inputStruct.focused);
    }
    
}

void SetScreenTitle(window_t* window,char* title){
    window->screenTitle = title;
}

void SetBusy(window_t* window,bool state){
    window->isBusy = state;
    inputStruct.rawMouse[3] = 1; // tell intuition to update mouse image
}

/*
void InsertI(list_t* list, node_t* node, node_t* pred){
    
    node->next = pred->next;
    node->prev = pred;
    
    node->next->prev = node;
    pred->next = node;
    
    list->count +=1;
    
}

void EnqueueI(list_t* list,node_t* node){

    node_t* pred = list->pred;
    
    while(pred->prev != NULL){
        
        if(pred->priority >= node->priority){
            Insert(list, node, pred);
            debug_write_string("Found a sutiable place\n");
            return;
        }
        
        pred = pred->prev;
    }
    
    
    AddHead(list, node);
    debug_write_string("stuck it at the head\n");
}
*/


//OpenWindowPrivate only works before multitasking is started
window_t* OpenWindowPrivate(window_t* parent,uint32_t x, uint32_t y, uint32_t w, uint32_t h,uint64_t flags,char* title){
    
    window_t* window =(window_t*)executive->Alloc(sizeof(window_t));
    window->node.nodeType = NODE_WINDOW;
    window->node.name = title;
    window->node.priority = 0;
    window->screenTitle = NULL;
    
    //Clamp window to screen bounds
    if(x>graphics.frameBuffer.width){x=0;}
    if(y>graphics.frameBuffer.height){y=0;}
    
    //init window values to ensure the window state is known at startup
    window->doubleClick = 0;
    window->mouseX  = 0;
    window->mouseX  = 0;
    window->innerX  = 0;
    window->innerY  = 0;
    window->innerW  = w;
    window->innerH  = h;
    window->focused = false;
    window->needsRedraw = true;
    window->isBusy = false;
    window->autoRefeshCountdown = 0;
    
    window->parent = parent;
    window->x = x;
    window->y = y;
    window->w = w;
    window->h = h;
    window->minWidth = 100;
    window->minHeight = 60;
    window->maxWidth = graphics.frameBuffer.width;
    window->maxHeight = graphics.frameBuffer.height;
    window->flags = flags;
    window->bitmap = graphics.NewBitmap(w, h);
    window->eventPort = NULL;
    intuition.GimmeZeroZero(window);
    
    window->foregroundColour = intuition.defaultWindowForegroundColour;
    window->backgroundColour = intuition.defaultWindowBackgroundColour;
    window->highlightColour = intuition.defaultWindowHighlightColour;
    
    window->noRise = false;    // by default windows should be (double) clickable to front
    

    //clear the window;
    uint32_t* p = window->bitmap->buffer;
    for(uint32_t i=0;i<w*h;++i){
        p[i] = window->backgroundColour;
    }

    

    
    InitList(&window->gadgetList);

    //debug_write_string("Building window decoration\n");
    
    //Build Window Decoration
    if(window->flags & WINDOW_DEPTH_GADGET){
        gadget_t* depth = CreateGadget(window,intuition.systemDepthFlags);
        depth->Draw = intuition.DrawSystemDepthGadget;
        depth->x        = intuition.systemDepthX;
        depth->y        = intuition.systemDepthY;
        depth->w        = intuition.systemDepthW;
        depth->h        = intuition.systemDepthH;
        depth->isDecoration = true;
        depth->MouseUp  = DefaultDepthGadgetRelease;
    }
    
    if(window->flags & WINDOW_RESIZABLE){
        gadget_t* resize = CreateGadget(window,intuition.systemSizeFlags);
        resize->Draw = intuition.DrawSystemSizeGadget;
        resize->x        = intuition.systemSizeX;
        resize->y        = intuition.systemSizeY;
        resize->w        = intuition.systemSizeW;
        resize->h        = intuition.systemSizeH;
        resize->isDecoration = true;
        resize->MouseMoved = DefaultResizeGadgetMoved;
        resize->MouseUp    = DefaultResizeGadgetRelease;
    }
    
    if(window->flags & WINDOW_CLOSE_GADGET){
        gadget_t* close = CreateGadget(window,0);
        close->Draw     = intuition.DrawSystemCloseGadget;
        close->x        = intuition.systemCloseX;
        close->y        = intuition.systemCloseY;
        close->w        = intuition.systemCloseW;
        close->h        = intuition.systemCloseH;
        close->isDecoration = true;
    }


    intuition.DrawDecoration(window);
    
    //set up first cliprect
    window->clipRectsLock.isLocked = false;
    window->clipRect[0].isVisible=true;
    window->clipRect[0].x = 0;
    window->clipRect[0].y = 0;
    window->clipRect[0].w = window->w;
    window->clipRect[0].h = window->h;
    window->clipRects = 1;
    
    
    //Add window to intuition's window list
    
    Lock(&intuition.windowList->lock);
    Enqueue(intuition.windowList, (node_t*)window);
    FreeLock(&intuition.windowList->lock);
    intuition.needsUpdate = true;
    window->needsRedraw = true;
    updateLayers(window);
    
    //RedrawWindow(window); // probably don't need this anymore
    

    
    return window;
}


window_t* OpenWindow(window_t* parent,uint32_t x, uint32_t y, uint32_t w, uint32_t h,uint64_t flags,char* title){
    
    //executive->debug_write_string("Open Window!\n");
    
    window_t* window =(window_t*)executive->Alloc(sizeof(window_t));
    window->node.nodeType = NODE_WINDOW;
    window->node.name = title;
    window->node.priority = 0;
    window->screenTitle = NULL;
    
    //Clamp window to screen bounds
    if(x>graphics.frameBuffer.width){x=0;}
    if(y>graphics.frameBuffer.height){y=0;}
    
    //init window values to ensure the window state is known at startup
    window->doubleClick = 0;
    window->mouseX  = 0;
    window->mouseX  = 0;
    window->innerX  = 0;
    window->innerY  = 0;
    window->innerW  = w;
    window->innerH  = h;
    window->focused = false;
    window->needsRedraw = true;
    window->isBusy = false;
    window->autoRefeshCountdown = 0;
    
    
    window->parent = parent;
    window->x = x;
    window->y = y;
    window->w = w;
    window->h = h;
    window->minWidth = 100;
    window->minHeight = 60;
    window->maxWidth = graphics.frameBuffer.width;
    window->maxHeight = graphics.frameBuffer.height;
    window->flags = flags;
    window->bitmap = graphics.NewBitmap(w, h);
    window->eventPort = NULL;
    intuition.GimmeZeroZero(window);
    
    window->foregroundColour = intuition.defaultWindowForegroundColour;
    window->backgroundColour = intuition.defaultWindowBackgroundColour;
    window->highlightColour = intuition.defaultWindowHighlightColour;
    
    window->noRise = false;    // by default windows should be (double) clickable to front
    

    //clear the window;
    uint32_t* p = window->bitmap->buffer;
    for(uint32_t i=0;i<w*h;++i){
        p[i] = window->backgroundColour;
    }

    

    
    InitList(&window->gadgetList);

    //debug_write_string("Building window decoration\n");
    
    //Build Window Decoration
    if(window->flags & WINDOW_DEPTH_GADGET){
        gadget_t* depth = CreateGadget(window,intuition.systemDepthFlags);
        depth->Draw = intuition.DrawSystemDepthGadget;
        depth->x        = intuition.systemDepthX;
        depth->y        = intuition.systemDepthY;
        depth->w        = intuition.systemDepthW;
        depth->h        = intuition.systemDepthH;
        depth->isDecoration = true;
        depth->MouseUp  = DefaultDepthGadgetRelease;
    }
    
    if(window->flags & WINDOW_RESIZABLE){
        gadget_t* resize = CreateGadget(window,intuition.systemSizeFlags);
        resize->Draw = intuition.DrawSystemSizeGadget;
        resize->x        = intuition.systemSizeX;
        resize->y        = intuition.systemSizeY;
        resize->w        = intuition.systemSizeW;
        resize->h        = intuition.systemSizeH;
        resize->isDecoration = true;
        resize->MouseMoved = DefaultResizeGadgetMoved;
        resize->MouseUp    = DefaultResizeGadgetRelease;
    }
    
    if(window->flags & WINDOW_CLOSE_GADGET){
        gadget_t* close = CreateGadget(window,0);
        close->Draw     = intuition.DrawSystemCloseGadget;
        close->x        = intuition.systemCloseX;
        close->y        = intuition.systemCloseY;
        close->w        = intuition.systemCloseW;
        close->h        = intuition.systemCloseH;
        close->isDecoration = true;
    }

    
    

    intuition.DrawDecoration(window);
    
    //set up first cliprect
    window->clipRectsLock.isLocked = false;
    window->clipRect[0].isVisible=true;
    window->clipRect[0].x = 0;
    window->clipRect[0].y = 0;
    window->clipRect[0].w = window->w;
    window->clipRect[0].h = window->h;
    window->clipRects = 1;
    
    
    //debug_write_string("Enqueuing Window!\n");
    
    //Add window to intuition's window list via a message
    intuitionEvent_t* event = (intuitionEvent_t*) executive->Alloc(sizeof(intuitionEvent_t));
    event->flags = INTUITION_REQUEST_OPEN_WINDOW;
    event->message.replyPort = NULL;
    event->window = window;
    event->data = intuition.windowList;
    executive->PutMessage(intuition.intuiPort,(message_t*)event);
    
    //debug_write_string("Enqueued Window!\n");

    return window;
}








window_t* Request(char* title){
    
    debug_write_string("Requester Called!\n");
    
    window_t* window = OpenWindow(NULL, 0,0,320,143,WINDOW_TITLEBAR | WINDOW_DRAGGABLE | WINDOW_DEPTH_GADGET| WINDOW_RESIZABLE,title);
    //graphics.DrawRect(window->bitmap,4,22,window->innerW-5,window->h - 26,intuition.white);
    //graphics.DrawRect(window->bitmap,6,24,window->innerW-9,window->h - 30,intuition.blue);
    //graphics.DrawRect(window->bitmap,8,26,window->innerW-13,window->h - 34,intuition.white);
    
    //fake gadgets
    //graphics.DrawRect(window->bitmap,17,98,63,33,intuition.orange);
    //graphics.DrawRect(window->bitmap,19,100,59,29,intuition.white);
    //graphics.DrawRect(window->bitmap,19,102,59,25,intuition.blue);
    //graphics.DrawRect(window->bitmap,21,104,55,21,intuition.white);
    
    //graphics.DrawRect(window->bitmap,200+17,98,63,33,intuition.orange);
    //graphics.DrawRect(window->bitmap,200+19,100,59,29,intuition.white);
    //graphics.DrawRect(window->bitmap,200+19,102,59,25,intuition.blue);
    //graphics.DrawRect(window->bitmap,200+21,104,55,21,intuition.white);
    
    //graphics.RenderString(window->bitmap,intuition.defaultFont,10,30," Error validating disk",intuition.blue,intuition.white);
    //graphics.RenderString(window->bitmap,intuition.defaultFont,10,30+32," Disk is unreadable",intuition.blue,intuition.white);
    
    //graphics.RenderString(window->bitmap,intuition.defaultFont,32,106,"Test",intuition.blue,intuition.white);
    //graphics.RenderString(window->bitmap,intuition.defaultFont,232,106,"Test",intuition.blue,intuition.white);
    
    return window;
    
}

void Plot(window_t* window,uint32_t x, uint32_t y, uint32_t colour){
    
   
    Lock(&window->clipRectsLock);
    for(uint32_t i = 0;i < window->clipRects; ++i){
        
        if(x > window->clipRect[i].x && x < (window->clipRect[i].x + window->clipRect[i].w) ){
            if(y > window->clipRect[i].y && y < (window->clipRect[i].y + window->clipRect[i].h) ){
                window->clipRect[i].needsUpdate = true;
                window->needsRedraw = true;
            }
        }
        
    }
  
    graphics.PutPixel(window->bitmap,x,y,colour);
    FreeLock(&window->clipRectsLock);
    
}

void PutChar(window_t* window, uint32_t x, uint32_t y, uint8_t character, uint32_t fColour, uint32_t bColour){
    
    
    Lock(&window->clipRectsLock);
       for(uint32_t i = 0;i < window->clipRects; ++i){
           
           uint32_t clipX1 = window->clipRect[i].x;
           uint32_t clipX2 = window->clipRect[i].x + window->clipRect[i].w;
           uint32_t clipY1 = window->clipRect[i].y;
           uint32_t clipY2 = window->clipRect[i].y + window->clipRect[i].h;
           
           if (clipX1 < x+8 && clipX2 > x && clipY1 < y+16 && clipY2 > y){
               window->clipRect[i].needsUpdate = true;
               window->needsRedraw = true;
           }
        }
    
    graphics.RenderGlyph(window->bitmap, intuition.defaultFont, x, y, character, fColour, bColour);
    FreeLock(&window->clipRectsLock);
}

void DrawRectangle(window_t* window,uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t colour){
    
    
    Lock(&window->clipRectsLock);
    for(uint32_t i = 0;i < window->clipRects; ++i){
        
        uint32_t clipX1 = window->clipRect[i].x;
        uint32_t clipX2 = window->clipRect[i].x + window->clipRect[i].w;
        uint32_t clipY1 = window->clipRect[i].y;
        uint32_t clipY2 = window->clipRect[i].y + window->clipRect[i].h;
        
        uint32_t winX1 = x;
        uint32_t winX2 = x + w;
        uint32_t winY1 = y;
        uint32_t winY2 = y + h;
        
        if (clipX1 < winX2 && clipX2 > winX1 && clipY1 < winY2 && clipY2 > winY1){
            window->clipRect[i].needsUpdate = true;
            window->needsRedraw = true;
        }
     }
    
    graphics.DrawRect(window->bitmap,x, y, w, h, colour);
    FreeLock(&window->clipRectsLock);
}


void DrawLineI(window_t* window,uint32_t x0, uint32_t y0, uint32_t x1, uint32_t y1, uint32_t colour){

    Lock(&window->clipRectsLock);
    for(uint32_t i = 0;i < window->clipRects; ++i){
        
        uint32_t clipX1 = window->clipRect[i].x;
        uint32_t clipX2 = window->clipRect[i].x + window->clipRect[i].w;
        uint32_t clipY1 = window->clipRect[i].y;
        uint32_t clipY2 = window->clipRect[i].y + window->clipRect[i].h;
        
        uint32_t winX1 = x0;
        uint32_t winX2 = x1;
        uint32_t winY1 = y0;
        uint32_t winY2 = y1;
        
        if (clipX1 < winX2 && clipX2 > winX1 && clipY1 < winY2 && clipY2 > winY1){
            window->clipRect[i].needsUpdate = true;
            window->needsRedraw = true;
        }
     }
    
    graphics.DrawLine(window->bitmap,x0, y0, x1, y1, colour);
    FreeLock(&window->clipRectsLock);
    
    
}

void FloodFillI(window_t* window, uint32_t x, uint32_t y, uint32_t colour){
    
    Lock(&window->clipRectsLock);
    /*
    for(uint32_t i = 0;i < window->clipRects; ++i){
        
        uint32_t clipX1 = window->clipRect[i].x;
        uint32_t clipX2 = window->clipRect[i].x + window->clipRect[i].w;
        uint32_t clipY1 = window->clipRect[i].y;
        uint32_t clipY2 = window->clipRect[i].y + window->clipRect[i].h;
        
        uint32_t winX1 = window->x;
        uint32_t winX2 = winX1 + window->w;
        uint32_t winY1 = window->y;
        uint32_t winY2 = winY1 + window->h;
        
        if (clipX1 < winX2 && clipX2 > winX1 && clipY1 < winY2 && clipY2 > winY1){
            window->clipRect[i].needsUpdate = true;
            window->needsRedraw = true;
        }
     }
    */
    graphics.FloodFill(window->bitmap, x, y, colour);
    FreeLock(&window->clipRectsLock);
    
}

void DrawCircleI(window_t* window, uint32_t x, uint32_t y, uint32_t r, uint32_t colour, bool filled){
    
    Lock(&window->clipRectsLock);
    for(uint32_t i = 0;i < window->clipRects; ++i){
        
        uint32_t clipX1 = window->clipRect[i].x;
        uint32_t clipX2 = window->clipRect[i].x + window->clipRect[i].w;
        uint32_t clipY1 = window->clipRect[i].y;
        uint32_t clipY2 = window->clipRect[i].y + window->clipRect[i].h;
        
        uint32_t winX1 = x;
        uint32_t winX2 = x + r;
        uint32_t winY1 = y;
        uint32_t winY2 = y + r;
        
        if (clipX1 < winX2 && clipX2 > winX1 && clipY1 < winY2 && clipY2 > winY1){
            window->clipRect[i].needsUpdate = true;
            window->needsRedraw = true;
        }
     }
    
    graphics.DrawCircle(window->bitmap, x, y, r, colour);
    
    if(filled==true){
        graphics.FloodFill(window->bitmap, x, y, colour);
    }
    
    FreeLock(&window->clipRectsLock);
    
}

void DrawTriangle(window_t* window, uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2, uint32_t x3, uint32_t y3, uint32_t colour, bool filled){
    debug_write_string("Intuition: Need to implement Triangle Draw\n");
    
    Lock(&window->clipRectsLock);
    /*
    for(uint32_t i = 0;i < window->clipRects; ++i){
        
        uint32_t clipX1 = window->clipRect[i].x;
        uint32_t clipX2 = window->clipRect[i].x + window->clipRect[i].w;
        uint32_t clipY1 = window->clipRect[i].y;
        uint32_t clipY2 = window->clipRect[i].y + window->clipRect[i].h;
        
        uint32_t winX1 = window->x;
        uint32_t winX2 = winX1 + window->w;
        uint32_t winY1 = window->y;
        uint32_t winY2 = winY1 + window->h;
        
        if (clipX1 < winX2 && clipX2 > winX1 && clipY1 < winY2 && clipY2 > winY1){
            window->clipRect[i].needsUpdate = true;
            window->needsRedraw = true;
        }
     }
    */
    
    graphics.DrawLine(window->bitmap,x1, y1, x2, y2, colour);
    graphics.DrawLine(window->bitmap,x2, y2, x3, y3, colour);
    graphics.DrawLine(window->bitmap,x3, y3, x1, y1, colour);
    
    if(filled==true){
        
        //find the centre of the triangle
        int x = (x1 + x2 + x3)/3;
        int y = (y1 + y2 +y3)/3;
        graphics.FloodFill(window->bitmap, x, y, colour);
    }
    FreeLock(&window->clipRectsLock);
    
    
}

void DrawVectorImageI(window_t* window, uint32_t x, uint32_t y, uint8_t* data){

    Lock(&window->clipRectsLock);
    /*
    for(uint32_t i = 0;i < window->clipRects; ++i){
        
        uint32_t clipX1 = window->clipRect[i].x;
        uint32_t clipX2 = window->clipRect[i].x + window->clipRect[i].w;
        uint32_t clipY1 = window->clipRect[i].y;
        uint32_t clipY2 = window->clipRect[i].y + window->clipRect[i].h;
        
        uint32_t winX1 = window->x;
        uint32_t winX2 = winX1 + window->w;
        uint32_t winY1 = window->y;
        uint32_t winY2 = winY1 + window->h;
        
        if (clipX1 < winX2 && clipX2 > winX1 && clipY1 < winY2 && clipY2 > winY1){
            window->clipRect[i].needsUpdate = true;
            window->needsRedraw = true;
        }
     }
    */
    
    graphics.DrawVectorImage(window->bitmap, x, y, data);
    FreeLock(&window->clipRectsLock);
}

void DrawString(window_t* window, uint32_t x, uint32_t y, char* string, uint32_t fColour, uint32_t bColour){
    graphics.RenderString(window->bitmap,intuition.defaultFont,x,y,string,fColour,bColour);
}

library_t* OpenIntuition(library_t* lib){
    
    lib->openCount += 1;
    return lib; // this is a singleton library,
}

void CloseIntuition(library_t* lib){
    
    lib->openCount -= 1;
    
    if(lib->openCount < 0){
        //do something as the library is no longer needed
        lib->openCount = 0;
    }
    
}


void InitIntuition(library_t* library){
    //this is a called by exec's AddLibrary
    
    library->node.name = "intuition.library";
    library->node.nodeType = NODE_LIBRARY;
    
    intuition.windowList = (list_t*)executive->Alloc(sizeof(list_t));
    InitList(intuition.windowList);
    intuition.windowList->node.name = "Intuition Window List";
    screenTitle = NULL;
    
    //clear screen
    graphics.DrawRect(&graphics.frameBuffer, 0,0,graphics.frameBuffer.width,graphics.frameBuffer.height,intuition.backgroundColour);
    drawTitleBar();
    
    
    mouseVisible = true;
    drawMouse();
    
    
}



void SetTheme(int guiThemeValue){
    
    switch(guiThemeValue){

        case THEME_NEW:
            intuition.backgroundColour              = intuition.grey2;
            
            intuition.defaultWindowForegroundColour = intuition.black;
            intuition.defaultWindowBackgroundColour = intuition.grey2;
            intuition.defaultWindowHighlightColour  = intuition.blue2;
            intuition.mouseImage                    = newMousePointer;
            intuition.normalMouseImage              = newMousePointer;
            intuition.busyMouseImage                = oldBusyPointer;
            intuition.windowAutorise                = false;
            intuition.doubleClickTime               = 250;
            intuition.DrawDecoration                = drawWindowDecorationNew;
            intuition.defaultFont                   = topazNew_font;
            intuition.GimmeZeroZero                 = GimmeZeroZeroOld;
            
            intuition.DrawSystemDepthGadget         = DrawDepthGadgetNew;
            intuition.systemDepthX                  = 23;
            intuition.systemDepthY                  = 0;
            intuition.systemDepthW                  = 23;
            intuition.systemDepthH                  = 22;
            intuition.systemDepthFlags              = GADGET_FLAG_ORIGIN_RIGHT;
            
            intuition.DrawSystemSizeGadget          = DrawSizeGadgetNew;
            intuition.systemSizeX                   = 18;
            intuition.systemSizeY                   = 19;
            intuition.systemSizeW                   = 17;
            intuition.systemSizeH                   = 18;
            intuition.systemSizeFlags               = GADGET_FLAG_ORIGIN_RIGHT | GADGET_FLAG_ORIGIN_BOTTOM;
            
            intuition.DrawSystemCloseGadget         = DrawCloseGadgetNew;
            intuition.systemCloseX                  = 0;
            intuition.systemCloseY                  = 0;
            intuition.systemCloseW                  = 20;
            intuition.systemCloseH                  = 22;
            
            break;
        case THEME_MAC:
            intuition.backgroundColour              = intuition.grey2;
            intuition.defaultWindowForegroundColour = intuition.black;
            intuition.defaultWindowBackgroundColour = intuition.white;
            intuition.defaultWindowHighlightColour  = intuition.black;
            intuition.mouseImage                    = macMousePointer;
            intuition.normalMouseImage              = macMousePointer;
            intuition.busyMouseImage                = oldBusyPointer;
            intuition.windowAutorise                = true;
            intuition.doubleClickTime               = 250;
            intuition.DrawDecoration                = drawWindowDecorationMac;
            intuition.defaultFont                   = topazNew_font;
            intuition.GimmeZeroZero                 = GimmeZeroZeroMac;
            
            
            intuition.DrawSystemDepthGadget         = DrawDepthGadgetMac;
            intuition.systemDepthX                  = 20;
            intuition.systemDepthY                  = 4;
            intuition.systemDepthW                  = 11;
            intuition.systemDepthH                  = 11;
            intuition.systemDepthFlags              = GADGET_FLAG_ORIGIN_RIGHT;
            
            intuition.DrawSystemSizeGadget          = DrawSizeGadgetMac;
            intuition.systemSizeX                   = 16;
            intuition.systemSizeY                   = 17;
            intuition.systemSizeW                   = 15;
            intuition.systemSizeH                   = 16;
            intuition.systemSizeFlags               = GADGET_FLAG_ORIGIN_RIGHT | GADGET_FLAG_ORIGIN_BOTTOM;
            
            intuition.DrawSystemCloseGadget         = DrawCloseGadgetMac;
            intuition.systemCloseX                  = 9;
            intuition.systemCloseY                  = 4;
            intuition.systemCloseW                  = 11;
            intuition.systemCloseH                  = 11;
            break;
            
        default:
            
            intuition.backgroundColour              = intuition.blue;
            intuition.defaultWindowForegroundColour = intuition.white;
            intuition.defaultWindowBackgroundColour = intuition.blue;
            intuition.defaultWindowHighlightColour  = intuition.orange;
            intuition.mouseImage                    = oldMousePointer;
            intuition.normalMouseImage              = oldMousePointer;
            intuition.busyMouseImage                = oldBusyPointer;
            intuition.windowAutorise                = false;
            intuition.doubleClickTime               = 250;
            intuition.DrawDecoration                = drawWindowDecorationOld;
            intuition.defaultFont                   = topazOld_font;
            intuition.GimmeZeroZero                 = GimmeZeroZeroOld;
            
            intuition.DrawSystemDepthGadget         = DrawDepthGadgetOld;
            intuition.systemDepthX                  = 25;
            intuition.systemDepthY                  = 0;
            intuition.systemDepthW                  = 20;
            intuition.systemDepthH                  = 20;
            intuition.systemDepthFlags              = GADGET_FLAG_ORIGIN_RIGHT;
            
            intuition.DrawSystemSizeGadget          = DrawSizeGadgetOld;
            intuition.systemSizeX                   = 17;
            intuition.systemSizeY                   = 18;
            intuition.systemSizeW                   = 17;
            intuition.systemSizeH                   = 18;
            intuition.systemSizeFlags               = GADGET_FLAG_ORIGIN_RIGHT | GADGET_FLAG_ORIGIN_BOTTOM;
            
            intuition.DrawSystemCloseGadget         = DrawCloseGadgetOld;
            intuition.systemCloseX                  = 4;
            intuition.systemCloseY                  = 0;
            intuition.systemCloseW                  = 20;
            intuition.systemCloseH                  = 21;
            break;
    }
    
    
}



void LoadIntuitionLibrary(){
    
    // a library's load function is supposed to be called by the ELF loader at load time to build the function table.
    //
    
    // It is the responsibility of Intuition to check if the graphics.library supports
    // all the drawing functions, if not it then Intuition must had fallback functions
    // to perform all the missing graphics drawing functions.
    
    inputStruct.focused = NULL;
    //intuition.library.node.name = "intuition.library"; // now done in InitIntuition()
    intuition.library.version   = 1;
    intuition.library.Init      = InitIntuition;
    intuition.library.Open      = OpenIntuition;
    intuition.library.Close     = CloseIntuition;
    intuition.screenWidth       = graphics.frameBuffer.width;;
    intuition.screenHeight      = graphics.frameBuffer.height;
    
    intuition.blue              = graphics.Colour(0x00,0x55,0xAA,0xFF);
    intuition.white             = graphics.Colour(0xFF,0xFF,0xFF,0xFF);
    intuition.orange            = graphics.Colour(0xFF,0x88,00,0xFF);
    intuition.black             = graphics.Colour(0x00,0x00,0x22,0xFF);
    intuition.grey              = graphics.Colour(171,187,205,0xFF);
    intuition.red               = graphics.Colour(217,46,31,0xFF);
    intuition.grey2             = graphics.Colour(170,170,170,0xFF);
    intuition.blue2             = graphics.Colour(102,136,187,0xFF);

    intuition.Update            = IntuitionUpdate;
    intuition.OpenWindowPrivate = OpenWindowPrivate;
    intuition.OpenWindow        = OpenWindow;
    intuition.SetTheme          = SetTheme;
    intuition.SetScreenTitle    = SetScreenTitle;
    intuition.MoveWindow        = MoveWindow;
    intuition.ResizeWindow      = ResizeWindow;
    intuition.WindowToBack      = WindowToBack;
    intuition.WindowToFront     = WindowToFront;
    intuition.PriorityOrderPrivate  = PriorityOrderPrivate;
    intuition.Focus             = Focus;
    intuition.Plot              = Plot;
    intuition.PutChar           = PutChar;
    intuition.DrawString        = DrawString;
    intuition.DrawRectangle     = DrawRectangle;
    intuition.DrawLine          = DrawLineI;
    intuition.FloodFill         = FloodFillI;
    intuition.DrawCircle        = DrawCircleI;
    intuition.DrawTriangle      = DrawTriangle;
    intuition.DrawVectorImage   = DrawVectorImageI;
    
    intuition.ClearWindow       = ClearWindow;
    intuition.RedrawWindow      = RedrawWindow;
    intuition.Request           = Request;
    intuition.CreateGadget      = CreateGadget;
    intuition.SetBusy           = SetBusy;

    SetTheme(guiTheme);
  
}


