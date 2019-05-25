/*
 * Licensed under BSD license.  See LICENCE.TXT  
 *
 * Produced by:	Jeff Lait
 *
 *      	7DRL Development
 *
 * NAME:        map.cpp ( Live Once Library, C++ )
 *
 * COMMENTS:
 */

#include "map.h"
#include "gfxengine.h"
#include "mob.h"
#include "msg.h"
#include "item.h"
#include "quest.h"
#include "text.h"

#include <assert.h>

#include <iostream>
#include <fstream>
using namespace std;

#define CODE_TOP 0
#define CODE_RIGHT 1
#define CODE_BOT 2
#define CODE_LEFT 3

#define SOLID_CODE 14

#define MAPFLAG_FOV 1
#define MAPFLAG_MAP 2

MAP *MAP::theMainMap = 0;

//
// Stores the legend information
//
class LEGEND
{
public:
	     LEGEND(u8 symbol, const char *descr);
    virtual ~LEGEND();

    const char	*getDescr() const { return myDescr; }
    u8		 getSymbol() const { return mySymbol; }

protected:
    u8		 mySymbol;
    char	*myDescr;
};

LEGEND::LEGEND(u8 symbol, const char *descr)
{
    // Strip leading spaces.
    while (ISSPACE(*descr))
	descr++;

    mySymbol = symbol;
    myDescr = strdup(descr);
}

LEGEND::~LEGEND()
{
    free(myDescr);
}

//
// Each of these is a 5x5 tile piece.
//
class PIECE
{
public:
    PIECE(u8 map[5][5], int sym);
    ~PIECE();

    bool	doesMatch(int top, int right, int bot, int left) const;
    int		getCode(int side) const;
    bool	codeSet(int x, int y) const;

    void	writeToMap(MAP *map, int mx, int my) const;

    PIECE	*getNext() const { return myNext; }
    void	setNext(PIECE *piece) { myNext = piece; }
    
protected:
    u8		myMap[5][5];

    PIECE	*myNext;
};

PIECE::PIECE(u8 map[5][5], int sym)
{
    int		x, y, rx, ry, t;

    for (x = 0; x < 5; x++)
    {
	for (y = 0; y < 5; y++)
	{
	    if (sym & 1)
		rx = x;
	    else
		rx = 4 - x;
	    if (sym & 2)
		ry = y;
	    else
		ry = 4 - y;
	    if (sym & 4)
	    {
		t = rx;
		rx = ry;
		ry = t;
	    }

	    myMap[rx][ry] = map[x][y];
	}
    }
	
    myNext = 0;
}

PIECE::~PIECE()
{
    delete myNext;
}

bool
PIECE::doesMatch(int top, int right, int bot, int left) const
{
    if (top != -1 && getCode(CODE_TOP) != top)
	return false;
    if (right != -1 && getCode(CODE_RIGHT) != right)
	return false;
    if (bot != -1 && getCode(CODE_BOT) != bot)
	return false;
    if (left != -1 && getCode(CODE_LEFT) != left)
	return false;

    return true;
}

int
PIECE::getCode(int side) const
{
    int		code = 0;
    int		x, y;

    switch (side)
    {
	case CODE_TOP:
	    y = 0;
	    for (x = 0; x < 5; x++)
	    {
		code <<= 1;
		if (codeSet(x, y))
		    code |= 1;
	    }
	    break;
	case CODE_RIGHT:
	    x = 4;
	    for (y = 0; y < 5; y++)
	    {
		code <<= 1;
		if (codeSet(x, y))
		    code |= 1;
	    }
	    break;
	case CODE_BOT:
	    y = 4;
	    for (x = 0; x < 5; x++)
	    {
		code <<= 1;
		if (codeSet(x, y))
		    code |= 1;
	    }
	    break;
	case CODE_LEFT:
	    x = 0;
	    for (y = 0; y < 5; y++)
	    {
		code <<= 1;
		if (codeSet(x, y))
		    code |= 1;
	    }
	    break;
	default:
	    assert(0);
	    break;
    }

    // We cannot care about corners!
    code &= SOLID_CODE;

    return code;
}

bool
PIECE::codeSet(int x, int y) const
{
    if (myMap[y][x] == '#')
	return true;

    return false;
}

void
PIECE::writeToMap(MAP *map, int mx, int my) const
{
    int		x, y;

    for (y = 0; y < 5; y++)
    {
	for (x = 0; x < 5; x++)
	{
	    if (myMap[y][x] == '#')
		map->setTile(mx+x, my+y, TILE_WALL);
	    else
		map->setTile(mx+x, my+y, TILE_FLOOR);
	}
    }
}

