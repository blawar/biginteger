#pragma once
#define PLUS	1
#define MINUS	-1

#include <limits>

typedef unsigned char byte;
typedef size_t word;

template<size_t BITS>
class integer
{
public:
	integer<BITS>()
	{
		*this = 0;
	}

	integer<BITS>(int n)
	{
		*this = n;
	}

	integer<BITS>(const void* n)
	{
		const word* ptr = (word*)n;
		for (int i = 0; i < size(); i++)
		{
			buffer[i] = ptr[i];
		}
	}

	integer<BITS>(integer<BITS>& n)
	{
		memcpy(buffer, (const word*)n, size() * sizeof(word));
	}

	operator word*() { return buffer; }
	integer<BITS>& operator=(int n)
	{
		memset(buffer, 0, size()  * sizeof(word));

		unsigned long i = 0;

		do
		{
			buffer[size() - i - 1] = ((word*)&n)[i];
			i++;
		}
		while (i * sizeof(word) < sizeof(n) && i < size());

		return *this;
	}

	integer<BITS>& operator*=(integer<BITS> n)
	{
		integer<BITS> temp = 0;
		integer<BITS> i = 0;
		while(i < n)
		{
			//temp += *this;
			i++;
		}
		*this = temp;
		return *this;
	}

	template<size_t PBITS>
	bool operator==(integer<BITS> n)
	{
		integer<BITS> temp = 0;
		for (integer<BITS> i = 0; i < n; i++)
		{
			if (read(i) != n.read(i))
			{
				return false;
			}
		}
		return true;
	}

	template<size_t PBITS>
	bool operator!=(integer<PBITS> n)
	{
		return !(*this == n);
	}

	template<size_t PBITS>
	bool operator<(integer<PBITS> n)
	{
		unsigned long i = size();
		while(i > 0)
		{
			i--;
			word a = read(i);
			word b = n.read(i);
			if (a < b)
			{
				return true;
			}
		}
		return false;
	}

	integer<BITS> operator+(integer<BITS> n)
	{
		integer<BITS> temp(n);
		temp += n;

		return temp;
	}

	integer<BITS> operator+=(integer<BITS> n)
	{
		word num = 0;
		int carry = 0;
		int sz = size();
		for (int i = 0; i < size(); i++)
		{
			word a = read(i);
			word b = n.read(i);
			num = a + b + carry;

			if (num < a || num < b)
			{
				carry = 1;
			}
			else
			{
				carry = 0;
			}

			write(i, num);
		}

		return *this;
	}

	integer<BITS>& operator++(int)
	{
		int num = 0;
		for (int i = 0; i < size(); i++)
		{
			num = read(i) + 1;

			if (num >= 0x100)
			{
				num = num - 0x100;
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

	integer<BITS>& operator--(int)
	{
		long num = 0;


		for (unsigned long i = 0; i < size(); i++)
		{
			num = read(i) - 1;

			if (num < 0)
			{
				num = 0xFF;
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

	integer<BITS>& pow(long exponent)
	{
		integer<BITS> temp = 1;
		for (long i = 0; i < exponent; i++)
		{
			temp *= *this;
		}
		*this = temp;
		return *this;
	}

	word read(unsigned long i) const
	{
		return buffer[i];
	}

	void write(unsigned long i, word value)
	{
		buffer[i] = value;
	}

	void print()
	{
		const char* fmt;
		switch (sizeof(word))
		{
		case 1:
			fmt = "%2.2x ";
			break;
		case 2:
			fmt = "%4.4x ";
			break;
		case 3:
			fmt = "%6.6x ";
			break;
		case 4:
			fmt = "%8.8x ";
			break;
		case 5:
			fmt = "%10.10x ";
			break;
		case 6:
			fmt = "%12.12x ";
			break;
		case 7:
			fmt = "%14.14x ";
			break;
		case 8:
			fmt = "%16.16x ";
			break;
		}
		for (unsigned long i = 0; i < size(); i++)
		{
			if (i % 8 == 0 && i != 0)
				printf("\n");
			
			for (int j = 0; j < sizeof(word); j++)
			{
				printf("%2.2x", ((byte*)&buffer[i])[j]);
			}
			printf(" ");
		}
		printf("\n");
	}

	static unsigned long size()
	{
		return BITS / 8 / sizeof(word);
	}
//private:
	word buffer[BITS / 8 / sizeof(word)];
};