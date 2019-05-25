/*
 * Licensed under BSD license.  See LICENCE.TXT  
 *
 * Produced by:	Jeff Lait
 *
 *      	7DRL Development
 *
 * NAME:        quest.h ( Live Once Library, C++ )
 *
 * COMMENTS:
 */

#ifndef __quest__
#define __quest__

#include <iostream>
using namespace std;

void quest_init();

void quest_save(ostream &os);
void quest_load(istream &is);

// Whether current avatar has enough shrooms.
bool quest_hasShrooms();

// Has tlosh spoken to us?
bool quest_hasTloshSpoken();
void quest_setTloshSpoken(bool val);

// if someone has got shrooms to surface.
bool quest_doneShrooms();
void quest_setDoneShrooms(bool val);

// Has someone found Timmy?
bool quest_foundTimmy();
void quest_setFoundTimmy(bool val);

// Is the kid dead yet?
bool quest_TimmyDead();
void quest_setTimmyDead(bool val);

#endif

