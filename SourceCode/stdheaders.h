//
//  stdheaders.h
//  
//
//  Created by Matt Parsons on 22/09/2020.
//
//

#ifndef stdheaders_h
#define stdheaders_h

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

//#warning void_ptr will need to be redefined on 64bit system, this is used to give sizes to pointers
typedef uint32_t void_ptr; 

//get the address of the executive, This should probably be a macro
//void* SysBase(void){
//    uint32_t* temp = (uint32_t*)0x100000;
//    return (void*)*temp;
//}

#endif
