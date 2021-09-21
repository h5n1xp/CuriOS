//
//  SystemLog.c
//  GFXConvert
//
//  Created by Matt Parsons on 31/10/2020.
//  Copyright © 2020 Matt Parsons. All rights reserved.
//

#include "SystemLog.h"
#include "intuition.h"
#include "memory.h"
#include "string.h"


uint32_t curX;
uint32_t curY;
uint32_t sysLogRow;
uint32_t sysLogCol;
uint32_t sysLogWidth;
uint32_t sysLogHeight;
uint8_t* sysLogBuffer;
window_t* sysLogWindow;

char scrTit[] = "Debugging Console";

intuition_t* intui;

void debug_show(){
    intui->WindowToFront(sysLogWindow); //<--This shouldn't be called outside of the input task, as it's not thread safe yet.
    intui->SetVisible(sysLogWindow,true);
}

void debug_hide(){
    intui->SetVisible(sysLogWindow,false);
}

void clear_cursor(void){
    
    if(sysLogWindow==NULL){return;}
    
    intui->DrawRectangle(sysLogWindow,curX, curY, 8, 16, sysLogWindow->backgroundColour);
    //sysLogWindow->needsRedraw = true;
    intui->Redraw(sysLogWindow);
}

void draw_cursor(uint32_t x,uint32_t y){
    
    if(sysLogWindow==NULL){return;}
    
    curX = (x*8)+4;
    curY = (y*16)+22;
    intui->DrawRectangle(sysLogWindow,curX, curY, 8, 16, sysLogWindow->highlightColour);
    //sysLogWindow->needsRedraw = true;
    intui->Redraw(sysLogWindow);
}

void debug_putchar_at(uint32_t x, uint32_t y, uint8_t c){
    
    sysLogBuffer[(y*sysLogWidth) + x] = c;
    intui->PutChar(sysLogWindow, (x*8)+4, (y*16)+22, c, sysLogWindow->foregroundColour, sysLogWindow->backgroundColour);
    //sysLogWindow->needsRedraw = true;
    intui->Redraw(sysLogWindow);
}

void debug_scroll_one_line(){


    for(uint32_t y=0;y<sysLogHeight-1;++y){
        for(uint32_t x=0;x<sysLogWidth;++x){
            debug_putchar_at(x,y,sysLogBuffer[((y+1) * sysLogWidth) + x]);
        }
    }
    for(uint32_t x=0;x<sysLogWidth;++x){
        debug_putchar_at(x,sysLogHeight-1,' ');
    }

}



void debug_putchar(char c){
    
    if(sysLogWindow==NULL){return;}
    
    clear_cursor();
    
    if(c=='\n'){
        ++sysLogRow;
        sysLogCol = 0;
    }else if(c=='\t'){
    
        sysLogCol = (sysLogCol + 7) & 0xFFF8;
        
    }else{
        debug_putchar_at(sysLogCol, sysLogRow, c);
        ++sysLogCol;
    }
    
    if (sysLogCol == sysLogWidth) {
        sysLogCol = 0;
        ++sysLogRow;
    }
    
    
    if (sysLogRow == sysLogHeight){
        debug_scroll_one_line();
        sysLogRow = sysLogHeight - 1;
    }
    
    draw_cursor(sysLogCol,sysLogRow);
    
}



void debug_write_string(char* str){
    uint32_t size = strlen(str);
    
    for (size_t i = 0; i < size; i++){
        debug_putchar(str[i]);
    }
    
    
}

void debug_write_hex(uint32_t n){
    int32_t tmp;
     
     debug_write_string("0x");
     
     char noZeroes = 1;
     
     int i;
     
     for (i = 28; i > 0; i -= 4){
         tmp = (n >> i) & 0xF;
         if (tmp == 0 && noZeroes != 0){
             continue;
         }
         
         if (tmp >= 0xA){
             noZeroes = 0;
             debug_putchar(tmp - 0xA + 'a');
         }else{
             noZeroes = 0;
             debug_putchar(tmp + '0');
         }
     }
     
     tmp = n & 0xF;
     if (tmp >= 0xA){
         debug_putchar(tmp - 0xA + 'a');
     }else{
         debug_putchar(tmp + '0');
     }
    
}

void debug_write_dec(uint32_t n){

    if (n == 0){
         debug_putchar('0');
         return;
     }
     
     int32_t acc = n;
     char c[32];
     int i = 0;
     while (acc > 0){
         c[i] = '0' + acc%10;
         acc /= 10;
         i++;
     }
     
     c[i] = 0;
     
     char c2[32];
     c2[i--] = 0;
     int j = 0;
     while(i >= 0){
         c2[i--] = c[j++];
     }
     
     debug_write_string(c2);
    
}

void debug_backspace(void){
    
    clear_cursor();
    
    sysLogCol -= 1;
    if(sysLogCol>sysLogWidth){
        sysLogCol = sysLogWidth - 1;
        sysLogRow -=1;
    }
    debug_putchar_at(sysLogCol, sysLogRow,' ');
    
    draw_cursor(sysLogCol, sysLogRow);
    
}

void InitSystemLog(uint32_t x, uint32_t y, uint32_t w, uint32_t h){
    
    intui = (intuition_t*)executive->OpenLibrary("intuition.library",0);
    
    sysLogWindow = NULL;
    sysLogWindow = intui->OpenWindowPrivate(NULL, x, y, w, h,WINDOW_TITLEBAR | WINDOW_DRAGGABLE | WINDOW_DEPTH_GADGET, "SysLog");
    
    sysLogWindow->isVisible = true; // need to see the window upon boot while debuggin PCI Driver
    
    intui->SetScreenTitle(sysLogWindow,scrTit);
    
    sysLogWidth = ((w - 4) / 8) - 1;
    sysLogHeight = ((h - 22) / 16) ;
    
    sysLogRow = 0;
    sysLogCol = 0;
    curX = 4;
    curY = 22;
    
    //should use AllocMem as this doesnt need to be a node structure
    node_t* node = executive->Alloc((sysLogWidth*sysLogHeight) + sizeof(node_t),0);
    node += 1;
    sysLogBuffer = (uint8_t*)node;
    
    //clear the buffer
    for(uint32_t i=0; i< sysLogWidth*sysLogHeight;++i){
        sysLogBuffer[i] = ' ';
    }
    
}
