rem ��������� ������� ������������ ������ �� ������� ��������� ����������
cudamatrix.exe -generate -input random.txt -output history.txt
rem �������� ��� ������������� �������� � ���� �� ����������� ����� ��� ����� ��������� � ������� � ������������ �������
rem �� ���� ��������� Accuracy �� ������������� ������
cudamatrix.exe -predict -history history.txt -input random.txt 
rem ��������� ���������� � ��������� ����������� � ���������� ����������� ��� ������� ������������� �������� ����� ���������� ��������
rem ������� ���� ����������� ��� ���������
cudamatrix.exe -compare -history history.txt -input test.txt 