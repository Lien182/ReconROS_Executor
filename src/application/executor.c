#include "executor.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int ReconROS_Executor_Init(t_reconros_executor * reconros_executor, uint32_t nSlots, uint32_t nSwThreads, char * bitstream_dir)
{
    reconros_executor->nExecutorsHw = nSlots;
    reconros_executor->nExecutorsSw = nSwThreads;

    reconros_executor->pBitstreamDir = malloc(strlen(bitstream_dir));
    if(reconros_executor->pBitstreamDir == 0)
        return -1;

    strcpy(reconros_executor->pBitstreamDir, bitstream_dir);

    if(reconros_executor->nExecutorsHw > 0)
    {
        Zycap_Init(&reconros_executor->Zycap);

        reconros_executor->pExecutorsHw = malloc(sizeof(t_reconros_hwexecutor) * nSlots);
        printf("debug: %x \n", reconros_executor->pExecutorsHw);
        if(reconros_executor->pExecutorsHw == 0)
        {
            printf("[ReconROS_Executor_Init] ERROR: failed to allocate memory for pExecutorsHw\n");
            return -1;
        }

        for(int i = 0; i < reconros_executor->nExecutorsHw; i++)
        {
            if(ReconROS_HWExecutor_Init(&(reconros_executor->pExecutorsHw[i]), &reconros_executor->Zycap, &reconros_executor->CallbackLists, i) != 0)
            {
                printf("[ReconROS_Executor_Init] ReconROS_HWExecutor_Init (%d) failed \n", i);
                return -1;
            }
        }
    }


    if(reconros_executor->nExecutorsSw > 0)
    {
        reconros_executor->pExecutorsSw = malloc(sizeof(t_reconros_swexecutor) * nSwThreads);
        if(reconros_executor->pExecutorsSw == 0)
        {
            printf("[ReconROS_Executor_Init] ERROR: failed to allocate memory for pExecutorsSw\n");
            return -1;
        }

        for(int i = 0; i < reconros_executor->nExecutorsSw; i++)
        {
            if(ReconROS_SWExecutor_Init(&(reconros_executor->pExecutorsSw)[i], &reconros_executor->CallbackLists, i) != 0)
            {
                printf("[ReconROS_Executor_Init] ReconROS_SWExecutor_Init (%d) failed \n", i);
                return -1;
            }
        }
    }


    if(Callbacklist_Init(&reconros_executor->CallbackLists) != 0)
        return -1;

    return 0;
}


int ReconROS_Executor_Spin(t_reconros_executor * reconros_executor)
{
    for(int i = 0; i < reconros_executor->nExecutorsHw; i++)
    {
        ReconROS_HWExecutor_Spin(&(reconros_executor->pExecutorsHw[i]));
    }

    for(int i = 0; i < reconros_executor->nExecutorsSw; i++)
    {
        ReconROS_SWExecutor_Spin(&(reconros_executor->pExecutorsSw[i]));
    }

    ReconROS_Executor_Join(reconros_executor);

    return 0;

}


int ReconROS_Executor_Join(t_reconros_executor * reconros_executor)
{
    for(int i = 0; i < reconros_executor->nExecutorsHw; i++)
    {
        //ReconROS_HWExecutor_Join(&(reconros_executor->pExecutorsHw)[i]);
    }

    for(int i = 0; i < reconros_executor->nExecutorsSw; i++)
    {
        ReconROS_SWExecutor_Join(&(reconros_executor->pExecutorsSw)[i]);
    }

    return 0;
}


