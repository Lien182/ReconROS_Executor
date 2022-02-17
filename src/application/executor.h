#ifndef EXECUTOR_H
#define EXECUTOR_H

#include <stdint.h>
#include <pthread.h>
#include "zycap_linux.h"

#include "callbacklist.h"
#include "hw_executor.h"
#include "sw_executor.h"

typedef struct 
{
    uint32_t    callback_id;
    void *      message;
}t_readylistelement;

typedef struct
{   char *                      pBitstreamDir;
    t_reconros_hwexecutor *     pExecutorsHw;
    uint32_t                    nExecutorsHw;
    t_reconros_swexecutor *     pExecutorsSw;
    uint32_t                    nExecutorsSw;

    t_zycap                     Zycap;    

    uint32_t                    nCallbackIdCnt;

    t_callback_lists            CallbackLists;

}t_reconros_executor;



typedef struct 
{
    void *                      pReconROSPrimitive;
    enum ReconROS_primitive     eReconROSPrimitive;
    uint32_t                    uExecutionType;
    t_bitstream *               pBitstreams;
    uint32_t                    nSlotMask;
    void *                      pFunction;
    void *                      pArgs;

}t_callback;

int ReconROS_Executor_Init(t_reconros_executor * reconros_executor, uint32_t nSlots, uint32_t nSwThreads, char * bitstream_dir);

int ReconROS_Executor_Add_HW_Callback(t_reconros_executor * reconros_executor, char * CallbackName, uint32_t nSlotMask, enum ReconROS_primitive primitive, void * object, void * object_result, void * resources, int resource_cnt );

int ReconROS_Executor_Add_SW_Callback(t_reconros_executor * reconros_executor, char * CallbackName, function_ptr pCallback, enum ReconROS_primitive primitive, void * object, void * object_result, void * resources, int resource_cnt );

int ReconROS_Executor_Spin(t_reconros_executor * reconros_executor);

int ReconROS_Executor_Join(t_reconros_executor * reconros_executor);

int ReconROS_Executor_Terminate(t_reconros_executor * reconros_executor);




#endif