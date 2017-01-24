//
//  CasRSA_CL.c
//  CasRSA_CL
//
//  Created by Carter McCardwell on 11/18/15.
//  Copyright (c) 2015 Carter McCardwell. All rights reserved.
//

#define __NO_STD_VECTOR
#define MAX_SOURCE_SIZE (0x100000)


#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <string.h>

#include <CL/cl.h>
#include "integer.h"

#include <string>

const unsigned long WordMask = 0x00000000FFFFFFFFUL;

const byte _modulus[] = "\xDE\xCF\xB6\xFC\x3D\x33\xE9\x55\xFD\xAC\x90\xE8\x88\x17\xB0\x03\xA1\x6B\x9A\xAB\x72\x70\x79\x32\xA2\xA0\x8C\xBB\x33\x6F\xB0\x76\x96\x2E\xC4\xE9\x2E\xD8\x8F\x92\xC0\x2D\x4D\x41\x0F\xDE\x45\x1B\x25\x3C\xBE\x37\x6B\x45\x82\x21\xE6\x4D\xB1\x23\x81\x82\xB6\x81\x62\xB7\x30\xF4\x60\x4B\xC7\xF7\xF0\x17\x0C\xB5\x75\x88\x77\x93\x52\x63\x70\xF0\x0B\xC6\x73\x43\x41\xEE\xE4\xF0\x71\xEC\xC8\xC1\x32\xC4\xDC\xA9\x99\x1D\x31\xB8\xA4\x7E\xDD\x19\x04\x0F\x02\xA8\x1A\xAF\xB3\x48\x9A\x29\x29\x5E\x49\x84\xE0\x94\x11\xD1\x7E\xAB\xB2\xC0\x44\x7E\xA1\x1B\x5E\x9D\x0D\x1A\xF9\x02\x9A\x2E\x53\x03\x2D\x48\x96\x7C\x2C\xA6\xD7\xAC\xF1\xED\x2B\x18\xBB\x01\xCB\x13\xB9\xAC\xA6\xEE\x55\x00\x37\x7C\x69\x61\x62\x89\x01\x54\x77\x9F\x07\x5D\x26\x34\x3A\xA9\x49\xA5\xAF\xF2\x5E\x06\x51\xB7\x1C\xE0\xDE\xDA\x5C\x0B\x9F\x98\xC2\x15\xFD\xBA\xD8\xA9\x99\x00\xAB\xA4\x8E\x4A\x16\x9D\x66\x2A\xE8\x56\x64\xB2\xB6\xC0\x93\xAF\x4D\x38\xA0\x16\x5C\xE4\xBD\x62\xC2\x46\x6B\xC9\x5A\x59\x4A\x72\x58\xFD\xB2\xCC\x36\x87\x30\x85\xE8\xA1\x04\x5B\xE0\x17\x9B\xD0\xEC\x9B";
const unsigned int exponent = 65537;

std::string& readFile(const char* fileName)
{
	static std::string buffer;

	buffer = "";
	FILE* f = fopen(fileName, "r");
	if (f == NULL)
	{
		return buffer;
	}
	fseek(f, 0, SEEK_END);

	long fileSize = ftell(f);
	buffer.resize(fileSize);
	fread((void*)buffer.c_str(), 1, fileSize, f);
	fclose(f);
	return buffer;
}

