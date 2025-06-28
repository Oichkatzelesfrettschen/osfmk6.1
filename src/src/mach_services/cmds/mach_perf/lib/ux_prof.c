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

#include <mach_perf.h>
#include <sys/types.h>
#include <stdio.h>
#include <sys/signal.h>

char prof_command[1024] = "mprof -print -";

prof_print()
{
	FILE *input;
	register i;
	struct sample_buf *buf;

	if (!prof_opt)
		return;

	prof_time_stop.seconds = client_stats.total.xsum / 1000000;
	prof_time_stop.microseconds = client_stats.total.xsum -
	  				prof_time_stop.seconds * 1000000;

	if (debug > 1)
		printf("prof_print(): %s\n", prof_command);

	if ((input = popen(prof_command, "w")) == (FILE *)-1) {
		perror(prof_command);
		exit(1);
	}
	if (debug > 1)
		printf("writing start time\n");
	if (fwrite(&prof_time_start, sizeof(prof_time_start), 1, input) != 1) {
		perror("prof_print of start time");
		exit(1);
	}
	for (i = sample_count, buf = sample_bufs; i; buf = buf->next) {
		int count;

		count = samples_per_buf - buf->free_count;
		if (debug > 1)
			printf("prof_print writes %d samples from %x\n", count, buf->samples);
		if (fwrite(buf->samples,
			   sizeof(unsigned),
			   count,
			   input) != count) {
			perror("prof_print of samples");
			exit(1);
		}
		i -= count;
	}
	if (debug > 1)
		printf("writing stop time\n");
	if (fwrite(&prof_time_stop, sizeof(prof_time_stop), 1, input) != 1) {
		perror("prof_print of stop time");
		exit(1);
	}
	pclose(input);
	printf("\n");
}




