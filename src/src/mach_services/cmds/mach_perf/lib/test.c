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

char *version_string = "1.1";

int		loops;		/* number of iterations */
int		nruns;		/* number of runs */
int		verbose;	/* print detailed timing informations */
int		iopt;		/* User specified number of iterations */
int		topt;		/* User specified tests */
int		max_loops;
int		min_loops;	
int		debug;
int		no_cache;	/* do not cache */
int		(*more_measures)();	/* test specific */
int		(*start_test)();	/* test specific */
int		(*stop_test)();		/* test specific */
char	 	*command_name;
int 		standalone;
boolean_t	need_banner = FALSE;
int 		vm_opt;
int 		halt_opt;

double	min_elapsed; 	       	/* time each measure must last */
double	max_elapsed_ratio = 1.5;/* min_elapsed <= measured_time
				 * <= min_elapsed * max_elapsed_ratio */

#define MIN_VALID_TIME (50 * ONE_MS)

#define MAX_CALIBRATIONS 20

#undef tests
#undef private_options

null_func() {
}

reset_options()
{
	loops = 0;		/* number of iterations */
	nruns = 3;		/* number of runs */
	verbose = 0;	/* print detailed timing informations */
	min_elapsed = (1 * ONE_SEC); 	/* time each measure must last */
	iopt = 0;		/* User specified number of iterations */
	topt = 0;		/* User specified tests */
	max_loops = 0;
	min_loops = MIN_LOOPS;	
	debug = 0;
	debug_ressources = 0;
	need_banner = TRUE;
	more_measures = server_time;
	start_test = start_server_test;
	stop_test = stop_server_test;
	no_cache = 0;
	vm_opt = 0;
	client_only = FALSE;
	server_only = FALSE;
	server_times = 0;
	server = MACH_PORT_NULL;
	server_task = MACH_PORT_NULL;
	use_opt = 0;
	norma_node = NORMA_NODE_NULL;
	prof_opt = 0;
	prof_trunc = 0;
	use_timer = FALSE;
	timer = -1;
	reset_time_stats(&server_stats);
}

jmp_buf test_jmp_buf;
int skip_test;

measure(test)
struct test *test;
{
	register i;
	int calibrations = 0;
	int prev_loops = 0;
	int reversions = 0;	/* number of times calibrate()
				   decreased loops */

	if (!iopt)
		loops = min_loops;
	max_loops = 0;

	reset_time_stats(&client_stats);
	prof_reset();
	server_in_same_task = FALSE;
	if (verbose) {
		printf("\n");
		print_header(test);
		printf("\n");
	} else
		print_header(test);

	for (i = 0; i < nruns;) {
		prev_loops = loops;	       
		ressources_start();
		if (verbose)
			printf("run %3d: %8d loops       ", i+1, loops);
		if (mach_setjmp(test_jmp_buf)) {
			skip_test = 0;
			printf(" test failed\n");
			return;
		} else {
			skip_test = 1;
			(*start_test)();
			(*test->func)(test->arg0, test->arg1,
				      test->arg2, test->arg3);
			(*stop_test)();
			skip_test = 0;
		}
		ressources_stop();
		if (verbose)
			print_one_sample(0, &client_stats);
		if (!run_is_ok()) {
			
			if (calibrations++ > MAX_CALIBRATIONS) {
			  printf("could not calibrate after %d runs, use -v\n",
				 calibrations);
			  return;
			}
			calibrate(test, (reversions > 1));
			if (loops < prev_loops)
				reversions++;
			prof_drop();
			continue;
		} else {
			calibrations = 0;
			reversions = 0;
		}
		collect_stats(&client_stats, loops);
		prof_save();
		(*more_measures)(i+1);
		i++;
	}
	if (verbose)
		printf("Average                       ");
	print_results(&client_stats);
	ressources_use("Test ");       
	(*more_measures)(0);
	prof_print();
} 

int exit_in_progress;

