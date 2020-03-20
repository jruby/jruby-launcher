/*
 *  Copyright (c) 1993, Intergraph Corporation
 *
 *  You may distribute under the terms of either the GNU General Public
 *  License or the Artistic License, as specified in the perl README file.
 *
 *  Various Unix compatibility functions and NT specific functions.
 *
 *  Some of this code was derived from the MSDOS port(s) and the OS/2 port.
 *
 */

#include <windows.h>

#ifndef _RB_W32_CMDVECTOR_H
#define	_RB_W32_CMDVECTOR_H

#ifdef	__cplusplus
extern "C" {
#endif

#define MAXPATHLEN 512
#define NTMALLOC 0x2	// string in element was malloc'ed

#define isascii(c) ( (c>=0x00&&c<=0x7f)?1:0 )
#define isspace(c) ( ((c>=0x09&&c<=0x0d)||c==0x20)?1:0 )
#define ISASCII(c) isascii((int)(unsigned char)(c))
#define ISSPACE(c) (ISASCII(c) && isspace((int)(unsigned char)(c)))

static inline char * skipspace(char *ptr) {
    while (ISSPACE(*ptr))
	ptr++;
    return ptr;
}

typedef struct _NtCmdLineElement {
    struct _NtCmdLineElement *next;
    char *str;
    int len;
    int flags;
} NtCmdLineElement;


int strlcpy(char *dst, const char *src, size_t siz);

// NOTE: this method is taken from MRI source basically verbatim,
// to preserve compatibility
int rb_w32_cmdvector(const char *cmd, char ***vec) {
    int globbing, len;
    int elements, strsz, done;
    int slashes, escape;
    char *ptr, *base, *buffer, *cmdline;
    char **vptr;
    char quote;
    NtCmdLineElement *curr;
    NtCmdLineElement *cmdhead = NULL, **cmdtail = &cmdhead;

    //
    // just return if we don't have a command line
    //

    while (ISSPACE(*cmd))
	cmd++;
    if (!*cmd) {
	*vec = NULL;
	return 0;
    }

    ptr = cmdline = strdup(cmd);

    //
    // Ok, parse the command line, building a list of CmdLineElements.
    // When we've finished, and it's an input command (meaning that it's
    // the processes argv), we'll do globing and then build the argument
    // vector.
    // The outer loop does one interation for each element seen.
    // The inner loop does one interation for each character in the element.
    //

    while (*(ptr = skipspace(ptr))) {
	base = ptr;
	quote = slashes = globbing = escape = 0;
	for (done = 0; !done && *ptr; ) {
	    //
	    // Switch on the current character. We only care about the
	    // white-space characters, the  wild-card characters, and the
	    // quote characters.
	    //

	    switch (*ptr) {
	      case '\\':
		if (quote != '\'') slashes++;
	        break;

	      case ' ':
	      case '\t':
	      case '\n':
		//
		// if we're not in a string, then we're finished with this
		// element
		//

		if (!quote) {
		    *ptr = 0;
		    done = 1;
		}
		break;

	      case '*':
	      case '?':
	      case '[':
	      case '{':
		//
		// record the fact that this element has a wildcard character
		// N.B. Don't glob if inside a single quoted string
		//

		if (quote != '\'')
		    globbing++;
		slashes = 0;
		break;

	      case '\'':
	      case '\"':
		//
		// if we're already in a string, see if this is the
		// terminating close-quote. If it is, we're finished with
		// the string, but not neccessarily with the element.
		// If we're not already in a string, start one.
		//

		if (!(slashes & 1)) {
		    if (!quote)
			quote = *ptr;
		    else if (quote == *ptr) {
			if (quote == '"' && quote == ptr[1])
			    ptr++;
			quote = '\0';
		    }
		}
		escape++;
		slashes = 0;
		break;

	      default:
		ptr = CharNext(ptr);
		slashes = 0;
		continue;
	    }
	    ptr++;
	}

	//
	// when we get here, we've got a pair of pointers to the element,
	// base and ptr. Base points to the start of the element while ptr
	// points to the character following the element.
	//

	len = ptr - base;
	if (done) --len;

	//
	// if it's an input vector element and it's enclosed by quotes,
	// we can remove them.
	//

	if (escape) {
	    char *p = base, c;
	    slashes = quote = 0;
	    while (p < base + len) {
		switch (c = *p) {
		  case '\\':
		    p++;
		    if (quote != '\'') slashes++;
		    break;

		  case '\'':
		  case '"':
		    if (!(slashes & 1) && quote && quote != c) {
			p++;
			slashes = 0;
			break;
		    }
		    memcpy(p - ((slashes + 1) >> 1), p + (~slashes & 1),
			   base + len - p);
		    len -= ((slashes + 1) >> 1) + (~slashes & 1);
		    p -= (slashes + 1) >> 1;
		    if (!(slashes & 1)) {
			if (quote) {
			    if (quote == '"' && quote == *p)
				p++;
			    quote = '\0';
			}
			else
			    quote = c;
		    }
		    else
			p++;
		    slashes = 0;
		    break;

		  default:
		    p = CharNext(p);
		    slashes = 0;
		    break;
		}
	    }
	}

	curr = (NtCmdLineElement *)calloc(sizeof(NtCmdLineElement), 1);
	if (!curr) goto do_nothing;
	curr->str = base;
	curr->len = len;

        // TODO: globbing!
//	if (globbing && (tail = cmdglob(curr, cmdtail))) {
//	    cmdtail = tail;
//	}
//	else {
	    *cmdtail = curr;
	    cmdtail = &curr->next;
//	}
    }

    //
    // Almost done!
    // Count up the elements, then allocate space for a vector of pointers
    // (argv) and a string table for the elements.
    //

    for (elements = 0, strsz = 0, curr = cmdhead; curr; curr = curr->next) {
	elements++;
	strsz += (curr->len + 1);
    }

    len = (elements+1)*sizeof(char *) + strsz;
    buffer = (char *)malloc(len);
    if (!buffer) {
      do_nothing:
	while ((curr = cmdhead)) {
	    cmdhead = curr->next;
	    if (curr->flags & NTMALLOC) free(curr->str);
	    free(curr);
	}
	free(cmdline);
	for (vptr = *vec; *vptr; ++vptr);
	return vptr - *vec;
    }

    //
    // make vptr point to the start of the buffer
    // and ptr point to the area we'll consider the string table.
    //
    //   buffer (*vec)
    //   |
    //   V       ^---------------------V
    //   +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
    //   |   |       | ....  | NULL  |   | ..... |\0 |   | ..... |\0 |...
    //   +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
    //   |-  elements+1             -| ^ 1st element   ^ 2nd element

    vptr = (char **) buffer;

    ptr = buffer + (elements+1) * sizeof(char *);

    while ((curr = cmdhead)) {
	strlcpy(ptr, curr->str, curr->len + 1);
	*vptr++ = ptr;
	ptr += curr->len + 1;
	cmdhead = curr->next;
	if (curr->flags & NTMALLOC) free(curr->str);
	free(curr);
    }
    *vptr = 0;

    *vec = (char **) buffer;
    free(cmdline);
    return elements;
}


#ifdef	__cplusplus
}
#endif

#endif	/* _RB_W32_CMDVECTOR_H */

