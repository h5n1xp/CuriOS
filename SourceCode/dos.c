//
//  dos.c
//
//  Created by Matt Parsons on 20/11/2020.
//  Copyright © 2020 Matt Parsons. All rights reserved.
//

#include "dos.h"
#include "memory.h"

#include "ata.h"
#include "handler.h"
#include "string.h"

#include "SystemLog.h"

dos_t dos;




//***************** ELF LOADING STRUCTURES

typedef struct {
    unsigned char     e_ident[16];      // The first uin32_t should = 1179403647 and this is ".ELF"
    uint16_t          e_type;           // This need to be 1 as that is a relocatable type which we use
    uint16_t          e_machine;        // This should be 3 for i386 (4 for 68000)
    uint32_t          e_version;        // Always 1
    uint32_t          e_entry;          // N/A
    uint32_t          e_phoff;          // N/A
    uint32_t          e_shoff;          // Where the program sections reside
    uint32_t          e_flags;          // N/A
    uint16_t          e_ehsize;         // The size of this header (52bytes)
    uint16_t          e_phentsize;      // N/A
    uint16_t          e_phnum;          // N/A
    uint16_t          e_shentsize;      // Section header size
    uint16_t          e_shnum;          // Number of Sections
    uint16_t          e_shstrndx;       // string table
} Elf32_Header_t;

typedef struct {
    uint32_t    sh_name;    //offset into string table
    uint32_t    sh_type;
    uint32_t    sh_flags;
    uint32_t    sh_addr;
    uint32_t    sh_offset;
    uint32_t    sh_size;
    uint32_t    sh_link;
    uint32_t    sh_info;
    uint32_t    sh_addralign;
    uint32_t    sh_entsize;
} Elf32_Section_Header_t;

typedef struct {
    uint32_t        st_name;    //offset into string table
    uint32_t        st_value;
    uint32_t        st_size;    //symbol size in bytes
    unsigned char   st_info;
    unsigned char   st_other;
    uint16_t        st_shndx;
} Elf32_Symbol_t;







//forward declare this function
void Close(file_t* file);