class PIECEDICT
{
public:
    PIECEDICT();
    ~PIECEDICT();
    
    PIECE	*getRandomPiece(int top, int right, int bot, int left);
    void	 append(PIECE *piece);

protected:
    PIECE	*myPieces;
};

PIECEDICT::PIECEDICT()
{
    char		buf[100];
    ifstream		is("piecelist.map");
    u8			map[5][5];
    int			x, y, i;

    myPieces = 0;

    while (is.getline(buf, 100))
    {
	if (buf[0] == '+')
	{
	    y = 0;
	    while (is.getline(buf, 100))
	    {
		if (buf[0] == '+')
		    break;
		if (y < 5 && buf[0] == '|')
		{
		    for (x = 0; x < 5; x++)
		    {
			if (!buf[x+1])
			    break;
			map[y][x] = buf[x+1];
		    }
		    y++;
		}
	    }

	    // Got a new map.  Build the pieces.
	    for (i = 0; i < 8; i++)
	    {
		append(new PIECE(map, i));
	    }
	}
    }
}

PIECEDICT::~PIECEDICT()
{
    delete myPieces;
}

PIECE *
PIECEDICT::getRandomPiece(int top, int right, int bot, int left)
{
    int		 nummatch = 0;
    PIECE	*cur = 0, *match = 0;
    
    for (cur = myPieces; cur; cur = cur->getNext())
    {
	if (cur->doesMatch(top, right, bot, left))
	{
	    nummatch++;

	    if (!rand_choice(nummatch))
		match = cur;
	}	
    }

    return match;
}

void
PIECEDICT::append(PIECE *piece)
{
    piece->setNext(myPieces);
    myPieces = piece;
}

//
// MAP implementation
//
MAP::MAP()
{
    myWidth = myHeight = 0;
    myTiles = 0;
    myTileFlags = 0;
    myMobs = 0;
    myItems = 0;

    myDownMap = myUpMap = 0;
    myMobSafeNext = 0;
    myTotalRooms = 0;
}

MAP::~MAP()
{
    delete [] myTiles;
    delete [] myTileFlags;

    delete myMobs;
    delete myItems;
}

void
MAP::loadMap(const char *fname)
{
    char		line[100];

    ifstream		is(fname);
    PTRLIST<LEGEND *>	legendlist;
    PTRLIST<char *>	linelist;
    int			w = 0, h = 0, i;
    int			x, y;
    u8			symbol;

    while (is.getline(line, 100))
    {
	// Comments.
	if (line[0] == '#' && line[1] == ' ')
	    continue;

	// Legends:
	if (line[0] && line[1] == ':')
	{
	    legendlist.append(new LEGEND(line[0], &line[2]));
	}

	if (line[0] == '+' && line[1] == '-')
	{
	    // We have start of map proper.
	    for (w = 1; line[w] == '-'; w++);

	    if (line[w] != '+')
	    {
		cerr << "Map does not have proper bounded map." << endl;
	    }

	    w--;

	    // Load the actual map lines.
	    while (is.getline(line, 100))
	    {
		if (line[0] == '+')
		    break;

		if (line[0] != '|')
		{
		    cerr << "Map line missing | prefix." << endl;
		}
		if (line[w+1] != '|')
		{
		    cerr << "Map line too short!" << endl;
		}
		line[w+1] = '\0';
		linelist.append(strdup(&line[1]));
	    }

	    h = linelist.entries();
	}
    }

    if (!w)
    {
	// No map found, abort.
	cerr << "No map segment found!" << endl;
	gfx_shutdown();
	exit(0);
    }

    // Build our map.
    myWidth = w;
    myHeight = h;

    myTiles = new u8[myWidth * myHeight];
    myTileFlags = new u8[myWidth * myHeight];
    memset(myTileFlags, 0, myWidth * myHeight);

    for (y = 0; y < myHeight; y++)
    {
	for (x = 0; x < myWidth; x++)
	{
	    symbol = linelist(y)[x];

	    parseLegend(x, y, legendlist, symbol);
	}
    }

    // Delete line list.
    for (y = 0; y < myHeight; y++)
    {
	free(linelist(y));
    }

    // Delete legend.
    for (i = 0; i < legendlist.entries(); i++)
	delete legendlist(i);
}

