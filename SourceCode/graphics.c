//
//  graphics.c
//  GFXConvert
//
//  Created by Matt Parsons on 25/10/2020.
//  Copyright © 2020 Matt Parsons. All rights reserved.
//

#include "graphics.h"
#include "font.h"
#include "math.h"

graphics_t graphics;


uint32_t get_pixel16(bitmap_t* bm, uint32_t x,uint32_t y){
    uint16_t* p = bm->buffer;
    return (uint32_t)p[(y * bm->width) + x];
}

void put_pixel16(bitmap_t* bm, uint32_t x,uint32_t y, uint32_t colour){

       //clipping
       if(x>=bm->width){
           return;
       }
    
       if(y>=bm->height){
           return;
       }

    uint16_t* p = bm->buffer;
    p[(y * bm->width) + x] = (uint16_t)colour;
}


uint32_t get_pixel32(bitmap_t* bm, uint32_t x,uint32_t y){
    //Lock(&bm->lock);
    //executive->Forbid();
    uint32_t* p = bm->buffer;
    //executive->Permit();
    //FreeLock(&bm->lock);
    return (uint32_t)p[(y * bm->width) + x];
}

void put_pixel32(bitmap_t* bm, uint32_t x,uint32_t y, uint32_t colour){
    //Lock(&bm->lock);
    //executive->Forbid();
    //clipping
    if(x>=bm->width){ //can't be negative
        //executive->Permit();
        //FreeLock(&bm->lock);
        return;
    }
 
    if(y>=bm->height){ //can't be negative
        //executive->Permit();
        //FreeLock(&bm->lock);
        return;
    }

    uint32_t* p = bm->buffer;
    p[(y * bm->width) + x] = colour;
    //executive->Permit();
    //FreeLock(&bm->lock);
}


void PutPixelFast(bitmap_t* bm, uint32_t x,uint32_t y, uint32_t colour){
    //Lock(&bm->lock);
    //executive->Forbid();
    //No Clipping... You have been warned
    uint32_t* p = bm->buffer;
    p[(y * bm->width) + x] = colour;
    //executive->Permit();
    //FreeLock(&bm->lock);
}



uint32_t colour16(uint8_t red,uint8_t green, uint8_t blue,uint8_t alpha){
    
    if(alpha ==255){
        red = red >> 3;
        green = green >> 3;
        blue = blue >> 3;
    }else{
        return 0;
    }
    
    
    return (red << 12) | (green << 6) | blue;
    
}

uint32_t colour32(uint8_t red, uint8_t green, uint8_t blue,uint8_t alpha){
    
    uint32_t retVal = 0;
    retVal |=(alpha << 24);
    retVal |=(red << 16);
    retVal |=(green << 8);
    retVal |= blue;
    return retVal;//(255 << 24) | (red << 16) | (green << 8) | blue;
    
}



/*
void CopyRect(uint32_t sx,uint32_t sy,uint32_t dx,uint32_t dy,uint32_t w, uint32_t h){
    
    
    for(int y=0; y<)
    
    
}
*/

//*************** Assembly highspeed copy
inline void movsd(void* dst, const void* src, size_t size){
    asm volatile("rep movsl" : "+D"(dst), "+S"(src), "+c"(size) : : "memory");
}

