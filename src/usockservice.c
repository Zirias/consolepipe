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

    return self;
}

void
UsockService_PollEvent(UsockService *self, UsockEvent *ev)
{
    fd_set rfds;
    int nfds = 0;

    FD_ZERO(&rfds);
    FD_SET(self->fd, &rfds);
    nfds = self->fd;

    UsockClient *c = self->clients;
    while (c)
    {
	FD_SET(c->fd, &rfds);
	nfds = c->fd > nfds ? c->fd : nfds;
	c = c->next;
    }

    select(nfds+1, &rfds, 0, 0, 0);

    if (FD_ISSET(self->fd, &rfds))
    {
	ev->type = UET_ClientConnected;
    }
}
