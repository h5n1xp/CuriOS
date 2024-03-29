//
//  library.h
//
//  Created by Matt Parsons on 02/11/2020.
//  Copyright © 2020 Matt Parsons. All rights reserved.
//

#ifndef library_h
#define library_h

#include "stdheaders.h"
#include "list.h"

#define LIBRARY_INIT_FAIL       0
#define LIBRARY_INIT_SUCCESS    1

typedef struct library_t library_t;

struct library_t{
    node_t      node;

    //Audit info
    int64_t     openCount;
    uint64_t    version;
    library_t*  baseLibrary;               //this is the "base class" of the library rather than an instance return by Open()
    
    //Function area
    uint32_t    (*Init)(library_t* lib);   //this is called by the AddLibrary() function and sets up the library in memory... an Init() can fail and return an error code
    library_t*  (*Open)(library_t* lib);   //this is called by OpenLibrary() function, and returns an instance of the library
    void        (*Close)(library_t* lib);
    void        (*Expunge)(library_t* lib);
    void        (*Reserved)(void);
    

    
};

extern library_t LibExample;

#endif /* library_h */
