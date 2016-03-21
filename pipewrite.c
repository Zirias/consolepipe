#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>

char buf[1024];
FILE *pipef = 0;

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s /path/to/fifo\n", argv[0]);
        return EXIT_FAILURE;
    }

    struct sigaction sigact;
    memset(&sigact, 0, sizeof(sigact));
    sigact.sa_handler = SIG_IGN;
    sigact.sa_flags = SA_RESTART;
    sigaction(SIGPIPE, &sigact, NULL);

    pipef = fopen(argv[1], "w");

    if (!pipef)
    {
        fprintf(stderr, "Cannot open `%s' for writing: %s\n",
                argv[1], strerror(errno));
        return EXIT_FAILURE;
    }

    while (fgets(buf, 1024, stdin))
    {
	int rc;
	while ((rc = fputs(buf, pipef)) < 0)
	{
	    if (errno == EPIPE)
	    {
		fclose(pipef);
		pipef = fopen(argv[1], "w");

		if (!pipef)
		{
		    fprintf(stderr, "Cannot open `%s' for writing: %s\n",
			    argv[1], strerror(errno));
		    return EXIT_FAILURE;
		}
	    }
	    else
	    {
		fprintf(stderr, "Error writing to `%s': %s\n",
			argv[1], strerror(errno));
		fclose(pipef);
		return EXIT_FAILURE;
	    }
	}
	fflush(pipef);
    }

    fclose(pipef);

    return EXIT_SUCCESS;
}

