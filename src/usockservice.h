#ifndef USOCK_SERVICE_H
#define USOCK_SERVICE_H

#include <stdlib.h>

typedef enum UsockEventType
{
    UET_None,
    UET_CustomFd,
    UET_ClientConnected,
    UET_ClientData
} UsockEventType;

typedef struct UsockService UsockService;
typedef struct UsockEvent UsockEvent;

UsockService *UsockService_Create(const char *path);
void UsockService_PollEvent(UsockService *self, UsockEvent *ev);
void UsockService_Broadcast(UsockService *self, const char *buf, size_t length);
void UsockService_Destroy(UsockService *self);

UsockEvent *UsockEvent_Create(void);
UsockEventType *UsockEvent_Type(const UsockEvent *self);
int UsockEvent_Fd(const UsockEvent *self);
void UsockEvent_Destroy(UsockEvent *self);

#endif
