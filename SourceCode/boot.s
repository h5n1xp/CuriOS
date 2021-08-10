/* Declare constants for the multiboot header. */
.set ALIGN,    1<<0                      /* align loaded modules on page boundaries */
.set MEMINFO,  1<<1                      /* provide memory map */
.set VBEINFO,  1<<2
.set FLAGS,    ALIGN | MEMINFO | VBEINFO
.set MAGIC,    0x1BADB002                /* 'magic number' lets bootloader find the header */
.set CHECKSUM, -(MAGIC + FLAGS)          /* checksum of above, to prove we are multiboot */

.set header_addr, 0  //Not needed as I use ELF
.set load_addr, 0     //Not needed, as above
.set load_end_addr, 0//as above
.set bss_end_addr, 0  // as above
.set entry_addr, 0 // as above

.set mode_type, 0   //Linear framebuffer please
.set width, 1024  //1024 //1280 /1280 /640 /1440
.set height, 768    //768 //800 /720 /480 /900
//.set width, 1280  //1024 //1280 /1280 /640 /1440
//.set height, 720    //768 //800 /720 /480 /900
//.set width, 1920    //1024 //1280 /1280 /640 /1440
//.set height, 1200    //768 //800 /720 /480 /900

.set depth, 32

/* 
Declare a multiboot header that marks the program as a kernel. These are magic
values that are documented in the multiboot standard. The bootloader will
search for this signature in the first 8 KiB of the kernel file, aligned at a
32-bit boundary. The signature is in its own section so the header can be
forced to be within the first 8 KiB of the kernel file.
*/
.section .multiboot
.align 4
.long MAGIC
.long FLAGS
.long CHECKSUM
.long header_addr  //Not needed as I use ELF
.long load_addr    //Not needed, as above
.long load_end_addr//as above
.long bss_end_addr  // as above
.long entry_addr // as above
.long mode_type
.long width
.long height
.long depth

 
/*
The multiboot standard does not define the value of the stack pointer register
(esp) and it is up to the kernel to provide a stack. This allocates room for a
small stack by creating a symbol at the bottom of it, then allocating 16384
bytes for it, and finally creating a symbol at the top. The stack grows
downwards on x86. The stack is in its own section so it can be marked nobits,
which means the kernel file is smaller because it does not contain an
uninitialized stack. The stack on x86 must be 16-byte aligned according to the
System V ABI standard and de-facto extensions. The compiler will assume the
stack is properly aligned and failure to align the stack will result in
undefined behavior.
*/

.section .bss
.align 16


.global tss_entry
tss_entry:
.skip 4
tss_esp0:
.skip 8
tss_esp1:
.skip 104
stack_bottom:
.skip 65536 # 64 KiB boot stack
.global stack_top
stack_top:
/*
.skip 16384 # 16 KiB
.global supervisor_stack_top
supervisor_stack_top:
*/