void BlitRect(bitmap_t* source, uint32_t sx, uint32_t sy, uint32_t w, uint32_t h, bitmap_t* dest, uint32_t x, uint32_t y){
    
    //Lock(&source->lock);
    //Lock(&dest->lock);
    //executive->Forbid();
    
    uint32_t* s = source->buffer;
    uint32_t* d = dest->buffer;
    
    
    if(x+w >= dest->width){w = dest->width - x;}
    if(y+h >= dest->height){h = dest->height - y;}
    
    if(w < 1 || h < 1){
        //executive->Permit();
        //FreeLock(&dest->lock);
        //FreeLock(&source->lock);
        return;
    }
    
    for(uint32_t by=0;by<h;++by){
        
        /*
        for(uint32_t bx=0;bx<w;++bx){
            
            d[((by+y)*dest->width)+bx+x] = s[((by+sy)*source->width)+bx+sx];
        }
        */
        
        movsd(&d[((by+y)*dest->width)+x],&s[((by+sy)*source->width)+sx],w);
        
    }
    
    
    
  /*
  for(uint32_t by=0;by<h;++by){
      for(uint32_t bx=0;bx<w;++bx){
          
          PutPixel(dest,bx+x,by+y, GetPixel(source,bx+sx,by+sy));

      }

  }
   */
    //executive->Permit();
    //FreeLock(&dest->lock);
    //FreeLock(&source->lock);
}

void BlitBitmap(bitmap_t* source, bitmap_t* dest, uint32_t x, uint32_t y){
    //Lock(&source->lock);
    //Lock(&dest->lock);
    //executive->Forbid();
    
    uint32_t* s = source->buffer;
    uint32_t* d = dest->buffer;
    
    
    uint32_t w = source->width;
    uint32_t h = source->height;
    
    if(x+w >= dest->width){w = dest->width - x;}
    if(y+h >= dest->height){h = dest->height - y;}
    
    if(w < 1 || h < 1){
        //executive->Permit();
        //FreeLock(&dest->lock);
        //FreeLock(&source->lock);
        return;
    }
    
    
  for(uint32_t by=0;by<h;++by){
      /*
      for(uint32_t bx=0;bx<w;++bx){
          
          // d[ ((by+y)*dest->width) + bx+x] = s[(by*source->width)+x];
          
          // PutPixel(dest,bx+x,by+y, GetPixel(source,bx,by));

      }
       */
    movsd(&d[((by+y)*dest->width)+x],&s[(by*source->width)],w);
  }
    //executive->Permit();
    //FreeLock(&dest->lock);
    //FreeLock(&source->lock);
}

bitmap_t* NewBitmap(uint32_t w,uint32_t h){
    
    bitmap_t* bm = (bitmap_t*)executive->Alloc( (w*h*4) + sizeof(bitmap_t));
    bm->node.nodeType = NODE_BITMAP;
    bm->width = w;
    bm->height = h;
    bm->bpp = 32;
    void* buffer = bm + 1;
    bm->buffer = buffer;
    return bm;
    
}

void FreeBitmap(bitmap_t* bitmap){
    executive->Dealloc((node_t*)bitmap);
}

bitmap_t* ResizeBitmap(bitmap_t* old,uint32_t w, uint32_t h){
    
    if(old->width > w && old->height > h){
        return old;
    }
    
     bitmap_t* bm = (bitmap_t*)executive->Alloc( (w*h*4) + sizeof(bitmap_t));
    
    return bm;
}

void DrawRect(bitmap_t* bm, uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t rgb){
    //Lock(&bm->lock);
    //executive->Forbid();
    uint32_t height = y + h;
    uint32_t width  = x + w;
   
    uint32_t* d = (uint32_t*)bm->buffer;
    
    if(height >= bm->height){height = bm->height;}
    if(width  >= bm->width){width=bm->width;}
    
    for(uint32_t j=y;j<height;++j){
        for(uint32_t i=x;i<width;++i){
        
            d[(j*bm->width)+i] = rgb;
        }
    }
    //executive->Permit();
    //FreeLock(&bm->lock);
}

void ClearBitmap(bitmap_t* bm,uint32_t rgb){
    
    uint32_t size = bm->width * bm->height;
    uint32_t* d = bm->buffer;
    
    do{
        size -=1;
        d[size] = rgb;
    }while(size > 0);
    
}

