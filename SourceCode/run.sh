hdiutil detach /Volumes/KICKSTART
qemu-system-x86_64 -vga virtio -m 1024 -drive format=raw,file="./disk.img" -boot menu=off -device qemu-xhci
