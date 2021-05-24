#pragma once

#include <vector>
#include <string>
#include <map>
#include <list>
#include <functional>
#include <stack>
#include <set>
#include <cstdint>
#include <algorithm>

#ifdef MIN
	#undef MIN
#endif

#ifdef MAX
	#undef MAX
#endif

#ifdef MINMAX
	#undef MINMAX
#endif

#ifndef itertype
#define itertype(v) decltype((v).begin())
#endif

#ifdef _WIN32
#pragma warning(push) 
#pragma warning(disable: 4242 4244)
#endif // _WIN32

inline void stl_lowers(std::string& rstRet)
{
	for (size_t i = 0; i < rstRet.length(); ++i)
		rstRet[i] = tolower(rstRet[i]);
}

#ifdef _WIN32
#pragma warning(push) 
#endif // _WIN32

inline std::vector <std::string> string_split(const std::string & str, const std::string & tok = " ")
{
	std::vector <std::string> vec;

	std::size_t prev = 0;
	auto cur = str.find(tok);
	while (cur != std::string::npos)
	{
		vec.emplace_back(str.substr(prev, cur - prev));
		prev = cur + tok.size();
		cur = str.find(tok, prev);
	}

	vec.emplace_back(str.substr(prev, cur - prev));
	return vec;
}

inline void string_replace(std::string& str, const std::string& from, const std::string& to)
{
	if (from.empty())
		return;

	std::size_t pos = 0;
	while ((pos = str.find(from, pos)) != std::string::npos)
	{
		str.replace(pos, from.length(), to);
		pos += to.length();
	}
}

struct stringhash
{
	size_t operator () (const std::string & str) const
	{
		const unsigned char * s = (const unsigned char*) str.c_str();
		const unsigned char * end = s + str.size();
		size_t h = 0;

		while (s < end)
		{
			h *= 16777619;
			h ^= *(s++);
		}

		return h;
	}
};

// code from tr1/functional_hash.h
template<typename T>
struct hash;

template<class ArgumentType, class ResultType>
struct custom_unary_function
{
    typedef ArgumentType argument_type;
    typedef ResultType result_type;
};

template<typename _Tp>
struct hash<_Tp*> :
	public custom_unary_function<_Tp*, std::size_t>
{
	std::size_t operator()(_Tp* __p) const
	{
		return reinterpret_cast<std::size_t>(__p);
	}
};

namespace std
{
	template <class container, class Pred>
	void erase_if (container & a, typename container::iterator first, typename container::iterator past, Pred pred)
	{
		while (first != past)
		{
			if (pred(*first))
				a.erase(first++);
			else
				++first;
		}
	}

	template <class container>
	void wipe(container & a)
	{
		typename container::iterator first, past;

		first = a.begin();
		past = a.end();

		while (first != past)
			delete *(first++);

		a.clear();
	}

	template <class container>
	void wipe_second(container & a)
	{
		typename container::iterator first, past;

		first = a.begin();
		past = a.end();

		while (first != past)
		{
			delete first->second;
			++first;
		}

		a.clear();
	}

	template <typename T>
	T MIN(T a, T b)
	{
		return a < b ? a : b;
	}

	template <typename T>
	T MAX(T a, T b)
	{
		return a > b ? a : b;
	}

	template <typename T>
	T MINMAX(T min, T value, T max)
	{
		T tv;

		tv = (min > value ? min : value);
		return (max < tv) ? max : tv;
	}

	template <class _Ty>
	class void_mem_fun_t : public custom_unary_function<_Ty *, void>
	{
		public:
			explicit void_mem_fun_t(void (_Ty::*_Pm)()) : _Ptr(_Pm)
			{
			}

			void operator()(_Ty* p) const
			{
				((p->*_Ptr)());
			}

		private:
			void (_Ty::*_Ptr)();
	};

	template <class _Ty> inline
	void_mem_fun_t<_Ty> void_mem_fun(void (_Ty::*_Pm)())
	{
		return (void_mem_fun_t<_Ty>(_Pm));
	}

	template <class _Ty>
	class void_mem_fun_ref_t : public custom_unary_function<_Ty, void>
	{
		public:
			explicit void_mem_fun_ref_t(void (_Ty::*_Pm)()) : 
				_Ptr(_Pm)
			{
			}

			void operator()(_Ty& x) const
			{
				return ((x.*_Ptr)());
			}

		private:
			void (_Ty::*_Ptr)();
	};

	template <class _Ty> inline
	void_mem_fun_ref_t<_Ty> void_mem_fun_ref(void (_Ty::*_Pm)())
	{
		return (void_mem_fun_ref_t< _Ty>(_Pm));
	}
};
