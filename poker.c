#include "arrays.h"
#include "poker.h"

short max(short a, short b)
{
    return (a > b ? a : b);
}

unsigned short count_bits(unsigned int v)
{
    unsigned short c;
    for (c = 0; v; ++c) v &= v - 1;
    return c;
}

short count_same(int s, int *ranks) {
    short c = 0;
    for (int i = 0; i < 5; i++)
    {
        if (s == ranks[i]) c++;
    }
    return c;
}

short eval_pairness(int r1, int r2, int r3, int r4, int r5)
{
    int ranks[] = { r1, r2, r3, r4, r5 };
    short m1 = 0, m2 = 0;
    for (int i = 0; i < 13; i++)
    {
        short c = count_same(1 << i, ranks);
        if (c == 4) return 5;
        if (c > m1)
        {
            m2 = m1;
            m1 = c;
        }
        else if (c > m2)
        {
            m2 = c;
        }
        if (m1 == 3 && m2 == 2) return 4;
        if (m1 == 2 && m2 == 2) return 2;
    }
    return m1 == 3 ? 3 : m1 - 1;
}

short eval_flushiness(int s1, int s2, int s3, int s4)
{
    short c1 = count_bits(s1);
    short c2 = count_bits(s2);
    short c3 = count_bits(s3);
    short c4 = count_bits(s4);
    short m = max(c1, max(c2, max(c3, c4)));
    if (m > 2) return m;
    if (m == 2)
    {
        short pm = max(c1 * c2, max(c1 * c3, max(c1 * c4, max(c2 * c3, max(c2 * c4, c3 * c4)))));
        return pm / 2;
    }
    return 0;
}

short eval_straightness(int q)
{
    // add ace before deuce also
    q = (q << 1) | (q >> 12);
    short s = 0;
    for (int i = 0; i < 10; i++)
    {
        s = max(s, straigtness[(q >> i) & 0x1f]);
        if (s == 5) return 5;
    }
    return s;
}

//   +--------+--------+--------+--------+
//   |xxxbbbbb|bbbbbbbb|cdhsrrrr|xxpppppp|
//   +--------+--------+--------+--------+
//   p = prime number of rank (deuce=2,trey=3,four=5,five=7,...,ace=41)
//   r = rank of card (deuce=0,trey=1,four=2,five=3,...,ace=12)
//   cdhs = suit of card
//   b = bit turned on depending on rank of card
short eval_board(int c1, int c2, int c3, int c4, int c5)
{
    short p = eval_pairness(c1 >> 16, c2 >> 16, c3 >> 16, c4 >> 16, c5 >> 16);
    short f = eval_flushiness(
      ((c1 >> 12) & 1) | ((c2 >> 11) & 2) | ((c3 >> 10) & 4) | ((c4 >>  9) & 8) | ((c5 >>  8) & 16),
      ((c1 >> 13) & 1) | ((c2 >> 12) & 2) | ((c3 >> 11) & 4) | ((c4 >> 10) & 8) | ((c5 >>  9) & 16),
      ((c1 >> 14) & 1) | ((c2 >> 13) & 2) | ((c3 >> 12) & 4) | ((c4 >> 11) & 8) | ((c5 >> 10) & 16),
      ((c1 >> 15) & 1) | ((c2 >> 14) & 2) | ((c3 >> 13) & 4) | ((c4 >> 12) & 8) | ((c5 >> 11) & 16));
    short s = eval_straightness((c1 | c2 | c3 | c4 | c5) >> 16);
    return (s << 6) | (f << 3) | p;
}

// 0 = 0000
// 1 = 0001
// 2 = 0010
// 3 = 0011
// 4 = 0100
// 5 = 0101
// 6 = 0110
// 7 = 0111
// 8 = 1000
// 9 = 1001
// a = 1010
// b = 1011
// c = 1100
// d = 1101
// e = 1110
// f = 1111