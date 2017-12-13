// bit_vector.cpp
#include <iostream>
#include <cstdint>
#include <cmath>
#include <cassert>
#include <cstring>
#include <algorithm>
#include <iterator>

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
	bit_vector(std::int64_t n)
		: n_(n), 
		blocks_((n_ >> 6ull) + 1), 
		arr_(new std::int64_t[blocks_])
	{
		assert(n_ > 0);
		std::memset(arr_, 0, sizeof(std::int64_t) * blocks_);
	}

	bit_vector(const bit_vector&) = delete;
	bit_vector& operator = (const bit_vector&) = delete;

	~bit_vector()
	{
		delete[] arr_;
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
	const std::int64_t n_;
	const std::int64_t blocks_;
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

	bv.get_values(std::ostream_iterator<int>(std::cout, " "));
}
