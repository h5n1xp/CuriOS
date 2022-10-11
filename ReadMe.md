# Read Me

CuriOS is closely modelled upon the original AmigaOS architecture, but with several very distinct differences. The biggest differences are as follows:
1.	AmigaOS is a slave to memory efficiency. CuriOS is not. The CuriOS GUI, for example, uses memory buffers and caching to ensure smooth, user responsiveness at all times. This requires significantly more memory but makes the GUI much faster even on very low-end Hardware. CuriOS also has more extensive resource tracking, which again requires more memory, but allows for cleaner task teardown (upon exit or crash). This is a conscious design decision, and the system could easily be made to be more memory efficient if needed for a port to a lower spec platform.
2.	AmigaOS makes provision for Assembly coders and as such is very closely tied to the 32bit 68000 CPU. CuriOS uses C and the SystemV ABI as its lowest level, system variables/pointers are generally 64bits in size (all structures are 16byte aligned) and endianness is CPU native (with a preference for Little Endian, due to x86 and ARM being the primary target processors).
3.	AmigaOS is full of brilliant architectural ideas, but nothing is very consistent. There are multiple ways to do almost everything, and some parts (like DOS) are built using a completely different but equivalent set of paradigms. The primary architectural model of CuriOS follows the AmigaOS Exec design as much as possible (and where it still makes sense in the 21st century), with care taken to ensure that all systems/subsystems are structured and operate in a consistent manner. This is the most difficult part to get right, and the project has had several code clean-ups (there are still plenty of places where vestiges of older approaches are still present).
4.	AmigaOS does not enforce policy, there are many places in AmigaOS where the system expects programs/tasks to operate in a specific way but does not throw an error/warning if they violate these expectations. A good example is with memory allocations, AmigaOS expects memory which is shared between tasks to be marked as such. CuriOS aims to enforce policy either in software or where appropriate using available hardware (i.e. an MMU).
5.	AmigaOS Exec has a few features which are poorly suited to multicore CPUs. Several features, like the Forbid()/Permit() function pair have the ability stop multitasking completely, and these are usually used instead of a proper resource locks. Such functions are deprecated in CuriOS, in favour of proper resource locking taking advantage of the atomic instructions found in modern CPU ISAs. Another single-core design quirk of AmigaOS is that a high priority task will ALWAYS execute in favour of a lower priority task, this “feature” is often abused in AmigaOS to avoid concurrency and resource locking issues. Since a low priority task may be executing on one core at the same time as a higher priority task, one cannot rely on the lower task being locked out of execution by the higher priority task. Do not rely on this behaviour when developing for CuriOS, use proper resource locking.



The Zip files here are the bootable raw hard disk image (and a VDI for VirtualBox), these builds tend to be reasonabily up to date...

The command shell currently supports a few commands, type help for a full list.

Three example programs are included on the boot disk, called prog.elf, draw.elf, and clock.elf
-Warning- The Kernel API is currently in flux, so you will likely need to recompile programs between kernel updates at this time. Once the Executive API stabilises programs shouldn't need to be recompiled between kernel updates.

The graphical user interface can be "themed", in the style of one of three built in themes, which can be selected at compile time to suit the user's preference. To set the theme, change the guiTheme variable in the intuition.c file to either THEME_OLD, THEME_NEW, or THEME_MAC.