int ReconROS_Executor_Add_HW_Callback(t_reconros_executor * reconros_executor, char * CallbackName, uint32_t nSlotMask, enum ReconROS_primitive primitive, void * object, void * object_result, void * resources, int resource_cnt )
{
    t_callback_list_element * pCallback = 0;

    if(primitive == ReconROS_TMR)
    {
        pCallback = &(reconros_executor->CallbackLists.alRosTmr[reconros_executor->CallbackLists.alRosTmrCnt]);
        reconros_executor->CallbackLists.alRosTmrCnt++;
    }
    else if(primitive == ReconROS_SUB)
    {
        pCallback = &(reconros_executor->CallbackLists.alRosSub[reconros_executor->CallbackLists.alRosSubCnt]);
        reconros_executor->CallbackLists.alRosSubCnt++;
    }
    else if(primitive == ReconROS_SRV)
    {
        pCallback = &(reconros_executor->CallbackLists.alRosSrv[reconros_executor->CallbackLists.alRosSrvCnt]);
        reconros_executor->CallbackLists.alRosSrvCnt++;
    }
    else if(primitive == ReconROS_CLT)
    {
        pCallback = &(reconros_executor->CallbackLists.alRosClt[reconros_executor->CallbackLists.alRosCltCnt]);
        reconros_executor->CallbackLists.alRosCltCnt++;
    }

    if(pCallback == 0)
    {
        printf("[ReconROS_Executor_Add_HW_Callback] ERROR: pCallback not found\n");
        return -1;
    }
        

    int nSlotCnt = 0;
    int *  slots = malloc(reconros_executor->nExecutorsHw);

    for(int i = 0; i < reconros_executor->nExecutorsHw; i++)
    {
        if(nSlotMask & (1<<i))
        {
            slots[nSlotCnt] = i;
            nSlotCnt++;
        }
    }

    pCallback->bitstreams = malloc(sizeof(t_bitstream) * nSlotCnt);
    if(!pCallback->bitstreams)
    {
        printf("[ReconROS_Executor_Add_HW_Callback] ERROR: !pCallback->bitstreams not allocated\n");    
        return -1;   
    }

    for(int i = 0; i < nSlotCnt; i++)
    {
        pCallback->bitstreams[i].size = 0;
        pCallback->bitstreams[i].data = 0;
    }
        

    char buf[255];

    for(int i = 0; i < nSlotCnt; i++)
    {
        sprintf(buf, "%s/pblock_slot_%d_%s_%d_partial.bit", reconros_executor->pBitstreamDir, slots[i], CallbackName, slots[i]);
        printf("%s \n", buf);
        Zycap_Prefetch_Bitstream(buf, &pCallback->bitstreams[i]);
    }

    pCallback->pReconROSPrimitive = object;
    pCallback->eReconROSPrimitive = primitive;
    pCallback->pReconROSResultPrimitive = object_result;
    pCallback->nSlotMask = nSlotMask;
    pCallback->sCallbackName = CallbackName;

    pCallback->callback_id = reconros_executor->nCallbackIdCnt;
    reconros_executor->nCallbackIdCnt++;
    pthread_mutex_init(&pCallback->object_lock, 0);

    pCallback->pHWthread = (struct reconos_thread *)malloc(sizeof(struct reconos_thread));
	if (!pCallback->pHWthread) {
		printf("[ReconROS_Executor_Add_HW_Callback] ERROR: failed to allocate memory for thread\n");
        return -1;
	}
   
	reconos_thread_init(pCallback->pHWthread, CallbackName, 0);
	reconos_thread_setinitdata(pCallback->pHWthread, 0); //later set: object ptr
	reconos_thread_setallowedslots(pCallback->pHWthread, slots, nSlotCnt);
	reconos_thread_setresourcepointers(pCallback->pHWthread, resources, resource_cnt);
	//reconos_thread_create_auto(pCallback->pHWthread, RECONOS_THREAD_HW);

    
    return 1;
}


int ReconROS_Executor_Add_SW_Callback(t_reconros_executor * reconros_executor, char * CallbackName, function_ptr pCallbackFunction, enum ReconROS_primitive primitive, void * object, void * object_result, void * resources, int resource_cnt )
{
    t_callback_list_element * pCallback = 0;

    if(primitive == ReconROS_TMR)
    {
        pCallback = &(reconros_executor->CallbackLists.alRosTmr[reconros_executor->CallbackLists.alRosTmrCnt]);
        reconros_executor->CallbackLists.alRosTmrCnt++;
    }
    else if(primitive == ReconROS_SUB)
    {
        pCallback = &(reconros_executor->CallbackLists.alRosSub[reconros_executor->CallbackLists.alRosSubCnt]);
        reconros_executor->CallbackLists.alRosSubCnt++;
    }
    else if(primitive == ReconROS_SRV)
    {
        pCallback = &(reconros_executor->CallbackLists.alRosSrv[reconros_executor->CallbackLists.alRosSrvCnt]);
        reconros_executor->CallbackLists.alRosSrvCnt++;
    }
    else if(primitive == ReconROS_CLT)
    {
        pCallback = &(reconros_executor->CallbackLists.alRosClt[reconros_executor->CallbackLists.alRosCltCnt]);
        reconros_executor->CallbackLists.alRosCltCnt++;
    }

    if(pCallback == 0)
    {
        printf("[ReconROS_Executor_Add_SW_Callback] Could not find an empty entry in the callback list \n");
        return -1;
    }


    pCallback->pReconROSPrimitive = object;
    pCallback->eReconROSPrimitive = primitive;
    pCallback->pReconROSResultPrimitive = object_result;
    pCallback->pSWCallback = pCallbackFunction;
    pCallback->sCallbackName = CallbackName;
    pCallback->nSlotMask = 0;

    pCallback->callback_id = reconros_executor->nCallbackIdCnt;
    reconros_executor->nCallbackIdCnt++;
    
    pthread_mutex_init(&pCallback->object_lock, 0);

    return 0;
}


int ReconROS_Executor_Terminate(t_reconros_executor * reconros_executor)
{
    for(int i = 0; i < reconros_executor->nExecutorsHw; i++)
    {
        ReconROS_HWExecutor_Terminate(&(reconros_executor->pExecutorsHw)[i]);
    }

    for(int i = 0; i < reconros_executor->nExecutorsSw; i++)
    {
        ReconROS_SWExecutor_Terminate(&(reconros_executor->pExecutorsSw)[i]);
    }

//    ReconROS_Executor_Join(reconros_executor);

    return 0;

}



