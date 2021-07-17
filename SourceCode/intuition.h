//
//  intuition.h
//
//  Created by Matt Parsons on 26/10/2020.
//  Copyright © 2020 Matt Parsons. All rights reserved.
//

#ifndef intuition_h
#define intuition_h

#include "stdheaders.h"
#include "graphics.h"
#include "list.h"
#include "ports.h"
#include "library.h"

typedef struct {
    bool isVisible;
    bool needsUpdate;
    uint32_t x;
    uint32_t y;
    uint32_t w;
    uint32_t h;
} clipRect_t;

typedef struct window_t window_t;

struct window_t{
    node_t node;
    window_t* parent;
    char* screenTitle;
    uint64_t doubleClick;
    uint32_t x;
    uint32_t y;
    uint32_t w;
    uint32_t h;
    uint32_t minWidth;
    uint32_t minHeight;
    uint32_t maxWidth;
    uint32_t maxHeight;
    uint32_t innerX;
    uint32_t innerY;
    uint32_t innerW;
    uint32_t innerH;
    uint32_t foregroundColour;
    uint32_t backgroundColour;
    uint32_t highlightColour;
    int32_t mouseX;             //window realtive mose coordinates
    int32_t mouseY;
    uint64_t flags;
    messagePort_t* eventPort;
    bool focused;
    bool needsRedraw;
    bool noRise;             //no clicking to front
    bool isBusy;             //window busy, show busy pointer and should not receive events
    bitmap_t* bitmap;
    int32_t autoRefeshCountdown;
    list_t gadgetList;
    
    lock_t clipRectsLock;   //always lock the cliprects list before using.
    uint32_t clipRects;
    clipRect_t clipRect[64];    //Very large, but only consumes 3072bytes per window
};

#define GADGET_FLAG_ORIGIN_RIGHT    1
#define GADGET_FLAG_ORIGIN_BOTTOM   2

#define GADGET_STATE_NORMAL 0
#define GADGET_STATE_HOVER  1
#define GADGET_STATE_ACTIVE 2

typedef struct gadget_t gadget_t;

struct gadget_t{
    node_t node;
    window_t* window;
    bool isDecoration;  // will be drawn when the window decoration refreshes.
    uint32_t x;
    uint32_t y;
    int32_t w;
    int32_t h;
    int32_t ix;    //internal x , these are used by the gfx library and intuition
    int32_t iy;    //internal y
    uint32_t mouseState;
    uint64_t doubleClickTimeOut;
    uint32_t flags;
    uint32_t state;
    uint32_t customEventCode;   // generate a custom event code.
    void (*MouseDown)(gadget_t*);
    void (*MouseDoubleClick)(gadget_t*);
    void (*MouseUp)(gadget_t*);
    void (*MouseMoved)(gadget_t*);
    void (*Draw)(gadget_t*);  // if a System Type, then this is the code that draws it
};


#define WINDOW_EVENT_RESIZE     0x1
#define WINDOW_EVENT_RESIZE_END 0x2
#define WINDOW_EVENT_KEYDOWN    0x4

typedef struct{
    message_t message;
    uint64_t flags;
    window_t* window;
    gadget_t* gadget;
    uint8_t rawKey;
    uint8_t scancode;
    int32_t mouseX;
    int32_t mouseY;
    int32_t mouseXrel;
    int32_t mouseYrel;
}intuitionEvent_t;

#define WINDOW_DRAGGABLE    0x1
#define WINDOW_CLOSE_GADGET 0x2
#define WINDOW_DEPTH_GADGET 0x4
#define WINDOW_ZOOM_GADGET  0x8
#define WINDOW_TITLEBAR     0x10
#define WINDOW_RESIZABLE    0x20
#define WINDOW_BORDERLESS   0x40




