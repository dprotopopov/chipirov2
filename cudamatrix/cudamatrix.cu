#define _CRT_SECURE_NO_WARNINGS
#define _SCL_SECURE_NO_WARNINGS

#include <thrust/host_vector.h>
#include <thrust/device_vector.h>
#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <sstream>
#include <string>
#include <time.h>
#include <fstream>

using namespace std;
using namespace thrust;

// Thrust is a C++ template library for CUDA based on the Standard Template Library (STL).
// Thrust allows you to implement high performance parallel applications with minimal programming effort through a high-level interface that is fully interoperable with CUDA C.
// Thrust provides a rich collection of data parallel primitives such as scan, sort, and reduce, which can be composed together to implement complex algorithms with concise, readable source code.
// By describing your computation in terms of these high-level abstractions you provide Thrust with the freedom to select the most efficient implementation automatically.
// As a result, Thrust can be utilized in rapid prototyping of CUDA applications, where programmer productivity matters most, as well as in production, where robustness and absolute performance are crucial.
// Read more at: http://docs.nvidia.com/cuda/thrust/index.html#ixzz3hymTnQwX 

template <typename T>
T mul_functor(T value1, T value2)
{
	return value1 * value2;
}

template <typename T>
T add_functor(T value1, T value2)
{
	return value1 + value2;
}

struct t_previous_result
{
	int m;
	int n;
	int k;
	int grid_size;
	int block_size;
	double price;
};

/////////////////////////////////////////////////////////
// ¬ычисление расто€ни€ между двум€ векторами координат
double delta(std::vector<double> x, std::vector<double> y)
{
	auto s = 0.0;
	auto i = 0;
	for (; i < x.size() && i < y.size(); i++) s += (x[i] - y[i])*(x[i] - y[i]);
	for (; i < x.size(); i++) s += x[i] * x[i];
	for (; i < y.size(); i++) s += y[i] * y[i];
	return s;
}

/////////////////////////////////////////////////////////
// ¬озвращает предсказание цены дл€ указанных параметров исход€ из исторических данных
double predict(int grid_size, int block_size, size_t m, size_t n, size_t k, std::vector<t_previous_result> & previous_results)
{
		std::vector<double> x1;
		x1.push_back(m);
		x1.push_back(n);
		x1.push_back(k);
		x1.push_back(grid_size);
		x1.push_back(block_size);
		auto sy = 0.0;
		auto sw = 0.0;
		for (auto it = previous_results.begin(); it != previous_results.end(); ++it)
		{
			std::vector<double> x2;
			x2.push_back(it->m);
			x2.push_back(it->n);
			x2.push_back(it->k);
			x2.push_back(it->grid_size);
			x2.push_back(it->block_size);

			auto w = 1.0/(1.0+delta(x1, x2));
			sy += it->price*w;
			sw += w;
		}
		return sy / sw;
}

__global__ void matrix_mul_kernel(double *a, double *b, double *c, size_t m, size_t n, size_t k)
{
	int total = m*k;
	for (int id = threadIdx.x; id < total; id += blockDim.x)
	{
		int x = id / k;
		int y = id % k;
		double s = 0;
		for (int i = 0; i < n; i++)
			s += a[x*n + i] * b[i*k + y];
		c[id] = s;

	}
}

__global__ void matrix_fill_kernel(double *a, size_t m, size_t n)
{
	int total = m*n;
	for (int id = threadIdx.x; id < total; id += blockDim.x)
		a[id] = id;
}

/////////////////////////////////////////////////////////
// ѕроводит вычислени€ и возвращает цену дл€ указанных параметров
__host__ double body(int grid_size, int block_size, size_t m, size_t n, size_t k, int count)
{
	auto time = clock();
	for (auto s = 0; s < count; s++)
	{
		thrust::device_vector<double> a(m*n);
		thrust::device_vector<double> b(n*k);
		thrust::device_vector<double> c(m*k);
		auto aPtr = thrust::raw_pointer_cast(&a[0]);
		auto bPtr = thrust::raw_pointer_cast(&b[0]);
		auto cPtr = thrust::raw_pointer_cast(&c[0]);
		matrix_fill_kernel <<< grid_size, block_size >>> (aPtr, m, n);
		matrix_fill_kernel <<< grid_size, block_size >>> (bPtr, n, k);
		matrix_mul_kernel <<< grid_size, block_size >>> (aPtr, bPtr, cPtr, m, n, k);
	}
	time = clock() - time;
	auto price = static_cast<double>(time) / CLOCKS_PER_SEC / count / m/n/k;
	return price;
}


enum t_mode
{
	DEFAULT = 0,
	GENERATE = 1,
	PREDICT = 2,
	COMPARE = 3
};

t_mode mode = DEFAULT;

/////////////////////////////////////////////////////////
// ƒефолтные значени€
static const unsigned _count = 1;
static const int _m = 2;
static const int _n = 2;
static const int _k = 2;
static const int _grid_size = 1;
static const int _block_size = 255;

