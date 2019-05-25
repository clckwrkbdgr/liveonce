/*
 * Licensed under BSD license.  See LICENCE.TXT  
 *
 * Produced by:	Jeff Lait
 *
 *      	POWDER Development
 *
 * NAME:        gfx_sdl.cpp ( 7DRL Library, C++ )
 *
 * COMMENTS:
 */

#include "gfxengine.h"
#include "gfx_sdl.h"

#include <iostream>
using namespace std;

#include <assert.h>

#include "SDL.h"

#define		WIDTH  80
#define		HEIGHT 30

#define KEY_REPEAT_INITIAL 15
#define KEY_REPEAT_AFTER 7

SDL_Surface	*glbVideoSurface = 0;

bool		 screendirty = false;

//
// Key Queue
//
int	*glbKeyQueue = 0;
int	 glbKeyQueueLen = 0;
int	 glbKeyQueueSize = 0;

int	 glbKeyPusher = 0;
int	 glbKeyPushTime = 0;

//
// Current frame number
//
volatile int	 glbFrameCount = 0;

//
// Cached font information
//
u8 *glbFontMap;
int glbFontOffset[256];
int glbFontFullWidth, glbFontFullHeight;
int glbFontWidth, glbFontHeight;

// Lookup table
// Text look up tables...
const char *glbTextTable[] = { 
    "abcdefghij",
    "klmnopqrst",
    "uvwxyz    ",
    "ABCDEFGHIJ",
    "KLMNOPQRST",
    "UVWXYZ    ",
    "0123456789",
    "!@#$%^&*()",
    "=-\\',./+_|",
    "\"<>?[]{}`~",
    ",.<>;:    ",
    0
};

int
gfxsdl_getframecount()
{
    return glbFrameCount;
}

void
gfxsdl_delay(int frames)
{
    int		start;

    start = gfxsdl_getframecount();

    while (gfxsdl_getframecount() - start < frames)
    {
	gfxsdl_awaitEvent();
    }
}

int
gfxsdl_cookkey(int unicode, int sdlkey)
{
    if (unicode && unicode < 128)
	return unicode;

    switch (sdlkey)
    {
	case SDLK_UP:
	    return GFX_KEYUP;
	case SDLK_DOWN:
	    return GFX_KEYDOWN;
	case SDLK_LEFT:
	    return GFX_KEYLEFT;
	case SDLK_RIGHT:
	    return GFX_KEYRIGHT;
    }
    return 0;
}

//
// Finds the offset for the given character.
//
int
gfxsdl_findoffset(u8 c)
{
    if (!c)
	return -1;

    int		x, y;

    for (y = 0; y < 11; y++)
    {
	for (x = 0; x < 10; x++)
	{
	    if (c == glbTextTable[y][x])
	    {
		return (x * glbFontWidth + y * glbFontHeight * glbFontFullWidth) * 3;
	    }
	}
    }
    // Not found.
    return -1;
}

void
gfxsdl_initfont()
{
    int	    i;

    glbFontMap = gfx_loadbitmap("alphabet.bmp", glbFontFullWidth, glbFontFullHeight);

    // Determine size of characters.
    glbFontWidth = glbFontFullWidth / 10;
    glbFontHeight = glbFontFullHeight / 11;

    for (i = 0; i < 256; i++)
    {
	glbFontOffset[i] = gfxsdl_findoffset(i);
    }
}

Uint32
glb_VBLTimerCallback(Uint32 interval, void *fp)
{
    glbFrameCount++;

    // Push a fake event so we wake up.
    SDL_Event		event;
    event.type = SDL_USEREVENT;
    SDL_PushEvent(&event);

    return 16;
}

void
gfxsdl_init()
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0)
    {
	cerr << "Video init failed." << endl;
	SDL_Quit();
    }

    gfxsdl_initfont();

    // Clear our text array
    gfx_clearbox(0, 0, WIDTH, HEIGHT, ' ', ATTR_NORMAL);

    glbVideoSurface = SDL_SetVideoMode(SCR_WIDTH * glbFontWidth, SCR_HEIGHT * glbFontHeight, 24, 0);
    if (!glbVideoSurface)
    {
	cerr << "Failed to acquire video surface." << endl;
	SDL_Quit();
    }

    SDL_WM_SetCaption("Live Once", 0);

    SDL_EnableUNICODE(1);

    SDL_AddTimer(16, glb_VBLTimerCallback, 0);
}

void
gfxsdl_shutdown()
{
    delete [] glbFontMap;
    glbFontMap = 0;
    SDL_Quit();
}

