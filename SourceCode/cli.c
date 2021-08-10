//
//  cli.c
//
//  Created by Matt Parsons on 10/11/2020.
//  Copyright © 2020 Matt Parsons. All rights reserved.
//

#include "cli.h"

#include "string.h"
#include "list.h"
#include "memory.h"
#include "intuition.h"

//This will be the Boot task, so it will bring up the early devices
#include "ata.h"
#include "fat_handler.h"
#include "dos.h"

#include "SystemLog.h"



int32_t width;
int32_t height;
int32_t x;
int32_t y;

uint32_t conCurX;
uint32_t conCurY;

intuition_t* intuibase;

uint32_t bufferIndex;


char consoleBuffer[4096];

void ConsoleClearCursor(window_t* window){

    intuibase->DrawRectangle(window,conCurX, conCurY, 8, 16, window->backgroundColour);
}



void ConsoleDrawCursor(window_t* window, uint32_t x, uint32_t y){
    conCurX = (x*8)+4;
    conCurY = (y*16)+22;
    intuibase->DrawRectangle(window,conCurX, conCurY, 8, 16, window->highlightColour);
}

void ConsoleRedraw(window_t* window);


void ConsoleScroll(window_t* window){
    
    
    if(bufferIndex >= 4095){
        
        for(int i = 0 ; i< 2048;++i){
            
            consoleBuffer[i] = consoleBuffer[i+2048];
            
        }
        
        bufferIndex = 2047;
        
    }
    
    ConsoleRedraw(window);
}



void ConsolePutCharPrivate(window_t* window, char character){
    
    //Doesn't record the character placement in the console buffer
    
    ConsoleClearCursor(window);
    
    if(character=='\n'){
        ++y;
        x=0;
        
        if(y >= height){
               ConsoleScroll(window);
               //y -= 1;
           }
        
    }else if(character=='\t'){
        
        //x = (x + 7) & 0xFFF8;
        if(x%4){
            x=x-1;
            x=x/4;
            x=x*4;
            x=x+4;
        }
        
    }else{
    
        intuibase->PutChar(window,(x*8)+4,(y*16)+22,character,window->foregroundColour,window->backgroundColour);
    
        x++;
        if(x>width){
            y++;
            
            if(y >= height){
                ConsoleScroll(window);
                //y -= 1;
            }
            
            x=0;
        }
    
    }
    ConsoleDrawCursor(window,x,y);
}

void ConsolePutChar(window_t* window, char character){
    
    
    consoleBuffer[bufferIndex++] = character;
    if(bufferIndex >= 4095){
        
        for(int i = 0 ; i < 2048;++i){
            
            consoleBuffer[i] = consoleBuffer[i+2048];
            
        }
        
        bufferIndex = 2048;
        
    }
    
    
    ConsolePutCharPrivate(window, character);

    
}

void ConsoleRedraw(window_t* window){
 
    
    //Count lines
    int32_t lines = 0;
    uint32_t i = 0;
    char c = 0;
    
    int32_t lineLength = 0;
    
    do{
        c = consoleBuffer[i];
        lineLength += 1;
        
        if(c =='\n'){
            lines += 1;
            lineLength = 0;
        }
        
        if(lineLength >= width){
            lines += 1;
            lineLength = 0;
        }
        
        i += 1;
    }while(i<bufferIndex);
    
    if(lines >= height){
    
        uint32_t startline = lines - height;

        lines = 0;
        i = 0;
        c = 0;
    
        uint32_t lineLength = 0;
    
        do{
            c = consoleBuffer[i];
            lineLength += 1;
        
            if(c =='\n'){
                lines += 1;
                lineLength = 0;
            }
        
            if((uint32_t)lineLength >= (uint32_t)width){
                lines += 1;
                lineLength = 0;
            }
        
            i += 1;
        }while((uint32_t)lines <= startline);
    
    }else{
        i = 0;
    }
    
    x = 0;
    y = 0;
    
    executive->Forbid(); //intuition doesn't lock the buffers properly
    
    intuibase->ClearWindow(window);
    
    for(; i<bufferIndex;++i){
        ConsolePutCharPrivate(window,consoleBuffer[i]);
    }

    executive->Permit();
    
    
}

void ConsoleWriteString(window_t* window, char* str){
    
    uint32_t size = strlen(str);
    
    for (size_t i = 0; i < size; i++){
        ConsolePutChar(window,str[i]);
    }
    
    
}

