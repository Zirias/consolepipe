#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <ctype.h>
#include <curses.h>
#include "usockclient.h"

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


    UsockClient *consoleLog;
    int lasterr = 0;
    while (running)
    {
        consoleLog = UsockClient_Create(sockpath);
        if (!running) break;

        if (!consoleLog)
        {
	    if (errno != lasterr)
	    {
		lasterr = errno;
		standout();
		printw("[xcons] error connecting to `%s': %s\n",
			sockpath, strerror(lasterr));
		standend();
		refresh();
	    }
            sleep(1);
            if (!running) break;
            continue;
        }

	if (lasterr)
	{
	    lasterr = 0;
	    printw("[xcons] (re-)connected to `%s'.\n", sockpath);
	    refresh();
	}

        while (UsockClient_ReadLine(consoleLog, buf, 1024, &running))
        {
            void *iskern = strstr(buf, " kernel: ");
            if (iskern) attrset(krnlhl);
            addstr(buf);
            if (iskern) attrset(A_NORMAL);
            refresh();
            while (getch() != ERR);
        }
        UsockClient_Destroy(consoleLog);
        consoleLog = 0;
	printw("[xcons] connection to `%s' lost.\n", sockpath);
	refresh();
    }

    while (getch() != ERR);
    endwin();
    if (consoleLog) UsockClient_Destroy(consoleLog);
}

