/*
 * Licensed under BSD license.  See LICENCE.TXT  
 *
 * Produced by:	Jeff Lait
 *
 *      	7DRL Development
 *
 * NAME:        mob.cpp ( Live Once Library, C++ )
 *
 * COMMENTS:
 */

#include "mob.h"

#include "map.h"
#include "msg.h"
#include "text.h"
#include "speed.h"
#include "item.h"
#include "gfxengine.h"
#include "quest.h"

#include <iostream>
using namespace std;

//
// MOB Implementation
//

MOB *MOB::theAvatar = 0;

MOB::MOB()
{
    myX = myY = -1;
    myTX = myTY = -1;
    myNext = 0;
    myMap = 0;
    myHX = myHY = -1;
    myFleeCount = 0;
    myHP = 0;
    myInventory = 0;
}

MOB::~MOB()
{
    if (this == theAvatar)
	setAvatar(0);
	
    delete myNext;
    delete myInventory;
}

MOB *
MOB::create(MOB_NAMES def)
{
    MOB		*mob;

    mob = new MOB();

    mob->myDefinition = def;

    mob->myHP = glb_mobdefs[def].max_hp;

    return mob;
}

MOB *
MOB::createNPC(int depth)
{
    int		i;
    MOB_NAMES	mob = MOB_NONE;
    int		choice = 0;

    for (i = 0; i < NUM_MOBS; i++)
    {
	// Stuff with 0 depth is never created manually.
	if (!glb_mobdefs[i].depth)
	    continue;

	if (glb_mobdefs[i].depth <= depth)
	{
	    choice++;
	    if (!rand_choice(choice))
		mob = (MOB_NAMES) i;
	}
    }

    return MOB::create(mob);
}

void
MOB::getLook(u8 &symbol, ATTR_NAMES &attr) const
{
    symbol = glb_mobdefs[myDefinition].symbol;
    attr = (ATTR_NAMES) glb_mobdefs[myDefinition].attr;
}

const char *
MOB::getName() const
{
    if (isAvatar())
	return "you";
    return glb_mobdefs[myDefinition].name;
}

const char *
MOB::getRawName() const
{
    return glb_mobdefs[myDefinition].name;
}

bool
MOB::canMove(int x, int y, bool checkmob) const
{
    MAP		*map = getMap();

    // Can move anywhere on non-maps.
    if (!map)
	return true;

    if (!map->isPassable(x, y))
	return false;

    if (checkmob && map->getMob(x, y))
	return false;

    return true;
}

void
MOB::move(int x, int y)
{
    myX = x;
    myY = y;
}

bool
MOB::applyDamage(int hits)
{
    if (hits >= getHP())
    {
	// Death!
	msg_format("%S <die>!", this);

	// If we are the avatar, special stuff happens.
	if (isAvatar())
	{
	    if (getDefinition() == MOB_FARMERAMY)
	    {
		// Would be bad continuity of Treska dies!
		msg_format("Your ruby necklace glows brightly!", this);
		msg_format("You come back to life!", this);
		msg_format("\n\nThat was too close.", this);
		msg_format("\n\nYou decide to halt this ethnographic study until summer.", this);
		msg_format("You cast Recall and teleport back to your Tower and await news of what will happen to the village.", this);
		msg_update();
		gfx_update();
	    }
	    else
	    {
		msg_update();
		getMap()->fadeToBlack();
	    }

	    msg_report("\nHit any key to continue...");
	    msg_update();
	    gfx_update();

	    // If it was Timmy that died, we want to make the zombie.
	    if (getDefinition() == MOB_TIMMY)
	    {
		MAP		*map;
		MOB		*mob;
		int		 x, y;

		map = MAP::getMainMap();
		while (map->getDepth() < 6)
		    map = map->getDownMap();
		map->findTile(TILE_FLOOR, x, y, true);
		mob = MOB::create(MOB_ZOMBIETIMMY);
		mob->move(x, y);
		map->addMob(mob);
		quest_setTimmyDead(true);
	    }
	    
	    gfx_breakKeyRepeat();
	    gfx_clearKeyBuffer();

	    gfx_getKey();
	    msg_report("\n\n");
	}

	if (getDefinition() == MOB_TLOSH)
	{
	    // Tlosh's death means you won!
	    msg_report("\n\n");
	    msg_report(text_lookup("win", MOB::getAvatar()->getRawName()));
		    
	    char		buf[100];
	    int			num = 1;
	    MOB			*m;
	    m = MAP::getMainMap()->getMobHead();
	    for (; m; m = m->getNext())
	    {
		num++;
	    }
	    sprintf(buf, "%s survived the ravages of Tlosh.\n",
		    gram_createcount("villager", num, true));
	    msg_report(buf);

	    msg_report("\n\nHit 'Q' to quit.");
	    msg_update();
	    gfx_update();

	    gfx_breakKeyRepeat();
	    gfx_clearKeyBuffer();

	    while (gfx_getKey() != 'Q')
	    {
	    }

	    gfx_shutdown();
	    // I have 2 hours and 45 minutes left.  Apologies
	    // for unclean exit!
	    exit(0);
	}

	// Drop any stuff we have.
	ITEM		*c, *n;
	for (c = myInventory; c; c = n)
	{
	    n = c->getNext();

	    removeItem(c);
	    c->move(getX(), getY());
	    getMap()->addItem(c);
	}
	myInventory = 0;
	
	getMap()->removeMob(this);
	delete this;
	return true;
    }

    myHP -= hits;
    return false;
}

