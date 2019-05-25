/*
 * Licensed under BSD license.  See LICENCE.TXT  
 *
 * Produced by:	Jeff Lait
 *
 *      	POWDER Development
 *
 * NAME:        grammar.h ( POWDER Library, C++ )
 *
 * COMMENTS:
 *	These handle all the bizarre exceptions which English can
 *	throw at us.  Well, theoritically they are all handled, but
 *	as exceptions are found, this is where to add them.
 *
 *	This predates the 7DRL challenge.
 */

#ifndef __grammar_h__
#define __grammar_h__

enum VERB_PERSON
{
    VERB_I,
    VERB_YOU,
    VERB_HE,
    VERB_SHE,
    VERB_IT,
    VERB_WE,
    VERB_YALL,
    VERB_HES,
    VERB_SHES,
    VERB_THEY,
    NUM_VERBS
};

// Utility functions to expand ctype.
bool
gram_ispronoun(const char *word);

// Utility function to capitalize a sentence...
// This will also capitalize any sub sentences.
const char *
gram_capitalize(const char *str);

// This will convert the given name into the possessive tense.
// Ie, you -> your, orc -> orc's, moss -> moss'
const char *
gram_makepossessive(const char *str);

// This will convert the given name into a plural
const char *
gram_makeplural(const char *phrase);

// XXX hits the Foo.
// he/she/it/you/I
const char *
gram_getpronoun(VERB_PERSON person);

// XXX rock dissolved in acid.
// his/her/its/your/my
const char *
gram_getpossessive(VERB_PERSON person);

// That rock is XXX.
// his/hers/its/yours/mine
const char *
gram_getownership(VERB_PERSON person);

// Suicidially, Foo hits XXX.
// himself/herself/itself/yourself/myself
const char *
gram_getreflexive(VERB_PERSON person);

// Foo hits XXX.
// him/her/it/you/me
const char *
gram_getaccusative(VERB_PERSON person);

bool
gram_isvowel(char c);

// True if c is the end of a sentence.
bool
gram_isendsentence(char c);

bool
gram_isplural(const char *noun);

// This takes a complicated name, like:
// holy +3 wand of fireballs (5)
// and determines if "wand" is plural.
bool
gram_isnameplural(const char *name);

// This fetches a temporary buffer.  There is a finite number of them,
// of maximum size 50, so use wisely.
char *
gram_gettmpbuf();

// This will fetch the appropriate article, ie: a, an, the.
// As the article may be empty (for proper nouns or plural nouns)
// the trailing space is included, so it would be "a ".
const char *
gram_getarticle(const char *noun);

// This builds the appropriate phrase, such as "5 arrows of dragon slaying"
// or "no tea" according to the singular basename and the count variable.
// If article is false, it will not use "a" or "the" in the singular
// cases.
const char *
gram_createcount(const char *basename, int count, bool article);

// This builds the appropriate place number.  Ie, 1st, 2nd, 23rd.
const char *
gram_createplace(int place);

// Conjugates the given infinitive verb according the given person.
const char *
gram_conjugate(const char *verb, VERB_PERSON person, bool past = false);

#endif
