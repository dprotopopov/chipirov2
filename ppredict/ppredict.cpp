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
	double m;
	double n;
	double grid_size;
	double block_size;
	double price;
};

/////////////////////////////////////////////////////////
// ¬ычисление расто€ни€ между двум€ векторами координат
double delta(std::vector<double>& x, std::vector<double>& y, std::vector<double>& s2)
{
	auto s = 0.0;
	auto i = 0;
	for (; i < x.size() && i < y.size() && i < s2.size(); i++) s += (x[i] - y[i]) * (x[i] - y[i]) / s2[i];
	for (; i < x.size() && i < s2.size(); i++) s += x[i] * x[i] / s2[i];
	for (; i < y.size() && i < s2.size(); i++) s += y[i] * y[i] / s2[i];
	return sqrt(s);
}
/////////////////////////////////////////////////////////
// ¬ычисление расто€ни€ между двум€ векторами координат
double scalar(std::vector<double>& x, std::vector<double>& y, std::vector<double>& s2)
{
	auto s = 0.0;
	auto i = 0;
	for (; i < x.size() && i < y.size() && i < s2.size(); i++) s += (x[i] * y[i]) / s2[i];
	return s;
}

/////////////////////////////////////////////////////////
// ¬озвращает предсказание цены дл€ указанных параметров исход€ из исторических данных
double predict(double grid_size, double block_size, double m, double n, 
	std::vector<t_previous_result>& previous_results, 
	std::vector<double>& s2, 
	int p)
{
	std::vector<double> x;
	x.push_back(m);
	x.push_back(n);
	x.push_back(grid_size);
	x.push_back(block_size);
	std::vector<std::pair<t_previous_result, double>> neighbors;
	for (auto it = previous_results.begin(); it != previous_results.end(); ++it)
	{
		std::vector<double> x2;
		x2.push_back(it->m);
		x2.push_back(it->n);
		x2.push_back(it->grid_size);
		x2.push_back(it->block_size);
		auto d = delta(x, x2, s2);
		std::pair<t_previous_result, double> pair(*it, d);
		neighbors.push_back(pair);
	}
	std::sort(neighbors.begin(), neighbors.end(),
	          [](std::pair<t_previous_result, double> const& i, std::pair<t_previous_result, double> const& j)
	          {
		          return (i.second < j.second);
	          });
	neighbors.resize(std::min(p, static_cast<int>(neighbors.size())));
	auto y = 0.0;
	for (auto iti = neighbors.begin(); iti != neighbors.end(); ++iti)
	{
		auto s = iti->first.price;
		std::vector<double> xi;
		xi.push_back(iti->first.m);
		xi.push_back(iti->first.n);
		xi.push_back(iti->first.grid_size);
		xi.push_back(iti->first.block_size);
		for (auto itj = neighbors.begin(); itj != neighbors.end(); ++itj)
		{
			if (iti == itj) continue;
			std::vector<double> xj;
			std::vector<double> xxj;
			std::vector<double> xixj;
			xj.push_back(itj->first.m);
			xj.push_back(itj->first.n);
			xj.push_back(itj->first.grid_size);
			xj.push_back(itj->first.block_size);
			for (auto i = 0; i < x.size() && i < xj.size(); i++) xxj.push_back(x[i] - xj[i]);
			for (auto i = 0; i < xi.size() && i < xj.size(); i++) xixj.push_back(xi[i] - xj[i]);
			s *= scalar(xxj, xixj, s2) / scalar(xixj, xixj, s2);
		}
		y += s;
	}
	return y;
}

enum t_mode
{
	THEBEST = 0,
	PREDICTONLY = 1,
};

t_mode mode = THEBEST;

/////////////////////////////////////////////////////////
// ƒефолтные значени€
static const int _m = 2;
static const int _n = 2;
static const int _p = 3;
static const int _grid_size = 1;
static const int _block_size = 255;
static const int _gmax = 16;
static const int _bmax = 1024;

int main(int argc, char* argv[])
{
	std::vector<t_previous_result> previous_results;
	std::vector<double> s2(4, 1.0);
	char* input_file_name = nullptr;
	char* output_file_name = nullptr;
	char* previous_results_file_name = nullptr;

	auto gmax = _gmax;
	auto bmax = _bmax;
	auto p = _p;
	auto m = _m;
	auto n = _n;
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
		else if (strcmp(argv[i], "-gmax") == 0) gmax = atoi(argv[++i]);
		else if (strcmp(argv[i], "-bmax") == 0) bmax = atoi(argv[++i]);
		else if (strcmp(argv[i], "-p") == 0) p = atoi(argv[++i]);
	}

	if (input_file_name != nullptr) freopen(input_file_name, "r", stdin);
	if (output_file_name != nullptr) freopen(output_file_name, "w", stdout);

	if (previous_results_file_name != nullptr)
	{
		std::vector<double> m1(4, 0.0);
		std::vector<double> m2(4, 0.0);
		std::vector<double> mx(4, 0.0);
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
			previous_result.m = m;
			previous_result.n = n;
			previous_result.grid_size = grid_size;
			previous_result.block_size = block_size;
			previous_result.price = price;
			previous_results.push_back(previous_result);
			m1[0] += m;
			m1[1] += n;
			m1[2] += grid_size;
			m1[3] += block_size;
			m2[0] += m * m;
			m2[1] += n * n;
			m2[2] += grid_size * grid_size;
			m2[3] += block_size * block_size;
			mx[0] = std::max(mx[0],(double)m);
			mx[1] = std::max(mx[1], (double)n);
			mx[2] = std::max(mx[2], (double)grid_size);
			mx[3] = std::max(mx[0], (double)block_size);
		}
		for (auto it = m1.begin(); it != m1.end(); ++it) *it /= previous_results.size();
		for (auto it = m2.begin(); it != m2.end(); ++it) *it /= previous_results.size();
		auto i = 0;
		for (auto it = s2.begin(); it != s2.end(); ++it, ++i) *it = m2[i] - m1[i] * m1[i];
		// for (auto it = s2.begin(); it != s2.end(); ++it, ++i) *it = mx[i];
	}
	switch (mode)
	{
	case PREDICTONLY:
		{
			auto price = predict(grid_size, block_size, m, n, previous_results, s2, p);
			std::cout << m << ' ' << n << ' ' << ' ' << grid_size << ' ' << block_size << ' ' << price << std::endl;
		}
		break;
	case THEBEST:
		{
			// находим параметры при которых предсказанна€ цена будет минимальна
			int grid_size1;
			int block_size1;
			double price1 = DBL_MAX;
			for (auto i = 1; i <= gmax; i++)
				for (auto j = 1; j <= bmax; j++)
				{
					auto price2 = predict(i, j, m, n, previous_results, s2, p);
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
