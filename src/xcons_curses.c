#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <ctype.h>
#include <curses.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#define ERR_SECONDS 60

char buf[1024];
sig_atomic_t running = 1;

typedef struct SocketFile
{
    int fd;
    char *bufptr;
    int bufbytes;
    char buf[1024];
} SocketFile;

static SocketFile *
SocketFile_Open(int fd)
{
    SocketFile *self = malloc(sizeof(SocketFile));
    self->fd = fd;
    self->bufptr = self->buf;
    self->bufbytes = 0;
    return self;
}

static void
SocketFile_Close(SocketFile *self)
{
    if (!self) return;
    close(self->fd);
    free(self);
}

static void sighdl(int signum)
{
    if (signum != SIGALRM)
    {
        running = 0;
    }
}

static SocketFile *
openSocketReader(const char *path)
{
    int fd = socket(PF_UNIX, SOCK_STREAM, 0);
    if (!fd) return 0;

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, path);

    if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) return 0;
    return SocketFile_Open(fd);
}

static int
_readLineIntr_bufcopy(char *s, int maxlen, SocketFile *sock)
{
    int to_go = sock->bufbytes < maxlen ? sock->bufbytes : maxlen;
    int copied = 0;
    while (to_go)
    {
        char c = *(sock->bufptr++);
        --sock->bufbytes;
        *s++ = c;
        --to_go;
        ++copied;
        if (c == '\n') break;
    }
    return copied;
}

static char *
readLineIntr(char *s, int size, SocketFile *sock, sig_atomic_t *running)
{
    int len = 0;
    int max = size-1;

    while (1)
    {
        if (sock->bufbytes)
        {
            int copied = _readLineIntr_bufcopy(s+len, max, sock);
            len +=copied;
            max -= copied;
            if (!max || s[len-1] == '\n')
            {
                s[len] = 0;
                return s;
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
        FD_SET(sock->fd, &fds);
        pselect(sock->fd+1, &fds, 0, 0, 0, &sigmask);
        sigprocmask(SIG_SETMASK, &sigmask, 0);
        if (running && !*running) return 0;
        sock->bufptr = sock->buf;
        sock->bufbytes = (int) read(sock->fd, sock->buf, 1024);
	if (sock->bufbytes <= 0)
	{
	    sock->bufbytes = 0;
	    return 0;
	}
    }
}

int main(int argc, char **argv)
{
    struct sigaction sigact;
    memset(&sigact, 0, sizeof(sigact));
    sigact.sa_handler = sighdl;
    sigaction(SIGHUP, &sigact, NULL);
    sigaction(SIGINT, &sigact, NULL);
    sigaction(SIGPIPE, &sigact, NULL);
    sigaction(SIGALRM, &sigact, NULL);
    sigaction(SIGTERM, &sigact, NULL);

    const char *sockpath;
    if (argc > 2)
    {
        fprintf(stderr, "Usage: %s [/path/to/socket]\n", argv[0]);
        return EXIT_FAILURE;
    }
    else if (argc == 2)
    {
	sockpath = argv[1];
    }
    else
    {
	sockpath = runstatedir "/xcons.sock";
    }

    initscr();
    noecho();
    cbreak();
    curs_set(0);
    timeout(0);
    scrollok(stdscr, 1);
    clear();
    refresh();

    int krnlhl = A_BOLD;

    if (has_colors())
    {
        start_color();
        if (use_default_colors() == ERR)
        {
            init_pair(1, COLOR_BLACK, COLOR_WHITE);
        }
        else
        {
            init_pair(1, COLOR_BLACK, -1);
        }
        krnlhl |= COLOR_PAIR(1);
    }


    SocketFile *consoleLog;
    int failcount = 0;
    while (running)
    {
        consoleLog = openSocketReader(sockpath);
        if (!running) break;

        if (!consoleLog)
        {
            if (failcount) --failcount;
            else
            {
                failcount = ERR_SECONDS;
                standout();
                printw("ERROR: cannot open socket `%s' for reading: %s\n",
                        sockpath, strerror(errno));
                standend();
                refresh();
            }
            sleep(1);
            if (!running) break;
            continue;
        }

        failcount = 0;
        while (readLineIntr(buf, 1024, consoleLog, &running))
        {
            void *iskern = strstr(buf, " kernel: ");
            if (iskern) attrset(krnlhl);
            addstr(buf);
            if (iskern) attrset(A_NORMAL);
            refresh();
            while (getch() != ERR);
        }
        SocketFile_Close(consoleLog);
        consoleLog = 0;
    }

    while (getch() != ERR);
    endwin();
    if (consoleLog) SocketFile_Close(consoleLog);
}

