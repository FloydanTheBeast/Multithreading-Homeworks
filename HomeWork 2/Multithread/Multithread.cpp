﻿#include <iostream>
#include <sstream>
#include <cmath>
#include <algorithm>
#include "omp.h"
#include <clocale>

using namespace std;

int matrixSize,
threadSize; // количество ячеек, обрабатываемых одним потоком;

/// Метод для печати матрицы на экран
/// \param matrix - Указатель на матрицу (двумерный массив)
void PrintMatrix(int** matrix) {
	for (int i = 0; i < matrixSize; i++) {
		for (int j = 0; j < matrixSize; j++) {
			cout << matrix[i][j] << '\t';
		}
		cout << endl;
	}
}

/// Мето для считывания матрицы со стандартного потока ввода
/// \param matrix - Указатель на матрицу (двумерный массив)
void ScanMatrix(int** matrix) {
	istringstream iss;
	string str;

	for (int i = 0; i < matrixSize; i++) {
		getline(cin, str);
		iss.clear();
		iss.str(str);
		for (int j = 0; j < matrixSize; j++) {
			if (!(iss >> matrix[i][j])) {
				cout << "Строка " << i + 1 << " введена неправильно, попробуйте ещё раз" << endl;
				--i;
				break;
			}
		}
	}
}

struct ThreadData {
	int i; // индекс начального элемента
	int** matrix1;
	int** matrix2;
	int** resultMatrix;
};

void matrixMultiplication(void* threadData)
{
	ThreadData* data = (ThreadData*)threadData;

	int iStart = (int)data->i / matrixSize,
		jStart = data->i % matrixSize;

	// Количество оставшихся чисел для посчета этим потоком
	int threadCounter = threadSize;

	for (int i = iStart; i < matrixSize && threadCounter > 0; i++)
		for (int j = i == iStart ? jStart : 0; j < matrixSize && threadCounter > 0; j++, threadCounter--)
			for (int k = 0; k < matrixSize; k++)
				data->resultMatrix[i][j] +=
				data->matrix1[i][k] * data->matrix2[k][j];

	cout << "Поток №" << omp_get_thread_num() + 1 << " из " << omp_get_num_threads() << 
		". Закончился подсчет элементов отрезка [" << data->i << "," << min(data->i + threadSize, matrixSize * matrixSize) << ")" << endl;

}

int main(int argc, char* argv[]) {
	setlocale(LC_ALL, "Russian");

	if (argc < 3) {
		cout << "Долнжо быть передано 2 аргумента командной строки: количество потоков и размер матриц" << endl;
		exit(1);
	}

	int threadsNumber;

	try {
		threadsNumber = atoi(argv[1]);
		matrixSize = atoi(argv[2]);

		if (threadsNumber <= 0 || matrixSize <= 0) {
			cout << "Оба аргумента командной строки должны быть положительными" << endl;
			exit(1);
		}

		int size = matrixSize * matrixSize; // количество обрабатываемых элементов
		threadSize = ceil((double)size / threadsNumber);

		int maxThreads = ceil((double)size / threadSize);

		if (threadsNumber > maxThreads) {
			cout << "Количество потоков превышает допустимое значение, равное " << maxThreads
				<< ", будет использоваться максимальное количество" << endl;
			threadsNumber = maxThreads;
		}
	}
	catch (...) {
		cout << "Оба аргумента командной строки должны быть числами" << endl;
		exit(1);
	}

	// Выделение памяти для матриц (двумерных массивов)
	int** matrix1 = (int**)malloc(matrixSize * sizeof(int*));
	int** matrix2 = (int**)malloc(matrixSize * sizeof(int*));
	int** resultMatrix = (int**)malloc(matrixSize * sizeof(int*));

	// Выделение памяти для строк матриц
	for (int i = 0; i < matrixSize; i++) {
		matrix1[i] = (int*)malloc(matrixSize * sizeof(int));
		matrix2[i] = (int*)malloc(matrixSize * sizeof(int));
		resultMatrix[i] = (int*)malloc(matrixSize * sizeof(int));
	}

	for (int i = 0; i < matrixSize; i++)
		for (int j = 0; j < matrixSize; j++)
			resultMatrix[i][j] = 0;

	cout << "Введите матрицу A:" << endl;
	ScanMatrix(matrix1);
	cout << "Введите матрицу B:" << endl;
	ScanMatrix(matrix2);

	ThreadData* threadsData = (ThreadData*)malloc(matrixSize * matrixSize * sizeof(ThreadData));

	for (int i = 0, threadIndex = 0; i < threadsNumber; i++, threadIndex += threadSize) {
		threadsData[i].i = threadIndex;
		threadsData[i].matrix1 = matrix1;
		threadsData[i].matrix2 = matrix2;
		threadsData[i].resultMatrix = resultMatrix;
	}

#pragma omp parallel for num_threads(threadsNumber)
	for (int i = 0; i < threadsNumber; i++) {
		#pragma omp critical
		{
			matrixMultiplication(&threadsData[i]);
		}
	}

	cout << endl
		<< "A * B = " << endl;
	PrintMatrix(resultMatrix);
	return 0;
}