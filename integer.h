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
#define MAX(a,b) (((a)>(b))?(a):(b))
#define BIT_MASK(x) (((word)1 << x) - 1)

template <word x>
struct ilog2 {
	enum { value = (1 + ilog2<x / 2>::value) };
};

template <>
struct ilog2<1> {
	enum { value = 0 };
};

template<class T>
inline constexpr T pow(const T base, unsigned const exponent)
{
	return (exponent == 0) ? 1 : (base * pow(base, exponent - 1));
}

template < typename T, T base, unsigned exponent >
using pow_ = std::integral_constant < T, pow(base, exponent) >;

#define POW(BASE, EXPONENT) (pow_ < decltype(BASE), BASE, EXPONENT > :: value)

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
	integer<BITS>(const integer<PBITS>& n)
	{
		*this = n;
	}

	template<size_t PBITS>
	integer<BITS>& operator=(const integer<PBITS>& n)
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

	operator const word*() const { return buffer; }
	operator word*() { return buffer; }
	const word* operator &() const { return buffer; }
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
	integer<BITS + PBITS> operator*(const integer<PBITS>& b) const
	{
		integer<BITS + PBITS> temp = 0;

		int i, j;
		for (i = 0; i < size(); ++i)
		{
			//if (read(i) != 0)
			{
				for (j = 0; j < b.size(); ++j)
				{
					//if (b[j] != 0)
					{
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
		if (shift >= sizeof(word) * 8)
		{
			unsigned long wordShift = shift / (sizeof(word) * 8);
			shift %= sizeof(word) * 8;

			for (long i = size() - 1; i >= wordShift; i--)
			{
				(*this)[i - wordShift] = (*this)[i];
			}
		}

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
					(*this)[i - 1] = (*this)[i - 1] | (carry << (sizeof(word) * 8 - s));
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

	integer<BITS>& operator <<= (unsigned long shift)
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
			for (long i = size() - 1; i >= 0; i--)
			{
				if (i != size() - 1)
				{
					carry = ~BIT_MASK(sizeof(word) * 8 - s) & (*this)[i];
					(*this)[i + 1] = (*this)[i + 1] | carry;
				}
				if (s == sizeof(word) * 8)
				{
					(*this)[i] = 0;
				}
				else
				{
					(*this)[i] = (*this)[i] << s;
				}
			}
		}
		return *this;
	}

	integer<BITS> operator << (unsigned long shift) const
	{
		integer<BITS> temp = *this;
		temp <<= shift;
		return temp;
	}

	template<size_t PBITS>
	integer<BITS>& operator*=(const integer<PBITS> &n)
	{
		return *this = *this * n;
	}

	template <size_t PBITS>
	integer<PBITS> powmod(long exp, const integer<PBITS> modulus)
	{
		integer<BITS> product = 1;
		integer<PBITS> sequence = *this % modulus;

		while (exp != 0)
		{
			if (exp & 1)
			{
				product = (product * sequence) % modulus;
			}
			sequence = (sequence * sequence) % modulus;
			exp >>= 1;
		}
		return product;
	}

	/*template <size_t PBITS>
	integer<BITS> divide(const integer<PBITS> &b, integer<BITS> &modulus) const
	{
		integer<BITS> tmp = 0;
		integer<BITS> c = 0;

		for (long i = size() - 1; i >= 0; i--)
		{
			modulus <<= sizeof(word) * 8;

			modulus[0] = (*this)[i];
			c[i] = 0;

			while (modulus > b)
			{
				c[i]++;
				modulus -= b;
			}
		}

		return c;
	}*/

	template <size_t PBITS>
	integer<BITS> divide(const integer<PBITS> &divisor, integer<BITS>& remainder) const
	{
		integer<BITS> tmp = 0;
		integer<BITS> c = 0;

		for (long i = size() - 1; i >= 0; i--)
		{
			remainder <<= sizeof(word) * 8;

			remainder[0] = (*this)[i];
			c[i] = 0;

			while (remainder > divisor)
			{
				c[i]++;
				remainder -= divisor;
			}
		}

		return c;
	}


	template <size_t PBITS>
	integer<BITS> divideAndConquer(const integer<PBITS>& divisor, integer<BITS>& remainder) const
	{
		const integer<BITS>& dividend = *this;
		integer<BITS> quotient;

		if (dividend < divisor)
		{
			quotient = 0;
			remainder = dividend;
			return quotient;
		}

		if (dividend == divisor)
		{
			quotient = 1;
			remainder = 0;
			return quotient;
		}

		if (divisor == integer<PBITS>(0))
		{
			// should throw exception here
			quotient = 0;
			remainder = 0;
			return quotient;
		}

		integer<BITS> min = 0, max = dividend;
		unsigned long c = 0;
		while (true)
		{
			if (c++ == 0x87d)
			{
				int y = 0;
			}
			integer<BITS> mid = (max + min) >> 1;
			integer<BITS + PBITS> p = (divisor * mid);
			integer<BITS>  posRem = dividend - p;

			if (min >= integer<2048>("\xa9\xaa\xf4\xb5\x40\x55\x2a\xed\x22\xae\x23\x48\x3f\x28\x69\xa7\x7c\x41\x97\x42\x98\x70\x2c\xe5\x17\x78\x9c\xe8\x7b\xfd\x98\x0d\xc6\xd8\x95\xb6\x94\xc8\x36\x83\xd8\xe1\x87\x4b\x56\x7d\xd9\x40\x9b\xbf\x8b\x34\x68\x75\xee\x10\xf4\x3f\x69\x75\x90\x07\x3a\xce\xe9\x6b\xc3\xc2\x2e\x33\x82\x35\xe7\x63\x3d\xba\x54\x91\x15\x91\x74\x1e\x85\xe2\xea\x65\x28\x84\x50\xc4\xb3\x13\xa9\xdf\x6e\xee\x99\xd5\x1c\xba\xc4\x48\x3c\x39\x11\x18\x7d\x91\x25\x85\x6d\x8a\x2a\x0f\xe5\x96\x28\xea\x9b\xd9\xa7\x0a\xa6\x81\x43\xb5\xa2\xc1\x35\x3e\x76\xfa\x33\xa2\x5a\x3c\x32\x67\x77\x3e\x57\xc7\xa3\x94\xa2\x3c\x63\x70\xde\x15\x8a\x32\x36\x1a\x39\x92\x41\x5f\x36\x26\xdd\xda\x88\x49\x6e\xe2\x06\x0e\xcc\x1f\xc4\xde\xda\x1e\xd3\xdf\x66\x35\xef\x07\xd7\x67\x24\x9a\xb8\x5c\x6f\xf9\x22\x79\xff\xee\xa8\x48\x74\x29\x69\x47\xfc\xab\x68\x51\x0b\x2b\xf7\x01\xd2\x19\x66\xb8\x77\x3f\x08\x26\xde\x3a\x27\xa5\x72\x6b\x42\x4a\xcf\xec\x5b\x9a\xa4\xb5\x3c\x86\x1c\xd0\x12\xaf\xc6\x97\x86\x03\xc4\x5d\x71\x7c\xac\xad\x1e\xee\x05\x47\x16\xb0\xfd\xeb\x96\xbe\x60\x01"))
			{
				int x = 0;
			}

			if (p > dividend)
			{
				if (mid == max)
				{
					max -= integer<BITS>(1);
				}
				else
				{
					max = mid;
				}
			}
			else if (posRem >= divisor)
			{
				if (mid == min)
				{
					min += integer<BITS>(1);
				}
				else
				{
					min = mid;
				}
			}
			else
			{
				quotient = mid;
				remainder = posRem;
				break;
			}

		}
		return quotient;
	}

	template <size_t PBITS>
	integer<BITS> operator/(const integer<PBITS> &b) const
	{
		integer<BITS> modulus;
		return divideAndConquer(b, modulus);
	}

	template <size_t PBITS>
	integer<BITS> operator/=(const integer<PBITS> &b)
	{
		integer<BITS> temp = *this / b;
		return *this = temp;
	}

	template <size_t PBITS>
	integer<BITS> operator%(const integer<PBITS> &b) const
	{
		integer<BITS> modulus;
		divideAndConquer(b, modulus);
		return modulus;
	}

	template <size_t PBITS>
	integer<BITS> operator%=(const integer<PBITS> &b)
	{
		integer<BITS> temp = *this % b;
		return *this = temp;
	}

	template<size_t PBITS>
	bool operator==(const integer<PBITS>& n) const
	{
		word i, len = MIN(size(), n.size());
		for (i = 0; i < len; i++)
		{
			if (read(i) != n.read(i))
			{
				return false;
			}
		}
		if (size() < n.size())
		{
			for (; i < n.size(); i++)
			{
				if (n.read(i))
				{
					return false;
				}
			}
		}

		if (size() > n.size())
		{
			for (; i < size(); i++)
			{
				if (read(i))
				{
					return false;
				}
			}
		}
		return true;
	}

	integer<BITS> modularInverse(integer<BITS> b)
	{
		integer<BITS>& a = *this;
		integer<BITS> b0 = b, t, q;
		integer<BITS> x0 = 0, x1 = 1;
		bool sign = false;

		if (b == integer<BITS>(1))
		{
			return integer<BITS>(1);
		}

		while (a > integer<BITS>(1))
		{
			q = a / b;
			t = b;
			b = a % b;
			a = t;

			t = x0;

			integer<BITS * 2> t2 = q * x0;
			x0 = x1 - t2;
			if (t2 > x1)
			{
				sign = true;
			}
			x1 = t;
		}

		if (sign)
		{
			x1 += b0;
		}
		return x1;
	}

	template<size_t PBITS>
	bool operator!=(const integer<PBITS>& n) const
	{
		return !(*this == n);
	}

	template<size_t PBITS>
	bool operator<(const integer<PBITS>& n) const
	{
		long i;
		if (n.size() > size())
		{
			for (i = n.size() - 1; i >= size(); i--)
			{
				if (n[i] != 0)
				{
					return true;
				}
			}
		}
		else
		{
			for (i = size() - 1; i >= n.size(); i--)
			{
				if ((*this)[i] != 0)
				{
					return false;
				}
			}
		}
		for (; i >= 0; i--)
		{
			word a = read(i);
			word b = n.read(i);

			if (a == b)
			{
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
		}
		return false;
	}

	template<size_t PBITS>
	bool operator>(const integer<PBITS>& n) const
	{
		return n < *this;
	}

	template<size_t PBITS>
	bool operator>=(const integer<PBITS>& n) const
	{
		return !(*this < n);
	}

	template<size_t PBITS>
	bool operator<=(const integer<PBITS>& n) const
	{
		return !(*this > n);
	}

	integer<BITS> operator+(const integer<BITS> n) const
	{
		integer<BITS> temp = *this;
		temp += n;

		return temp;
	}

	integer<BITS>& operator+=(const integer<BITS> n)
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

	template<size_t PBITS>
	integer<BITS> operator-(const integer<PBITS>& n) const
	{
		integer<BITS> temp = *this;
		temp -= n;

		return temp;
	}

	template<size_t PBITS>
	integer<BITS>& operator-=(const integer<PBITS>& n)
	{
		word num = 0;
		int borrow = 0;
		long len = MIN(size(), n.size());
		long i;

		for (i = 0; i < len; i++)
		{
			word a = read(i);
			word b = n.read(i);
			num = a - b - borrow;

			if (num > a)
			{
				borrow = 1;
				num += std::numeric_limits<word>::max() + 1;
			}
			else
			{
				borrow = 0;
			}

			write(i, num);
		}

		if (size() < n.size())
		{
			for (; i < n.size(); i++)
			{
				word a = 0;
				word b = n.read(i);
				num = a - b - borrow;

				if (num > a)
				{
					borrow = 1;
					num += std::numeric_limits<word>::max() + 1;
				}
				else
				{
					borrow = 0;
				}

				//write(i, num);
			}
		}
		else if (size() > n.size())
		{
			for (; i < size(); i++)
			{
				word a = read(i);
				word b = 0;
				num = a - b - borrow;

				if (num > a)
				{
					borrow = 1;
					num += std::numeric_limits<word>::max() + 1;
				}
				else
				{
					borrow = 0;
				}

				write(i, num);
			}
		}

		return *this;
	}

	integer<BITS>& operator++(int)
	{
		word num = 0;
		int carry = 0;
		long len = size();
		for (long i = 0; i < len; i++)
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

	template<size_t exponent>
	integer<BITS * exponent>& pow(integer<BITS * exponent>& result) const
	{
		result = 1;
		integer<BITS * POW(2, ilog2<exponent>::value)> temp = *this;
		long exp = exponent;

		while (exp > 0)
		{
			if (exp % 2 == 1)
			{
				result *= temp;
			}

			exp >>= 1;

			if (exp != 0)
			{
				temp *= temp;
			}
		}
		return result;
	}

	template<size_t exponent>
	integer<BITS * exponent> pow() const
	{
		integer<BITS * exponent> result;
		return pow<exponent>(result);
	}

	integer<BITS * 2>& pow(long exponent) const
	{
		integer<BITS * 2> temp = 1;
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
		integer<BITS / 2> temp = *this;
		//memcpy(&temp, (byte*)buffer, BITS / 8 / 2);
		return temp;
	}

	integer<BITS / 2> high() const
	{
		/*integer<BITS / 2> temp;// = *this >> (BITS / 2);
		memcpy(&temp, (byte*)buffer + (BITS / 8 / 2), BITS / 8 / 2);
		return temp;*/
		if (BITS / 2 == 64)
		{
			int t = 0;
		}
		integer<BITS / 2> temp = (*this >> (BITS / 2));
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
	template <size_t PBITS>
	constexpr integer(const integer<PBITS>& t) : t(t[0]) { }
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
				uint64 c = (uint64)a[i] * (uint64)b[j] + temp[i + j];

				temp[i + j] = integer<64>(c).low();
				temp[i + j + 1] += ((integer<64>*)&c)->high();
			}
		}
		return result;
	}

	const uint64* operator& () const { return &t; }
	uint64* operator& () { return &t; }
	uint32 low() { return t; }
	uint32 high() { return t >> 32; }
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

template <size_t BITS, size_t PBITS>
integer<PBITS> powmod(integer<BITS>& b, long exp, const integer<PBITS>& modulus)
{
	return b.powmod(exp, modulus);
}