void
MAP::parseLegend(int x, int y, const PTRLIST<LEGEND *> &list,
		u8 symbol)
{
    // Find the legend.
    int			 i, j;
    const char		*s, *comma;
    char		*token;
    bool		 foundsymbol = false;

    for (i = 0; i < list.entries(); i++)
    {
	if (list(i)->getSymbol() == symbol)
	{
	    s = list(i)->getDescr();

	    foundsymbol = true;
	    
	    while (*s)
	    {
		for (comma = s; *comma && *comma != ','; comma++)
		{
		    if (*comma == '(')
		    {
			while (*comma && *comma != ')')
			{
			    comma++;
			}
		    }
		}
	    
		token = strdup(s);
		token[comma - s] = '\0';

		// We wish to apply token to this dude.
		if (token[0] == '(')
		{
		    // We have inheritance!
		    if (token[1] == symbol)
		    {
			cerr << "Naive infinite recursion: Try harder!" << endl;
			return;
		    }
		    parseLegend(x, y, list, token[1]);
		}
		else
		{
		    bool	madewish = false;
		    // Run through our wish engine...
		    for (j = 0; j < NUM_TILES; j++)
		    {
			if (!strcmp(glb_tiledefs[j].legend, token))
			{
			    madewish = true;
			    setTile(x, y, (TILE_NAMES) j);
			}
		    }

		    for (j = 0; j < NUM_MOBS; j++)
		    {
			if (!strcmp(glb_mobdefs[j].name, token))
			{
			    madewish = true;
			    MOB		*mob;

			    mob = MOB::create((MOB_NAMES) j);
			    mob->move(x, y);
			    addMob(mob);
			}
		    }

		    assert(madewish);
		}

		free(token);

		// Find next word.
		s = comma;
		while (*s == ',' || ISSPACE(*s))
		    s++;
	    }
	}
    }

    if (!foundsymbol)
    {
	cerr << "Did not find symbol " << symbol << endl;
    }
}

MAP *
MAP::getDownMap() const
{
    if (!myDownMap)
    {
	myDownMap = new MAP();
	myDownMap->myUpMap = (MAP *) this;
	myDownMap->build(myTotalRooms);
    }
    return myDownMap;
}

MAP *
MAP::getUpMap() const
{
    if (!myUpMap)
    {
	myUpMap = new MAP();
	myUpMap->myDownMap = (MAP *) this;
	myUpMap->build(1);
    }
    return myUpMap;
}

void
MAP::setDownMap(MAP *down)
{
    assert(!myDownMap);

    myDownMap = down;
}

void
MAP::setUpMap(MAP *up)
{
    assert(!myUpMap);

    myUpMap = up;
}

bool
MAP::findTile(TILE_NAMES tile, int &mx, int &my, bool avoidmob) const
{
    int		nummatch = 0;
    int		x, y;
    
    for (y = 0; y < myHeight; y++)
    {
	for (x = 0; x < myWidth; x++)
	{
	    if (getTile(x, y) == tile)
	    {
		if (avoidmob && getMob(x, y))
		    continue;

		nummatch++;
		if (!rand_choice(nummatch))
		{
		    mx = x;
		    my = y;
		}
	    }
	}
    }

    if (nummatch)
	return true;
    return false;
}
    

