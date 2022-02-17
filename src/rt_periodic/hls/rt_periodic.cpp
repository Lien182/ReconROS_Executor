#include "reconos_thread.h"
#include "reconos_calls.h"

#include <stdlib.h>
#include <memory.h>
#include <stddef.h>

#include <std_msgs/msg/u_int32_multi_array.h>


#define INPUT_DWORDS		1555200
#define INPUT_CHUNK_SIZE	1024


/****************************** MACROS ******************************/
#define SHA256_BLOCK_SIZE 32            // SHA256 outputs a 32 byte digest

/**************************** DATA TYPES ****************************/
typedef unsigned char BYTE;             // 8-bit byte
typedef unsigned int  WORD;             // 32-bit word, change to "long" for 16-bit machines

typedef struct {
	BYTE data[64];
	WORD datalen;
	unsigned long long bitlen;
	WORD state[8];
} SHA256_CTX;

/****************************** MACROS ******************************/
#define ROTLEFT(a,b) (((a) << (b)) | ((a) >> (32-(b))))
#define ROTRIGHT(a,b) (((a) >> (b)) | ((a) << (32-(b))))

#define CH(x,y,z) (((x) & (y)) ^ (~(x) & (z)))
#define MAJ(x,y,z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))
#define EP0(x) (ROTRIGHT(x,2) ^ ROTRIGHT(x,13) ^ ROTRIGHT(x,22))
#define EP1(x) (ROTRIGHT(x,6) ^ ROTRIGHT(x,11) ^ ROTRIGHT(x,25))
#define SIG0(x) (ROTRIGHT(x,7) ^ ROTRIGHT(x,18) ^ ((x) >> 3))
#define SIG1(x) (ROTRIGHT(x,17) ^ ROTRIGHT(x,19) ^ ((x) >> 10))

/**************************** VARIABLES *****************************/
static const WORD k[64] = {
	0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
	0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
	0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
	0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
	0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
	0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
	0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
	0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2
};

/*********************** FUNCTION DEFINITIONS ***********************/
void sha256_transform(SHA256_CTX *ctx, const BYTE data[])
{
	#pragma HLS inline region
	WORD a, b, c, d, e, f, g, h, i, j, t1, t2, m[64];

	#pragma HLS array_partition variable=data cyclic factor=64 dim=0
	#pragma HLS array_partition variable=m cyclic factor=64 dim=0

	for (i = 0, j = 0; i < 16; ++i, j += 4)
	{
		#pragma HLS UNROLL
		m[i] = (data[j] << 24) | (data[j + 1] << 16) | (data[j + 2] << 8) | (data[j + 3]);
	}

	for ( ; i < 64; ++i)
	{
		#pragma HLS UNROLL
		m[i] = SIG1(m[i - 2]) + m[i - 7] + SIG0(m[i - 15]) + m[i - 16];
	}


	a = ctx->state[0];
	b = ctx->state[1];
	c = ctx->state[2];
	d = ctx->state[3];
	e = ctx->state[4];
	f = ctx->state[5];
	g = ctx->state[6];
	h = ctx->state[7];

	for (i = 0; i < 64; ++i) {
		#pragma HLS PIPELINE
		t1 = h + EP1(e) + CH(e,f,g) + k[i] + m[i];
		t2 = EP0(a) + MAJ(a,b,c);
		h = g;
		g = f;
		f = e;
		e = d + t1;
		d = c;
		c = b;
		b = a;
		a = t1 + t2;
	}

	ctx->state[0] += a;
	ctx->state[1] += b;
	ctx->state[2] += c;
	ctx->state[3] += d;
	ctx->state[4] += e;
	ctx->state[5] += f;
	ctx->state[6] += g;
	ctx->state[7] += h;
}

void sha256_init(SHA256_CTX *ctx)
{
	#pragma HLS inline region
	ctx->datalen = 0;
	ctx->bitlen = 0;
	ctx->state[0] = 0x6a09e667;
	ctx->state[1] = 0xbb67ae85;
	ctx->state[2] = 0x3c6ef372;
	ctx->state[3] = 0xa54ff53a;
	ctx->state[4] = 0x510e527f;
	ctx->state[5] = 0x9b05688c;
	ctx->state[6] = 0x1f83d9ab;
	ctx->state[7] = 0x5be0cd19;
}

