hdiutil detach /Volumes/KICKSTART
qemu-system-x86_64 -vga std -m 1024 -drive format=raw,file="./disk.img" -boot menu=off
