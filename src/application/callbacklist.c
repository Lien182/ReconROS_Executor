#include "callbacklist.h"

#include "ros_timer.h"
#include "ros_sub.h"
#include "ros_service_server.h"
#include "ros_service_client.h"


int Callbacklist_Init(t_callback_lists * callback_lists)
{
    callback_lists->alRosTmrCnt = 0;
    callback_lists->alRosSubCnt = 0;
    callback_lists->alRosSrvCnt = 0;
    callback_lists->alRosCltCnt = 0;

    //Object lists
    callback_lists->alRosTmr = malloc(sizeof(t_callback_list_element) * RECONROS_EXECUTOR_MAX_NR_OF_OBJECTS);
    callback_lists->alRosSub = malloc(sizeof(t_callback_list_element) * RECONROS_EXECUTOR_MAX_NR_OF_OBJECTS);
    callback_lists->alRosSrv = malloc(sizeof(t_callback_list_element) * RECONROS_EXECUTOR_MAX_NR_OF_OBJECTS);
    callback_lists->alRosClt = malloc(sizeof(t_callback_list_element) * RECONROS_EXECUTOR_MAX_NR_OF_OBJECTS);

    if((callback_lists->alRosTmr == 0) || (callback_lists->alRosSub == 0) || (callback_lists->alRosSrv == 0) || (callback_lists->alRosClt == 0))
        return -1;

    return 0;
}

int Callbacklist_GetNextTimer(t_callback_lists * callback_lists, uint32_t offset, t_callback_list_element ** element, uint32_t u32SlotMask) // if slotmask is 0, return sw
{
    //printf("[Callbacklist_GetNextTimer] debug -1, %x \n", callback_lists);
    for(int i = offset; i < callback_lists->alRosTmrCnt; i++)
    {
        //printf("[Callbacklist_GetNextSubscriber] debug 0 \n");
        if((u32SlotMask == 0 && callback_lists->alRosTmr[i].pSWCallback != 0 ) || (callback_lists->alRosTmr[i].nSlotMask & u32SlotMask))
        {
            //printf("[Callbacklist_GetNextSubscriber] debug 1 \n");
            if(pthread_mutex_trylock(&(callback_lists->alRosTmr[i].object_lock)) == 0) // check if resource is free
            {
                //printf("[Callbacklist_GetNextSubscriber] debug 2 \n");
                if(ros_timer_is_ready((struct ros_timer_t*)(callback_lists->alRosTmr[i].pReconROSPrimitive), (uint32_t*)&callback_lists->alRosTmr[i].pReconROSResultPrimitive) == 0) // if no 
                {
                    //printf("[Callbacklist_GetNextSubscriber] debug 3 \n");
                    //we found a subscriber with a new message;
                    *element = &callback_lists->alRosTmr[i];
                    return i+1;
                }
                else
                {
                    //printf("[Callbacklist_GetNextSubscriber] debug 4 \n");
                    pthread_mutex_unlock(&(callback_lists->alRosTmr[i].object_lock));            
                }

            }
            else 
            {
                continue;
            }
        }
       

    }

    return -1;
}

int Callbacklist_ReleaseTimer(t_callback_lists * callback_lists, t_callback_list_element * element)
{
    pthread_mutex_unlock(&element->object_lock);   
    return 1;
}

int Callbacklist_GetNextSubscriber(t_callback_lists * callback_lists, uint32_t offset, t_callback_list_element ** element, uint32_t u32SlotMask) // if slotmask is 0, return sw
{
    //printf("[Callbacklist_GetNextSubscriber] debug -1, %x \n", callback_lists);
    for(int i = offset; i < callback_lists->alRosSubCnt; i++)
    {
        //printf("[Callbacklist_GetNextSubscriber] debug 0 \n");
        if((u32SlotMask == 0 && callback_lists->alRosSub[i].pSWCallback != 0 ) || (callback_lists->alRosSub[i].nSlotMask & u32SlotMask))
        {
            //printf("[Callbacklist_GetNextSubscriber] debug 1 \n");
            if(pthread_mutex_trylock(&(callback_lists->alRosSub[i].object_lock)) == 0) // check if resource is free
            {
                if(ros_subscriber_message_try_take((struct ros_subscriber_t*)(callback_lists->alRosSub[i].pReconROSPrimitive), callback_lists->alRosSub[i].pReconROSResultPrimitive ) == 0) // if no 
                {
                    //printf("[Callbacklist_GetNextSubscriber] debug 2 %s\n", callback_lists->alRosSub[i].sCallbackName);
                    //printf("[Callbacklist_GetNextSubscriber] debug 3 \n");
                    //we found a subscriber with a new message;
                    *element = &callback_lists->alRosSub[i];
                    return i+1;
                }
                else
                {
                    //printf("[Callbacklist_GetNextSubscriber] debug 4 \n");
                    pthread_mutex_unlock(&(callback_lists->alRosSub[i].object_lock));            
                }

            }
            else 
            {
                continue;
            }
        }
       

    }
    //printf("[Callbacklist_GetNextSubscriber] debug 5 \n");
    return -1;
}

