/*
 * Licensed under BSD license.  See LICENCE.TXT  
 *
 * Produced by:	Jeff Lait
 *
 *      	7DRL Development
 *
 * NAME:        text.h ( Live Once Library, C++ )
 *
 * COMMENTS:
 */

#ifndef __text__
#define __text__

void text_init();
void text_shutdown();

const char *text_lookup(const char *dict, const char *word);
const char *text_lookup(const char *dict, const char *word, const char *subword);

#endif

