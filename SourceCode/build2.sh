i686-elf-as ./boot.s -o boot.o
i686-elf-gcc -c ./*.c -std=gnu99 -ffreestanding -O3 -Wall -Wextra
i686-elf-gcc -T ./linker.ld -o kernel.elf -ffreestanding -O3 -nostdlib ./*.o -lgcc

hdiutil attach ./disk.img
cp ./kernel.elf /Volumes/KICKSTART/kernel.elf
hdiutil detach /Volumes/KICKSTART
qemu-system-x86_64 -m 128m -vga std -drive format=raw,file="./disk.img" -boot menu=off