typedef struct{
    library_t library;
    bool needsUpdate;
    graphics_t* graphics;    //Eventually the intuition library will need to open the gfx library
    uint32_t blue;
    uint32_t white;
    uint32_t orange;
    uint32_t black;
    uint32_t grey;
    uint32_t red;
    uint32_t grey2;
    uint32_t blue2;
    uint32_t backgroundColour;
    uint32_t defaultWindowForegroundColour;
    uint32_t defaultWindowBackgroundColour;
    uint32_t defaultWindowHighlightColour;
    uint8_t* defaultFont;
    uint32_t screenWidth;
    uint32_t screenHeight;
    uint8_t* mouseImage;
    uint8_t* normalMouseImage;
    uint8_t* busyMouseImage;
    uint64_t doubleClickTime;
    bool windowAutorise;
    
    void (*DrawSystemDepthGadget)(gadget_t*);
    uint32_t systemDepthX;
    uint32_t systemDepthY;
    uint32_t systemDepthW;
    uint32_t systemDepthH;
    uint32_t systemDepthFlags;
    void (*DrawSystemSizeGadget)(gadget_t*);
    uint32_t systemSizeX;
    uint32_t systemSizeY;
    uint32_t systemSizeW;
    uint32_t systemSizeH;
    uint32_t systemSizeFlags;
    void (*DrawSystemCloseGadget)(gadget_t*);
    uint32_t systemCloseX;
    uint32_t systemCloseY;
    uint32_t systemCloseW;
    uint32_t systemCloseH;
    
    window_t* (*OpenWindow)(window_t* parent,uint32_t x, uint32_t y, uint32_t w, uint32_t h,uint64_t flags,char* title);
    void (*DrawDecoration)(window_t* window);
    void (*GimmeZeroZero)(window_t* window);
    void (*SetScreenTitle)(window_t* window,char* title);
    void (*MoveWindow)(window_t* window,int32_t x, int32_t y);
    void (*ResizeWindow)(window_t*, uint32_t, uint32_t );
    void (*WindowToBack)(window_t* window);
    void (*WindowToFront)(window_t* window);
    void (*PriorityOrderPrivate)(window_t* window);
    void (*Focus)(window_t* window);
    void (*SetBusy)(window_t* window, bool state);
    void (*Plot)(window_t* window,uint32_t x, uint32_t y, uint32_t colour);
    void (*ClearWindow)(window_t*);
    void (*PutChar)(window_t*, uint32_t x, uint32_t y, uint8_t character, uint32_t fColour, uint32_t bColour);
    void (*DrawString)(window_t*, uint32_t x, uint32_t y, char* string, uint32_t fColour, uint32_t bColour);
    void (*DrawRectangle)(window_t* window,uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t colour);
    void (*DrawLine)(window_t* window,uint32_t x0, uint32_t y0, uint32_t x1, uint32_t y1, uint32_t colour);
    void (*DrawCircle)(window_t* window, uint32_t x, uint32_t y, uint32_t r, uint32_t colour);
    void (*FloodFill)(window_t* window, uint32_t x, uint32_t y, uint32_t colour);
    void (*DrawVectorImage)(window_t* window, uint32_t x, uint32_t y, uint8_t* data);
    
    void (*RedrawWindow)(window_t* window);
    gadget_t* (*CreateGadget)(window_t* window,uint32_t flags);
    window_t* (*Request)(char* title);
} intuition_t;

extern intuition_t intuition;

void LoadIntuitionLibrary(void);
void updateMouse(void);
void clearMouse(void);
void drawMouse(void);


extern list_t windowList;
extern volatile int32_t mouseX;
extern volatile int32_t mouseY;
extern int32_t mouseXold;
extern int32_t mouseYold;

window_t* OpenWindow(window_t* parent,uint32_t x, uint32_t y, uint32_t w, uint32_t h,uint64_t flags,char* title);
void SetScreenTitle(window_t* window,char* title);
void MoveWindow(window_t* window,int32_t x, int32_t y);
void Focus(window_t* window);
void ResizeWindow(window_t*, uint32_t, uint32_t );

void IntuitionUpdate(void);
void RedrawWindow(window_t* window);

void WindowToBack(window_t* window);
void WindowToFront(window_t* window);
void PriorityOrderPrivate(window_t* window);


// drawing functions
void Plot(window_t* window,uint32_t x, uint32_t y, uint32_t colour);
void PutChar(window_t*, uint32_t x, uint32_t y, uint8_t character, uint32_t fColour, uint32_t bColour);
void DrawRectangle(window_t* window,uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t colour);

#endif /* intuition_h */
