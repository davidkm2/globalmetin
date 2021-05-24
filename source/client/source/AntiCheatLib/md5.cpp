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

#include "stdafx.h"
#include "md5.h"
#include "DataBuffer.h"
#include "Exception.h"

#ifndef WIN32
#include <cstring>
#endif

#include <exception>
#include <algorithm>

MD5_Impl::MD5_Impl()
{
	reset();
}

std::string MD5_Impl::get_hash(bool uppercase) const
{
	if (calculated == false)
		throw Exception("MD5 hash has not been calculated yet!");

	char digest[33];
	to_hex_le(digest, h0, uppercase);
	to_hex_le(digest + 8, h1, uppercase);
	to_hex_le(digest + 16, h2, uppercase);
	to_hex_le(digest + 24, h3, uppercase);
	digest[32] = 0;

	return digest;
}

void MD5_Impl::get_hash(uint8_t out_hash[16]) const
{
	if (calculated == false)
		throw Exception("MD5 hash has not been calculated yet!");

	out_hash[0] = (uint8_t)(h0 & 0xff);
	out_hash[1] = (uint8_t)((h0 >> 8) & 0xff);
	out_hash[2] = (uint8_t)((h0 >> 16) & 0xff);
	out_hash[3] = (uint8_t)((h0 >> 24) & 0xff);
	out_hash[4] = (uint8_t)(h1 & 0xff);
	out_hash[5] = (uint8_t)((h1 >> 8) & 0xff);
	out_hash[6] = (uint8_t)((h1 >> 16) & 0xff);
	out_hash[7] = (uint8_t)((h1 >> 24) & 0xff);
	out_hash[8] = (uint8_t)(h2 & 0xff);
	out_hash[9] = (uint8_t)((h2 >> 8) & 0xff);
	out_hash[10] = (uint8_t)((h2 >> 16) & 0xff);
	out_hash[11] = (uint8_t)((h2 >> 24) & 0xff);
	out_hash[12] = (uint8_t)(h3 & 0xff);
	out_hash[13] = (uint8_t)((h3 >> 8) & 0xff);
	out_hash[14] = (uint8_t)((h3 >> 16) & 0xff);
	out_hash[15] = (uint8_t)((h3 >> 24) & 0xff);

}

void MD5_Impl::reset()
{
	//  http://tools.ietf.org/html/rfc1321 section 3.3
	h0 = 0x67452301;
	h1 = 0xEFCDAB89;
	h2 = 0x98BADCFE;
	h3 = 0x10325476;
	memset(chunk, 0, block_size);
	chunk_filled = 0;
	length_message = 0;
	calculated = false;
	hmac_enabled = false;
}

void MD5_Impl::add(const void *_data, int32_t size)
{
	if (calculated)
		reset();

	const uint8_t *data = (const uint8_t *)_data;
	int32_t pos = 0;
	while (pos < size)
	{
		int32_t data_left = size - pos;
		int32_t buffer_space = block_size - chunk_filled;
		int32_t data_used = std::min(buffer_space, data_left);
		memcpy(chunk + chunk_filled, data + pos, data_used);
		chunk_filled += data_used;
		pos += data_used;
		if (chunk_filled == block_size)
		{
			process_chunk();
			chunk_filled = 0;
		}
	}
	length_message += size;
}

void MD5_Impl::set_hmac(const void *key_data, int32_t key_size)
{
	memset(hmac_key_chunk, 0, block_size);

	int32_t key_chunk_filled;

	if (key_size > block_size)
	{
		MD5 md5;
		md5.add(key_data, key_size);
		md5.calculate();
		key_chunk_filled = MD5::hash_size;
		md5.get_hash(hmac_key_chunk);
	}
	else
	{
		memcpy(hmac_key_chunk, key_data, key_size);
		key_chunk_filled = key_size;
	}

	for (auto & elem : hmac_key_chunk)	// XOR key with inner pad values
	{
		elem ^= 0x36;
	}

	add(hmac_key_chunk, block_size);	// Add the inner HMAC

	for (auto & elem : hmac_key_chunk)	// XOR key with outer pad values
	{
		elem ^= 0x36;	// Undo the inner pad
		elem ^= 0x5c;
	}
	hmac_enabled = true;	// This has to be after the add(), as that function may call reset()

}

