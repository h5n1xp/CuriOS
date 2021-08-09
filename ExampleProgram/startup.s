/* Declare constants for the header. */
.set MAGIC,    0x80DECADE               /* 'magic number' lets loader find the header */
.set FLAGS,    0xF3030000               /* no real flags yet */
.set CHECKSUM, -(MAGIC + FLAGS)         /* checksum of above, to prove we are genuine */

//.global executive    /* The ELF loader will write a pointer to my global function table here*/

.section .start
.align 4
magic:
.long MAGIC
//flags:
//.long FLAGS

.section .text
.global _start
.type _start, @function
_start:

	call main
 
    //leave
    ret