void RenderGlyph(bitmap_t* bm, uint8_t* font, uint32_t x, uint32_t y, uint8_t character, uint32_t fColour, uint32_t bColour){

           
           for(int cy=0;cy<16;++cy){
               
                uint8_t font_row = font[((cy)*256)+character];
               
               
               
                   if( font_row & 0x80){
                       //PutPixelFast(bm,x,y+cy,fColour);
                       put_pixel32(bm,x,y+cy,fColour);
                   }else{
                       //PutPixelFast(bm,x,y+cy,bColour);
                       put_pixel32(bm,x,y+cy,bColour);
                   }
                   
                   if(font_row & 0x40){
                       //PutPixelFast(bm,x+1,y+cy,fColour);
                       put_pixel32(bm,x+1,y+cy,fColour);
                   }else{
                       //PutPixelFast(bm,x+1,y+cy,bColour);
                       put_pixel32(bm,x+1,y+cy,bColour);
                   }
    
                   if(font_row & 0x20){
                       //PutPixelFast(bm,x+2,y+cy,fColour);
                       put_pixel32(bm,x+2,y+cy,fColour);
                   }else{
                       //PutPixelFast(bm,x+2,y+cy,bColour);
                       put_pixel32(bm,x+2,y+cy,bColour);
                   }
                   
                   if(font_row & 0x10){
                       //PutPixelFast(bm,x+3,y+cy,fColour);
                       put_pixel32(bm,x+3,y+cy,fColour);
                   }else{
                       //PutPixelFast(bm,x+3,y+cy,bColour);
                       put_pixel32(bm,x+3,y+cy,bColour);
                   }
                   
                   if(font_row & 0x08){
                       //PutPixelFast(bm,x+4,y+cy,fColour);
                       put_pixel32(bm,x+4,y+cy,fColour);
                   }else{
                       //PutPixelFast(bm,x+4,y+cy,bColour);
                       put_pixel32(bm,x+4,y+cy,bColour);
                   }
                                  
                   if(font_row & 0x04){
                       //PutPixelFast(bm,x+5,y+cy,fColour);
                       put_pixel32(bm,x+5,y+cy,fColour);
                   }else{
                       //PutPixelFast(bm,x+5,y+cy,bColour);
                       put_pixel32(bm,x+5,y+cy,bColour);
                   }
                   
                   if(font_row & 0x02){
                       //PutPixelFast(bm,x+6,y+cy,fColour);
                       put_pixel32(bm,x+6,y+cy,fColour);
                   }else{
                       //PutPixelFast(bm,x+6,y+cy,bColour);
                       put_pixel32(bm,x+6,y+cy,bColour);
                   }
                                  
                   if(font_row & 0x01){
                       //PutPixelFast(bm,x+7,y+cy,fColour);
                       put_pixel32(bm,x+7,y+cy,fColour);
                   }else{
                       //PutPixelFast(bm,x+7,y+cy,bColour);
                       put_pixel32(bm,x+7,y+cy,bColour);
                   }
               
           }

}

void RenderString(bitmap_t* bm,uint8_t* font,uint32_t x,uint32_t y,char* str,uint32_t fColour,uint32_t bColour){
    
    uint32_t index = 0;
    
    while (str[index] != 0) {
        
        if(x >= ( bm->width - 8) ){
            break;
        }
        
        RenderGlyph(bm, font,x,y,str[index],fColour,bColour);
            
        x += 8;
        index += 1;
    }
    
}


void DrawHorizontalLine(bitmap_t* bm, uint32_t x, uint32_t y,uint32_t length, uint32_t rgb){
  
    do{
        --length;
        put_pixel32(bm,x+length,y,rgb);
    }while(length>0);
    
}

void DrawVerticalLine(bitmap_t* bm, uint32_t x, uint32_t y,uint32_t length, uint32_t rgb){
    
    do{
        --length;
        put_pixel32(bm,x,y+length,rgb);
    }while(length>0);
    
    
}

//Quick and dirty asb() function for the line draw alogrythm, replace with a real function
//int32_t absG(int32_t value){
//
//    if ( value < 0){value = -value;}
//    return value;
//}

