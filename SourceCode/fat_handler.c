//
//  fat_handler.c
//
//  Created by Matt Parsons on 27/11/2020.
//  Copyright © 2020 Matt Parsons. All rights reserved.
//

#include "fat_handler.h"
#include "memory.h"

#include "SystemLog.h"


typedef struct{
    uint8_t boot_flag;
    uint8_t starting_head;
    uint16_t starting_sector;
    uint8_t system_id;
    uint8_t ending_head;
    uint16_t ending_sector;
    uint32_t starting_block;
    uint32_t total_blocks;
} mbr_partition_entry;

typedef struct{
    mbr_partition_entry first;
    mbr_partition_entry second;
    mbr_partition_entry third;
    mbr_partition_entry fourth;
} mbr_partition_table;


struct volume_boot_record{
    uint8_t  start[3];
    uint8_t  identifier[8];
    uint16_t bytes_per_sector;
    uint8_t  sectors_per_cluster;
    uint16_t reserved_sectors;
    uint8_t  number_of_file_allocation_tables;
    uint16_t number_of_directory_entries;
    uint16_t sector_count;
    uint8_t  media_descriptor_type;
    uint16_t sectors_per_fat_old;
    uint16_t sectors_per_track;
    uint16_t heads_per_media;
    uint32_t starting_block;
    uint32_t large_sector_count;
} __attribute__((packed));
typedef struct volume_boot_record volume_boot_record_t;


struct volume_boot_record_ext16{
    uint8_t drive_number;
    uint8_t NT_flags;
    uint8_t signature;          //if this is 0x28 0r 0x29 then we have ex16
    uint32_t VolumeID;
    uint8_t volume_label[11];
    uint8_t FAT32[8];           // always FAT32, nothing depends upon this
    uint8_t boot_code[448];
    uint16_t bootable_signature;
} __attribute__((packed));

typedef struct volume_boot_record_ext16 vbr_ext16_t;



struct volume_boot_record_ext32{
    uint32_t sectors_per_fat;
    uint16_t flags;
    uint16_t FAT_version;
    uint32_t cluster_number_of_root_directory;
    uint16_t sector_number_of_FSInfo;
    uint16_t sector_number_of_backup_boot_sector;
    uint8_t  reserved[12];
    uint8_t  drive_number;
    uint8_t  NT_flags;
    uint8_t  signature;         //if this has the value 0x28 or 0x29 we have a FAT32 volume.
    uint32_t volume_id;
    uint8_t  volume_label[11];
    uint8_t  FAT32[8];          //Always contains the text FAT32, nothing depends upon this fact.
    uint8_t  boot_code[420];    //Of no use to any OS other than MSDos, I guess
    uint16_t bootable_signature;
} __attribute__((packed));

typedef struct volume_boot_record_ext32 vbr_ext32_t;


struct dirEntry{
    uint8_t entry[32];
} __attribute__((packed));

typedef struct dirEntry dirEntry_t;


fatHandler_t fatHandler;




void readBlock(file_t* file, uint32_t block){
    
    dosEntry_t* entry = file->entry;
    
    ioRequest_t* request = file->request;
    messagePort_t* port = request->message.replyPort;
    
    request->command = CMD_READ;
    request->offset = (entry->startBlock + block)*512;
    
    
    //debug_write_string("FAT Handler: readBlock() - Preparing to request data from ATA Device!\n");
    //debug_write_hex((uint32_t)request->data);debug_putchar('\n');
    //debug_write_hex((uint32_t)request->offset);debug_putchar('\n');
    //debug_write_hex((uint32_t)request->length);debug_putchar('\n');
    executive->SendIO(request);
    //debug_write_string("FAT Handler: readBlock() - Request sent!\n");
    executive->WaitPort(port);
    //debug_write_string("FAT Handler: readBlock() - response recived!\n");
    executive->GetMessage(port);    //must GetMessage() to access it;
    //debug_write_string("FAT Handler: readBlock() - Got message!\n");
}

uint32_t FATGetClusterLBA(file_t* file,uint32_t cluster){
        
    dosEntry_t* entry = file->entry;
    
    uint32_t block = cluster / 128;     // get the block where the FAT entry we are interestd in is on.
    cluster = cluster - (block * 128);  //calculate correct offset into the block
    
    //debug_write_string("FAT Handler: Get Cluster at LBA! ");
    //debug_write_string(entry->deviceName);
    //debug_putchar('\n');
    
    readBlock(file,entry->FAT + block);
    
    uint32_t* table = file->request->data;
    
    return (table[cluster] & 0x0FFFFFFF);
}

