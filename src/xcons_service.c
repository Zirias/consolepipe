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
    (void)signum;
    running = 0;
}

int main(int argc, char **argv)
{
    struct sigaction sigact;
    memset(&sigact, 0, sizeof(sigact));
    sigact.sa_handler = sighdl;
    sigaction(SIGPIPE, &sigact, NULL);
    sigaction(SIGTERM, &sigact, NULL);
    sigaction(SIGINT, &sigact, NULL);

    int rc = EXIT_SUCCESS;

    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s /path/to/socket\n", argv[0]);
        return EXIT_FAILURE;
    }

    UsockService *usock = UsockService_Create(argv[1]);
    if (!usock)
    {
        fprintf(stderr, "Cannot open socket `%s': %s\n",
                argv[1], strerror(errno));
        return EXIT_FAILURE;
    }

    UsockService_RegisterCustomFd(usock, STDIN_FILENO);

    UsockEvent *ev = UsockEvent_Create();
    while (running)
    {
	UsockService_PollEvent(usock, ev);
	if (running && UsockEvent_Type(ev) == UET_CustomFd)
	{
	    if (fgets(buf, 1024, stdin))
	    {
		UsockService_Broadcast(usock, buf, strlen(buf));
	    }
	}
    }

cleanup:
    UsockEvent_Destroy(ev);
    UsockService_Destroy(usock);
    return rc;
}