VERB_PERSON
MOB::getPerson() const
{
    if (isAvatar())
	return VERB_YOU;

    return VERB_IT;
}

bool
MOB::isFriends(MOB *other) const
{
    if (other == this)
	return true;
    
    if (isAvatar())
    {
	if (glb_mobdefs[other->getDefinition()].isfriendly)
	    return true;
	else
	    return false;
    }

    // Only the avatar is evil!
    if (glb_mobdefs[getDefinition()].isfriendly)
    {
	return true;
    }
    else
    {
	// Monsters hate the avtar!
	if (other->isAvatar())
	    return false;
    }
    return true;
}

AI_NAMES
MOB::getAI() const
{
    return (AI_NAMES) glb_mobdefs[getDefinition()].ai;
}

bool
MOB::aiForcedAction()
{
    // Check for the phase...
    PHASE_NAMES		phase;

    phase = spd_getphase();

    switch (phase)
    {
	case PHASE_FAST:
	    // Not fast, no phase
	    if (!glb_mobdefs[getDefinition()].isfast)
		return true;
	    break;

	case PHASE_QUICK:
	    // Not quick, no phase!
	    return true;

	case PHASE_SLOW:
	    if (glb_mobdefs[getDefinition()].isslow)
		return true;
	    break;

	case PHASE_NORMAL:
	    break;
    }
    
    if (getDefinition() == MOB_FARMERJOE && isAvatar())
    {
	// Farmer Joe has been dipping too much into that old moonshine.
	if (!rand_choice(10))
	{
	    const char *drunk_msg[] = 
	    { 
		"*hiC*  ", 
		"*hic*  ", 
		"%S <stumble> drunkenly." 
	    };

	    msg_format(drunk_msg[rand_choice(3)], this);

	    aiRandomWalk();
	    return true;
	}
    }
    return false;
}