void
MAP::build(int roomssofar)
{
    // Build the world...
    int		rooms = 0;

    myWidth = 25;
    myHeight = 25;
    myTiles = new u8 [myWidth * myHeight];
    myTileFlags = new u8 [myWidth * myHeight];

    // Clear ourselves to wall.
    clear(TILE_WALL);
    clearFlag(MAPFLAG_MAP, false);
    clearFlag(MAPFLAG_FOV, false);

    PIECE		*piece[5][5];
    int			x, y, i;
    bool		newpiece;
    
    for (y = 0; y < 5; y++)
	for (x = 0; x < 5; x++)
	    piece[y][x] = 0;

    PIECEDICT		piecedict;
    int			top, right, bot, left;

    // Initialize center & grow...
    piece[2][2] = piecedict.getRandomPiece(-1, -1, -1, -1);
    
    newpiece = true;
    while (newpiece)
    {
	newpiece = false;
	for (y = 0; y < 5; y++)
	{
	    for (x = 0; x < 5; x++)
	    {
		if (piece[y][x])
		    continue;

		// See if any point continuing.
		if ((x && piece[y][x-1] && piece[y][x-1]->getCode(CODE_RIGHT) != SOLID_CODE) ||
		    (x < 4 && piece[y][x+1] && piece[y][x+1]->getCode(CODE_LEFT) != SOLID_CODE) ||	
		    (y && piece[y-1][x] && piece[y-1][x]->getCode(CODE_BOT) != SOLID_CODE) ||	
		    (y < 4 && piece[y+1][x] && piece[y+1][x]->getCode(CODE_TOP) != SOLID_CODE))
		{
		    // Get our codes.
		    if (x)
		    {
			if (piece[y][x-1])
			    left = piece[y][x-1]->getCode(CODE_RIGHT);
			else
			    left = -1;
		    }
		    else
			left = SOLID_CODE;
		    if (x < 4)
		    {
			if (piece[y][x+1])
			    right = piece[y][x+1]->getCode(CODE_LEFT);
			else
			    right = -1;
		    }
		    else
			right = SOLID_CODE;
		    if (y)
		    {
			if (piece[y-1][x])
			    top = piece[y-1][x]->getCode(CODE_BOT);
			else
			    top = -1;
		    }
		    else
			top = SOLID_CODE;
		    if (y < 4)
		    {
			if (piece[y+1][x])
			    bot = piece[y+1][x]->getCode(CODE_TOP);
			else
			    bot = -1;
		    }
		    else
			bot = SOLID_CODE;

		    piece[y][x] = piecedict.getRandomPiece(top, right, bot, left);
		    // assert(piece[y][x]);
		    if (piece[y][x])
			newpiece = true;
		}
	    }
	}
    }

    // Write out all the pieces.
    for (y = 0; y < 5; y++)
    {
	for (x = 0; x < 5; x++)
	{
	    if (piece[y][x])
	    {
		rooms++;
		piece[y][x]->writeToMap(this, x*5, y*5);
	    }
	}
    }

    // Force the outer wall to be wall...
    for (x = 0; x < myWidth; x++)
    {
	setTile(x, 0, TILE_WALL);
	setTile(x, myHeight-1, TILE_WALL);
    }
    for (y = 0; y < myHeight; y++)
    {
	setTile(0, y, TILE_WALL);
	setTile(myWidth-1, y, TILE_WALL);
    }

    // Update total room count.
    myTotalRooms = roomssofar + rooms;

    // Make up and down stairs.
    findTile(TILE_FLOOR, x, y);
    setTile(x, y, TILE_UPSTAIRS);
    if (getDepth() < 6)
    {
	findTile(TILE_FLOOR, x, y);
	setTile(x, y, TILE_DOWNSTAIRS);

	// Add two mushrooms
	findTile(TILE_FLOOR, x, y);
	ITEM		*shroom;
	shroom = ITEM::create(ITEM_MUSHROOM);
	shroom->move(x, y);
	addItem(shroom);

	findTile(TILE_FLOOR, x, y);
	shroom = ITEM::create(ITEM_MUSHROOM);
	shroom->move(x, y);
	addItem(shroom);
    }
    if (getDepth() >= 6)
    {
	// Add good old tlosh.
	findTile(TILE_FLOOR, x, y, true);
	
	MOB	*tlosh;
	tlosh = MOB::create(MOB_TLOSH);
	tlosh->move(x, y);
	addMob(tlosh);
    }

    // Add critters
    for (i = 0; i < (rooms/2); i++)
    {
	MOB		*mob;

	findTile(TILE_FLOOR, x, y, true);
	mob = MOB::createNPC(getDepth());
	// mob = MOB::create(MOB_TLOSH);
	mob->move(x, y);
	addMob(mob);
    }
}

void
MAP::clear(TILE_NAMES tile)
{
    int	    x, y;

    for (y = 0; y < myHeight; y++)
    {
	for (x = 0; x < myWidth; x++)
	{
	    setTile(x, y, tile);
	}
    }
}

void
MAP::clearFlag(int flag, bool val)
{
    int	    x, y;

    for (y = 0; y < myHeight; y++)
    {
	for (x = 0; x < myWidth; x++)
	{
	    setFlag(x, y, flag, val);
	}
    }
}

