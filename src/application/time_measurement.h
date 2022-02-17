#ifndef TIMEMEASUREMENT_H
#define TIMEMEASUREMENT_H

#include <time.h>



void timespec_diff(struct timespec *start, struct timespec *stop,
                   struct timespec *result);


#endif