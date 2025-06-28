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


#define TEST_NAME sched

#include <mach_perf.h>

typedef int priority_t;

int base_pri = -1;
int max_pri = -1;
int policy = POLICY_TIMESHARE;

char *private_options = "\n\
\t[-pri <base pri>]        Set base priority to <base pri>\n\
\t[-max <max pri>]         Set max priority to <max pri>\n\
\t[-policy <policy>]       Set policy to <policy>\n\
";

int thread_switch_test();
int double_thread_switch_test();
int sync_thread_test();
int sync_task_test();

struct test tests[] = {
"thread_switch(0, NONE)",	0, thread_switch_test, 0,
			           SWITCH_OPTION_NONE, 0, 0,
"thread_switch(0, DEPRESS)",	0, thread_switch_test, 0,
			           SWITCH_OPTION_DEPRESS, 0, 0,
"2 threads switch(0, DEPRESS)",	0, sync_thread_test, 0,
			           SWITCH_OPTION_DEPRESS, 0, 0,
"2 threads switch(handoff, NONE)",	0, sync_thread_test, 0,
			           SWITCH_OPTION_NONE, 1, 0,
"2 threads switch(0, NONE)",	0, sync_thread_test, 0,
			           SWITCH_OPTION_NONE, 0, 0,
"2 task switch(0, DEPRESS)",	0, sync_task_test, 0,
			           SWITCH_OPTION_DEPRESS, 0, 0,
0, 0, 0, 0, 0, 0, 0
};

int *sync;

main(argc, argv)
int argc;
char *argv[];
{
	int i;

	MACH_CALL( vm_allocate, (mach_task_self(),
				  (vm_offset_t *)&sync,
				  vm_page_size,
				  TRUE));
	MACH_CALL( vm_inherit, (mach_task_self(),
			 (vm_offset_t)sync,
			 vm_page_size,
			 VM_INHERIT_SHARE));

	test_init();
	for (i = 1; i < argc; i++)
		if (!strcmp(argv[i], "-pri")) {
			if (++i >= argc || *argv[i] == '-')
				usage();
		  	base_pri = atoi(argv[i]);
		} else if  (!strcmp(argv[i], "-max")) {
			if (++i >= argc || *argv[i] == '-')
				usage();
		  	max_pri = atoi(argv[i]);
		} else if (!strcmp(argv[i], "-policy")) {
			if (++i >= argc || *argv[i] == '-')
				usage();
			policy = atoi(argv[i]);
		} else if (!is_gen_opt(argc, argv, &i, tests, private_options))
			usage();


	run_tests(tests);
}

thread_switch_test(switch_option)
{
	register int i;
	
	start_time();
	for (i=loops; i--;)  {
		MACH_CALL( thread_switch, (0, switch_option, 1000));
	}
	stop_time();
}

volatile mach_port_t slave_thread;
mach_port_t master_thread;
extern int slave();

struct slave_args {
	int switch_option;
	int handoff;
} slave_args;

