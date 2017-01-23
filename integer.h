#pragma once
#define MAXDIGITS 1
#define PLUS	1
#define MINUS	-1

typedef unsigned char byte;
class integer
{
public:
	integer();
	integer(int n);
	integer(const byte* n);
	integer(integer& n);

	operator byte*() { return buffer; }
	integer& operator=(int n);
	integer& operator*=(integer n);
	integer operator+(integer n);
	integer operator+=(integer n);
	integer& operator++();

	byte read(unsigned long i) { return buffer[i]; }
	void write(unsigned long i, byte value) { buffer[i] = value; }
	void print();
private:
	byte buffer[MAXDIGITS];
};