//
//  descriptor_tables.c
//  
//
//  Created by Matt Parsons on 22/09/2020.
//
//

#include "descriptor_tables.h"
#include "x86cpu_ports.h"
#include "string.h"

// GDT
// Internal function prototypes.
static void init_gdt();
static void gdt_set_gate(int32_t,uint32_t,uint32_t,uint8_t,uint8_t);
static void init_idt();
static void idt_set_gate(uint8_t,uint32_t,uint16_t,uint8_t);


gdt_entry_t gdt_entries[6];
gdt_ptr_t   gdt_ptr;
idt_entry_t idt_entries[256];
idt_ptr_t   idt_ptr;
extern tss_entry_t tss_entry;

extern uint32_t stack_top;

// Initialisation routine - zeroes all the interrupt service routines,
// initialises the GDT and IDT.
void init_descriptor_tables(){
    // Initialise the global descriptor table.
    init_idt();
    init_gdt();
}

// Initialise our task state segment structure.
static void write_tss(int32_t num, int16_t ss0, uint32_t esp0){
    // Firstly, let's compute the base and limit of our entry into the GDT.
    uint32_t base = (uint32_t) &tss_entry;
    uint32_t limit = base + sizeof(tss_entry);
    
    // Now, add our TSS descriptor's address to the GDT.
    gdt_set_gate(num, base, limit, 0x89, 0x0);

    // Ensure the descriptor is initially zero.
    memset(&tss_entry, 0, sizeof(tss_entry));

    tss_entry.ss0  = ss0;  // Set the kernel stack segment.
    tss_entry.esp0 = esp0; // Set the kernel stack pointer.
    
    // Here we set the cs, ss, ds, es, fs and gs entries in the TSS. These specify what
    // segments should be loaded when the processor switches to kernel mode. Therefore
    // they are just our normal kernel code/data segments - 0x08 and 0x10 respectively,
    // but with the last two bits set, making 0x0b and 0x13. The setting of these bits
    // sets the RPL (requested privilege level) to 3, meaning that this TSS can be used
    // to switch to kernel mode from ring 3.
    tss_entry.cs   = 0x1b;
    tss_entry.ss = tss_entry.ds = tss_entry.es = tss_entry.fs = tss_entry.gs = 0x23;
}


extern void flush_tss();

static void init_gdt(){
    gdt_ptr.limit = (sizeof(gdt_entry_t) * 6) - 1;
    gdt_ptr.base  = (uint32_t)&gdt_entries;
    
    gdt_set_gate(0, 0, 0, 0, 0);                // Null segment
    gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF); // Code segment
    gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF); // Data segment
    gdt_set_gate(3, 0, 0xFFFFFFFF, 0xFA, 0xCF); // User mode code segment
    gdt_set_gate(4, 0, 0xFFFFFFFF, 0xF2, 0xCF); // User mode data segment
    write_tss(5, 0x10, stack_top);
    
    uint32_t ptr = (uint32_t)&gdt_ptr;
    
    asm volatile("movl %0,%%eax    \n\t"
                 "lgdt (%%eax)     \n\t"
                 
                 
                 "movl %%cr0,%%eax \n\t"
                 "or    $0x1, %%al \n\t"       // set PE (Protection Enable) bit in CR0 (Control Register 0)
                 "movl %%eax, %%cr0\n\t"
                 
                 "movw $0x10, %%ax \n\t"
                 "movw  %%ax, %%ds \n\t"
                 "movw  %%ax, %%es \n\t"
                 "movw  %%ax, %%fs \n\t"
                 "movw  %%ax, %%gs \n\t"
                 "movw  %%ax, %%ss \n\t"
                 "ljmp $0x08,$boing\n\t"
                 "boing:" : : "r" (ptr) : "eax");
    
    //Flush the TSS
    asm volatile("movw $0x28, %%ax \n\t"
                 "ltr  %%ax"::: "ax");
    
}

// Set the value of one GDT entry.
static void gdt_set_gate(int32_t num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran){
    gdt_entries[num].base_low    = (base & 0xFFFF);
    gdt_entries[num].base_middle = (base >> 16) & 0xFF;
    gdt_entries[num].base_high   = (base >> 24) & 0xFF;
    
    gdt_entries[num].limit_low   = (limit & 0xFFFF);
    gdt_entries[num].granularity = (limit >> 16) & 0x0F;
    
    gdt_entries[num].granularity |= gran & 0xF0;
    gdt_entries[num].access      = access;
}


void set_kernel_stack(uint32_t stack){
   tss_entry.esp0 = stack;
}


//IDT - Interrupts


#define  IRQ0 32 //System Timer
#define  IRQ1 33
#define  IRQ2 34
#define  IRQ3 35
#define  IRQ4 36
#define  IRQ5 37
#define  IRQ6 38
#define  IRQ7 39
#define  IRQ8 40
#define  IRQ9 41
#define IRQ10 42
#define IRQ11 43
#define IRQ12 44
#define IRQ13 45
#define IRQ14 46
#define IRQ15 47


