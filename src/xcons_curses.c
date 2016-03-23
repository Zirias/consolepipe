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

static void sighdl(int signum)
{
    (void)signum;
    running = 0;
}

static FILE *
openSocketReader(const char *path)
{
    int fd = socket(PF_UNIX, SOCK_STREAM, 0);
    if (!fd) return 0;

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, path);

    if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) return 0;
    return fdopen(fd, "r");
}

int main(int argc, char **argv)
{
    struct sigaction sigact;
    memset(&sigact, 0, sizeof(sigact));
    sigact.sa_handler = sighdl;
    sigaction(SIGTERM, &sigact, NULL);
    sigaction(SIGINT, &sigact, NULL);

    if (argc != 2)
    {
	fprintf(stderr, "Usage: %s /path/to/fifo\n", argv[0]);
	return EXIT_FAILURE;
    }

    initscr();
    noecho();
    curs_set(0);
    scrollok(stdscr, 1);

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


    FILE *consoleLog;
    int failcount = 0;
    while (running)
    {
        consoleLog = openSocketReader(argv[1]);
	if (!running) break;

        if (!consoleLog)
        {
            if (failcount) --failcount;
            else
            {
                failcount = ERR_SECONDS;
                standout();
                printw("ERROR: cannot open socket `%s' for reading: %s\n",
                        argv[1], strerror(errno));
                standend();
		refresh();
            }
            sleep(1);
	    if (!running) break;
            continue;
        }

	failcount = 0;
        while (fgets(buf, 1024, consoleLog))
        {
	    void *iskern = strstr(buf, " kernel: ");
	    if (iskern) attrset(krnlhl);
            addstr(buf);
	    if (iskern) attrset(A_NORMAL);
            refresh();
        }
        fclose(consoleLog);
    }

    endwin();
    if (consoleLog) fclose(consoleLog);
}