/*
uint32_t FileSizeAtCluster(dosEntry_t* entry,uint32_t cluster){
    return 0;
}
*/
 
uint8_t* LoadFileAtCluster(file_t* file,uint32_t cluster){
   
   // debug_write_string("FAT Handler: load file at cluster\n");
    
    dosEntry_t* entry = file->entry;
    
    //First 2 entries are always Junk
    if(cluster<2){cluster=2;}
    
   // debug_write_string("FAT: ");
    
    uint32_t oldCluster = cluster;
    
    //countblocks
    uint32_t count = 0;
    do{
        count++;
        cluster = FATGetClusterLBA(file,cluster);
        //debug_write_dec(cluster);debug_putchar('\n');
    }while(cluster < 0x0FFFFFF8);
    

    
    //debug_write_string("Size: ");debug_write_dec(count);debug_putchar('\n');
    
    //int size = count;
    uint8_t* data = executive->AllocMem(count*512,0);

    
    cluster = oldCluster;
    count = 0;
    
    do{
        oldCluster = cluster;
        cluster = FATGetClusterLBA(file,oldCluster);
        //debug_write_dec(count);debug_putchar('|');debug_write_dec(oldCluster);debug_putchar('\n');
        
        readBlock(file,entry->dataArea + oldCluster);
        
        uint8_t* t = file->request->data;
        for(int i = 0; i<512;++i){
            data[i + count] = t[i];
        }
        
        count += 512;
    }while(cluster < 0x0FFFFFF8);
    
    
    /*
    for(int i = 0; i <(512*size); ++i){
        debug_putchar(data[i]);
    }
    
    debug_putchar('\n');//debug_putchar('\n');
    */
    
    return data;
}