void DrawLine(bitmap_t* bm,uint32_t x0, uint32_t y0, uint32_t x1, uint32_t y1, uint32_t rgb){
    int32_t dx =  abs(x1-x0);
    int32_t sx = x0<x1 ? 1 : -1;
    int32_t dy = -abs(y1-y0);
    int32_t sy = y0<y1 ? 1 : -1;
    int32_t err = dx+dy;  /* error value e_xy */
    
    while(1){
        put_pixel32(bm,x0,y0,rgb);
        
        if (x0 == x1 && y0 == y1){break;}
        
        int32_t e2 = 2*err;
        
        if (e2 >= dy){
            err += dy;
            x0 += sx;
        }
        
        if (e2 <= dx){
            err += dx;
            y0 += sy;
        }
    }
}

void DrawCircle(bitmap_t* bm, uint32_t x0, uint32_t y0, uint32_t r, uint32_t rgb){
    
    int r1 = r * r;
    
    int x = x0;
    int y = y0 - r;

    int x2 = x0 + r;
    int y2 = y0;
    
    int x3 = x0 + r;
    int y3 = y0;
    
    int x4 = x0;
    int y4 = y0 + r;
    
    int x5 = x0;
    int y5 = y0 + r;
    
    int x6 = x0 - r;
    int y6 = y0;
    
    int x7 = x0 - r;
    int y7 = y0;
    
    int x8 = x0;
    int y8 = y0 - r;
    
    //calculate number of iterations required
    float g = r * 0.72;
    int end =(int)g;
    int lastD = 0;
    
    int i = 0;
    do{
        put_pixel32(bm,x,y,rgb);
        put_pixel32(bm,x2,y2,rgb);
        put_pixel32(bm,x3,y3,rgb);
        put_pixel32(bm,x4,y4,rgb);
        put_pixel32(bm,x5,y5,rgb);
        put_pixel32(bm,x6,y6,rgb);
        put_pixel32(bm,x7,y7,rgb);
        put_pixel32(bm,x8,y8,rgb);
        
        int x1 = x + 1;
        int y1 = y + 1;
    
        int xd0 = (x0 - (x1));
        int yd0 = (y0 - y);
        int d0 = (xd0 * xd0) + (yd0 * yd0);
    
        int xd1 = (x0 - (x1));
        int yd1 = (y0 - (y1));
        int d1 = (xd1 * xd1) + (yd1 * yd1);
    
        int td1 = abs(r1 - d0);
        int td2 = abs(r1 - d1);
        
        if( td1 < td2 ){
        
            x = x1;
            
            y2 = y2 - 1;
            
            y3 = y3 + 1;
            
            x4 = x1;
            
            x5 = x5 - 1;
            
            y6 = y6 + 1;
            
            y7 = y7 - 1;
            
            x8 = x8 - 1;
            

            if(lastD == td1){
                break;
            }
            lastD = td1;
        }else{
            x = x1;
            y = y1;
            
            x2 = x2 - 1;
            y2 = y2 - 1;
            
            x3 = x3 - 1;
            y3 = y3 + 1;
            
            x4 = x1;
            y4 = y4 - 1;
            
            x5 = x5 - 1;
            y5 = y5 - 1;
            
            x6 = x6 + 1;
            y6 = y6 + 1;
            
            x7 = x7 + 1;
            y7 = y7 - 1;
            
            x8 = x8 - 1;
            y8 = y8 + 1;
            

            if(lastD == td2){
                lastD = td2;
                break;
            }
            lastD = td2;
        }
        
        i++;
        
    }while(i <= end);
       
    
}


