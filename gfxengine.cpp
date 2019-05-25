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
#include "gfx_sdl.h"

#include <iostream>
#include <fstream>
using namespace std;

#include <assert.h>

// Screen Listing
u8	glbScreenCharMap[SCR_HEIGHT][SCR_WIDTH];
int	glbScreenAttrMap[SCR_HEIGHT][SCR_WIDTH];
bool	glbScreenDirty = false;

void
gfx_init()
{
    gfxsdl_init();
}

void
gfx_shutdown()
{
    gfxsdl_shutdown();
}

void
gfx_delay(int frames)
{
    gfxsdl_delay(frames);
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
	gfxsdl_rebuildscreen();
	glbScreenDirty = false;
    }
}

bool
gfx_isKeyWaiting()
{
    return gfxsdl_isKeyWaiting();
}

void
gfx_clearKeyBuffer()
{
    gfxsdl_clearKeyBuffer();
}

void
gfx_breakKeyRepeat()
{
    gfxsdl_breakKeyRepeat();
}

int
gfx_getKey(bool block)
{
    if (block)
    {
	while (!gfx_isKeyWaiting())
	{
	    gfxsdl_awaitEvent();
	}
    }
    return gfxsdl_getKey();
}

//
// Bitmap loading code.
// (Pre 7DRL)
//
#pragma pack(push, 1)

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
    int	    ypelpm;
    int	    clrused;
    int	    clrimport;
};

#pragma pack(pop)

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

    is.read((char *) &head, sizeof(BMPHEAD));

    if (head.bits != 24 && head.bits != 8)
    {
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

