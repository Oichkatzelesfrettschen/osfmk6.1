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

mach_port_t 		host;		/* Used to acquire time */
struct time_stats 	client_stats;
struct time_stats 	server_stats;
mach_msg_type_number_t	thread_info_count;
mach_port_t 		mach_clock;

/*
 * Returns elapsed microseconds
 */

double
elapsed(before_time , after_time)
time_value_t *before_time, *after_time;
{
	double start, stop;

	start = before_time->seconds * ONE_SEC;
	stop = after_time->seconds * ONE_SEC;
	if (use_timer) {
	  	/*
		 * timers return nanoseconds 
		 */
		start += before_time->microseconds/1000;
		stop += after_time->microseconds/1000;
		stop -= timer_overhead;
	} else {
		start += before_time->microseconds;
		stop += after_time->microseconds;
	}
	return stop-start;
}

double
total_time(ts)
struct time_stats *ts;
{
	return(elapsed(&ts->before.timeval, &ts->after.timeval));
}

double
user_time(ts)
struct time_stats *ts;
{
	if (use_timer)
		return(0.0);
	return(elapsed(&ts->before.thread_basic_info.user_time,
		       &ts->after.thread_basic_info.user_time));
}

double
system_time(ts)
struct time_stats *ts;
{
	if (use_timer)
		return(0.0);
	return(elapsed(&ts->before.thread_basic_info.system_time,
		       &ts->after.thread_basic_info.system_time));
}

int time_initialized = 0;

time_init()
{
	tvalspec_t time = {0, 0};

	host = mach_host_self();
	client_stats.thread = mach_thread_self();
	thread_info_count = sizeof (struct thread_basic_info);
	MACH_CALL(host_get_clock_service, (host,
					   REALTIME_CLOCK,
					   &mach_clock));
	MACH_CALL(clock_get_time, (mach_clock, &time));
	reset_time_stats(&client_stats);
	reset_time_stats(&server_stats);

#if 0
	measure_timer(REALTIME_CLOCK);
	measure_timer(BATTERY_CLOCK);
	measure_timer(HIGHRES_CLOCK);
#endif /* 0 */
}

hertz()
{
	unsigned resolution;
	unsigned count;

	count = 1;
	MACH_CALL( clock_get_attributes, (mach_clock,
					  CLOCK_ALARM_CURRES,
					  &resolution,
					  &count));

	return (NSEC_PER_SEC/resolution);
}
       
boolean_t		use_timer;
mach_port_t		timer_port;
double			timer_overhead;
int			timer;
		
measure_timer(timer)
{
	tvalspec_t Start_time = {0, 0};
	tvalspec_t Stop_time = {0, 0};
	unsigned resolution;
	unsigned count = 1;
	register unsigned i;
	struct stats timer_stat;
	double overhead, base;
	double dev;

	if (debug)
		printf("measure(%d)\n", timer);
	host = mach_host_self();
	MACH_CALL(host_get_clock_service , (host, timer, &timer_port));

	MACH_CALL( clock_get_attributes, (timer_port,
					  CLOCK_GET_TIME_RES,
					  &resolution,
					  &count));

	resolution /= ONE_MS;

	if (resolution > 1000) {
		printf("timer %d: resolution to low (%d us)\n",
		       timer, resolution);
		leave(1);
	}
	
	reset_sampling(&timer_stat);

	MACH_CALL(clock_get_time, (timer_port, &Start_time));
	MACH_CALL(clock_get_time, (timer_port, &Stop_time));
	base = (Stop_time.tv_sec - Start_time.tv_sec) * ONE_SEC +
		   (Stop_time.tv_nsec - Start_time.tv_nsec)/1000;

	MACH_CALL(clock_get_time, (timer_port, &Start_time));
	MACH_CALL(clock_get_time, (timer_port, &Stop_time));
	overhead = (Stop_time.tv_sec - Start_time.tv_sec) * ONE_SEC +
		   (Stop_time.tv_nsec - Start_time.tv_nsec)/1000;

	if (overhead < base)
		base = overhead;


	for (i = 0; i < 100; i++) {
		MACH_CALL(clock_get_time, (timer_port, &Start_time));
		MACH_CALL(clock_get_time, (timer_port, &Stop_time));
		overhead = (Stop_time.tv_sec * ONE_SEC + 
			    Stop_time.tv_nsec / 1000 ) -
			      (Start_time.tv_sec * ONE_SEC +
			       Start_time.tv_nsec / 1000);
		if (debug)
			printf("1: %6.2f %s\n",
			       overhead,
			       (overhead > 1.10*base) ? "dropped" : "");
		if (overhead > 1.10*base)
			continue;
		collect_sample(&timer_stat, overhead);
	}
	overhead = average(&timer_stat);
	dev = deviation(&timer_stat)*100/overhead;
	if (verbose || dev > 1.0)
         printf(
	   "timer %d: %d us resolution, kcall overhead %8.2f us (+/- %5.2f)\n",
	   timer,
	   resolution,
	   overhead, 
	   dev);
	use_timer = TRUE;
	timer_overhead = overhead;
	min_elapsed = overhead * 1000;
}

reset_time_stats(ts)
struct time_stats *ts;
{
	reset_sampling(&ts->total_per_op);
	reset_sampling(&ts->user_per_op);
	reset_sampling(&ts->system_per_op);
	reset_sampling(&ts->total);
}
	

collect_stats(ts, loops)
struct time_stats *ts;
{
	double sample;

	sample = total_time(ts)/loops;
	collect_sample(&ts->total_per_op, sample);
	sample = user_time(ts)/loops;
	collect_sample(&ts->user_per_op, sample);
	sample = system_time(ts)/loops;
	collect_sample(&ts->system_per_op, sample);
	collect_sample(&ts->total, total_time(ts));
}