void MD5_Impl::calculate()
{
	if (calculated)
		reset();

	// append a single "1" bit to message
	// append "0" bits until message length ? 448 ? -64 (mod 512)
	// append length of message, in bits as 64-bit little-endian integer to message

	uint8_t end_data[block_size * 2];
	memset(end_data, 0, block_size * 2);
	end_data[0] = 128;

	int32_t size = block_size - chunk_filled;
	if (size < 9)
		size += block_size;

	uint64_t length_message64 = length_message * (uint64_t)8;

	uint32_t length_upper = (uint32_t)(length_message64 >> 32);
	uint32_t length_lower = (uint32_t)(length_message64 & 0xffffffff);

	end_data[size - 8] = (length_lower & 0x000000ff);
	end_data[size - 7] = (length_lower & 0x0000ff00) >> 8;
	end_data[size - 6] = (length_lower & 0x00ff0000) >> 16;
	end_data[size - 5] = (length_lower & 0xff000000) >> 24;
	end_data[size - 4] = (length_upper & 0x000000ff);
	end_data[size - 3] = (length_upper & 0x0000ff00) >> 8;
	end_data[size - 2] = (length_upper & 0x00ff0000) >> 16;
	end_data[size - 1] = (length_upper & 0xff000000) >> 24;

	add(end_data, size);

	if (chunk_filled != 0)
		throw Exception("Error in MD5_Impl class. Still chunk data at end of calculate");

	calculated = true;

	if (hmac_enabled)
	{
		uint8_t temp_hash[MD5::hash_size];
		get_hash(temp_hash);
		reset();
		add(hmac_key_chunk, block_size);	// Add the outer HMAC
		add(temp_hash, MD5::hash_size);
		calculate();
	}
}

void MD5_Impl::process_chunk()
{
	int32_t i;
	uint32_t w[16];

	for (i = 0; i < 16; i++)
	{
		uint32_t b1 = chunk[i * 4 + 0];
		uint32_t b2 = chunk[i * 4 + 1];
		uint32_t b3 = chunk[i * 4 + 2];
		uint32_t b4 = chunk[i * 4 + 3];
		w[i] = (b4 << 24) | (b3 << 16) | (b2 << 8) | b1;
	}

	static const uint8_t r[64] =
	{
		// Round 0
		7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22,

		// Round 1
		5, 9, 14, 20, 5, 9, 14, 20, 5, 9, 14, 20, 5, 9, 14, 20,

		// Round 2
		4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23,

		// Round 3
		6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21
	};

	static const uint32_t k[64] =
	{
		// for i from 0 to 63
		//	k[i] := floor(abs(sin(i + 1)) � (2 pow 32))
		// end for
		0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee,
		0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
		0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be,
		0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,
		0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa,
		0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
		0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed,
		0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,
		0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c,
		0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
		0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05,
		0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
		0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039,
		0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
		0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1,
		0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391
	};

	uint32_t a = h0;
	uint32_t b = h1;
	uint32_t c = h2;
	uint32_t d = h3;

	for (i = 0; i < 64; i++)
	{
		uint32_t f, g;
		if (i < 16)
		{
			f = d ^ (b & (c ^ d));	// f = (b & c) | ((~b) & d);
			g = i;
		}
		else if (i < 32)
		{
			f = c ^ (d & (b ^ c));	// f = (d & b) | ((~d) & c);
			g = (5 * i + 1) & 0xF;
		}
		else if (i < 48)
		{
			f = b ^ c ^ d;
			g = (3 * i + 5) & 0xF;
		}
		else
		{
			f = c ^ (b | (~d));
			g = (7 * i) & 0xF;
		}

		uint32_t temp = d;
		d = c;
		c = b;
		b = b + leftrotate_uint32(a + f + k[i] + w[g], r[i]);
		a = temp;
	}

	h0 += a;
	h1 += b;
	h2 += c;
	h3 += d;
}

MD5::MD5()
	: impl(std::make_shared<MD5_Impl>())
{
}

std::string MD5::get_hash(bool uppercase) const
{
	return impl->get_hash(uppercase);
}

void MD5::get_hash(uint8_t out_hash[16]) const
{
	impl->get_hash(out_hash);
}

void MD5::reset()
{
	impl->reset();
}

void MD5::add(const void *data, int32_t size)
{
	impl->add(data, size);
}

void MD5::add(const DataBuffer &data)
{
	add(data.get_data(), data.get_size());
}

void MD5::calculate()
{
	impl->calculate();
}

void MD5::set_hmac(const void *key_data, int32_t key_size)
{
	impl->set_hmac(key_data, key_size);
}

