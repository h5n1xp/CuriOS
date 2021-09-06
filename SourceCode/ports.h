//
//  ports.h
//
//  Created by Matt Parsons on 02/11/2020.
//  Copyright © 2020 Matt Parsons. All rights reserved.
//

#ifndef ports_h
#define ports_h

#include "stdheaders.h"
#include "list.h"
#include "task.h"

#define MSGPORT_FLAG_SIGNAL    0    // Signal owning task - default action
#define MSGPORT_FLAG_IGNORE    2    // Ignore arrival

typedef struct{
    node_t node;
    task_t* owner;
    uint64_t sigNum;
    list_t messageList;
} messagePort_t;


typedef struct{
    node_t node;
    uint64_t timestamp;
    messagePort_t* replyPort;
} message_t;


messagePort_t* CreatePort(char* name);
void DeletePort(messagePort_t* port);
messagePort_t* FindPort(char* name);    //finds a port in the public list.

message_t* GetMessage(messagePort_t* port); //Must be called before you access a message, as ownership is transferred to you.
void PutMessage(messagePort_t* port, message_t* message);//You on longer own the message once called, so don't access it.
void ReplyMessage(message_t* message);  // If you reply to a message with no reply port it will be deallocated.

void AddPort(messagePort_t* port);      // adds a port the public list
void RemovePort(messagePort_t* port);   // removes a port from the public list.

message_t* WaitPort(messagePort_t* port);   //waits for a message to arrive at a port, you must GetMessage() to access the message.

#endif /* ports_h */