void ConsoleWriteDec(window_t* window, uint32_t n){
    
    if (n == 0){
         ConsolePutChar(window,'0');
         return;
     }
     
     int32_t acc = n;
     char c[32];
     int i = 0;
     while (acc > 0){
         c[i] = '0' + acc%10;
         acc /= 10;
         i++;
     }
     
     c[i] = 0;
     
     char c2[32];
     c2[i--] = 0;
     int j = 0;
     while(i >= 0){
         c2[i--] = c[j++];
     }
     
    ConsoleWriteString(window, c2);
    
}


void ConsoleWriteHex(window_t* window, uint32_t n){
    int32_t tmp;
     
    // debug_write_string("0x");
    ConsoleWriteString(window, "0x");
    
     char noZeroes = 1;
     
     int i;
     
     for (i = 28; i > 0; i -= 4){
         tmp = (n >> i) & 0xF;
         if (tmp == 0 && noZeroes != 0){
             continue;
         }
         
         if (tmp >= 0xA){
             noZeroes = 0;
             //debug_putchar(tmp - 0xA + 'a');
             ConsolePutChar(window,tmp - 0xA + 'a');
         }else{
             noZeroes = 0;
             //debug_putchar(tmp + '0');
             ConsolePutChar(window,tmp + '0');
         }
     }
     
     tmp = n & 0xF;
     if (tmp >= 0xA){
         //debug_putchar(tmp - 0xA + 'a');
         ConsolePutChar(window,tmp - 0xA + 'a');
     }else{
         //debug_putchar(tmp + '0');
         ConsolePutChar(window,tmp + '0');
     }
   
}


void ConsoleBackSpace(window_t* window){
    ConsoleClearCursor(window);
    x -= 1;
    bufferIndex -= 1;
    
    if(x<0){
        x = width;
        y -= 1;
    }
    ConsoleDrawCursor(window,x,y);
}


void ConsoleSize(window_t* window){
    width = (window->innerW / 8) - 2;
    height = ((window->innerH - 22) / 16) - 1;
}

void ConsoleMoveCursorX(window_t* console, int rx){

    while(x<rx){
        ConsolePutChar(console,' ');
    }
    
}

// Command Interpretor ***************************************************************
dos_t* dosbase;
window_t* console;
uint32_t commandBufferIndex = 0;
char commandBuffer[512];
char lastCommand[512];


void reportError(char** arguments){
    
    //debug_write_string("Reporting Error");
    //window_t* req;
    
    switch(executive->thisTask->dosError){
        case DOS_ERROR_UNKNOWN_DEVICE:
            
            //req = intuibase->Request("System Request");    //Throw a test Requestor, but why does it crash?
            /*
            //fake gadgets
            intuibase->DrawRectangle(req,17,98,63,33,intuibase->orange);
            intuibase->DrawRectangle(req,19,100,59,29,intuibase->white);
            intuibase->DrawRectangle(req,19,102,59,25,intuibase->blue);
            intuibase->DrawRectangle(req,21,104,55,21,intuibase->white);
            
            intuibase->DrawRectangle(req,200+17,98,63,33,intuibase->orange);
            intuibase->DrawRectangle(req,200+19,100,59,29,intuibase->white);
            intuibase->DrawRectangle(req,200+19,102,59,25,intuibase->blue);
            intuibase->DrawRectangle(req,200+21,104,55,21,intuibase->white);
            
            intuibase->DrawString(req,10,30," Error validating disk",intuibase->blue,intuibase->white);
            intuibase->DrawString(req,10,30+32," Disk is unreadable",intuibase->blue,intuibase->white);
            
            intuibase->DrawString(req,32,106,"Test",intuibase->blue,intuibase->white);
            intuibase->DrawString(req,232,106,"Test",intuibase->blue,intuibase->white);
            */
            ConsoleWriteString(console,"Unknown Device\n");
            break;
        case DOS_ERROR_DEVICE_NOT_SPECIFIED:
            ConsoleWriteString(console,"Device not specified\n");
            break;
        case DOS_ERROR_UNABLE_TO_OPEN_DEVICE:
            ConsoleWriteString(console,"Can't open device");
            break;
        case DOS_ERROR_OBJECT_NOT_FOUND:
            ConsoleWriteString(console,"File not found!\n");
            break;
        case DOS_ERROR_NOT_A_DIRECTORY:
            ConsoleWriteString(console,"Not a directory!\n");
            break;
        case DOS_ERROR_OBJECT_NOT_OF_REQUIRED_TYPE:
            ConsoleWriteString(console,"Object not of required type! (Not all devices support filesystem actions)\n");
            break;
            
        default:
            ConsoleWriteString(console,"An unknown error occured... Not very helpful.\n");
            ConsoleWriteString(console,arguments[0]);
            break;
    }
}

