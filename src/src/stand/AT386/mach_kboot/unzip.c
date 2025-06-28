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

/*
 * File : unzip.c
 *
 * Author : Eric PAIRE (O.S.F. Research Institute)
 *
 * This file contains the code for uncompressing [g]zip files.
 */

#include "boot.h"
#include "unzip.h"

/*
 * Miscellaneous needed by inflate.c
 */
unsigned mask_bits[] = { 0x00000000,
	0x00000001, 0x00000003, 0x00000007, 0x0000000F,
	0x0000001F, 0x0000003F, 0x0000007F, 0x000000FF,
	0x000001FF, 0x000003FF, 0x000007FF, 0x00000FFF,
	0x00001FFF, 0x00003FFF, 0x00007FFF, 0x0000FFFF,
	0x0001FFFF, 0x0003FFFF, 0x0007FFFF, 0x000FFFFF,
	0x001FFFFF, 0x003FFFFF, 0x007FFFFF, 0x00FFFFFF,
	0x01FFFFFF, 0x03FFFFFF, 0x07FFFFFF, 0x0FFFFFFF,
	0x1FFFFFFF, 0x3FFFFFFF, 0x7FFFFFFF, 0xFFFFFFFF };
uch *slide;

struct unzip_alloc {
	unsigned		length;		/* must be first field */
	struct unzip_alloc	*next;
	struct unzip_alloc	*prev;
};

static struct unzip_alloc unzip_root;	/* free list root pointer */
static void *unzip_brk;			/* start of unzip heap */
static void *unzip_last;		/* last addr of uncompressed data */
static void *unzip_addr;		/* last addr of compressed data */
static unsigned unzip_length;		/* remaining size of compressed data */
static unsigned unzip_block;		/* current block number */
static int (*unzip_copy)(unsigned, void *, void **, unsigned);
					/* current copying procedure */

int
unzip(void **load_addr,
      void *addr,
      unsigned len,
      int (*boot_copy)(unsigned, void *, void **, unsigned))
{
	unsigned char *ptr;
	unsigned char method;
	unsigned char flags;
	unsigned long mtime;
	unsigned i;
	unsigned too_short;
	int ret;

	if (debug)
		printf("Start unzip(0x%x, 0x%x, 0x%x, 0x%x)\n",
		       *load_addr, addr, len, boot_copy);

	ptr = addr;
	too_short = (len < 10);
	if (!too_short) {
		ptr += 2;	/* Magic header */
		if ((method = *ptr++) != GZIP_DEFLATED) {
			printf("unzip: Unknown compression method (%d)\n",
			       method);
			return (1);
		}

		if ((flags = *ptr++) & GZIP_ENCRYPTED) {
			printf("unzip: Can't handle encrypted image\n");
			return (1);
		}
		if (flags & GZIP_CONTINUATION) {
			printf("unzip: Can't handle continuation image\n");
			return (1);
		}
		if (flags & GZIP_RESERVED) {
			printf("unzip: Can't handle reserved bits\n");
			return (1);
		}

		mtime = ((ptr[0] << 24) |
			 (ptr[1] << 16) |
			 (ptr[2] << 8) | ptr[3]);
		ptr += 4;
		/* XXX Should handle mtime */

		ptr++;	/* Extra flags */
		ptr++;	/* Operating System */

		if (flags & GZIP_EXTRA) {
			too_short = ((void *)ptr - addr > len + 2);
			if (!too_short) {
				i = (ptr[0] << 8) | ptr[1];
				ptr += 2 + i;
				too_short = ((void *)ptr - addr > len);
			}
		}

		if (!too_short && (flags & GZIP_FILENAME))
			do {
				too_short = ((void *)ptr - addr> len + 1);
			} while (!too_short && *ptr++ != '\0');


		if (!too_short && (flags & GZIP_COMMENT))
			do {
				too_short = ((void *)ptr - addr> len + 1);
			} while (!too_short && *ptr++ != '\0');
	}

	if (too_short) {
		printf("unzip: Too short compressed file\n");
		return (1);
	}

	/*
	 * allocate slide buffer and unzip_malloc/unzip_free area.
	 */
	unzip_brk = slide = (uch *)(((unsigned long)addr - WSIZE) &
				    ~(sizeof (unzip_root.length) - 1));
	unzip_last = *load_addr;
	unzip_root.length = 0;
	unzip_root.next = unzip_root.prev = &unzip_root;
	unzip_copy = boot_copy;
	unzip_block = 1;
	unzip_addr = ptr;
	unzip_length = len - ((void *)ptr - addr);

	switch (ret = inflate()) {
	case 0: /* Success */
		*load_addr = unzip_last;
		return (0);
	case 4: /* Interrupt */
		return (2);
	}
	if (debug)
		printf("unzip: inflate returned %d\n", ret);
	return (1);
}

