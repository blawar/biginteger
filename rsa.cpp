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
#include <chrono>

#include <CL/cl.h>
#include "integer.h"

#include <string>

const unsigned long WordMask = 0x00000000FFFFFFFFUL;

integer<2048> modulus = "\xDE\xCF\xB6\xFC\x3D\x33\xE9\x55\xFD\xAC\x90\xE8\x88\x17\xB0\x03\xA1\x6B\x9A\xAB\x72\x70\x79\x32\xA2\xA0\x8C\xBB\x33\x6F\xB0\x76\x96\x2E\xC4\xE9\x2E\xD8\x8F\x92\xC0\x2D\x4D\x41\x0F\xDE\x45\x1B\x25\x3C\xBE\x37\x6B\x45\x82\x21\xE6\x4D\xB1\x23\x81\x82\xB6\x81\x62\xB7\x30\xF4\x60\x4B\xC7\xF7\xF0\x17\x0C\xB5\x75\x88\x77\x93\x52\x63\x70\xF0\x0B\xC6\x73\x43\x41\xEE\xE4\xF0\x71\xEC\xC8\xC1\x32\xC4\xDC\xA9\x99\x1D\x31\xB8\xA4\x7E\xDD\x19\x04\x0F\x02\xA8\x1A\xAF\xB3\x48\x9A\x29\x29\x5E\x49\x84\xE0\x94\x11\xD1\x7E\xAB\xB2\xC0\x44\x7E\xA1\x1B\x5E\x9D\x0D\x1A\xF9\x02\x9A\x2E\x53\x03\x2D\x48\x96\x7C\x2C\xA6\xD7\xAC\xF1\xED\x2B\x18\xBB\x01\xCB\x13\xB9\xAC\xA6\xEE\x55\x00\x37\x7C\x69\x61\x62\x89\x01\x54\x77\x9F\x07\x5D\x26\x34\x3A\xA9\x49\xA5\xAF\xF2\x5E\x06\x51\xB7\x1C\xE0\xDE\xDA\x5C\x0B\x9F\x98\xC2\x15\xFD\xBA\xD8\xA9\x99\x00\xAB\xA4\x8E\x4A\x16\x9D\x66\x2A\xE8\x56\x64\xB2\xB6\xC0\x93\xAF\x4D\x38\xA0\x16\x5C\xE4\xBD\x62\xC2\x46\x6B\xC9\x5A\x59\x4A\x72\x58\xFD\xB2\xCC\x36\x87\x30\x85\xE8\xA1\x04\x5B\xE0\x17\x9B\xD0\xEC\x9B";
integer<2048> _sample = "\xC2\x6E\xAA\xCE\xDD\x4F\xF3\x1C\xD9\x70\x26\x2B\x2A\x6B\xE0\x6D\x5C\xEC\x11\x15\x52\x8C\xAA\x6F\x00\xBA\xDA\x3A\x6B\x9A\x88\x6B\x5E\x35\xDE\x4F\xB7\xE9\xE4\x35\x6C\x4B\x06\xB3\x10\xCC\xA1\x5A\xED\x2B\x7B\x43\x3D\xAB\x68\x1B\x03\x66\xCC\x3C\x76\x9F\x6D\x35\x79\xE6\xB8\x16\xA8\xF0\x1B\xE9\xC5\x8C\x1A\x61\xA5\xAB\x81\x7E\x2C\x2F\xC5\x5C\x8C\x70\xF5\x84\xD8\xD4\x85\xE7\x55\x84\xD7\x1A\x0E\xA1\xA6\x09\x27\x51\xDB\xE6\xBC\xBB\xE3\xC1\x19\xA4\xCB\xA5\xE3\x83\xE7\x40\x81\x31\x29\xAA\x4E\x9C\xB4\x9D\xD3\x96\xBB\x7F\x97\xF3\x32\xFA\xA2\x4F\x0A\x4B\xCB\xC3\x62\xE3\x4D\x4F\x09\xF1\x39\x5B\x56\x5C\xC6\x15\x3D\x37\xF0\x57\xA0\x49\x68\x86\xE6\x6E\x96\x5B\xE0\x8A\x10\x30\xEA\x03\x8B\xC4\x5D\xDF\x6D\x4F\x52\x7F\x3E\xD4\x1E\x25\x45\xC0\xE4\x77\x2E\xA6\xA3\xF9\x7D\xD2\xA0\xC7\x0D\x34\x07\x69\xE8\xAF\x21\x1C\xD1\xEE\xB5\x04\xA9\x6C\x70\xB4\xDE\x40\xAD\x14\x6B\xF6\x3F\x50\x9F\xD5\x6A\x55\x35\x82\x11\xCC\x27\xA9\x69\x14\x76\x9E\x50\x86\x4F\xF4\xEE\xA2\x45\xA5\xFF\xA9\x52\x65\xD5\x73\x3E\xDB\x0D\x33\xD9\xD1\x60\x2F\x5F\x3C\xC8\xE6";
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

