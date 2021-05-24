#ifndef __GAME_SRC_STDAFX_H__
#define __GAME_SRC_STDAFX_H__

#define M2_NEW new
#define M2_DELETE(p) delete (p)
#define M2_DELETE_ARRAY(p) delete[] (p)

template<typename T>
T* get_pointer (T* p)
{
	return p;
}

#include "../../libthecore/include/stdafx.h"

#include "../../common/singleton.h"
#include "../../common/utils.h"
#include "../../common/defines.h"

#include <algorithm>
#include <math.h>
#include <list>
#include <map>
#include <set>
#include <queue>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <float.h>

#include "typedef.h"
#include "locale.hpp"
#include "event.h"

#define PASSES_PER_SEC(sec) ((sec) * passes_per_sec)

#ifndef M_PI
#define M_PI    3.14159265358979323846 /* pi */
#endif
#ifndef M_PI_2
#define M_PI_2  1.57079632679489661923 /* pi/2 */
#endif

#define IN
#define OUT

#endif

