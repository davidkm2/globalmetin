#pragma once

#include "../EterLib/StdAfx.h"
#include "../EterGrnLib/StdAfx.h"

// python include
extern "C"
{
#ifdef AT
	#undef AT
#endif

#include <Python/Python.h>

#ifdef AT
	#undef AT
#endif
}

#include "PythonUtils.h"
#include "PythonLauncher.h"
#include "Resource.h"

void initdbg();