void processCommand(int commandLength){
    
    //debug_write_string("process command\n");
    
    //save last command
    strcpy(lastCommand, commandBuffer);
    
    //debug_write_string("saved buffer\n");
    
    //remove leading spaces
    int start = 0;
    while(commandBuffer[start] == ' ' && start < commandLength){
        start++;
    }
    
    //count arguments
    int count = 1;  // there is alwasy one argument if we get to the process buffer stage
    for(int i=start;i<commandLength;++i){
        
        if(commandBuffer[i]==' ' && (commandBuffer[i+1]!=' ' && commandBuffer[i+1]!=0) ){
            count++;
        }
    }
   
    //create an argument pointer array
    char* arguments[count];
    
    arguments[0] = &commandBuffer[start];
    int pos = 0;
    for(int i = start;i<commandLength;++i){
        
        if(commandBuffer[i]==' ' && (commandBuffer[i+1]!=' ' && commandBuffer[i+1]!=0) ){
            pos++;
            commandBuffer[i]=0;
            arguments[pos] = &commandBuffer[i + 1];
        }else if(commandBuffer[i]==' '){
            commandBuffer[i]=0; // null terminate the argument
        }
        
    }
    
    //disply the available devices, and add assigns
    if(strcmp(commandBuffer,"assign") == 0){

        ConsoleWriteString(console,"Devices:\n");
        
        
        list_t* dosList = &dosbase->dosList;
        node_t* dev = dosList->head;
        
        do{
            ConsoleWriteString(console,dev->name);ConsolePutChar(console,'\n');
            dev = dev->next;
        }while(dev->next != NULL);
    
        return;
    }
    
    
    //Need to set the current directory
    if(strcmp(commandBuffer,"cd") == 0){



        
        //if no arguments just print the progdir
        if(count < 2){
            if(executive->thisTask->progdir == NULL){
                ConsoleWriteString(console,"No current directory!\n");
            }else{
                //ConsoleWriteString(console," ");
                ConsoleWriteString(console,executive->thisTask->progdir);
                ConsolePutChar(console,'\n');
            }
            return;
        }
        
        //if arguments, then try to open and check the path is vaild
        //if path is valid then save it to the progdir
        
        file_t* file = dosbase->Open(arguments[1],0);

        
        if(file == NULL){
            reportError(arguments); //print the Dos Error
            return;
        }
        
        if(file->isDIR){
            
            char temp[512];
            for(int s=0;s<512;++s){
                temp[s] = 0;    //clear string
            }
            strcpy(temp,arguments[1]);
            
            int tmpLen = strlen(temp);
            
            if(temp[tmpLen-1] =='/' || temp[tmpLen-1] ==':'){
                //ConsoleWriteString(console,"Ok!\n");
            }else{
                temp[tmpLen] = '/';
                temp[tmpLen+1] = 0;
                //ConsoleWriteString(console,"Need to add traing slash\n");
            }
            
            
            if(executive->thisTask->progdir != NULL){
                executive->FreeMem(executive->thisTask->progdir);
            }
            
            executive->thisTask->progdir = (char*) executive->AllocMem(strlen(temp),0);
            strcpy(executive->thisTask->progdir,temp);
            
        }else{
            ConsoleWriteString(console,"Path was valid.. but it was a file.\n");
        }

        
        dosbase->Close(file);
        return;
    }
    
    //messy way to implment the dir command :-)
    if(strcmp(commandBuffer,"dir") == 0){
        //debug_write_string("Dir!\n");
            
        file_t* file;
        
        if(count<2){
            char temp[512];
            temp[0] = 0;
            file = dosbase->Open(temp,0);
        }else{
        
            file = dosbase->Open(arguments[1],0);
        }
            
            if(file == NULL){
            
                reportError(arguments); //print the Dos Error
                
            }else{
                
                directoryStruct_t* ds = dosbase->Examine(file);
                
                if(ds==NULL){
                    
                    switch(executive->thisTask->dosError){
                        case DOS_ERROR_NOT_A_DIRECTORY:
                            ConsoleWriteString(console,"Not a directory!\n");
                            break;
                            
                        default:
                            ConsoleWriteString(console,"Um... An error occured!\n");
                            break;
                            
                    }
                    return;
                    
                }
                
                //Print directory using the returned structure
                directoryEntry_t* dt = ds->entry;

                
                for(uint32_t i = 0;i<ds->size;i++){
                    
                    //Hide hidden
                    if(dt[i].isHidden){
                        continue;
                    }
                    
                    ConsoleWriteString(console,"   ");
                    ConsoleWriteString(console,dt[i].name);
                    ConsoleMoveCursorX(console,32);
                    
                    
                    if(dt[i].isDir){
                        ConsoleWriteString(console,"(dir)");
                    }else{
                        ConsoleWriteDec(console,dt[i].fileSize);
                    }
                    
                    //ConsoleWriteString(console," | Cluster: ");
                    //ConsoleWriteDec(console,dt[i].cluster);
                    
                    if(dt[i].isHidden){
                        ConsoleWriteString(console," (Hidden)");
                    }
                    
                    ConsolePutChar(console,'\n');
                    executive->Reschedule();    //slow the update rate down
                }
                executive->FreeMem(ds);//free the directory structure returned by Examine()
                
                dosbase->Close(file);
            }
            
            ConsolePutChar(console,'\n');
            
        return;
        
    }
    
    //An odd way to implment the echo command :-)
    if(strcmp(commandBuffer,"echo") == 0){
        
        if(count > 1){
           // ConsoleWriteString(console,"Openning file:\n");
            file_t* file = dosbase->Open(arguments[1],0);
            
            if(file == NULL){
                
                reportError(arguments); //print the Dos Error
                
            }else{
            
                if(file->isDIR){
                    ConsoleWriteString(console,"Not a file.\n");
                    dosbase->Close(file);
                    return;
                }

                ConsoleWriteString(console," Echoing file:\n");
                uint8_t* temp = dosbase->LoadFile(file);

                intuibase->SetBusy(console, true); //if the window can't process any events let the user know
                for(uint64_t i=0; i < file->size; ++i){
                    ConsolePutChar(console,temp[i]);
                    //debug_putchar(temp[i]);
                    //for(int j=0; j<1000; ++j){
                        executive->Reschedule();    //slow the update rate down
                    //}
                   // ConsoleBackSpace(console);
                }
                intuibase->SetBusy(console, false); //user can do things again.
                
                ConsoleWriteString(console,"\n");
                executive->FreeMem(temp);
                dosbase->Close(file);
            }
            
        }else{
            ConsoleWriteString(console,"Error: No file provided.\n");
        }
        
            return;
    }
    
    //A quick way to crash the machine
    if(strcmp(commandBuffer,"guru") == 0){
        char i = 0;
        char j = 128;
        char c = j / i;
        ConsolePutChar(console, c);
        return;
    }
    
    //A quick way to crash the machine
    if(strcmp(commandBuffer,"help") == 0){
        ConsoleWriteString(console," Supported Commands:\n");
        ConsoleWriteString(console,"   cd   (Usage: cd path) - changes the current path.\n");
        ConsoleWriteString(console,"   dir  (Usage: dir path) - lists files at the path.\n");
        ConsoleWriteString(console,"   echo (Usage: echo filename) - echos the contents of a file to the console.\n");
        ConsoleWriteString(console,"   guru (Usage: guru) - purposely crashes this task.\n");
        ConsoleWriteString(console,"   help (Usage: help) - prints this help.\n");
        ConsoleWriteString(console,"   load (Usage: load filename) - loads a relocatable ELF file into memory... and execute it?\n");
        return;
    }
    
    //Load an ELF Relocatable Object :-)
    if(strcmp(commandBuffer,"load") == 0){
        
        if(count > 1){
           // ConsoleWriteString(console,"Openning file:\n");
            file_t* file = dosbase->Open(arguments[1],0);
            
            if(file == NULL){
                
                reportError(arguments); //print the Dos Error
                
            }else{
            
                if(file->isDIR){
                    ConsoleWriteString(console,"Not a file.\n");
                    dosbase->Close(file);
                    return;
                }

                dosbase->LoadELF(file);
                
                //ConsoleWriteString(console,"\n");
                dosbase->Close(file);
            }
            
        }else{
            ConsoleWriteString(console,"Error: No file provided.\n");
        }
        
            return;
    }
    
    // THese three theming commands don't really work as intution isn't set up for runtime theme swapping yet.
    if(strcmp(commandBuffer,"themenew") == 0){
        intuibase->SetTheme(1);
        ConsoleWriteString(console,"AmigaOS 3.1 theme\n");
        return;
    }
    
    if(strcmp(commandBuffer,"themeold") == 0){
        intuibase->SetTheme(0);
        ConsoleWriteString(console,"AmigaOS 1.3 theme\n");
        return;
    }
    
    if(strcmp(commandBuffer,"thememac") == 0){
        intuibase->SetTheme(2);
        ConsoleWriteString(console,"Classic Mac theme\n");
        return;
    }
    
    if(strcmp(commandBuffer,"a") == 0){
        intuibase->OpenWindow(NULL, 0, 0, 200, 120,WINDOW_TITLEBAR | WINDOW_DRAGGABLE | WINDOW_DEPTH_GADGET, "Bouncy!");
        ConsoleWriteString(console,"New Window\n");
        return;
    }
    
    ConsoleWriteString(console,"Unknown Command ");
    ConsoleWriteString(console,arguments[0]);
    ConsolePutChar(console,'\n');

}








