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
**    Michael J. Fromberger
*/

// This class is based on the original MPI library (not NSS, because of license restrictions) with some modifications.
// Some ideas and algorithms are from NSS (Netscape Security Suite).  Where they have been used, the function contains a reference note
//
// Note, since September 2011, I believe the MPI homepage is now: http://spinning-yarns.org/michael/mpi/
// The license is as follows
// This software was written by Michael J. Fromberger,
//   http://www.dartmouth.edu/~sting/
//
// See the MPI home page at
//   http://www.dartmouth.edu/~sting/mpi/
//
// This software is in the public domain.  It is entirely free, and you
// may use it and/or redistribute it for whatever purpose you choose;
// however, as free software, it is provided without warranty of any
// kind, not even the implied warranty of merchantability or fitness for
// a particular purpose.

#pragma once

#include "big_int.h"
#include "DataBuffer.h"
#include "Secret.h"
#include "Random.h"

typedef struct
{
	BigInt modulus;
	BigInt public_exponent;
	BigInt private_exponent;
	BigInt prime1;
	BigInt prime2;
	BigInt exponent1;
	BigInt exponent2;
	BigInt coefficient;
}  RSAPrivateKey;

class RSA_Impl
{
public:
	RSA_Impl();

	static DataBuffer encrypt(int32_t block_type, Random &random, const void *in_public_exponent, uint32_t in_public_exponent_size, const void *in_modulus, uint32_t in_modulus_size, const void *in_data, uint32_t in_data_size);
	static Secret decrypt(const Secret &in_private_exponent, const void *in_modulus, uint32_t in_modulus_size, const void *in_data, uint32_t in_data_size);

	/// \brief Create the keypair
	void create(Random &random, int32_t key_size_in_bits, int32_t public_exponent_value);

	/// \brief Create a keypair
	///
	/// \param random = Random number generator
	/// \param out_private_exponent = Private exponent (to decrypt with)
	/// \param out_public_exponent = Public exponent (to encrypt with)
	/// \param out_modulus = Modulus
	/// \param key_size_in_bits = key size in bits
	/// \param public_exponent_value = public exponent value
	void create_keypair(Random &random, Secret &out_private_exponent, DataBuffer &out_public_exponent, DataBuffer &out_modulus, int32_t key_size_in_bits, int32_t public_exponent_value);

private:
	void generate_prime(Random &random, BigInt &prime, int32_t prime_len);
	bool build_from_primes(BigInt *p, BigInt *q, BigInt *e, BigInt *d, uint32_t key_size_in_bits);

	static void rsaep(BigInt *msg, const BigInt *e, const BigInt *modulus, BigInt *cipher);
	static void rsadp(BigInt *cipher, const BigInt *d, const BigInt *modulus, BigInt *msg);

	// PKCS#1 v.1.5 message padding and encoding
	// msg       - input message
	// mlen      - length of input message, in bytes
	// emsg      - output buffer
	// emlen     - length of output buffer
	//
	static void pkcs1v15_encode(int32_t block_type, Random &random, const char *msg, int32_t mlen, char *emsg, int32_t emlen);

	// PKCS#1 v1.5 padded message decoding
	// emsg      - encoded message
	// emlen     - length of encoded message, in bytes
	static Secret pkcs1v15_decode(const char *emsg, int32_t emlen);

	// Encrypt a message using RSA and PKCS#1 v.1.5 padding
	// msg       - input message
	// mlen      - length of input message, in bytes
	// e         - encryption exponent
	// modulus   - encryption key modulus
	static DataBuffer pkcs1v15_encrypt(int32_t block_type, Random &random, const char *msg, int32_t mlen, const BigInt *e, const BigInt *modulus);

	// Decrypt a message using RSA and PKCS#1 v.1.5 padding
	// msg       - input message (ciphertext)
	// mlen      - length of input message, in bytes
	// d         - decryption exponent
	// modulus   - decryption key modulus
	static Secret pkcs1v15_decrypt(const char *msg, int32_t mlen, const BigInt *d, const BigInt *modulus);

	RSAPrivateKey rsa_private_key;
};


class RSA
{
public:
	/// \brief Create a keypair
	///
	/// \param random = Random number generator
	/// \param out_private_exponent = Private exponent (to decrypt with)
	/// \param out_public_exponent = Public exponent (to encrypt with)
	/// \param out_modulus = Modulus
	/// \param key_size_in_bits = key size in bits
	/// \param public_exponent_value = public exponent value
	static void create_keypair(Random &random, Secret &out_private_exponent, DataBuffer &out_public_exponent, DataBuffer &out_modulus, int32_t key_size_in_bits = 1024, int32_t public_exponent_value = 65537);

	/// \brief Encrypt
	///
	/// \param block_type = 0 (private key), 1 (private key) or 2 (public key)
	/// \param random = Random number generator
	/// \param in_public_exponent = Public exponent
	/// \param in_modulus = Modulus
	/// \param in_data = Data to encrypt (maximum length is in_modulus.get_size() - 11)
	/// \return Encrypted data
	static DataBuffer encrypt(int32_t block_type, Random &random, const DataBuffer &in_public_exponent, const DataBuffer &in_modulus, const Secret &in_data);

	/// \brief Encrypt
	///
	/// \param block_type = 0 (private key), 1 (private key) or 2 (public key)
	/// \param random = Random number generator
	/// \param in_public_exponent = Public exponent
	/// \param in_public_exponent_size = size in bytes of in_public_exponent
	/// \param in_modulus = Modulus
	/// \param in_modulus_size = size in bytes of in_modulus
	/// \param in_data = Data to encrypt 
	/// \param in_data_size = size in bytes of in_data (maximum size is in_modulus_size - 11)
	/// \return Encrypted data
	static DataBuffer encrypt(int32_t block_type, Random &random, const void *in_public_exponent, uint32_t in_public_exponent_size, const void *in_modulus, uint32_t in_modulus_size, const void *in_data, uint32_t in_data_size);

	/// \brief Decrypt
	///
	/// Warning: An exception may be thrown when decrypting if in_data is not valid.
	/// Be careful handling this, to prevent "timing attacks"
	///
	/// \param in_private_exponent = Private exponent
	/// \param in_modulus = Modulus
	/// \param in_data = Data to decrypt (length equals in_modulus.get_size())
	/// \return Decrypted data
	static Secret decrypt(const Secret &in_private_exponent, const DataBuffer &in_modulus, const DataBuffer &in_data);

	/// \brief Decrypt
	///
	/// Warning: An exception may be thrown when decrypting if in_data is not valid.
	/// Be careful handling this, to prevent "timing attacks"
	///
	/// \param in_private_exponent = Private exponent
	/// \param in_modulus = Modulus
	/// \param in_modulus_size = size in bytes of in_modulus
	/// \param in_data = Data to encrypt 
	/// \param in_data_size = size in bytes of in_data (length equals in_modulus_size)
	/// \return Decrypted data
	static Secret decrypt(const Secret &in_private_exponent, const void *in_modulus, uint32_t in_modulus_size, const void *in_data, uint32_t in_data_size);
};

