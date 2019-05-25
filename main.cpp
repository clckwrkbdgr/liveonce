/*
 * Licensed under BSD license.  See LICENCE.TXT  
 *
 * Produced by:	Jeff Lait
 *
 *      	7DRL Development
 *
 * NAME:        main.cpp ( Live Once Library, C++ )
 *
 * COMMENTS:
 */

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "gfxengine.h"
#include "glbdef.h"
#include "map.h"
#include "mob.h"
#include "panel.h"
#include "msg.h"
#include "text.h"
#include "speed.h"
#include "item.h"
#include "quest.h"

#include <fstream>
using namespace std;

void
save_stuff()
{
    // I hate streams with a passion.
#ifdef WIN32
    ofstream	os("valley.sav", ios::out | ios::binary);
#else
    ofstream	os("valley.sav");
#endif

    // Save avatar.
    MOB_NAMES	 avatar = MOB_NONE;
    int		 val;

    if (MOB::getAvatar())
	avatar = MOB::getAvatar()->getDefinition();

    val = avatar;
    os.write((const char *) &val, sizeof(int));

    quest_save(os);

    MAP		*map;
    u8		 c;

    for (map = MAP::getMainMap(); map; map = map->getRawDownMap())
    {
	c = 1;
	os.write((const char *) &c, 1);
	map->save(os);
    }
    c = 0;
    os.write((const char *) &c, 1);
}

bool
load_stuff()
{
    // Destroy any existing maps...
    MOB::setAvatar(0);
    if (MAP::getMainMap())
    {
	delete MAP::getMainMap();
	MAP::setMainMap(0);
    }

    // Open file for reading.
#ifdef WIN32
    ifstream	is("valley.sav", ios::in | ios::binary);
#else
    ifstream	is("valley.sav");
#endif

    if (!is)
	return false;

    // Load avatar name.
    MOB_NAMES		avatarname;
    int			val;
    u8			c;
    
    is.read((char *) &val, sizeof(int));
    avatarname = (MOB_NAMES) val;

    quest_load(is);

    // Load the maps...
    MAP			*map, *lastmap;

    lastmap = 0;
    while (1)
    {
	is.read((char *) &c, 1);
	if (!c)
	    break;

	map = new MAP;
	map->load(is);

	if (lastmap)
	    lastmap->setDownMap(map);
	else
	    MAP::setMainMap(map);

	lastmap = map;
    }

    // Find the avatar...
    MOB		*mob;
    for (map = MAP::getMainMap(); map; map = map->getRawDownMap())
    {
	for (mob = map->getMobHead(); mob; mob = mob->getNext())
	{
	    if (mob->getDefinition() == avatarname)
	    {
		MOB::setAvatar(mob);
		break;
	    }
	}
	if (MOB::getAvatar())
	    break;
    }

    return true;
}

bool
getdirection(int key, int &dx, int &dy)
{
    dx = dy = 0;

    switch (key)
    {
	case 'y':
	case '7':
	    dx = -1;
	    dy = -1;
	    break;
	case 'u':
	case '9':
	    dx = 1;
	    dy = -1;
	    break;
	case '4':
	case 'h':
	case GFX_KEYLEFT:
	    dx = -1;
	    dy = 0;
	    break;
	case '2':
	case 'j':
	case GFX_KEYDOWN:
	    dx = 0;
	    dy = 1;
	    break;
	case '8':
	case 'k':
	case GFX_KEYUP:
	    dx = 0;
	    dy = -1;
	    break;
	case '6':
	case 'l':
	case GFX_KEYRIGHT:
	    dx = 1;
	    dy = 0;
	    break;
	case 'b':
	case '1':
	    dx = -1;
	    dy = 1;
	    break;
	case 'n':
	case '3':
	    dx = 1;
	    dy = 1;
	    break;
	case '5':
	case '.':
	    dx = 0;
	    dy = 0;
	    break;
	    
	default:
	    // No valid direction!
	    return false;
    }

    return true;
}

void
writestatusline(PANEL *status)
{
    MOB		*avatar;

    avatar = MOB::getAvatar();

    status->clear();
    if (!avatar)
    {
	status->appendText("You are Dead.");
    }
    else
    {
	char	buf[100];

	sprintf(buf, "You are %s.", avatar->getRawName());
	status->appendText(buf);
	status->newLine();
	sprintf(buf, "Hits: %d", avatar->getHP());
	status->appendText(buf);
    }
}

