// bit_vector.cpp
#include <iostream>
#include <cstdint>
#include <cmath>
#include <cassert>
#include <cstring>
#include <algorithm>
#include <numeric>
#include <iterator>

// For testing.
#include <vector>

#if defined (_MSC_VER)
#include <intrin.h> 
#endif

inline std::int64_t popcount64(std::int64_t x)
{
#if defined(_MSC_VER)
	return __popcnt64(x);
#elif defined(__GNUC__)
	return __builtin_popcountll(x);
#endif
}

inline bool bittest64(std::int64_t x, std::int64_t idx)
{
#if defined(_MSC_VER)
	return _bittest64(&x, idx);
#elif defined(__GNUC__)
	return x & (1 << idx);
#endif
}

class bit_vector
{
public:
	bit_vector(std::int64_t n, std::int64_t init = 0)
		: n_(n),
		blocks_((n_ >> 6ull) + 1),
		arr_(new std::int64_t[blocks_])
	{
		assert(n_ > 0);

		std::fill(arr_, arr_ + blocks_, init);

		const std::int64_t i = n_ % 64;
		const std::int64_t mask = (1ull << i) - 1ull;
		arr_[blocks_ - 1] &= mask;
	}

	bit_vector(const bit_vector& other)
		: n_(other.n_),
		blocks_(other.blocks_),
		arr_(new std::int64_t[other.n_])
	{
		assert(n_ == other.n_);
		assert(blocks_ == other.blocks_);
		
		if (arr_)
		{
			std::copy(other.arr_, other.arr_ + other.blocks_, arr_);
		}

		assert(std::equal(arr_, arr_ + n_, other.arr_));
	}

	bit_vector& operator=(const bit_vector& other)
	{
		bit_vector temp(other);
		temp.swap(*this);
		return *this;
	}

	bit_vector(bit_vector&& other) noexcept
	{
		other.swap(*this);
	}

	bit_vector& operator=(bit_vector&& other) noexcept
	{
		other.swap(*this);
		return *this;
	}

	~bit_vector()
	{
		if (arr_)
		{
			delete[] arr_;
		}
	}

	void swap(bit_vector& other) noexcept
	{
		std::swap(n_, other.n_);
		std::swap(blocks_, other.blocks_);
		std::swap(arr_, other.arr_);
	}

	friend void swap(bit_vector& lhs, bit_vector& rhs)
	{
		lhs.swap(rhs);
	}

	bool get(std::int64_t i) const
	{
		assert(i <= n_);

		const std::int64_t idx = i >> 6ull;
		const std::int64_t pos = i - 64ull * idx;

		return bittest64(arr_[idx], pos);
	}

	void set(std::int64_t i)
	{
		assert(i <= n_);

		const std::int64_t idx = i >> 6ull;
		const std::int64_t pos = i - 64ull * idx;

		arr_[idx] |= (1ull << pos);
	}

	void clear()
	{
		std::memset(arr_, 0, sizeof(std::int64_t) * blocks_);

		assert(0 == count());
	}

	std::int64_t count() const
	{
		std::int64_t count = 0;

		for (std::int64_t i = 0; i < blocks_; ++i)
			count += popcount64(arr_[i]);

		return count;
	}

	template <typename OutputIterator>
	void get_values(OutputIterator out)
	{
		for (std::int64_t i = 0; i < blocks_; ++i)
		{
			std::int64_t adj = arr_[i];

			for (unsigned long j; adj != 0; adj &= ~(1ull << j))
			{
				// TODO: use __builtin_ffs for GCC
				_BitScanForward64(&j, adj);
				*out = (j + 64 * i);
			}
		}
	}

	private:
		std::int64_t n_;
		std::int64_t blocks_;
		std::int64_t* arr_;
};

int main()
{
	bit_vector bv(200);
	bv.set(0); assert(bv.get(0));
	bv.set(16); assert(bv.get(16));
	bv.set(32); assert(bv.get(32));
	bv.set(63); assert(bv.get(63));
	bv.set(64); assert(bv.get(64));
	bv.set(65); assert(bv.get(65));
	bv.set(199); assert(bv.get(199));
	bv.set(200); assert(bv.get(200));

	for (std::int64_t i = 1; i < 4096; ++i)
	{
		bit_vector b(i, 0xFFFFFFFFFFFFFFFF);
		assert(b.count() == i);

		b.clear();
		assert(b.count() == 0);

		bit_vector q(i);
		assert(q.count() == 0);
	}

	std::vector<int> v;
	for (auto i = 0; i < 100; ++i)
		v.push_back(i);

	srand(123456789);
	std::random_shuffle(v.begin(), v.end());

	bit_vector sorter(*std::max_element(v.cbegin(), v.cend()));
	std::for_each(v.cbegin(), v.cend(), [&](int e) { sorter.set(e); });

	std::vector<int> result;
	result.reserve(v.size());
	sorter.get_values(std::back_inserter(result));

	std::sort(v.begin(), v.end());
	assert(std::equal(result.cbegin(), result.cend(), v.cbegin()));

	std::cin.get();
}