bool
MOB::aiDoAI()
{
    // Rebuild our home location.
    if (myHX < 0)
    {
	myHX = getX();
	myHY = getY();
    }

    switch (getAI())
    {
	case AI_NONE:
	    // Just stay put!
	    return true;

	case AI_STAYHOME:
	    // If we are at home stay put.
	    if (getX() == myHX && getY() == myHY)
		return true;

	    // Otherwise, go home.
	    // FALL THROUGH

	case AI_HOME:
	    // Move towards our home square.
	    int		dist;

	    dist = MAX(ABS(getX()-myHX), ABS(getY() - myHY));

	    // A good chance to just stay put.
	    if (rand_chance(70))
	    {
		return actionBump(0, 0);
	    }

	    if (rand_choice(dist))
	    {
		// Try to home.
		if (aiMoveTo(myHX, myHY))
		    return true;
	    }
	    // Default to wandering
	    return aiRandomWalk();

	case AI_CHARGE:
	    // If we can see avatar, charge at them!  Otherwise, move
	    // to our target if set.  Otherwise random walk.
	    if (getMap()->isFOV(getX(), getY()))
	    {
		MOB		*a;
		a = MOB::getAvatar();
		if (a)
		{
		    myTX = a->getX();
		    myTY = a->getY();
		}
	    }
	    if (myTX >= 0)
	    {
		// Charge to last known location.  If there, abandon
		// the search.
		if (myTX == getX() && myTY == getY())
		    myTX = -1;
		else
		{
		    if (aiMoveTo(myTX, myTY))
			return true;
		}
	    }
	    // Default to wandering
	    return aiRandomWalk();

	case AI_FLANK:
	    // Like charge, except we don't move to, but flank move.
	    if (getMap()->isFOV(getX(), getY()))
	    {
		MOB		*a;
		a = MOB::getAvatar();
		if (a)
		{
		    myTX = a->getX();
		    myTY = a->getY();
		}
	    }
	    if (myTX >= 0)
	    {
		// Charge to last known location.  If there, abandon
		// the search.
		if (myTX == getX() && myTY == getY())
		    myTX = -1;
		else
		{
		    if (aiFlankTo(myTX, myTY))
			return true;
		}
	    }
	    // Default to wandering
	    return aiRandomWalk();

	case AI_RANGE:
	    // If we can see avatar, shoot at them!
	    // We keep ourselves at range if possible, unless
	    // retreat is blocked, in which case we will melee.
	    if (getMap()->isFOV(getX(), getY()))
	    {
		MOB		*a, *v;
		int		 dist, dx, dy;
		a = MOB::getAvatar();
		if (a)
		{
		    myTX = a->getX();
		    myTY = a->getY();

		    dx = SIGN(myTX - getX());
		    dy = SIGN(myTY - getY());
		    dist = MAX(ABS(getX()-myTX), ABS(getY() - myTY));

		    if (dist == 1)
		    {
			// We are in melee range.  Try to flee.
			if (aiFleeFrom(myTX, myTY))
			    return true;

			// Failed to flee.  Kill!
			return actionMelee(dx, dy);
		    }

		    // Try ranged attack.
		    v = getMap()->traceBullet(getX(), getY(), glb_mobdefs[getDefinition()].range_range, dx, dy);
		    if (v == a)
		    {
			// Potential range attack.
			return actionFire(dx, dy);
		    }

		    // Failed range attack.  If outside of range, charge.
		    if (dist > glb_mobdefs[getDefinition()].range_range)
			if (aiMoveTo(myTX, myTY))
			    return true;

		    // Otherwise, wander within the range hoping to line up.
		    // Trying to double think the human is likely pointless
		    // as they'll be trying to avoid lining up to.
		    return aiRandomWalk();
		}
	    }
		
	    if (myTX >= 0)
	    {
		// Charge to last known location.  If there, abandon
		// the search.
		if (myTX == getX() && myTY == getY())
		    myTX = -1;
		else
		{
		    if (aiMoveTo(myTX, myTY))
			return true;
		}
	    }
	    // Default to wandering
	    return aiRandomWalk();
    }

    return false;
}

bool
MOB::aiFleeFrom(int x, int y)
{
    int		dx, dy;
    int		angle, resultangle = 0, i;
    int		choice = 0;

    dx = -SIGN(x - getX());
    dy = -SIGN(y - getY());
    
    angle = rand_dirtoangle(dx, dy);

    // 3 ones in same direction get equal chance.
    for (i = angle-1; i <= angle+1; i++)
    {
	rand_angletodir(i, dx, dy);
	if (canMove(getX()+dx, getY()+dy))
	{
	    choice++;
	    if (!rand_choice(choice))
		resultangle = i;
	}
    }

    if (!choice)
    {
	// Try a bit more desperately...
	for (i = angle-2; i <= angle+2; i += 4)
	{
	    rand_angletodir(i, dx, dy);
	    if (canMove(getX()+dx, getY()+dy))
	    {
		choice++;
		if (!rand_choice(choice))
		    resultangle = i;
	    }
	}
    }

    if (choice)
    {
	// Move in the direction
	rand_angletodir(resultangle, dx, dy);
	return actionBump(dx, dy);
    }

    // Failed
    return false;
}