//public Functions
file_t* Open(char* fileName, uint64_t attributes){
    
    char path[512]; //the longest possible path length
    path[0]=0;      //Ensure the path string is null terminated
    
    executive->thisTask->dosError = DOS_ERROR_NO_ERROR;
    
    //debug_write_string("get device name\n");
    attributes +=1; //suppress compiler warning
    int i = 0;
    int devNameLen = 0;
    while(fileName[i] !=0){
        
        if(fileName[i]==':'){
            devNameLen = i;
            break;
        }
        
        i++;
    }
        
    if(devNameLen==0){
        
        //Now we need to see if a device/path is provided in the current task's progdir
        if(executive->thisTask->progdir == NULL){
            //debug_write_string("No Device specified\n");
            executive->thisTask->dosError = DOS_ERROR_DEVICE_NOT_SPECIFIED;
            return NULL;
        }

       //need to build a new path with the progdir prefix
        //debug_write_string("device specified in progdir!\n");
        

        
        char temp[512];
        
        strcpy(temp,executive->thisTask->progdir);
        //debug_write_dec(strlen(fileName));
        //debug_write_string(temp);
        int prgLen = strlen(executive->thisTask->progdir);
        int fnLen = strlen(fileName);
        //debug_write_dec(prgLen);
        
        if(fnLen>0){
            //debug_write_string("This did happen\n");
            //debug_write_string(fileName);debug_putchar('\n');
            char* suffix = &temp[prgLen];

            strcpy(suffix,fileName);
        
        }
                
        strcpy(fileName,temp);
        
        //debug_write_string(fileName);debug_putchar('\n');
        
        
        i = 0;
        while(fileName[i] !=0){
            
            if(fileName[i]==':'){
                devNameLen = i;
                break;
            }
            
            i++;
        }
            
        
    }
    
    //debug_write_dec(devNameLen);debug_putchar('\n');
    //debug_write_string(fileName);debug_putchar('@');debug_putchar('\n');
    
    char dev[devNameLen+1];
    for(int i=0;i<devNameLen;++i){
        dev[i] = fileName[i];
    }
    dev[devNameLen] = 0;
    tolower(dev);
    
    
    
    //now to get the path.
    int j = 0;
    int pathElementCount = 0;
    
   // debug_write_string("get path\n");
    while(fileName[i] !=0){
        
        //End of path reached with a space
        if(fileName[i] ==' ' ){
            path[j] = 0;
            break;
        }
        
        //End of path reached with a trailing forward slash
        if(fileName[i] =='/' && (fileName[i+1] == 0 || fileName[i+1] == ' ' ) ){
            path[j-1] = 0;
            break;
        }
        
        if(fileName[i] =='/'){
            pathElementCount +=1;
        }
        
        i++;
        path[j] = fileName[i];
        j++;
    }
    //debug_write_dec(j);

    if(j>1){
        pathElementCount +=1;
    }
    
    
    //debug_write_string("finding dos entry for device\n");
    dosEntry_t* entry = (dosEntry_t*)executive->FindName(&dos.dosList,dev);
    
    if(entry == NULL){
        //debug_write_string("unknown device ");debug_write_string(dev);debug_putchar('\n');
        executive->thisTask->dosError = DOS_ERROR_UNKNOWN_DEVICE;
        return NULL;
    }
   // debug_write_string("found dos entry for device\n");
    //debug_write_string(dev);debug_putchar('\n');
    
    
    
    //need to Open the underlying device, likely the ata.device
    //save the IORequest into the file_t structure.
    file_t* file = (file_t*)executive->Alloc(sizeof(file_t));
    file->node.type = NODE_FILE_DESCRIPTOR;
    file->request = executive->CreateIORequest(executive->thisTask->dosPort, sizeof(ioRequest_t));
    file->entry = entry;
    //debug_write_string("allocated a file structure\n");
    
    if(executive->OpenDevice(entry->deviceName,entry->unitNumber,file->request,0)){
        //debug_write_string("DOS Library: No Hard Disk!!!\n");
        
        //Need to do the Clean DOS init clean up!!!
        executive->Dealloc((node_t*)file->request); //deallocate the IORequest
        executive->Dealloc((node_t*)file);
        executive->thisTask->dosError = DOS_ERROR_UNABLE_TO_OPEN_DEVICE;
        return NULL;
    }
    
    
    
    //debug_write_string("underlying device opened\n");
    
    //Since the device opened OK, Allocate a one 512byte buffer for the File IORequest
    uint8_t* buffer = executive->AllocMem(512,0);
    file->request->data = buffer;
    file->request->length = 512;
    
    
    //debug_write_string("allocated IORequest buffer\n");
    
    //Need to read the path data into the file_t structure...
    //debug_write_string("Path: " );debug_write_string(path);debug_putchar('\n');
    //debug_write_dec(pathElementCount);debug_putchar('\n');
    
    char* elements[pathElementCount];
    elements[0] = path;
    
    j=0;
    int count = 1;
    while(path[j] !=0){
        
        if(path[j]=='/'){
            
            path[j] = 0;
            elements[count] = &path[j+1];
            
            count += 1;
        }
        
        j++;
    }
    //debug_write_string("counted path elements\n");

    
    
    //Just return the root
    if(pathElementCount==0){
        file->isDIR = true;     //this only returns a directory for now...
        file->startBlock = 0;   // the root block, should probably get this from the DOS entry...
        //debug_write_string("DOS Library: Just return the Root, no need to explore the drive\n\n");
        return file;
    }
    
    //debug_write_string("preparing to scan root\n");
    
    //Get the handler
    handler_t* handler = (handler_t*) file->entry->handler;
    
    //debug_write_string("DOS Library: Reading Root\n\n");
    directoryStruct_t* ds = NULL;
    
    count = 0;
    j = 0;
    uint32_t clusterNumber = 0;
    bool sucess;
    do{
        sucess = false;
        //ds = readDir(file,clusterNumber);
        ds = handler->ReadDir(file,clusterNumber);
        
        if(ds==NULL){
            debug_write_string("ERROR!!! CAN'T READ DIR at block"); debug_putchar(clusterNumber);debug_putchar('\n');
        }
        
        directoryEntry_t* dirE = ds->entry;
        //debug_write_string("Searching For ");debug_write_string(elements[count]);debug_putchar('\n');
        for(uint32_t i=0; i < ds->size; ++i){
            
            //debug_write_string(dirE[i].name);debug_putchar('\n');
            
            if( strcmp(dirE[i].name,elements[count]) == 0){
                //debug_write_string("Found: ");
                //debug_write_string(elements[count]);
                //debug_putchar('\n');
                
                clusterNumber = dirE[i].cluster;
                
                file->size = dirE[i].fileSize;
                file->startBlock = clusterNumber;
                file->isDIR = dirE[i].isDir;
                sucess = true;
            }
            

        }
        
        executive->FreeMem(ds);//free the directory structure returned by readDir()
        count +=1;
    }while(count < pathElementCount);

    if(sucess==false){
        Close(file);
        executive->thisTask->dosError = DOS_ERROR_OBJECT_NOT_FOUND;
        return NULL;
    }
    
    //debug_write_dec((uint32_t)ds);
    //executive->FreeMem(ds);//free the directory structure returned by readDir()
    
    //file->isDIR = true;     //this only returns a directory for now...
    //file->startBlock = 0;   // the root block, should probably get this from the DOS entry...
    return file;
    
}


