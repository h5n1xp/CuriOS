//
//  dos.c
//
//  Created by Matt Parsons on 20/11/2020.
//  Copyright © 2020 Matt Parsons. All rights reserved.
//

#include "dos.h"
#include "memory.h"

#include "ata.h"
#include "fat_handler.h"    //DOS calls the FAT functions directly until we have a proper handler
#include "string.h"

#include "SystemLog.h"

dos_t dos;



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
    toLower(dev);

    
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
    file_t* file = (file_t*)executive->Alloc(sizeof(file_t)); //remember to dealloc this at the end of the DOS init
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
    
    //debug_write_string("DOS Library: Reading Root\n\n");
    directoryStruct_t* ds = NULL;
    
    count = 0;
    j = 0;
    uint32_t clusterNumber = 0;
    bool sucess;
    do{
        sucess = false;
        ds = readDir(file,clusterNumber);
        
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


directoryStruct_t* Examine(file_t* dir){
    executive->thisTask->dosError = DOS_ERROR_NO_ERROR;
    
    if(!dir->isDIR){
        executive->thisTask->dosError = DOS_ERROR_NOT_A_DIRECTORY;
        return NULL;
    }
    
    directoryStruct_t* ds = readDir(dir,dir->startBlock);

    return ds;
}


uint8_t* LoadFile(file_t* file){
    return LoadFileAtCluster(file,file->startBlock);
}





void InitDOS(library_t* library){
    //called by AddLibrary(), does the library set up.
    
    InitList(&dos.dosList);
    library->node.name = "dos.library";
 
    //The DOS.library will need to access the disks
    //setup the ata device
    //LoadATADevice();
    //executive->AddDevice((device_t*)&ata);
    
    
    

    
    //The boot disk will be FAT32
    LoadFATHandler();
    executive->AddDevice((device_t*)&fatHandler);
    
    debug_write_string("DOS Library: Setting up hard disk device... dh0: \n");
    
    //Add a FAT file system handler to DOS on top of the ATA device, first partition.
    node_t* node                    = executive->Alloc(sizeof(dosEntry_t));
    node->name                      = "dh0";
    dosEntry_t* bootDOSEntry        = (dosEntry_t*)node;
    bootDOSEntry->handlerName       = "fat.handler";
    bootDOSEntry->handlerNumber     = 0;
    bootDOSEntry->deviceName        = "ata.device";
    bootDOSEntry->unitNumber        = 0;


    // Use the current task context DOSPort for the IORequest, which should be the boot task at this stage.
    
    //This will be NULL as nothing can have Opened the DOS library yet!
    messagePort_t* dosPort = executive->CreatePort("DOS Port"); // Cannot do ANY DOS operations AT ALL without a DOS PORT! Open Dos Library does this automatically
    executive->thisTask->dosPort = dosPort;
    
    
    //need to create a temporary file_t structure as all DOS/Handler operations rely on one.
    file_t* root = (file_t*)executive->Alloc(sizeof(file_t)); //remember to dealloc this at the end of the DOS init
    root->isDIR = true; //Because the root is a directory :-)
    root->request = executive->CreateIORequest(dosPort, sizeof(ioRequest_t));
    root->entry = bootDOSEntry;
    
    // At this point we need to call the Handler's mount function which should perform the setup done below
    // Need to add an Open Handler function to Exec to open a device as a library?
    //
    //
    
    //now we have a valid IO Request, open the ata.device, remember to close the device at the end of the DOS init
    if(executive->OpenDevice(bootDOSEntry->deviceName,bootDOSEntry->unitNumber,root->request,0)){
        debug_write_string("DOS Library: No Hard Disk!!!\n");
        
        //Need to do the Clean DOS init clean up!!!
        executive->Dealloc((node_t*)root->request); //deallocate the IORequest
        executive->Dealloc((node_t*)root);
        
        return;
    }

    //Since the device opened OK, Allocate a one 512byte buffer for the File IORequest
    uint8_t* buffer = executive->AllocMem(512,0);
    root->request->data = buffer;
    
    
    //if we get this far the dos entry is vaild, so add it to the doslist!
    dos.AddDosEntry(bootDOSEntry);

    
    //get location of first partition, which will be our Boot partition
    getPartitionData(root, 0);

    //Examine(root);
    
    //file_t clean up!
    Close(root);
   // executive->CloseDevice(root->request);      //close the ata device
   // executive->FreeMem(root->request->data);    //free the memory used by the IORequest
   // executive->Dealloc((node_t*)root->request); //deallocate the IORequest
   // executive->Dealloc((node_t*)root);          //deallocate the file structure node
}






void OpenLib(library_t* lib){
    //dos.library.openCount +=1;
    lib->openCount +=1;
    
    //initilise the DOS Message port of the calling task
    task_t* task  = executive->thisTask;
    
    //The boot task get a DOS Port as part of the Library Initilisation, but no other task will
    if(task->dosPort == NULL){
        task->dosPort = executive->CreatePort("DOS Port");
    }
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
    
    dos.library.Init    = InitDOS;
    dos.library.Open    = OpenLib;
    dos.library.Close   = CloseLib;
    dos.AddDosEntry     = AddDosEntry;
    dos.Open            = Open;
    dos.Close           = Close;
    dos.Examine         = Examine;
    dos.LoadFile        = LoadFile;
}
