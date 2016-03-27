#ifndef USOCK_CLIENT_H
#define USOCK_CLIENT_H

#include <stdlib.h>
#include <signal.h>

typedef struct UsockClient UsockClient;

UsockClient *UsockClient_Create(const char *path);
char *UsockClient_ReadLine(UsockClient *self,
	char *buf, size_t size, sig_atomic_t *running);
void UsockClient_Destroy(UsockClient *self);

#endif
