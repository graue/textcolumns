/*
This file is public domain; anyone may deal in it without restriction.

game.c: playing the game
*/

#include "columns.h"

static gamestate_t state;

static int width;
static int height;

static char playfield [MAX_HEIGHT+3][MAX_WIDTH];
static char blinking  [MAX_HEIGHT+3][MAX_WIDTH];
static char cleanblock[MAX_HEIGHT+3][MAX_WIDTH];

static int fallcol     = 0;  /* column of the 1x3 falling blocks */
static int fallrow     = 0; /* top row of the 1x3 falling blocks */

static char fallspecial = ' '; /* what the %%% block fell on */

static int falldelay;

static int blinkcount;

static int level;
static int score;
static int nextlevel;
static int blockcount;

static int destlevel; /* last level on which a destroyer block fell */

static int scorebonus;

static const char *blocks = CH_BLOCKS;

static int tolevel[] =
{
	0,      /* to Level 1 */
	40,     /* to Level 2 */
	60,     /* to Level 3 */
	80,     /* to Level 4 */
	90,     /* to Level 5 */
	100,    /* to Level 6 */
	110,    /* to Level 7 */
	120,    /* to Level 8 */
	130,    /* to Level 9 */
	140,    /* to Level 10 */
	INT_MAX /* let's hope this doesn't happen */
};

static long progstarttime;

static void starttimer(void)
{
	struct timeval progstart;
	(void)gettimeofday(&progstart, NULL);
	progstarttime = progstart.tv_sec * 1000
		+ progstart.tv_usec / 1000;
}

static long gettime(void)
{
	struct timeval tv;
	long thetime;
	(void)gettimeofday(&tv, NULL);
	thetime = tv.tv_sec * 1000 + tv.tv_usec / 1000;
	return thetime - progstarttime;
}

/* delay(n)  to start a delay of n milliseconds
   delay(0)  to actually wait until the delay is over
   delay(-1) to check how many ms of the delay are left */
static int delay(int ms)
{
	static long endtime;

	if (ms > 0)       /* set delay */
	{
		endtime = gettime() + ms;
		return ms;
	}
	else if (ms == 0) /* actually wait */
	{
		long timeleft = endtime - gettime();
		while (timeleft > 0)
		{
			millisleep((int)timeleft);
			timeleft = endtime - gettime();
		}
		return 0;
	}
	else              /* how many ms left? */
	{
		long timeleft = endtime - gettime();
		if (timeleft < 0)
			return 0;
		return (int)timeleft;
	}
}

static void setblock(int row, int col, char content)
{
	playfield [row][col] = content;
	cleanblock[row][col] = 0;
}

static char getblock(int row, int col)
{
	if (row < 0 || row >= height || col < 0 || col >= width)
		return ' ';
	return playfield[row][col];
}

/* draw all the blocks */
static void drawscreen(void)
{
	static int lastlevel = -2;
	static int lastscore = -2;
	int r, c;

	for (r = 3; r < height; r++)
	for (c = 0; c < width;  c++)
	{
		if (!cleanblock[r][c])
		{
			drawblock(r-3, c, (chtype)getblock(r, c));
			cleanblock[r][c] = 1;
		}
	}

	if (score != lastscore)
	{
		lastscore = score;
		drawscore(score);
		if (level != lastlevel)
		{
			lastlevel = level;
			drawlevel(level);
		}
	}
	updatescreen();
}

static void startfall(void)
{
	fallrow = 0;
	fallcol = width / 2;
	if (destlevel < level /* no destroyer blocks yet for this level */
		&& nextlevel >= DESTROYER_BLOCK_WINSTART
		&& nextlevel < DESTROYER_BLOCK_WINEND
		&& level >= DESTROYER_BLOCK_MINLEVEL
		&& blockcount > DESTROYER_BLOCK_MINCOUNT
		&& random()%DESTROYER_BLOCK_CHANCE == 0)
	{
		/* make it a %%% block */
		setblock(fallrow,   fallcol, blocks[0]);
		setblock(fallrow+1, fallcol, blocks[0]);
		setblock(fallrow+2, fallcol, blocks[0]);

		/* make sure not to send another one on the same level */
		destlevel = level;
	}
	else
	{
		setblock(fallrow,   fallcol, blocks[1 + random()%NUMBLOCKS]);
		setblock(fallrow+1, fallcol, blocks[1 + random()%NUMBLOCKS]);
		setblock(fallrow+2, fallcol, blocks[1 + random()%NUMBLOCKS]);
	}
	state = STATE_FALL;

	blockcount += 3;
}

