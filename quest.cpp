/*
 * Licensed under BSD license.  See LICENCE.TXT  
 *
 * Produced by:	Jeff Lait
 *
 *      	7DRL Development
 *
 * NAME:        quest.cpp ( live Once Library, C++ )
 *
 * COMMENTS:
 */

#include "quest.h"

#include "glbdef.h"
#include "mob.h"
#include "item.h"

bool	glbDoneShrooms, glbFoundTimmy, glbTimmyDead, glbTloshSpoke;

void
quest_init()
{
    glbDoneShrooms = false;
    glbFoundTimmy = false;
    glbTimmyDead = false;
    glbTloshSpoke = false;
}

void
quest_save(ostream &os)
{
    os.write((const char *) &glbDoneShrooms, sizeof(bool));
    os.write((const char *) &glbFoundTimmy, sizeof(bool));
    os.write((const char *) &glbTimmyDead, sizeof(bool));
    os.write((const char *) &glbTloshSpoke, sizeof(bool));
}

void
quest_load(istream &is)
{
    is.read((char *) &glbDoneShrooms, sizeof(bool));
    is.read((char *) &glbFoundTimmy, sizeof(bool));
    is.read((char *) &glbTimmyDead, sizeof(bool));
    is.read((char *) &glbTloshSpoke, sizeof(bool));
}

bool
quest_hasShrooms()
{
    MOB		*a;
    ITEM	*i;
    int		 c;

    a = MOB::getAvatar();
    if (!a)
	return false;
    
    c = 0;
    for (i = a->getInventory(); i; i = i->getNext())
    {
	if (i->getDefinition() == ITEM_MUSHROOM)
	    c += i->getStackCount();
    }

    // Need at least 5 shrooms.
    return (c >= 5);
}

bool
quest_doneShrooms()
{
    return glbDoneShrooms;
}

void
quest_setDoneShrooms(bool val)
{
    glbDoneShrooms = val;
}

bool
quest_foundTimmy()
{
    return glbFoundTimmy;
}

void
quest_setFoundTimmy(bool val)
{
    glbFoundTimmy = val;
}

bool
quest_TimmyDead()
{
    return glbTimmyDead;
}

void
quest_setTimmyDead(bool val)
{
    glbTimmyDead = val;
}

bool
quest_hasTloshSpoken()
{
    return glbTloshSpoke;
}

void
quest_setTloshSpoken(bool val)
{
    glbTloshSpoke = val;
}
