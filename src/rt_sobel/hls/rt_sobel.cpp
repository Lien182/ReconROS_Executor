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

#include <sensor_msgs/msg/image.h>

#include "ap_int.h"
#include "ap_fixed.h"
#include "hls_math.h"

#define INPUT_WIDTH 480 //Because of the brg format -< 24 bit per pixel
#define INPUT_HEIGHT 480
#define INPUT_LINEBUFFER_SIZE (INPUT_WIDTH * 4 * 4)
#define INPUT_PREFETCH_SIZE	  (INPUT_WIDTH * 4 * 2)
#define INPUT_LINESIZE (INPUT_WIDTH * 4)
#define OUTPUT_LINEBUFFER_SIZE (INPUT_WIDTH * 4)
#define OUTPUT_WIDTH INPUT_WIDTH
#define OUTPUT_LINE_SIZE (OUTPUT_WIDTH * 4 )

const int filter_x[] = { 1,  2,  1,  0,  0,  0, -1, -2, -1};
const int filter_y[] = { 1,  0, -1,  2,  0, -2,  1,  0, -1};

//const int filter_x[] = { 0,  0,  0,  0,  4,  0,  0,  0,  0};
//const int filter_y[] = { 0,  0,  0,  0,  4,  0,  0,  0,  0};

THREAD_ENTRY()
{

	int32_t input_linebuffer[INPUT_LINEBUFFER_SIZE];
	int32_t output_linebuffer[OUTPUT_LINEBUFFER_SIZE];;
	
	#pragma HLS array_partition variable=input_linebuffer cyclic factor=9 dim=0
	#pragma HLS array_partition variable=output_linebuffer cyclic factor=16 dim=0

	int32_t i,k,j, ii, jj;
	int16_t tmp_x[4], tmp_y[4];
	uint8_t filter_pointer;
	uint32_t status = 0;
	uint32_t input_payload_addr[1];
	uint32_t output_payload_addr[1];

	THREAD_INIT();

	uint32_t pMessage = GET_INIT_DATA();	
	MEM_READ(OFFSETOF(sensor_msgs__msg__Image, data.data) + pMessage,			input_payload_addr,		4);	
	uint32_t output_buffer_addr = MEMORY_GETOBJECTADDR(rsobel_image_msg_out);
	MEM_READ(OFFSETOF(sensor_msgs__msg__Image, data.data) + output_buffer_addr, output_payload_addr, 	4);

	//address <<=2;
	MEM_READ( input_payload_addr[0], input_linebuffer, INPUT_PREFETCH_SIZE);

	input_payload_addr[0] += (INPUT_WIDTH<<3); // <<3 = *2*4

	for(i = 1; i < (INPUT_HEIGHT-1); i++)
	{
		#pragma hls dataflow
		
		if(i > 1)
			MEM_WRITE( output_linebuffer , (output_payload_addr[0] + (i-1)*OUTPUT_LINE_SIZE), INPUT_LINESIZE );
		
		MEM_READ( input_payload_addr[0] , &(input_linebuffer[INPUT_WIDTH* ((i+1)&3)]) , INPUT_LINESIZE );
		input_payload_addr[0] += (INPUT_WIDTH<<2);
		
		for(j = 1; j < (INPUT_WIDTH-1); j++)
		{
			#pragma HLS pipeline
			#pragma HLS unroll factor=4
			tmp_x[0]= 0; tmp_y[0] = 0;
			tmp_x[1]= 0; tmp_y[1] = 0;
			tmp_x[2]= 0; tmp_y[2] = 0;
			tmp_x[3]= 0; tmp_y[3] = 0;

			filter_pointer = 0;
			for(ii=-1; ii < 2; ii++)
			{	
				#pragma HLS unroll factor=3
				for(jj=-1; jj < 2; jj++)  
				{	
					#pragma HLS unroll factor=3	
					uint32_t buffer_pointer = ((INPUT_WIDTH*((i+ii)&3)+(j+jj)));
					uint32_t actindata  = 	input_linebuffer[buffer_pointer];	
					for(k = 0; k < 4; k++)
					{
						#pragma HLS unroll factor=4
						int16_t data = ((actindata >> 8*k) & 0x000000ff);
						tmp_x[k] += data * filter_x[filter_pointer];
						tmp_y[k] += data * filter_y[filter_pointer];
						
					}
					filter_pointer++;
				}	
			}
			output_linebuffer[(j)] = (((abs(tmp_x[0]) + abs(tmp_y[0])) >> 3)) | (((abs(tmp_x[1]) + abs(tmp_y[1])) >> 3) << 8) | (((abs(tmp_x[2]) + abs(tmp_y[2])) >> 3) << 16) | (((abs(tmp_x[3]) + abs(tmp_y[3])) >> 3) << 24);
		}
		
		

	}
	ROS_PUBLISH(rsobel_pubdata, rsobel_image_msg_out);

	THREAD_EXIT();	
	

}