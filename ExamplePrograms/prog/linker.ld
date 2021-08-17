/* start execution at the symbol designated as the entry point. */
ENTRY(_start)
 
/* Tell where the various sections of the object files will be put in the final
   Executable image. */
SECTIONS{

	.text : ALIGN(16){
		*(.start)		
		*(.text)
	}
 
	/* Read-only data. */
	.rodata : ALIGN(16){
		sysbase = .;
		*(.rodata)
	}
 
	/* Read-write data (initialised) */
	.data : ALIGN(16){
		*(.data)
	}
 
	/* Read-write data (uninitialised) */
	.bss : ALIGN(16){
		*(.bss)
	}
	
	.rel.text : ALIGN(16){
		*(.rel.text)
	}

	/* The compiler may produce other sections, by default it will put them in
	   a segment with the same name. Simply add stuff here as needed. */
}