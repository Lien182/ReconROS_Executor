#ifndef SWEXECUTOR_H
#define SWEXECUTOR_H

#include <stdint.h>
#include <pthread.h>

#include "callbacklist.h"

typedef struct
{
    uint32_t                bRun;
    pthread_t               ptAgent;
    int                     id;
    t_callback_lists  *     callbacklists;
}t_reconros_swexecutor;




int     ReconROS_SWExecutor_Init(t_reconros_swexecutor * reconros_swexecutor, t_callback_lists * callbacklists, int id);

int     ReconROS_SWExecutor_Spin(t_reconros_swexecutor * reconros_swexecutor);

int     ReconROS_SWExecutor_Join(t_reconros_swexecutor * reconros_swexecutor);

int     ReconROS_SWExecutor_Terminate(t_reconros_swexecutor * reconros_swexecutor);

#endif