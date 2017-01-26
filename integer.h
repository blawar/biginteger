#pragma once
#define PLUS	1
#define MINUS	-1

#include <limits>

typedef unsigned char byte;
typedef unsigned long word;
typedef unsigned long long dword;

#define MIN(a,b) (((a)<(b))?(a):(b))

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
			byte* a = (byte*)&buffer[i];
			const byte* b = (byte*)&ptr[size() - i - 1];
			for (int j = 0; j < sizeof(word); j++)
			{
				a[j] = b[sizeof(word) - j - 1];
			}
		}
	}

	template<size_t PBITS>
	integer<BITS>(integer<PBITS>& n)
	{
		memset(buffer, 0, size() * sizeof(word));
		memcpy(buffer, (const word*)n, MIN(size() * sizeof(word), n.size() * sizeof(word)));
	}

	operator word*() { return buffer; }
	integer<BITS>& operator=(int n)
	{
		memset(buffer, 0, size() * sizeof(word));
		for (int i = 0; i < sizeof(n) && i < sizeof(word); i++)
		{
			((byte*)&buffer[0])[i] = ((byte*)&n)[i];
		}
		//memset(buffer, 0, size()  * sizeof(word));
		//buffer[size() - 1] = n;

		return *this;
	}

	integer<BITS>& operator*=(integer<BITS> n)
	{
		integer<BITS * 2> temp = 0;

		int i, j;
		for (i = 0; i < size(); ++i)
		{
			if (read(i) != 0)
			{
				for (j = 0; j < n.size(); ++j)
				{
					if (n.read(j) != 0) {
						dword c = dword((*this)[i]) * n[j] + temp[i + j];
						temp[i + j] = c;// % std::numeric_limits<word>::max();
						temp[i + j + 1] += c / std::numeric_limits<word>::max();
						//temp[i + j] %= std::numeric_limits<word>::max();
					}
				}
			}
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
		unsigned long sz = size();
		unsigned long i = 0;
		while(i < sz)
		{
			word a = read(i);
			word b = n.read(i);

			if (a == b)
			{
				i++;
				continue;
			}

			if (a < b)
			{
				return true;
			}
			else if (a > b)
			{
				return false;
			}
			i++;
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
		long len = size();
		for (long i = 0; i < len; i++)
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
		word num = 0;
		int carry = 0;
		long len = size();
		for(long i=0; i < len; i++)
		{
			word a = read(i);
			num = a + 1 + carry;

			if (num < a)
			{
				carry = 1;
			}
			else
			{
				write(i, num);
				break;
			}

			write(i, num);
		};
		return *this;
	}

	integer<BITS>& operator--(int)
	{
		long num = 0;


		for (long i = 0; i < size(); i++)
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
		};
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

	word& operator[](unsigned long i)
	{
		return buffer[i];
	}

	const word& operator[](unsigned long i) const
	{
		return buffer[i];
	}

	const word& read(unsigned long i) const
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
				printf("%2.2x", ((byte*)&buffer[size() - i - 1])[sizeof(word) - j - 1]);
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