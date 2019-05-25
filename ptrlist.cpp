/*
 * Licensed under BSD license.  See LICENCE.TXT  
 *
 * Produced by:	Jeff Lait
 *
 *      	7DRL Development
 *
 * NAME:        ptrlist.cpp ( Utility Library, C++ )
 *
 * COMMENTS:
 * 	Implementation of the ptrlist functions
 *	This is from before the 7DRL contest.
 */

#include <string.h>
#include "assert.h"
#include "ptrlist.h"

template <typename PTR>
PTRLIST<PTR>::PTRLIST()
{
    myEntries = 0;
    mySize = 0;
    myList = 0;
}

template <typename PTR>
PTRLIST<PTR>::~PTRLIST()
{
    delete [] myList;
}

template <typename PTR>
void
PTRLIST<PTR>::append(PTR item)
{
    if (myEntries >= mySize)
    {
	// Need to expand the extra stack.
	PTR		*newlist;

	mySize = mySize * 2 + 10;
	newlist = new PTR[mySize];
	if (myList)
	    memcpy(newlist, myList, 
		    sizeof(PTR) * myEntries);
	delete [] myList;
	myList = newlist;
    }
    
    myList[myEntries] = item;

    myEntries++;
}

template <typename PTR>
void
PTRLIST<PTR>::set(int idx, PTR item)
{
    assert(idx < entries());
    assert(idx >= 0);
    if (idx < 0)
	return;
    if (idx >= entries())
	return;

    myList[idx] = item;
}

template <typename PTR>
PTR
PTRLIST<PTR>::operator()(int idx) const
{
    assert(idx >= 0 && idx < myEntries);

    return myList[idx];
}


template <typename PTR>
int
PTRLIST<PTR>::entries() const
{
    return myEntries;
}

template <typename PTR>
void
PTRLIST<PTR>::clear()
{
    myEntries = 0;
}

template <typename PTR>
void
PTRLIST<PTR>::reverse()
{
    PTR		 tmp;
    int		 i, n, r;

    n = entries();
    for (i = 0; i < n / 2; i++)
    {
	// Reversed entry.
	r = n - 1 - i;
	tmp = (*this)(i);
	set(i, (*this)(r));
	set(r, tmp);
    }
}

template <typename PTR>
void
PTRLIST<PTR>::removePtr(PTR item)
{
    int		s, d, n;

    n = entries();
    s = d = 0;
    while (s < n)
    {
	if ((*this)(s) == item)
	{
	    // Don't copy this value.
	}
	else
	{
	    // Copy this value.
	    if (s != d)
		set(d, (*this)(s));
	    d++;
	}
	// Go to next value.
	s++;
    }
    // Final n is d.
    myEntries = d;
}

template <typename PTR>
int
PTRLIST<PTR>::find(PTR item) const
{
    int		i, n;

    n = entries();
    for (i = 0; i < n; i++)
    {
	if ((*this)(i) == item)
	    return i;
    }
    return -1;
}
