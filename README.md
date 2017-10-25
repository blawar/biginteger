A C++ template based statically allocated arbitary length big integer library designed for OpenCL.

I created this library because existing arbitary length bigint libraries leveraged dynamic memory allocation, and thus could not be ported to GPU's / OpenCL.  The use of static allocation does limit the size of integers that can be used, however the main purpose was to be able to do math operations on 512 bit to 4096 bit integers on the GPU for cryptopgraphy operations.
