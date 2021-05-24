#pragma once
#include <cstdint>
#include <random>
#include <utility>
#include <type_traits>

std::mt19937& GetRand();

// Return value is between [lo, hi]
template <typename T>
T GetRandomInt(T lo, T hi)
{
	// TODO(noff): wchar_t isn't supported by std::uniform_int_distribution
	static_assert(std::is_integral<T>::value, "Only usable with integral types");

	// TODO(noff): Fix callers
	if (lo == hi)
		return lo;

	if (hi < lo)
		std::swap(lo, hi);

	std::uniform_int_distribution<T> d(lo, hi);
	return d(GetRand());
}

// Return value *should be* between [lo, hi)
template <typename T>
T GetRandomFloat(T lo, T hi)
{
	static_assert(std::is_floating_point<T>::value, "Only usable with real types");

	// TODO(noff): Fix callers
	if (lo == hi)
		return lo;

	if (hi < lo)
		std::swap(lo, hi);

	std::uniform_real_distribution<T> d(lo, hi);
	return d(GetRand());
}

// std::pair<I, I> overloads

template <typename T>
inline int32_t GetRandomInt(const std::pair<T, T>& lohi)
{
	return GetRandomInt(lohi.first, lohi.second);
}

template <typename T>
inline float GetRandomFloat(const std::pair<T, T>& lohi)
{
	return GetRandomFloat(lohi.first, lohi.second);
}
