#define  _GNU_SOURCE

#define RECONOS_DEBUG


#define SETUP 2

#include "reconos.h"
#include "reconos_app.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "main.h"
#include "executor.h"

void init_msg(void)
{
	//std_msgs__msg__Header header;
  	rsobel_image_msg_out->height = IMAGE_HEIGHT;
  	rsobel_image_msg_out->width = IMAGE_WIDTH;
  	rsobel_image_msg_out->encoding.data = "bgr8";
	rsobel_image_msg_out->encoding.size = 4;  
	rsobel_image_msg_out->encoding.capacity = 5;  
  	rsobel_image_msg_out->is_bigendian = 0;
  	rsobel_image_msg_out->step = IMAGE_WIDTH*3;
  	rsobel_image_msg_out->data.data = malloc(IMAGE_HEIGHT*IMAGE_WIDTH*3);
	rsobel_image_msg_out->data.size = IMAGE_HEIGHT*IMAGE_WIDTH*3;
	rsobel_image_msg_out->data.capacity = IMAGE_HEIGHT*IMAGE_WIDTH*3;


	rperiodic_output_msg->data.data = malloc(32);
	rperiodic_output_msg->data.size = 32/4;
	rperiodic_output_msg->data.capacity = 32/4;

	for(int i = 0; i < 1555200; i++)
		rperiodic_srcmem->data[i] = i;
	
	rsort_sort_srv_res->sorted.data = malloc(2048*sizeof(uint32_t));
	rsort_sort_srv_res->sorted.size = 2048;
	rsort_sort_srv_res->sorted.capacity = 2048;

	return;
}

extern void *rt_sortdemo(void *data);
extern struct reconos_resource *resources_sortdemo[];
extern void *rt_inverse(void *data);
extern struct reconos_resource *resources_inverse[];
extern void *rt_mnist(void *data);
extern struct reconos_resource *resources_mnist[];
extern void *rt_sobel(void *data);
extern struct reconos_resource *resources_sobel[];
extern void *rt_periodic(void *data);
extern struct reconos_resource *resources_periodic[];

t_reconros_executor reconros_executor;

static void exit_signal(int sig) 
{
	ReconROS_Executor_Terminate(&reconros_executor);
	reconos_cleanup();
	printf("[recobop] aborted\n");
	//exit(0);
}

