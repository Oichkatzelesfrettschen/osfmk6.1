/*
 * Copyright (c) 1995, 1994, 1993, 1992, 1991, 1990  
 * Open Software Foundation, Inc. 
 *  
 * Permission to use, copy, modify, and distribute this software and 
 * its documentation for any purpose and without fee is hereby granted, 
 * provided that the above copyright notice appears in all copies and 
 * that both the copyright notice and this permission notice appear in 
 * supporting documentation, and that the name of ("OSF") or Open Software 
 * Foundation not be used in advertising or publicity pertaining to 
 * distribution of the software without specific, written prior permission. 
 *  
 * OSF DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE 
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS 
 * FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL OSF BE LIABLE FOR ANY 
 * SPECIAL, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES 
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN 
 * ACTION OF CONTRACT, NEGLIGENCE, OR OTHER TORTIOUS ACTION, ARISING 
 * OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE 
 */
/*
 * OSF Research Institute MK6.1 (unencumbered) 1/31/1995
 */

extern int getchar(void);

void
safe_gets(char *str, int maxlen)
{
	register char *lp;
	register int c;
	char *strmax = str + maxlen - 1; /* allow space for trailing 0 */

	lp = str;
	for (;;) {
	    c = getchar();
	    switch (c) {
		case '\n':
		case '\r':
		    printf("\n");
		    *lp++ = 0;
		    return;

		case '\b':
		case '#':
		case '\177':
		    if (lp > str) {
			printf("\b \b");
			lp--;
		    }
		    continue;
		case '@':
		case 'u'&037:
		    lp = str;
		    printf("\n\r");
		    continue;
		default:
		    if (c >= ' ' && c < '\177') {
			if (lp < strmax) {
			    *lp++ = c;
			    printf("%c", c);
			}
			else {
			    printf("%c", '\007'); /* beep */
			}
		    }
	    }
	}
}