int CliEntry(void){
    
    
    //The Boot Task should be responsible for loading the initial Libraries and devices into memory.

    //setup the ata device as early as possible before the DOS Library, if the ATA device isn't ready DOS will hang the machine.
    LoadATADevice();
    executive->AddDevice((device_t*)&ata);
  
    //The boot disk will be FAT32, so load the FAT Handler
    LoadFATHandler();
    executive->AddDevice((device_t*)&fatHandler);
    
    //just busy wait,
    //volatile int t = 0;
    //for(int i=0;i<1000000;++i){
    //    t++;
    //}
    
    intuibase = (intuition_t*) executive->OpenLibrary("intuition.library",0);
    
    console = intuibase->OpenWindowPrivate(NULL,0,22,intuibase->screenWidth,(intuibase->screenHeight/2)-24,WINDOW_TITLEBAR | WINDOW_DRAGGABLE | WINDOW_DEPTH_GADGET | WINDOW_RESIZABLE, "BootShell");
    console->eventPort = executive->CreatePort("Event Port");

    ConsoleSize(console);
    x = 0;
    y = 0;
    conCurX = (x*8)+4;
    conCurY = (y*16)+22;

     // This bootshell task, this should be responsible for much of the system set up
    
    ConsoleWriteString(console,"Copyright ");
    ConsolePutChar(console,169);
    ConsoleWriteString(console,"2021 Matt Parsons.\nAll rights reserved.\nRelease 0.45a\n\n");
    

 
    
    //Initilise and open DOS library
    LoadDOSLibrary();   //Load DOS Library into memory (already done here) and call its "LoadXLibrary()" function
    executive->AddLibrary((library_t*)&dos);    // add DOS to the Exec library list.
    dosbase = (dos_t*) executive->OpenLibrary("dos.library",0);    // never use a library without opening it first

    
    debug_write_string("CLI MESSAGE: Type help for available commands.\n");
    ConsoleWriteString(console,"1> ");
    while(1){
        
        //debug_write_string("BootShell: Wait Event\n");
        executive->WaitPort(console->eventPort);
        intuitionEvent_t* event = (intuitionEvent_t*) executive->GetMessage(console->eventPort);
    
        while(event != NULL){
        
            if(event->flags & WINDOW_EVENT_RESIZE){
                intuibase->ResizeWindow(console, console->w - event->mouseXrel, console->h - event->mouseYrel);
                ConsoleSize(console);
            }
            
            if(event->flags & WINDOW_EVENT_RESIZE_END){
                
                ConsoleRedraw(console);
            }
            
            
            if(event->flags & WINDOW_EVENT_KEYDOWN){
            

                
                    if(event->rawKey == 0xE){
                        //delete key
                        
                        if(commandBufferIndex > 0){
                            commandBufferIndex -=1;
                            ConsoleBackSpace(console);
                        }
                        
                    }else if(event->rawKey == 0x1C){
                        // enter key
                        ConsolePutChar(console,'\n');   // newline as enter has been pressed
                        commandBuffer[commandBufferIndex]= 0; // append a NULL to the command buffer

                        
                        if(commandBufferIndex>0){
                            
                           processCommand(commandBufferIndex);
                
                        }
                        

                        commandBufferIndex  = 0;
                        ConsoleWriteString(console,"1> ");
                    }else if(event->rawKey == 0x48){
                        //up cursor
                        
                        //Delete current line
                        while(commandBufferIndex>0){
                            commandBufferIndex -=1;
                            ConsoleBackSpace(console);
                        }
                        
                        int i = 0;
                        while(lastCommand[i] != 0){
                            commandBuffer[commandBufferIndex] = lastCommand[i];
                            commandBufferIndex += 1;
                            ConsolePutChar(console, lastCommand[i]);
                            i++;
                        }
                            
                        
                        
                        
                    }else{
                        if(commandBufferIndex < 255){
                            commandBuffer[commandBufferIndex] = event->scancode;
                            commandBufferIndex += 1;
                            ConsolePutChar(console, event->scancode);
                        }
                    }
        
                
            }

            
            executive->ReplyMessage((message_t*)event); // Always reply a message as soon as possible
            //get next message
            event = (intuitionEvent_t*) executive->GetMessage(console->eventPort);
        }
    
    }
}
