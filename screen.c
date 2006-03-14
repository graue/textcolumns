/*
This file is public domain; anyone may deal in it without restriction.

screen.c: drawing
*/

#include "columns.h"

#define DOUBLEWIDTH

static int drawwidth;
static int drawheight;

static int drawleft;
static int drawtop;

static void drawhorizline(int row, int colstart, int colend)
{
	int i;

	(void)move(row, colstart);
	for (i = colstart; i <= colend; i++)
		(void)addch('-');
}

static void drawvertline(int rowstart, int rowend, int col)
{
	int i;

	for (i = rowstart; i <= rowend; i++)
		(void)mvaddch(i, col, '|');
}

int playsizeok(int width, int height)
{
#ifdef DOUBLEWIDTH
	width *= 2;
#endif
	if (width + (2+PANEL_WIDTH)*2 > COLS)
		return 0;
	if (height + 2 > LINES)
		return 0;
	return 1;
}

void drawborders(int width, int height)
{
	drawwidth  = width;
	drawheight = height;

#ifdef DOUBLEWIDTH
	drawwidth  *= 2;
#endif

	/* center the playfield */
	drawleft = (COLS  - drawwidth)  / 2;
	drawtop  = (LINES - drawheight) / 2;

	/* clear the screen */
	(void)erase();

	/* draw the horizontal borders */
	drawhorizline(drawtop - 1,          drawleft, drawleft + drawwidth - 1);
	drawhorizline(drawtop + drawheight, drawleft, drawleft + drawwidth - 1);

	/* draw the vertical borders */
	drawvertline(drawtop, drawtop + drawheight - 1, drawleft - 1);
	drawvertline(drawtop, drawtop + drawheight - 1, drawleft + drawwidth);
}

void drawblock(int row, int col, chtype ch)
{
#ifdef DOUBLEWIDTH
	col *= 2;
#endif
	(void)mvaddch(row + drawtop, col + drawleft, ch);
#ifdef DOUBLEWIDTH
	(void)addch(ch);
#endif
}

void drawlevel(int level)
{
	char buf[200] = "";
	int startcol;

	(void)snprintf(buf, 200, "Level %d", level);

	startcol = drawleft + drawwidth + 2;
	startcol += (PANEL_WIDTH - (int)strlen(buf)) / 2;

	(void)mvaddstr(drawtop + 1, startcol, buf);
}

void drawscore(int score)
{
	char buf[200] = "";
	int startcol;

	(void)snprintf(buf, 200, "%d", score);

	startcol = drawleft - 2 - PANEL_WIDTH;
	startcol += (PANEL_WIDTH - (int)strlen(buf)) / 2;

	(void)mvaddstr(drawtop + 1, startcol, buf);
}

void updatescreen(void)
{
	(void)refresh();
}