int
unzip_getc(unsigned short *p)
{
	if (unzip_length == 0) {
		*p = 0;
		return (0);
	}
	*p = *((unsigned char *)unzip_addr);
	unzip_addr++;
	unzip_length--;
	return (8);
}

int
unzip_flush(unsigned len)
{
	int ret;
	unsigned rlen;
	unsigned block;

	if (len == 0)
		return (0);

	block = 0;
	do {
		if (is_intr_char()) {
			printf(" \n");
			return (3);
		}

		if ((len - block) < 512)
			rlen = (len - block);
		else
			rlen = 512;
		ret = (*unzip_copy)(unzip_block++,
				    &slide[block], &unzip_last, rlen);
		block += rlen;
	} while (ret == 0 && block < len);

	if (ret == 0 && len < WSIZE) {
		printf("Error: Kernel Image is too short\n");
		ret = 1;
	}
	return (ret);
}

void *
unzip_malloc(unsigned len)
{
	struct unzip_alloc *addr;
	struct unzip_alloc *best;

	if (len < sizeof (struct unzip_alloc) - sizeof (addr->length))
		len = sizeof (struct unzip_alloc) - sizeof (addr->length);
	len = (len + 2 * sizeof (addr->length)) & ~(sizeof (addr->length) - 1);

	/*
	 * Look for the best matching free element.
	 */
	best = (struct unzip_alloc *)0;
	for (addr = unzip_root.next; addr != &unzip_root; addr = addr->next) {
		if (addr->length < len)
			continue;
		if (addr->length == len)
			break;
		if (best == (struct unzip_alloc *)0 ||
		    addr->length < best->length)
			best = addr;
	}

	/*
	 * Don't split best element if there is not enough place to use
	 *	its other part.
	 */
	if (addr == &unzip_root &&
	    best != (struct unzip_alloc *)0 &&
	    best->length < (len +
			    sizeof (struct unzip_alloc) +
			    sizeof (best->length)))
		addr = best;

	/*
	 * Extract matching element.
	 */
	if (addr != &unzip_root) {
		addr->prev->next = addr->next;
		addr->next->prev = addr->prev;
		return ((void *)addr + sizeof(addr->length));
	}

	/*
	 * Split best element or allocate new memory.
	 */
	if (best != (struct unzip_alloc *)0) {
		best->length -= len + sizeof (best->length);
		addr = (struct unzip_alloc *)((void *)best + best->length);
	} else {
		if (unzip_brk < unzip_last + len)
			return ((void *)0);
		addr = (struct unzip_alloc *)(unzip_brk -= len);
	}
	addr->length = len;
	return ((void *)addr + sizeof(addr->length));
}

void
unzip_free(void *p)
{
	struct unzip_alloc *curr;
	struct unzip_alloc *addr;

	curr = p - sizeof (curr->length);

	/*
	 * Look for next element in the list (list is ordered by address).
	 */
	for (addr = unzip_root.next; addr != &unzip_root; addr = addr->next)
		if ((void *)addr + addr->length >= (void *)curr)
			break;

	/*
	 * Collapse before the current element or insert it in the list.
	 */
	if ((void *)addr + addr->length == curr) {
		addr->length += curr->length;
		curr = addr;
	} else {
		curr->next = addr;
		curr->prev = addr->prev;
		curr->next->prev = curr;
		curr->prev->next = curr;
	}

	/*
	 * Collapse after the current element.
	 */
	addr = curr->next;
	if (addr != &unzip_root &&
	    (void *)curr + curr->length == (void *)addr) {
		curr->length += addr->length;
		curr->next = addr->next;
		curr->next->prev = curr;
	}

	/*
	 * Try to reduce malloc'ed area.
	 */
	if ((void *)curr == unzip_brk) {
		unzip_brk += curr->length;
		curr->prev->next = curr->next;
		curr->next->prev = curr->prev;
	}
}