void Close(file_t* file){
    executive->thisTask->dosError = DOS_ERROR_NO_ERROR;
    executive->CloseDevice(file->request);      //close the ata device
    executive->FreeMem(file->request->data);    //free the memory used by the IORequest
    executive->Dealloc((node_t*)file->request); //deallocate the IORequest
    executive->Dealloc((node_t*)file);          //deallocate the file structure node
}



int Read(file_t* file, void* buffer, uint32_t count){
    
    debug_write_string(file->fileName);
    debug_write_dec((uint32_t)buffer);
    debug_write_dec(count);
    
    return 0;
}




directoryStruct_t* Examine(file_t* dir){
    executive->thisTask->dosError = DOS_ERROR_NO_ERROR;
    
    if(!dir->isDIR){
        executive->thisTask->dosError = DOS_ERROR_NOT_A_DIRECTORY;
        return NULL;
    }
    
    //Get the handler
    handler_t* handler = (handler_t*) dir->entry->handler;
    
   // debug_write_string(" Prepare to ReadDIR...");
    directoryStruct_t* ds = handler->ReadDir(dir,dir->startBlock);
    //debug_write_string(" red ");

    return ds;
}




uint8_t* LoadFile(file_t* file){
    
    //Get the handler
    handler_t* handler = (handler_t*) file->entry->handler;
    
    return handler->LoadFileAtCluster(file,file->startBlock);
}


//************************* LOAD RELOCATABLE ELF OBJECTS
executable_t LoadELF(file_t* file){
    executive->thisTask->dosError = DOS_ERROR_NO_ERROR;
    //debug_write_string("loading File\n");
    
    executable_t ret;
    ret.type = 0;   //Not executable
    
    if(file->isDIR){
        executive->thisTask->dosError = DOS_ERROR_OBJECT_NOT_OF_REQUIRED_TYPE;
        return ret;
    }
    
    //Get the handler
    handler_t* handler = (handler_t*) file->entry->handler;
    
    uint8_t* buffer = handler->LoadFileAtCluster(file,file->startBlock);
    
    uint32_t* check = (uint32_t*)buffer;
    
    //is this a valid ELF file?
    if(*check != 1179403647){
        debug_write_string("File Not Executable\n");
        executive->FreeMem(buffer);
        return ret;
    }
    
    if(buffer[4] != 1){
        debug_write_string("Can't Load 64bit files\n");
        executive->FreeMem(buffer);
        return ret;
    }
    
    if(buffer[5] != 1){
        debug_write_string("Can't Load Big Endian files\n");
        executive->FreeMem(buffer);
        return ret;
    }
    
    //It's a vaild ELF
    Elf32_Header_t* header = (Elf32_Header_t*) buffer;
    
    //Check it is for this CPU
    if(header->e_machine != 3){
        debug_write_string("File not x86 executable\n");
        executive->FreeMem(buffer);
        return ret;
    }
  
    //Check it is an executible ELF
    if(header->e_type != 2){
        debug_write_string("File not Executable\n");
        executive->FreeMem(buffer);
        return ret;
    }
    
    
    // the text section always starts at offest 4096, but really we should look at the Program Header Tables to find PT_LOAD segments and load them
    uint32_t codeStart = 4096;
    
    //check for magic number, identifying this as a CuriOS executable
    uint32_t* magic = (uint32_t*)&buffer[codeStart];
    if(*magic != 0x80DECADE){
        debug_write_string("File not a Curios Executable/Library\n");
        executive->FreeMem(buffer);
        return ret;
    }
    
    
    //Fix up the executive pointer
    Elf32_Section_Header_t* sectionTable = (Elf32_Section_Header_t*) &buffer[header->e_shoff];
    
    //String Section
    Elf32_Section_Header_t stringTableHeader = sectionTable[header->e_shstrndx];
    char* strings = (char*) &buffer[stringTableHeader.sh_offset];
    
    //Walk the secton table, first entry is always NULL
    //debug_write_string("Sections:\n");
    for(int i=1; i < header->e_shnum; ++i){
        
        Elf32_Section_Header_t entry = sectionTable[i];
        //debug_write_dec(i);debug_write_string(": ");
        //debug_write_string(&strings[entry.sh_name]); debug_putchar(' ');
        
        //Find the executive section
        if(!strcmp(&strings[entry.sh_name],".executive")){
            //debug_putchar(' ');debug_write_dec(entry.sh_offset);
            
            //Fixup the executive pointer
            uint32_t* exec =  (uint32_t*) &buffer[entry.sh_offset];
            *exec = (uint32_t) executive;
            
        }
        
        //Find the exit section
        if(!strcmp(&strings[entry.sh_name],".exit")){
            //debug_putchar(' ');debug_write_dec(entry.sh_offset);
            
            //Fixup the exit function pointer
            //uint32_t* exit_fn =  (uint32_t*) &buffer[entry.sh_offset];
            //*exit_fn = (uint32_t) ....
            
        }
        
        //debug_putchar('\n');
        
    }
   
    
    
    //Entry function, here DOS should allocate a proger TASK_SEGMENT node and copy the program into it, freeing the loaded ELF file from memory
    ret.type = 1;
    ret.segment = (node_t*)buffer;
    ret.entry = &buffer[4096 + header->e_entry];
    

    
    return ret;
        
    //debug_write_string("ELF Executed.\n");
    
    //executive->FreeMem(buffer);
}


