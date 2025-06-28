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

#ifndef _TEST_SERVICES_H
#define _TEST_SERVICES_H

#include <mach_perf.h>

extern int		loops;
extern int		nruns;
extern int		verbose;
extern int		iopt, topt;
extern double		min_elapsed;
extern double		max_elapsed_ratio;
extern int		max_loops;
extern int		debug;
extern int		prof_opt;
extern int		min_percent;
extern int 		show_ressources;
extern int 		total_runs;
extern int		standalone;
extern int		no_cache;
extern int		vm_opt;

extern int		(*more_measures)();
extern int		(*start_test)();
extern int		(*stop_test)();
extern int		no_measure();
extern boolean_t	need_banner;

extern char *time_opts;
/* extern char *private_options; */

struct test {
	char	*name;
	int	enabled;
	int	(*func)();
	int	arg0, arg1, arg2, arg3;
};

#define test_init() {					\
	if (argc) {					\
	  	reset_options();			\
		print_command_line(argc, argv);		\
	}						\
	threads_init();					\
	_printf_init();					\
	more_test_init();				\
}

/*
 * Did a test run long enough ? 
 */

#define run_is_ok()							     \
	(iopt ||							     \
	 ((total_time(&client_stats) < (min_elapsed * max_elapsed_ratio)) && \
	  (((!max_loops) && total_time(&client_stats) >= min_elapsed) ||     \
	   (max_loops && (total_time(&client_stats) >= min_elapsed ||	     \
			  loops >= max_loops)))))

/*
 * Some tests imply large ressource allocations 
 * of kernel objects. The goal is not to measure
 * the ressource management but the cost of a single
 * kernel service. To be sure the ressources allocation
 * does not influence this cost, we rerun the test to cache
 * this ressource allocation overhead (like port name
 * splay trees)
 */

#define test_cache(func, args)				\
{							\
	static func##_cached = 0;	        	\
	if (func##_cached < loops &&			\
	    run_is_ok() &&				\
	    !no_cache) {				\
		if (debug)				\
			printf("cache loop\n");		\
		prof_drop();				\
		func##_cached = loops;			\
		func args;				\
	}						\
}

#define set_max_loops(l)			\
	max_loops = l;				\
	if (!iopt && loops > max_loops)		\
		loops = max_loops;		

#define usage()	print_usage(tests, private_options)

#define concatenate_2(x, y)	x##y
#define concatenate(x, y)	concatenate_2(x, y)

extern mach_port_t privileged_host_port;
extern mach_port_t host_port;

extern mach_port_t bootstrap_port;
extern mach_port_t master_host_port;
extern mach_port_t master_device_port;

#if	STANDALONE
#define main TEST_NAME

#define private_options concatenate(TEST_NAME, _private_options)
#define tests		concatenate(TEST_NAME, _tests)

#define TEST(a, b) # a, a, b 

struct test_dir {
	char		*name;
	int		(*func)();
	int		is_a_test;
};

#endif	/* STANDALONE */


#endif /* _TEST_SERVICES_H */








