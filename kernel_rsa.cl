
typedef struct {
    char digits[MAXDIGITS];         /* represent the number */
    int signbit;            /* 1 if positive, -1 if negative */
    int lastdigit;          /* index of high-order digit */
} bignum;

void zero_justify(bignum *n);
void subtract_bignum(bignum *a, bignum *b, bignum *c);

void print_bignum(bignum *n)
{
    int i;
    
    if (n->signbit == MINUS) printf("- ");
    for (i=n->lastdigit; i>=0; i--)
        printf("%c",'0'+ n->digits[i]);
    
    printf("\n");
}

void int_to_bignum(int s, bignum *n)
{
    int i;              /* counter */
    int t;              /* int to work with */
    
    if (s >= 0) n->signbit = PLUS;
    else n->signbit = MINUS;
    
    for (i=0; i<MAXDIGITS; i++) n->digits[i] = (char) 0;
    
    n->lastdigit = -1;
    
    t = abs(s);
    
    while (t > 0) {
        n->lastdigit ++;
        n->digits[ n->lastdigit ] = (t % 10);
        t = t / 10;
    }
    
    if (s == 0) n->lastdigit = 0;
}

void initialize_bignum(bignum *n)
{
    int_to_bignum(0,n);
}

int scan_for_start_str(char* num_str)
{
    int places = MAXDIGITS;
    for (int i = 0; i < MAXDIGITS; i++)
    {
        if (num_str[i] != 0) { places = i; }
    }
    return places;
}

void initialize_bignum_with_str(char* num_str, bignum *big_num)
{
    int_to_bignum(0, big_num);
    for (int i = 0; i < MAXDIGITS; i++) { big_num->digits[i] = num_str[i]; }
    big_num->lastdigit = scan_for_start_str(num_str);
}

int lmax(int a, int b)
{
    if (a > b) return(a); else return(b);
}

/*  c = a +-/ * b;   */

void add_bignum(bignum *a, bignum *b, bignum *c)
{
    int carry;          /* carry digit */
    int i;              /* counter */
    
    initialize_bignum(c);
    
    if (a->signbit == b->signbit) c->signbit = a->signbit;
    
    c->lastdigit = lmax(a->lastdigit,b->lastdigit)+1;
    carry = 0;
    
    for (i=0; i<=(c->lastdigit); i++) {
        c->digits[i] = (char) (carry+a->digits[i]+b->digits[i]) % 10;
        carry = (carry + a->digits[i] + b->digits[i]) / 10;
    }
    
    zero_justify(c);
}


void subtract_bignum(bignum *a, bignum *b, bignum *c)
{
    int borrow;         /* has anything been borrowed? */
    int v;              /* placeholder digit */
    int i;              /* counter */
    
    initialize_bignum(c);
    
    c->lastdigit = lmax(a->lastdigit,b->lastdigit);
    borrow = 0;
    
    for (i=0; i<=(c->lastdigit); i++) {
        v = (a->digits[i] - borrow - b->digits[i]);
        if (a->digits[i] > 0)
            borrow = 0;
        if (v < 0) {
            v = v + 10;
            borrow = 1;
        }
        
        c->digits[i] = (char) v % 10;
    }
    
    zero_justify(c);
}

int compare_bignum(bignum *a, bignum *b)
{
    int i;              /* counter */
    
    if ((a->signbit == MINUS) && (b->signbit == PLUS)) return(PLUS);
    if ((a->signbit == PLUS) && (b->signbit == MINUS)) return(MINUS);
    
    if (b->lastdigit > a->lastdigit) return (PLUS * a->signbit);
    if (a->lastdigit > b->lastdigit) return (MINUS * a->signbit);
    
    for (i = a->lastdigit; i>=0; i--) {
        if (a->digits[i] > b->digits[i]) return(MINUS * a->signbit);
        if (b->digits[i] > a->digits[i]) return(PLUS * a->signbit);
    }
    
    return(0);
}

void zero_justify(bignum *n)
{
    while ((n->lastdigit > 0) && (n->digits[ n->lastdigit ] == 0))
        n->lastdigit --;
    
    if ((n->lastdigit == 0) && (n->digits[0] == 0))
        n->signbit = PLUS;  /* hack to avoid -0 */
}


