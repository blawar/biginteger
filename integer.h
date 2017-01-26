#pragma once
#define PLUS	1
#define MINUS	-1

#include <limits>

typedef unsigned long long uint64;
typedef unsigned long long uint32;
typedef unsigned long long uint16;
typedef unsigned char uint8;
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

	integer<BITS>& operator=(word n)
	{
		memset(buffer, 0, size() * sizeof(word));
		buffer[0] = n;

		return *this;
	}

	operator word*() { return buffer; }

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
						//integer<sizeof(word) * 8 * 2> c = ((*this)[i]) * n[j] + temp[i + j];
						integer<sizeof(word) * 8 * 2> c = (*this)[i];
						c *= n[j];
						c += temp[i + j];

						temp[i + j] = c;
						temp[i + j + 1] += c / std::numeric_limits<word>::max();
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


template<>
class integer<64>
{
public:
	constexpr integer() : t() { }
	constexpr integer(const uint64&t) : t(t) {}
	operator uint64& () { return t; }
	constexpr operator const uint64& () const { return t; }

	const uint64* operator& () const { return &t; }
	uint64* operator& () { return &t; }
private:
	uint64 t;
};

template<>
class integer<32>
{
public:
	constexpr integer() : t() { }
	constexpr integer(const uint32&t) : t(t) {}
	operator uint32& () { return t; }
	constexpr operator const uint32& () const { return t; }

	const uint32* operator& () const { return &t; }
	uint32* operator& () { return &t; }
private:
	uint32 t;
};

template<>
class integer<16>
{
public:
	constexpr integer() : t() { }
	constexpr integer(const uint16&t) : t(t) {}
	operator uint16& () { return t; }
	constexpr operator const uint16& () const { return t; }

	const uint16* operator& () const { return &t; }
	uint16* operator& () { return &t; }
private:
	uint16 t;
};

template<>
class integer<8>
{
public:
	constexpr integer() : t() { }
	constexpr integer(const uint8&t) : t(t) {}
	operator uint8& () { return t; }
	constexpr operator const uint8& () const { return t; }

	const uint8* operator& () const { return &t; }
	uint8* operator& () { return &t; }
private:
	uint8 t;
};
