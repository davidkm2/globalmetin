#include "StdAfx.h"

#include <assert.h>

static unsigned long randseed = 1;

void srandom(unsigned long seed)
{
	randseed = seed;
}

unsigned long random()
{
	register long x, hi, lo, t;

	x = randseed;
	hi = x / 127773;
	lo = x % 127773;
	t = 16807 * lo - 2836 * hi;
	if (t <= 0)
		t += 0x7fffffff;
	randseed = t;
	return (t);
}
float frandom(float flLow, float flHigh)
{
	float fl = float(random()) / float(2147483648.0f); 
	return (fl * (flHigh - flLow)) + flLow; 
}

long random_range(long from, long to)
{
	assert(from <= to);
	return ((random() % (to - from + 1)) + from);
}