void
MAP::buildFOV(int cx, int cy)
{
    // Clear out FOV flag.
    clearFlag(MAPFLAG_FOV, false);

    // Starting with x, y, we spiral outwards so long
    // as we still have valid values.

    int		radius, maxrad;
    bool	hadfov;
    int		i, iminus;

    if (!isValidCoord(cx, cy))
	return;

    // Center is in FOV.
    setFlag(cx, cy, MAPFLAG_FOV|MAPFLAG_MAP, true);

    maxrad = MAX(myWidth, myHeight);
    for (radius = 1; radius < maxrad; radius++)
    {
	hadfov = false;
	for (i = -radius; i <= radius; i++)
	{
	    // This is i, but one step closer to the center.
	    iminus = i - SIGN(i);

	    if (isFOV(cx + i, cy + (radius-1)) ||
		isFOV(cx + iminus, cy + (radius-1)))
	    {
		if (hasLOSLoose(cx, cy, cx+i, cy+radius))
		{
		    setFlag(cx+i, cy+radius, MAPFLAG_FOV|MAPFLAG_MAP, true);
		    hadfov = true;
		}
	    }
	    if (isFOV(cx + i, cy - (radius-1)) ||
		isFOV(cx + iminus, cy - (radius-1)))
	    {
		if (hasLOSLoose(cx, cy, cx+i, cy-radius))
		{
		    setFlag(cx+i, cy-radius, MAPFLAG_FOV|MAPFLAG_MAP, true);
		    hadfov = true;
		}
	    }
	    if (isFOV(cx + (radius-1), cy + i) ||
		isFOV(cx + (radius-1), cy + iminus))
	    {
		if (hasLOSLoose(cx, cy, cx+radius, cy+i))
		{
		    setFlag(cx+radius, cy+i, MAPFLAG_FOV|MAPFLAG_MAP, true);
		    hadfov = true;
		}
	    }
	    if (isFOV(cx - (radius-1), cy + i) ||
		isFOV(cx - (radius-1), cy + iminus))
	    {
		if (hasLOSLoose(cx, cy, cx-radius, cy+i))
		{
		    setFlag(cx-radius, cy+i, MAPFLAG_FOV|MAPFLAG_MAP, true);
		    hadfov = true;
		}
	    }
	}

	// Early exit if everything in last circle was
	// out of fov.
	if (!hadfov)
	    break;
    }
}

bool
MAP::hasLOSLoose(int sx, int sy, int ex, int ey) const
{
    int		dx, dy;

    for (dy = -1; dy <= 1; dy++)
    {
	for (dx = -1; dx <= 1; dx++)
	{
	    if ((!dx && !dy) || isTransparent(sx+dx, sy+dy))
	    {
		if (hasLOS(sx+dx, sy+dy, ex, ey))
		    return true;
	    }
	}
    }

    return false;
}

bool
MAP::hasLOS(int sx, int sy, int ex, int ey) const
{
    int		cx, cy;
    int		acx, acy;
    int		x, y;

    cx = ex - sx;
    cy = ey - sy;
    acx = ABS(cx);
    acy = ABS(cy);

    // Invalid coords are never in LOS.
    if (!isValidCoord(sx, sy))
	return false;
    if (!isValidCoord(ex, ey))
	return false;
    
    // Check for trivial LOS.
    if (acx <= 1 && acy <= 1)
	return true;

    // We do a bresenham algorithm.
    // Note that we do *NOT* check the start or
    // the end coords!  Something is in LOS despite the value
    // of these coords.
    if (acx > acy)
    {
	// X major stepping.
	int		dx, dy, error;

	dx = SIGN(cx);
	dy = SIGN(cy);

	error = 0;
	
	// Take initial free step.
	error = acy;
	y = sy;
	for (x = sx + dx; x != ex; x += dx)
	{
	    if (error >= acx)
	    {
		error -= acx;
		y += dy;
	    }

	    // The current ray is between (x, y) and (x, y+dy).
	    // Thus, if error is not zero, we consider it a pass
	    // if *either* is transparent.
	    // Whoops. This gives tunnel vision!  You can tunnel
	    // through one thickness walls.
	    if (!isTransparent(x, y) ) // &&
//		(error == 0 || !isTransparent(x, y+dy)))    
		return false;

	    error += acy;
	}
    }
    else
    {
	// Y major stepping.
	int		dx, dy, error;

	dx = SIGN(cx);
	dy = SIGN(cy);

	error = 0;
	
	// Take initial free step.
	error = acx;
	x = sx;
	for (y = sy + dy; y != ey; y += dy)
	{
	    if (error >= acy)
	    {
		error -= acy;
		x += dx;
	    }

	    // The current ray is between (x, y) and (x+dx, y).
	    // Thus, if error is not zero, we consider it a pass
	    // if *either* is transparent.
	    if (!isTransparent(x, y) ) // &&
//		(error == 0 || !isTransparent(x + dx, y)))
		return false;

	    error += acx;
	}
    }

    // Nothing got in our way, so we're done.
    return true;
}

