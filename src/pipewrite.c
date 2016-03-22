#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include "usockservice.h"

char buf[1024];
FILE *pipef = 0;

int main(int argc, char **argv)
{
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

    while (1)
    {
	UsockService_PollEvent(usock, ev);
	if (UsockEvent_Type(ev) == UET_CustomFd)
	{
	    if (!fgets(buf, 1024, stdin))
	    {
		fprintf(stderr, "Error reading input: %s",
			strerror(errno));
		return EXIT_FAILURE;
	    }
	    UsockService_Broadcast(usock, buf, strlen(buf));
	}
    }

    return EXIT_SUCCESS;
}

