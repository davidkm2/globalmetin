#include "../include/random.h"

std::mt19937& GetRand()
{
	static std::mt19937 rnd;
	return rnd;
}
