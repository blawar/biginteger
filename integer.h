#pragma once
#define PLUS	1
#define MINUS	-1

#include <limits>

typedef uint64_t uint64;
typedef uint32_t uint32;
typedef uint16_t uint16;
typedef uint8_t uint8;
typedef unsigned char byte;

typedef unsigned long long word;

#define MIN(a,b) (((a)<(b))?(a):(b))
#define BIT_MASK(x) (((word)1 << x) - 1)

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

	integer<BITS>(word n)
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
		*this = n;
	}

	template<size_t PBITS>
	integer<BITS>& operator=(integer<PBITS>& n)
	{
		memset(buffer, 0, size() * sizeof(word));
		memcpy(buffer, (const word*)n, MIN(size() * sizeof(word), n.size() * sizeof(word)));
		return *this;
	}

	/*integer<BITS>& operator=(int n)
	{
		memset(buffer, 0, size() * sizeof(word));
		for (int i = 0; i < sizeof(n) && i < sizeof(word); i++)
		{
			((byte*)&buffer[0])[i] = ((byte*)&n)[i];
		}
		//memset(buffer, 0, size()  * sizeof(word));
		//buffer[size() - 1] = n;

		return *this;
	}*/

	integer<BITS>& operator=(word n)
	{
		memset(buffer, 0, size() * sizeof(word));
		buffer[0] = n;

		return *this;
	}

	operator word*() { return buffer; }
	word* operator &() { return buffer; }
	//operator const word& () const { return buffer[0]; }

	integer<BITS + (sizeof(word) * 8)> operator*(word n)
	{
		integer<BITS + (sizeof(n) * 8)> temp = 0;

		int i, j;
		for (i = 0; i < size(); ++i)
		{
			if (read(i) != 0)
			{
				integer<MIN(sizeof(word) * 8 * 2, BITS + (sizeof(n) * 8))> c = integer<sizeof(word) * 8>((*this)[i]) * integer<sizeof(word) * 8>(n);
				c += temp[i];

				temp[i] = c.low();
				temp[i + 1] += c.high();
			}
		}
		return temp;
	}

	integer<BITS + (sizeof(word) * 8)> operator*=(word n)
	{
		return *this = *this * n;
	}

	template<size_t PBITS>
	integer<BITS+PBITS> operator*(const integer<PBITS> &b)
	{
		integer<BITS+PBITS> temp = 0;

		int i, j;
		for (i = 0; i < size(); ++i)
		{
			if (read(i) != 0)
			{
				for (j = 0; j < b.size(); ++j)
				{
					if (b[j] != 0) {
						//integer<sizeof(word) * 8 * 2> c = ((*this)[i]) * n[j] + temp[i + j];
						integer<sizeof(word) * 8 * 2> c = integer<sizeof(word) * 8>((*this)[i]) * integer<sizeof(word) * 8>(b[j]);
						//c *= n[j];
						c += temp[i + j];

						temp[i + j] = c.low();
						temp[i + j + 1] += c.high();
					}
				}
			}
		}
		return temp;
	}

	integer<BITS>& operator >>= (unsigned long shift)
	{
		word carry, s;
		while (shift > 0)
		{
			if (shift > sizeof(word) * 8)
			{
				s = sizeof(word) * 8;
				shift -= sizeof(word) * 8;
			}
			else
			{
				s = shift;
				shift = 0;
			}
			for (long i = 0; i < size(); i++)
			{
				if (i)
				{
					carry = BIT_MASK(s) & (*this)[i];
					(*this)[i] = (*this)[i] | (carry << (sizeof(word) * 8 - s));
				}
				(*this)[i] = (*this)[i] >> s;
			}
		}
		return *this;
	}

	integer<BITS> operator >> (unsigned long shift) const
	{
		integer<BITS> temp = *this;
		temp >>= shift;
		return temp;
	}

	template<size_t PBITS>
	integer<BITS>& operator*=(const integer<PBITS> &n)
	{
		return *this = *this * n;
	}

	/*template<size_t PBITS>
	operator/(const integer<PBITS> &b) {
		int i, l1 = (len - 1)*Blen, l2 = (b.len - 1)*Blen;
		int64 x = data[len], y = b[b.len];
		while (x)x /= 10, l1++;
		while (y)y /= 10, l2++;
		bignum tmp, chu, B;
		chu = *this; B = b;

		for (i = 1; i*Blen <= l1 - l2; ++i)B *= base;
		for (i = 1; i <= (l1 - l2) % Blen; ++i)B *= 10;
		for (i = l1 - l2; i >= 0; --i) {
			x = 0;
			while (chu >= B)chu -= B, x++;
			tmp[i / Blen + 1] = tmp[i / Blen + 1] * 10 + x;
			B /= 10;
		}
		tmp.len = (l1 - l2) / Blen + 1;
		while (tmp.len >= 1 && !tmp[tmp.len])tmp.len--;
		return tmp;
	}*/

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

	integer<BITS*2>& pow(long exponent) const
	{
		integer<BITS*2> temp = 1;
		for (long i = 0; i < exponent; i++)
		{
			temp *= *this;
		}
		return temp;
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

	void print() const
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

	integer<BITS / 2> low() const
	{
		integer<BITS / 2> temp;
		memcpy(&temp, (byte*)buffer, BITS / 8 / 2);
		return temp;
	}

	integer<BITS / 2> high() const
	{
		integer<BITS / 2> temp;
		memcpy(&temp, (byte*)buffer + (BITS / 8 / 2), BITS / 8 / 2);
		return temp;
	}

	bool bit(unsigned long i)
	{
		return (((byte*)buffer)[i / 8] >> (i % 8)) & 1;
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
	integer<128> operator*(uint64 s)
	{
		integer<128> result;
		uint32* temp = (uint32*)result.buffer;
		uint32* a = (uint32*)&t;
		uint32* b = (uint32*)&s;

		int i, j;
		for (i = 0; i < 2; ++i)
		{
			for (j = 0; j < 2; ++j)
			{
				uint64 c = (uint64)a[i] * b[j] + temp[i + j];

				temp[i + j] = integer<64>(c).low();
				temp[i + j + 1] += ((integer<64>*)&c)->high();
			}
		}
		return result;
	}

	const uint64* operator& () const { return &t; }
	uint64* operator& () { return &t; }
	uint32 low() { return t; }
	uint32 high() { return t / 0xFFFFFFFF; }
	//static unsigned long size() { return 1; }
	//const uint64& operator[](unsigned int i) const { return t; }
	//uint64& operator[](unsigned int i) { return t; }
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
