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

	/* NOT REACHED */
	return 0;
}
