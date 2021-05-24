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
#include "sha256.h"
#include "DataBuffer.h"
#include "Exception.h"

#ifndef WIN32
#include <cstring>
#endif

#include <exception>
#include <algorithm>

SHA256_Impl::SHA256_Impl(cl_sha_type new_sha_type) : sha_type(new_sha_type)
{
	reset();
}

std::string SHA256_Impl::get_hash(bool uppercase) const
{
	if (calculated == false)
		throw Exception("SHA-256 hash has not been calculated yet!");

	char digest[32 * 2 + 1];
	to_hex_be(digest, h0, uppercase);
	to_hex_be(digest + 8, h1, uppercase);
	to_hex_be(digest + 16, h2, uppercase);
	to_hex_be(digest + 24, h3, uppercase);
	to_hex_be(digest + 32, h4, uppercase);
	to_hex_be(digest + 40, h5, uppercase);
	to_hex_be(digest + 48, h6, uppercase);

	if (sha_type == cl_sha_224)
	{
		digest[56] = 0;
	}
	else
	{
		// Must be cl_sha_256
		to_hex_be(digest + 56, h7, uppercase);
		digest[64] = 0;
	}
	return digest;
}

void SHA256_Impl::get_hash(uint8_t *out_hash) const
{
	if (calculated == false)
		throw Exception("SHA-256 hash has not been calculated yet!");

	out_hash[0] = (uint8_t)((h0 >> 24) & 0xff);
	out_hash[1] = (uint8_t)((h0 >> 16) & 0xff);
	out_hash[2] = (uint8_t)((h0 >> 8) & 0xff);
	out_hash[3] = (uint8_t)(h0 & 0xff);
	out_hash[4] = (uint8_t)((h1 >> 24) & 0xff);
	out_hash[5] = (uint8_t)((h1 >> 16) & 0xff);
	out_hash[6] = (uint8_t)((h1 >> 8) & 0xff);
	out_hash[7] = (uint8_t)(h1 & 0xff);
	out_hash[8] = (uint8_t)((h2 >> 24) & 0xff);
	out_hash[9] = (uint8_t)((h2 >> 16) & 0xff);
	out_hash[10] = (uint8_t)((h2 >> 8) & 0xff);
	out_hash[11] = (uint8_t)(h2 & 0xff);
	out_hash[12] = (uint8_t)((h3 >> 24) & 0xff);
	out_hash[13] = (uint8_t)((h3 >> 16) & 0xff);
	out_hash[14] = (uint8_t)((h3 >> 8) & 0xff);
	out_hash[15] = (uint8_t)(h3 & 0xff);
	out_hash[16] = (uint8_t)((h4 >> 24) & 0xff);
	out_hash[17] = (uint8_t)((h4 >> 16) & 0xff);
	out_hash[18] = (uint8_t)((h4 >> 8) & 0xff);
	out_hash[19] = (uint8_t)(h4 & 0xff);
	out_hash[20] = (uint8_t)((h5 >> 24) & 0xff);
	out_hash[21] = (uint8_t)((h5 >> 16) & 0xff);
	out_hash[22] = (uint8_t)((h5 >> 8) & 0xff);
	out_hash[23] = (uint8_t)(h5 & 0xff);
	out_hash[24] = (uint8_t)((h6 >> 24) & 0xff);
	out_hash[25] = (uint8_t)((h6 >> 16) & 0xff);
	out_hash[26] = (uint8_t)((h6 >> 8) & 0xff);
	out_hash[27] = (uint8_t)(h6 & 0xff);

	if (sha_type == cl_sha_256)
	{
		out_hash[28] = (uint8_t)((h7 >> 24) & 0xff);
		out_hash[29] = (uint8_t)((h7 >> 16) & 0xff);
		out_hash[30] = (uint8_t)((h7 >> 8) & 0xff);
		out_hash[31] = (uint8_t)(h7 & 0xff);
	}	// Else cl_sha_224
}

void SHA256_Impl::reset()
{
	if (sha_type == cl_sha_224)
	{
		h0 = 0xc1059ed8;
		h1 = 0x367cd507;
		h2 = 0x3070dd17;
		h3 = 0xf70e5939;
		h4 = 0xffc00b31;
		h5 = 0x68581511;
		h6 = 0x64f98fa7;
		h7 = 0xbefa4fa4;
	}
	else if (sha_type == cl_sha_256)
	{
		// These words were obtained by taking the first thirty-two bits of the fractional parts of the square roots of the first eight prime numbers
		h0 = 0x6a09e667;
		h1 = 0xbb67ae85;
		h2 = 0x3c6ef372;
		h3 = 0xa54ff53a;
		h4 = 0x510e527f;
		h5 = 0x9b05688c;
		h6 = 0x1f83d9ab;
		h7 = 0x5be0cd19;
	}
	else
	{
		throw Exception("SHA256_Impl, unsupported sha type for 256 bits");
	}

	memset(chunk, 0, block_size);
	chunk_filled = 0;
	length_message = 0;
	calculated = false;
	hmac_enabled = false;
}

void SHA256_Impl::add(const void *_data, int32_t size)
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
	length_message += size * (uint64_t)8;
}

