rem ������� ���ᨢ� �����᪨� ������ �� ���ᨢ� ��砩��� ��ࠬ��஢
cudamatrix.exe -generate -input random.txt -output history.txt
rem �஢�ઠ �� �।᪠����� ���祭�� � ⥬� �� ��ࠬ��ࠬ� ����� ��� ����� ᮢ������ � ����묨 � �����᪮� ���ᨢ�
rem � ���� �஢��塞 Accuracy �� �७�஢���� ������
cudamatrix.exe -predict -history history.txt -input random.txt 
rem �믮��塞 ���᫥��� � ������묨 ��ࠬ��ࠬ� � ������訬� ��ࠬ��ࠬ� �� ������ �।᪠������ ���祭�� 業� ����� ���������� ����稭�
rem �뢮��� ���� १���⮢ ��� �ࠢ�����
cudamatrix.exe -compare -history history.txt -input test.txt 