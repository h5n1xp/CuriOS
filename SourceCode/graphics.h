//
//  graphics.h
//  GFXConvert
//
//  Created by Matt Parsons on 25/10/2020.
//  Copyright © 2020 Matt Parsons. All rights reserved.
//

#ifndef graphics_h
#define graphics_h

#include "stdheaders.h"
#include "memory.h"
#include "multiboot.h"

typedef struct{
    node_t node;
    uint32_t width;
    uint32_t height;
    uint32_t bpp;
    lock_t lock;
    void* buffer;
} bitmap_t;




typedef struct{
    union{
        struct{
            uint8_t blue;
            uint8_t green;
            uint8_t red;
            uint8_t alpha;
        };
        uint32_t data;
    };
} colour_t;

typedef struct{
    node_t node;
    uint32_t count;
    colour_t* colour;
} palette_t;

//  simple image format, for sprites, intuition icons/gadgets etc...
typedef struct{
    node_t node;
    uint64_t type;
    uint64_t colours;
    uint64_t width;
    uint64_t height;
    uint64_t frames;
    uint32_t* palette;
    uint8_t* pixels;
    uint32_t* backingStore;
}sprite_t;

typedef struct{
    uint32_t x;
    uint32_t y;
}Point2D;


/*
typedef struct{
    node_t node;
    uint32_t width;
    uint32_t height;
    uint8_t* data;
} font_t;
*/

extern uint8_t topazNew_font[];
extern uint8_t topazOld_font[];

typedef struct{
    library_t library;

    bitmap_t* (*NewBitmap)(uint32_t w,uint32_t h);
    void (*FreeBitmap)(bitmap_t* bitmap);
    uint32_t (*Colour)(uint8_t red, uint8_t green, uint8_t blue,uint8_t alpha);
    uint32_t (*GetPixel)(bitmap_t* bm, uint32_t x, uint32_t y);
    void (*PutPixel)(bitmap_t* bm,uint32_t x, uint32_t y, uint32_t colour);
    void (*RenderGlyph)(bitmap_t* bm, uint8_t* font, uint32_t x, uint32_t y, uint8_t character, uint32_t fColour, uint32_t bColour);
    void (*RenderString)(bitmap_t* bm,uint8_t* font,uint32_t x,uint32_t y,char* str,uint32_t fColour,uint32_t bColour);
    void (*DrawRect)(bitmap_t* bm, uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t rgb);
    void (*BlitBitmap)(bitmap_t* source, bitmap_t* dest, uint32_t x, uint32_t y);
    void (*DrawLine)(bitmap_t* bm,uint32_t x0, uint32_t y0, uint32_t x1, uint32_t y1, uint32_t rgb);
    void (*DrawCircle)(bitmap_t* bm, uint32_t x0, uint32_t y0, uint32_t r, uint32_t rgb);
    void (*FloodFill)(bitmap_t* bm, uint32_t x, uint32_t y, uint32_t rgb);
    void (*DrawVectorImage)(bitmap_t* bm, uint32_t x, uint32_t y, uint8_t* data);
    void (*BlitRect)(bitmap_t* source, uint32_t sx, uint32_t sy, uint32_t w, uint32_t h, bitmap_t* dest, uint32_t x, uint32_t y);
    void (*ClearBitmap)(bitmap_t* bm,uint32_t rgb);
    void (*DrawSprite)(sprite_t* sprite, uint32_t x, uint32_t y);
    void (*ClearSprite)(sprite_t* sprite);
    
    palette_t* (*CreatePalette)(uint32_t numberOfColours);
    void (*SetColour)(palette_t* palette,uint32_t colour, uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha);
    colour_t (*GetColour)(palette_t* palette, uint32_t colour);
    
    void (*ChangeFrameBufferPrivate)(void* address,uint32_t width, uint32_t height, uint32_t bpp, bool hasHardwareCursor);
    
    bitmap_t* (*ConvertIndexImageToBitmap)(uint8_t* image, uint32_t w, uint32_t h, palette_t* palette);
    void (*SetCursorImage)(bitmap_t* image);
    void (*ClearCursor)(void);              //this will be an empty function on machines with a hardware cursor
    void (*DrawCursor)(uint32_t x, uint32_t y);       //will simply update the cursor x and y on machines with a hardware cursor
    
    //Private data area, only access this data via the accessor functions
    bitmap_t frameBuffer;
    uint32_t cursorX;               //for machines with no hardware cursor
    uint32_t cursorY;               //for machines with no hardware cursor
    uint32_t cursorSize;            //for machines with no hardware cursor optimise so the blit is no larger than needed
    bitmap_t* cursorBackingStore;   //for machines with no hardware cursor
    bitmap_t* cursorBitmapImage;

    
}graphics_t;


extern graphics_t graphics;


void LoadGraphicsLibrary();


#endif /* graphics_h */