#define OLD 0
#define NEW 1
cooperate(thread, switch_option, slave)
mach_port_t thread;
{
	register int i;
	register ocount;
	int failed = 0;
	struct thread_basic_info thread_basic_info;
	mach_msg_type_number_t thread_info_count = \
	  			sizeof(struct thread_basic_info);
	processor_set_control_port_t pset;
	policy_t old_policy = POLICY_TIMESHARE;
	priority_t old_base;
	priority_t old_max;

	struct policy_timeshare_base tr_base[2];
	struct policy_fifo_base ff_base[2];
	struct policy_rr_base rr_base[2];
	policy_base_t base_pt[2];

	struct policy_timeshare_limit tr_limit[2];
	struct policy_fifo_limit ff_limit[2];
	struct policy_rr_limit rr_limit[2];
	policy_limit_t limit_pt[2];

	if (debug)
		printf("start %s\n", slave ? "slave" : "master");

	if (base_pri != -1 || max_pri != -1 || policy != POLICY_TIMESHARE) {
		pset  = get_processor_set();
		MACH_CALL(thread_info, (mach_thread_self(),
					THREAD_BASIC_INFO,
					(thread_info_t)&thread_basic_info,
					&thread_info_count));
		old_policy = thread_basic_info.policy;
		switch(old_policy) {
		case POLICY_TIMESHARE: {
		    	struct policy_timeshare_info time_share;

			thread_info_count = sizeof(time_share);
			MACH_CALL(thread_info, (mach_thread_self(),
						THREAD_SCHED_TIMESHARE_INFO,
						(thread_info_t)&time_share,
						&thread_info_count));
			old_base = time_share.base_priority;
			old_max = time_share.max_priority;
			tr_base[OLD].base_priority = time_share.base_priority;
			tr_limit[OLD].max_priority = time_share.max_priority;
			tr_base[NEW] = tr_base[OLD];
			tr_limit[NEW] = tr_limit[OLD];
			if (base_pri != -1)
				tr_base[NEW].base_priority= base_pri;
			if (max_pri != -1)
				tr_limit[NEW].max_priority = max_pri;
			base_pt[NEW] = (policy_base_t)&tr_base[NEW];
			base_pt[OLD] = (policy_base_t)&tr_base[OLD];
			limit_pt[NEW] = (policy_limit_t)&tr_limit[NEW];
			limit_pt[OLD] = (policy_limit_t)&tr_limit[OLD];
			break;
			}
		case POLICY_RR: {
		    	struct policy_rr_info round_robin;

			thread_info_count = sizeof(round_robin);
			MACH_CALL(thread_info, (mach_thread_self(),
						THREAD_SCHED_RR_INFO,
						(thread_info_t)&round_robin,
						&thread_info_count));
			old_base =  round_robin.base_priority;
			old_max =  round_robin.max_priority;
			rr_base[OLD].base_priority = round_robin.base_priority;
			rr_limit[OLD].max_priority = round_robin.max_priority;
			rr_base[NEW] = rr_base[OLD];
			rr_limit[NEW] = rr_limit[OLD];
			if (base_pri != -1)
				rr_base[NEW].base_priority = base_pri;
			if (max_pri != -1)
				rr_limit[NEW].max_priority = max_pri;
			rr_base[NEW].quantum = round_robin.quantum;
			base_pt[NEW] = (policy_base_t)&rr_base[NEW];
			base_pt[OLD] = (policy_base_t)&rr_base[OLD];
			limit_pt[NEW] = (policy_limit_t)&rr_limit[NEW];
			limit_pt[OLD] = (policy_limit_t)&rr_limit[OLD];
			break;
			}
		case POLICY_FIFO: {
		    	struct policy_fifo_info fifo;

			thread_info_count = sizeof(fifo);
			MACH_CALL(thread_info, (mach_thread_self(),
						THREAD_SCHED_FIFO_INFO,
						(thread_info_t)&fifo,
						&thread_info_count));
			old_base = fifo.base_priority;
			old_max = fifo.max_priority;
			ff_base[OLD].base_priority = fifo.base_priority;
			ff_limit[OLD].max_priority = fifo.max_priority;
			ff_base[NEW] = ff_base[OLD];
			ff_limit[NEW] = ff_limit[OLD];
			if (base_pri != -1)
				ff_base[NEW].base_priority = base_pri;
			if (max_pri != -1)
				ff_limit[NEW].max_priority = max_pri;
			base_pt[NEW] = (policy_base_t)&ff_base[NEW];
			base_pt[OLD] = (policy_base_t)&ff_base[OLD];
			limit_pt[NEW] = (policy_limit_t)&ff_limit[NEW];
			limit_pt[OLD] = (policy_limit_t)&ff_limit[OLD];
			break;
			}
		}

		if (debug) {
			printf("old policy %s, old base pri: %d\n",
			       (old_policy == POLICY_TIMESHARE) ?
					"POLICY_TIMESHARE" :
				(old_policy == POLICY_FIFO) ?
					"POLICY_FIFO" :
				 (old_policy == POLICY_RR) ?
			                "POLICY_RR" :
					"unknown",
			       old_base);
		}
			
		if (policy != old_policy || base_pri != -1 || max_pri != -1 ) {
			MACH_CALL(thread_set_policy, (mach_thread_self(),
						      pset,
						      policy,
						      &base_pt[NEW],
						      &limit_pt[NEW]));
		}
	}

	if (slave) {
		*(sync+1) = -1;
		while (*(sync+1) != 0)
			thread_switch(MACH_PORT_NULL, SWITCH_OPTION_NONE, 0);
		if (debug > 1)
			printf("continue slave\n");
		ocount = 0;
	} else {
		while (*(sync+1) != -1)
			thread_switch(MACH_PORT_NULL, SWITCH_OPTION_NONE, 0);
		if (debug > 1)
			printf("continue master\n");
		*(sync+1) = 0;
		ocount = -1;
	}

	if (!slave)
		start_time();
	for (i=loops; i--;)  {
		while (*sync == ocount && *sync > 0) {
			failed++;
			MACH_CALL( thread_switch,  (thread,
						    switch_option,
						    10000));
		}
		ocount = *sync + 1;
		(*sync)++;
		if (debug > 1)
			printf("%s switch(%x, %x) loop %d\n",
			       slave ? "slave" : "master",
			       thread, switch_option, i);
		MACH_CALL( thread_switch, (thread, switch_option, 10000));
	}
	if (!slave)
		stop_time();
	if (policy != old_policy || base_pri != -1 || max_pri != -1 ) {
		MACH_CALL(thread_set_policy, (mach_thread_self(),
					      pset,
					      old_policy,
					      &base_pt[OLD],
					      &limit_pt[OLD]));
	}
	if (failed && debug)
		printf("%d failures for %s\n", failed, slave ? "slave" : "master");
	if (debug)
		printf("%s done\n", slave ? "slave" : "master");
}