int mul_inv(int a, int b)
{
	int b0 = b, t, q;
	int x0 = 0, x1 = 1;
	if (b == 1) return 1;
	while (a > 1) {
		q = a / b;
		t = b, b = a % b, a = t;
		t = x0, x0 = x1 - q * x0, x1 = t;
	}
	if (x1 < 0) x1 += b0;
	return x1;
}

int main(int argc, const char * argv[])
{    
    clock_t c_start, c_stop;

	integer<1024> p;
	integer<1024> q;
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

	integer<128> c = 42;
	integer<2048> a;
	//a <<= 64;
	//a -= integer<128>(1);
	//a += 0;
	//a >>= 1;
	//a /= integer<128>(0x100);
	//integer<128> c = integer<128>(2);
	//integer<128> m = 33;
	//auto d = (integer<128>((word)0xabcdef0017569217) *  integer<128>((word)0x7a88500ae544c291));
	//d.print();

	//powmod(integer<128>((word)0x12345678aabbccdd), 0x100, integer<128>(0xabcdef0017569217)).print();
	powmod(_sample, 0x1, modulus).print();
	return 0;
	//a <<= 64;
	//a -= 1;
	//integer<64> b = 200;
	//integer<128> c = a * b;
	//a -= 2;
	//a.print();
	//return 0;
	//c *= 2;

	/*p = BIT_MASK(64);
	for (int i = 0; i <128; i++)
	{
		if (i % 16 == 0) printf("\n");
		printf("%d ", p.bit(i)?1:0);
	}
	printf("\n");
	return 0;*/
	//printf("log: %d\n", ilog2<0xFFFFFF>::value);
	
	/*auto t = new integer<2048 * 0x100>();
	printf("sizeof(t) = %d\n", sizeof(*t));
	modulus.pow<0x100>(*t).print();
	delete t;*/

	//printf("%u\n", c.modularInverse(a));
	//printf("%d\n", mul_inv(42, 2017));
	//modulus.modularInverse(integer<2048>(0xFFFF)).print();
	return 0;

	std::chrono::milliseconds start = std::chrono::duration_cast< std::chrono::milliseconds >(std::chrono::system_clock::now().time_since_epoch());
	//modulus = 1;
	p = 2;
	unsigned long i;
	for (i = 0; i < 0x1000; i++)
	{
		c / integer<1024>(0xFFFF);
	}
	std::chrono::milliseconds end = std::chrono::duration_cast< std::chrono::milliseconds >(std::chrono::system_clock::now().time_since_epoch());
	if ((end - start).count() == 0)
	{
		printf("ops/sec = infinity\n");
	}
	else
	{
		printf("ops/sec = %d\n", i * 1000 / (end - start).count());
	}
	//modulus = integer<1024>(0xFF) + integer<1024>(2);


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

