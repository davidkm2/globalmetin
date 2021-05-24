#include "stdafx.h"
#include "../../common/pool.h"
#include "affect.h"

ObjectPool<CAffect> affect_pool;

CAffect* CAffect::Acquire()
{
	return affect_pool.Construct();
}

void CAffect::Release(CAffect* p)
{
	affect_pool.Destroy(p);
}
