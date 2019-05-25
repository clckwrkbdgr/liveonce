/*
 * Licensed under BSD license.  See LICENCE.TXT  
 *
 * Produced by:	Jeff Lait
 *
 *      	7DRL Development
 *
 * NAME:        msg.cpp ( Live once Library, C++ )
 *
 * COMMENTS:
 */

#include "msg.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "panel.h"
#include "grammar.h"
#include "mob.h"
#include "item.h"

PANEL *glbPanel = 0;

bool glbBlankNewTurn = false;

void
msg_update()
{
    glbPanel->redraw();
}

void 
msg_registerpanel(PANEL *panel)
{
    glbPanel = panel;
}

void
msg_report(const char *msg)
{
    glbPanel->appendText(msg);
    glbBlankNewTurn = false;
}

void
msg_quote(const char *msg)
{
    glbPanel->setIndent(1);
    glbPanel->newLine();
    msg_report(msg);
    glbPanel->setIndent(0);
}

void
msg_newturn()
{
    if (!glbBlankNewTurn)
    {
	glbBlankNewTurn = true;
	glbPanel->newLine();
	glbPanel->appendText("> ");
    }
}

//
// Builder functions that respect the triple possibilities.
//
VERB_PERSON
msg_getPerson(MOB *m, ITEM *i, const char *s)
{
    VERB_PERSON		person = VERB_IT;

    if (m)
    {
	person = m->getPerson();
    }
    else if (i)
    {
	person = i->getPerson();
    }
    else if (s)
    {
	person = VERB_IT;
	if (gram_isnameplural(s))
	    person = VERB_THEY;
    }

    return person;
}


const char *
msg_buildVerb(const char *verb, MOB *m_subject, ITEM *i_subject, const char *s_subject)
{
    VERB_PERSON		person;

    person = msg_getPerson(m_subject, i_subject, s_subject);

    return gram_conjugate(verb, person);
}

const char *
msg_buildReflexive(MOB *m, ITEM *i, const char *s)
{
    VERB_PERSON		p;

    p = msg_getPerson(m, i, s);

    return gram_getreflexive(p);
}

const char *
msg_buildFullName(MOB *m, ITEM *i, const char *s)
{
    const char		*rawname = "tea"; 

    if (m)
    {
	rawname = m->getName();
    }
    else if (i)
    {
	rawname = i->getName();
    }
    else if (s)
	rawname = s;
    
    char 		*buf;
    const char		*art;

    buf = gram_gettmpbuf();
    art = gram_getarticle(rawname);

    sprintf(buf, "%s%s", art, rawname);

    return buf;
}

//
// This is the universal formatter
//
void
msg_format(const char *msg, MOB *m_subject, ITEM *i_subject, const char *s_subject, const char *verb, MOB *m_object, ITEM *i_object, const char *s_object)
{
    char		 buf[500];
    int			 bufpos = 0;
    const char		*newtext;

    while (*msg)
    {
	if (*msg == '%')
	{
	    newtext = "";
	    switch (msg[1])
	    {
		case '%':
		    // Pure %.
		    newtext = "%";
		    break;

		case '<':
		    // Escapped <
		    newtext = "<";
		    break;
		    
		case 'v':
		    // Conjugate the given verb & append.
		    assert(verb);
		    newtext = msg_buildVerb(verb, m_subject, i_subject, s_subject);
		    break;

		case 'S':
		    newtext = msg_buildFullName(m_subject, i_subject, s_subject);
		    break;

		case 'O':
		    if (m_subject && (m_subject == m_object) ||
			i_subject && (i_subject == i_object))
		    {
			// Reflexive case!
			newtext = msg_buildReflexive(m_object, i_object, s_object);
		    }
		    else
			newtext = msg_buildFullName(m_object, i_object, s_object);
		    break;
	    }

	    msg += 2;
	    // Append the new text
	    while (*newtext)
	    {
		buf[bufpos++] = *newtext++;
	    }
	}
	else if (*msg == '<')
	{
	    char *v = strdup(&msg[1]);
	    char *startv = v;
	    
	    msg++;
	    while (*v && *v != '>')
	    {
		msg++;
		v++;
	    }
	    *v = 0;
	    // Must be closed!
	    assert(*msg == '>');
	    if (*msg == '>')
		msg++;

	    newtext = msg_buildVerb(startv, m_subject, i_subject, s_subject);

	    while (*newtext)
	    {
		buf[bufpos++] = *newtext++;
	    }

	    free(startv);
	}
	else
	{
	    // Normal character!
	    buf[bufpos++] = *msg++;
	}
    }

    // If it ends with puntuation, add spaces.
    if (bufpos && gram_isendsentence(buf[bufpos-1]))
    {
	buf[bufpos++] = ' ';
	buf[bufpos++] = ' ';
    }

    // Null terminate.
    buf[bufpos] = '\0';

    // Formatted into buf.  Capitalize & print.
    newtext = gram_capitalize(buf);

    msg_report(newtext);
}

//
// These are the specific instantiations.
//
void
msg_format(const char *msg, MOB *subject)
{
    msg_format(msg, subject, 0, 0, 0, 0, 0, 0);
}

void
msg_format(const char *msg, ITEM *subject)
{
    msg_format(msg, 0, subject, 0, 0, 0, 0, 0);
}

void
msg_format(const char *msg, MOB *subject, MOB *object)
{
    msg_format(msg, subject, 0, 0, 0, object, 0, 0);
}

void
msg_format(const char *msg, MOB *subject, ITEM *object)
{
    msg_format(msg, subject, 0, 0, 0, 0, object, 0);
}

void
msg_format(const char *msg, MOB *subject, const char *verb, MOB *object)
{
    msg_format(msg, subject, 0, 0, verb, object, 0, 0);
}

void
msg_format(const char *msg, MOB *subject, const char *object)
{
    msg_format(msg, subject, 0, 0, 0, 0, 0, object);
}
