/*
 * Licensed under BSD license.  See LICENCE.TXT  
 *
 * Produced by:	Jeff Lait
 *
 *      	7DRL Development
 *
 * NAME:        item.cpp ( Live Once Library, C++ )
 *
 * COMMENTS:
 */

#include "item.h"

#include <assert.h>

//
// Item Definitions
//

ITEM::ITEM()
{
    myX = myY = -1;
    myDefinition = (ITEM_NAMES) 0;
    myNext = 0;
    myCount = 1;
}

ITEM::~ITEM()
{
    delete myNext;
}

ITEM *
ITEM::create(ITEM_NAMES item)
{
    ITEM	*i;

    i = new ITEM();

    i->myDefinition = item;

    return i;
}

VERB_PERSON
ITEM::getPerson() const
{
    return VERB_IT;
}

const char *
ITEM::getName() const
{
    return gram_createcount(glb_itemdefs[getDefinition()].name, myCount, false);
}

void
ITEM::getLook(u8 &symbol, ATTR_NAMES &attr) const
{
    symbol = glb_itemdefs[getDefinition()].symbol;
    attr = (ATTR_NAMES) glb_itemdefs[getDefinition()].attr;
}

void
ITEM::move(int x, int y)
{
    myX = x;
    myY = y;
}

bool
ITEM::canStackWith(const ITEM *stack) const
{
    if (getDefinition() != stack->getDefinition())
	return false;

    // No reason why not...
    return true;
}

void
ITEM::incStackCount(int inc)
{
    myCount += inc;
    assert(myCount >= 0);
    if (myCount < 0)
	myCount = 0;
}

void
ITEM::save(ostream &os) const
{
    int		val;

    val = myDefinition;
    os.write((const char *) &val, sizeof(int));

    os.write((const char *) &myX, sizeof(int));
    os.write((const char *) &myY, sizeof(int));

    os.write((const char *) &myCount, sizeof(int));
}

ITEM *
ITEM::load(istream &is)
{
    int		val;
    ITEM	*i;

    i = new ITEM();

    is.read((char *)&val, sizeof(int));
    i->myDefinition = (ITEM_NAMES) val;

    is.read((char *)&i->myX, sizeof(int));
    is.read((char *)&i->myY, sizeof(int));

    is.read((char *)&i->myCount, sizeof(int));

    return i;
}
