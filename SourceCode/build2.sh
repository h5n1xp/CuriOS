i686-elf-as ./boot.s -o boot.o
i686-elf-gcc -c ./*.c -std=gnu99 -ffreestanding -O3 -Wall -Wextra
i686-elf-gcc -T ./linker.ld -o kernel.elf -ffreestanding -O3 -nostdlib ./*.o -lgcc -s

hdiutil attach ./disk.img
cp ./kernel.elf /Volumes/KICKSTART/kernel.elf
hdiutil detach /Volumes/KICKSTART
qemu-system-x86_64 -m 128m -vga virtio -drive format=raw,file="./disk.img" -boot menu=off -device qemu-xhci

# USB stuff
# https://www.kraxel.org/blog/2018/08/qemu-usb-tips/
#
# -device qemu-xhci -device usb-mouse # this setting seems to disable the PS2 mouse
