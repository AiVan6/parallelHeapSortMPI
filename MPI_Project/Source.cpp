#include<iostream>
#include <vector>
#include <chrono>
#include <string>
/*ѕирамидальна€ сортировка MPI*/
#include "mpi.h"

void print(int arr[], int size) {
	for (int i = 0; i < 10; i++) {
		std::cout << arr[i] << " ";
	}
	std::cout << "\n";
}

void heapify(int arr[], int n, int root)
{
	int largest = root; // корень - самый большой элемент
	int l = 2 * root + 1; // слева = 2*корень + 1
	int r = 2 * root + 2; // справа = 2*корень + 2

	// ≈сли левый дочерний элемент больше корневого
	if (l < n && arr[l] > arr[largest])
		largest = l;

	// ≈сли правый дочерний элемент больше, чем самый большой на данный момент
	if (r < n && arr[r] > arr[largest])
		largest = r;

	// ≈сли самый большой не €вл€етс€ корневым
	if (largest != root)
	{
		//помен€ть местами корневой и самый большой
		std::swap(arr[root], arr[largest]);
		// –екурсивное нагромождение поддерева
		heapify(arr, n, largest);
	}
}

void sort(int arr[], int n)
{
	// создать кучу
	for (int i = n / 2 - 1; i >= 0; i--) {
		heapify(arr, n, i);
	}

	// извлечение элементов из кучи один за другим
	for (int i = n - 1; i >= 0; i--)
	{
		// ѕереместить текущий корень в конец
		std::swap(arr[0], arr[i]);

		// снова вызовите max heapify дл€ уменьшенной кучи
		heapify(arr, i, 0);
	}
}

void parallelHeapSort(int* Start_Array, int Start_Array_Size,int rank, int size) {
	int tempArray = Start_Array_Size / size;

	int* Work_Array = Start_Array + rank * tempArray;

	if (rank == size - 1)
		tempArray = Start_Array_Size - tempArray * (size - 1);
	sort(Work_Array, tempArray);

	if (rank == 0 && size > 1) {
		MPI_Status Status;
		int* receivedArray = Start_Array + tempArray;

		int Total_Received_Values = tempArray;

		for (int i = 1; i < size - 1; i++) {
			MPI_Recv(receivedArray, tempArray, MPI_INT, i, 0, MPI_COMM_WORLD, &Status);
			receivedArray += tempArray;
			Total_Received_Values += tempArray;
		}

		tempArray = Start_Array_Size - Total_Received_Values;
		MPI_Recv(receivedArray, tempArray, MPI_INT, size - 1, 0, MPI_COMM_WORLD, &Status);
	}
	else if (rank > 0 && size > 1) {
		MPI_Send(Work_Array, tempArray, MPI_INT, 0, 0, MPI_COMM_WORLD);
	}
}

void fillArray(int arr[],int size) {
	for (int i = 0; i < size; i++) {
		arr[i] = rand()%size;
	}
}

int main(int argc, char** argv) {
	int arrSize = std::stoi(argv[1]);	
	MPI_Init(&argc, &argv);

	int rank, size;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	

	int *arr = new int[arrSize];

	fillArray(arr, arrSize);
	
	if (rank == 0) {
		std::cout << "Create parallel array: " << std::endl;
		print(arr, arrSize);
		std::cout << "\n";
	}
	
	auto start = std::chrono::high_resolution_clock::now();
	parallelHeapSort(arr, arrSize,rank,size);
	//sort(arr, arrSize);
	auto end = std::chrono::high_resolution_clock::now();
	std::chrono::duration<float> duration = end - start;
	
	if (rank == 0) {
		std::cout << "Sorted parallel array: " << std::endl;
		print(arr, arrSize);
		std::cout << "\n";
		std::cout << "Time in nano: " << duration.count() << std::endl;
	}
	
	//выводит врем€ в нано сек.
	MPI_Finalize();
	

	return 0;
}