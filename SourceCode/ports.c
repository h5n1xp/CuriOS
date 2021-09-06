//
//  ports.c
//
//  Created by Matt Parsons on 02/11/2020.
//  Copyright © 2020 Matt Parsons. All rights reserved.
//

#include "ports.h"
#include "memory.h"

#include "SystemLog.h"

messagePort_t* CreatePort(char* name){
    int32_t sigNum = executive->AllocSignal(-1);
    
    if(sigNum == -1){
        return NULL;
    }
    
    messagePort_t* port = (messagePort_t*)executive->Alloc(sizeof(messagePort_t),0);
    port->node.name = name;
    port->node.type = NODE_MSGPORT;
    port->owner = executive->thisTask;
    port->sigNum = sigNum;
    executive->InitList(&port->messageList);
    return port;
}

void DeletePort(messagePort_t* port){
    
    //TO DO: NEED TO REPLY TO ALL MESSAGES STILL QUEUED
    //
    //
    executive->FreeSignal(port->sigNum);
    executive->Dealloc((node_t*)port);
    
}



messagePort_t* FindPort(char* name){
    Lock(&executive->portList.lock);
    messagePort_t* port = (messagePort_t*) FindName(&executive->portList,name);
    FreeLock(&executive->portList.lock);
    return port;
}

void* CreateMessage(uint64_t size, messagePort_t* replyPort){
    
    message_t* message = (message_t*)executive->Alloc(size,0);
    message->node.type = NODE_MESSAGE;
    message->replyPort = replyPort;
    
    return message;
}

message_t* GetMessage(messagePort_t* port){
    Lock(&port->messageList.lock);
    message_t* message = (message_t*) RemHead(&port->messageList);
    FreeLock(&port->messageList.lock);
    return message;
}

void PutMessage(messagePort_t* port, message_t* message){

    Lock(&port->messageList.lock);
    AddTail(&port->messageList,(node_t*)message);
    FreeLock(&port->messageList.lock);
    executive->Signal(port->owner,1 << port->sigNum);

}

void ReplyMessage(message_t* message){
    
    messagePort_t* port = message->replyPort;
    
    if(port == NULL){
        executive->Dealloc((node_t*)message);
        return;
    }
    
    PutMessage(port,message);
    
    
}

void AddPort(messagePort_t* port){
    
    Lock(&executive->portList.lock);
    list_t* portList = &executive->portList;
    AddHead(portList,(node_t*)port);
    FreeLock(&executive->portList.lock);
}

void RemovePort(messagePort_t* port){
   
    Lock(&executive->portList.lock);
    list_t* portList = &executive->portList;
    Remove(portList,(node_t*)port);
    FreeLock(&executive->portList.lock);
}

message_t* WaitPort(messagePort_t* port){
    
    executive->Wait( 1 << port->sigNum );
    
    list_t* messageList = &port->messageList;
    
    return (message_t*)messageList->head;
}
