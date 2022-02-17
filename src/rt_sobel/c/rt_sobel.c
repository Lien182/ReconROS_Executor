/********************************************************************          
* rt_sobel.c           -- software sobel implementation             *
*                         calculated the sobel operation for        *
*                         all three color channels                  * 
*																	* 
* Author(s):  Christian Lienen                                      *   
*                                                                   *   
********************************************************************/

#include "reconos.h"
#include "reconos_thread.h"
#include "reconos_calls.h"
#include "utils.h"

#include <math.h>
#include <stdio.h>

#include "../../application/main.h"

#define TIME_MEASUREMENT 0


#define INPUT_WIDTH IMAGE_WIDTH/4*3 // because of the rgb8 format -> 24 bit per pixel
#define INPUT_HEIGHT IMAGE_HEIGHT
#define INPUT_LINEBUFFER_SIZE (INPUT_WIDTH * 4 * 4)
#define INPUT_PREFETCH_SIZE	  (INPUT_WIDTH * 4 * 2)
#define INPUT_LINESIZE (INPUT_WIDTH * 4)
#define OUTPUT_LINEBUFFER_SIZE (INPUT_WIDTH * 4)
#define OUTPUT_WIDTH INPUT_WIDTH
#define OUTPUT_LINE_SIZE (OUTPUT_WIDTH * 4 )

#define MEM_READ_L( adr, dest, length ) { memcpy( dest, (void*)adr,  length); }
#define MEM_WRITE_L( src, adr, length ) { memcpy((void*)adr, src, length); }


const int filter_x[] = { 1,  2,  1,  0,  0,  0, -1, -2, -1};
const int filter_y[] = { 1,  0, -1,  2,  0, -2,  1,  0, -1};

THREAD_ENTRY()
{
	clock_t start, end;
	uint32_t input_linebuffer[INPUT_LINEBUFFER_SIZE/4];
	uint32_t output_linebuffer[OUTPUT_LINEBUFFER_SIZE/4];
	int32_t i,k,j, ii, jj;
	int16_t tmp_x[4], tmp_y[4];
	uint8_t filter_pointer;

	THREAD_INIT();

#if TIME_MEASUREMENT == 1	
	start = clock();
#endif

	uint32_t address = (uint32_t)rsobel_image_msg->data.data;
	
	MEM_READ_L( address, input_linebuffer, INPUT_PREFETCH_SIZE);
	address += (INPUT_WIDTH<<3);

	for(i = 1; i < (INPUT_HEIGHT-1); i++)
	{
		MEM_READ_L( address , &(input_linebuffer[INPUT_WIDTH* ((i+1)&3)]) , INPUT_LINESIZE );
		address += (INPUT_WIDTH<<2);

		for(j = 1; j < (INPUT_WIDTH-1); j++)
		{
			tmp_x[0]= 0; tmp_y[0] = 0;
			tmp_x[1]= 0; tmp_y[1] = 0;
			tmp_x[2]= 0; tmp_y[2] = 0;
			tmp_x[3]= 0; tmp_y[3] = 0;

			filter_pointer = 0;
			for(ii=-1; ii < 2; ii++)
			{
				for(jj=-1; jj < 2; jj++)
				{		
					uint32_t buffer_pointer = ((INPUT_WIDTH*((i+ii)&3)+(j+jj)));
					uint32_t actindata  = 	input_linebuffer[buffer_pointer];	
					for(k = 0; k < 4; k++)
					{
						int16_t data = ((actindata >> 8*k) & 0x000000ff);
						tmp_x[k] += data * filter_x[filter_pointer];
						tmp_y[k] += data * filter_y[filter_pointer];
						
					}
					filter_pointer++;
				}	
			}
			output_linebuffer[(j)] = (((abs(tmp_x[0]) + abs(tmp_y[0])) >> 3)) | (((abs(tmp_x[1]) + abs(tmp_y[1])) >> 3) << 8) | (((abs(tmp_x[2]) + abs(tmp_y[2])) >> 3) << 16) | (((abs(tmp_x[3]) + abs(tmp_y[3])) >> 3) << 24);
		}
		
		//uint32_t sum = 0;
		//for(int i = 0; i < INPUT_LINESIZE/4; i++)
		//sum+= ((uint8_t*)outputdata + i* OUTPUT_LINE_SIZE)[i];

		//printf("sum : %d \n", sum);

		MEM_WRITE_L( output_linebuffer , ((uint8_t*)rsobel_image_msg_out->data.data + i* OUTPUT_LINE_SIZE), INPUT_LINESIZE);
		
	}
#if TIME_MEASUREMENT == 1	
	end = clock();
#endif
	ROS_PUBLISH(rsobel_pubdata, rsobel_image_msg_out);
#if TIME_MEASUREMENT == 1
	printf("%3.6f\n", (double)(end-start)/CLOCKS_PER_SEC);
#endif
}