/*
The linker script specifies _start as the entry point to the kernel and the
bootloader will jump to this position once the kernel has been loaded. It
doesn't make sense to return from this function as the bootloader is gone.
*/
.section .text
.global _start
.type _start, @function
_start:
	/*
	The bootloader has loaded us into 32-bit protected mode on a x86
	machine. Interrupts are disabled. Paging is disabled. The processor
	state is as defined in the multiboot standard. The kernel has full
	control of the CPU. The kernel can only make use of hardware features
	and any code it provides as part of itself. There's no printf
	function, unless the kernel provides its own <stdio.h> header and a
	printf implementation. There are no security restrictions, no
	safeguards, no debugging mechanisms, only what the kernel provides
	itself. It has absolute and complete power over the
	machine.
	*/
 
	/*
	To set up a stack, we set the esp register to point to the top of the
	stack (as it grows downwards on x86 systems). This is necessarily done
	in assembly as languages such as C cannot function without a stack.
	*/
	mov $stack_top, %esp

 
	/*
	This is a good place to initialize crucial processor state before the
	high-level kernel is entered. It's best to minimize the early
	environment where crucial features are offline. Note that the
	processor is not fully initialized yet: Features such as floating
	point instructions and instruction set extensions are not initialized
	yet. The GDT should be loaded here. Paging should be enabled here.
	C++ features such as global constructors and exceptions will require
	runtime support to work as well.
	*/
 
	/*
	Enter the high-level kernel. The ABI requires the stack is 16-byte
	aligned at the time of the call instruction (which afterwards pushes
	the return pointer of size 4 bytes). The stack was originally 16-byte
	aligned above and we've pushed a multiple of 16 bytes to the
	stack since (pushed 0 bytes so far), so the alignment has thus been
	preserved and the call is well defined.
	*/
    push %eax       // save a pointer to the multiboot header
    push %ebx       // and the magic number to confirm this is multiboot

	call kernel_main
 
	/*
	If the system has nothing more to do, put the computer into an
	infinite loop. To do that:
	1) Disable interrupts with cli (clear interrupt enable in eflags).
	   They are already disabled by the bootloader, so this is not needed.
	   Mind that you might later enable interrupts and return from
	   kernel_main (which is sort of nonsensical to do).
	2) Wait for the next interrupt to arrive with hlt (halt instruction).
	   Since they are disabled, this will lock up the computer.
	3) Jump to the hlt instruction if it ever wakes up due to a
	   non-maskable interrupt occurring or due to system management mode.
	*/
	cli
1:	hlt
	jmp 1b
 
/*
Set the size of the _start symbol to the current location '.' minus its start.
This is useful when debugging or when you implement call tracing.
*/
.size _start, . - _start

// Useful ASM functions
.global flush_tss
flush_tss:
    movw $0x2B, %ax
    ltr %ax
    ret

// Interrupt handlers / x86 Traps
.global isr0
isr0:
    //cli

    push $0
    push $0
    jmp isr_common_stub

.global isr1
isr1:
    //cli

    push $0
    push $1
    jmp isr_common_stub

.global isr2
isr2:
    //cli

    push $0
    push $2
    jmp isr_common_stub

.global isr3
isr3:
    //cli

    push $0
    push $3
    jmp isr_common_stub

.global isr4
isr4:
    //cli

    push $0
    push $4
    jmp isr_common_stub

.global isr5
isr5:
    //cli

    push $0
    push $5
    jmp isr_common_stub

.global isr6
isr6:
    //cli

    push $0
    push $6
    jmp isr_common_stub


.global isr7
isr7:
    //cli

    push $0
    push $7
    jmp isr_common_stub

.global isr8    //pushes its own error code on to stack
isr8:
    //cli

    push $8
    jmp isr_common_stub

.global isr9
isr9:
    //cli

    push $0
    push $9
    jmp isr_common_stub

.global isr10   //pushes its own error code on to stack
isr10:
    //cli

    push $10
    jmp isr_common_stub

.global isr11 //pushes its own error code on to stack
isr11:
    //cli

    push $11
    jmp isr_common_stub

.global isr12 //pushes its own error code on to stack
isr12:
    //cli

    push $12
    jmp isr_common_stub

.global isr13 //pushes its own error code on to stack
isr13:
    //cli

    push $13
    jmp isr_common_stub

.global isr14 //pushes its own error code on to stack
isr14:
    //cli

    push $14
    jmp isr_common_stub

.global isr15
isr15:
    //cli

    push $0
    push $15
    jmp isr_common_stub

.global isr16
isr16:
    //cli

    push $0
    push $16
    jmp isr_common_stub

.global isr17
isr17:
    //cli

    push $0
    push $17
    jmp isr_common_stub

.global isr18
isr18:
    //cli

    push $0
    push $18
    jmp isr_common_stub

.global isr19
isr19:
    //cli

    push $0
    push $19
    jmp isr_common_stub

.global isr20
isr20:
    //cli

    push $0
    push $20
    jmp isr_common_stub