bool
MAP::isValidCoord(int x, int y) const
{
    if (x < 0 || x >= myWidth)
	return false;

    if (y < 0 || y >= myHeight)
	return false;

    return true;
}

TILE_NAMES
MAP::getTile(int x, int y) const
{
    if (!isValidCoord(x, y))
	return TILE_INVALID;
    
    return (TILE_NAMES) myTiles[y * myWidth + x];
}

void
MAP::setTile(int x, int y, TILE_NAMES tile)
{
    assert(isValidCoord(x, y));
    if (!isValidCoord(x, y))
	return;

    myTiles[y * myWidth + x] = tile;
}

bool
MAP::isPassable(int x, int y) const
{
    TILE_NAMES	tile;

    tile = getTile(x, y);

    return glb_tiledefs[tile].ispassable;
}

bool
MAP::isTransparent(int x, int y) const
{
    TILE_NAMES	tile;

    tile = getTile(x, y);

    return glb_tiledefs[tile].istransparent;
}

bool
MAP::isMapped(int x, int y) const
{
    return getFlag(x, y, MAPFLAG_MAP);
}

bool
MAP::isFOV(int x, int y) const
{
    return getFlag(x, y, MAPFLAG_FOV);
}

ITEM *
MAP::getItem(int x, int y) const
{
    ITEM		*item;

    for (item = myItems; item; item = item->getNext())
    {
	if (item->getX() == x && item->getY() == y)
	    return item;
    }
    return 0;
}

MOB *
MAP::getMob(int x, int y) const
{
    MOB		*mob;

    for (mob = myMobs; mob; mob = mob->getNext())
    {
	if (mob->getX() == x && mob->getY() == y)
	    return mob;
    }
    return 0;
}

MOB *
MAP::findMob(MOB_NAMES def) const
{
    MOB		*mob;

    for (mob = myMobs; mob; mob = mob->getNext())
    {
	if (mob->getDefinition() == def)
	    return mob;
    }
    return 0;
}

MOB *
MAP::traceBullet(int x, int y, int range, int dx, int dy) const
{
    MOB		*hit = 0;

    x += dx;
    y += dy;
    while (isValidCoord(x, y))
    {
	hit = getMob(x, y);
	if (hit)
	    break;

	if (!isTransparent(x, y))
	    break;
	
	// Move one step
	x += dx;
	y += dy;
	range--;
	if (range <= 0)
	    break;
    }

    return hit;
}

void
MAP::displayBullet(int x, int y, int range, int dx, int dy, u8 sym, ATTR_NAMES attr) const
{
    MOB		*hit = 0;
    int		 sx, sy;

    x += dx;
    y += dy;
    while (isValidCoord(x, y))
    {
	hit = getMob(x, y);

	if (!isTransparent(x, y))
	    break;

	// No delay for out of sight changes.
	if (isFOV(x, y))
	{
	    re_redraw();

	    // Plot our char.
	    sx = x + myLastX - myLastMX;
	    sy = y + myLastY - myLastMY;
	    // Ensure on screen...
	    if (sx >= myLastX && sx < myLastX + myLastW &&
		sy >= myLastY && sy < myLastY + myLastH)
	    {
		gfx_printchar(sx, sy, sym, attr);
	    }
	    
	    gfx_update();
	    gfx_delay(5);
	}
	
	// Move one step
	x += dx;
	y += dy;
	range--;
	if (range <= 0)
	    break;
	if (hit)
	    break;
    }

    // clear up display.
    re_redraw();
    gfx_update();
}

void
MAP::describeSquare(int x, int y) const
{
    if (isFOV(x, y))
    {
	MOB		*mob;
	ITEM		*item;

	mob = getMob(x, y);
	msg_format("%S <see> %O.", MOB::getAvatar(), getTerrainName(x, y));
	if (mob)
	    msg_format("%S <see> %O.", MOB::getAvatar(), mob);
	item = getItem(x, y);
	if (item)
	    msg_format("%S <see> %O.", MOB::getAvatar(), item);
    }
    else if (isMapped(x, y))
    {
	msg_format("%S <recall> %O.", MOB::getAvatar(), getTerrainName(x, y));
    }
    else
    {
	msg_format("%S <know> nothing of that spot.", MOB::getAvatar());
    }
}

