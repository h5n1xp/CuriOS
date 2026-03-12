# CLAUDE.md — CuriOS Codebase Guide

## Project Overview

CuriOS is a from-scratch x86 operating system kernel written in C, modelled on the AmigaOS architecture but modernised for 21st-century hardware (64-bit pointers, multicore-safe locking, little-endian preference, System V ABI). It boots via GRUB/Multiboot2 and targets i686/x86_64 bare metal, running under QEMU for development.

The codebase is at an **active development / stabilisation stage** (currently described as "step 8" of the author's rewrites). The build system is intentionally simple shell scripts — a proper hierarchical Makefile structure is planned but not yet present.

---

## Repository Layout

```
CuriOS/
├── SourceCode/          # Kernel source (the entire OS kernel)
│   ├── build.sh         # Primary build script
│   ├── build2.sh        # macOS build + QEMU run (copies to disk.img)
│   ├── build3.sh        # Alternative build variant
│   ├── run.sh           # Run QEMU only (disk already updated)
│   ├── run2.sh          # Alternative run script
│   ├── tovdi.sh         # Convert disk.img → VirtualBox VDI
│   ├── boot.s           # Multiboot2 header + x86 entry point + all ISR stubs
│   ├── linker.ld        # Linker script — kernel loaded at 1 MiB
│   ├── kernel.c         # kernel_main() entry + KernelTaskEntry (Executive task)
│   └── *.c / *.h        # Kernel subsystem files (see below)
│
├── ExamplePrograms/     # User-space ELF programs (built separately)
│   ├── prog/            # Minimal hello-world style program
│   ├── draw/            # GUI drawing demo (uses intuition.library)
│   └── clock/           # Clock demo (uses math + intuition)
│
├── disk.img.zip         # Bootable FAT32 raw disk image (GRUB + kernel.elf)
├── disk.vdi.zip         # VirtualBox VDI version of the same disk
├── Amiga.zip            # Reference materials
├── ReadMe.md            # Original project readme
└── ScreenShot*.png      # GUI theme screenshots
```

---

## Toolchain Requirements

The kernel **must** be built with a bare-metal cross-compiler. Using the host system's GCC will produce a `#error` at compile time.

| Tool | Version / Notes |
|---|---|
| `i686-elf-gcc` | i686 bare-metal cross-compiler (no OS, no stdlib) |
| `i686-elf-as` | Corresponding binutils assembler |
| QEMU | `qemu-system-x86_64` for running |
| GRUB | Installed on `disk.img` — not rebuilt from source here |

Build flags used by `build.sh`:
```sh
i686-elf-as ./boot.s -o boot.o
i686-elf-gcc -c ./*.c -std=gnu99 -ffreestanding -O3 -Wall -Wextra
i686-elf-gcc -T ./linker.ld -o kernel.elf -ffreestanding -O3 -nostdlib ./*.o -lgcc
```

Key flags: `-ffreestanding` (no hosted libc), `-nostdlib` (link only `libgcc`), `-O3`.

### Example Programs (User-space ELF)

Each program under `ExamplePrograms/` has its own `build.sh`:
```sh
i686-elf-as ./startup.s -o startup.o
i686-elf-gcc -c ./*.c -fPIE -static-pie -nostdlib -ffreestanding -O3 -Wall -Wextra
i686-elf-gcc -T ./linker.ld -o <name>.elf -fPIE -static-pie -ffreestanding -O3 -nostdlib ./*.o -lgcc -s
```
Key difference from kernel: `-fPIE -static-pie` — user programs are position-independent ELF executables loaded by the CLI/ELF loader at runtime.

### Running

```sh
cd SourceCode
# Build kernel:
./build.sh

# Copy kernel.elf to disk image and run (macOS hdiutil workflow):
./build2.sh

# Run without rebuilding:
./run.sh    # 1 GiB RAM
```
QEMU is started with `-vga virtio` and `-device qemu-xhci`. Screen resolution is set in `boot.s` (default 1024×768×32bpp).

---

## Boot Sequence

1. **GRUB** loads `kernel.elf` (Multiboot2 ELF) at 1 MiB (`linker.ld`).
2. **`boot.s` / `_start`** sets up the stack and calls `kernel_main()`.
3. **`kernel_main()`** (`kernel.c`):
   - `init_descriptor_tables()` — GDT, IDT, TSS.
   - `init_from_multiboot()` — parse memory map, set up framebuffer address.
   - `InitMultitasking()` — creates the `executive_t` structure and idle context.
   - Creates the **Executive task** (`KernelTaskEntry`, priority 127, supervisor mode).
   - `LoadGraphicsLibrary()` + `ChangeFrameBufferPrivate()`.
   - `LoadIntuitionLibrary()`.
   - `InitSystemLog()` — debug console rendered in the lower half of the screen.
   - `LoadPCIDevice()`, `InitTimer(1000Hz)`, `InitPS2()`.
   - `asm volatile("sti")` — enables interrupts, starts multitasking. `kernel_main` never returns.
4. **`KernelTaskEntry()`** runs as the highest-priority kernel task:
   - Loads ATA device, FAT handler, DOS library.
   - Spawns the **BootShell** task (`CliEntry`).
   - Sits in a message loop handling `executiveRequest_t` messages (add/remove task, shutdown).

---

## Kernel Architecture

### The Executive (`executive_t`)

The global `executive` pointer (defined in `memory.h`) is the **single system object** that exposes the entire kernel API as a function-pointer struct. It is conceptually the public vtable of the kernel. Tasks access all OS services through it:

```c
executive->AllocMem(size, type);
executive->CreateTask("name", pri, entry, stackSize);
executive->OpenLibrary("graphics.library", 0);
executive->PutMessage(port, message);
executive->Wait(signalMask);
```

The Executive struct is split into functional groups: memory, lists, locking, libraries, tasks, signals, message ports, devices/handlers, and interrupts. There is also a **private area** (functions ending in `Private`) that should never be called by user-space tasks.

### Fundamental Object: `node_t`

Everything in the OS is a `node_t`. Nodes carry:
- `next` / `prev` — doubly-linked list pointers.
- `type` — `NODE_TASK`, `NODE_LIBRARY`, `NODE_DEVICE`, `NODE_MESSAGE`, `NODE_WINDOW`, etc.
- `size`, `flags`, `allocFlags`, `priority`, `name`.

`list_t` is itself a `node_t` and contains a `lock_t` for thread safety.

### Three Core Object Types

| Type | Header | Description |
|---|---|---|
| **Library** | `library.h` | Group of related functions. Runtime-linked. Most are singletons. `OpenLibrary()` returns an instance. Has `Init`, `Open`, `Close`, `Expunge` hooks. |
| **Device** | `device.h` | Library + standard message-passing I/O interface (`BeginIO`, `AbortIO`). Has a `unit_t` list. Communicates asynchronously via `ioRequest_t` messages. |
| **Handler** | `handler.h` | Device + DOS-compatible interface (`Mount`, `Unmount`, `ReadDir`, `LoadFileAtCluster`). Used for filesystems (e.g., FAT32 handler sits on top of ATA device). |

### Library Instance Pattern

When a task opens a library it receives a `library_t` instance:
```c
struct library_t {
    node_t      node;
    int64_t     openCount;
    uint64_t    version;
    library_t*  baseLibrary;
    uint32_t    (*Init)(library_t*);
    library_t*  (*Open)(library_t*);
    void        (*Close)(library_t*);
    void        (*Expunge)(library_t*);
    void        (*Reserved)(void);
};
```
Each library extends this (via C struct embedding) with its own function pointers and data. For example, `graphics_t` embeds `library_t` and adds drawing functions + framebuffer state.

### Tasks and Scheduling

- `task_t` embeds `node_t`. States: `TASK_SUSPENDED`, `TASK_WAITING`, `TASK_READY`, `TASK_RUNNING`, `TASK_ENDED`.
- Tasks have 64 signals (`signalAlloc`, `signalReceived`, `signalWait`).
- Priority scheduling: higher-priority tasks preempt lower ones. Same-priority = round-robin.
- `Wait(signalMask)` blocks until a matched signal arrives.
- **`Forbid()`/`Permit()` are deprecated** — use proper `lock_t` locking instead (especially critical on multicore). Do not rely on high-priority tasks excluding lower-priority ones from execution.
- Each task has a `memoryList` tracking its allocations for clean teardown.

### Message Passing (IPC)

```
CreateMessage() → PutMessage() → [task receives signal] → GetMessage() → [process] → ReplyMessage()
```
- `CreateMessage()` allocates and associates a reply port.
- `GetMessage()` transfers ownership to the receiver — sender cannot touch it after `PutMessage`.
- `ReplyMessage()` returns ownership to the sender (or deletes it if no reply port).
- All device I/O uses `ioRequest_t` (which embeds `message_t`).

### Memory Management

- `AllocMem(size, type)` / `FreeMem(ptr)` — tracked per-task in `task_t.memoryList`.
- `Alloc(size, attrs)` / `Dealloc(node)` — lower-level node allocator.
- No MMU-enforced protection yet; any task can access any address. **Always allocate via Executive** — this will matter once MMU protection is added.
- Memory sharing between tasks requires `ALLOC_FLAGS_PUBLIC` flag.
- `DefragMem()` coalesces free blocks.

### Locking

```c
executive->Lock(&myLock);      // blocks until acquired
executive->FreeLock(&myLock);  // releases
executive->TestLock(&myLock);  // non-blocking test
```
`lock_t` uses a `volatile bool isLocked` field with task priority tracking. Use locking (not `Forbid`/priority tricks) for all shared resource access.

---

## Key Subsystems

| File(s) | Subsystem | Notes |
|---|---|---|
| `boot.s` | Multiboot header, `_start`, all ISR/IRQ stubs | Sets requested screen res (default 1024×768) |
| `descriptor_tables.c/h` | GDT, IDT, TSS setup | |
| `interrupts.c/h` | ISR/IRQ dispatch, `isr_handler`, `irq_handler` | |
| `timer.c/h` | PIT at 1000 Hz — drives scheduler ticks | |
| `task.c/h` | Task structs, `InitMultitasking()`, context switching | |
| `memory.c/h` | Executive struct, memory allocator, executive API | |
| `list.c/h` | `node_t`, `list_t`, all list operations | |
| `ports.c/h` | `messagePort_t`, `message_t`, IPC primitives | |
| `library.c/h` | `library_t` base, `LibExample` | |
| `device.c/h` | `device_t`, `ioRequest_t`, `unit_t`, device API | |
| `handler.c/h` | `handler_t` — device + DOS interface | |
| `graphics.c/h` | `graphics_t` library — bitmap, pixel, line, circle, flood-fill, sprites, vector images, font rendering | |
| `intuition.c/h` | `intuition_t` — windowing system, gadgets, themes, events | Theme: `THEME_OLD`, `THEME_NEW`, `THEME_MAC` — set `guiTheme` in `intuition.c` |
| `dos.c/h` | DOS library — file operations | |
| `dosCommonStructures.h` | Shared DOS types (`dosEntry_t`, `file_t`, `directoryStruct_t`) | |
| `fat_handler.c/h` | FAT32 filesystem handler | Sits on top of ATA device |
| `ata.c/h` | ATA disk device driver (read-only currently) | |
| `pci.c/h` | PCI bus enumeration device | |
| `ps2.c/h` | PS/2 keyboard + mouse driver | |
| `cli.c/h` | Boot shell / CLI — temporary, will become proper boot task | ELF loader lives here |
| `console_device.c/h` | Console device (in progress) | |
| `multiboot.c/h` | Multiboot info parsing | |
| `SystemLog.c/h` | Debug log (rendered in lower screen half) | `debug_write_string()`, `debug_write_hex()`, etc. |
| `string.c/h` | String utilities | |
| `stdlib.c/h` | Standard library helpers | |
| `math.c/h` | Math utilities | |
| `stdheaders.h` | Includes `<stdbool.h>`, `<stddef.h>`, `<stdint.h>`. Defines `void_ptr` as `uint32_t` | |
| `x86cpu_ports.c/h` | x86 `inb`/`outb` port I/O | |
| `font.h` | Bitmap font data | |

---

## Writing User-Space Programs

User programs live in `ExamplePrograms/`. Each has its own directory with a `curios.h` (copy of the public headers) and a `startup.s`.

### Minimal Program Structure

```c
#include "curios.h"

// The ELF loader will patch this with the real executive address
executive_t* executive __attribute__((section(".executive"))) = {0};

const char VERSTAG[] = "\0$VER: MyProg 0.1a (date) by Author";

void main() {
    // Open a library
    intuition_t* intuibase = (intuition_t*)executive->OpenLibrary("intuition.library", 0);
    if (intuibase == NULL) return;

    // Open a window
    window_t* win = intuibase->OpenWindow(NULL, 100, 100, 400, 300,
        WINDOW_TITLEBAR | WINDOW_CLOSE_GADGET | WINDOW_DRAGGABLE, "My Window");
    win->eventPort = executive->CreatePort("myEventPort");

    int running = 1;
    while (running) {
        uint64_t sig = executive->Wait(1 << win->eventPort->sigNum);
        if (sig & (1 << win->eventPort->sigNum)) {
            intuitionEvent_t* ev = (intuitionEvent_t*)executive->GetMessage(win->eventPort);
            while (ev) {
                if (ev->flags & WINDOW_EVENT_CLOSE) {
                    running = 0;
                    executive->ReplyMessage((message_t*)ev);
                    intuibase->CloseWindow(win);
                    break;
                }
                executive->ReplyMessage((message_t*)ev);
                ev = (intuitionEvent_t*)executive->GetMessage(win->eventPort);
            }
        }
    }

    executive->RemTask(NULL); // clean up this task
}
```

### Key User-Space Rules

1. **Always `ReplyMessage()` every message** you receive — failure leaks memory and can deadlock.
2. **Call `executive->RemTask(NULL)` at program exit** to clean up the task.
3. **Never access a message after `PutMessage()` or `ReplyMessage()`** — ownership is transferred.
4. **Use `executive->AllocMem()` / `executive->FreeMem()`** for heap allocation, never `malloc`.
5. **Do not use `Forbid()`/`Permit()`** — use `Lock()`/`FreeLock()` instead.
6. The `executive` pointer is in the `.executive` ELF section and is patched by the ELF loader at load time.

---

## Coding Conventions

- **Language**: C (GNU99 dialect), AT&T syntax Assembly (GAS).
- **Naming**: `snake_case` for variables and functions; `_t` suffix for all typedefs (`task_t`, `list_t`, `library_t`, etc.); `UPPER_CASE` for `#define` constants.
- **Header guards**: `#ifndef foo_h` / `#define foo_h` / `#endif /* foo_h */`.
- **Struct embedding for inheritance**: `device_t` starts with `library_t library;`, `handler_t` starts with `device_t device;`. Cast the pointer to access the embedded type.
- **All structures 16-byte aligned** (required by ABI; keep in mind when adding struct fields).
- **64-bit pointers** for system data structures (`uint64_t` sizes, etc.), but the current `void_ptr` typedef is `uint32_t` — a known in-progress issue (see `stdheaders.h`).
- **`VERSTAG`**: every source file (kernel module or user program) should have a version string: `"\0$VER: Name Version (date) by Author"`.
- **Private vs Public**: functions/fields named `*Private` are kernel-internal only; never call them from user tasks.

---

## Architecture Decisions to Be Aware Of

1. **No `malloc`/`printf`/libc** — this is freestanding C. Everything is provided by the Executive or kernel subsystems.
2. **No MMU protection yet** — memory isolation will be enforced in the future. Write code as if it will be there (use the Executive allocator, don't cache pointers across ownership boundaries).
3. **Kernel API is in flux** — user programs may need recompilation after kernel updates until the Executive API stabilises.
4. **No formal build system** — build is driven by `build.sh` shell scripts. A hierarchical Makefile structure is planned.
5. **All kernel code compiles as a single blob** — all `.c` files in `SourceCode/` are compiled together; they share namespace. Some older components are "primeval" and access each other's internals directly rather than via the Executive interface.
6. **`cli.c` is temporary** — it currently handles boot responsibilities that will eventually move to a proper boot task. The ELF loader is here.
7. **GUI themes** are set at compile time via the `guiTheme` variable in `intuition.c`.
8. **Multicore readiness**: the task scheduler and locking primitives are designed with multicore in mind, but are not yet fully tested on SMP hardware.

---

## Development Workflow

### Building the Kernel

```sh
cd SourceCode
./build.sh          # produces kernel.elf
```

### Running Under QEMU (Linux)

You need to manually place `kernel.elf` on the FAT32 `disk.img`. Mount the image, copy the file, unmount, then run QEMU. The `build2.sh` script automates this on macOS using `hdiutil`. On Linux, adapt using `mount -o loop`.

```sh
# Example Linux workflow:
sudo mount -o loop,offset=<partition_offset> disk.img /mnt/curios
sudo cp kernel.elf /mnt/curios/kernel.elf
sudo umount /mnt/curios
qemu-system-x86_64 -m 128m -vga virtio -drive format=raw,file=disk.img -boot menu=off -device qemu-xhci
```

### Building Example Programs

```sh
cd ExamplePrograms/draw
./build.sh      # produces draw.elf
# copy draw.elf to the disk image
```

### Debugging

- `debug_write_string()`, `debug_write_hex()`, `debug_write_dec()` write to the `SystemLog` — visible in the lower half of the screen and accessible via the `executive` function pointers.
- No GDB integration is currently set up. QEMU's `-monitor stdio` or `-s -S` with a remote GDB session can be used.

---

## Current Status & Known Gaps

- **File system**: FAT32 read-only via ATA device + FAT handler. Write support not yet implemented.
- **CLI**: Temporary boot shell with ELF loader. `help` command lists available commands.
- **Console device**: `console_device.c` exists but is not yet replacing the `SystemLog` debug output.
- **Task teardown**: `EXEC_REQUEST_REM_TASK` handling in `kernel.c` is incomplete — memory/port deallocation is noted as TODO.
- **Privilege system**: All tasks currently run at the same privilege level. CPU ring separation and message-based privilege checks are planned.
- **ObjC application framework**: Planned (using ObjFW runtime) to simplify GUI app development.
- **Makefile**: Planned replacement for the shell script build system.
