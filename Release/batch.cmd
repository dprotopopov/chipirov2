rem генерация массива исторических данных из массива случайных параметров
cudamatrix.exe -generate -input random.txt -output history.txt
rem проверка что предсказанные значения с теми же параметрами более или менее совпадают с данными в историческом массиве
rem то есть проверяем Accuracy на тренировочных данных
cudamatrix.exe -predict -history history.txt -input random.txt 
rem выполняем вычисления с заданными параметрами и наилучшими параметрами при которых предсказанное значение цены имеет наименьшую величину
rem выводим пары результатов для сравнения
cudamatrix.exe -compare -history history.txt -input test.txt 