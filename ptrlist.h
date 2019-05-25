/*
 * Licensed under BSD license.  See LICENCE.TXT  
 *
 * Produced by:	Jeff Lait
 *
 *      	7DRL Development
 *
 * NAME:        ptrlist.h ( Live Once Library, C++ )
 *
 * COMMENTS:
 *	Implements a simple list of pointers.
 *	This predates the 7DRL contest.
 */

#ifndef __ptrlist_h__
#define __ptrlist_h__

template <typename PTR>
class PTRLIST
{
public:
    PTRLIST();
    ~PTRLIST();

    void		 append(PTR item);

    // Reverses the order of the stack.
    void		 reverse();

    // Empties the stack
    void		 clear();

    PTR			 operator()(int idx) const;
    int			 entries() const;

    void		 set(int idx, PTR item);

    // Removes all instances of "item" from
    // this list.
    void		 removePtr(PTR item);

    // Finds the given item, returns -1 if fails.
    int			 find(PTR item) const;

private:
    PTR			 *myList;
    int			  myEntries;
    int			  mySize;
};

// For crappy platforms:
#include "ptrlist.cpp"

#endif