directoryStruct_t* readDir(file_t* file, uint32_t cluster){

    //debug_write_string("FAT Handler: reading Dir\n");
    //dosEntry_t* entry = file->entry;
    
    dirEntry_t* dirTable = (dirEntry_t*) LoadFileAtCluster(file,cluster);
    //debug_write_string("FAT Handler: file at cluster loaded\n");

    
    //Count directory entries
    int i = 0;
    int count = 0;
    while(dirTable[i].entry[0] != 0){
                
        /*
        if( (dirTable[i].entry[11] & 0x02) == 0x02 ){ //Hidden... actually we want hidden files
          ++i;
          continue;
        }
        */
         
        if( (dirTable[i].entry[11] & 0x04) == 0x04 ){ //System
            ++i;
            //debug_write_string("                       <- Skip: System\n");
            continue;
        }
        
        if( (dirTable[i].entry[11] & 0x08) == 0x08 ){ // Volume
            ++i;
           // debug_write_string("                       <- Skip: Volume\n");
            continue;
        }
        
        if( (dirTable[i].entry[11] & 0x0F) == 0x0F ){    // Long File Name entry
            ++i;
            //debug_write_string("                       <- Skip: LFN Entry\n");
            continue;
        }
        
        if( dirTable[i].entry[0] == 0xE5 ){
            ++i;
            //debug_write_string("                       <- Skip: Deleted\n");
            continue;
        }
        
        /*
        for(int j=0;j<8;j++){
            if(dirTable[i].entry[j] !=0 && dirTable[i].entry[j] !=0x20){
                debug_putchar((char)dirTable[i].entry[j]);
            }
        }
        */
         
        if( dirTable[i].entry[0] == 0xE5 ){
            //debug_write_string("                       <- Skip: Deleted");
        }
        
        if( (dirTable[i].entry[11] & 0x02) == 0x02 ){ //Hidden... actually we want hidden files
            //debug_write_string("                       <- Is Hidden");
        }
        
       //debug_putchar('\n');
        
        ++count;
        ++i;
        
    };
    
    //debug_write_dec(count);debug_putchar('\n');
    
    
    
    //Allocate enough memory for the directory
    directoryStruct_t* retStruct = (directoryStruct_t*)executive->AllocMem( (sizeof(directoryEntry_t)*count)+(sizeof(directoryStruct_t)),0);
    retStruct->size = count;
    retStruct->entry = &retStruct->data;
    directoryEntry_t* directory = retStruct->entry;
    

    
    //Populate directory entries
    i = 0;
    int dirEntryCount = 0;
    char LFNCount = 0;
    
    
    while(dirTable[i].entry[0] != 0){
        
        //Entry is to be ignored
        if( dirTable[i].entry[0] == 0xE5 ){
            ++i;
            continue;
        }
        
        //if 0xF, then this is a Long File name entry
        if( dirTable[i].entry[11] == 0x0F ){
            LFNCount += 1;
            uint32_t seq = ((dirTable[i].entry[0] & 0xF) - 1) * 13;
            
            directory[dirEntryCount].name[seq + 0] = dirTable[i].entry[1];
            directory[dirEntryCount].name[seq + 1] = dirTable[i].entry[3];
            directory[dirEntryCount].name[seq + 2] = dirTable[i].entry[5];
            directory[dirEntryCount].name[seq + 3] = dirTable[i].entry[7];
            directory[dirEntryCount].name[seq + 4] = dirTable[i].entry[9];
            directory[dirEntryCount].name[seq + 5] = dirTable[i].entry[14];
            directory[dirEntryCount].name[seq + 6] = dirTable[i].entry[16];
            directory[dirEntryCount].name[seq + 7] = dirTable[i].entry[18];
            directory[dirEntryCount].name[seq + 8] = dirTable[i].entry[20];
            directory[dirEntryCount].name[seq + 9] = dirTable[i].entry[22];
            directory[dirEntryCount].name[seq + 10] = dirTable[i].entry[24];
            directory[dirEntryCount].name[seq + 11] = dirTable[i].entry[28];
            directory[dirEntryCount].name[seq + 12] = dirTable[i].entry[30];
            
            
        }else{
            
            // no long file name?
            if(LFNCount == 0){
                
                //Copy in the dos name and convert to lower case
                uint32_t nameIndex = 0;
                for(int j=0;j<8;++j){
                    if(dirTable[i].entry[j]==0 || dirTable[i].entry[j]==0x20){
                        break;}
                    directory[dirEntryCount].name[j] = (dirTable[i].entry[j] | 0x20);    // set the lower case bit
                    nameIndex += 1;
                }
                


                directory[dirEntryCount].name[nameIndex] = '.';

                
                for(int j=8;j<11;++j){
                    if(dirTable[i].entry[j]==0 || dirTable[i].entry[j]==0x20){
                        nameIndex -= 1;
                        break;}
                    nameIndex += 1;
                    directory[dirEntryCount].name[nameIndex] = (dirTable[i].entry[j] | 0x20);    // set the lower case bit
                }
                
                directory[dirEntryCount].name[nameIndex+1] = 0;
                                                        
            }
            
            //Create a Directory Entry!
            
            uint16_t clusterHi = *(uint16_t*)&dirTable[i].entry[20];
            uint16_t clusterLo = *(uint16_t*)&dirTable[i].entry[26];
            
            directory[dirEntryCount].cluster = (clusterHi << 16) | clusterLo;
            //flags;    // Need to be filled out as this is all that is provided to dos... but not used right now
            directory[dirEntryCount].fileSize = *(uint32_t*)&dirTable[i].entry[28];
            
            if( (dirTable[i].entry[11] & 0x10) == 0x10 ){
                directory[dirEntryCount].isDir = true;
            }else{
                directory[dirEntryCount].isDir = false;
            }
            
            if( (dirTable[i].entry[11] & 0x02) == 0x02 ){
                directory[dirEntryCount].isHidden = true;
            }else{
                directory[dirEntryCount].isHidden = false;
            }
            
            //FAT specific dirs need to be hidden
            if(directory[dirEntryCount].name[0]=='.' && directory[dirEntryCount].name[1]==0){
                directory[dirEntryCount].isHidden = true;
            }
            
            if(directory[dirEntryCount].name[0]=='.' && directory[dirEntryCount].name[1]=='.' && directory[dirEntryCount].name[2]==0){
                directory[dirEntryCount].isHidden = true;
            }
            
           // debug_write_string(directory[dirEntryCount].name); debug_putchar('\n');
            
            LFNCount = 0;
            
            ++dirEntryCount;
        }
        
        ++i;
    }


    executive->FreeMem(dirTable); // don't need the FAT32 dir table copy anymore
    //debug_putchar('\n');//debug_putchar('\n');
    
    return retStruct;
    
}

