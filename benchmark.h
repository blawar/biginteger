#pragma once

#include "integer.h"

template<size_t BITS>
void benchmark()
{
	integer<BITS> _sample;
	integer<BITS> modulus;

	{
		std::chrono::milliseconds start = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
		unsigned long i;
		for (i = 0; i < 0x100000; i++)
		{
			_sample + modulus;
		}
		std::chrono::milliseconds end = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
		if ((end - start).count() == 0)
		{
			printf("addition %d ops/sec = infinity\n", BITS);
		}
		else
		{
			printf("addition %d ops/sec = %d\n", BITS, i * 1000 / (end - start).count());
		}
	}

	{
		std::chrono::milliseconds start = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
		unsigned long i;
		for (i = 0; i < 0x100000; i++)
		{
			_sample - modulus;
		}
		std::chrono::milliseconds end = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
		if ((end - start).count() == 0)
		{
			printf("subtraction %d ops/sec = infinity\n", BITS);
		}
		else
		{
			printf("subtraction %d ops/sec = %d\n", BITS, i * 1000 / (end - start).count());
		}
	}

	{
		std::chrono::milliseconds start = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
		unsigned long i;
		for (i = 0; i < 0x100000; i++)
		{
			_sample / modulus;
		}
		std::chrono::milliseconds end = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
		if ((end - start).count() == 0)
		{
			printf("divide %d ops/sec = infinity\n", BITS);
		}
		else
		{
			printf("divide %d ops/sec = %d\n", BITS, i * 1000 / (end - start).count());
		}
	}

	{
		std::chrono::milliseconds start = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
		unsigned long i;
		for (i = 0; i < 0x1000; i++)
		{
			_sample.multiply(modulus);
		}
		std::chrono::milliseconds end = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
		if ((end - start).count() == 0)
		{
			printf("multiply %d ops/sec = infinity\n", BITS);
		}
		else
		{
			printf("multiply %d ops/sec = %d\n", BITS, i * 1000 / (end - start).count());
		}
	}

	{
		std::chrono::milliseconds start = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
		unsigned long i;
		for (i = 0; i < 0x10000; i++)
		{
			_sample.multiplyKaratsuba(modulus);
		}
		std::chrono::milliseconds end = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
		if ((end - start).count() == 0)
		{
			printf("multiply %d Karatsuba ops/sec = infinity\n", BITS);
		}
		else
		{
			printf("multiply %d Karatsuba ops/sec = %d\n", BITS, i * 1000 / (end - start).count());
		}
	}
	printf("\n*******************************************\n\n");
}