int main(int argc, const char * argv[])
{    
    clock_t c_start, c_stop;

	integer<1024> p;
	integer<1024> q;
	integer<1024> modulus(_modulus);
	integer<1024> message;
	integer<1024> result;
	/*modulus = 0xFFFFFFFF;
	for (integer<1024> i = 0; i < modulus; i++)
	{
		//
	}*/
	/*unsigned long i = 0;
	while(i < 0xFFFFFFFF)
	{
		i++;
	}
	printf("i: %d\n", i);*/

	modulus = 10;
	modulus += 1;
	//modulus = integer<1024>(0xFF) + integer<1024>(2);

	//modulus.pow(1);
	modulus.print();
	return 0;

	printf("INPUT:\n\tM:"); message.print();
	printf("\n\tp:"); p.print();
	printf("\n\tq:"); q.print();
    
	std::string source = std::string("#pragma OPENCL EXTENSION cl_khr_byte_addressable_store : enable\n#define PLUS 1\n#define MINUS -1\n");
	source += readFile("kernel_rsa.cl");

    //Set OpenCL Context
    cl_int err;
    cl_platform_id platform;
    cl_context context;
    cl_command_queue queue;
    cl_device_id device;
	
    err = clGetPlatformIDs(1, &platform, NULL);
    if (err != CL_SUCCESS) { printf("platformid"); }

    err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, 1, &device, NULL);
    if (err != CL_SUCCESS) { printf("getdeivceid %i", err); }

    cl_uint numberOfCores;
    clGetDeviceInfo(device, CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(numberOfCores), &numberOfCores, NULL);
    printf("\nThis GPU supports %i compute units", numberOfCores); //Utilize the maximum number of compute units

    cl_uint maxThreads;
    clGetDeviceInfo(device, CL_KERNEL_WORK_GROUP_SIZE, sizeof(maxThreads), &maxThreads, NULL);
    //printf("\nRunning with %i threads per compute units", maxThreads); //Utilize the maximum number of threads/cu

    context = clCreateContext(0, 1, &device, NULL, NULL, &err);
    if (err != CL_SUCCESS) { printf("context"); }

    queue = clCreateCommandQueue(context, device, 0, &err);
    if (err != CL_SUCCESS) { printf("queue"); }

	const char* append_str = source.c_str();
	const size_t length = source.length();
    cl_program program = clCreateProgramWithSource(context, 1, (const char**)&append_str, &length, &err); //Compile program with expanded key included in the source
    if (err != CL_SUCCESS) { printf("createprogram"); }

    printf("\nBuilding CL Kernel...");
    err = clBuildProgram(program, 1, &device, "-I ./ -cl-std=CL1.2", NULL, NULL); //The fourth parameter is specific to OpenCL 2.0

    if (err == CL_BUILD_PROGRAM_FAILURE) {
        printf("\nBuild Error = %i", err);
        // Determine the size of the log
        size_t log_size;
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);

        // Allocate memory for the log
        char *log = (char *) malloc(log_size);

        // Get the log
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, log_size, log, NULL);

        // Print the log
        //printf("%s\n", log);
		FILE* f = fopen("error.log", "w+");
		fwrite(log, 1, log_size, f);
		fclose(f);
    }
    printf("\t[Done]");

    cl_mem cl_p, cl_q, cl_M, cl_result;
    cl_int status = CL_SUCCESS;
    
    cl_p = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(integer<1024>), &p, &status);
    if (status != CL_SUCCESS || cl_p == NULL) { printf("\nCreate p: %i", status); }

    cl_q = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(integer<1024>), &q, &status);
    if (status != CL_SUCCESS || cl_q == NULL) { printf("\nCreate q: %i", status); }

    cl_M = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(integer<1024>), &message, &status);
    if (status != CL_SUCCESS || cl_M == NULL) { printf("\nCreate M: %i", status); }

    cl_result = clCreateBuffer(context, CL_MEM_WRITE_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(integer<1024>), &result, &status);
    if (status != CL_SUCCESS || cl_result == NULL) { printf("\nCreate result: %i", status); }

	c_start = clock(); //Create a clock to benchmark the time taken for execution

    cl_kernel rsa_kernel = clCreateKernel(program, "rsa_cypher", &status);
    if (status != CL_SUCCESS) { printf("\nclCreateKernel: %i", status); }

    status = clSetKernelArg(rsa_kernel, 0, sizeof(cl_mem), &cl_p);
    status = clSetKernelArg(rsa_kernel, 1, sizeof(cl_mem), &cl_q);
    status = clSetKernelArg(rsa_kernel, 2, sizeof(cl_mem), &cl_M);
    status = clSetKernelArg(rsa_kernel, 3, sizeof(cl_mem), &cl_result);
    status = clSetKernelArg(rsa_kernel, 4, sizeof(int), &exponent);
    if (status != CL_SUCCESS) { printf("\nclSetKernelArg: %i", status); }

    size_t local_ws = 1, global_ws = 1;
    printf("\nRun Parameters: Local: %zu - Global: %zu", local_ws, global_ws);

    status = clEnqueueNDRangeKernel(queue, rsa_kernel, 1, NULL, &global_ws, &local_ws, 0, NULL, NULL);
    if (status != CL_SUCCESS) { printf("\nclEnqueueNDRangeKernel: %i", status); }

    clFinish(queue);

    status = clEnqueueReadBuffer(queue, cl_result, CL_TRUE, 0, sizeof(integer<1024>), &result, 0, NULL, NULL);
    if (status != CL_SUCCESS) { printf("\nclEnqueueReadBuffer: %i", status); }

    printf("\nEncrypted Result: ");
	result.print();

	c_stop = clock();
    float diff = (((float)c_stop - (float)c_start) / CLOCKS_PER_SEC ) * 1000;
    
    printf("\nDone - Time taken: %f ms\n", diff);
    clReleaseMemObject(cl_p);
    clReleaseMemObject(cl_q);
    clReleaseMemObject(cl_M);
    clReleaseMemObject(cl_result);
    clReleaseContext(context);
    clReleaseCommandQueue(queue);
    return 0;
}