int Callbacklist_ReleaseSubscriber(t_callback_lists * callback_lists, t_callback_list_element * element)
{
    pthread_mutex_unlock(&element->object_lock);   
    return 1;
}

int Callbacklist_GetNextService(t_callback_lists * callback_lists, uint32_t offset, t_callback_list_element ** element, uint32_t u32SlotMask) // if slotmask is 0, return sw
{
    for(int i = offset; i < callback_lists->alRosSrvCnt; i++)
    {
        //printf("[Callbacklist_GetNextService] debug 1 \n");
        if((u32SlotMask == 0 && callback_lists->alRosSrv[i].pSWCallback != 0  ) || (callback_lists->alRosSrv[i].nSlotMask & u32SlotMask))
        {
            //printf("[Callbacklist_GetNextService] debug 2 \n");
            if(pthread_mutex_trylock(&(callback_lists->alRosSrv[i].object_lock)) == 0) // check if resource is free
            {
                //printf("[Callbacklist_GetNextService] debug 3 \n");
                if(ros_service_server_request_try_take((struct ros_service_server_t*)(callback_lists->alRosSrv[i].pReconROSPrimitive), callback_lists->alRosSrv[i].pReconROSResultPrimitive ) == 0) // if no 
                {
                    //printf("[Callbacklist_GetNextService] debug 4 \n");
                    //we found a subscriber with a new message;
                    *element = &callback_lists->alRosSrv[i];
                    return i+1;
                }
                else
                {
                    //printf("[Callbacklist_GetNextService] debug 5 \n");
                    pthread_mutex_unlock(&(callback_lists->alRosSrv[i].object_lock));            
                }

            }
            else 
            {
                continue;
            }
        }
       

    }
    //printf("[Callbacklist_GetNextService] debug 6 \n");
    return -1;
}

int Callbacklist_ReleaseService(t_callback_lists * callback_lists, t_callback_list_element * element)
{
    pthread_mutex_unlock(&element->object_lock);   
    return 1;
}

int Callbacklist_GetNextClient(t_callback_lists * callback_lists, uint32_t offset, t_callback_list_element ** element, uint32_t u32SlotMask) // if slotmask is 0, return sw
{
    for(int i = offset; i < callback_lists->alRosCltCnt; i++)
    {
        //printf("[Callbacklist_GetNextService] debug 1 \n");
        if((u32SlotMask == 0 && callback_lists->alRosClt[i].pSWCallback != 0  ) || (callback_lists->alRosClt[i].nSlotMask & u32SlotMask))
        {
            //printf("[Callbacklist_GetNextService] debug 2 \n");
            if(pthread_mutex_trylock(&(callback_lists->alRosClt[i].object_lock)) == 0) // check if resource is free
            {
                //printf("[Callbacklist_GetNextService] debug 3 \n");
                if(ros_service_client_response_try_take((struct ros_service_client_t*)(callback_lists->alRosClt[i].pReconROSPrimitive), callback_lists->alRosClt[i].pReconROSResultPrimitive ) == 0) // if no 
                {
                    //printf("[Callbacklist_GetNextService] debug 4 \n");
                    //we found a subscriber with a new message;
                    *element = &callback_lists->alRosClt[i];
                    return i+1;
                }
                else
                {
                    //printf("[Callbacklist_GetNextService] debug 5 \n");
                    pthread_mutex_unlock(&(callback_lists->alRosClt[i].object_lock));            
                }

            }
            else 
            {
                continue;
            }
        }
       

    }
    //printf("[Callbacklist_GetNextService] debug 6 \n");
    return -1;
}

int Callbacklist_ReleaseClient(t_callback_lists * callback_lists, t_callback_list_element * element)
{
    pthread_mutex_unlock(&element->object_lock);   
    return 1;
}


