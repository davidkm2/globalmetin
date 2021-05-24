#pragma once

#include "../../libthecore/include/stdafx.h"

#ifndef _WIN32
#include <semaphore.h>
#else
#define isdigit iswdigit
#define isspace iswspace
#endif

#include <memory>

#include "../../common/length.h"
#include "../../common/tables.h"
#include "../../common/singleton.h"
#include "../../common/utils.h"
#include "../../common/stl.h"
#include "../../common/defines.h"

#include <list>
#include <map>
#include <string>
#include <vector>
#include <memory>
