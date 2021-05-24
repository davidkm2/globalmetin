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
// Some ideas and algorithms are from NSS (Netscape Security Suite). Where they have been used, the function contains a reference note
//
// Note, since September 2011, I believe the MPI homepage is now: http://spinning-yarns.org/michael/mpi/
// Note, since 2013, the MPI page no longer exists, however the internet archive has the details here: http://web.archive.org/web/20100426001741/http://spinning-yarns.org/michael/mpi/README
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

#include <cstdint>
#include <memory>
#include <vector>

class BigInt_Impl
{
public:
	BigInt_Impl(uint32_t prec = default_allocated_precision);
	BigInt_Impl(const BigInt_Impl &other);
	~BigInt_Impl();

	void read_unsigned_octets(const uint8_t *input_str, uint32_t input_length);
	void zero();
	bool make_prime(uint32_t num_bits);
	int32_t cmp_z() const;
	void set_bit(uint32_t bit_number, uint32_t value);
	int32_t significant_bits() const;
	void sieve(const uint32_t *primes, uint32_t num_primes, std::vector<uint8_t> &sieve);
	uint32_t mod_d(uint32_t d) const;
	void div_d(uint32_t d, BigInt_Impl *q, uint32_t *r) const;
	void copy(BigInt_Impl *to) const;
	void add_d(uint32_t d, BigInt_Impl *out_b) const;
	bool fermat(uint32_t w) const;
	bool pprime(int32_t nt) const;
	void set(uint32_t d);
	void set(int32_t d);
	void set(uint64_t d);
	void set(int64_t d);
	void get(uint32_t &d);
	void get(int32_t &d);
	void get(uint64_t &d);
	void get(int64_t &d);
	void exptmod(const BigInt_Impl *b, const BigInt_Impl *m, BigInt_Impl *c) const;
	void mod(const BigInt_Impl *m, BigInt_Impl *c) const;
	void div(const BigInt_Impl *b, BigInt_Impl *q, BigInt_Impl *r) const;
	void add(const BigInt_Impl *b, BigInt_Impl *c) const;
	void sub(const BigInt_Impl *b, BigInt_Impl *c) const;
	int32_t cmp(const BigInt_Impl *b) const;
	int32_t cmp_d(uint32_t d) const;
	void sub_d(uint32_t d, BigInt_Impl *b) const;
	void neg(BigInt_Impl *b) const;
	uint32_t trailing_zeros() const;
	void div_2d(uint32_t d, BigInt_Impl *q, BigInt_Impl *r) const;
	void sqrmod(const BigInt_Impl *m, BigInt_Impl *c) const;
	void sqr(BigInt_Impl *b) const;
	void random();
	void exch(BigInt_Impl *mp2);
	void mul(const BigInt_Impl *b, BigInt_Impl *c) const;
	void mul_d(uint32_t d, BigInt_Impl *c) const;
	bool invmod(const BigInt_Impl *m, BigInt_Impl *c) const;
	void xgcd(const BigInt_Impl *b, BigInt_Impl *g, BigInt_Impl *x, BigInt_Impl *y) const;
	void abs(BigInt_Impl *b) const;
	int32_t iseven() const;
	int32_t isodd() const;
	void div_2(BigInt_Impl *c) const;
	void to_unsigned_octets(uint8_t *output_str, uint32_t output_length) const;
	uint32_t unsigned_octet_size() const;

	static const int32_t default_allocated_precision;

private:
	static const int32_t num_bits_in_digit = (8 * sizeof(uint32_t));
	static const int32_t num_bits_in_word = (8 * sizeof(uint64_t));
	static const uint64_t digit_radix = 1ULL << (8 * sizeof(uint32_t));
	static const uint32_t digit_half_radix = 1U << (8 * sizeof(uint32_t) - 1);
	static const uint64_t word_maximim_value = ~0;

	static const int32_t prime_tab_size = 6542;
	static std::vector<uint32_t> prime_tab;

	static void build_primes();

	static inline uint32_t internal_carryout(uint64_t w)
	{
		return w >> num_bits_in_digit;
	}

	static inline uint32_t internal_accum(uint64_t w)
	{
		return (uint32_t)w;
	}

