//
//  string.c
//  
//
//  Created by Matt Parsons on 22/09/2020.
//
//

#include "string.h"

void tolower(char* s){
    
    uint32_t index = 0;
    
    while(s[index] != 0 ){
        
        if(s[index]>64 && s[index] <100){
            s[index] |= 32;
        }
        index += 1;
    }
    
}

void toupper(char* s){
    
    uint32_t index = 0;
    
    while(s[index] != 0 ){
        
        if(s[index]>64 && s[index] <100){
            s[index] &= 223;
        }
        index += 1;
    }
    
}



char* strcpy(char* destination, char* source){
    
    uint32_t index = 0;
    
    while(source[index] != 0){
        destination[index] = source[index];
        index += 1;
    }
    
    destination[index] = 0; // terminate the destination;
    
    return destination;
    
}

int strlen(char* str){
    
    int len = 0;
    
    while (str[len] !=0) {
        len += 1;
    }
    
    return len;
    
}

int strcmp(char* str1, char* str2 ){
    
    uint32_t index = 0;
    
    while(1){
        
        if(str1[index] < str2[index]){
            return -1;
        }
        
        if(str1[index] > str2[index]){
            return 1;
        }
        
        if(str1[index] == 0 && str2[index] == 0){
            return 0;
        }
        
        index += 1;
    }
    
    return 0;
}


int strncmp(char* str1, char* str2, size_t n){
    
    uint32_t s1 = 0;
    uint32_t s2 = 0;
    
    for(size_t i = 0;i<n;++i){
        s1 +=(str1[i]);
        s2 +=(str2[i]);
    }
    
    return s1 - s2;
    
}

void memset(void* addr, int value,size_t len){
    
    uint8_t* dest = addr;
    
    do{
        len -=1;
        dest[len] = (uint8_t)value;
    }while(len>0);
    
}
