//
//  input.h
//  PrepStep8
//
//  Created by Matt Parsons on 03/11/2020.
//  Copyright © 2020 Matt Parsons. All rights reserved.
//

#ifndef input_h
#define input_h

#include "stdheaders.h"
#include "intuition.h"
#include "task.h"

typedef struct{
    window_t* screenTitle;
    window_t* focused;
    task_t* inputTask;
    int8_t rawMouse[4];
    uint8_t keyboardBuffer[32];
    uint32_t keyboardBufferPosition;
}inputStruct_t;


extern inputStruct_t inputStruct;


void InputTaskEntry();


#endif /* input_h */
