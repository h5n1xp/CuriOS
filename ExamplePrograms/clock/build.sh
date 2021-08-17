i686-elf-as ./startup.s -o startup.o
i686-elf-gcc -c ./*.c -fPIE -static-pie -nostdlib -ffreestanding -O3 -Wall -Wextra
i686-elf-gcc -T ./linker.ld -o clock.elf -fPIE -static-pie -ffreestanding -O3 -nostdlib ./*.o -lgcc -s
