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

	move(row, colstart);
	for (i = colstart; i <= colend; i++)
		addch('-');
}

static void drawvertline(int rowstart, int rowend, int col)
{
	int i;

	for (i = rowstart; i <= rowend; i++)
		mvaddch(i, col, '|');
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
	erase();

	/* draw the horizontal borders */
	drawhorizline(drawtop - 1,          drawleft, drawleft + drawwidth - 1);
	drawhorizline(drawtop + drawheight, drawleft, drawleft + drawwidth - 1);

	/* draw the vertical borders */
	drawvertline(drawtop, drawtop + drawheight - 1, drawleft - 1);
	drawvertline(drawtop, drawtop + drawheight - 1, drawleft + drawwidth);
}

void drawblock(int row, int col, char ch)
{
#ifdef DOUBLEWIDTH
	col *= 2;
#endif
	mvaddch(row + drawtop, col + drawleft, ch);
#ifdef DOUBLEWIDTH
	addch(ch);
#endif
}

void drawlevel(int level)
{
	char buf[200] = "";
	int startcol;

	snprintf(buf, 200, "Level %d", level);

	startcol = drawleft + drawwidth + 2;
	startcol += (PANEL_WIDTH - strlen(buf)) / 2;

	mvaddstr(drawtop + 1, startcol, buf);
}

void drawscore(int score)
{
	char buf[200] = "";
	int startcol;

	snprintf(buf, 200, "%d", score);

	startcol = drawleft - 2 - PANEL_WIDTH;
	startcol += (PANEL_WIDTH - strlen(buf)) / 2;

	mvaddstr(drawtop + 1, startcol, buf);
}

void updatescreen(void)
{
	refresh();
}