/* test if a block can fall */
static int canfall(int row, int col)
{
	return row + 1 < height && getblock(row + 1, col) == ' '
		&& getblock(row, col) != ' ';
}

/* blocks were destroyed; lower the next level countdown accordingly
   and increase score */
static void blocksdestroyed(int num)
{
	score      += num * (scorebonus + level);
	nextlevel  -= num;
	blockcount -= num;
	if (nextlevel < 0)
	{
		/* advance a level */
		level++;
		falldelay -= DELAY_DECREASE;

		nextlevel = tolevel[level];
	}
}

/* move a block */
static void moveblock(int orow, int ocol, int nrow, int ncol)
{
	setblock(nrow, ncol, getblock(orow, ocol));
	setblock(orow, ocol, ' ');
}

/* lower a block by one row */
static void lower(int row, int col)
{
	moveblock(row, col, row + 1, col);
}

/* move the falling blocks left or right */
static int movefallingblocks(int colchange)
{
	int newcol = fallcol + colchange;

	if (newcol < 0 || newcol >= width
		|| getblock(fallrow    , newcol) != ' '
		|| getblock(fallrow + 1, newcol) != ' '
		|| getblock(fallrow + 2, newcol) != ' ')
	{
		return 0;
	}

	moveblock(fallrow    , fallcol, fallrow    , newcol);
	moveblock(fallrow + 1, fallcol, fallrow + 1, newcol);
	moveblock(fallrow + 2, fallcol, fallrow + 2, newcol);

	fallcol = newcol;
	return 1;
}

/* shuffle the falling blocks' order */
static void shuffleblocks(void)
{
	char tmp;

	/* shift each one down */
	tmp = getblock(fallrow + 2, fallcol);
	setblock(fallrow + 2, fallcol, getblock(fallrow + 1, fallcol));
	setblock(fallrow + 1, fallcol, getblock(fallrow    , fallcol));
	setblock(fallrow    , fallcol, tmp);
}

/* make the 1x3 block fall down a row; return 1 if successful */
static int makeblocksfall(void)
{
	if (!canfall(fallrow + 2, fallcol))
	{
		/* if it's a %%% block falling, set fallspecial, so
		   we can destroy all blocks of the color it landed on */
		if (getblock(fallrow + 2, fallcol) == blocks[0]
			&& fallrow + 3 < height)
		{
			fallspecial = getblock(fallrow + 3, fallcol);
		}

		return 0;
	}

	lower(fallrow + 2, fallcol);
	lower(fallrow + 1, fallcol);
	lower(fallrow,     fallcol);

	fallrow++;

	return 1;
}

/* destroy all blinking blocks */
static void destroyblinkers(void)
{
	int r, c;
	int numdest = 0;

	for (r = 3; r < height; r++)
	for (c = 0; c < width;  c++)
	{
		if (blinking[r][c])
		{
			blinking[r][c] = 0;
			setblock(r, c, ' ');
			numdest++;
		}
	}

	blocksdestroyed(numdest);
}

static int matches(int row, int col, char color)
{
	return getblock(row, col) == color;
}

