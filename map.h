/*
 * Licensed under BSD license.  See LICENCE.TXT  
 *
 * Produced by:	Jeff Lait
 *
 *      	7DRL Development
 *
 * NAME:        map.h ( Live Once Library, C++ )
 *
 * COMMENTS:
 */

#ifndef __map__
#define __map__

#include "glbdef.h"

#include "ptrlist.h"

#include <iostream>
using namespace std;

class MOB;
class ITEM;
class LEGEND;

class MAP
{
public:
		 MAP();
		~MAP();

    void	doMoveNPC();		
    
    int		getWidth() const { return myWidth; }
    int		getHeight() const { return myHeight; }
    bool	isValidCoord(int x, int y) const;

    TILE_NAMES	getTile(int x, int y) const;
    void	setTile(int x, int y, TILE_NAMES tile);

    bool	findTile(TILE_NAMES tile, int &x, int &y, bool avoidmob = false) const;

    bool	isPassable(int x, int y) const;
    bool	isTransparent(int x, int y) const;

    bool	isMapped(int x, int y) const;
    bool	isFOV(int x, int y) const;

    ITEM	*getItem(int x, int y) const;
    MOB		*getMob(int x, int y) const;
    MOB		*findMob(MOB_NAMES def) const;

    MOB		*traceBullet(int x, int y, int r, int dx, int dy) const;
    void	 displayBullet(int x, int y, int r, int dx, int dy, u8 sym, ATTR_NAMES attr) const;

    static MAP	*getMainMap() { return theMainMap; }
    static void	 setMainMap(MAP *map) { theMainMap = map; }

    bool	 isMainMap() const { return this == theMainMap; }

    void	 describeSquare(int x, int y) const;

    const char	*getTerrainName(int x, int y) const;

    void	 addItem(ITEM *item);
    void	 removeItem(ITEM *item);

    void	 addMob(MOB *mob);
    void	 removeMob(MOB *mob);

    int	 	 getDepth()  const { return (myTotalRooms / 30) + 1; }

    // Redraws to the given window.
    // cx & cy are the center of the map, x-w and y-h are the
    // window to draw into in screen coords.
    void	redraw(int x, int y, int w, int h,
			int cx, int cy) const;

    // Redraws with the last known window.
    void	re_redraw() const;

    // Fades the map to black.
    void	fadeToBlack() const;

    // Constructs a proper map for the given depth.
    void	build(int roomsuptonow);

    MAP		*getDownMap() const;
    MAP		*getUpMap() const;

    // Used for loading...
    void	 setDownMap(MAP *down);
    MAP		*getRawDownMap() const { return myDownMap; }
    MAP		*getRawUpMap() const { return myDownMap; }

    MOB		*getMobHead() const { return myMobs; }

    // Applies given symbol to given x,y coord according to given legend.
    void	parseLegend(int x, int y, const PTRLIST<LEGEND *> &list,
			    u8 symbol);

    // Loads a map from the given .map file
    void	loadMap(const char *fname);

    void	clear(TILE_NAMES tile);
    void	clearFlag(int flag, bool val);

    // Sets the FOV flag for everything in sight of cx & cy.
    void	buildFOV(int cx, int cy);

    // Checks all 9 squares around sx-sy for LOS.  This lets you
    // look around corners.
    bool	hasLOSLoose(int sx, int sy, int ex, int ey) const;

    // Return strue if (sx, sy) has line of sight to (ex, ey)
    bool	hasLOS(int sx, int sy, int ex, int ey) const;

    void	load(istream &is);
    void	save(ostream &os) const;
    
protected:
    bool	 getFlag(int x, int y, int flag) const;
    void	 setFlag(int x, int y, int flag, bool val);
    
    int		 myWidth, myHeight;

    // These cache the last displayed values of the map so we can
    // redraw it at our leisure.
    mutable int	 myLastX, myLastY, myLastW, myLastH, myLastMX, myLastMY;

    // Raw array of tiles.
    u8		*myTiles;
    // Raw array of tile flags.
    u8		*myTileFlags;

    // List of all of our mobs.
    MOB		*myMobs;
    MOB		*myMobSafeNext;

    // List of all our items.
    ITEM	*myItems;

    static MAP	*theMainMap;

    int		 myTotalRooms;

    mutable MAP	*myDownMap, *myUpMap;
};

#endif

