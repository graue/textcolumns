/*
This file is public domain; anyone may deal in it without restriction.

columns.c: main program
*/

#include "columns.h"

static char *endmsg = NULL;

static void finish(int sig)
{
	sig = sig;

	curs_set(1); /* visible */
	endwin();

	putchar('\n');
	if (endmsg != NULL)
		printf("%s\n", endmsg);

	exit(0);
}

#ifdef UNUSED
static void die(char *s)
{
	endmsg = s;
	finish(0);
}
#endif

void millisleep(int ms)
{
	struct timespec tsp;
	tsp.tv_sec  = ms / 1000;
	tsp.tv_nsec = (ms % 1000) * 1000000L;
	nanosleep(&tsp, NULL);
}

int main(int argc, char *argv[])
{
	signal(SIGINT, finish);

	initscr();
	keypad(stdscr, TRUE);
	nonl();
	noecho();
	cbreak();
	nodelay(stdscr, TRUE);
	curs_set(0); /* invisible cursor */

	(void) argc;
	(void) argv;

	playgame(DEF_WIDTH, DEF_HEIGHT);

	finish(0);

	/* NOTREACHED */
	return 0;
}