test_exit(format, routine, file, line, error)
char *format, *routine, *file, *error;
{
	if (exit_in_progress)
		return;
	if (skip_test && !verbose)
		mach_longjmp(test_jmp_buf, 1);
	printf(format, routine, file, line, error);
	if (halt_opt) {
		if (standalone) {
			MACH_CALL(host_reboot, (privileged_host_port,
						0x1000 /* RB_DEBUGGER */));
		} else {
                        MACH_CALL(task_suspend, (mach_task_self()));
		}
	}
	if (is_master_task) {
		if ((server != MACH_PORT_NULL) && !client_only) {
			if (debug)
				printf("kill_server(%x)\n", server);
			kill_server(server);
		}
		leave(1);
	}
	else
		task_terminate(mach_task_self());
}

print_banner()
{
	if (!prof_opt && !need_banner)
		return;
	need_banner = FALSE;
	printf("%d run(s), min elapsed %8.3f ms\n\n", nruns, min_elapsed/ONE_MS);
	printf("test name                     nops/sec  Elapsed     User   System  +/- %%  Total\n");
	printf("                                          us/op    us/op    us/op           sec\n\n");

}

char *strncpy();

print_header(test)
struct test *test;
{
	char name[31], len;

	print_banner();
	strncpy(name, test->name, 31);
	if ((len = strlen(test->name)) < 30)
	while (len < 30)
		name[len++] = ' ';
	name[30] = 0;
	printf("%s", name);
}

print_results(ts)
struct time_stats *ts;
{
	double avg;
	double dev;

	avg = average(&ts->total_per_op);
	dev = deviation(&ts->total_per_op)*100/average(&ts->total_per_op);
	if (dev < 0.005) /* Otherwise standalone printf formats incorrectly */
		dev = 0.0;
	printf("%8.0f", ONE_SEC/avg);
	printf(" %8.2f %8.2f %8.2f %6.2f %6.2f\n",
	       average(&ts->total_per_op),
	       average(&ts->user_per_op),
	       average(&ts->system_per_op),
	       deviation(&ts->total_per_op)*100/average(&ts->total_per_op),
	       ts->total.xsum/ONE_SEC);
}

print_one_sample(test, ts) 
struct test *test;
struct time_stats *ts;
{
	double tot;

	if (test)
		print_header(test);
	tot = (total_time(ts)/ONE_SEC);
	printf("%8.0f", loops/tot);
	printf(" %8.2f %8.2f %8.2f        %6.2f\n",
	       total_time(ts)/loops,
	       user_time(ts)/loops,
	       system_time(ts)/loops,
	       total_time(ts)/ONE_SEC);
}

calibrate(test, smooth)
struct test *test;
boolean_t smooth;
{
	double new_loops = loops;
	struct time_stats *ts = &client_stats;
	double total = total_time(ts);

	if (total < min_elapsed/20) /* get meaningful relative time 5 % */
		new_loops *= 10;
	else if (total < min_elapsed)
		if (smooth)
			new_loops += new_loops * 
			             ((min_elapsed / total_time(ts)) - 1) *
				     0.5 ;
		else
			new_loops *= min_elapsed * 1.05 / total_time(ts);
	else
		new_loops *= min_elapsed * 0.95 / total_time(ts);
	if (max_loops && (new_loops > max_loops))
		new_loops = max_loops;
	loops = new_loops;
}

char *
get_basename(name)
char *name;
{
	char *cn = name; 

	while (*name)
		if (*name++ == '/')
			cn = name;
	return(cn);
}