sync_slave(args)
struct slave_args *args;
{
	mach_port_t thread = 0;
	int switch_option = args->switch_option;

	slave_thread = mach_thread_self();
	if (args->handoff)
		thread = master_thread;
	cooperate(thread, switch_option, 1);
	*(sync+1) = 1;
	while(1)
		thread_switch(MACH_PORT_NULL, SWITCH_OPTION_NONE, 0);
}

sync_thread_test(thread, switch_option, handoff)
{
	mach_thread_t child;

	slave_thread = 0;
	*sync = 0;
	*(sync+1) = 0;
	master_thread = mach_thread_self();
	slave_args.switch_option = switch_option;
	slave_args.handoff = handoff;
	new_thread(&child, sync_slave, &slave_args);
	while (!slave_thread)
		thread_switch(MACH_PORT_NULL, SWITCH_OPTION_NONE, 0);
	if (handoff)
		thread = slave_thread;
	cooperate(thread, switch_option, 0);
	if (debug > 1)
		printf("master waits %d\n", slave_thread);
	while(!*(sync+1))
		thread_switch(MACH_PORT_NULL, SWITCH_OPTION_NONE, 0);
	kill_thread(child);
	if (debug > 1)
		printf("master exits\n");
}

sync_task_test(thread, switch_option)
{
	mach_port_t slave_task;

	*sync = 0;
	*(sync+1) = 0;
	if (!(slave_task = mach_fork())) {
		
		cooperate(0, switch_option, 1);
		*(sync+1) = 1;
		MACH_CALL(task_terminate, (mach_task_self()));
	} else {
		cooperate(0, switch_option, 0);
		while(!*(sync+1))
			thread_switch(MACH_PORT_NULL, SWITCH_OPTION_NONE, 0);
		task_terminate(slave_task);
		MACH_CALL(mach_port_destroy, (mach_task_self(), slave_task));
	}
}


