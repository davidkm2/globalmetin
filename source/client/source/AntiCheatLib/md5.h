#pragma once

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
**    Mark Page
*/

#include "sha.h"
#include <memory>
#include <string>

class DataBuffer;

class MD5_Impl : private SHA
{
public:
	MD5_Impl();

	std::string get_hash(bool uppercase = false) const;
	void get_hash(uint8_t out_hash[16]) const;
	void reset();
	void add(const void *data, int32_t size);
	void calculate();
	void set_hmac(const void *key_data, int32_t key_size);

private:
	void process_chunk();

	const static int32_t block_size = 64;

	inline uint32_t leftrotate_uint32(uint32_t value, int32_t shift) const
	{
		return (value << shift) | (value >> (32 - shift));
	}

	uint32_t h0, h1, h2, h3;
	uint8_t chunk[block_size];
	int32_t chunk_filled;
	uint32_t length_message;
	bool calculated;

	bool hmac_enabled;
	uint8_t hmac_key_chunk[block_size];
};

class MD5
{
public:
	/// \brief Constructs a MD5 hash generator.
	MD5();

	static const int32_t hash_size = 16;

	/// \brief Returns the calculated hash.
	std::string get_hash(bool uppercase = false) const;

	/// \brief Get hash
	///
	/// \param out_hash = where to write to
	void get_hash(uint8_t out_hash[hash_size]) const;

	/// \brief Resets the hash generator.
	void reset();

	/// \brief Enable a HMAC based calculation
	///
	/// Call this function before the initial add(). It is reset by reset()
	///
	/// \param key_data = The HMAC key
	/// \param key_size = The size of the key_data
	void set_hmac(const void *key_data, int32_t key_size);

	/// \brief Adds data to be hashed.
	void add(const void *data, int32_t size);

	/// \brief Add
	///
	/// \param data = Data Buffer
	void add(const DataBuffer &data);

	/// \brief Finalize hash calculation.
	void calculate();

private:
	std::shared_ptr<MD5_Impl> impl;
};