is_gen_opt(argc, argv, index, tests, private_options)	
char *argv[];
int argc, *index;
struct test *tests;
char *private_options;
{
	int i,j;
	int ret = 1;

	command_name = get_basename(argv[0]);

	i = *index;

	if (debug > 3)
		printf("token %s\n", argv[i]);
	if (i+1 < argc && debug)
		printf("token+1 %s\n", argv[i+1]);
	if (!strcmp(argv[i], "-i")) {
		if (++i >= argc || *argv[i] == '-')
			usage();
		iopt++ ;
		if (!atod(argv[i], &loops))
			usage();
	} else if (!strcmp(argv[i], "-ii")) {
		if (++i >= argc || *argv[i] == '-')
			usage();
		if (!atod(argv[i], &min_loops))
			usage();
	} else if (!strcmp(argv[i], "-r")) {
		if (++i >= argc || *argv[i] == '-')
			usage();
		if (!atod(argv[i], &nruns))
			usage();
	} else if (!strcmp(argv[i], "-v")) {
		verbose++;
	} else if (!strcmp(argv[i], "-vm_stats")) {
		vm_opt++;
	} else if (!strcmp(argv[i], "-debug_resources")) {
		debug_ressources++;
	} else if (!strcmp(argv[i], "-d")) {
		if (i+1 >= argc || *argv[i+1] == '-')
			debug++;
		else if (!atod(argv[++i], &debug))
			usage();
	} else if (!strcmp(argv[i], "-timer")) {
		if (i+1 >= argc || *argv[i+1] == '-')
			timer = REALTIME_CLOCK;
		else if (!atod(argv[++i], &timer))
			usage();
		measure_timer(timer);
	} else if (!strcmp(argv[i], "-min")) {
		if (++i >= argc || *argv[i] == '-' || iopt)
			usage();
		if (!atod(argv[i], &j))
			usage();
		min_elapsed = j * ONE_MS;
		if (min_elapsed < MIN_VALID_TIME) {
			printf("min_elapsed must be at least %d ms\n",
			       MIN_VALID_TIME/ONE_MS);
			leave(1);
		}
	} else if (!strcmp(argv[i], "-prof")) {
		prof_opt++;
		if (standalone) {
			if (i+1 < argc) {
				if ((!strcmp(argv[++i], "-trunc")) &&
				    i+1 < argc &&
				    atod(argv[++i], &j))
					prof_trunc = j;
				else
					usage();
			}
		} else {
			while(i+1 < argc) {
				strcat(prof_command, " ");
				strcat(prof_command, argv[++i]);
			}
		}
	} else if (!strcmp(argv[i], "-no_cache")) {
		no_cache++;
	} else if (!strcmp(argv[i], "-prof_unit")) {
		extern int prof_alloc_unit;
		if (++i >= argc || *argv[i] == '-')
			usage();
		if (!atod(argv[i], &prof_alloc_unit))
			usage();
	} else if (!strcmp(argv[i], "-t")) {
		if (++i >= argc || *argv[i] == '-')
			usage();
		if (is_a_test(argv[i], tests))
			topt++;
		else
			usage();
        } else if (!strcmp(argv[i], "-h")) {
	  	halt_opt = 1;
		if (standalone) {
			MACH_CALL(host_reboot, (privileged_host_port,
						0x1000 /* RB_DEBUGGER */));
		} else {
                        MACH_CALL(task_suspend, (mach_task_self()));
		}
	} else if (!strcmp(argv[i], "-tn")) {
		if (i+1 >= argc || *argv[i+1] == '-')
			usage();
		while (i+1 < argc && *argv[i+1] != '-') {
			int tn;

			i++;
			if (!atod(argv[i], &tn))
				usage();
			if (is_a_test_number(tn, tests))
				topt++;
			else
				usage();
		}
	} else
		ret = 0;
	*index = i;
	return(ret);
}

show_tests(tests)
struct test *tests;
{
	int i = 0;
	printf("\n\ttests are:\n\n");
	while (tests->name) {
		if (tests->func) 
			printf("\ttest %3d\t%s\n", i++, tests->name);
		else if (strlen(tests->name))
			printf("\n\t%s\n\n", tests->name);
		else
			printf("\n");
		tests++;
	}
}

is_a_test(test, tests)
char *test;
struct test *tests;
{
	int ret = 0;
	while (tests->name) {
		if (tests->func &&
		    !strncmp(test, tests->name, strlen(test))) {
			tests->enabled++;
			ret++;
		}
		tests++;
	}
	return(ret);
}

is_a_test_number(n, tests)
struct test *tests;
{
	int ret = 0;
	int i = 0;

	while (tests->name) {
		if (tests->func && n == i++) {
			tests->enabled++;
			return(1);
		}
		tests++;
	}
	return(0);
}


run_tests(tests)
struct test *tests;
{
	char *title = 0;
	need_banner = TRUE;

