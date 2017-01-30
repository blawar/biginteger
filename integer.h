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
	integer<BITS + PBITS> operator*(const integer<PBITS>& b) const
	{
		integer<BITS + PBITS> temp = 0;

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
		integer<PBITS> product = 1;
		integer<BITS> sequence = *this % modulus;

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

		if (*this == divisor)
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

		while (true)
		{
			integer<BITS> mid = (max + min) >> 1;
			integer<BITS * 2> p = (divisor * mid);

			if (p > dividend)
			{
				max = mid - integer<BITS>(1);
			}
			else
			{
				integer<BITS>  posRem = dividend - p;
				if (posRem >= divisor)
				{
					min = mid + integer<BITS>(1);
				}
				else
				{
					quotient = mid;
					remainder = posRem;
					break;
				}
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

			integer<BITS*2> t2 = q * x0;
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
		for (long i = size() - 1; i >= 0; i--)
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
	integer<BITS * exponent> pow() const
	{
		integer<BITS * exponent> temp = 1;
		for (long i = 0; i < exponent; i++)
		{
			temp *= *this;
		}
		return temp;
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

template <size_t BITS, size_t PBITS>
integer<PBITS> powmod(integer<BITS>& b, long exp, const integer<PBITS>& modulus)
{
	return b.powmod(exp, modulus);
}