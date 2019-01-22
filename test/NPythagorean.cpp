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

// 加入代码调用并返回下一个三元数组
struct pytriples
{
	pytriples() : x(1), y(1), z(1) {}

	void next()
	{
		do 
		{
			if (y <= z)
				++y;
			else
			{
				if (x <= z)
					++x;
				else
				{
					x = 1;
					++z;
				}
				y = x;
			}
		} while (x*x+y*y != z*z);
	}

	int x, y, z;
};

int main()
{
	clock_t t0 = clock();

	pytriples py;
	for (int c = 0; c < 100; ++c)
	{
		py.next();
		printf("(%i,%i,%i)\n", py.x, py.y, py.z);
	}

	clock_t t1 = clock();
	printf("%ims\n", (int)(t1 - t0) * 1000 / CLOCKS_PER_SEC);
	return 0;
}

// 伪  C++ coroutine 实现三元组生成器
generator<std::tuple<int, int, int>> pytriples()

{

	for (int z = 1; ; ++z)

		for (int x = 1; x <= z; ++x)

			for (int y = x; y <= z; ++y)

				if (x*x + y * y == z * z)

					co_yield std::make_tuple(x, y, z);

}

// clang ranges.cpp -I. -std=c++17 -lc++ -o outranges
// clang ranges.cpp -I. -std=c++17 -lc++ -o outranges -O2
// isrange-v3
// 通过避免 STL 位（提交），可以让最终运行时快 10 倍，也可以让编译时间更快且调试更容易，
// 因为微软的 STL 实现特别喜欢深度嵌套的函数调用。这并不是说 STL 必然不好，
// 有可能编写 STL 实现在非优化版本中不会变慢 10 倍（如 EASTL 或 libc ++ 那样），
// 但由于微软的 STL 过度依赖深度嵌套，因此会变慢。
// ranges.cpp#include <time.h>#include <stdio.h>#include <range/v3/all.hpp>

using namespace ranges;

int main() {

	clock_t t0 = clock();



	auto triples = view::for_each(view::ints(1), {

		return view::for_each(view::ints(1, z + 1), = {

			return view::for_each(view::ints(x, z + 1), = {

				return yield_if(x * x + y * y == z * z,

					std::make_tuple(x, y, z));

			});

		});

		});



	RANGES_FOR(auto triple, triples | view::take(100))

	{

		printf("(%i,%i,%i)\n", std::get<0>(triple), std::get<1>(triple), std::get<2>(triple));

	}



	clock_t t1 = clock();

	printf("%ims\n", (int)(t1 - t0) * 1000 / CLOCKS_PER_SEC);

	return 0;

}


//  C＃中“毕达哥拉斯三元数组”实现

using System; using System.Diagnostics; using System.Linq;

class Program

{

	public static void Main()

	{

		var timer = Stopwatch.StartNew();

		var triples =

			from z in Enumerable.Range(1, int.MaxValue)

			from x in Enumerable.Range(1, z)

			from y in Enumerable.Range(x, z)

		where x*x + y * y == z * z

			select(x:x, y : y, z : z);

		foreach(var t in triples.Take(100))

		{

			Console.WriteLine($"({t.x},{t.y},{t.z})");

		}

		timer.Stop();

		Console.WriteLine($"{timer.ElapsedMilliseconds}ms");

	}

}

// C＃LINQ 的另一种“数据库较少”的形式

var triples = Enumerable.Range(1, int.MaxValue)

.SelectMany(z = > Enumerable.Range(1, z), (z, x) = > new {z, x})

.SelectMany(t = > Enumerable.Range(t.x, t.z), (t, y) = > new {t, y})

.Where(t = > t.t.x * t.t.x + t.y * t.y == t.t.z * t.t.z)

.Select(t = > (x: t.t.x, y : t.y, z : t.t.z));

