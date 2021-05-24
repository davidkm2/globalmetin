/*
**  ClanLib SDK
**  Copyright (c) 1997-2016 The ClanLib Team
**
**  This software is provided 'as-is', without any express or implied
**  warranty.  In no event will the authors be held liable for any damages
**  arising from the use of this software.
**
**  Permission is granted to anyone to use this software for any purpose,
**  including commercial applications, and to alter it and redistribute it
**  freely, subject to the following restrictions:
**
**  1. The origin of this software must not be misrepresented; you must not
**     claim that you wrote the original software. If you use this software
**     in a product, an acknowledgment in the product documentation would be
**     appreciated but is not required.
**  2. Altered source versions must be plainly marked as such, and must not be
**     misrepresented as being the original software.
**  3. This notice may not be removed or altered from any source distribution.
**
**  Note: Some of the libraries ClanLib may link to may have additional
**  requirements or restrictions.
**
**  File Author(s):
**
**    Magnus Norddahl
*/

#include "stdafx.h"
#include "cl_endian.h"

void Endian::swap(void *data, int32_t type_size, int32_t total_times)
{
	if (type_size == 1) return;

	uint8_t *d = (uint8_t *)data;

	for (int32_t j = 0; j < total_times; j++)
	{
		for (int32_t i = 0; i < type_size / 2; i++)
		{
			uint8_t a = d[i];
			d[i] = d[type_size - 1 - i];
			d[type_size - 1 - i] = a;
		}

		d += type_size;
	}
}

bool Endian::is_system_big()
{
	const int32_t i = 1;
	return !(*(char *)(&i));
}

bool Endian::is_system_64bit()
{
	return (sizeof(int32_t*) == 8);
}

