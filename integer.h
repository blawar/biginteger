#pragma once
#define PLUS	1
#define MINUS	-1

typedef unsigned char byte;

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

	integer<BITS>(const byte* n)
	{
		for (int i = 0; i < size(); i++)
		{
			buffer[i] = n[i];
		}
	}

	integer<BITS>(integer<BITS>& n)
	{
		memcpy(buffer, (const byte*)n, size());
	}

	operator byte*() { return buffer; }
	integer<BITS>& operator=(int n)
	{
		memset(buffer, 0, size());
		for (int i = 0; i < sizeof(n) && i < size(); i++)
		{
			buffer[size() - i - 1] = ((byte*)&n)[i];
		}
		return *this;
	}

	integer<BITS>& operator*=(integer<BITS> n)
	{
		integer<BITS> temp = 0;
		integer<BITS> i = 0;
		while(i < n)
		{
			temp += *this;
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
			byte a = read(i);
			byte b = n.read(i);
			if (a < b)
			{
				return true;
			}
		}
		return false;
	}

	integer<BITS> operator+(integer<BITS> n)
	{
		integer temp(n);
		temp += n;

		return temp;
	}
	integer<BITS> operator+=(integer<BITS> n)
	{
		int num = 0;
		int carry = 0;
		for (int i = 0; i < size(); i++)
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

	byte read(unsigned long i)
	{
		return buffer[size() - i - 1];
	}

	void write(unsigned long i, byte value)
	{
		buffer[size() - i - 1] = value;
	}

	void print()
	{
		for (unsigned long i = 0; i < size(); i++)
		{
			if (i % 8 == 0 && i != 0)
				printf("\n");
			printf("%2.2x ", buffer[i]);
		}
		printf("\n");
	}

	unsigned long size() const
	{
		return BITS / 8;
	}
private:
	byte buffer[BITS / 8];
};