bool
MOB::aiRandomWalk()
{
    int		dx, dy;
    int		angle, resultangle = 0;
    int		choice = 0;

    for (angle = 0; angle < 8; angle++)
    {
	rand_angletodir(angle, dx, dy);
	if (canMove(getX()+dx, getY()+dy))
	{
	    choice++;
	    if (!rand_choice(choice))
		resultangle = angle;
	}
    }

    if (choice)
    {
	// Move in the direction
	rand_angletodir(resultangle, dx, dy);
	return actionWalk(dx, dy);
    }

    // Failed
    return false;
}
bool
MOB::aiMoveTo(int x, int y)
{
    int		dx, dy, dist;
    int		angle, resultangle = 0, i;
    int		choice = 0;

    dx = SIGN(x - getX());
    dy = SIGN(y - getY());

    dist = MAX(ABS(x - getX()), ABS(y - getY()));

    if (dist == 1)
    {
	// Attack!
	if (canMove(x, y, false))
	    return actionBump(dx, dy);
	return false;
    }

    // Move in general direction, preferring a straight line.
    angle = rand_dirtoangle(dx, dy);

    for (i = 0; i <= 2; i++)
    {
	rand_angletodir(angle-i, dx, dy);
	if (canMove(getX()+dx, getY()+dy))
	{
	    choice++;
	    if (!rand_choice(choice))
		resultangle = angle-i;
	}
	
	rand_angletodir(angle+i, dx, dy);
	if (canMove(getX()+dx, getY()+dy))
	{
	    choice++;
	    if (!rand_choice(choice))
		resultangle = angle+i;
	}

	if (choice)
	{
	    rand_angletodir(resultangle, dx, dy);
	    return actionWalk(dx, dy);
	}
    }

    return false;
}

bool
MOB::aiFlankTo(int x, int y)
{
    int		dx, dy, dist;
    int		angle, resultangle = 0, i, j;
    int		choice = 0;

    dx = SIGN(x - getX());
    dy = SIGN(y - getY());

    dist = MAX(ABS(x - getX()), ABS(y - getY()));

    if (dist == 1)
    {
	// Attack!
	if (canMove(x, y, false))
	    return actionBump(dx, dy);
	return false;
    }

    // Move in general direction, preferring a straight line.
    angle = rand_dirtoangle(dx, dy);

    for (j = 0; j <= 2; j++)
    {
	// To flank, we prefer the non-straight approach.
	switch (j)
	{
	    case 0:
		i = 1;
		break;
	    case 1:
		i = 0;
		break;
	    case 2:
		i = 2;
		break;
	}
	rand_angletodir(angle-i, dx, dy);
	if (canMove(getX()+dx, getY()+dy))
	{
	    choice++;
	    if (!rand_choice(choice))
		resultangle = angle-i;
	}
	
	rand_angletodir(angle+i, dx, dy);
	if (canMove(getX()+dx, getY()+dy))
	{
	    choice++;
	    if (!rand_choice(choice))
		resultangle = angle+i;
	}

	if (choice)
	{
	    rand_angletodir(resultangle, dx, dy);
	    return actionWalk(dx, dy);
	}
    }

    return false;
}
bool
MOB::actionBump(int dx, int dy)
{
    int		 x, y;
    MOB		*mob;

    x = getX() + dx;
    y = getY() + dy;

    // Stand in place.
    if (!dx && !dy)
	return true;
    
    mob = getMap()->getMob(x, y);
    if (mob)
    {
	// Either kill or chat!
	if (mob->isFriends(this))
	{
	    return actionChat(dx, dy);
	}
	else
	{
	    return actionMelee(dx, dy);
	}
    }

    // No mob, see if we can move that way.
    // We let actionWalk deal with unable to move notification.
    return actionWalk(dx, dy);
}