void InitDOS(library_t* library){
    //called by AddLibrary(), does the library set up.
    
    InitList(&dos.dosList);
    library->node.name = "dos.library";
 
    //Handler should already have been loaded and inited by the boot task
    handler_t* fat = executive->OpenHandler("fat.handler",0);
    //debug_write_hex((uint32_t)fat);debug_putchar('\n');
    
    
    //debug_write_string("DOS Library: Setting up hard disk device... dh0: \n");
    
    //Add a FAT file system handler to DOS, which sits on top of the ATA device, first partition.
    node_t* node                    = executive->Alloc(sizeof(dosEntry_t));
    node->type                      = NODE_DOS_ENTRY;
    node->name                      = "dh0";
    dosEntry_t* bootDOSEntry        = (dosEntry_t*)node;
    

    bootDOSEntry->handler           = (library_t*)fat;
    bootDOSEntry->handlerNumber     = 0;
    bootDOSEntry->deviceName        = "ata.device";
    bootDOSEntry->unitNumber        = 0;


    // Use the current task context DOSPort for the IORequest, which should be the boot task at this stage.
    
    //This will be NULL as nothing can have Opened the DOS library yet!
    messagePort_t* dosPort = executive->CreatePort("DOS Port"); // Cannot do ANY DOS operations AT ALL without a DOS PORT! Open Dos Library does this automatically
    executive->thisTask->dosPort = dosPort;
    
    
    
    
    if(fat->Mount(bootDOSEntry)){;
        //the handler didn't mount :-(
        return;
    }
    //if we get this far the dos entry is vaild, so add it to the doslist!
    dos.AddDosEntry(bootDOSEntry);
    
    //set the progdir to the boot disk
    executive->thisTask->progdir = executive->AllocMem(5,0); //need 5 bytes, the name plus null
    strcpy(executive->thisTask->progdir,"dh0:");
    return;
    
    
    
}






library_t* OpenLib(library_t* lib){
    
    lib->openCount +=1;
    
    //initilise the DOS Message port of the calling task
    task_t* task  = executive->thisTask;
    
    //The boot task get a DOS Port as part of the Library Initilisation, but no other task will
    if(task->dosPort == NULL){
        task->dosPort = executive->CreatePort("DOS Port");
    }
    
    return lib; // this probably shouldn't be a singleton... but it is for now
    
}


void CloseLib(library_t* lib){
    //dos.library.openCount -=1;
    lib->openCount -=1;
    
    //remove DOS message port
    task_t* task  = executive->thisTask;
    executive->DeletePort((messagePort_t*)task->dosPort);
    debug_write_string("DOS Closed by ");
    debug_write_string(task->node.name);
    debug_putchar('\n');
}

void AddDosEntry(dosEntry_t* entry){

    //need to setup the requested unit.
    
    AddTail(&dos.dosList,(node_t*) entry);
    
    /*
    ioRequest_t* handlerRequest = executive->CreateIORequest(NULL,sizeof(ioRequest_t));
    uint32_t error = executive->OpenDevice(entry->handlerName,entry->handlerNumber,handlerRequest,0);
    
    if(error == DEVICE_ERROR_DEVICE_NOT_FOUND){
        debug_write_string("Handler Not Found\n");
        return;
    }
    
    //if we get this far we know the fat handler has opened.
    fatHandler_t* handler = (fatHandler_t*)handlerRequest->device;
    handler->SetHandler(entry);
     */
     
}


void LoadDOSLibrary(){
    
    dos.library.node.type       = NODE_LIBRARY;
    dos.library.Init            = InitDOS;
    dos.library.Open            = OpenLib;
    dos.library.Close           = CloseLib;
    dos.AddDosEntry             = AddDosEntry;
    dos.Open                    = Open;
    dos.Close                   = Close;
    dos.Read                    = Read;
    dos.Examine                 = Examine;
    dos.LoadFile                = LoadFile;
    dos.LoadELF                 = LoadELF;
}
