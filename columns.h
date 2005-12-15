/*
This file is public domain; anyone may deal in it without restriction.

columns.h: definitions
*/

#include <curses.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>
#include <signal.h>
#include <ctype.h>
#include <time.h>

#define MAX_WIDTH   50
#define DEF_WIDTH   10
#define MIN_WIDTH    8

#define MAX_HEIGHT  30
#define DEF_HEIGHT  15
#define MIN_HEIGHT  10

#define PANEL_WIDTH 12

#define CH_BLOCKS "%@#$&O"              /* blocks; first one is special */
#define NUMBLOCKS (strlen(CH_BLOCKS)-1) /* % doesn't count */

/*
A Columns game in progress can be in one of three states at a particular 
time:

1. Controlled falling:

	A 1x3 block is falling. The player can move it to the left, move 
it to the right, shift the 1x1 blocks comprising it, or speed its 
descent.

	The blocks move down a row every d milliseconds, where d is some 
number (which gets smaller as the game progresses). When it is time for 
the blocks to move again but they can move no further due to blocks 
below them, either one of the blocks is above the visible top of the 
playing field and the game ends, or there are some blocks to destroy and 
the game goes to state 2, or there are no blocks to destroy and the game 
returns to this state with a new 1x3 block.

2. Blinking:

	Some blocks are to be destroyed. They are blinking; every b 
milliseconds they disappear or reappear. The blinking occurs t times. b 
and t are some constants.

	After the blinking is done, the blocks to be destroyed are 
destroyed and the state goes to 3.

3. Uncontrolled falling:

	Any blocks that are now above space fall. They fall at a rate of 
one row every d' milliseconds. I think d' is a constant that is less 
than the initial value of d, but it might be the same as d itself.

	When it is time for blocks to fall but none have space to fall 
into, either more blocks need destroying and the state goes to 2, or no 
blocks are left to be destroyed and the state goes to 1.

So: we need constants b and t, and a variable d, and a constant d'.
*/

#define BLINK_DELAY          33
#define BLINK_TIMES           8

#define DELAY_DECREASE       31

/* initial: starting fall delay for a normal fall
   gravity: delay for a fall induced by gravity (non-interactive)
   accel:   delay for when the player is holding the down arrow key */
#define FALL_DELAY_INITIAL (355 + DELAY_DECREASE)
#define FALL_DELAY_GRAVITY   50
#define FALL_DELAY_ACCEL     50

/* destroyer blocks are the %%% blocks that occasionally come down and
   destroy all blocks of the color they land on
   window:   how many values for "number of blocks to next level" during
             which a destroyer block is possible
   minlevel: never drop a destroyer block on a lower level than this
   mincount: minimum number of blocks in playing field to drop a
             destroyer block
   chance:   the chance of dropping a destroyer block when the above
             conditions are met is 1 in this many */
#define DESTROYER_BLOCK_WINDOW    8
#define DESTROYER_BLOCK_MINLEVEL  2
#define DESTROYER_BLOCK_MINCOUNT 16
#define DESTROYER_BLOCK_CHANCE    4

/* when the "destroyer block window" starts and ends */
#define DESTROYER_BLOCK_WINSTART 22
#define DESTROYER_BLOCK_WINEND \
	(DESTROYER_BLOCK_WINSTART + DESTROYER_BLOCK_WINDOW)

/*
	When you destroy numblocks blocks at a time, you get (numblocks *
(level + scorebonus)) points, where level is your level number (1 to 10), 
and scorebonus is 0 for blocks destroyed by a direct match. This formula 
is not from the original Columns but seems to work fairly well.

	Every time there is a chain reaction (e.g. gravity causes more 
blocks to match and be eliminated), scorebonus is incremented by one, 
until it reaches the following, its maximum value.
*/
#define SCOREBONUS_MAX            4

typedef enum
{
	STATE_FALL,
	STATE_BLINK,
	STATE_GRAVITY,
	STATE_GAMEOVER
} gamestate_t;

void millisleep(int ms);
void playgame(int w, int h);

int playsizeok(int width, int height);
void drawborders(int width, int height);
void drawblock(int row, int col, chtype ch);
void drawlevel(int level);
void drawscore(int score);
void updatescreen(void);
