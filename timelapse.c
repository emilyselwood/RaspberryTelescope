
// need this to make the time functions work.
#define _GNU_SOURCE

#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <sys/time.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>

#include "timelapse.h"
#include "stringutils.h"

// Having spent a while tracking down a typeo in the following code these make sure the time
// intervals are what we think they are.
#define NANO 1000000000L
#define MILLI 1000000L

// holds the running parameters of a time lapse run.
// these should not be accessable outside of this file.
const char * working_extension;
char * working_prefix;
int working_count = 0;
int working_interval = 0;

int working_progress = 0;

// locks to prevent bits of the program stepping on each others toes.
pthread_mutex_t parameter_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t timelapse_lock = PTHREAD_MUTEX_INITIALIZER;

// the worker thread id if it exists.
pthread_t worker;
bool working = false;

bool should_cancel = false;

long seconds_to_nanos(const long seconds) {
	return seconds * NANO;
}

void nanos_to_time(struct timespec * target, const long nanos) {
	target->tv_sec = nanos / NANO;
	target->tv_nsec = nanos % NANO;
}

// Borrowed fron the timeval man page.
int timeval_subtract ( struct timeval *result, struct timeval *x, struct timeval *y) {
	/* Perform the carry for the later subtraction by updating y. */
	if (x->tv_usec < y->tv_usec) {
		int nsec = (y->tv_usec - x->tv_usec) / MILLI + 1;
		y->tv_usec -= MILLI * nsec;
		y->tv_sec += nsec;
	}
	if (x->tv_usec - y->tv_usec > MILLI) {
		int nsec = (x->tv_usec - y->tv_usec) / MILLI;
		y->tv_usec += MILLI * nsec;
		y->tv_sec -= nsec;
	}

	/* Compute the time remaining to wait.
		tv_usec is certainly positive. */
	result->tv_sec = x->tv_sec - y->tv_sec;
	result->tv_usec = x->tv_usec - y->tv_usec;

	/* Return 1 if result is negative. */
	return x->tv_sec < y->tv_sec;
}

// work out nano seconds between two timeval structures.
long time_diff(struct timeval * start, struct timeval * end) {
	struct timeval res;
	timeval_subtract(&res, end, start);
	return (res.tv_usec * 1000);
}

// processing function for our thread.
void * internal_capture(void * arg) {
	
	pthread_mutex_lock(&timelapse_lock);
	
	struct timeval start, end;
	
	struct timespec to_wait, waited;
	
	// these shouldn't change during a run and so this should lower contention.
	int local_count = tl_get_count();
	int local_interval = tl_get_interval();
	
	char time_buffer[15];
	
	const long nano_interval = seconds_to_nanos(local_interval);
	const int num_len = digits(local_count); // we want to have the 
	
	// file names in the format prefix_count_datestamp.extension
	const int buf_len = n_strlen(working_prefix) + num_len + n_strlen(working_extension) + 18; // 14 for date + 2 underscores + 1 dot + 1 EOS
	char * buffer = (char*) malloc(sizeof(char) * buf_len);
	
	long diff = 0;
	for(int i = 0; i < local_count; i++) {
		
		gettimeofday(&start, NULL); 
		time_t t = time(NULL);
		strftime(time_buffer, 15, "%Y%m%d%H%M%S", localtime(&t));
		
		snprintf(buffer, buf_len, "%s_%0*d_%s.%s", working_prefix, num_len, i+1, time_buffer, working_extension);
		
		printf("Timelapse capture: %s\n", buffer);
		
		tc_take_picture(buffer, false, true);
		
		pthread_mutex_lock(&parameter_lock);
		working_progress = i + 1;
		pthread_mutex_unlock(&parameter_lock);
		
		gettimeofday(&end, NULL); 
		
		// In the finally itteration it is pointless to wait.
		if(i < (local_count -1)) {
			diff = time_diff(&start, &end);
			nanos_to_time(&to_wait, nano_interval - diff);
			if(nanosleep(&to_wait, &waited) == -1) { // we may get asked to cancle while asleep.
				pthread_mutex_lock(&parameter_lock);
				if(should_cancel) {
					pthread_mutex_unlock(&parameter_lock);
					break;
				}
				pthread_mutex_unlock(&parameter_lock);
			}
		}
		
		pthread_mutex_lock(&parameter_lock);
		if(should_cancel) {
			pthread_mutex_unlock(&parameter_lock);
			break;
		}
		pthread_mutex_unlock(&parameter_lock);
	}
	
	free(buffer);
	free(working_prefix);
		
	working = false;
	pthread_mutex_unlock(&timelapse_lock);
	return NULL;	// end of thread;
}

int tl_start(const int interval, const int count, char * prefix, const char * extension) {
	if(tl_in_progress()) {
		return -1;
	}
	
	// With the two locks here I don't think its possible to create two threads.
	// either way we shouldn't be creating stupid numbers of these threads.
	// there should only ever be one in play.
	pthread_mutex_lock(&timelapse_lock);
	pthread_mutex_lock(&parameter_lock);
	working_interval = interval;
	working_count = count;
	
	working_prefix = prefix;
	working_extension = extension;
	
	working = true;
	should_cancel = false;
	
	pthread_mutex_unlock(&timelapse_lock);
	pthread_create(&worker, NULL, &internal_capture, NULL);
	pthread_mutex_unlock(&parameter_lock);
	return 0;
}

int tl_get_count() {
	pthread_mutex_lock(&parameter_lock);
	int result = working_count;
	pthread_mutex_unlock(&parameter_lock);
	return result;
}

int tl_get_progress() {
	pthread_mutex_lock(&parameter_lock);
	int result = working_progress;
	pthread_mutex_unlock(&parameter_lock);
	return result;
}

int tl_get_interval() {
	pthread_mutex_lock(&parameter_lock);
	int result = working_interval;
	pthread_mutex_unlock(&parameter_lock);
	return result;
}
 
bool tl_in_progress() {
	pthread_mutex_lock(&parameter_lock);
	bool result = working;
	pthread_mutex_unlock(&parameter_lock);
	return result;
}

void tl_cancel() {
	if(tl_in_progress()) {
		pthread_mutex_lock(&parameter_lock);
		should_cancel = true;
		pthread_kill(worker,SIGALRM);	// signal the thread so it should wake up if asleep
		pthread_mutex_unlock(&parameter_lock);
	}
}

void tl_wait() {
	pthread_join(worker, NULL);
}

int tl_get_status(FILE * output) {
	fprintf(output, "{\n");
	
	if(tl_in_progress()) {
		fprintf(output, "\t\"status\" : \"processing\",\n");
		fprintf(output, "\t\"interval\" : \"%d\",\n", tl_get_interval());
		fprintf(output, "\t\"number\" : \"%d\",\n", tl_get_count());
		fprintf(output, "\t\"current\" : \"%d\"\n", tl_get_progress());
	}
	else {
		fprintf(output, "\t\"status\" : \"off\"\n");
	}
	fprintf(output, "}\n");
	fflush(output);
	
	return 0;
}