Normal Theme:
![Screen shot](https://github.com/h5n1xp/CuriOS/blob/main/ScreenShot.png)

Classic Mac Theme:
![Classic Mac](https://github.com/h5n1xp/CuriOS/blob/main/ScreenShot1.png)

AmigaOS3.0 Theme:
![Amiga3](https://github.com/h5n1xp/CuriOS/blob/main/ScreenShot2.png)

Things to note:
1. Some components (libraries, devices), notably the older ones, are currently in a "primeval" state, that is to say they do not follow the normal initialisation process. Since they are compiled together a single blob, they have access to name-spaces out of normal scope. 

2. There is rudimentray file system support for FAT32 drives. There is a working ata device driver (read only), so you can read raw data blocks from a hard drive. There is a FAT32 filesystem handler (a "handler" is a special device/library which knows how to translate the block data returned from a device into stream data for DOS, all file systems are implmented this way, a device can be its own handler usually when the device doesn't support file systems) which sits on top of the ata device. When the filesystem is more mature, all future components will be loaded from disk and will be initialised and accessed via the proper formal executive interface. The code currently in cli.c is just temporary while I'm building the file system and DOS code. This will eventually become the boot task whick will take over most of the role that kernel.c currently performs. I will then write a proper command shell, which will then be spawned by the boot task. The CLI can load ELF executable files and execute them.

3. The is no proper build process, this is the 8th step (code rewrite) and I will formalise the build process with a hierarchical file structure and Make files once step 8 reaches maturity. At the moment the whole thing is built using simple shell scripts.

4. The Executive (the public interface of the kernel), is just a composite of various sub components (i.e. memory.h, list.h, task.h, ports.h, etc...) which is constructed during their initialisation, this is due to API being in flux at the moment. I plan to formalise this at some point.

5. The most basic operating system unit is the node. Everything in the operating system is a node, nodes record their type, their size, and also can have a name string. Nodes may be added to list instance (which is itself a node), but can only be a member of one list at any given time.

6. There are three fundamental components (everything is built from one of these):
    1. Libraries: these are groups of related functions, these are based on a shared code concept for speed and smallness of memory footprint and are linked at runtime. Currently library calls are generally handled in the context of the calling task, likely this will change and some functions will execute in a separate contex, as each library matures and where it makes sense. Perhaps unusually, libraries are linked at runtime, rather than compile time, or load time. With the exception of the executive library which is linked at load time. Libraries are closer to the OOP concept of classes than classic software libraries. when opening a library for use, the library returns an instance of itself. Most libraries are singltons and always return the same instance, where library member functions are instance dependant, they will take the instance address as an argument. 
    2. Devices: These are libraries, but with a standard API to support their their message passing interface. Devices are generally used to interact with real physical hardware, or virtual hardware devices which might emulate the functions of hardware which is not present. Devices usually have an associated task which processes requests asynchronously (and more often than not handles real hardware interrupts), and it is this context which requests are processed. Communication with a device is performed via the message passing interface provided by the executive.
       There is a superset of the device called a handler, this is a device which has extra set of standard functions allowing it to interface with the dos.library, it is not necessary for a handler to have it's own task context as it may be a simple translation layer, filesystems are like this. It will probably make sense for the Handler API to be added to all devices... not sure how Unix I want this.
    3. Tasks: This the smallest unit of executable code, each task executes in it's own context, all tasks are linked at load time with the executive, but can runtime link with any library available on the system. A task has 64 signals available to it, will execute until it Wait()s for a signal, a which point it won't be scheduled again until a waited signal is received. Tasks are scheduled acording to priority, high priority tasks will always preempt lower priority tasks, all tasks at the same priority are scheduled in a round robin. Signals are low level and not normally used by the developers. Messages are the normal method of IPC, and are built on the signalling system. Currently, tasks all operate at the same level of privilege, but I plan to introduce a privilege system where messages will only be received by a task/device if the calling task has a high enough privilege (this is unrelated the CPU privilege, all tasks execute at the lowest CPU privilege level, only interrupts and the "idle task" execute at the highest CPU privilege).

7. Message passing is the primary IPC used in the operating system. Messages have a specific structure and must be allocated using the executive CreateMessage() function, but they do allow for user defined data. For a receiving task to access a message it must call the executive function GetMessage(), this transfers ownership of the message to the receiving task, at which point the sending task can no longer access it. All messages need to be replied to once the receiving task has finished with the message (i.e. made a copy of the data it needs). If the message has no reply port set, the message will be deleted by the kernel. If the message has a reply port set, ownership of the message is returned to the sending task.

8. The kernel design is a fairly pure microkernel; Library calls generally happen in the context of the calling task, messages should be handled in the context of the receiving task (devices have mechanisms to get around this).

9. No memory protection, any task can access any memory address. Don't rely on it always being like this, only access memory obtained via the executive interface (always manipulate data structures via the documented interface), and don't try to access messages if they are not in the possession of that task. It might work now, it won't work in the future. 

10. There is no documentation... yet :-)

11. I plan to add an Application framework built on Objective-C using Jonathan Schleifer's Obj-FW runtime. This should hide the complexity of developing GUI applications, but CLI applications will probably still need to deal with the lower level system components.


CuriOS2
