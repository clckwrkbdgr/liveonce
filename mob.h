/*
 * Licensed under BSD license.  See LICENCE.TXT  
 *
 * Produced by:	Jeff Lait
 *
 *      	7DRL Development
 *
 * NAME:        mob.h ( Live Once Library, C++ )
 *
 * COMMENTS:
 */

#ifndef __mob__
#define __mob__

#include "glbdef.h"

#include "grammar.h"

#include <iostream>
using namespace std;

class MAP;
class ITEM;

class MOB
{
public:
    static MOB		*create(MOB_NAMES def);

    static MOB		*createNPC(int depth);
    
    ~MOB();

    int			 getX() const { return myX; }
    int			 getY() const { return myY; }

    int			 getHP() const { return myHP; }

    // Returns true if we die!
    bool		 applyDamage(int hp);

    MOB_NAMES		 getDefinition() const { return myDefinition; }

    bool		 isAvatar() const { return this == getAvatar(); }
    static MOB		*getAvatar() { return theAvatar; }
    static void		 setAvatar(MOB *avatar) { theAvatar = avatar; };

    VERB_PERSON		 getPerson() const;

    // Symbol and attributes for rendering.
    void		 getLook(u8 &symbol, ATTR_NAMES &attr) const;

    // Retrieve the raw name.
    const char		*getName() const;
    const char		*getRawName() const;

    MAP			*getMap() const { return myMap; }
    static MAP		*getAvatarMap() { theAvatar ? theAvatar->getMap() : 0; }

    ITEM		*getInventory() const { return myInventory; }

    bool		 canMove(int x, int y, bool checkmob = true) const;

    void		 move(int x, int y);

    bool		 isFriends(MOB *other) const;

    void		 addItem(ITEM *item);
    void		 removeItem(ITEM *item);

    //
    // High level AI functions.
    //

    AI_NAMES		 getAI() const;

    // Determines if there are any mandatory actions to be done.
    // Return true if a forced action occured, in which case the
    // player gets no further input.
    bool		 aiForcedAction();

    // Runs the normal AI routines.  Called if aiForcedAction failed
    // and not the avatar.
    bool		 aiDoAI();

    // Runs away from (x, y).  Return true if action taken.
    bool		 aiFleeFrom(int x, int y);

    // Runs straight towards (x, y).  Return true if action taken.
    bool		 aiMoveTo(int x, int y);

    // Tries to go to (x, y) by flanking them.
    bool		 aiFlankTo(int x, int y);
    
    bool		 aiRandomWalk();

    // Action methods.  These are how the AI and user manipulates
    // mobs.
    // Return true if the action consumed a turn, else false.
    bool		 actionBump(int dx, int dy);
    bool		 actionChat(int dx, int dy);
    bool		 actionMelee(int dx, int dy);
    bool		 actionWalk(int dx, int dy);
    bool		 actionFire(int dx, int dy);
    bool		 actionPickup();

    bool		 actionClimb();

    // The following are used at the map level..
    MOB			*getNext() const { return myNext; }
    void		 setNext(MOB *next) { myNext = next; }
    void		 setMap(MAP *map) { myMap = map; }

    void		 save(ostream &os) const;
    static MOB		*load(istream &is);
    
protected:
    MOB();

    MOB_NAMES		 myDefinition;
    static MOB		*theAvatar;

    int			 myX, myY;
    MOB			*myNext;

    ITEM		*myInventory;

    // Current target
    int			 myTX, myTY;
    int			 myFleeCount;

    // My home spot.
    int			 myHX, myHY;

    // Hitpoints
    int			 myHP;

    MAP			*myMap;
};

#endif

