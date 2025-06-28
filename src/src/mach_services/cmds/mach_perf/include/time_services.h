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

#ifndef _TIME_SERVICES_H
#define _TIME_SERVICES_H

#include <mach_perf.h>

struct time_info {
	struct thread_basic_info thread_basic_info;
	time_value_t timeval;
};

struct time_stats {
	mach_port_t		thread;
	struct time_info	before;
	struct time_info	after;
	struct stats		total_per_op;
	struct stats 		user_per_op;
	struct stats 		system_per_op;
	struct stats 		total;
};

extern mach_msg_type_number_t	thread_info_count;

extern mach_port_t 		clock;

extern struct time_stats 	client_stats;
extern struct time_stats 	server_stats;

extern mach_port_t 		host;

#define	_start_time(ts, prof) { 					      \
	if (use_timer) {						      \
		MACH_CALL(clock_get_time, (timer_port, &ts.before.timeval));  \
	} else {							      \
		if (prof)						      \
			do_prof_start();				      \
		MACH_CALL(thread_info,					      \
			  (ts.thread,				 	      \
			   THREAD_BASIC_INFO,			  	      \
			   (thread_info_t)&ts.before.thread_basic_info,	      \
			   &thread_info_count));		 	      \
		(void) host_get_time(host, &ts.before.timeval);		      \
	}								      \
}

#define	_stop_time(ts, prof) {						      \
	if (use_timer) {						      \
		MACH_CALL(clock_get_time, (timer_port, &ts.after.timeval));   \
	} else {							      \
		(void) host_get_time(host, &ts.after.timeval);		      \
		MACH_CALL(thread_info,				      	      \
			  (ts.thread,				      	      \
			   THREAD_BASIC_INFO,			 	      \
			   (thread_info_t)&ts.after.thread_basic_info,	      \
			   &thread_info_count));			      \
		if (prof)						      \
			do_prof_stop();					      \
	}								      \
}

#define	start_time() _start_time(client_stats, prof_opt)
#define	stop_time() _stop_time(client_stats, prof_opt)

#define ONE_SEC 1000000
#define ONE_MS	1000

#define MIN_LOOPS 10

double 				elapsed();
double 				total_time();
double 				user_time();
double 				system_time();

extern int			loops;
extern int			nruns;
extern int			time_verbose;
extern int			iopt, topt;
extern double			min_elapsed;
extern int			max_loops;

extern int			(*more_measures)();
extern int			no_measure();

extern boolean_t		use_timer;
extern mach_port_t		timer_port;
extern double			timer_overhead;
extern int			timer;

#endif /* _TIME_SERVICES_H */






