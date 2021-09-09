//
//  string.h
//  
//
//  Created by Matt Parsons on 22/09/2020.
//
//

#ifndef string_h
#define string_h

#include "stdheaders.h"

//Ctype functions
void tolower(char* str);

void toupper(char* str);


char* strcpy(char* destination, char* source);

int strlen(char* str);

int strcmp(char* str1, char* str2);

int strncmp(char* str1, char* str2, size_t n);

void memset(void* addr, int value, size_t len);

void memcpy(void* desti, void* srci, uint64_t n);

#endif
