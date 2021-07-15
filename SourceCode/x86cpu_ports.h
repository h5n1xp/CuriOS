//
//  x86 CPU Port access.h
//  
//
//  Created by Matt Parsons on 22/09/2020.
//
//

#ifndef x86cpu_ports_h
#define x86cpu_ports_h

#include "stdheaders.h"

//Hardware Access functions
void outb(uint16_t port, uint8_t value);

uint8_t inb(uint16_t port);

uint16_t inw(uint16_t port);

#endif