void FloodFill(bitmap_t* bm, uint32_t x, uint32_t y, uint32_t rgb){
        
    // Get Target Colour
    uint32_t targetColour = get_pixel32(bm,x,y);

    //Point2D map[bm->width*bm->height];
    Point2D* map = (Point2D*)executive->AllocMem((bm->width*bm->height)*sizeof(Point2D),0);
    
    if(map==NULL){
        return;
    }
    
    int mapIndex = 0;
    int mapPointer = -1;
    
    
    do{
        
        // if pixel is already drawn, then jump over it.
        if( get_pixel32(bm,x,y) == rgb){
            mapPointer++;
            x = map[mapPointer].x;
            y = map[mapPointer].y;
            continue;
        }
        
        // draw pixel
        put_pixel32(bm,x,y,rgb);

        
        //check pixel above
        if(get_pixel32(bm,x,y-1) == targetColour){
            map[mapIndex].x = x;
            map[mapIndex].y = y - 1;
            mapIndex++;
        }
        //check pixel below
        if(get_pixel32(bm,x,y+1) == targetColour){
            map[mapIndex].x = x;
            map[mapIndex].y = y + 1;
            mapIndex++;
        }
        
        //check pixel to the right
        if(get_pixel32(bm,x+1,y) == targetColour){
            map[mapIndex].x = x + 1;
            map[mapIndex].y = y;
            mapIndex++;
            
        }
        //check pixel to the left
        if(get_pixel32(bm,x-1,y) == targetColour){
            map[mapIndex].x = x - 1;
            map[mapIndex].y = y;
            mapIndex++;
        }
        mapPointer++;
        
        x = map[mapPointer].x;
        y = map[mapPointer].y;
        
    }while(mapPointer < mapIndex);
        
    executive->FreeMem(map);
    
    return;

}

void DrawVectorImage(bitmap_t* bm, uint32_t x, uint32_t y, uint8_t* data){
    
    uint32_t colour[4];
    
    colour[0] = colour32(255,255,255,255);
    colour[1] = colour32(0,0,0,255);
    colour[2] = colour32(123,134,205,255);
    colour[3] = colour32(191,191,191,255);
    
    uint32_t currentColour = colour[0];
    
    uint8_t a;
    uint8_t b;
    
    uint32_t tx = 0;
    uint32_t ty = 0;
    
    int loop = 1;
    
    do{
        a = *data++;
        b = *data++;
        
        switch(a){
            case 0xFF: //Move or Quit
                
                if(b==0xFF){
                    loop = 0;
                }else{
                    tx = (uint32_t) *data++;
                    ty = (uint32_t) *data++;
                    currentColour = colour[b];
                }
                
                break;
                
            case 0xFE: //Flood Fill
                
                tx = (uint32_t) *data++;
                ty = (uint32_t) *data++;
                FloodFill(bm, x + tx, y + ty,colour[b]);
                
                break;
                
            default: //Draw line
                
                DrawLine(bm, x + tx, y + ty, x + a, y + b,currentColour);
                
                tx = a;
                ty = b;
                
                break;
                
        }
        
    }while(loop);
    
}

void DrawSprite(sprite_t* sprite, uint32_t sx, uint32_t sy){
    uint32_t index = 0;
    uint32_t* p = graphics.frameBuffer.buffer;
    uint32_t* b = sprite->backingStore;
    
    //save background
    uint32_t w = sprite->width;
    uint32_t h = sprite->height;
    
    if(w >= graphics.frameBuffer.width){w = graphics.frameBuffer.width - sx;}
    if(h >= graphics.frameBuffer.height){h = graphics.frameBuffer.height - sy;}
    
    for(uint32_t y = 0; y<h;++y){
        
        movsd(&b[index],&p[((sy+y)*graphics.frameBuffer.width)+sx],w);
        
        index += sprite->width;
        
    }
   /*
    //draw pointer
    w = sx + sprite->width;
    h = sy + sprite->height;
    
    if(w >= graphics.frameBuffer.width){w = graphics.frameBuffer.width;}
    if(h >= graphics.frameBuffer.height){h = graphics.frameBuffer.height;}
    
    index = 0;
    for(uint32_t y = sy;y < (uint32_t)mouseY+22;++y){
        for(uint32_t x = sx; x < (uint32_t)mouseX+22;++x){
            
            if(x<graphics.frameBuffer.width && y<graphics.frameBuffer.height){
                if(intuition.mouseImage[index] == 1){
                    p[(y * graphics.frameBuffer.width) + x] =  0;   //black
                }
                
                if(intuition.mouseImage[index] == 2){
                    p[(y * graphics.frameBuffer.width) + x] =  intuition.grey;
                }
                
                if(intuition.mouseImage[index] == 3){
                    p[(y * graphics.frameBuffer.width) + x] =  intuition.red;
                }
                
                if(intuition.mouseImage[index] == 4){
                    p[(y * graphics.frameBuffer.width) + x] =  intuition.white;
                }
                
            }
            index += 1;
        }
    }
    */
}

