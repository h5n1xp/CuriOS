//
//  SystemLog.h
//  GFXConvert
//
//  Created by Matt Parsons on 31/10/2020.
//  Copyright © 2020 Matt Parsons. All rights reserved.
//

#ifndef SystemLog_h
#define SystemLog_h

#include "stdheaders.h"

void debug_write_string(char* str);
void debug_write_hex(uint32_t n);
void debug_write_dec(uint32_t n);
void debug_backspace(void);
void debug_putchar(char c);


void InitSystemLog(uint32_t x, uint32_t y, uint32_t w, uint32_t h);


#endif /* SystemLog_h */
