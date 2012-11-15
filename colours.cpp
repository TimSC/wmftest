// For compilers that support precompilation, includes "wx/wx.h".

#include <mfapi.h>
#include <mfidl.h>
#include <assert.h>
#include "colours.h"

void YV12_To_RGB32(BYTE *in, LONG inStride, UINT32 width, UINT32 height, BYTE *out, LONG outStride)
{
	unsigned int numPix = width * height;
	assert(inStride > 0);//Only positive supported so far
	assert(outStride > 0);

	for (unsigned int y=0; y < height; y++)
		for (unsigned int x=0; x < width; x++)
		{
			BYTE Y = in[y * inStride + x];
			out[y * outStride + 3*x + 0] = Y;
			out[y * outStride + 3*x + 1] = Y;
			out[y * outStride + 3*x + 2] = Y;
		}

}