	void internal_init_size(uint32_t prec);
	void internal_free();
	void internal_lshd(uint32_t p);
	void internal_pad(uint32_t min);
	void internal_grow(uint32_t min);
	void internal_clamp();
	int32_t internal_ispow2d(uint32_t d) const;
	void internal_div_2d(uint32_t d);
	void internal_rshd(uint32_t p);
	int32_t  internal_ispow2() const;
	void internal_mod_2d(uint32_t d);
	void internal_mul_2d(uint32_t d);

	// Exchange the data for a and b; (b, a) = (a, b)
	void internal_exch(BigInt_Impl *b);

	// Compare |a| <=> d, return 0 if equal, <0 if a<d, >0 if a>d
	int32_t internal_cmp_d(uint32_t d) const;

	void internal_div_d(uint32_t d, uint32_t *r);

	void internal_add_d(uint32_t d);
	void internal_sub_d(uint32_t d);
	uint32_t internal_norm(BigInt_Impl *b);
	void internal_sub(const BigInt_Impl *b);

	int32_t internal_cmp(const BigInt_Impl *b) const;
	void internal_div(BigInt_Impl *b);
	void internal_mul_d(uint32_t d);
	void internal_add(const BigInt_Impl *b);
	void internal_mul(const BigInt_Impl *b);
	void internal_div_2();
	void internal_mul_2();

	void internal_reduce(const BigInt_Impl *m, BigInt_Impl *mu);
	void internal_sqr();

	bool digits_negative;	// True if the value is negative
	uint32_t digits_alloc;		// How many digits allocated
	uint32_t digits_used;		// How many digits used
	uint32_t *digits;	// The digits themselves

	BigInt_Impl &operator=(const BigInt_Impl& other) = delete;	// Not defined
};

/// \brief Big Integer class
class BigInt
{
public:
	/// \brief Constructs a big integer (initialised to zero)
	BigInt();

	/// \brief Constructs a big integer (initialised to value)
	explicit BigInt(uint32_t value);

	/// \brief Constructs a big integer (initialised to value)
	explicit BigInt(int32_t value);

	/// \brief Constructs a big integer (initialised to value)
	explicit BigInt(uint64_t value);

	/// \brief Constructs a big integer (initialised to value)
	explicit BigInt(int64_t value);

	/// \brief Copy constructor
	BigInt(const BigInt &other);

	/// \brief Destructor
	~BigInt();

	BigInt &operator=(const BigInt& other);

	void read_unsigned_octets(const uint8_t *input_str, uint32_t input_length);

	void zero();

	bool make_prime(uint32_t num_bits);

	/// \brief  Compare a <=> 0.  Returns <0 if a<0, 0 if a=0, >0 if a>0.
	int32_t cmp_z() const;

	void set_bit(uint32_t bit_number, uint32_t value);

	int32_t significant_bits() const;

	void sieve(const uint32_t *primes, uint32_t num_primes, std::vector<uint8_t> &sieve);

	/// \brief  Compute c = a (mod d).  Result will always be 0 <= c < d
	uint32_t mod_d(uint32_t d) const;

	/// \brief  Compute the quotient q = a / d and remainder r = a mod d, for a single digit d.  Respects the sign of its divisor (single digits are uint32_t anyway).
	void div_d(uint32_t d, BigInt *q, uint32_t *r) const;

	/// \brief  Using w as a witness, try pseudo-primality testing based on Fermat's little theorem. 
	///
	/// If a is prime, and (w, a) = 1, then w^a == w (mod a).
	/// So, we compute z = w^a (mod a) and compare z to w; if they are
	/// equal, the test passes and we return true.  Otherwise, we return false.
	bool fermat(uint32_t w) const;

	/// \brief  Performs nt iteration of the Miller-Rabin probabilistic primality test on a.
	///
	/// Returns true if the tests pass, false if one fails.
	/// If false is returned, the number is definitely composite.  If true
	bool pprime(int32_t nt) const;

	/// \brief Sets a value
	void set(int32_t d);
	void set(uint32_t d);
	void set(uint64_t d);
	void set(int64_t d);

	/// \brief Gets a value.
	///
	/// Throws exception if number exceeds datatype bounds
	void get(uint32_t &d);
	void get(uint64_t &d);
	void get(int64_t &d);
	void get(int32_t &d);