void
gfxsdl_blitchar(u8 *dst, int pitch, u8 c, int attr)
{
    // Colours.
    assert(attr >= 0 && attr < NUM_ATTRS);
    if (attr < 0 || attr >= NUM_ATTRS)
	attr = ATTR_NORMAL;

    int		 offset;
    u8		*src, r, g, b;
    int		 x, y;

    src = glbFontMap;
    offset = glbFontOffset[c];
    if (offset < 0)
    {
	// Invalid character.  Ignore.
	assert(!"Unknown character.");
	return;
    }
    src += offset;

    for (y = 0; y < glbFontHeight; y++)
    {
	for (x = 0; x < glbFontWidth; x++)
	{
	    r = *src++;
	    g = *src++;
	    b = *src++;

	    // All white, use foreground.  Otherwise
	    // background.
	    if ((r & g & b) == 255)
	    {
		*dst++ = glb_attrdefs[attr].fg_b;
		*dst++ = glb_attrdefs[attr].fg_g;
		*dst++ = glb_attrdefs[attr].fg_r;
	    }
	    else
	    {
		*dst++ = glb_attrdefs[attr].bg_b;
		*dst++ = glb_attrdefs[attr].bg_g;
		*dst++ = glb_attrdefs[attr].bg_r;
	    }
	}
	src += (glbFontFullWidth - glbFontWidth) * 3;
	dst += pitch - 3 * glbFontWidth;
    }
}

void
gfxsdl_rebuildscreen()
{
    int		x, y;
    u8		c;
    ATTR_NAMES	attr;

    SDL_LockSurface(glbVideoSurface);

    u8		*dst;
    dst = (u8 *) glbVideoSurface->pixels;
    if (!dst)
    {
	cerr << "Lock failure: " << SDL_GetError() << endl;
    }

    for (y = 0; y < SCR_HEIGHT; y++)
    {
	for (x = 0; x < SCR_WIDTH; x++)
	{
	    gfx_getchar(x, y, c, attr);
	    
	    gfxsdl_blitchar(&dst[x*glbFontWidth*3+y*glbFontHeight*glbVideoSurface->pitch], glbVideoSurface->pitch, c, attr);
	}
    }

    SDL_UnlockSurface(glbVideoSurface);

    SDL_UpdateRect(glbVideoSurface, 0, 0, 0, 0);
}

//
// Event Polling
//
void
gfxsdl_pollEvents()
{
    SDL_Event		event;

    while (SDL_PollEvent(&event))
    {
	switch (event.type)
	{
	    case SDL_KEYDOWN:
		// For now, ignore arrow keys..
		int key;

		key = gfxsdl_cookkey(event.key.keysym.unicode, event.key.keysym.sym);
		gfxsdl_putKey(key);
		// Set the pusher.
		glbKeyPusher = key;
		glbKeyPushTime = gfxsdl_getframecount() + KEY_REPEAT_INITIAL;
		break;

	    case SDL_KEYUP:
		// Stop the pusher.
		glbKeyPusher = 0;
		break;

	    case SDL_QUIT:
		SDL_Quit();
		exit(0);
		break;
	}
    }

    // Note that we handle key repeat here, not in the actual timer loop!
    // This way if our processing freezes, the key repeating also
    // freezes.
    {
	int		frame;

	frame = gfxsdl_getframecount();
	if (glbKeyPusher)
	{
	    if (frame >= glbKeyPushTime)
	    {
		gfxsdl_putKey(glbKeyPusher);
		glbKeyPushTime = frame + KEY_REPEAT_AFTER;
	    }
	}
    }
}

//
// Keyboard functions
//
bool
gfxsdl_isKeyWaiting()
{
    gfxsdl_pollEvents();

    if (glbKeyQueueLen > 0)
	return true;
    return false;
}

void
gfxsdl_clearKeyBuffer()
{
    gfxsdl_pollEvents();
    glbKeyQueueLen = 0;
}

void
gfxsdl_putKey(int key)
{
    if (glbKeyQueueLen >= glbKeyQueueSize)
    {
	// Grow the queue.
	int		*newqueue;

	newqueue = new int [(glbKeyQueueSize + 4)*2];

	if (glbKeyQueueLen)
	{
	    memcpy(newqueue, glbKeyQueue, sizeof(int) * glbKeyQueueLen);
	}
	delete [] glbKeyQueue;
	glbKeyQueue = newqueue;
	glbKeyQueueSize = (glbKeyQueueSize + 4) * 2;
    }

    glbKeyQueue[glbKeyQueueLen++] = key;
}

void
gfxsdl_breakKeyRepeat()
{
    glbKeyPusher = 0;
}

int
gfxsdl_getKey()
{
    if (!gfxsdl_isKeyWaiting())
	return 0;

    int		key;

    key = glbKeyQueue[0];
    memmove(glbKeyQueue, &glbKeyQueue[1], sizeof(int) * (glbKeyQueueLen-1));

    glbKeyQueueLen--;
    return key;
}

void
gfxsdl_awaitEvent()
{
    SDL_WaitEvent(0);
}
