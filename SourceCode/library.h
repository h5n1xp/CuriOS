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

typedef struct library_t library_t;

struct library_t{
    node_t node;
    int64_t openCount;
    uint64_t version;
    void (*Init)(library_t* lib);
    void (*Open)(library_t* lib);
    void (*Close)(library_t* lib);
    void (*Expunge)(library_t* lib);
};

#endif /* library_h */
