#include "reconos_thread.h"
#include "reconos_calls.h"

#define BLOCK_SIZE 2048



void bubblesort(uint32_t *data, int data_count) {
	int i;
	uint32_t tmp;
	int s, n, newn;

	s = 1;
	n = data_count - 1;
	newn = n;

	while (s) {
		s = 0;
		for (i = 0; i < n; i++) {
			if (data[i] > data[i + 1]) {
				tmp = data[i];
				data[i] = data[i + 1];
				data[i + 1] = tmp;
				newn = i;
				s = 1;
			}
		}

		n = newn;
	}
}

void *rt_sortdemo(void *data) {
	
	sorter_msgs__srv__Sort_Request * req = (sorter_msgs__srv__Sort_Request*)data;

	bubblesort(req->unsorted.data, BLOCK_SIZE);

	rsort_sort_srv_res->sorted = req->unsorted;

	ROS_SERVICESERVER_SEND_RESPONSE(rsort_srv,rsort_sort_srv_res);

	return 0;
}