.global isr21
isr21:
    //cli

    push $0
    push $21
    jmp isr_common_stub

.global isr22
isr22:
    //cli

    push $0
    push $22
    jmp isr_common_stub

.global isr23
isr23:
    //cli

    push $0
    push $23
    jmp isr_common_stub

.global isr24
isr24:
    //cli

    push $0
    push $24
    jmp isr_common_stub

.global isr25
isr25:
    //cli

    push $0
    push $25
    jmp isr_common_stub

.global isr26
isr26:
    //cli

    push $0
    push $26
    jmp isr_common_stub

.global isr27
isr27:
    //cli

    push $0
    push $27
    jmp isr_common_stub

.global isr28
isr28:
    //cli

    push $0
    push $28
    jmp isr_common_stub

.global isr29
isr29:
    //cli

    push $0
    push $29
    jmp isr_common_stub

.global isr30
isr30:
    ///cli

    push $0
    push $30
    jmp isr_common_stub

.global isr31
isr31:
    //cli

    push $0
    push $31
    jmp isr_common_stub

//software interupts ******************************
.global isr48
isr48:
    //cli

    push $0
    push $48
    jmp isr_common_stub

.global isr49
isr49:
    //cli

    push $0
    push $49
    jmp isr_common_stub

.global isr50
isr50:
    //cli

    push $0
    push $50
    jmp isr_common_stub


isr_common_stub:
    pusha
    movw %ds,%ax
    push %eax
    push %esp       //Save the curent stack poiner as this is where we want the interrupt to continue from after it has processed
                    //A contect switch may have happened so after processing, this might point to a new context.
    movw $0x10,%ax
    movw %ax,%ds
    movw %ax,%es
    movw %ax,%fs
    movw %ax,%gs

    cld
    call isr_handler

    pop %esp
    pop %ebx
    movw %bx,%ds
    movw %bx,%es
    movw %bx,%fs
    movw %bx,%gs

    popa
    add $8, %esp

    //sti
    iret


// Interrupt handlers
.global irq0
irq0:
//cli

    push $0
    push $32
    jmp irq_common_stub

.global irq1
irq1:
//cli

    push $0
    push $33
    jmp irq_common_stub

.global irq2
irq2:
   // cli

    push $0
    push $34
    jmp irq_common_stub

.global irq3
irq3:
   // cli

    push $0
    push $35
    jmp irq_common_stub

.global irq4
irq4:
   // cli

    push $0
    push $36
    jmp irq_common_stub

.global irq5
irq5:
   // cli

    push $0
    push $37
    jmp irq_common_stub

.global irq6
irq6:
   // cli

    push $0
    push $38
    jmp irq_common_stub


.global irq7
irq7:
   // cli

    push $0
    push $39
    jmp irq_common_stub

.global irq8  
irq8:
    //cli

    push $0
    push $40
    jmp irq_common_stub

.global irq9
irq9:
  //  cli

    push $0
    push $41
    jmp irq_common_stub

.global irq10
irq10:
  //  cli

    push $0
    push $42
    jmp irq_common_stub

.global irq11
irq11:
   // cli

    push $0
    push $43
    jmp irq_common_stub

.global irq12
irq12:
//cli

    push $0
    push $44
    jmp irq_common_stub

.global irq13
irq13:
 //   cli

    push $0
    push $45
    jmp irq_common_stub

.global irq14
irq14:
  //  cli

    push $0
    push $46
    jmp irq_common_stub

.global irq15
irq15:
   // cli

    push $0
    push $47
    jmp irq_common_stub


irq_common_stub:
    pusha
    movw %ds,%ax
    push %eax
    push %esp

    movw $0x10,%ax
    movw %ax,%ds
    movw %ax,%es
    movw %ax,%fs
    movw %ax,%gs
    
    cld
    call irq_handler

    pop %esp
    pop %ebx
    movw %bx,%ds
    movw %bx,%es
    movw %bx,%fs
    movw %bx,%gs

    popa
    add $8, %esp

    //sti
    iret
