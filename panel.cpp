/*
 * Licensed under BSD license.  See LICENCE.TXT  
 *
 * Produced by:	Jeff Lait
 *
 *      	7DRL Development
 *
 * NAME:        panel.cpp ( Live Once Library, C++ )
 *
 * COMMENTS:
 */

#include "panel.h"

#include "rand.h"
#include "gfxengine.h"

#include <ctype.h>
#include <stdlib.h>
#include <memory.h>

PANEL::PANEL(int w, int h)
{
    int		i;

    myX = 0;
    myY = 0;
    myW = w;    
    myH = h;
    myIndent = 0;
    
    myLines = new char *[h];
    for (i = 0; i < h; i++)
    {
	myLines[i] = new char [w+10];
	myLines[i][0] = '\0';
    }

    myCurLine = 0;
    myCurPos = 0;
    myAttr = ATTR_NORMAL;
}

PANEL::~PANEL()
{
    int		i;

    for (i = 0; i < myH; i++)
	delete [] myLines[i];
    delete [] myLines;
}

void
PANEL::appendText(const char *text)
{
    // Start copying text into myLines[myCurLine][myCurPos], doing
    // wordwrap at myW.
    const char		*start;
    char		*dst;
    int			 dstpos;

    start = text;
    dst = myLines[myCurLine];
    dstpos = myCurPos;

    while (*text)
    {
	// Check for control characters.
	if (*text == '\n')
	{
	    dst[dstpos] = '\0';
	    newLine();
	    appendText(text+1);
	    return;
	}
	
	// Check to see if we have hit myW.
	// Note we always scroll one character early!  This is more
	// visually pleasing than filling to the boundrary.
	if (dstpos >= myW - 1)
	{
	    // If this is a space, eat all succeeding spaces.
	    if (ISSPACE(*text))
	    {
	    }
	    else
	    {
		// Not a space, we want to roll back until the next space.
		while (text > start)
		{
		    text--;
		    dstpos--;
		    if (ISSPACE(*text))
			break;
		}
	    }

	    // Write in a null and new line.
	    dst[dstpos] = '\0';

	    // Go forward in text removing all spaces.
	    while (*text && ISSPACE(*text))
	    {
		text++;
	    }
	    newLine();
	    // Append the remainder.
	    appendText(text);
	    return;
	}

	// All good for normal addition.
	dst[dstpos] = *text;
	dst[dstpos+1] = '\0';

	dstpos++;
	text++;
	myCurPos = dstpos;
    }
}

void
PANEL::newLine()
{
    myCurLine++;
    if (myCurLine == myH)
    {
	scrollUp();
    }
    myCurPos = 0;
    int		i;
    for (i = 0; i < myIndent; i++)
	appendText(" ");
}

void
PANEL::clear()
{
    int		i;

    for (i = 0; i < myH; i++)
    {
	myLines[i][0] = '\0';
    }
    myCurPos = 0;
    myCurLine = 0;
}

void
PANEL::move(int x, int y)
{
    myX = x;
    myY = y;
}

void
PANEL::redraw()
{
    int		x, y;

    for (y = 0; y < myH; y++)
    {
	if (y + myY >= 0 && y + myY < SCR_HEIGHT)
	{
	    for (x = 0; x < myW; x++)
	    {
		// Hit end of line.
		if (!myLines[y][x])
		    break;

		if (x + myX < 0 || x + myX >= SCR_WIDTH)
		    continue;
		gfx_printchar(x + myX, y + myY, myLines[y][x], myAttr);
	    }

	    // Pad with spaces.
	    for (; x < myW; x++)
	    {
		if (x + myX < 0 || x + myX >= SCR_WIDTH)
		    continue;
		gfx_printchar(x + myX, y + myY, ' ', myAttr);
	    }
	}
    }
}

void
PANEL::setIndent(int indent)
{
    myIndent = indent;
}

void
PANEL::scrollUp()
{
    int		y;

    for (y = 1; y < myH; y++)
    {
	memcpy(myLines[y-1], myLines[y], myW+1);
    }
    myLines[myH-1][0] = '\0';

    myCurLine--;
    if (myCurLine < 0)
	myCurLine = 0;
}