void SHA256_Impl::set_hmac(const void *key_data, int32_t key_size)
{
	memset(hmac_key_chunk, 0, block_size);

	int32_t key_chunk_filled;

	if (key_size > block_size)
	{
		if (sha_type == cl_sha_256)
		{
			SHA256 sha256;
			sha256.add(key_data, key_size);
			sha256.calculate();
			key_chunk_filled = SHA256::hash_size;
			sha256.get_hash(hmac_key_chunk);
		}
		else
		{
			throw Exception("SHA256_Impl, unsupported sha type for hmac");
		}
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

void SHA256_Impl::calculate()
{
	if (calculated)
		reset();

	// append a single "1" bit to message
	// append "0" bits until message length ? 448 ? -64 (mod 512)
	// append length of message, in bits as 64-bit big-endian integer to message

	uint8_t end_data[block_size * 2];
	memset(end_data, 0, block_size * 2);
	end_data[0] = 128;

	int32_t size = block_size - chunk_filled;
	if (size < 9)
		size += block_size;

	uint32_t length_upper = (uint32_t)(length_message >> 32);
	uint32_t length_lower = (uint32_t)(length_message & 0xffffffff);

	end_data[size - 8] = (length_upper & 0xff000000) >> 24;
	end_data[size - 7] = (length_upper & 0x00ff0000) >> 16;
	end_data[size - 6] = (length_upper & 0x0000ff00) >> 8;
	end_data[size - 5] = (length_upper & 0x000000ff);
	end_data[size - 4] = (length_lower & 0xff000000) >> 24;
	end_data[size - 3] = (length_lower & 0x00ff0000) >> 16;
	end_data[size - 2] = (length_lower & 0x0000ff00) >> 8;
	end_data[size - 1] = (length_lower & 0x000000ff);

	add(end_data, size);

	if (chunk_filled != 0)
		throw Exception("Error in SHA256_Impl class. Still chunk data at end of calculate");

	calculated = true;

	if (hmac_enabled)
	{
		int32_t hash_size;
		if (sha_type == cl_sha_256)
		{
			hash_size = SHA256::hash_size;
		}
		else
		{
			throw Exception("SHA512_Impl, unsupported sha type for hmac");
		}

		uint8_t temp_hash[SHA256::hash_size];
		get_hash(temp_hash);
		reset();
		add(hmac_key_chunk, block_size);	// Add the outer HMAC
		add(temp_hash, hash_size);
		calculate();
	}
}

void SHA256_Impl::process_chunk()
{
	// Constants defined in FIPS 180-3, section 4.2.2
	static const uint32_t constant_K[64] = {
		0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b,
		0x59f111f1, 0x923f82a4, 0xab1c5ed5, 0xd807aa98, 0x12835b01,
		0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7,
		0xc19bf174, 0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
		0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da, 0x983e5152,
		0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147,
		0x06ca6351, 0x14292967, 0x27b70a85, 0x2e1b2138, 0x4d2c6dfc,
		0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
		0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819,
		0xd6990624, 0xf40e3585, 0x106aa070, 0x19a4c116, 0x1e376c08,
		0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f,
		0x682e6ff3, 0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
		0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
	};

	int32_t i;
	uint32_t w[64];

	for (i = 0; i < 16; i++)
	{
		uint32_t b1 = chunk[i * 4];
		uint32_t b2 = chunk[i * 4 + 1];
		uint32_t b3 = chunk[i * 4 + 2];
		uint32_t b4 = chunk[i * 4 + 3];
		w[i] = (b1 << 24) + (b2 << 16) + (b3 << 8) + b4;
	}

	for (i = 16; i < 64; i++)
	{
		w[i] = sigma_rr17_rr19_sr10(w[i - 2]) + w[i - 7] + sigma_rr7_rr18_sr3(w[i - 15]) + w[i - 16];
	}

	uint32_t a = h0;
	uint32_t b = h1;
	uint32_t c = h2;
	uint32_t d = h3;
	uint32_t e = h4;
	uint32_t f = h5;
	uint32_t g = h6;
	uint32_t h = h7;

	for (i = 0; i < 64; i++)
	{
		uint32_t t1, t2;

		t1 = h + sigma_rr6_rr11_rr25(e) + sha_ch(e, f, g) + constant_K[i] + w[i];
		t2 = sigma_rr2_rr13_rr22(a) + sha_maj(a, b, c);
		h = g;
		g = f;
		f = e;
		e = d + t1;
		d = c;
		c = b;
		b = a;
		a = t1 + t2;
	}

	h0 += a;
	h1 += b;
	h2 += c;
	h3 += d;
	h4 += e;
	h5 += f;
	h6 += g;
	h7 += h;
}


SHA256::SHA256()
	: impl(std::make_shared<SHA256_Impl>(cl_sha_256))
{
}

std::string SHA256::get_hash(bool uppercase) const
{
	return impl->get_hash(uppercase);
}

void SHA256::get_hash(uint8_t out_hash[32]) const
{
	impl->get_hash(out_hash);
}

void SHA256::reset()
{
	impl->reset();
}

void SHA256::add(const void *data, int32_t size)
{
	impl->add(data, size);
}

void SHA256::add(const DataBuffer &data)
{
	add(data.get_data(), data.get_size());
}

void SHA256::calculate()
{
	impl->calculate();
}

void SHA256::set_hmac(const void *key_data, int32_t key_size)
{
	impl->set_hmac(key_data, key_size);
}
