/*
 * Licensed under BSD license.  See LICENCE.TXT  
 *
 * Produced by:	Jeff Lait
 *
 *      	POWDER Development
 *
 * NAME:        gfx_curses.cpp ( 7DRL Library, C++ )
 *
 * COMMENTS:
 */

// Redefine our colours so we don't run into problems with curses.
#define COLOR_BLACK COLOUR_BLACK
#define COLOR_RED COLOUR_RED
#define COLOR_GREEN COLOUR_GREEN
#define COLOR_YELLOW COLOUR_YELLOW
#define COLOR_BLUE COLOUR_BLUE
#define COLOR_MAGENTA COLOUR_MAGENTA
#define COLOR_CYAN COLOUR_CYAN
#define COLOR_WHITE COLOUR_WHITE

#include "glbdef.h"

// Window's colours are red/blue reversed.
int glbWinLUT[8] =
{
    COLOUR_BLACK,
    COLOUR_BLUE,
    COLOUR_GREEN,
    COLOUR_CYAN,
    COLOUR_RED,
    COLOUR_MAGENTA,
    COLOUR_YELLOW,
    COLOUR_WHITE
};

#undef COLOR_BLACK
#undef COLOR_RED
#undef COLOR_GREEN
#undef COLOR_YELLOW
#undef COLOR_BLUE
#undef COLOR_MAGENTA
#undef COLOR_CYAN
#undef COLOR_WHITE

#include "gfxengine.h"
#include "gfx_curses.h"

#include <string.h>
#include <iostream>
using namespace std;

#include <assert.h>

// Windows specific code
#ifdef WIN32
#include <windows.h>
#define usleep(x) Sleep((x)/1000)
#define sleep(x) Sleep((x))
#else
#include <unistd.h>
#endif

#include <curses.h>

#define		WIDTH  80
#define		HEIGHT 30

//
// Key Queue
//
int	*glbCurseKeyQueue = 0;
int	 glbCurseKeyQueueLen = 0;
int	 glbCurseKeyQueueSize = 0;

//
// Current frame number
//
volatile int	 glbCurseFrameCount = 0;

int
gfxcurses_getframecount()
{
    return glbCurseFrameCount;
}

void
gfxcurses_delay(int frames)
{
    usleep((int) (frames * 1000000 / 60.0));
}

int
gfxcurses_cookkey(int curseskey)
{
    switch (curseskey)
    {
	case KEY_LEFT:
	    return GFX_KEYLEFT;
	case KEY_RIGHT:
	    return GFX_KEYRIGHT;
	case KEY_UP:
	    return GFX_KEYUP;
	case KEY_DOWN:
	    return GFX_KEYDOWN;
	case 127:
	case KEY_DL:
	case KEY_BACKSPACE:
	    return '\b';
    }
    
    return curseskey;
}

void
gfxcurses_init()
{
    initscr();
    start_color();
    cbreak();
    noecho();
    nodelay(stdscr, true);
    intrflush(stdscr, false);
    keypad(stdscr, true);

    // Initialize our colour pairs.
    int		i;
    for (i = 0; i < NUM_ATTRS; i++)
    {
#ifdef WIN32
	init_pair(i, glbWinLUT[glb_attrdefs[i].fore], 
		     glbWinLUT[glb_attrdefs[i].back]);
#else
	init_pair(i, glb_attrdefs[i].fore, glb_attrdefs[i].back);
#endif
    }

    // Verify the terminal is valid.
    if (WIDTH != COLS)
    {
	endwin();
	cerr << "You must play in a 80 column terminal." << endl;
	exit(-1);
    }

    if (LINES < HEIGHT)
    {
	endwin();
	cerr << "You must have at least 30 lines in your terminal." << endl;
	exit(-1);
    }

    // Clear our text array
    gfx_clearbox(0, 0, WIDTH, HEIGHT, ' ', ATTR_NORMAL);
}

void
gfxcurses_shutdown()
{
    endwin();
}

void
gfxcurses_rebuildscreen()
{
    int		x, y;
    u8		c;
    ATTR_NAMES	attr;

    move(0, 0);
    for (y = 0; y < SCR_HEIGHT; y++)
    {
	for (x = 0; x < SCR_WIDTH; x++)
	{
	    gfx_getchar(x, y, c, attr);
	    
	    if (glb_attrdefs[attr].bold)
	    {
		attrset(COLOR_PAIR(attr) | A_BOLD);
	    }
	    else
	    {
		attrset(COLOR_PAIR(attr));
	    }
	    addch(c);
	}
    }

    refresh();
}

//
// Keyboard functions
//
bool
gfxcurses_isKeyWaiting()
{
    if (glbCurseKeyQueueLen > 0)
	return true;

    gfxcurses_awaitEvent();

    // Test for a new key...
    int		c;
    c = getch();
    if (c == ERR)
	return false;

    gfxcurses_putKey(gfxcurses_cookkey(c));
    return true;
}

void
gfxcurses_clearKeyBuffer()
{
    gfxcurses_isKeyWaiting();
    glbCurseKeyQueueLen = 0;
}

void
gfxcurses_putKey(int key)
{
    if (glbCurseKeyQueueLen >= glbCurseKeyQueueSize)
    {
	// Grow the queue.
	int		*newqueue;

	newqueue = new int [(glbCurseKeyQueueSize + 4)*2];

	if (glbCurseKeyQueueLen)
	{
	    memcpy(newqueue, glbCurseKeyQueue, sizeof(int) * glbCurseKeyQueueLen);
	}
	delete [] glbCurseKeyQueue;
	glbCurseKeyQueue = newqueue;
	glbCurseKeyQueueSize = (glbCurseKeyQueueSize + 4) * 2;
    }

    glbCurseKeyQueue[glbCurseKeyQueueLen++] = key;
}

void
gfxcurses_breakKeyRepeat()
{
    // Nothing to be done here.
}

int
gfxcurses_getKey()
{
    if (!gfxcurses_isKeyWaiting())
	return 0;

    int		key;

    key = glbCurseKeyQueue[0];
    memmove(glbCurseKeyQueue, &glbCurseKeyQueue[1], sizeof(int) * (glbCurseKeyQueueLen-1));

    glbCurseKeyQueueLen--;
    return key;
}

void
gfxcurses_awaitEvent()
{
    sleep(0);
}

