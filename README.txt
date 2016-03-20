Умножение матриц m*n*k
Сложение m*n
Транспонирование m*n
Обратная n^3

result.csv
mul;add;rot;inv;blocks;threads;energy
0;537500;0;0;2;16;0,050279827
0;537500;0;0;2;32;0,048105128
0;537500;0;0;2;48;0,036886194
...


x64\Release\ppredict -thebest -history result.csv  -gmax 16 -bmax 1024 mul 30000 -p 3
30000 0 0 0 10 742 0.000718208