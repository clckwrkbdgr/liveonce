/*
 * Licensed under BSD license.  See LICENCE.TXT  
 *
 * Produced by:	Jeff Lait
 *
 *      	7DRL Development
 *
 * NAME:        text.cpp ( Live Once Library, C++ )
 *
 * COMMENTS:
 */

#include "text.h"

#include <fstream>
using namespace std;

#include "ptrlist.h"
#include "grammar.h"
#include "rand.h"

PTRLIST<char *> glbTextEntry;
PTRLIST<char *> glbTextKey;

char *
text_append(char *oldtxt, const char *append)
{
    int		len;
    char	*txt;
    
    len = (int)strlen(oldtxt) + (int)strlen(append) + 5;
    txt = (char *) malloc(len);
    strcpy(txt, oldtxt);
    strcat(txt, append);
    free(oldtxt);
    return txt;
}

void
text_init()
{
    ifstream	is("text.txt");
    char	line[500];
    bool	hasline = false;
    char	*text;

    while (hasline || is.getline(line, 500))
    {
	hasline = false;

	// Ignore comments.
	if (line[0] == '#')
	    continue;

	// See if an entry...
	if (!ISSPACE(line[0]))
	{
	    // This line is a key.
	    glbTextKey.append(strdup(line));
	    // Rest is the message...
	    text = strdup("");
	    while (is.getline(line, 500))
	    {
		int		firstnonws;

		for (firstnonws = 0; line[firstnonws] && ISSPACE(line[firstnonws]); firstnonws++);

		if (!line[firstnonws])
		{
		    // Completely blank line - insert a hard return!
		    text = text_append(text, "\n\n");
		}
		else if (!firstnonws)
		{
		    // New dictionary entry, break out!
		    hasline = true;
		    break;
		}
		else
		{
		    // Allow some ascii art...
		    if (firstnonws > 4)
		    {
			text = text_append(text, "\n");
			firstnonws = 4;
		    }

		    // Append remainder.
		    // Determine if last char was end of sentence.
		    if (*text && text[strlen(text)-1] != '\n')
		    {
			if (gram_isendsentence(text[strlen(text)-1]))
			    text = text_append(text, " ");
			text = text_append(text, " ");
		    }

		    text = text_append(text, &line[firstnonws]);
		}
	    }

	    // Append the resulting text.
	    glbTextEntry.append(text);
	}
    }
}

void
text_shutdown()
{
    int		i;

    for (i = 0; i < glbTextKey.entries(); i++)
    {
	free(glbTextKey(i));
	free(glbTextEntry(i));
    }
}

const char *
text_lookup(const char *key)
{
    int		i;
    for (i = 0; i < glbTextKey.entries(); i++)
    {
	if (!strcmp(key, glbTextKey(i)))
	    return glbTextEntry(i);
    }

    char	*buf;

    buf = gram_gettmpbuf();

    sprintf(buf, "Missing text entry: \"%s\".", key);

    return buf;
}

const char *
text_lookup(const char *dict, const char *word)
{
    char		buf[100];

    sprintf(buf, "%s::%s", dict, word);
    return text_lookup(buf);
}

const char *
text_lookup(const char *dict, const char *word, const char *subword)
{
    char		buf[100];

    sprintf(buf, "%s::%s::%s", dict, word, subword);
    return text_lookup(buf);
}