t_callback_list_element * Callbacklist_GetCallback (t_callback_lists * callbacklists, uint32_t *nCallbackRetention, uint32_t nSlotMask)
{
    t_callback_list_element * element = NULL;
    int ret = 0;
   
    ret = Callbacklist_GetNextTimer(callbacklists, 0, &element, nSlotMask);
    if(ret >= 0)
    {
        return element;
    }
    else 
    {   
        if((*nCallbackRetention & 0xC0000000) == 0x40000000 )
        {
            ret = Callbacklist_GetNextSubscriber(callbacklists, *nCallbackRetention & 0x3fffffff, &element, nSlotMask);
            if(ret >= 0)
            {
                *nCallbackRetention = 0x40000000 | ret;
                return element;
            }
            else
            {
                *nCallbackRetention = 0x80000000;
            }            
        }

        if((*nCallbackRetention & 0xC0000000) == 0x80000000 )
        {
            ret = Callbacklist_GetNextService(callbacklists, *nCallbackRetention & 0x3fffffff, &element, nSlotMask);
            if(ret >= 0)
            {
                *nCallbackRetention = 0x80000000 | ret;
                return element;
            }
            else
            {
                *nCallbackRetention = 0xC0000000;
            }            
        }

        if((*nCallbackRetention & 0xC0000000) == 0xC0000000 )
        {
            ret = Callbacklist_GetNextClient(callbacklists, *nCallbackRetention & 0x3fffffff, &element, nSlotMask);
            if(ret >= 0)
            {
                *nCallbackRetention = 0xC0000000 | ret;
                return element;
            }
            else
            {
                *nCallbackRetention = 0x40000000;
            }            
        }
    }

    return NULL;
}


int Callbacklist_GetSWCallback(t_callback_lists * callbacklists, uint32_t *nCallbackRetention, function_ptr * pCallback, void ** ppMessage)
{
    t_callback_list_element * element = NULL;
    int ret = 0;
    //printf("[Callbacklist_GetSWCallback] check for new events \n");


    element = Callbacklist_GetCallback (callbacklists, nCallbackRetention, 0);
    if(element != NULL)
    {
        *pCallback = element->pSWCallback;
        *ppMessage = element->pReconROSResultPrimitive;
        return element->callback_id;
    }
    else
    {
        return -1;
    }

}



t_bitstream * Callbacklist_GetSlotBitstream(t_bitstream * bitstreams, uint32_t nBitstreamsSlotMask, uint32_t nThreadSlotMask)
{
    uint32_t tmpslotmask = 1UL;


    if(!(nBitstreamsSlotMask & nThreadSlotMask))
    {
        //printf("[Callbacklist_GetSlotBitstream] No valid bitstream available\n");
        return NULL;    //no valid bitstream in the list
    }

    do{
        if(tmpslotmask & nBitstreamsSlotMask) //bitstream is in the list
        {
            if(tmpslotmask & nThreadSlotMask)
            {
                //printf("[Callbacklist_GetSlotBitstream] found bitstream for slotmask %x \n", tmpslotmask);
                return bitstreams;
            }
            else
            {
                bitstreams++;
            }
        }

        tmpslotmask =  tmpslotmask << 1;
    }while(tmpslotmask > 0);

    //printf("[Callbacklist_GetSlotBitstream] No valid bitstream found\n");
    return NULL;
}

int Callbacklist_GetHWCallback(t_callback_lists * callbacklists, uint32_t nThreadSlotMask, uint32_t *nCallbackRetention, struct reconos_thread ** reconos_hw_thread,  t_bitstream ** pBitstream, void ** ppMessage)
{

    t_callback_list_element * element = NULL;

    element = Callbacklist_GetCallback (callbacklists, nCallbackRetention, nThreadSlotMask);
    if(element != NULL)
    {
        *pBitstream = Callbacklist_GetSlotBitstream(element->bitstreams, element->nSlotMask, nThreadSlotMask);
        *reconos_hw_thread = element->pHWthread;
        *ppMessage = element->pReconROSResultPrimitive;
        return element->callback_id;
    }
    else
    {
        return -1;
    }
}



int Callbacklist_Release(t_callback_lists * callbacklists, int id)
{
    //printf("[Callbacklist_Release] release element\n");

    for(int i = 0; i < callbacklists->alRosTmrCnt; i++)
    {
        if(callbacklists->alRosTmr[i].callback_id == id)
        {
            Callbacklist_ReleaseTimer(callbacklists, &callbacklists->alRosTmr[i]);
            return 1;
        }
    }

    for(int i = 0; i < callbacklists->alRosSubCnt; i++)
    {
        if(callbacklists->alRosSub[i].callback_id == id)
        {
            Callbacklist_ReleaseSubscriber(callbacklists, &callbacklists->alRosSub[i]);
            return 1;
        }
    }

    for(int i = 0; i < callbacklists->alRosSrvCnt; i++)
    {
        if(callbacklists->alRosSrv[i].callback_id == id)
        {
            Callbacklist_ReleaseService(callbacklists, &callbacklists->alRosSrv[i]);
            return 1;
        }
    }

    for(int i = 0; i < callbacklists->alRosCltCnt; i++)
    {
        if(callbacklists->alRosClt[i].callback_id == id)
        {
            Callbacklist_ReleaseClient(callbacklists, &callbacklists->alRosClt[i]);
            return 1;
        }
    }


    return 0;
}