	if(!standalone) {
	  	version();
		kernel_version();
		print_vm_stats();
	}
	prof_init();
	if (!prof_opt)
		print_banner();
	while (tests->name) {
		if (!tests->func)
			title = tests->name;
		else if (tests->enabled || !topt) {
			if (title) {
				if (strlen(title))
					printf("\n%s\n\n", title);
				else
					printf("\n");
				title = 0;
			}
			tests->enabled = 0;
			measure(tests);
		}
		tests++;
	}
	printf("\n");
	print_vm_stats();
	prof_terminate();
}

char *gen_opts = "\
\t[-min <milli_secs>]      Set minimum time for each run\n\
\t                         (default is 1000, exclusive with -i).\n\
\t[-r <runs>]              Set number of runs (default is 3).\n\
\t[-i <iterations>]        Set number of iterations per run\n\
\t                         (exclusive with -min).\n\
\t[-vm_stats]              Print vm statistics.\n\
\t[-t <test_name> ]        Run tests for which name\n\
\t                         starts with <test_name>.\n\
\t[-tn <test_numbers>]     Run tests number <test_numbers>.\n\
\t[-timer [<timer_id>]]    Use timer <timer_id> (default 0) for\n\
\t                         measurements instead of host_get_time() and\n\
\t                         thread_info(). In this case user mode time,\n\
\t                         system mode time and profiling data\n\
\t                         are meaningless.\n\
\t[-ii <iterations>]       Start test calibration with <iterations>\n\
\t                         iterations (default is 10).\n\
\t[-v]                     Verbose mode: print results for all runs\n\
\t                         and print extended status in case of failure.\n\
\t[-d [<debug_level>]      Set debug level (default is 0).\n\
\t[-debug_resources]       Set resources debug on (default is 0).\n\
\t[-h]                     halt: at start time suspend thread or enter\n\
\t                         kernel debugger if running standalone.\n\
\t[-prof [<mprof args>]]   Profile microkernel task. When running\n\
\t                         standalone, -trunc <val> is the only\n\
\t                         meaningful profile argument.\n\
";

#if 0
\t[-no_cache]              Disables caching\n\
\t                         (some tests are executed at start an extra\n\
\t                         time to remove noise from kernel allocating\n\
\t                         large tables like ipc splay trees)\n\

#endif


print_usage(tests, private_options)
struct test *tests;
char *private_options;
{
	printf("%s	", command_name);
	if (private_options && strlen(private_options))
		printf("%s\n", private_options);
	printf("%s", gen_opts);
	show_tests(tests);
	leave(1);
}

more_test_init()
{
	vm_offset_t addr = 0;

	privileged_user();
	if (vm_allocate(mach_task_self(),
		       &addr,
		       vm_page_size,
		       FALSE) == KERN_SUCCESS)
		MACH_CALL(vm_protect, (mach_task_self(),
				       addr,
				       vm_page_size,
				       FALSE,
				       VM_PROT_NONE));
	time_init();
}

atod(s, i)
char *s;
int *i;
{
	char c;

	*i = 0;
	while(c = *s++) {
		if (c >= '0' && c <= '9')
			*i = (*i)*10 + c - '0';
		else
			return(0);
	}
	return(1);			
}

print_command_line(argc, argv)
char *argv[];
{
	register i;

	printf("\n");
	printf("%s ", get_basename(argv[0]));
	for (i=1; i < argc; i++) {
		printf("%s ", argv[i]);
	}
	printf("\n\n");
}

kernel_version()
{
	kernel_version_t version;

	MACH_CALL(host_kernel_version, (mach_host_self(),
					&version[0]));
	printf("%s\n", version);
}

version()
{
	printf("Mach Microkernel benchmarks version %s\n\n", version_string);
}

leave(status)
{
	mach_thread_t thread = printf_thread;

	if (debug > 1)
		printf("leave(%d)\n", status);
	debug = 0;
	exit_in_progress = 1;
	if ((!standalone) && printf_thread) {
		printf_thread = (mach_thread_t) 0;
		kill_thread(thread);
	}
	exit(status);
}

