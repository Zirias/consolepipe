#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include "usockservice.h"

char buf[1024];
sig_atomic_t running = 1;

static void sighdl(int signum)
{
    if (signum != SIGALRM)
    {
        running = 0;
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

    UsockService *usock = UsockService_Create(sockpath);
    if (!usock)
    {
        fprintf(stderr, "Cannot open socket `%s': %s\n",
                sockpath, strerror(errno));
        return EXIT_FAILURE;
    }

    UsockService_RegisterCustomFd(usock, STDIN_FILENO);

    UsockEvent *ev = UsockEvent_Create();
    while (running)
    {
        UsockService_PollEvent(usock, ev, &running);
        if (running && UsockEvent_Type(ev) == UET_CustomFd)
        {
            if (fgets(buf, 1024, stdin))
            {
                UsockService_Broadcast(usock, buf, strlen(buf));
            }
            else break;
        }
    }

    UsockEvent_Destroy(ev);
    UsockService_Destroy(usock);
    return EXIT_SUCCESS;
}

