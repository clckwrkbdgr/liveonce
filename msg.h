/*
 * Licensed under BSD license.  See LICENCE.TXT  
 *
 * Produced by:	Jeff Lait
 *
 *      	7DRL Development
 *
 * NAME:        msg.h ( Live Once Library, C++ )
 *
 * COMMENTS:
 */

#ifndef __msg__
#define __msg__

class PANEL;
class MOB;
class ITEM;

// Sets where messages go.
void msg_registerpanel(PANEL *panel);

// Prints the given message.  Capitalizes.
void msg_report(const char *msg);

// Quotes the message by proper indentation.
void msg_quote(const char *msg);

void msg_newturn();

// Triggers redraw of panel.  You still need to do gfx_udate though.
void msg_update();

// These are different public versions of the format code.
// They all dump the result to the registered panel.
void msg_format(const char *msg, MOB *subject);
void msg_format(const char *msg, ITEM *subject);
void msg_format(const char *msg, MOB *subject, MOB *object);
void msg_format(const char *msg, MOB *subject, ITEM *object);
void msg_format(const char *msg, MOB *subject, const char *verb, MOB *object);
void msg_format(const char *msg, MOB *subject, const char *object);

#endif
