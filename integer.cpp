#include "integer.h"
#include <stdint.h>
#include <math.h>
#include <string.h>
#include <stdio.h>

integer::integer()
{
	*this = 0;
}

integer::integer(int s)
{
	*this = s;
}

integer::integer(integer& n)
{
	memcpy(buffer, (const byte*)n, MAXDIGITS);
}

integer::integer(const byte* n)
{
	for (int i = 0; i < MAXDIGITS; i++)
	{
		buffer[i] = n[i];
	}
}

integer& integer::operator=(int n)
{
	memset(buffer, 0, MAXDIGITS);
	for (int i = 0; i < sizeof(n) && i < MAXDIGITS; i++)
	{
		buffer[MAXDIGITS - i - 1] = ((byte*)&n)[i];
	}
	return *this;
}

integer& integer::operator*=(integer n)
{
	memset(buffer, 0, MAXDIGITS);
	for (int i = 0; i < sizeof(n) && i < MAXDIGITS; i++)
	{
		buffer[MAXDIGITS - i - 1] = ((byte*)&n)[i];
	}
	return *this;
}

integer integer::operator+(integer n)
{
	integer temp(n);
	temp += n;

	return temp;
}

integer integer::operator+=(integer n)
{
	int num = 0;
	int carry = 0;
	for (int i = 0; i < MAXDIGITS; i++)
	{
		num = read(i) + n.read(i) + carry;

		if (num >= 0x100)
		{
			num = num - 0x100;
			carry = 1;
		}

		write(i, num);
	}

	return *this;
}

integer& integer::operator++()
{
	int num = 0;
	int carry = 0;
	for (int i = 0; i < MAXDIGITS; i++)
	{
		num = read(i) + 1 + carry;

		if (num >= 0x100)
		{
			num = num - 0x100;
			carry = 1;
		}
		else
		{
			write(i, num);
			break;
		}

		write(i, num);
	}
	return *this;
}

void integer::print()
{
	for (unsigned long i = 0; i < MAXDIGITS; i++)
	{
		printf("%2.2x ", buffer[i]);
		if (i % 10 == 0)
			printf("\n");
	}
}