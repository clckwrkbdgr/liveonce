/*
 * Licensed under BSD license.  See LICENCE.TXT  
 *
 * Produced by:	Jeff Lait
 *
 *      	7DRL Development
 *
 * NAME:        gfxengine.h ( Live Once Library, C++ )
 *
 * COMMENTS:
 *	Provides the common interface to the screen display functions
 *	and input handling functions.
 */

#ifndef __gfxengine__
#define __gfxengine__

#include "glbdef.h"

#define SCR_WIDTH	80
#define SCR_HEIGHT	30

// Key defines.
enum GFX_Keydefine
{
    GFX_KEYSTART = 256,
    GFX_KEYUP,
    GFX_KEYLEFT,
    GFX_KEYRIGHT,
    GFX_KEYDOWN,
    GFX_KEYLAST
};

typedef	unsigned char u8;

void gfx_init();
void gfx_shutdown();

void gfx_delay(int frames);

void gfx_printchar(int x, int y, u8 c, ATTR_NAMES attr);
void gfx_printstring(int x, int y, const char *s, ATTR_NAMES attr);
void gfx_clearbox(int x, int y, int w, int h, u8 c, ATTR_NAMES attr);

void gfx_getchar(int x, int y, u8 &c, ATTR_NAMES &attr);

// Nothing is drawn until this is called.
void gfx_update();

// True if a key is waiting on the input queue.
bool gfx_isKeyWaiting();

// This empties the keyboard buffer without blocking.
void gfx_clearKeyBuffer();

// This attempts to end any current key repeating.
void gfx_breakKeyRepeat();

// Returns 0 for no key.  ASCII for ascii keys.
// Other stuff gets interesting.
// If block is true, will wait for a key press.
int gfx_getKey(bool block = true);

// Allocates a buffer for the bitmap.  Converts it to 24 bit
// rgb format.  w & h are set to the size.
u8 *gfx_loadbitmap(const char *fname, int &w, int &h);

#endif