const char *
MAP::getTerrainName(int x, int y) const
{
    TILE_NAMES		tile;

    tile = getTile(x, y);

    return glb_tiledefs[tile].legend;
}

void
MAP::addItem(ITEM *item)
{
    assert(item->getNext() == 0);

    item->setNext(myItems);
    myItems = item;
}

void
MAP::removeItem(ITEM *item)
{
    ITEM	*c, *l;

    assert(item);
    if (!item)
	return;

    l = 0;
    for (c = myItems; c; c = c->getNext())
    {
	if (c == item)
	{
	    if (l)
		l->setNext(item->getNext());
	    else
		myItems = item->getNext();
	    item->setNext(0);
	    return;
	}

	l = c;
    }	    

    assert(0);
}

void
MAP::addMob(MOB *mob)
{
    assert(mob->getNext() == 0);

    mob->setNext(myMobs);
    myMobs = mob;

    mob->setMap(this);
}

void
MAP::removeMob(MOB *mob)
{
    MOB		*cur, *last;

    // Protect our safe next flag.
    if (mob && mob == myMobSafeNext)
    {
	myMobSafeNext = mob->getNext();
    }	    

    last = 0;
    for (cur = myMobs; cur; cur = cur->getNext())
    {
	if (cur == mob)
	{
	    if (last)
		last->setNext(cur->getNext());
	    else
		myMobs = cur->getNext();
	    cur->setNext(0);

	    cur->setMap(0);

	    return;
	}

	last = cur;
    }

    // Failure to remove a mob!
    assert(0);
}

bool
MAP::getFlag(int x, int y, int flag) const
{
    if (!isValidCoord(x, y))
	return false;

    if (myTileFlags[x + y * myWidth] & flag)
	return true;
    return false;
}

void
MAP::setFlag(int x, int y, int flag, bool val)
{
    assert(isValidCoord(x, y));
    if (!isValidCoord(x, y))
	return;

    if (val)
	myTileFlags[x + y * myWidth] |= flag;
    else
	myTileFlags[x + y * myWidth] &= ~flag;
}

void
MAP::redraw(int sx, int sy, int w, int h,
	    int cx, int cy) const
{
    // Adjust cx & cy so they refer to top left.
    cx = cx - w / 2;
    cy = cy - h / 2;

    myLastX = sx;
    myLastY = sy;
    myLastMX = cx;
    myLastMY = cy;
    myLastW = w;
    myLastH = h;

    re_redraw();
}

void
MAP::re_redraw() const
{
    int		sx, sy, w, h;
    int		x, y, cx, cy;
    TILE_NAMES	tile;
    u8		symbol;
    ATTR_NAMES	attr;

    sx = myLastX;
    sy = myLastY;
    w = myLastW;
    h = myLastH;
    cx = myLastMX;
    cy = myLastMY;

    // Plot all the points.
    for (y = sy; y < sy + h; y++)
    {
	for (x = sx; x < sx + w; x++)
	{
	    tile = getTile(x - sx + cx, y - sy + cy);

	    // Plot relevant character.
	    if (x >= 0 && x <= SCR_WIDTH && y >= 0 && y <= SCR_WIDTH)
	    {
		symbol = glb_tiledefs[tile].symbol;
		attr = (ATTR_NAMES) glb_tiledefs[tile].attr;

		// Override if out of FOV - shows up dim then.
		if (!isFOV(x - sx + cx, y - sy + cy))
		{
		    // We never draw the stairs dark as the user
		    // should be able to find them easily.
		    if (symbol != '>' && symbol != '<')
			attr = ATTR_OUTOFFOV;
		}

		// If it is not mapped, always ' '.
		if (!isMapped(x - sx + cx, y - sy + cy))
		    symbol = ' ';
		
		gfx_printchar(x, y, symbol, attr);
	    }
	}
    }

    // Print out all of our items.
    ITEM	*item;

    for (item = myItems; item; item = item->getNext())
    {
	x = item->getX() + sx - cx;
	y = item->getY() + sy - cy;

	// Verify it is in FOV.
	if (!isFOV(item->getX(), item->getY()))
	    continue;

	// Verify it is on screen.
	if (x >= sx && y >= sy && x < sx + w && y < sy + h)
	{
	    item->getLook(symbol, attr);
	    gfx_printchar(x, y, symbol, attr);
	}
    }

    // Print out all of our mobs.
    MOB		*mob;
    
    for (mob = myMobs; mob; mob = mob->getNext())
    {
	x = mob->getX() + sx - cx;
	y = mob->getY() + sy - cy;

	// Verify it is in FOV.
	if (!isFOV(mob->getX(), mob->getY()))
	    continue;

	// Verify it is on screen.
	if (x >= sx && y >= sy && x < sx + w && y < sy + h)
	{
	    mob->getLook(symbol, attr);
	    gfx_printchar(x, y, symbol, attr);

	    // If this is Timmy, we have found the poor wretch!
	    if (mob->getDefinition() == MOB_ZOMBIETIMMY && MOB::getAvatar())
	    {
		if (!quest_foundTimmy())
		{
		    quest_setFoundTimmy(true);
		    msg_report(text_lookup("found", MOB::getAvatar()->getRawName()));
		}
	    }

	    // If htis is Tlosh, he will brag
	    if (mob->getDefinition() == MOB_TLOSH && MOB::getAvatar())
	    {
		if (!quest_hasTloshSpoken())
		{
		    quest_setTloshSpoken(true);
		    msg_format("%S <yell>:\n", mob);
		    msg_report(text_lookup("tlosh", MOB::getAvatar()->getRawName()));
		}
	    }
	}
    }
}

