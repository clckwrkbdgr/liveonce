/*
 * Licensed under BSD license.  See LICENCE.TXT  
 *
 * Produced by:	Jeff Lait
 *
 *      	POWDER Development
 *
 * NAME:        gfx_curses.h ( 7DRL Library, C++ )
 *
 * COMMENTS:
 */

#ifndef __gfx_curses__
#define __gfx_curses__

void gfxcurses_init();
void gfxcurses_shutdown();

void gfxcurses_delay(int frames);

void gfxcurses_rebuildscreen();

bool gfxcurses_isKeyWaiting();
void gfxcurses_clearKeyBuffer();
void gfxcurses_breakKeyRepeat();
int gfxcurses_getKey();
void gfxcurses_putKey(int key);

int gfxcurses_getframecount();
void gfxcurses_awaitEvent();

#endif

