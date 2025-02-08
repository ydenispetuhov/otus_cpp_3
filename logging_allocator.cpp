#ifndef __PRETTY_FUNCTION__
#include "pretty.h"
#endif

#include <iostream>
#include <map>
#include <vector>

#define USE_PRETTY 1

template <typename T>
struct logging_allocator
{
	using value_type = T;

	using pointer = T *;
	using const_pointer = const T *;
	using reference = T &;
	using const_reference = const T &;

	template <typename U>
	struct rebind
	{
		using other = logging_allocator<U>;
	};

	logging_allocator() = default;
	~logging_allocator() = default;

	template <typename U>
	logging_allocator(const logging_allocator<U> &)
	{
	}

	T *allocate(std::size_t n)
	{
#ifndef USE_PRETTY
		std::cout << "allocate: [n = " << n << "]" << std::endl;
#else
		std::cout << __PRETTY_FUNCTION__ << "[n = " << n << "]" << std::endl;
#endif
		auto p = std::malloc(n * sizeof(T));
		if (!p)
			throw std::bad_alloc();
		return reinterpret_cast<T *>(p);
	}

	void deallocate(T *p, std::size_t n)
	{
#ifndef USE_PRETTY
		std::cout << "deallocate: [n  = " << n << "] " << std::endl;
#else
		std::cout << __PRETTY_FUNCTION__ << "[n = " << n << "]" << std::endl;
#endif
		std::free(p);
	}

	template <typename U, typename... Args>
	void construct(U *p, Args &&...args)
	{
#ifndef USE_PRETTY
		std::cout << "construct" << std::endl;
#else
		std::cout << __PRETTY_FUNCTION__ << std::endl;
#endif
		new (p) U(std::forward<Args>(args)...);
	};

	template <typename U>
	void destroy(U *p)
	{
#ifndef USE_PRETTY
		std::cout << "destroy" << std::endl;
#else
		std::cout << __PRETTY_FUNCTION__ << std::endl;
#endif
		p->~U();
	}
};

int main(int, char *[])
{
	auto v = std::vector<int, logging_allocator<int>>{};
	v.reserve(5);
	for (int i = 0; i < 6; ++i)
	{
		std::cout << "vector size = " << v.size() << std::endl;
		v.emplace_back(i);
		std::cout << std::endl;
	}

	auto m = std::map<
		int,
		float,
		std::less<int>,
		logging_allocator<
			std::pair<
				const int, float>>>{};

	for (int i = 0; i < 1; ++i)
	{
		m[i] = static_cast<float>(i);
	}

	return 0;
}
