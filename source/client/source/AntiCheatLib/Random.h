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
**    Mark Page
*/

#pragma once

#ifdef _WIN32
#include <Windows.h>
#include <wincrypt.h>
#endif

#include <memory>

class DataBuffer;

class Random_Impl
{
public:
	Random_Impl(int32_t cache_size);
	~Random_Impl();

	void get_random_bytes(uint8_t *out_dest_ptr, int32_t num_bytes);
	void get_random_bytes_nzero(uint8_t *out_dest_ptr, int32_t num_bytes);
	bool get_random_bool();

private:
	void fill_random_pool();

	int32_t random_pool_size;
	int32_t random_pool_free;
	uint8_t *random_pool;

#ifdef WIN32
	HCRYPTPROV hProvider;
#endif

	uint8_t random_bool;
	int32_t random_bool_bits_free;
};


class Random
{
public:
	/// \brief Constructs the object
	///
	/// cache_size = Random number cache size
	Random(int32_t cache_size = 1024);

	/// \brief Get the random bytes
	///
	/// out_dest_ptr = Where to write to
	/// num_bytes = Number of bytes to copy
	void get_random_bytes(uint8_t *out_dest_ptr, int32_t num_bytes);

	/// \brief Get the random bytes excluding bytes containing 0
	///
	/// out_dest_ptr = Where to write to
	/// num_bytes = Number of bytes to copy
	void get_random_bytes_nzero(uint8_t *out_dest_ptr, int32_t num_bytes);

	/// \brief Get the random boolean
	///
	/// out_dest_ptr = Where to write to
	/// num_bytes = Number of bytes to copy
	bool get_random_bool();

private:
	std::shared_ptr<Random_Impl> impl;
};