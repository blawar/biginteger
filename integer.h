#pragma once

#include <limits>
#ifdef _MSC_VER
#include <intrin.h>
#endif

typedef uint64_t uint64;
typedef uint32_t uint32;
typedef uint16_t uint16;
typedef uint8_t uint8;
typedef unsigned char byte;

typedef unsigned long long word;

#define WORD_BITS (sizeof(word) * 8)
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

	integer<BITS>(word n)
	{
		*this = n;
	}

	integer<BITS>(const char(&n)[BITS / 8 + 1])
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
		memcpy(buffer, (const word*)&n, MIN(size() * sizeof(word), n.size() * sizeof(word)));
		return *this;
	}

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

	const constexpr word& first() const { return *buffer; }
	word& first() { return *buffer; }

	const constexpr word& last() const { return read(size()-1); }
	word& last() { return read(size()-1); }

	integer<BITS + (WORD_BITS)> operator*(word n)
	{
		integer<BITS + (sizeof(n) * 8)> temp = 0;

		int i, j;
		for (i = 0; i < size(); ++i)
		{
			//if (read(i) != 0)
			{
				integer<WORD_BITS * 2> c = integer<WORD_BITS>(read(i)) * integer<WORD_BITS>(n);
				c += temp[i];

				temp[i] = c.low();
				temp[i + 1] = c.high();
			}
		}
		return temp;
	}

	integer<BITS + (WORD_BITS)> operator*=(word n)
	{
		return *this = *this * n;
	}

	template<size_t PBITS>
	integer<BITS + PBITS> operator*(const integer<PBITS>& b) const
	{
		if (BITS + PBITS >= 4096 && BITS == PBITS)
		{
			return multiplyKaratsuba(b);
		}
		else
		{
			return multiply(b);
		}
	}

	template<size_t PBITS>
	integer<(BITS + PBITS) * 2> multiplyWithCarry(const integer<PBITS>& b, bool a_carry, bool b_carry) const
	{
		const integer<BITS>& a = *this;
		integer<(BITS + PBITS) * 2> result;

		auto& s0 = result.low().low();
		auto& s1 = result.low().high();
		word& s2 = result.high().low().first();

		integer<BITS * 2> x = a * b;

		s0 = x.low();

		x = integer<BITS * 2>(x).high();
		if (a_carry)
		{
			x += b;
		}

		word tmps2 = x.high().first();

		x = integer<BITS * 2>(x).low();
		if (b_carry)
		{
			x += a;
		}

		s1 = x.low();

		if (a_carry && b_carry)
		{
			x = integer<BITS * 2>(x).high();
			x++;
			x += tmps2;
		}
		else
		{
			x = integer<BITS * 2>(x).high();
			x += tmps2;
		}

		s2 = x.low().first();
		return result;
	}

	integer<BITS * 2> multiplyWithCarry(const integer<BITS>& b) const
	{
		const integer<BITS>& a = *this;
		integer<BITS * 2> result;

		integer<BITS / 2>& s0 = result.low().low();
		integer<BITS / 2>& s1 = result.low().high();
		integer<BITS / 2>& s2 = result.high().low();
		integer<BITS / 2>& s3 = result.high().high();

		bool bh = !b.high().empty();
		bool ah = !a.high().empty();

		integer<BITS> x = a.low() * b.low();

		s0 = x.low();

		x = integer<BITS>(x).high();
		if (ah)
		{
			x += b.low();
		}

		s1 = x.low();
		s2 = x.high();

		x = s1;
		if (bh)
		{
			x += a.low();
		}

		s1 = x.low();


		if (ah && bh)
		{
			x = integer<BITS>(x).high();
			x++;
			x += s2;
		}
		else
		{
			x = integer<BITS>(x).high();
			x += s2;
		}

		s2 = x.low();
		s3 = x.high();
		return result;
	}

	integer<BITS> multiplyHalfWithCarry(const integer<BITS>& b) const
	{
		const integer<BITS>& a = *this;
		integer<BITS> result;

		bool bh = !b.high().empty();
		bool ah = !a.high().empty();

		integer<BITS> x = a.low() * b.low();

		result.low() = x.low();

		x = integer<BITS>(x).high();
		if (ah)
		{
			x += b.low();
		}

		result.high() = x.low();

		x = result.high();
		if (bh)
		{
			x += a.low();
		}

		result.high() = x.low();
		return result;
	}

	template<size_t PBITS>
	integer<BITS + PBITS> multiply(const integer<PBITS>& b) const
	{
		integer<BITS + PBITS> temp = 0;

		for (unsigned int i = 0; i < size(); ++i)
		{
			for (unsigned int j = 0; j < b.size(); ++j)
			{
				integer<WORD_BITS * 2> c = integer<WORD_BITS>((*this)[i]) * integer<WORD_BITS>(b[j]);

				c += temp[i + j];

				temp[i + j] = c.low();

				bool carry = addWithCarry(temp[i + j + 1], c.high());

				for (unsigned int z = 2; carry; z++)
				{
					carry = addWithCarry(temp[i + j + z], 1);
				}
			}
		}
		return temp;
	}

	template<size_t PBITS>
	integer<(BITS + PBITS) / 2> multiplyHalf(const integer<PBITS>& b) const
	{
		integer<(BITS + PBITS) / 2> temp = 0;

		for (unsigned int i = 0; i < size(); ++i)
		{
			for (unsigned int j = 0; j < b.size(); ++j)
			{
				if(i + j >= temp.size())
				{
					break;
				}

				integer<WORD_BITS * 2> c = integer<WORD_BITS>((*this)[i]) * integer<WORD_BITS>(b[j]);

				c += temp[i + j];

				temp[i + j] = c.low();

				bool carry = addWithCarry(temp[i + j + 1], c.high());

				for (unsigned int z = 2; carry; z++)
				{
					carry = addWithCarry(temp[i + j + z], 1);
				}
			}
		}
		return temp;
	}

	template<size_t PBITS, size_t P2BITS>
	static integer<BITS + PBITS> multiplyKaratsuba(const integer<P2BITS>& a, const integer<PBITS>& b)
	{
		return a.multiplyKaratsuba(b);
	}

	static integer<sizeof(word) * 8 * 2> multiplyKaratsuba(word a, word b)
	{
		return integer<sizeof(word) * 8>(a) * integer<sizeof(word) * 8>(b);
	}

	template<size_t PBITS>
	integer<BITS + PBITS> multiplyKaratsuba(const integer<PBITS>& b) const
	{
		const integer<BITS>& a = *this;

		const auto& al = a.low();
		const auto& ah = a.high();

		const auto& bl = b.low();
		const auto& bh = b.high();

		integer<BITS / 2> asum = al;
		bool a_carry = asum.addWithCarry(ah);
		integer<PBITS / 2> bsum = bl;
		bool b_carry = bsum.addWithCarry(bh);


		const auto z0 = al * bl;
		const auto z1 = asum.multiplyWithCarry(bsum, a_carry, b_carry);
		const auto z2 = ah * bh;

		integer<BITS + PBITS> result;
		result.high() = z2;
		result.low() = z0;
		result += ((z1 - z2 - z0) << (MAX(BITS, PBITS) / 2));
		return result;
	}

	bool static addWithCarry(word& a, const word b)
	{
		word swap = a;
		a += b;
		return swap > a;
	}

	bool static addWithCarry(bool carry, word& a, const word b)
	{
		word swap = a;
		a += b + carry;
		return swap > a;
	}

	bool static addWithCarry(bool carry, word& a)
	{
		word swap = a;
		a += carry;
		return swap > a;
	}

	bool static subtractWithBorrow(word& output, const word a, const word b)
	{
		output = a - b;
		return b > a;
	}

	integer<BITS>& operator >>= (unsigned long shift)
	{
		word carry;
		if (shift >= WORD_BITS)
		{
			unsigned long wordShift = shift / (WORD_BITS);
			shift %= WORD_BITS;

			long i;

			for (i = 0; i < size() - wordShift; i++)
			{
				(*this)[i] = (*this)[i + wordShift];
			}

			for (; i < size(); i++)
			{
				(*this)[i] = 0;
			}
		}

		if (shift > 0)
		{
			for (long i = 0; i < size(); i++)
			{
				if (i)
				{
					carry = BIT_MASK(shift) & (*this)[i];
					(*this)[i - 1] = (*this)[i - 1] | (carry << (WORD_BITS - shift));
				}
				(*this)[i] = (*this)[i] >> shift;
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

	integer<BITS>& operator<<=(unsigned long shift)
	{
		word carry;

		if (shift >= WORD_BITS)
		{
			unsigned long wordShift = shift / (WORD_BITS);
			shift %= WORD_BITS;

			long i;

			for (i = size() - wordShift - 1; i >= 0; i--)
			{
				(*this)[i + wordShift] = (*this)[i];
			}

			for (i = wordShift - 1; i >= 0; i--)
			{
				(*this)[i] = 0;
			}
		}

		if (shift > 0)
		{
			for (long i = size() - 1; i >= 0; i--)
			{
				if (i != size() - 1)
				{
					carry = ~BIT_MASK(WORD_BITS - shift) & (*this)[i];
					(*this)[i + 1] = (*this)[i + 1] | carry;
				}
				(*this)[i] = (*this)[i] << shift;
			}
		}
		return *this;
	}

	integer<BITS> operator<<(unsigned long shift) const
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
			exp >>= 1;
			if (exp == 0)
			{
				break;
			}
			sequence = (sequence * sequence) % modulus;
		}
		return product;
	}

	template <size_t PBITS>
	integer<PBITS> powmodx(long exp, const integer<PBITS> modulus)
	{
		integer<BITS> product = 1;
		integer<PBITS> sequence = *this % modulus;

		printf("calculating inverse...\n");
		integer<PBITS> inv = this->modularInverse(modulus);
		inv.print();
		printf("done\n\n");

		while (exp != 0)
		{
			if (exp & 1)
			{
				printf("modulus: \n");
				((product * sequence) % modulus).print();
				printf("\ninversion multiply: \n");
				((product * sequence) * inv).print();
				return modulus;
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
			modulus <<= WORD_BITS;

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
			remainder <<= WORD_BITS;

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

		if (divisor.empty())
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
			integer<BITS> mid = (max + min) >> 1;
			auto p = (divisor * mid);
			integer<BITS>  posRem = dividend - p;

			if (p > dividend)
			{
				max = mid - 1;
			}
			else if (posRem >= divisor)
			{
				min = mid + 1;
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

	bool empty() const
	{
		const integer<BITS>& a = *this;
		for (long i = 0; i < size(); i++)
		{
			if (a[i])
			{
				return false;
			}
		}
		return true;
	}

	template<size_t PBITS>
	bool operator==(const integer<PBITS>& n) const
	{
		word i, len = MIN(size(), n.size());
		for (i = 0; i < len; i++)
		{
			word a = read(i);
			word b = n.read(i);
			if (a != b)
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

	integer<BITS> modularInverse(integer<BITS> b) const
	{
		integer<BITS> a = *this;
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
			//x0 = x1 - q * x0;
			sign |= x1.subtractWithBorrow(q * x0);
			x0 = x1;
			//sign |= subtractWithBorrow(x0, x1, q * x0);
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

	template<size_t PBITS>
	integer<BITS> operator+(const integer<PBITS> n) const
	{
		integer<BITS> temp = *this;
		temp += n;

		return temp;
	}

	integer<BITS> operator+(const word n) const
	{
		integer<BITS> temp = *this;
		temp += n;

		return temp;
	}

	template<size_t PBITS>
	integer<BITS>& operator+=(const integer<PBITS>& n)
	{
		addWithCarry(n);

		return *this;
	}

	integer<BITS>& operator+=(const word n)
	{
		addWithCarry(n);
		return *this;
	}

	template<size_t PBITS>
	bool addWithCarry(const integer<PBITS>& n)
	{
		word num = 0;
		bool carry = 0;
		long len = MIN(size(), n.size());
		long i = 0;
		for (; i < len; i++)
		{
			word a = read(i);
			word b = n.read(i);
#ifdef _MSC_VER
			carry = _addcarry_u64(carry, a, b, &num);
#else
			num = a + b + carry;

			if (num < a || num < b)
			{
				carry = 1;
			}
			else
			{
				carry = 0;
			}
#endif

			write(i, num);
		}

		if (!carry)
		{
			return false;
		}

		if (size() > n.size())
		{
			for (; i < size(); i++)
			{
				word a = read(i);
				num = a + 1;
				write(i, num);

				if (num != 0)
				{
					return false;
				}
			}
			return true;
		}
		else
		{
			return true;
		}
	}

	bool addWithCarry(const word n)
	{
		word a = read(0);
		word num = a + n;

		write(0, num);

		if (num >= a)
		{
			return false;
		}

		for (long i = 1; i < size(); i++)
		{
			word a = read(i);
			num = a + 1;
			write(i, num);

			if (num != 0)
			{
				return false;
			}
		}
		return true;
	}

	template<size_t PBITS>
	integer<BITS> operator-(const integer<PBITS>& n) const
	{
		integer<BITS> temp = *this;
		temp -= n;

		return temp;
	}

	integer<BITS> operator-(const word n) const
	{
		integer<BITS> temp = *this;
		temp -= n;

		return temp;
	}

	template<size_t PBITS>
	integer<BITS>& operator-=(const integer<PBITS>& n)
	{
		subtractWithBorrow(n);
		return *this;
	}

	integer<BITS>& operator-=(const word n)
	{
		subtractWithBorrow(n);
		return *this;
	}

	template<size_t PBITS>
	bool subtractWithBorrow(const integer<PBITS>& n)
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
			return borrow;
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
					write(i, num);
				}
				else
				{
					write(i, num);
					return false;
				}
			}
			return true;
		}
		else
		{
			return borrow;
		}
	}

	bool subtractWithBorrow(const word n)
	{
		word a = read(0);
		write(0, a - n);

		if (a > n)
		{
			return false;
		}

		for (long i = 1; i < size(); i++)
		{
			word a = read(i);

			if (a == 0)
			{
				write(i, std::numeric_limits<word>::max());
			}
			else
			{
				write(i, a - 1);
				return false;
			}
		}
		return true;

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

	word& read(unsigned long i)
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

	const constexpr integer<BITS / 2>& low() const
	{
		return reinterpret_cast<const integer<BITS / 2>&>(*this);
	}

	const constexpr integer<BITS / 2>& high() const
	{
		return reinterpret_cast<const integer<BITS / 2>&>(*(buffer + size() / 2));
	}

	integer<BITS / 2>& low()
	{
		return reinterpret_cast<integer<BITS / 2>&>(*this);
	}

	integer<BITS / 2>& high()
	{
		return reinterpret_cast<integer<BITS / 2>&>(*(buffer + size() / 2));
	}

	template<size_t PBITS>
	integer<PBITS / 2>& low()
	{
		return reinterpret_cast<integer<PBITS / 2>&>(*this);
	}

	template<size_t PBITS>
	integer<PBITS / 2>& high()
	{
		return reinterpret_cast<integer<PBITS / 2>&>(*(buffer + (PBITS / WORD_BITS) / 2));
	}

	bool bit(unsigned long i)
	{
		return (((byte*)buffer)[i / 8] >> (i % 8)) & 1;
	}

	constexpr static unsigned long size()
	{
		return BITS / 8 / sizeof(word);
	}
	//private:
	word buffer[BITS / 8 / sizeof(word)];
};

template<size_t BITS, class T, class HT>
class primitive
{
public:
	constexpr primitive() : t() { }
	template <size_t PBITS>
	constexpr primitive(const integer<PBITS>& t) : t(t[0]) { }
	constexpr primitive(const T&t) : t(t) {}

	constexpr operator const T& () const { return t; }
	operator T& () { return t; }
	const T* operator& () const { return &t; }
	T* operator& () { return &t; }

	const constexpr word& first() const { return t; }
	word& first() { return t; }

	const constexpr word& last() const { return t; }
	word& last() { return t; }

	bool addWithCarry(const T& b)
	{
		T swap = t;
		t += b;
		return swap > t;
	}

	/*bool addWithCarry(const integer<BITS>& n)
	{
		return false;
	}*/

	//operator const word*() const { return (const word*)&t; }
	//operator word*() { return (word*)&t; }

	word read(unsigned long i) const
	{
		return t;
	}

	integer<BITS * 2> operator*(const T& s) const
	{
		return multiply(s);
	}

	integer<BITS * 2> multiplyOld(const T& s) const
	{
		integer<BITS * 2> result;
		//HT* temp = (HT*)result.buffer;
		/*HT* a = (HT*)&t;
		HT* b = (HT*)&s;*/

		const HT(&a)[2] = reinterpret_cast<const HT(&)[2]>(t);
		const HT(&b)[2] = reinterpret_cast<const HT(&)[2]>(s);

		for (int i = 0; i < 2; ++i)
		{
			for (int j = 0; j < 2; ++j)
			{
				T c = T(a[i]) * T(b[j]) + RB[i + j];

				RB[i + j] = integer<BITS>(c).low();

				HT swap = RB[i + j + 1];
				RB[i + j + 1] += integer<BITS>(c).high();

				if (swap > RB[i + j + 1])
				{
					RB[i + j + 2]++;
				}
			}
		}
		return result;
	}

	integer<BITS * 2> operator*(const integer<BITS>& b) const
	{
		return multiply(b);
	}

	integer<BITS * 2> multiplyKaratsuba(const integer<BITS>& b) const
	{
		return multiply(b);
	}

	integer<BITS * 2> multiply(const integer<BITS>& b) const
	{
		return multiply(b.t);
	}

	integer<BITS * 2> multiply(const T& b) const
	{
		integer<BITS * 2> result;

#ifdef _MSC_VER
		if (BITS == 64)
		{
			result.low() = _umul128(this->t, b, &result.high());
			return result;
		}
		else if (BITS == 32)
		{
			result = __emulu(this->t, b);
			return result;
		}
#endif

		T s0, s1, s2, s3;

		T x = (T)integer<BITS>(t).low() * integer<BITS>(b).low();
		s0 = integer<BITS>(x).low();

		x = (T)integer<BITS>(t).high() * integer<BITS>(b).low() + integer<BITS>(x).high();
		s1 = integer<BITS>(x).low();
		s2 = integer<BITS>(x).high();

		x = s1 + (T)integer<BITS>(t).low() * integer<BITS>(b).high();
		s1 = integer<BITS>(x).low();

		x = s2 + (T)integer<BITS>(t).high() * integer<BITS>(b).high() + integer<BITS>(x).high();
		s2 = integer<BITS>(x).low();
		s3 = integer<BITS>(x).high();

		result.low() = s1 << 32 | s0;
		result.high() = s3 << 32 | s2;
		return result;
	}

	static integer<BITS * 2> multiply(const T& t, const T& b)
	{
		integer<BITS * 2> result;

		T s0, s1, s2, s3;

		T x = (T)integer<BITS>(t).low() * integer<BITS>(b).low();
		s0 = integer<BITS>(x).low();

		x = (T)integer<BITS>(t).high() * integer<BITS>(b).low() + integer<BITS>(x).high();
		s1 = integer<BITS>(x).low();
		s2 = integer<BITS>(x).high();

		x = s1 + (T)integer<BITS>(t).low() * integer<BITS>(b).high();
		s1 = integer<BITS>(x).low();

		x = s2 + (T)integer<BITS>(t).high() * integer<BITS>(b).high() + integer<BITS>(x).high();
		s2 = integer<BITS>(x).low();
		s3 = integer<BITS>(x).high();

		result.low() = s1 << 32 | s0;
		result.high() = s3 << 32 | s2;
		return result;
	}

	integer<BITS * 2> multiplyWithCarry(const integer<BITS>& b, bool a_carry, bool b_carry) const
	{
		integer<BITS * 2> result;

		HT& s0 = result.low().low();
		HT& s1 = result.low().high();
		HT& s2 = result.high().low();
		HT& s3 = result.high().high();

		integer<BITS * 2> x = (*this) * b;

		s0 = x.low();

		x = integer<BITS>(x).high();
		if (a_carry)
		{
			x += b;
		}

		s1 = x.low();
		s2 = x.high();

		x = s1;
		if (b_carry)
		{
			x += (*this);
		}

		s1 = x.low();


		if (a_carry && b_carry)
		{
			x = integer<BITS>(x).high();
			x++;
			x += s2;
		}
		else
		{
			x = integer<BITS>(x).high();
			x += s2;
		}

		s2 = x.low();
		s3 = x.high();
		return result;
	}

	template<size_t PBITS>
	integer<BITS * 2> multiplyWithCarry(const integer<PBITS>& b, bool a_carry, bool b_carry) const
	{
		integer<BITS * 2> result;
		return result;
	}

	template<size_t PBITS>
	integer<BITS * 2> multiplyWithCarry(const integer<PBITS>& b) const
	{
		integer<BITS * 2> result;
		return result;
	}

	const constexpr HT& low() const { return (HT&)t; }
	const constexpr HT& high() const { return *((HT*)&t + 1); }
	bool empty() const { return t == 0; }

	HT& low() { return (HT&)t; }
	HT& high() { return *((HT*)&t + 1); }

	constexpr long size()
	{
		return 1;
	}

	T t;
};


template<>
class integer<64> : public primitive<64, uint64, uint32>
{
public:
	constexpr integer() : primitive() { }
	template <size_t PBITS>
	constexpr integer(const integer<PBITS>& t) : primitive(t[0]) { }
	constexpr integer(const uint64&t) : primitive(t) {}
};

template<>
class integer<32> : public primitive<32, uint32, uint16>
{
public:
	constexpr integer() : primitive() { }
	constexpr integer(const uint32&t) : primitive(t) {}
	constexpr integer(const word t) : primitive(t) {}
	operator uint32& () { return t; }
	constexpr operator const uint32& () const { return t; }

};

template<>
class integer<16> : public primitive<16, uint16, uint8>
{
public:
	constexpr integer() : primitive() { }
	constexpr integer(const uint16&t) : primitive(t) {}
	constexpr integer(const word t) : primitive(t) {}
	operator uint16& () { return t; }
	constexpr operator const uint16& () const { return t; }
};

template<>
class integer<8> : public primitive<8, uint8, uint8>
{
public:
	constexpr integer() : primitive() { }
	constexpr integer(const uint8&t) : primitive(t) {}
	constexpr integer(const word t) : primitive(t) {}
	operator uint8& () { return t; }
	constexpr operator const uint8& () const { return t; }
};

template <size_t BITS, size_t PBITS>
integer<PBITS> powmod(integer<BITS>& b, long exp, const integer<PBITS>& modulus)
{
	return b.powmod(exp, modulus);
}

template <size_t BITS, size_t PBITS>
integer<PBITS> powmodx(integer<BITS>& b, long exp, const integer<PBITS>& modulus)
{
	return b.powmodx(exp, modulus);
}