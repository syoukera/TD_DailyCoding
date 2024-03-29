/* Shared Use License: This file is owned by Derivative Inc. (Derivative)
* and can only be used, and/or modified for use, in conjunction with
* Derivative's TouchDesigner software, and only if you are a licensee who has
* accepted Derivative's TouchDesigner license or assignment agreement
* (which also govern the use of this file). You may share or redistribute
* a modified version of this file provided the following conditions are met:
*
* 1. The shared file or redistribution must retain the information set out
* above and this list of conditions.
* 2. Derivative's name (Derivative Inc.) or its trademarks may not be used
* to endorse or promote products derived from this file without specific
* prior written permission from Derivative.
*/

#include "cuda_runtime.h"
#include "device_launch_parameters.h"

#include <stdio.h>

__global__ void
copyTextureRGBA8(int width, int height, cudaSurfaceObject_t input, cudaSurfaceObject_t output)
{
	unsigned int x = blockIdx.x * blockDim.x + threadIdx.x;
	unsigned int y = blockIdx.y * blockDim.y + threadIdx.y;

	if (x >= width || y >= height)
		return;

	uchar4 color_center;
	surf2Dread(&color_center, input, x * 4, y, cudaBoundaryModeZero);
	uchar4 color_right;
	surf2Dread(&color_right, input, (x+1) * 4, y, cudaBoundaryModeZero);
	uchar4 color_left;
	surf2Dread(&color_left, input, (x-1) * 4, y, cudaBoundaryModeZero);
	
	uchar4 color;
	color.x = (color_center.x + color_right.x + color_left.x)/3;
	color.y = (color_center.y + color_right.y + color_left.y)/3;
	color.z = (color_center.z + color_right.z + color_left.z)/3;
	color.w = (color_center.w + color_right.w + color_left.w)/3;

	surf2Dwrite(color, output, x * 4, y, cudaBoundaryModeZero);
}

__global__ void
makeOutputRed(int width, int height, cudaSurfaceObject_t output)
{
	unsigned int x = blockIdx.x * blockDim.x + threadIdx.x;
	unsigned int y = blockIdx.y * blockDim.y + threadIdx.y;

	if (x >= width || y >= height)
		return;

	uchar4 color = make_uchar4(255, 0, 0, 255);
	surf2Dwrite(color, output, x * 4, y, cudaBoundaryModeZero);
}

int
divUp(int a, int b)
{
	return ((a % b) != 0) ? (a / b + 1) : (a / b);
}


cudaError_t doCUDAOperation(int width, int height, cudaSurfaceObject_t input, cudaSurfaceObject_t output)
{
	cudaError_t cudaStatus;

	dim3 blockSize(16, 16, 1);

	dim3 gridSize(divUp(width, blockSize.x), divUp(height, blockSize.y), 1);

	if (input)
	{
		copyTextureRGBA8<<<gridSize, blockSize>>>(width, height, input, output);
	}
	else
	{
		makeOutputRed << <gridSize, blockSize >> > (width, height, output);
	}


#ifdef _DEBUG
	// any errors encountered during the launch.
	cudaStatus = cudaDeviceSynchronize();
	if (cudaStatus != cudaSuccess)
	{
		fprintf(stderr, "cudaDeviceSynchronize returned error code %d after launching kernel!\n", cudaStatus);
	}
#else
	cudaStatus = cudaSuccess;
#endif

	return cudaStatus;
}