void digit_shift(bignum *n, int d)      /* multiply n by 10^d */
{
    int i;              /* counter */
    
    if ((n->lastdigit == 0) && (n->digits[0] == 0)) return;
    
    for (i=n->lastdigit; i>=0; i--)
        n->digits[i+d] = n->digits[i];
    
    for (i=0; i<d; i++) n->digits[i] = 0;
    
    n->lastdigit = n->lastdigit + d;
}



void multiply_bignum(bignum *a, bignum *b, bignum *c)
{
    bignum row;         /* represent shifted row */
    bignum tmp;         /* placeholder bignum */
    int i,j;            /* counters */
    
    initialize_bignum(c);
    
    row = *a;
    
    for (i=0; i<=b->lastdigit; i++) {
        for (j=1; j<=b->digits[i]; j++) {
            add_bignum(c,&row,&tmp);
            *c = tmp;
        }
        digit_shift(&row,1);
    }
    
    c->signbit = a->signbit * b->signbit;
    
    zero_justify(c);
}


bignum divide_bignum(bignum *a, bignum *b, bignum *c) //returns modulus
{
    bignum row;                     /* represent shifted row */
    bignum tmp;                     /* placeholder bignum */
    int asign, bsign;       /* temporary signs */
    int i,j;                        /* counters */
    
    initialize_bignum(c);
    
    c->signbit = a->signbit * b->signbit;
    
    asign = a->signbit;
    bsign = b->signbit;
    
    a->signbit = PLUS;
    b->signbit = PLUS;
    
    initialize_bignum(&row);
    initialize_bignum(&tmp);
    
    c->lastdigit = a->lastdigit;
    
    for (i=a->lastdigit; i>=0; i--) {
        digit_shift(&row,1);
        row.digits[0] = a->digits[i];
        c->digits[i] = 0;
        while (compare_bignum(&row,b) != PLUS) {
            c->digits[i] ++;
            subtract_bignum(&row,b,&tmp);
            row = tmp;
        }
    }
    
    zero_justify(c);
    
    a->signbit = asign;
    b->signbit = bsign;
    
    return row;
}

void bignum_pow(bignum *num, int exp)
{
    bignum r = *num;
    bignum t1 = *num, t2;
    for (int i = 0; i < exp; i++)
    {
        if (i % 2 == 0) { multiply_bignum(&t1, &r, &t2); }
        if (i % 2 == 1) { multiply_bignum(&t2, &r, &t1); }
    }
    if (exp % 2 == 1) { *num = t1; }
    if (exp % 2 == 0) { *num = t2; }
}

bignum bignum_fme_modpow(bignum *ia, bignum *ib, bignum *imod) /* (ia^ib) % imod */
{
	bignum b = *ib;
	bignum a = *ia;
	bignum mod = *imod;
	bignum product, sequence, nul, tmp, zero, div2;

	int_to_bignum(0, &zero);
	int_to_bignum(2, &div2);
	int_to_bignum(1, &product);
	sequence = divide_bignum(&a, &mod, &nul);

	while (true)
	{
		int last_digit = (int)b.digits[0];
		if (last_digit&1)
		{
			multiply_bignum(&product, &sequence, &tmp);
			product = divide_bignum(&tmp, &mod, &nul);
		}
		multiply_bignum(&sequence, &sequence, &tmp);
		sequence = divide_bignum(&tmp, &mod, &nul);
		divide_bignum(&b, &div2, &tmp); // Emulates >>1
		b = tmp;
		if ((b.digits[0]) == 0 && b.lastdigit == 0) { break; }
	}

	return product;
}

__kernel void rsa_cypher(__global bignum *p, __global bignum *q, __global bignum *M, __global bignum *result, int e)
{
    bignum lp,lq,n,c,cmod,one,e_bignum;
    lp = *p;
    lq = *q;

    int_to_bignum(e, &e_bignum);
    int_to_bignum(1, &one);
    
    multiply_bignum(&lp, &lq, &n);
    
    c = *M;
    cmod = bignum_fme_modpow(&c, &e_bignum, &n);

    *result = cmod;
}