void ClearSprite(sprite_t* sprite){
    
    volatile uint32_t w = sprite->width;

    w = w + 1;
    
}



palette_t* CreatePalette(uint32_t numberOfColours){
    palette_t* pal = (palette_t*)executive->Alloc( ( sizeof(colour_t) * numberOfColours ) + sizeof(palette_t));
    pal->node.nodeType = NODE_PALETTE;
    pal->count = numberOfColours;
    void* data = pal + 1;
    pal->colour = (colour_t*)data;
    return pal;
}
void SetColour(palette_t* palette,uint32_t index, uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha){
    palette->colour[index].red = red;
    palette->colour[index].green = green;
    palette->colour[index].blue = blue;
    palette->colour[index].alpha = alpha;
}
colour_t GetColour(palette_t* palette, uint32_t index){
    return palette->colour[index];
}



void InitGraphics(library_t* library){
    //perhaps check for a proper gfx card here?
    library->node.nodeType = NODE_LIBRARY;
    library->node.name  = "graphics.library";
}

library_t* OpenGraphics(library_t* library){
    
    library->openCount += 1;
    return library;
}

void LoadGraphicsLibrary(multiboot_info_t* mbd){
    //Graphics version
    
    //graphics.library.node.name  = "graphics.library"; // now done in the Init function
    graphics.library.Init       = InitGraphics;
    graphics.library.Open       = OpenGraphics;
    
    if(mbd->framebuffer_addr != 0){
        
        graphics.frameBuffer.width = mbd->framebuffer_width;
        graphics.frameBuffer.height= mbd->framebuffer_height;
        
        void_ptr pointer = (void_ptr)mbd->framebuffer_addr;
        graphics.frameBuffer.buffer =(void*) pointer;
        
        graphics.frameBuffer.bpp = mbd->framebuffer_bpp;
        
        if(mbd->framebuffer_bpp == 16){
            /*
            blue = colour16(0x00,0x55,0xAA,0xFF);
            white = colour16(0xFF,0xFF,0xFF,0xFF);
            orange = colour16(0xFF,0x88,00,0xFF);
            black = colour16(0x00,0x00,0x22,0xFF);
        
            grey = colour16(171,187,205,0xFF);
            red = colour16(217,46,31,0xFF);
        
            colour = colour16;
             */
        }
        
        if(mbd->framebuffer_bpp == 32){
            
            graphics.NewBitmap      = NewBitmap;
            graphics.FreeBitmap     = FreeBitmap;
            graphics.Colour         = colour32;
            graphics.GetPixel       = get_pixel32;
            graphics.PutPixel       = put_pixel32;
            graphics.RenderGlyph    = RenderGlyph;
            graphics.RenderString   = RenderString;
            graphics.DrawRect       = DrawRect;
            graphics.BlitBitmap     = BlitBitmap;
            graphics.DrawLine       = DrawLine;
            graphics.DrawCircle     = DrawCircle;
            graphics.FloodFill      = FloodFill;
            graphics.DrawVectorImage= DrawVectorImage;
            graphics.BlitRect       = BlitRect;
            graphics.ClearBitmap    = ClearBitmap;
            
            graphics.CreatePalette  = CreatePalette;
            graphics.SetColour      = SetColour;
            graphics.GetColour      = GetColour;
        }
        
        
    }
    

}