void sha256_final(SHA256_CTX *ctx, BYTE hash[])
{
	#pragma HLS array_partition variable=hash dim=0
	#pragma HLS inline region

	// Pad whatever data is left in the buffer.
	ctx->data[0] = 0x80;
	for(int i = 1; i < 56; i++)
	{
		#pragma HLS UNROLL
		ctx->data[i] = 0x00;
	}
			


	// Append to the padding the total message's length in bits and transform.
	ctx->bitlen += ctx->datalen * 8;
	ctx->data[63] = ctx->bitlen;
	ctx->data[62] = ctx->bitlen >> 8;
	ctx->data[61] = ctx->bitlen >> 16;
	ctx->data[60] = ctx->bitlen >> 24;
	ctx->data[59] = ctx->bitlen >> 32;
	ctx->data[58] = ctx->bitlen >> 40;
	ctx->data[57] = ctx->bitlen >> 48;
	ctx->data[56] = ctx->bitlen >> 56;
	sha256_transform(ctx, ctx->data);

	// Since this implementation uses little endian byte ordering and SHA uses big endian,
	// reverse all the bytes when copying the final state to the output hash.
	for (int i = 0; i < 4; ++i) {
		#pragma HLS PIPELINE
		hash[i]      = (ctx->state[0] >> (24 - i * 8)) & 0x000000ff;
		hash[i + 4]  = (ctx->state[1] >> (24 - i * 8)) & 0x000000ff;
		hash[i + 8]  = (ctx->state[2] >> (24 - i * 8)) & 0x000000ff;
		hash[i + 12] = (ctx->state[3] >> (24 - i * 8)) & 0x000000ff;
		hash[i + 16] = (ctx->state[4] >> (24 - i * 8)) & 0x000000ff;
		hash[i + 20] = (ctx->state[5] >> (24 - i * 8)) & 0x000000ff;
		hash[i + 24] = (ctx->state[6] >> (24 - i * 8)) & 0x000000ff;
		hash[i + 28] = (ctx->state[7] >> (24 - i * 8)) & 0x000000ff;
	}
}


static void dataflow(hls::stream<uint32_t> &memif_hwt2mem, hls::stream<uint32_t> &memif_mem2hwt, uint32_t inputdata_adr, uint32_t outdata_adr)
{
	uint32_t inputstream[INPUT_CHUNK_SIZE];
	uint32_t outputstream[8];

	#pragma HLS array_partition variable=inputstream cyclic factor=32 dim=0
	#pragma HLS array_partition variable=outputstream cyclic factor=8 dim=0

	//#pragma HLS STREAM variable=inputstream depth=INPUT_CHUNK_SIZE dim=1
	//#pragma HLS STREAM variable=outputstream depth=64 dim=1
	
	//We work on buffers of up to 64 bytes - hard-coded into SHA256 algorithm
	unsigned char seg_buf[64];	   // 64byte segment buffer
	unsigned int seg_offset = 0;   // progress thru the region of interest


	// Initialize the SHA256 context
	SHA256_CTX sha256ctx;
	sha256_init(&sha256ctx);

	#pragma HLS array_partition variable=sha256ctx.state cyclic factor=8 dim=0


	for(int k = 0; k < INPUT_DWORDS/INPUT_CHUNK_SIZE; k++)
	{
		#pragma HLS DATAFLOW

		for(int kk = 0; kk < 0; k++)
		{
			MEM_READ(inputdata_adr, inputstream, INPUT_CHUNK_SIZE * 4);
			inputdata_adr+= (INPUT_CHUNK_SIZE * 4);
		}		

		uint32_t array_cnt = 0;
		// Process the data (byte at a time...)
		for(int j = 0; j < INPUT_CHUNK_SIZE/16; j++)
		{
			#pragma HLS PIPELINE
			for (int i=0; i<16; i++)
			{
				#pragma HLS UNROLL
				uint32_t tmp = inputstream[array_cnt];
				array_cnt++;
				sha256ctx.data[sha256ctx.datalen]	= (uint8_t)tmp;
				sha256ctx.data[sha256ctx.datalen+1] = (uint8_t)tmp >> 8;
				sha256ctx.data[sha256ctx.datalen+2] = (uint8_t)tmp >> 16;
				sha256ctx.data[sha256ctx.datalen+3] = (uint8_t)tmp >> 24;
				sha256ctx.datalen+=4;
			}

			sha256_transform(&sha256ctx, sha256ctx.data);
			sha256ctx.bitlen += 512;
			sha256ctx.datalen = 0;

		}
	}


	// Finish computing the hash (recycle FPGAbuf), and copy results back to proc mem
	sha256_final(&sha256ctx, seg_buf);

	for (int i = 0; i < 32; i += 4) {
		#pragma HLS UNROLL
		outputstream[i>>2] = ((uint32_t)seg_buf[i] | ((uint32_t)seg_buf[i+1] << 8) | ((uint32_t)seg_buf[i+2] << 16) | ((uint32_t)seg_buf[i+3] << 24));
	}
	MEM_WRITE( outputstream, outdata_adr, 8*4);
}


THREAD_ENTRY()
{
	THREAD_INIT();
	uint32_t output_payload_addr[1];
	uint32_t arg = (uint32_t)GET_INIT_DATA();
	uint32_t inputdata_adr = MEMORY_GETMEMORYADDR(rperiodic_srcmem);
	uint32_t outputmsg_adr = MEMORY_GETOBJECTADDR(rperiodic_pub_out);
	outputmsg_adr += OFFSETOF(std_msgs__msg__UInt32MultiArray, data.data);
	MEM_READ(outputmsg_adr, output_payload_addr, 4);					


	dataflow(memif_hwt2mem, memif_mem2hwt, inputdata_adr, output_payload_addr[0]);

	ROS_PUBLISH(rperiodic_pub_out, rperiodic_output_msg );
	THREAD_EXIT();
}