	/// \brief  Compute c = (a ** b) mod m.
	///
	/// Uses a standard square-and-multiply
	/// method with modular reductions at each step. (This is basically the
	/// same code as expt(), except for the addition of the reductions)
	///
	/// The modular reductions are done using Barrett's algorithm (see reduce() for details)
	void exptmod(const BigInt *b, const BigInt *m, BigInt *c) const;

	/// \brief  Compute c = a (mod m).  Result will always be 0 <= c < m.
	void mod(const BigInt *m, BigInt *c) const;

	/// \brief  Compute q = a / b and r = a mod b.
	///
	/// Input parameters may be re-used
	/// as output parameters.  If q or r is nullptr, that portion of the
	/// computation will be discarded (although it will still be computed)
	void div(const BigInt &b, BigInt *q, BigInt *r) const;
	void div(uint32_t d, BigInt *q, BigInt *r) const;

	/// \brief Compute result = this + b.
	BigInt operator + (const BigInt& b);
	BigInt operator + (uint32_t d);

	/// \brief Compute this += b.
	BigInt operator += (const BigInt& b);
	BigInt operator += (uint32_t d);

	/// \brief Compute result = this - b.
	BigInt operator - (const BigInt& b);
	BigInt operator - (uint32_t d);

	/// \brief Compute this -= b.
	BigInt operator -= (const BigInt& b);
	BigInt operator -= (uint32_t d);

	/// \brief Compute result = this * b.
	BigInt operator * (const BigInt& b);
	BigInt operator * (uint32_t d);

	/// \brief Compute this *= b.
	BigInt operator *= (const BigInt& b);
	BigInt operator *= (uint32_t d);

	/// \brief Compute result = this / b.
	BigInt operator / (const BigInt& b);
	BigInt operator / (uint32_t d);

	/// \brief Compute this /= b.
	BigInt operator /= (const BigInt& b);
	BigInt operator /= (uint32_t d);

	/// \brief Compute result = this % b.
	BigInt operator % (const BigInt& b);
	BigInt operator % (uint32_t d);

	/// \brief Compute this %= b.
	BigInt operator %= (const BigInt& b);
	BigInt operator %= (uint32_t d);

	int32_t cmp(const BigInt *b) const;

	/// \brief  Compare a <=> d.  Returns <0 if a<d, 0 if a=d, >0 if a>d
	int32_t cmp_d(uint32_t d) const;

	/// \brief  Compute b = -a.  'a' and 'b' may be identical.
	void neg(BigInt *b) const;

	uint32_t trailing_zeros() const;

	void sqrmod(const BigInt *m, BigInt *c) const;
	void sqr(BigInt *b) const;

	/// \brief Assigns a random value to a.
	///
	/// This value is generated using the
	/// standard C library's rand() function, so it should not be used for
	/// cryptographic purposes, but it should be fine for primality testing,
	/// since all we really care about there is reasonable statistical
	/// properties.
	/// As many digits as a currently has are filled with random digits.
	void random();

	/// \brief Exchange mp1 and mp2 without allocating any intermediate memory
	///
	/// (well, unless you count the stack space needed for this call and the
	/// locals it creates...).  This cannot fail.
	void exch(BigInt *mp2);

	/// \brief  Compute c = a^-1 (mod m), if there is an inverse for a (mod m).
	///
	/// This is equivalent to the question of whether (a, m) = 1.  If not,
	/// false is returned, and there is no inverse.
	bool invmod(const BigInt *m, BigInt *c) const;

	/// \brief  Compute g = (a, b) and values x and y satisfying Bezout's identity
	///
	/// (that is, ax + by = g).  This uses the extended binary GCD algorithm
	/// based on the Stein algorithm used for mp_gcd()
	void xgcd(const BigInt *b, BigInt *g, BigInt *x, BigInt *y) const;

	/// \brief   Compute b = |a|.  'a' and 'b' may be identical.
	void abs(BigInt *b) const;

	///  \brief  Returns a true if number is even
	bool is_even() const;

	///  \brief  Returns a true if number is odd
	bool is_odd() const;

	/// \brief  Compute c = a / 2, disregarding the remainder.
	void div_2(BigInt *c) const;

	void to_unsigned_octets(uint8_t *output_str, uint32_t output_length) const;

	uint32_t unsigned_octet_size() const;

private:
	std::unique_ptr<BigInt_Impl> impl;
};

