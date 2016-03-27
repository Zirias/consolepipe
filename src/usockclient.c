#include "usockclient.h"
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

struct UsockClient
{
    int fd;
    char *bufptr;
    size_t bufbytes;
    char buf[1024];
};

static UsockClient *
UsockClient_Open(int fd)
{
    UsockClient *self = malloc(sizeof(UsockClient));
    self->fd = fd;
    self->bufptr = self->buf;
    self->bufbytes = 0;
    return self;
}

static size_t
UsockClient_bufcopy(UsockClient *self, char *buf, size_t maxlen)
{
    size_t to_go = self->bufbytes < maxlen ? self->bufbytes : maxlen;
    size_t copied = 0;
    while (to_go)
    {
        char c = *(self->bufptr++);
        --self->bufbytes;
        *buf++ = c;
        --to_go;
        ++copied;
        if (c == '\n') break;
    }
    return copied;
}

UsockClient *
UsockClient_Create(const char *path)
{
    int fd = socket(PF_UNIX, SOCK_STREAM, 0);
    if (!fd) return 0;

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, path);

    if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) return 0;
    return UsockClient_Open(fd);
}

char *
UsockClient_ReadLine(UsockClient *self,
	char *buf, size_t size, sig_atomic_t *running)
{
    if (!size) return 0;
    if (size == 1)
    {
	buf[0] = 0;
	return buf;
    }

    size_t len = 0;
    size_t max = size-1;

    while (1)
    {
        if (self->bufbytes)
        {
            int copied = UsockClient_bufcopy(self, buf+len, max);
            len +=copied;
            max -= copied;
            if (!max || buf[len-1] == '\n')
            {
                buf[len] = 0;
                return buf;
            }
        }

        sigset_t sigmask;
        sigset_t blockall;
        sigfillset(&blockall);
        sigprocmask(SIG_SETMASK, &blockall, &sigmask);
        if (running && !*running)
        {
            sigprocmask(SIG_SETMASK, &sigmask, 0);
            return 0;
        }
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(self->fd, &fds);
        pselect(self->fd+1, &fds, 0, 0, 0, &sigmask);
        sigprocmask(SIG_SETMASK, &sigmask, 0);
        if (running && !*running) return 0;
        self->bufptr = self->buf;
        self->bufbytes = (int) read(self->fd, self->buf, 1024);
	if (self->bufbytes <= 0)
	{
	    self->bufbytes = 0;
	    return 0;
	}
    }
}

void
UsockClient_Destroy(UsockClient *self)
{
    if (!self) return;
    close(self->fd);
    free(self);
}

