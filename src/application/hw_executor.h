#ifndef HWEXECUTOR_H
#define HWEXECUTOR_H

#include <stdint.h>
#include <pthread.h>
#include "zycap_linux.h"

#include "callbacklist.h"

#include "reconos.h"


typedef struct
{
    pthread_t                   ptAgent;
    t_zycap *                   Zycap;
    uint32_t                    uSlotid;
    uint32_t                    uSlotMask;
    uint32_t                    bRun;

    t_callback_lists    *       pCallbacklists;

}t_reconros_hwexecutor;


int ReconROS_HWExecutor_Init(t_reconros_hwexecutor * reconros_hwexecutor, t_zycap * zycap, t_callback_lists * pCallbacklists, uint32_t nSlotId);

int ReconROS_HWExecutor_Spin(t_reconros_hwexecutor * reconros_hwexecutor);

int ReconROS_HWExecutor_Join(t_reconros_hwexecutor * reconros_hwexecutor);

int ReconROS_HWExecutor_Terminate(t_reconros_hwexecutor * reconros_hwexecutor);
#endif