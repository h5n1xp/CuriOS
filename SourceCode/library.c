//
//  library.c
//
//  Created by Matt Parsons on 05/11/2020.
//  Copyright © 2020 Matt Parsons. All rights reserved.
//

#include "library.h"

library_t LibExample;

void LibInit(library_t* library){
    //Called by AddLibrary() function in the Executive
    library->baseLibrary = library;
    library->node.nodeType = NODE_LIBRARY;
    library->node.name  = "example.library";
}

library_t* LibOpen(library_t* library){
    //Called by OpenLibrary() function in the Executive
    library->openCount += 1;
    return library;
}

void LibClose(library_t* library){
    //Called by CloseLibrary() function in the Executive
    
    //If this is an instance then you need to clean up manually
    
    library_t* lib = library->baseLibrary;
    lib->openCount += 1;
}

void LibLoad(){
    
    //build the library function table
    //this should be called by the ELF loader at load time.
    
    LibExample.Init     = LibInit;
    LibExample.Open     = LibOpen;
    LibExample.Close    = LibClose;
    
    
}