int main(int argc, char **argv) 
{
	reconos_init();
	reconos_app_init();

	init_msg();

	signal(SIGINT, exit_signal);
	signal(SIGTERM, exit_signal);
	signal(SIGABRT, exit_signal);

	printf("ReconROS init done \n");
	ReconROS_Executor_Init(&reconros_executor, 4, 2, "/mnt/bitstreams/");
	printf("ReconROS_Executor init done \n");
	
#if SETUP == 1

	ReconROS_Executor_Add_HW_Callback(&reconros_executor, "inverse", 	((1<<0)), 	ReconROS_SUB, rinverse_subdata, rinverse_input_msg, resources_inverse, 5);
	//ReconROS_Executor_Add_SW_Callback(&reconros_executor, "inverse", 	rt_inverse, 		ReconROS_SUB, rinverse_subdata, rinverse_input_msg, resources_inverse, 5);
	printf("ReconROS Callback added\n");
	ReconROS_Executor_Add_HW_Callback(&reconros_executor, "periodic", 	((1<<2)), 	ReconROS_TMR, rperiodic_periodic_tmr, 0, resources_periodic, 5);
	//ReconROS_Executor_Add_SW_Callback(&reconros_executor, "periodic", 	rt_periodic, 		ReconROS_TMR, rperiodic_periodic_tmr, 0, resources_periodic, 4);
	printf("ReconROS Callback added\n");
	
	ReconROS_Executor_Add_HW_Callback(&reconros_executor, "sortdemo", 	((1<<1)), 	ReconROS_SRV, rsort_srv, rsort_sort_srv_req, resources_sortdemo, 4);
	//ReconROS_Executor_Add_SW_Callback(&reconros_executor, "sortdemo", 	rt_sortdemo,		ReconROS_SRV, rsort_srv, rsort_sort_srv_req, resources_sortdemo, 4);
	printf("ReconROS Callback added\n");
	
	ReconROS_Executor_Add_HW_Callback(&reconros_executor, "mnist", 		((1<<3) ), 	ReconROS_SUB, rmnist_subdata, rmnist_image_msg, resources_mnist, 5);
	//ReconROS_Executor_Add_SW_Callback(&reconros_executor, "mnist", 		rt_mnist, 			ReconROS_SUB, rmnist_subdata, rmnist_image_msg, resources_mnist, 5);
	printf("ReconROS Callback added\n");

	ReconROS_Executor_Add_HW_Callback(&reconros_executor, "sobel", 		((1<<3)), 	ReconROS_SUB, rsobel_subdata, rsobel_image_msg, resources_sobel, 5);
	//ReconROS_Executor_Add_SW_Callback(&reconros_executor, "sobel", 		rt_sobel, 			ReconROS_SUB, rsobel_subdata, rsobel_image_msg, resources_sobel, 5);
	printf("ReconROS Callback added\n");

#elif SETUP == 2

	ReconROS_Executor_Add_HW_Callback(&reconros_executor, "inverse", 	((1<<0)), 	ReconROS_SUB, rinverse_subdata, rinverse_input_msg, resources_inverse, 5);
	//ReconROS_Executor_Add_SW_Callback(&reconros_executor, "inverse", 	rt_inverse, 		ReconROS_SUB, rinverse_subdata, rinverse_input_msg, resources_inverse, 5);
	printf("ReconROS Callback added\n");
	ReconROS_Executor_Add_HW_Callback(&reconros_executor, "periodic", 	((1<<3)), 			ReconROS_TMR, rperiodic_periodic_tmr, 0, resources_periodic, 5);
	//ReconROS_Executor_Add_SW_Callback(&reconros_executor, "periodic", 	rt_periodic, 		ReconROS_TMR, rperiodic_periodic_tmr, 0, resources_periodic, 4);
	printf("ReconROS Callback added\n");
	
	ReconROS_Executor_Add_HW_Callback(&reconros_executor, "sortdemo", 	((1<<1)), 	ReconROS_SRV, rsort_srv, rsort_sort_srv_req, resources_sortdemo, 4);
	//ReconROS_Executor_Add_SW_Callback(&reconros_executor, "sortdemo", 	rt_sortdemo,		ReconROS_SRV, rsort_srv, rsort_sort_srv_req, resources_sortdemo, 4);
	printf("ReconROS Callback added\n");
	
	ReconROS_Executor_Add_HW_Callback(&reconros_executor, "mnist", 		((1<<2) | (1<<3)), 	ReconROS_SUB, rmnist_subdata, rmnist_image_msg, resources_mnist, 5);
	//ReconROS_Executor_Add_SW_Callback(&reconros_executor, "mnist", 		rt_mnist, 			ReconROS_SUB, rmnist_subdata, rmnist_image_msg, resources_mnist, 5);
	printf("ReconROS Callback added\n");

	ReconROS_Executor_Add_HW_Callback(&reconros_executor, "sobel", 		((1<<2) | (1<<3)), 	ReconROS_SUB, rsobel_subdata, rsobel_image_msg, resources_sobel, 5);
	//ReconROS_Executor_Add_SW_Callback(&reconros_executor, "sobel", 		rt_sobel, 			ReconROS_SUB, rsobel_subdata, rsobel_image_msg, resources_sobel, 5);
	printf("ReconROS Callback added\n");

#endif

	ReconROS_Executor_Spin(&reconros_executor);
	
	reconos_app_cleanup();
	reconos_cleanup();

	return 0;
}


