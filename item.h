/*
 * Licensed under BSD license.  See LICENCE.TXT  
 *
 * Produced by:	Jeff Lait
 *
 *      	7DRL Development
 *
 * NAME:        item.h ( Live Once Library, C++ )
 *
 * COMMENTS:
 */

#ifndef __item__
#define __item__

#include "glbdef.h"

#include "grammar.h"

#include <iostream>
using namespace std;

class ITEM
{
public:
		~ITEM();

    static ITEM *create(ITEM_NAMES item);

    ITEM_NAMES	 getDefinition() const { return myDefinition; }
		
    VERB_PERSON	 getPerson() const;
    const char	*getName() const;

    void	 getLook(u8 &symbol, ATTR_NAMES &attr) const;

    int		 getX() const { return myX; }
    int		 getY() const { return myY; }

    bool	 canStackWith(const ITEM *stack) const;
    void	 incStackCount(int inc = 1);
    int		 getStackCount() const { return myCount; }

    void	 move(int x, int y);

    // Used by map & inventory...
    ITEM	*getNext() const { return myNext; }
    void	 setNext(ITEM *next) { myNext = next; }

    void	 save(ostream &os) const;
    static ITEM	*load(istream &is);
    
protected:
		 ITEM();

    ITEM_NAMES	 myDefinition;

    int		 myX, myY;
    int		 myCount;
    ITEM	*myNext;
};

#endif

