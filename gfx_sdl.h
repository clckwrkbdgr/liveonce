/*
 * Licensed under BSD license.  See LICENCE.TXT  
 *
 * Produced by:	Jeff Lait
 *
 *      	POWDER Development
 *
 * NAME:        gfx_sdl.h ( 7DRL Library, C++ )
 *
 * COMMENTS:
 */

#ifndef __gfx_sdl__
#define __gfx_sdl__

void gfxsdl_init();
void gfxsdl_shutdown();

void gfxsdl_delay(int frames);

void gfxsdl_rebuildscreen();

bool gfxsdl_isKeyWaiting();
void gfxsdl_clearKeyBuffer();
void gfxsdl_breakKeyRepeat();
int gfxsdl_getKey();
void gfxsdl_putKey(int key);

int gfxsdl_getframecount();
void gfxsdl_awaitEvent();

#endif