bool
MOB::actionClimb()
{
    TILE_NAMES		tile;
    int			x, y;

    tile = getMap()->getTile(getX(), getY());

    if (tile == TILE_DOWNSTAIRS)
    {
	MAP		*newmap;
	msg_format("%S <climb> down.", this);
	newmap = getMap()->getDownMap();
	getMap()->removeMob(this);

	newmap->findTile(TILE_UPSTAIRS, x, y);
	move(x, y);

	newmap->addMob(this);

	// Take no time as otherwise you can be insta-killed!
	return false;
    }
    else if (tile == TILE_UPSTAIRS)
    {
	MAP		*newmap;

	newmap = getMap()->getUpMap();

	// No escapes from the Dungeon Of Death!  But each
	// person has their own reason..
	if (newmap->isMainMap())
	{
	    if (glb_mobdefs[getDefinition()].quest_timmy)
	    {
		if (quest_foundTimmy())
		{
		    // Allow them to climb in horror!
		    msg_format("Having seen what horrors await beneath, you climb out to alert the Village Elder.", this);

		    msg_format("\n\nHit a key to continue...\n\n", this);
		    msg_update();
		    gfx_update();
		    gfx_breakKeyRepeat();
		    gfx_clearKeyBuffer();
		    gfx_getKey();
		    MOB::setAvatar(0);

		    getMap()->removeMob(this);
		    
		    newmap->findTile(TILE_DOWNSTAIRS, x, y);
		    move(x, y);
		    
		    newmap->addMob(this);
		    return false;
		}
	    }
	    if (glb_mobdefs[getDefinition()].quest_shroom)
	    {
		if (quest_hasShrooms())
		{
		    // Allow them to climb and win!
		    quest_setDoneShrooms(true);
		    msg_format("Having collected enough mushrooms, %S <climb> up.", this);
		    msg_format("\n\nHit a key to continue...\n\n", this);
		    msg_update();
		    gfx_update();
		    gfx_breakKeyRepeat();
		    gfx_clearKeyBuffer();
		    gfx_getKey();
		    MOB::setAvatar(0);

		    getMap()->removeMob(this);
		    
		    newmap->findTile(TILE_DOWNSTAIRS, x, y);
		    move(x, y);
		    
		    newmap->addMob(this);
		    return false;
		}
	    }
	    msg_report(text_lookup("exit", getRawName()));
	    return false;
	}
	
	msg_format("%S <climb> up.", this);
	getMap()->removeMob(this);
	
	newmap->findTile(TILE_DOWNSTAIRS, x, y);
	move(x, y);
	
	newmap->addMob(this);

	// Take no time as otherwise you can be insta-killed!
	return false;
    }
    else if (tile == TILE_FOREST)
    {
	msg_format("%S <climb> a tree... and <fall> down!", this);
	return true;
    }

    msg_format("%S <see> nothing to climb here.", this);

    return false;
}

bool
MOB::actionChat(int dx, int dy)
{
    MOB		*victim;
    int		 x, y;

    x = getX() + dx;
    y = getY() + dy;

    // This should never occur, but to avoid
    // embarassaments...
    if (!isAvatar())
	return false;
    
    victim = getMap()->getMob(x, y);
    if (!victim)
    {
	// Talk to self
	msg_format("%S <talk> to %O!", this, this);
	return false;
    }

    msg_format("%S <chat> with %O:\n", this, victim);

    msg_quote(text_lookup("chat", getRawName(), victim->getRawName()));
    
    return true;
}

bool
MOB::actionMelee(int dx, int dy)
{
    MOB		*victim;
    int		 x, y;

    x = getX() + dx;
    y = getY() + dy;
    
    victim = getMap()->getMob(x, y);
    if (!victim)
    {
	// Swing in air!
	msg_format("%S <swing> at empty air!", this);
	return false;
    }

    // Attack victim viciously!
    msg_format("%S %v %O.", this, glb_mobdefs[getDefinition()].melee_verb,
			    victim);
    victim->applyDamage(glb_mobdefs[getDefinition()].melee_damage);
    return true;
}

bool
MOB::actionFire(int dx, int dy)
{
    MOB		*victim;
 
    // Check for no ranged weapon.
    if (!glb_mobdefs[getDefinition()].range_damage)
    {
	msg_format("%S <have> no ranged weapon!", this);
	return false;
    }

    // No suicide.
    if (!dx && !dy)
    {
	msg_format("%S <decide> not to aim at %O.", this, this);
	return false;
    }

    // Check for friendly kill.
    victim = getMap()->traceBullet(getX(), getY(), glb_mobdefs[getDefinition()].range_range, dx, dy);

    if (victim && victim->isFriends(this))
    {
	// Not a clear shot!
	if (isAvatar())
	{
	    msg_report(text_lookup("fire", getRawName()));
	    // We have special messages.
	    return false;
	}

	// Avoid friendly fire
	return false;
    }

    getMap()->displayBullet(getX(), getY(), 
			glb_mobdefs[getDefinition()].range_range,
			dx, dy,
			glb_mobdefs[getDefinition()].range_symbol, 
			(ATTR_NAMES) glb_mobdefs[getDefinition()].range_attr);

    if (!victim)
    {
	// Shoot at air!
	msg_format("%S <fire> at empty air!", this);
	return true;
    }

    // Attack victim viciously!
    msg_format("%S %v %O.", this, glb_mobdefs[getDefinition()].range_verb,
			    victim);
    victim->applyDamage(glb_mobdefs[getDefinition()].range_damage);
    return true;
}

