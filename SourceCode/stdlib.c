//
//  stdlib.c
//
//
//  Created by Matt Parsons on 08/09/2021.
//
//

#include "stdlib.h"
#include "string.h"
#include "math.h"
#include "SystemLog.h"

int atoi(char* str){
    
    int places = strlen(str) - 1;
    
    int retVal = 0;
    int i = 0;
    
    while(str[i]>47 && str[i] < 58){
     
        retVal += (((int)str[i])-48) * pow(10,places - i);
        
        i++;
    }
    return retVal;
}