static void init_idt(){
    idt_ptr.limit = sizeof(idt_entry_t) * 256 -1;
    idt_ptr.base  = (uint32_t)&idt_entries;
    
    memset(&idt_entries, 0, sizeof(idt_entry_t)*256);
    
    // Remap the irq table.
    
    outb(0x20, 0x11);
    outb(0xA0, 0x11);
    
    outb(0x21, 0x20);
    outb(0xA1, 0x28); // James Molloy suggested 0x28
    
    outb(0x21, 0x04);
    outb(0xA1, 0x02);
    
    outb(0x21, 0x01);
    outb(0xA1, 0x01);
    
    outb(0x21, 0x0);
    outb(0xA1, 0x0);
    
    idt_set_gate( 0, (uint32_t)isr0  , 0x08, 0x8E);
    idt_set_gate( 1, (uint32_t)isr1  , 0x08, 0x8E);
    idt_set_gate( 2, (uint32_t)isr2  , 0x08, 0x8E);
    idt_set_gate( 3, (uint32_t)isr3  , 0x08, 0x8E);
    idt_set_gate( 4, (uint32_t)isr4  , 0x08, 0x8E);
    idt_set_gate( 5, (uint32_t)isr5  , 0x08, 0x8E);
    idt_set_gate( 6, (uint32_t)isr6  , 0x08, 0x8E);
    idt_set_gate( 7, (uint32_t)isr7  , 0x08, 0x8E);
    idt_set_gate( 8, (uint32_t)isr8  , 0x08, 0x8E);
    idt_set_gate( 9, (uint32_t)isr9  , 0x08, 0x8E);
    idt_set_gate(10, (uint32_t)isr10 , 0x08, 0x8E);
    idt_set_gate(11, (uint32_t)isr11 , 0x08, 0x8E);
    idt_set_gate(12, (uint32_t)isr12 , 0x08, 0x8E);
    idt_set_gate(13, (uint32_t)isr13 , 0x08, 0x8E);
    idt_set_gate(14, (uint32_t)isr14 , 0x08, 0x8E);
    idt_set_gate(15, (uint32_t)isr15 , 0x08, 0x8E);
    idt_set_gate(16, (uint32_t)isr16 , 0x08, 0x8E);
    idt_set_gate(17, (uint32_t)isr17 , 0x08, 0x8E);
    idt_set_gate(18, (uint32_t)isr18 , 0x08, 0x8E);
    idt_set_gate(19, (uint32_t)isr19 , 0x08, 0x8E);
    idt_set_gate(20, (uint32_t)isr20 , 0x08, 0x8E);
    idt_set_gate(21, (uint32_t)isr21 , 0x08, 0x8E);
    idt_set_gate(22, (uint32_t)isr22 , 0x08, 0x8E);
    idt_set_gate(23, (uint32_t)isr23 , 0x08, 0x8E);
    idt_set_gate(24, (uint32_t)isr24 , 0x08, 0x8E);
    idt_set_gate(25, (uint32_t)isr25 , 0x08, 0x8E);
    idt_set_gate(26, (uint32_t)isr26 , 0x08, 0x8E);
    idt_set_gate(27, (uint32_t)isr27 , 0x08, 0x8E);
    idt_set_gate(28, (uint32_t)isr28 , 0x08, 0x8E);
    idt_set_gate(29, (uint32_t)isr29 , 0x08, 0x8E);
    idt_set_gate(30, (uint32_t)isr30 , 0x08, 0x8E);
    idt_set_gate(31, (uint32_t)isr31 , 0x08, 0x8E);
    
    idt_set_gate(32, (uint32_t)irq0  , 0x08, 0x8E);
    idt_set_gate(33, (uint32_t)irq1  , 0x08, 0x8E);
    idt_set_gate(34, (uint32_t)irq2  , 0x08, 0x8E);
    idt_set_gate(35, (uint32_t)irq3  , 0x08, 0x8E);
    idt_set_gate(36, (uint32_t)irq4  , 0x08, 0x8E);
    idt_set_gate(37, (uint32_t)irq5  , 0x08, 0x8E);
    idt_set_gate(38, (uint32_t)irq6  , 0x08, 0x8E);
    idt_set_gate(39, (uint32_t)irq7  , 0x08, 0x8E);
    idt_set_gate(40, (uint32_t)irq8  , 0x08, 0x8E);
    idt_set_gate(41, (uint32_t)irq9  , 0x08, 0x8E);
    idt_set_gate(42, (uint32_t)irq10 , 0x08, 0x8E);
    idt_set_gate(43, (uint32_t)irq11 , 0x08, 0x8E);
    idt_set_gate(44, (uint32_t)irq12 , 0x08, 0x8E);
    idt_set_gate(45, (uint32_t)irq13 , 0x08, 0x8E);
    idt_set_gate(46, (uint32_t)irq14 , 0x08, 0x8E);
    idt_set_gate(47, (uint32_t)irq15 , 0x08, 0x8E);
    
    idt_set_gate(48, (uint32_t)isr48 , 0x08, 0x8E);
    idt_set_gate(49, (uint32_t)isr49 , 0x08, 0x8E);
    idt_set_gate(50, (uint32_t)isr50 , 0x08, 0x8E);
    
    uint32_t ptr = (uint32_t) &idt_ptr;
    
    asm volatile("movl %0,%%eax \n\t"
                 "lidt (%%eax)" : : "r" (ptr) : "eax");
    
}

static void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags){
    idt_entries[num].base_lo = base & 0xFFFF;
    idt_entries[num].base_hi = (base >> 16) & 0xFFFF;
    
    idt_entries[num].sel     = sel;
    idt_entries[num].always0 = 0;
    // We must uncomment the OR below when we get to using user-mode.
    // It sets the interrupt gate's privilege level to 3.
    idt_entries[num].flags   = flags | 0x60;
}

