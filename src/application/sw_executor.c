#include "time_measurement.h"
#include "sw_executor.h"
#include "executor.h"

#include <stdio.h>
#include <unistd.h>
#include <time.h>


static void * ReconROS_SWExecutor_Agent(void * args)
{

    t_reconros_swexecutor * reconros_swexecutor = ( t_reconros_swexecutor *)args;
    struct timespec t_start, t_end, t_res;
	clock_t start, end;
	uint32_t nCallbackRetention = 0x40000000;

	function_ptr pCallback;
	void * message;

	while(reconros_swexecutor->bRun)
	{
		//printf("[ReconROS_SWExecutor_Agent] Check the callback \n");

		int callbackid = Callbacklist_GetSWCallback(reconros_swexecutor->callbacklists, &nCallbackRetention, &pCallback, &message);
		if(callbackid < 0)
		{
			usleep(10000);
		}
		else
		{
			//printf("[ReconROS_SWExecutor_Agent] Going to execute the function \n");
			clock_gettime(CLOCK_MONOTONIC, &t_start);
			start = clock();
			pCallback(message);
			end = clock();
			clock_gettime(CLOCK_MONOTONIC, &t_end);
			timespec_diff(&t_start, &t_end, &t_res);
			printf("[ReconROS_SWExecutor_Agent %d] execution time: %3.6f (%3.6f) \n", reconros_swexecutor->id, (double)(t_res.tv_nsec)/1000000000, ((double) (end - start)) / CLOCKS_PER_SEC);
	
			Callbacklist_Release(reconros_swexecutor->callbacklists, callbackid);
		}

	}
	return 0;

}



int ReconROS_SWExecutor_Init(t_reconros_swexecutor * reconros_swexecutor, t_callback_lists * callbacklists, int id)
{
	reconros_swexecutor->id = id;
	reconros_swexecutor->callbacklists = callbacklists;

	return 0;
}

int ReconROS_SWExecutor_Spin(t_reconros_swexecutor * reconros_swexecutor)
{

	struct sched_param param;
	pthread_attr_t attr;
	int ret;

	/* Initialize pthread attributes (default values) */
	ret = pthread_attr_init(&attr);
	if (ret) {
			printf("init pthread attributes failed\n");
			return -1;
	}

	/* Set scheduler policy and priority of pthread */
	ret = pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
	if (ret) {
			printf("pthread setschedpolicy failed\n");
			return -1;
	}
	param.sched_priority = 75;
	ret = pthread_attr_setschedparam(&attr, &param);
	if (ret) 
	{
		printf("pthread setschedparam failed\n");
		return -1;
	}
	/* Use scheduling parameters of attr */
	ret = pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
	if (ret) {
		printf("pthread setinheritsched failed\n");
			return -1;
	}

	reconros_swexecutor->bRun = 1UL;

	if (pthread_create(&reconros_swexecutor->ptAgent, 0, ReconROS_SWExecutor_Agent, (void*)reconros_swexecutor) != 0) 
	{
		return -1;
    }

	return 0;
}

int ReconROS_SWExecutor_Join(t_reconros_swexecutor * reconros_swexecutor)
{
	uint32_t ret_value;
	pthread_join(reconros_swexecutor->ptAgent, (void**)&ret_value);
	return 0;
}


int ReconROS_SWExecutor_Terminate(t_reconros_swexecutor * reconros_swexecutor)
{
	reconros_swexecutor->bRun = 0UL;

	return 0;
}