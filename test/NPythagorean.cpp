// 毕达哥拉斯三元数组 C++ 20 Ranges 风格

// A sample standard C++20 program that prints the first N Pythagorean triples.

#include <iostream>
#include <optional>

#include <ranges>

#include <time.h>
#include <stdio.h>



using namespace std;

// maybe_view defines a view over xero or one objects.
template<Semiregular T>
struct maybe_view : view_interface<maybe_view<T>>
{
	maybe_view() = default;
	maybe_view(T t) : data_(std::move(t))
	{

	}

	T const *begin() const noexcept
	{
		return data_ ? &*data_ : nullptr;
	}

	T const *end() const noexcept
	{
		return data_ ? &*data_ + 1 : nullptr;
	}

private:
	optional<T> data_{};
};

// "for_each" creates a new view by applying a transformation to each element in an input range,
// and flattening the resulting range of ranges.
// (This uses one syntax for constrained lambdas in C++20.)
inline constexpr auto for_each = 
	[]<Range R,
		iterator I = iterator_t<R>,
		IndirectUnaryInvocable<I> Fun>(R&& r, Fun fun)
		requires Range<indirect_reault_t<Fun, I>>
	{
		return std::forward<R>(r) | view::transform(std::move(fun)) | view::join;
	};

// "yield_if" takes a bool and a value and returns a view of zero or one elements.
inline constexpr auto yield_if =
[]<Semiregular T>(bool b, T x)
{
	return b ? maybe_view{std::move(x)}
	: maybe_view<T>{};
};

int main()
{
	// Define an infinite range of all the Pythagorean triples;
	using view::itoa;
	auto triples =
		for_each(itoa(1), {
			return for_each(itoa(1, z + 1), = {
					return for_each(itoa(x, z + 1), = {
						return yield_if(x*x + y * y == z * z,
							make_tuple(x, y, z));
						});
				});
			});

	// Display the first 10 triples
	for (auto triple : triples | view::take(10))
	{
		cout << '('
			<< get<0>(triple) << ','
			<< get<1>(triple) << ','
			<< get<2>(triple) << ')'
			<< '\n';
	}
}

// C 函数打印第一个 N Pythagorean Triples
void printNTriples(int n)
{
	int i = 0;
	for(int z=1;;++z)
		for(int x=1;x<=z;++x)
			for(int y=x;y<=z;++y)
				if (x*x + y * y == z * z)
				{
					printf("%d, %d, %d\n", x, y, z);
					if (++i == n)
						return;
				}
}

// 打印前 100 个三元数组
int main()
{
	clock_t t0 = clock();

	int i = 0;
	for(int z = 1; ; ++z)
		for(int x = 1; x <= z; ++x)
			for(int y = x; y <= z; ++y)
				if (x*x + y * y == z * z)
				{
					printf("(%i,%i,%i\n", x, y, z);
					if (++i == 100)
						goto done;
				}

done:
	clock_t t1 = clock();
	printf("%ims\n", (int)(t1 - t0) * 1000 / CLOCKS_PER_SEC);
	return 0;
}

// 编译 Debug 版本 clang simplest.cpp -o outsimplest
// 优化的 Release 版本构建：clang simplest.cpp -o outsimplest -O2

