i686-elf-as ./boot.s -o boot.o
i686-elf-gcc -c ./*.c -std=gnu99 -ffreestanding -O3 -Wall -Wextra
i686-elf-gcc -T ./linker.ld -o kernel.elf -ffreestanding -O3 -nostdlib ./*.o -lgcc

