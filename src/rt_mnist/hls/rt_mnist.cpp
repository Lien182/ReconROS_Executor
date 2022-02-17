/********************************************************************          
* rt_sobel.cpp         -- hardware sobel implementation             *
*                         calculated the sobel operation for        *
*                         all three color channels                  * 
*																	* 
* Author(s):  Christian Lienen                                      *   
*                                                                   *   
********************************************************************/

#include "reconos_thread.h"
#include "reconos_calls.h"

#include "ap_int.h"
#include "ap_fixed.h"
#include <math.h>

#include "LeNet.h"

#include "parameters.h"

#include <sensor_msgs/msg/image.h>


#define INPUT_BATCH_SZ	1
#define INPUT_N_ROWS 28
#define INPUT_N_COLS 28


THREAD_ENTRY()
{
	ap_axis<HW_DATA_WIDTH,1,1,1> src[BUFFER_SIZE], dst[CLASSES];

	THREAD_INIT();
	
	uint32_t status = 0;
	uint32_t payload_addr[1];
	
	uint32_t ram[INPUT_BATCH_SZ*INPUT_N_ROWS*INPUT_N_COLS/4];

	uint32_t pMessage= GET_INIT_DATA();
	MEM_READ(OFFSETOF(sensor_msgs__msg__Image, data.data) + pMessage, payload_addr, 4);
	MEM_READ( payload_addr[0], ram, INPUT_N_ROWS*INPUT_N_COLS);

	//memcpy(image, ram,  N_ROWS*N_COLS);

	//read_image(ram, image);

	uint8_t *image = (uint8_t*)ram;

	for(int i=0; i<1; i++)
	{
		char tmp;
		for(int batch=0; batch<1; batch++)
		{
			for(int rows = 0; rows < 32 ; rows++)
			{
				for(int cols = 0; cols < 32; cols++)
				{
					#pragma HLS pipeline
					ap_fixed<32,16> scaled;
					uint8_t temp;

					if(cols<2 || rows<2 || cols>=30|| rows>=30)
					{
						temp = 0;
					}
					else
					{
						temp = image[batch*1024+(rows-2)*28+cols-2];
					}

					scaled =  ((((ap_fixed<32,16>)temp * (ap_fixed<32,16>)2) / (ap_fixed<32,16>)255 )- (ap_fixed<32,16>)1 );
					ap_int<HW_DATA_WIDTH> tmp;
					tmp = (ap_int<HW_DATA_WIDTH>)(scaled*DATA_CONVERT_MUL);
					src[rows*INPUT_WH+cols].data = tmp;

					src[i].keep = 1;
					src[i].strb = 1;
					src[i].user = 1;
					src[i].last = 0;
					src[i].id = 0;
					src[i].dest = 1;
				}
			}
		}
	}
	
	LeNet(src, dst, 0);

	ap_fixed<32,16> result[CLASSES];
	ap_fixed<32,16> max_num = -10000;
	int max_id = 0;
	for(int index=0; index<10; index++){
		#pragma HLS pipeline
		int tmp = dst[index].data;
		result[index] = (ap_fixed<32,16>)tmp/(ap_fixed<32,16>)DATA_CONVERT_MUL;
		if(result[index] > max_num){
			max_num = result[index];
			max_id = index;
		}
	}




	uint32_t output_digit[1];
	output_digit[0] = max_id;

	uint32_t output_buffer_addr =  MEMORY_GETOBJECTADDR(rmnist_output_msg);

	MEM_WRITE( output_digit, output_buffer_addr, 4);
	ROS_PUBLISH(rmnist_pubdata, rmnist_output_msg);			

	THREAD_EXIT();	
	
}
