//
//  device.h
//
//  Created by Matt Parsons on 14/11/2020.
//  Copyright © 2020 Matt Parsons. All rights reserved.
//

//The device is the most basic unit of system software, devices expose a standard interface for software
//to use for accessing hardware.
//
//There is another type of device, called a handler, these expose a seprate DOS compatible interface.
//Devices and handlers are otherwise indentical and can expose both types of interface if that makes sense.
//Handlers are generally used as a "middle man" for devices and DOS.

#ifndef device_h
#define device_h

#include "stdheaders.h"
#include "list.h"
#include "library.h"
#include "ports.h"
#include "task.h"

#define DEVICE_NO_ERROR 0
#define DEVICE_ERROR_DEVICE_NOT_FOUND 1
#define DEVICE_ERROR_UNIT_NOT_FOUND 2

//Standard Device commands
#define CMD_INVALID 0
#define CMD_RESET   1
#define CMD_READ    2
#define CMD_WRITE   3
#define CMD_UPDATE  4
#define CMD_CLEAR   5
#define CMD_STOP    6
#define CMD_START   7
#define CMD_FLUSH   8
#define CMD_NONSTD  9

//Handler commands
#define HANDLER_TEST 10

#define IOF_QUICK   1

#define IO_IN_PROGRESS  1
#define IO_NO_ERROR     0
#define IO_ERROR       -1

typedef struct ioRequest_t ioRequest_t;

typedef struct{
    library_t library;
    void (*BeginIO)(ioRequest_t*);
    void (*AbortIO)(ioRequest_t*);
    list_t unitList;
    task_t* task;
    bool isHandler;
} device_t;

typedef struct{
    node_t node;
    messagePort_t* messagePort;
    uint64_t flags;
    uint64_t openCount;
} unit_t;

struct ioRequest_t{
    message_t message;
    device_t* device;
    unit_t*   unit;
    uint64_t command;
    uint64_t flags;
     int64_t error;
    uint64_t offset;
    uint64_t length;
    void* data;
};

#endif /* device_h */