bool
MOB::actionWalk(int dx, int dy)
{
    int		 x, y;
    MOB		*victim;
    
    x = getX() + dx;
    y = getY() + dy;
    
    victim = getMap()->getMob(x, y);
    if (victim)
    {
	msg_format("%S <bump> into %O.", this, victim);
	// Bump into the mob.
	return false;
    }

    // Check to see if we can move that way.
    if (canMove(x, y))
    {
	// Move!
	move(x, y);
	
	// If we are the avatar and something is here, pick it up.
	if (isAvatar() && getMap()->getItem(x, y))
	    actionPickup();
	
	return true;
    }
    else
    {
	msg_format("%S <be> blocked by %O.", this, getMap()->getTerrainName(x, y));
	// Bump into a wall.
	return false;
    }
}

bool
MOB::actionPickup()
{
    ITEM		*item;

    item = getMap()->getItem(getX(), getY());

    if (!item)
    {
	msg_format("%S <grope> the ground foolishly.", this);
	return false;
    }

    // Pick up the item!
    msg_format("%S <pick> up %O.", this, item);

    getMap()->removeItem(item);

    addItem(item);

    return true;
}

void
MOB::addItem(ITEM *item)
{
    ITEM		*c;
    
    assert(!item->getNext());

    // First, check to see if we can merge...
    for (c = myInventory; c; c = c->getNext())
    {
	if (item->canStackWith(c))
	{
	    c->incStackCount(item->getStackCount());
	    delete item;
	    return;
	}
    }

    // Brand new item.
    item->setNext(myInventory);
    myInventory = item;
}

void
MOB::removeItem(ITEM *item)
{
    ITEM		*c, *l;

    l = 0;
    for (c = myInventory; c; c = c->getNext())
    {
	if (c == item)
	{
	    if (l)
		l->setNext(c->getNext());
	    else
		myInventory = c->getNext();
	    c->setNext(0);
	    return;
	}
	l = c;
    }
    assert(0);
}

void
MOB::save(ostream &os) const
{
    int		val;

    val = myDefinition;
    os.write((const char *) &val, sizeof(int));

    os.write((const char *) &myX, sizeof(int));
    os.write((const char *) &myY, sizeof(int));

    os.write((const char *) &myTX, sizeof(int));
    os.write((const char *) &myTY, sizeof(int));

    os.write((const char *) &myHX, sizeof(int));
    os.write((const char *) &myHY, sizeof(int));

    os.write((const char *) &myHP, sizeof(int));

    ITEM		*i;
    u8			 c;

    for (i = myInventory; i; i = i->getNext())
    {
	c = 1;
	os.write((const char *) &c, 1);

	i->save(os);
    }
    c = 0;
    os.write((const char *) &c, 1);
}

MOB *
MOB::load(istream &is)
{
    int		 val;
    MOB		*mob;
    u8		 c;

    mob = new MOB();

    is.read((char *)&val, sizeof(int));
    mob->myDefinition = (MOB_NAMES) val;

    is.read((char *)&mob->myX, sizeof(int));
    is.read((char *)&mob->myY, sizeof(int));

    is.read((char *)&mob->myTX, sizeof(int));
    is.read((char *)&mob->myTY, sizeof(int));

    is.read((char *)&mob->myHX, sizeof(int));
    is.read((char *)&mob->myHY, sizeof(int));

    is.read((char *)&mob->myHP, sizeof(int));

    ITEM		*i;

    while (1)
    {
	is.read((char *) &c, 1);
	if (!c)
	    break;

	i = ITEM::load(is);
	mob->addItem(i);
    }

    return mob;
}
