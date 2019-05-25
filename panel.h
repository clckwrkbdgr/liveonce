/*
 * Licensed under BSD license.  See LICENCE.TXT  
 *
 * Produced by:	Jeff Lait
 *
 *      	7DRL Development
 *
 * NAME:        panel.h ( Live Once Library, C++ )
 *
 * COMMENTS:
 *	Defines a text panel which can take scrolling text and handle
 *	all those icky line wrap issues.
 */

#ifndef __panel__
#define __panel__

#include "glbdef.h"

class PANEL
{
public:
	     PANEL(int w, int h);
    virtual ~PANEL();

    void	 appendText(const char *text);
    void	 newLine();

    void	 move(int x, int y);

    void	 clear();

    void	 redraw();

    int		 getX() const { return myX; }
    int		 getY() const { return myY; }
    int		 getW() const { return myW; }
    int		 getH() const { return myH; }

    void	 setAttr(ATTR_NAMES attr) { myAttr = attr; }

    void	 setIndent(int indent);
    
protected:
    void	 scrollUp();

    char	**myLines;
    int		 myX, myY, myW, myH;
    int		 myCurLine, myCurPos;
    int		 myIndent;

    ATTR_NAMES	 myAttr;
};

#endif