int
main()
{
    gfx_init();
    text_init();
    spd_init();
    quest_init();

    MAP		*mainmap;
    int		key;
    MOB		*avatar;

    if (load_stuff())
    {
	mainmap = MAP::getMainMap();

	// On successful load, delete the save game.
	unlink("valley.sav");
    }
    else
    {
	mainmap = new MAP;
	// map->build(0);
	mainmap->loadMap("valley.map");

	MAP::setMainMap(mainmap);
    }

    gfx_clearbox(0, 0, 80, 30, ' ', ATTR_BORDER);

    PANEL	output(79 - 27, 28);
    PANEL	status(25, 2);
    output.move(27, 1);
    status.move(1, 27);

    msg_registerpanel(&output);

    int		dx, dy;
    bool	didmove;
    bool	hasquit = false;

    while (!hasquit)
    {
	avatar = MOB::getAvatar();

	if (!avatar)
	{
	    MOB_NAMES	playlist[] = {
		MOB_TIMMY,
		MOB_BAKER,
		MOB_MRSBAKER,
		MOB_FARMERAMY,
		MOB_FARMERJOE,
		MOB_MRSSMITH,
		MOB_SMITH,
		MOB_ELDER,
		MOB_NONE };

	    int	i;
		    
	    // Break keyrepeat & the keyboard buffer.
	    gfx_breakKeyRepeat();
	    gfx_clearKeyBuffer();

	    // Find next avatar on top level.
	    i = 0;
	    while (1)
	    {
		if (playlist[i] == MOB_NONE)
		    break;
		
		avatar = mainmap->findMob(playlist[i]);
		
		// Verify we are open for the quests.
		if (glb_mobdefs[playlist[i]].quest_shroom)
		{
		    if (quest_doneShrooms())
		    {
			avatar = 0;
		    }
		}
		if (glb_mobdefs[playlist[i]].quest_timmy)
		{
		    if (quest_foundTimmy())
			avatar = 0;
		}

		if (avatar)
		{
		    break;
		}
		
		i++;
	    }

	    if (!avatar)
	    {
		// Quit the game!
		msg_report("\n\n");
		msg_report(text_lookup("game", "lose"));

		msg_report("Hit Q to Quit.");
		output.redraw();
		status.redraw();
		gfx_update();
		while (gfx_getKey() != 'Q');
		break;
	    }
	    
	    MOB::setAvatar(avatar);
	    msg_report(text_lookup("welcome", avatar->getRawName()));

	    // Each new avatar gets a new speech from TLosh
	    quest_setTloshSpoken(false);
	}

	msg_newturn();

	didmove = false;

	avatar->getMap()->buildFOV(avatar->getX(), avatar->getY());
	avatar->getMap()->redraw(1, 1, 25, 25, avatar->getX(), avatar->getY());
	writestatusline(&status);

	output.redraw();
	status.redraw();

	gfx_update();

	// Check for force move.
	didmove = avatar->aiForcedAction();
	
	if (!didmove)
	    key = gfx_getKey();
	else
	    key = 0;

	if (getdirection(key, dx, dy))
	{
	    didmove = avatar->actionBump(dx, dy);
	}

	dx = dy = 0;
	switch (key)
	{
	    case '>':
	    case '<':
		didmove = avatar->actionClimb();
		break;

	    case 'w':
	    {
		msg_report("Welcome Message:\n\n");
		msg_report(text_lookup("welcome", avatar->getRawName()));
		break;
	    }

	    case 'c':
	    case 'i':
	    {
		char		buf[100];
		msg_report("Your character sheet:\n");
		sprintf(buf, "Name: %s\n", avatar->getRawName());
		msg_report(buf);
		sprintf(buf, "Melee Weapon: %s\n", glb_mobdefs[avatar->getDefinition()].melee_name);
		msg_report(buf);
		sprintf(buf, "Ranged Weapon: %s\n", glb_mobdefs[avatar->getDefinition()].range_name);
		msg_report(buf);

		ITEM		*inv;

		inv = avatar->getInventory();
		if (inv)
		{
		    msg_report("You are carrying:\n");
		    while (inv)
		    {
			msg_format("  %S\n", inv);
			inv = inv->getNext();
		    }
		}

		break;
	    }

	    case 'f':
	    {
		msg_report("Fire in what direction?  ");
		output.redraw();
		gfx_update();
		key = gfx_getKey();
		if (getdirection(key, dx, dy))
		{
		    didmove = avatar->actionFire(dx, dy);
		}
		else
		{
		    msg_report("You decide not to fire anything.  ");
		}
		key = 0;
		break;
	    }

	    case 'Q':
	    {
		msg_report("Saving...  ");
		output.redraw();
		gfx_update();

		save_stuff();

		msg_report("Done.  Hit a key to quit.  ");
		output.redraw();
		gfx_update();

		gfx_breakKeyRepeat();
		gfx_clearKeyBuffer();

		key = gfx_getKey();
		key = 0;
		hasquit = true;
		break;
	    }

	    case 'x':
	    {
		int		x, y;
		
		x = avatar->getX();
		y = avatar->getY();
		
		// Examine...
		while (1)
		{
		    // Highlight current position
		    avatar->getMap()->redraw(1, 1, 25, 25, x, y);

		    u8		sym;
		    ATTR_NAMES  attr;

		    gfx_getchar(13, 13, sym, attr);
		    attr = ATTR_HILITE;
		    gfx_printchar(13, 13, sym, attr);

		    output.redraw();
		    status.redraw();
		    
		    gfx_update();

		    // Get a direction & apply it.
		    key = gfx_getKey();
		    
		    // Exit if bad...
		    if (!getdirection(key, dx, dy))
			break;

		    x += dx;
		    y += dy;

		    avatar->getMap()->describeSquare(x, y);
		    
		    msg_newturn();
		}
		key = 0;
		// Screen restored for us...
		break;
	    }

	    case '?':
		msg_report("Help:\n");
		msg_report(text_lookup("game", "help"));
		break;

	    case 'a':
		msg_report("About:\n");
		msg_report(text_lookup("game", "about"));
		break;
	}

	if (didmove)
	{
	    avatar->getMap()->doMoveNPC();
	    spd_inctime();
	}
    }

    text_shutdown();
    gfx_shutdown();

    return 0;
}
