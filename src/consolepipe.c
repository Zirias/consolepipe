#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <curses.h>

#define ERR_SECONDS 60

char buf[1024];
FILE *pipef = 0;

static void done(void)
{
    endwin();
    if (pipef) fclose(pipef);
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
	fprintf(stderr, "Usage: %s /path/to/fifo\n", argv[0]);
	return EXIT_FAILURE;
    }

    initscr();
    atexit(done);
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


    int failcount = 0;
    while (1)
    {
        pipef = fopen(argv[1], "r");

        if (!pipef)
        {
            if (failcount) --failcount;
            else
            {
                failcount = ERR_SECONDS;
                standout();
                printw("ERROR: cannot open pipe `%s' for reading: %s\n",
                        argv[1], strerror(errno));
                standend();
		refresh();
            }
            sleep(1);
            continue;
        }

	failcount = 0;
        while (fgets(buf, 1024, pipef))
        {
	    int iskern = (int)strstr(buf, " kernel: ");
	    if (iskern) attrset(krnlhl);
            addstr(buf);
	    if (iskern) attrset(A_NORMAL);
            refresh();
        }
        fclose(pipef);
    }
}

