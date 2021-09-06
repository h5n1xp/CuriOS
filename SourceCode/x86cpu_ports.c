//
//  x86 CPU Port access.c
//  
//
//  Created by Matt Parsons on 22/09/2020.
//
//

#include "x86cpu_ports.h"

//Hardware Access functions
inline void outb(uint16_t port, uint8_t value){
    asm volatile ("outb %1, %0" : : "dN" (port), "a" (value));
}

inline void outw(uint16_t port, uint16_t value){
    asm volatile ("outw %1, %0" : : "dN" (port), "a" (value));
}

inline void outl(uint16_t port, uint32_t value){
    asm volatile ("outl %1, %0" : : "dN" (port), "a" (value));
}


inline uint8_t inb(uint16_t port){
    uint8_t ret;
    asm volatile("inb %1, %0" : "=a" (ret) : "dN" (port));
    return ret;
}

inline uint16_t inw(uint16_t port){
    uint16_t ret;
    asm volatile ("inw %1, %0" : "=a" (ret) : "dN" (port));
    return ret;
}

inline uint32_t inl(uint16_t port){
    uint32_t ret;
    asm volatile ("inl %1, %0" : "=a" (ret) : "dN" (port));
    return ret;
}
