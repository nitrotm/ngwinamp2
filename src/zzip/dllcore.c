// dllcore.cpp : interface de compression
#include "zzip.h"



// Compresse un bloc de donn�e
int zzip_compress(unsigned char* data, unsigned long in_size, unsigned long* out_size)
{
	long code = ZzCompressBlock(data, in_size, 1, 1);
	if(code >= 0)
	{
		*out_size = code;
		return 0;
	}
	*out_size = 0;
	return -1;
}

// D�compresse un bloc de donn�e
int zzip_decompress(unsigned char* data, unsigned long in_size, unsigned long* out_size)
{
	unsigned long dw;
	long		  code;


	// V�rification des tailles
	if(in_size < 4)
		return -1;
	memcpy(&dw, data, 4);
	if(data == 0 || *out_size == 0)
	{
		*out_size = dw;
		return 0;
	}
	if(*out_size < dw)
	{
		*out_size = dw;
		return 1;
	}

	// D�compression
	code = ZzUncompressBlock(data);
	if(code >= 0)
	{
		*out_size = code;
		if(last_error == 0)
			return 0;
		*out_size = 0;
		return -3;
	}
	*out_size = 0;
	return -2;
}
