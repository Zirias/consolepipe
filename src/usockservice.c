#include "usockservice.h"
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

typedef struct UsockClient UsockClient;

struct UsockClient
{
    UsockClient *next;
    int fd;
};

typedef struct UsockCustomFd UsockCustomFd;

struct UsockCustomFd
{
    UsockCustomFd *next;
    int fd;
};

struct UsockEvent
{
    UsockEventType type;
    int fd;
    fd_set fds;
};

struct UsockService
{
    const char *path;
    int fd;
    UsockClient *clients;
    UsockCustomFd *customFds;
};

UsockService *
UsockService_Create(const char *path)
{
    int fd = socket(PF_UNIX, SOCK_STREAM, 0);
    if (fd < 0) return 0;

    if (unlink(path) < 0 && errno != ENOENT) return 0;

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, path);

    if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) return 0;

    UsockService *self = malloc(sizeof(UsockService));
    self->path = path;
    self->fd = fd;
    self->clients = 0;
    self->customFds = 0;

    return self;
}

void
UsockService_RegisterCustomFd(UsockService *self, int fd)
{
    UsockCustomFd *customFd = malloc(sizeof(UsockCustomFd));
    customFd->next = 0;
    customFd->fd = fd;

    if (self->customFds)
    {
	UsockCustomFd *prev = self->customFds;
	while (prev->next) prev = prev->next;
	prev->next = customFd;
    }
    else
    {
	self->customFds = customFd;
    }
}

void
UsockService_UnregisterCustomFd(UsockService *self, int fd)
{
    UsockCustomFd *prev = 0;
    UsockCustomFd *curr = self->customFds;

    while (curr)
    {
	if (curr->fd == fd)
	{
	    if (prev) prev->next = curr->next;
	    else self->customFds = curr->next;
	    prev = curr->next;
	    free(curr);
	    curr = prev;
	}
	else
	{
	    prev = curr;
	    curr = curr->next;
	}
    }
}

static UsockClient *
UsockService_Accept(UsockService *self)
{
    int fd = accept(self->fd, 0, 0);
    if (fd < 0) return 0;

    UsockClient *client = malloc(sizeof(UsockClient));
    client->next = 0;
    client->fd = fd;

    if (self->clients)
    {
	UsockClient *prev = self->clients;
	while (prev->next) prev = prev->next;
	prev->next = client;
    }
    else
    {
	self->clients = client;
    }

    return client;
}

static void
UsockService_Disconnect(UsockService *self, UsockClient *client)
{
    close(client->fd);
    if (self->clients == client)
    {
	self->clients = client->next;
    }
    else
    {
	UsockClient *prev = self->clients;
	while (prev)
	{
	    if (prev->next == client)
	    {
		prev->next = client->next;
		break;
	    }
	}
    }
    free(client);
}

static void
UsockService_NextEvent(UsockService *self, UsockEvent *ev)
{
    if (FD_ISSET(self->fd, &(ev->fds)))
    {
	FD_CLR(self->fd, &(ev->fds));
	UsockClient *c = UsockService_Accept(self);
	if (c)
	{
	    ev->fd = c->fd;
	    ev->type = UET_ClientConnected;
	    return;
	}
    }

    UsockCustomFd *cfd = self->customFds;
    while (cfd)
    {
	if (FD_ISSET(cfd->fd, &(ev->fds)))
	{
	    FD_CLR(cfd->fd, &(ev->fds));
	    ev->fd = cfd->fd;
	    ev->type = UET_CustomFd;
	    return;
	}
    }

    UsockClient *c = self->clients;
    while (c)
    {
	if (FD_ISSET(c->fd, &(ev->fds)))
	{
	    FD_CLR(c->fd, &(ev->fds));
	    char buf[32];
	    if (recv(c->fd, buf, sizeof(buf), MSG_PEEK|MSG_DONTWAIT) > 0)
	    {
		ev->fd = c->fd;
		ev->type = UET_ClientData;
		return;
	    }
	    else
	    {
		UsockService_Disconnect(self, c);
	    }
	}
    }

    ev->type = UET_None;
}

void
UsockService_PollEvent(UsockService *self, UsockEvent *ev)
{
    int nfds = 0;

    UsockService_NextEvent(self, ev);
    while (ev->type == UET_None)
    {
	FD_ZERO(&(ev->fds));
	FD_SET(self->fd, &(ev->fds));
	nfds = self->fd;

	UsockClient *c = self->clients;
	while (c)
	{
	    FD_SET(c->fd, &(ev->fds));
	    nfds = c->fd > nfds ? c->fd : nfds;
	    c = c->next;
	}

	UsockCustomFd *cfd = self->customFds;
	while (cfd)
	{
	    FD_SET(cfd->fd, &(ev->fds));
	    nfds = cfd->fd > nfds ? cfd->fd : nfds;
	    cfd = cfd->next;
	}

	select(nfds+1, &(ev->fds), 0, 0, 0);
	UsockService_NextEvent(self, ev);
    }
}