/* find matches centered at row, col */
static int findmatchesfrom(int row, int col)
{
	char color;
	int numfound = 0;

	color = getblock(row, col);
	if (color == ' ')
		return 0;

	/* vertical */
	if (
		(
			(
				matches(row-1, col, color)
				&& (
					matches(row-2, col, color)
					|| matches(row+1, col, color)
				)
			)
			|| (
				matches(row+1, col, color)
				&& matches(row+2, col, color)
			)
		)
	)
	{
		int j;
		for (j = row - 1; j >= 0; j--)
		{
			if (!matches(j, col, color))
				break;
			if (!blinking[j][col])
			{
				numfound++;
				blinking[j][col] = 1;
			}
		}
		for (j = row; j < height; j++)
		{
			if (!matches(j, col, color))
				break;
			if (!blinking[j][col])
			{
				numfound++;
				blinking[j][col] = 1;
			}
		}
	}

	/* horizontal */
	if (
		(
			(
				matches(row, col-1, color)
				&& (
					matches(row, col-2, color)
					|| matches(row, col+1, color)
				)
			)
			|| (
				matches(row, col+1, color)
				&& matches(row, col+2, color)
			)
		)
	)
	{
		int j;
		for (j = col - 1; j >= 0; j--)
		{
			if (!matches(row, j, color))
				break;
			if (!blinking[row][j])
			{
				numfound++;
				blinking[row][j] = 1;
			}
		}
		for (j = col; j < width; j++)
		{
			if (!matches(row, j, color))
				break;
			if (!blinking[row][j])
			{
				numfound++;
				blinking[row][j] = 1;
			}
		}
	}

	/* diagonal with positive slope */
	if (
		(
			(
				matches(row-1, col-1, color)
				&& (
					matches(row-2, col-2, color)
					|| matches(row+1, col+1, color)
				)
			)
			|| (
				matches(row+1, col+1, color)
				&& matches(row+2, col+2, color)
			)
		)
	)
	{
		int j, k;
		for (j = col - 1, k = row - 1; j >= 0 && k >= 0; j--, k--)
		{
			if (!matches(k, j, color))
				break;
			if (!blinking[k][j])
			{
				numfound++;
				blinking[k][j] = 1;
			}
		}
		for (j = col, k = row; j < width && k < height; j++, k++)
		{
			if (!matches(k, j, color))
				break;
			if (!blinking[k][j])
			{
				numfound++;
				blinking[k][j] = 1;
			}
		}
	}

	/* diagonal with negative slope */
	if (
		(
			(
				matches(row-1, col+1, color)
				&& (
					matches(row-2, col+2, color)
					|| matches(row+1, col-1, color)
				)
			)
			|| (
				matches(row+1, col-1, color)
				&& matches(row+2, col-2, color)
			)
		)
	)
	{
		int j, k;
		for (j = col + 1, k = row - 1; j < width && k >= 0; j++, k--)
		{
			if (!matches(k, j, color))
				break;
			if (!blinking[k][j])
			{
				numfound++;
				blinking[k][j] = 1;
			}
		}
		for (j = col, k = row; j >= 0 && k < height; j--, k++)
		{
			if (!matches(k, j, color))
				break;
			if (!blinking[k][j])
			{
				numfound++;
				blinking[k][j] = 1;
			}
		}
	}

	/* that was painful */

	return numfound;
}

/* find any blocks that will be eliminated, and set them as blinking,
   returning the number found */
static int findmatches(void)
{
	int numfound = 0;
	int r, c;

	if (fallspecial != ' ')
	{
		for (r = 3; r < height; r++)
		for (c = 0; c < width;  c++)
		{
			if (getblock(r, c) == fallspecial)
			{
				numfound++;
				blinking[r][c] = 1;
			}
		}
		fallspecial = ' ';
	}

	for (r = 3; r < height; r++)
	for (c = 0; c < width;  c++)
		numfound += findmatchesfrom(r, c);

	return numfound;
}

/* empty all the blocks */
static void emptyblocks(void)
{
	int r, c;

	for (r = 0; r < height; r++)
	for (c = 0; c < width;  c++)
		setblock(r, c, ' ');
}

/* replace all blinking blocks with spaces on the screen */
static void hideblinkers(void)
{
	int r, c;

	for (r = 3; r < height; r++)
	for (c = 0; c < width;  c++)
	{
		if (blinking[r][c])
			drawblock(r-3, c, ' ');
	}
}

/* draw all blinking blocks onto the screen */
static void showblinkers(void)
{
	int r, c;

	for (r = 3; r < height; r++)
	for (c = 0; c < width;  c++)
	{
		if (blinking[r][c])
			drawblock(r-3, c, (chtype)playfield[r][c]);
	}
}