int main(int argc, char* argv[])
{
	std::vector<t_previous_result> previous_results;

	auto count = _count;

	char* input_file_name = NULL;
	char* output_file_name = NULL;
	char* previous_results_file_name = NULL;

	auto m = _m;
	auto n = _n;
	auto k = _k;
	auto grid_size = _grid_size;
	auto block_size = _block_size;

	// ѕоддержка кириллицы в консоли Windows
	// ‘ункци€ setlocale() имеет два параметра, первый параметр - тип категории локали, в нашем случае LC_TYPE - набор символов, второй параметр Ч значение локали. 
	// ¬место второго аргумента можно писать "Russian", или оставл€ть пустые двойные кавычки, тогда набор символов будет такой же как и в ќ—.
	setlocale(LC_ALL, "");

	for (int i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "-help") == 0)
		{
			std::cout << "Usage :\t" << argv[0] << " [...] [g <grid_size>] [b <block_size>] [-input <inputfile>] [-output <outputfile>]" << std::endl;
			std::cout << "\t-c <количество повторений алгоритма дл€ замера времени>" << std::endl;
		}
		else if (strcmp(argv[i], "-generate") == 0) mode = GENERATE;
		else if (strcmp(argv[i], "-predict") == 0) mode = PREDICT;
		else if (strcmp(argv[i], "-compare") == 0) mode = COMPARE;
		else if (strcmp(argv[i], "-c") == 0) count = atoi(argv[++i]);
		else if (strcmp(argv[i], "-input") == 0) input_file_name = argv[++i];
		else if (strcmp(argv[i], "-output") == 0) output_file_name = argv[++i];
		else if (strcmp(argv[i], "-history") == 0) previous_results_file_name = argv[++i];
		else if (strcmp(argv[i], "m") == 0) m = atoi(argv[++i]);
		else if (strcmp(argv[i], "n") == 0) n = atoi(argv[++i]);
		else if (strcmp(argv[i], "k") == 0) k = atoi(argv[++i]);
		else if (strcmp(argv[i], "g") == 0) grid_size = atoi(argv[++i]);
		else if (strcmp(argv[i], "b") == 0) block_size = atoi(argv[++i]);
	}

	if (input_file_name != NULL) freopen(input_file_name, "r", stdin);
	if (output_file_name != NULL) freopen(output_file_name, "w", stdout);

	if (previous_results_file_name != NULL)
	{
		std::ifstream history(previous_results_file_name);
		if (!history.is_open()) throw "Error opening file";
		std::string line;
		while (std::getline(history, line))
		{
			std::stringstream lineStream(line);
			int m;
			int n;
			int k;
			int grid_size;
			int block_size;
			double price;
			lineStream >> m >> n >> k >> grid_size >> block_size >> price;
			t_previous_result previous_result;
			previous_result.m = m;
			previous_result.n = n;
			previous_result.k = k;
			previous_result.grid_size = grid_size;
			previous_result.block_size = block_size;
			previous_result.price = price;
			previous_results.push_back(previous_result);
		}
	}

		std::string line;
		switch (mode)
		{
		case GENERATE:
			for (std::getline(std::cin, line); !line.empty(); std::getline(std::cin, line))
			{
				std::stringstream lineStream(line);
				lineStream >> m >> n >> k >> grid_size >> block_size;
				auto price = body(grid_size, block_size, m, n, k, count);
				std::cout << m << ' ' << n << ' ' << k << ' ' << grid_size << ' ' << block_size << ' ' << price << std::endl;
			}
			break;
		case PREDICT:
			for (std::getline(std::cin, line); !line.empty(); std::getline(std::cin, line))
			{
				std::stringstream lineStream(line);
				lineStream >> m >> n >> k >> grid_size >> block_size;
				auto price = predict(grid_size, block_size, m, n, k, previous_results);
				std::cout << m << ' ' << n << ' ' << k << ' ' << grid_size << ' ' << block_size << ' ' << price << std::endl;
			}
			break;
		case COMPARE:
			for (std::getline(std::cin, line); !line.empty(); std::getline(std::cin, line))
			{
				std::stringstream lineStream(line);
				lineStream >> m >> n >> k >> grid_size >> block_size;
				int grid_size1;
				int block_size1;
				// выполн€ем вычислени€ с переданными параметрами
				auto price = body(grid_size, block_size, m, n, k, count);
				// находим параметры при которых предсказанна€ цена будет минимальна
				double price1 = DBL_MAX;
				for (auto i = 1; i < 32; i++)
					for (auto j = 1; j < 256; j++)
					{
						auto price = predict(grid_size, block_size, m, n, k, previous_results);
						if (price1 < price) continue;
						grid_size1 = i;
						block_size1 = j;
						price1 = price;
					}
				// выполн€ем вычислени€ с параметрами при которых предсказанна€ цена минимальна
				price1 = body(grid_size1, block_size1, m, n, k, count);
				std::cout << m << ' ' << n << ' ' << k << ' ' 
					<< grid_size << '/' << grid_size1 << ' ' 
					<< block_size << '/' << block_size1 << ' ' 
					<< price << '/' << price1 << std::endl;
			}
			break;
		default:
			{
				auto price = body(grid_size, block_size, m, n, k, count);
				std::cout << m << ' ' << n << ' ' << k << ' ' << grid_size << ' ' << block_size << ' ' << price << std::endl;
			}
			break;
		}

	return 0;
}