void
MAP::fadeToBlack() const
{
    int		r, x, y, d;
    u8		sym;
    ATTR_NAMES	attr;

    d = MAX(myLastW, myLastH) / 2;
    for (r = d+10; r > 0;  r--)
    {
	for (y = 0; y < myLastH; y++)
	{
	    for (x = 0; x < myLastW; x++)
	    {
		d = MAX(ABS(x - myLastW/2), ABS(y - myLastH/2));

		if (d == 0)
		    continue;
		if (d >= r)
		{
		    // pure black.
		    gfx_printchar(x+myLastX, y+myLastY, ' ', ATTR_NORMAL);
		}
		else if (d >= r-10)
		{
		    // Out of FOV.
		    gfx_getchar(x+myLastX, y+myLastY, sym, attr);
		    gfx_printchar(x+myLastX, y+myLastY, sym, ATTR_OUTOFFOV);
		}
	    }
	}
	// Refresh the screen
	gfx_update();
	// And delay a bit
	gfx_delay(3);
    }
}

void
MAP::doMoveNPC()
{
    MOB		*cur;

    for (cur = myMobs; cur; cur = myMobSafeNext)
    {
	myMobSafeNext = cur->getNext();

	// I forgot this line the first time.  That was embarrasing :>
	if (cur->isAvatar())
	    continue;
	
	if (!cur->aiForcedAction())
	{
	    cur->aiDoAI();
	}
    }
}

void
MAP::save(ostream &os) const
{
    os.write((const char *) &myWidth, sizeof(int));
    os.write((const char *) &myHeight, sizeof(int));

    os.write((const char *) myTiles, myWidth * myHeight);
    os.write((const char *) myTileFlags, myWidth * myHeight);
    
    os.write((const char *) &myTotalRooms, sizeof(int));

    u8		c;
    MOB		*mob;
    ITEM	*item;

    for (mob = myMobs; mob; mob = mob->getNext())
    {
	c = 1;
	os.write((const char *) &c, 1);
	mob->save(os);
    }
    c = 0;
    os.write((const char *) &c, 1);
    
    for (item = myItems; item; item = item->getNext())
    {
	c = 1;
	os.write((const char *) &c, 1);
	item->save(os);
    }
    c = 0;
    os.write((const char *) &c, 1);
}

void
MAP::load(istream &is)
{
    u8			c;
    
    is.read((char *) &myWidth, sizeof(int));
    is.read((char *) &myHeight, sizeof(int));

    delete [] myTiles;
    delete [] myTileFlags;

    myTiles = new u8 [myWidth * myHeight];
    myTileFlags = new u8 [myWidth * myHeight];

    is.read((char *) myTiles, myWidth * myHeight);
    is.read((char *) myTileFlags, myWidth * myHeight);

    is.read((char *) &myTotalRooms, sizeof(int));

    // Load mobs...
    while (1)
    {
	is.read((char *) &c, 1);
	if (!c)
	    break;

	MOB		*mob;

	mob = MOB::load(is);

	addMob(mob);
    }

    // Load items
    while (1)
    {
	is.read((char *) &c, 1);
	if (!c)
	    break;

	ITEM		*item;

	item = ITEM::load(is);

	addItem(item);
    }
}