/* move down a row any blocks that are above spaces,
   returning 1 if any are moved */
static int enforcegravity(void)
{
	int r, c;
	int anymoved = 0;

	for (r = height - 1; r >= 3; r--)
	for (c = width  - 1; c >= 0; c--)
	{
		if (canfall(r, c))
		{
			lower(r, c);
			anymoved = 1;
		}
	}
	return anymoved;
}

static void pausegame(void)
{
	(void)nodelay(stdscr, FALSE); /* do wait this time */
	(void)getch();
	(void)nodelay(stdscr, TRUE);  /* now stop delaying for input */
}

/* devour all input up to a 'q' (in which case return 1) or the end of
   the pending input (in which case return 0) */
static int wantstoquit(void)
{
	int ch;
	int pausenow = 0;

	while ((ch = getch()) != ERR)
	{
		if (ch == 'q')
			return 1;
		else if (ch == 'p')
			pausenow = 1;
	}

	if (pausenow)
		pausegame();

	return 0; /* nope, the player doesn't want to quit just yet */
}

static void dofall(void)
{
	long timeleft;

	(void)delay(falldelay); /* set a delay */

	while ((timeleft = delay(-1)) > 20)
	{
		int ch;

		if (timeleft < FALL_DELAY_ACCEL)
			millisleep(18);
		else
			millisleep(FALL_DELAY_ACCEL);

		while ((ch = getch()) != ERR)
		{
			if (ch == KEY_LEFT || ch == 'h')
			{
				if (movefallingblocks(-1))
					drawscreen();
			}
			else if (ch == KEY_RIGHT || ch == 'l')
			{
				if (movefallingblocks(+1))
					drawscreen();
			}
			else if (ch == KEY_UP || ch == 'k')
			{
				shuffleblocks();
				drawscreen();
			}
			else if (ch == KEY_DOWN || ch == 'j')
			{
				if (makeblocksfall())
					drawscreen();
			}
			else if (ch == 'q') /* quit the game */
			{
				state = STATE_GAMEOVER;
				return;
			}
			else if (ch == 'p')
				pausegame();
		}
	}

	if (timeleft)
		(void)delay(0); /* finish the delay */

	if (!makeblocksfall())
	{
		/* no more falling is to be done */

		scorebonus = 0;

		if (fallrow < 3) /* above visible playfield */
			state = STATE_GAMEOVER;
		else if (findmatches())
		{
			state = STATE_BLINK;
			blinkcount = 0;
		}
		else
			startfall();
	}
	else
		drawscreen();
}

static void doblink(void)
{
	millisleep(BLINK_DELAY);
	blinkcount++;
	if (blinkcount == BLINK_TIMES)
	{
		blinkcount = 0;
		state = STATE_GRAVITY;
		destroyblinkers();
	}
	if (blinkcount & 1)
		hideblinkers();
	else
		showblinkers();
	updatescreen();

	if (wantstoquit())
		state = STATE_GAMEOVER;
}

static void dogravity(void)
{
	millisleep(FALL_DELAY_GRAVITY);

	if (!enforcegravity())
	{
		if (scorebonus < SCOREBONUS_MAX)
			scorebonus++;
		if (findmatches())
		{
			state = STATE_BLINK;
			blinkcount = 0;
		}
		else
			startfall();
	}
	else
		drawscreen();

	if (wantstoquit())
		state = STATE_GAMEOVER;
}

void playgame(int w, int h)
{
	score      = 0;
	level      = 0;
	blockcount = 1;
	nextlevel  = tolevel[0];
	falldelay  = FALL_DELAY_INITIAL;
	blocksdestroyed(1);

	width  = w;
	height = h + 3; /* the extra 3 are at the top, not visible */

	srandom((unsigned int)time(NULL));
	emptyblocks();
	starttimer();
	startfall();

	drawborders(w, h);
	drawlevel(1);
	drawscore(0);
	updatescreen();

	while (state != STATE_GAMEOVER)
	{
		if (state == STATE_FALL)
			dofall();
		else if (state == STATE_BLINK)
			doblink();
		else if (state == STATE_GRAVITY)
			dogravity();
	}

	drawscreen();
	millisleep(1000);
}