void getPartitionData(file_t* file, int partitionNumber){

    dosEntry_t* entry = file->entry;
    
    ioRequest_t* req = file->request;
    
    req->command = CMD_READ;
    req->offset = 0;
    req->length = 512;

    messagePort_t* port = file->request->message.replyPort;
        
    executive->SendIO(req);             //Send the request
    //debug_write_string("FAT Hander: Sent\n");
    executive->WaitPort(port);      //wait for the response
    //debug_write_string("FAT Hander: Recevied\n");
    executive->GetMessage(port);    //must GetMessage() to access it;
    //debug_write_string("FAT Hander: Got\n");
    
    //debug_write_string("FAT Hander: First ATA Read OK!\n");
    
    //Partition Table - only work with the first partition for now
    uint8_t* buffer = req->data;
    mbr_partition_table* table = (mbr_partition_table*) &buffer[446];
    
    switch(partitionNumber){
        case 0: entry->startBlock  = table->first.starting_block;
                entry->totalBlocks = table->first.total_blocks;
            break;
        case 1: entry->startBlock  = table->second.starting_block;
                entry->totalBlocks = table->second.total_blocks;
            break;
        case 2: entry->startBlock  = table->third.starting_block;
                entry->totalBlocks = table->third.total_blocks;
            break;
        case 3: entry->startBlock  = table->fourth.starting_block;
                entry->totalBlocks = table->fourth.total_blocks;
            break;
        default:
            break;
    }
    
    getVolumeBootRecord(file);
    
    //readDir(entry,6);
    //readDir(entry,7);
    //readDir(entry,331);
    
}

void getVolumeBootRecord(file_t* file){
    
    dosEntry_t* entry = file->entry;
    ioRequest_t* request = file->request;
    
    //set up request to read the Volume boot record
    readBlock(file,0);
    
    //Get Volume Boot Rrecord, and extract the information we need
    uint8_t* buffer = (uint8_t*)request->data;
    volume_boot_record_t* vbr = (volume_boot_record_t*)buffer;
    
    entry->sectorSize = vbr->bytes_per_sector;
    entry->sectorsPerCluster = vbr->sectors_per_cluster;
    entry->FAT = vbr->reserved_sectors;
    
    vbr_ext16_t* vbr16 = (vbr_ext16_t*)&buffer[36];
    vbr_ext32_t* vbr32 = (vbr_ext32_t*)&buffer[36];
    if( (vbr16->signature == 0x28) || (vbr16->signature == 0x29) ){
        entry->sectorsPerFAT = vbr->sectors_per_fat_old;
        entry->rootBlock = 2 * entry->sectorsPerCluster;
        entry->dosType = 0xF16;
        //debug_write_string("FAT16\n");
    }else{
        entry->sectorsPerFAT = vbr32->sectors_per_fat;
        //entry->rootBlock  = vbr32->cluster_number_of_root_directory;
        entry->dosType = 0xF32;
        
        entry->dataArea   =  vbr->reserved_sectors + (vbr->number_of_file_allocation_tables * vbr32->sectors_per_fat);
        entry->dataArea  -= (vbr->sectors_per_cluster * 2);    //offset the data area pointer, so there is a 1:1 FAT to cluster relationship
        entry->rootBlock  = entry->dataArea + 2; //First 2 data clusters are unused with FAT
        debug_write_string("FAT Handler: FAT32\n");
    }
    
    //debug_write_string("Starting block: ");debug_write_dec(entry->startBlock);
    //debug_write_string("\nSize: ");debug_write_hex(entry->totalBlocks);
    //debug_write_string("\nBytes Per Sector: ");debug_write_dec(entry->sectorSize);
    //debug_write_string("\nSectors Per cluster: ");debug_write_dec(entry->sectorsPerCluster);
    //debug_write_string("\nRoot cluster: ");debug_write_dec(entry->rootBlock);
    //debug_write_string("\nDevice: ");debug_write_string(entry->deviceName); debug_write_string(" Opened by FAT Handler\n");
    //
    //debug_write_string("\nDataArea: ");debug_write_dec(entry->dataArea); debug_write_string("\n");
    
}


void FATInit(){
    // called by AddDevice();
    fatHandler.isMounted = false;
    fatHandler.device.library.node.name = "fat.handler";

    }

void FATOpen(library_t* lib){
    lib->openCount += 1;
}

int MountHandler(dosEntry_t* entry){

    entry->isAlias = false;
    fatHandler.isMounted = true;
    return 0;
}

void UnmountHandler(dosEntry_t* entry){

    entry->isAlias = false;
    fatHandler.isMounted = true;
    
}


void LoadFATHandler(){
    fatHandler.device.library.Init  = FATInit;
    fatHandler.device.library.Open  = FATOpen;
    fatHandler.device.isHandler     = true;
    fatHandler.Mount                = MountHandler;
    fatHandler.Unmount              = UnmountHandler;
}
