#define _CRT_SECURE_NO_WARNINGS
#define _SCL_SECURE_NO_WARNINGS

#include <vector>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>

struct t_previous_result
{
	double log_m;
	double log_n;
	double log_grid_size;
	double log_block_size;
	double price;
};

/////////////////////////////////////////////////////////
// Вычисление растояния между двумя векторами координат
double delta(std::vector<double> x, std::vector<double> y)
{
	auto s = 0.0;
	auto i = 0;
	for (; i < x.size() && i < y.size(); i++) s += (x[i] - y[i]) * (x[i] - y[i]);
	for (; i < x.size(); i++) s += x[i] * x[i];
	for (; i < y.size(); i++) s += y[i] * y[i];
	return s;
}

/////////////////////////////////////////////////////////
// Возвращает предсказание цены для указанных параметров исходя из исторических данных
double predict(double log_grid_size, double log_block_size, double log_m, double log_n, std::vector<t_previous_result>& previous_results)
{
	std::vector<double> x1;
	x1.push_back(log_m);
	x1.push_back(log_n);
	x1.push_back(log_grid_size);
	x1.push_back(log_block_size);
	std::vector<std::pair<t_previous_result, double>> neighbors;
	for (auto it = previous_results.begin(); it != previous_results.end(); ++it)
	{
		std::vector<double> x2;
		x2.push_back(it->log_m);
		x2.push_back(it->log_n);
		x2.push_back(it->log_grid_size);
		x2.push_back(it->log_block_size);
		auto d = delta(x1, x2);
		std::pair<t_previous_result, double> pair(*it, d);
		neighbors.push_back(pair);
	}
	std::sort(neighbors.begin(), neighbors.end(),
	          [](std::pair<t_previous_result, double> const& i, std::pair<t_previous_result, double> const& j)
	          {
		          return (i.second < j.second);
	          });
	neighbors.resize(std::min(6, static_cast<int>(neighbors.size())));
	auto sy = 0.0;
	auto sw = 0.0;
	for (auto it = neighbors.begin(); it != neighbors.end(); ++it)
	{
		//std::cout << exp(it->first.log_m) << ' ' << exp(it->first.log_n) << ' ' << exp(it->first.log_grid_size) << ' ' << exp(it->first.log_block_size) << ' ' << it->first.price << ' ' << it->second << std::endl;
		auto w = 1.0 / (1.0 + it->second);
		sy += it->first.price * w;
		sw += w;
	}
	return sy / sw;
}

enum t_mode
{
	THEBEST = 0,
	PREDICTONLY = 1,
};

t_mode mode = THEBEST;

/////////////////////////////////////////////////////////
// Дефолтные значения
static const int _m = 2;
static const int _n = 2;
static const int _grid_size = 1;
static const int _block_size = 255;

int main(int argc, char* argv[])
{
	std::vector<t_previous_result> previous_results;

	char* input_file_name = nullptr;
	char* output_file_name = nullptr;
	char* previous_results_file_name = nullptr;

	auto m = _m;
	auto n = _n;
	auto grid_size = _grid_size;
	auto block_size = _block_size;

	// Поддержка кириллицы в консоли Windows
	// Функция setlocale() имеет два параметра, первый параметр - тип категории локали, в нашем случае LC_TYPE - набор символов, второй параметр — значение локали. 
	// Вместо второго аргумента можно писать "Russian", или оставлять пустые двойные кавычки, тогда набор символов будет такой же как и в ОС.
	setlocale(LC_ALL, "");

	for (int i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "-help") == 0)
		{
			std::cout << "Usage :\t" << argv[0] << " [...] [g <grid_size>] [b <block_size>] [-input <inputfile>] [-output <outputfile>]" << std::endl;
		}
		else if (strcmp(argv[i], "-predictonly") == 0) mode = PREDICTONLY;
		else if (strcmp(argv[i], "-thebest") == 0) mode = THEBEST;
		else if (strcmp(argv[i], "-input") == 0) input_file_name = argv[++i];
		else if (strcmp(argv[i], "-output") == 0) output_file_name = argv[++i];
		else if (strcmp(argv[i], "-history") == 0) previous_results_file_name = argv[++i];
		else if (strcmp(argv[i], "m") == 0) m = atoi(argv[++i]);
		else if (strcmp(argv[i], "n") == 0) n = atoi(argv[++i]);
		else if (strcmp(argv[i], "g") == 0) grid_size = atoi(argv[++i]);
		else if (strcmp(argv[i], "b") == 0) block_size = atoi(argv[++i]);
	}

	if (input_file_name != nullptr) freopen(input_file_name, "r", stdin);
	if (output_file_name != nullptr) freopen(output_file_name, "w", stdout);

	if (previous_results_file_name != nullptr)
	{
		std::ifstream history(previous_results_file_name);
		if (!history.is_open()) throw "Error opening file";
		std::string line;
		while (std::getline(history, line))
		{
			std::stringstream lineStream(line);
			int m;
			int n;
			int grid_size;
			int block_size;
			double price;
			lineStream >> m >> n >> grid_size >> block_size >> price;
			t_previous_result previous_result;
			previous_result.log_m = log(m);
			previous_result.log_n = log(n);
			previous_result.log_grid_size = log(grid_size);
			previous_result.log_block_size = log(block_size);
			previous_result.price = price;
			previous_results.push_back(previous_result);
		}
	}
	switch (mode)
	{
	case PREDICTONLY:
		{
			auto price = predict(log(grid_size), log(block_size), log(m), log(n), previous_results);
			std::cout << m << ' ' << n << ' ' << ' ' << grid_size << ' ' << block_size << ' ' << price << std::endl;
		}
		break;
	case THEBEST:
		{
			// находим параметры при которых предсказанная цена будет минимальна
			int grid_size1;
			int block_size1;
			double price1 = DBL_MAX;
			for (auto i = 1; i < 32; i++)
				for (auto j = 1; j < 1024; j++)
				{
					auto price2 = predict(log(i), log(j), log(m), log(n), previous_results);
					if (price1 < price2) continue;
					grid_size1 = i;
					block_size1 = j;
					price1 = price2;
				}
			std::cout << m << ' ' << n << ' ' << ' ' << grid_size1 << ' ' << block_size1 << ' ' << price1 << std::endl;
		}
		break;
	}

	return 0;
}
