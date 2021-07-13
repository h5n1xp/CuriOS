# Read Me

The Zip files here are the bootable hard disk image of the July 13th, 2021 Build of CuriOS... Source code to follow.

This includes a source snapshot as of the 13 - 07 - 2021. 

The command shell currently only supports a single command, dir, which allows you to explore the boot disk, it needs a full path starting with the device name which for the boot disk is dh0: so to see the contents of the root one would type dir dh0:

The graphical user interface can be "themed", in the style of one of three built in themes, which can be selected at compile time to suit the user's preference.

Normal Theme:
![Screen shot](https://github.com/h5n1xp/CuriOS/blob/main/ScreenShot.png)

Classic Mac Theme:
![Classic Mac](https://github.com/h5n1xp/CuriOS/blob/main/ScreenShot1.png)

AmigaOS3.0 Theme:
![Amiga3](https://github.com/h5n1xp/CuriOS/blob/main/ScreenShot2.png)

Things to note:
1. All components (libraries, devices, and tasks) are currently in a "primeval" state, that is to say they do not follow the normal initialisation process. Since they are compiled together a single blob, they have access to name-spaces that they will normally access via the executive controlled interface, also they must be initialised either by the startup "kernel thread", or the boot task. This would normally be handled by the executive upon loading from disk.

2. There is no working file system. There is however, a working ata device driver, so you can read raw data blocks from a hard drive. I am currently adding a FAT filesystem handler to sit on top of the ata device, when this work is completed, all future components will be loaded from disk and will be initialised and accessed via the proper formal executive interface. The code currently in cli.c is just tempoary while I'm building the file system and DOS code. This will enventually become the boot task whick will take over most of the role that kernel.c currently performs.

3. The is no proper build process, this is the 8th step (code rewrite) and I will formalise the build process with a hierarchical file structure and Make files once step 8 reaches maturity. At the moment the whole thing is built using shell scripts.

4. The Executive (the public interface of the kernel), is just a composite of various sub components (i.e. memory.h, list.h, task.h, ports.h, etc...) which is constructed during their initialisation, this is due to API being in flux at the moment. I plan to formalise this at some point.

5. There are three fundamental components (everything is built from one of these):
    1. Libraries: these are groups of related functions, these are based on a shared code concept for speed and smallness of memory footprint and are linked at runtime. Currently library calls are generally handled in the context of the calling task, likely this will change and some functions will execute in a separate contex, as each library matures and where it makes sense.
    2. Devices: These are libraries, but with a standard API to support their their message passing interface. All Devices have an associated task which handles requests, and it is this context which requests are handled.
    3. Tasks: This the smallest unit of executable code, each task executes in it's own context, all tasks are linked at load time with the executive, but can runtime link with any library available on the system. A task has 64 signals available to it, will execute until it Wait()s for a signal, a which point it won't be scheduled again until a waited signal is received. Signals are low level and not normally used by the developers. Messages are the normal method of IPC, and are built on the signalling system. Currently, tasks all operate at the same level of privilege, but I plan to introduce a privilege system where messages will only be received by a task/device if the calling task has a high enough privilege (this is unrelated the CPU privilege, all tasks execute at the lowest CPU privilege level, interrupts and the "idle task" execute at the highest CPU privilege).

6. The kernel design is a fairly pure microkernel; Library calls generally happen in the context of the calling task, messages should be handled in the context of the receiving task (devices have mechanisms to get around this).

7. No memory protection, any task can access any memory address. Don't rely on it always being like this, only access memory obtained via the executive interface (always manipulate data structures via the documented interface), and don't try to access messages if they are not in the possession of that task. Its might work now, it won't work in future. 

8. There is no documentation... yet :-)
