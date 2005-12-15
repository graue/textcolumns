/*
This file is public domain; anyone may deal in it without restriction.

columns.c: main program
*/

#include "columns.h"

static char *endmsg = NULL;

static void finish(int sig)
{
	sig = sig;

	(void) curs_set(1); /* visible */
	(void) endwin();

	(void) putchar('\n');
	if (endmsg != NULL)
		(void) printf("%s\n", endmsg);

	exit(0);
}

static void die(char *s)
{
	endmsg = s;
	finish(0);
}

void millisleep(int ms)
{
	struct timespec tsp;
	tsp.tv_sec  = ms / 1000;
	tsp.tv_nsec = (ms % 1000) * 1000000L;
	(void) nanosleep(&tsp, NULL);
}

int main(int argc, char *argv[])
{
	extern char *optarg;
	extern int optind;
	int width = DEF_WIDTH;
	int height = DEF_HEIGHT;
	int ch;
	int warned = 0;

	while ((ch = getopt(argc, argv, "h:w:")) != -1)
	{
		switch (ch)
		{
		case 'h':
			height = atoi(optarg);
			if (height < MIN_HEIGHT)
			{
				(void) printf("Height value too small, "
					"using %d\n", MIN_HEIGHT);
				warned = 1;
				height = MIN_HEIGHT;
			}
			else if (height > MAX_HEIGHT)
			{
				(void) printf("Height value too big, "
					"using %d\n", MAX_HEIGHT);
				warned = 1;
				height = MAX_HEIGHT;
			}
			break;
		case 'w':
			width = atoi(optarg);
			if (width < MIN_WIDTH)
			{
				(void) printf("Width value too small, "
					"using %d\n", MIN_WIDTH);
				warned = 1;
				width = MIN_WIDTH;
			}
			else if (width > MAX_WIDTH)
			{
				(void) printf("Width value too big, "
					"using %d\n", MAX_WIDTH);
				warned = 1;
				width = MAX_WIDTH;
			}
			break;
		case '?':
		default:
			(void) printf("Unrecognized option '%c', ignoring\n",
				ch);
			warned = 1;
		}
	}

	if (warned)
		millisleep(1000);

	(void) signal(SIGINT, finish);

	(void) initscr();
	(void) keypad(stdscr, TRUE);
	(void) nonl();
	(void) noecho();
	(void) cbreak();
	(void) nodelay(stdscr, TRUE);
	(void) curs_set(0); /* invisible cursor */

	/* make sure the chosen width and height aren't too big */
	if (!playsizeok(width, height))
		die("Screen is too small to accommodate the playfield");

	(void) argc;
	(void) argv;

	playgame(width, height);

	finish(0);

	/* NOTREACHED */
	return 0;
}
