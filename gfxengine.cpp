/*
 * Licensed under BSD license.  See LICENCE.TXT  
 *
 * Produced by:	Jeff Lait
 *
 *      	7DRL Development
 *
 * NAME:        gfxengine.cpp ( Live Once Library, C++ )
 *
 * COMMENTS:
 */

#include "gfxengine.h"

#define USE_SDL

#ifdef USE_SDL
#include "gfx_sdl.h"
#else
#include "gfx_curses.h"
#endif

#ifdef USE_SDL
#define INVOKEGFX(func) gfxsdl_##func
#else
#define INVOKEGFX(func) gfxcurses_##func
#endif

#include <iostream>
#include <fstream>
using namespace std;

#include <assert.h>

// Screen Listing
u8	glbScreenCharMap[SCR_HEIGHT][SCR_WIDTH];
int	glbScreenAttrMap[SCR_HEIGHT][SCR_WIDTH];
bool	glbScreenDirty = false;

// There is one true endian style.
bool	glbEvilEndian = false;

void
gfx_init()
{
    INVOKEGFX(init)();

    // Determine our endianness.
    volatile unsigned	short	 s;
    u8			*c;

    s = 0;
    c = (u8 *) &s;
    *c = 1;

    // The short will now either be 256 or 1.
    if (s == 1)
	glbEvilEndian = false;
    else
	glbEvilEndian = true;
}

void
gfx_shutdown()
{
    INVOKEGFX(shutdown)();
}

void
gfx_delay(int frames)
{
    INVOKEGFX(delay)(frames);
}

void
gfx_printchar(int x, int y, u8 c, ATTR_NAMES attr)
{
    assert(x >= 0 && x < SCR_WIDTH);
    assert(y >= 0 && y < SCR_HEIGHT);
    glbScreenCharMap[y][x] = c;
    glbScreenAttrMap[y][x] = attr;
    glbScreenDirty = true;
}

void
gfx_printstring(int x, int y, const char *s, ATTR_NAMES attr)
{
    assert(y >= 0 && y < SCR_HEIGHT);

    while (*s && x < SCR_WIDTH)
    {
	gfx_printchar(x, y, *s, attr);
	s++;
	x++;
    }
}

void
gfx_clearbox(int sx, int sy, int w, int h, u8 c, ATTR_NAMES attr)
{
    int		x, y;
    
    for (y = sy; y < sy + h; y++)
    {
	for (x = sx; x < sx + w; x++)
	{
	    gfx_printchar(x, y, c, attr);
	}
    }
}

void
gfx_getchar(int x, int y, u8 &c, ATTR_NAMES &attr)
{
    assert(x >= 0 && x < SCR_WIDTH);
    assert(y >= 0 && y < SCR_HEIGHT);

    c = glbScreenCharMap[y][x];
    attr = (ATTR_NAMES) glbScreenAttrMap[y][x];
}

void
gfx_update()
{
    if (glbScreenDirty)
    {
	INVOKEGFX(rebuildscreen)();
	glbScreenDirty = false;
    }
}

bool
gfx_isKeyWaiting()
{
    return INVOKEGFX(isKeyWaiting)();
}

void
gfx_clearKeyBuffer()
{
    INVOKEGFX(clearKeyBuffer)();
}

void
gfx_breakKeyRepeat()
{
    INVOKEGFX(breakKeyRepeat)();
}

int
gfx_getKey(bool block)
{
    if (block)
    {
	while (!gfx_isKeyWaiting())
	{
	    INVOKEGFX(awaitEvent)();
	}
    }
    return INVOKEGFX(getKey)();
}

//
// Bitmap loading code.
// (Pre 7DRL)
//

static short
gfx_readportableshort(istream &is)
{
    u8		c[2];
    short	result;

    is.read((char *) c, 2);

    if (glbEvilEndian)
    {
	result = c[0] << 8;
	result |= c[1];
    }
    else
    {
	result = c[0];
	result |= c[1] << 8;
    }
    return result;
}

static int
gfx_readportableint(istream &is)
{
    unsigned short	s1, s2;
    int			result;

    s1 = (unsigned short) gfx_readportableshort(is);
    s2 = (unsigned short) gfx_readportableshort(is);

    if (glbEvilEndian)
    {
	result = s1 << 16;
	result |= s2;
    }
    else
    {
	result = s1;
	result |= s2 << 16;
    }

    return result;
}

struct BMPHEAD
{
    char    id[2];
    int	    size;
    int reserved;
    int	    headersize;
    int	    infosize;
    int	    width;
    int	    depth;
    short   biplane;
    short   bits;
    int	    comp;
    int	    imagesize;
    int	    xpelperm;
    int	    ypelperm;
    int	    clrused;
    int	    clrimport;
};

struct RGBQUAD
{
    u8 blue;
    u8 green;
    u8 red;
    u8 filler;
};

u8 *
gfx_loadbitmap(const char *fname, int &w, int &h)
{
    ifstream	    is;
    BMPHEAD	    head;
    int		    i, x, y;

    is.open(fname, ios_base::in | ios_base::binary);

    if (!is)
    {
	cerr << "Failure to open: " << fname << endl;
	return 0;
    }

    is.read(head.id, 2);
    
    head.size = gfx_readportableint(is);
    head.reserved = gfx_readportableint(is);
    head.headersize = gfx_readportableint(is);
    head.infosize = gfx_readportableint(is);
    head.width = gfx_readportableint(is);
    head.depth = gfx_readportableint(is);
    head.biplane = gfx_readportableshort(is);
    head.bits = gfx_readportableshort(is);
    head.comp = gfx_readportableint(is);
    head.imagesize = gfx_readportableint(is);
    head.xpelperm = gfx_readportableint(is);
    head.ypelperm = gfx_readportableint(is);
    head.clrused = gfx_readportableint(is);
    head.clrimport = gfx_readportableint(is);

    if (head.id[0] != 'B' && head.id[1] != 'M')
    {
	cerr << fname << " does not look like a bit map file!" << endl;
	return 0;
    }

    if (head.bits != 24 && head.bits != 8)
    {
	cerr << "Error reading " << fname << endl;
	cerr << "Head.bits is " << head.bits << endl;
	cerr << "Cannot support non 24 or 8 bit colour bitmaps." << endl;
	return 0;
    }

    u8	*result;

    h = head.depth;
    w = head.width;

    result = new u8 [h * w * 3];

    RGBQUAD	palette[256];

    if (head.bits == 8)
    {
	is.read((char *) &palette, sizeof(RGBQUAD) * 256);
    }

    // BMPs are stored bottom up, so we reverse it here.
    for (y = head.depth - 1; y >= 0; y--)
    {
	for (x = 0; x < head.width; x++)
	{
	    u8		red, blue, green, idx;

	    if (head.bits == 24)
	    {
		// BMPs are stored BGR.
		is.read((char *) &red, 1);
		is.read((char *) &green, 1);
		is.read((char *) &blue, 1);
	    }
	    else
	    {
		is.read((char *) &idx, 1);
		red = palette[idx].blue;
		green = palette[idx].green;
		blue = palette[idx].red;
	    }
	
	    // Account for endian issues & repack
	    i = x + y * head.width;

	    result[i*3] = red;
	    result[i*3+1] = green;
	    result[i*3+2] = blue;
	}
    }

    return result;
}

