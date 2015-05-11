#ifndef __TIME_LAPSE_H
#define __TIME_LAPSE_H

#include "telescopecamera.h"

/**
 * Capture a frame every interval seconds until count frames have been taken
 * prefix is not const because we will be freeing it when the time lase finishes.
 */
int tl_start(const int interval, const int count, char * prefix);

/**
 * Several getters for the status of the timelapse system
 * these should always be used to find out this information,
 * they handle the locking correctly.
 */
int tl_get_count();

int tl_get_progress();

int tl_get_interval();

/**
 * Return true if a time lapse is in progress
 */
bool tl_in_progress();

/**
 * request that the timelapse thread should exit as soon as possible.
 * Sends the timelapse thread a SIGALRM signal. This needs to be caught some where.
 */
void tl_cancel();

/**
 * wait for the timelapse thread to end
 */
void tl_wait();

/**
 * return a simple json document showing the current status of the timelapse
 */
int tl_get_status(FILE * output);



#endif
