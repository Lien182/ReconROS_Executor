#ifndef HELPERS_H
#define HELPERS_H


#define RECONROS_EXECUTOR_MAX_NR_OF_OBJECTS 10

typedef void * (*function_ptr) (void *arg);

enum ReconROS_primitive{ ReconROS_TMR = 0, ReconROS_SUB = 1, ReconROS_SRV = 2, ReconROS_CLT = 3 };


#endif