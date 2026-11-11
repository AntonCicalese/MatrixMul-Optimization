#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>

#define MAX_N 20000
#define MATRIX_VALUE_RANGE_MAX 10

#define SELECTED_FUNCTION 2
#define NUM_THREADS 8

void printMatrix(double *mat, size_t size);
void matrixMult0(double *mat1, double *mat2, double *outputMat, size_t size);
void matrixMult1(double *mat1, double *mat2, double *outputMat, size_t size);
void matrixMult2(double *mat1, double *mat2, double *outputMat, size_t size);
void zeroMatrix(double *mat, size_t size);

int main() {
	size_t n;
	int temp = 1, i, j;
	double *matA, *matB, *matC;
	double elapsedTime;
	
	//printf("System size_t: %zu\n", sizeof(size_t));
	
	srand(42);
	
	// Matrix size read
	
	do {
		if (temp <= 0)
			printf("ERROR: Value must be greater than 0\n");
		else if(temp > MAX_N)
			printf("ERROR: Value can't exceed %d\n", MAX_N);
		printf("Insert matrix size: ");
		scanf("%d", &temp);
	} while (temp <= 0 || temp > MAX_N);
	n = (size_t)temp;
	
	printf("Confirmed matrix size: %zu\n", n);
	
	// Matrix setup
	
	matA = malloc(n*n*sizeof(*matA));
	matB = malloc(n*n*sizeof(*matB));
	matC = malloc(n*n*sizeof(*matC));
	
	for(i = 0; i < n; i++) {
		for(j = 0; j < n; j++) {
			matA[i * n + j] = rand() % MATRIX_VALUE_RANGE_MAX;
			matB[i * n + j] = rand() % MATRIX_VALUE_RANGE_MAX;
		}
	}
	
	zeroMatrix(matC, n);
	/*
	printf("Matrix A:\n");
	printMatrix(matA, n);
	printf("Matrix B:\n");
	printMatrix(matB, n);
	*/
	// Operation
	switch(SELECTED_FUNCTION) {
		case 0: // ijk solution
			elapsedTime = clock();
			matrixMult0(matA, matB, matC, n);
			elapsedTime = (clock() - elapsedTime)  / CLOCKS_PER_SEC;
			break;
		case 1: // ikj solution
			elapsedTime = clock();
			matrixMult1(matA, matB, matC, n);
			elapsedTime = (clock() - elapsedTime)  / CLOCKS_PER_SEC;
			break;
		case 2: // Parallel ikj solution
			omp_set_num_threads(NUM_THREADS);
			elapsedTime = omp_get_wtime();
			matrixMult2(matA, matB, matC, n);
			elapsedTime = omp_get_wtime() - elapsedTime;
			break;
	}
	
	printf("Processor time taken:\n");
	printf("%lf seconds\n", elapsedTime);
	
	// Output 
	/*
	printf("Output Matrix:\n");
	printMatrix(matC, n);
	*/
	free(matA);
	free(matB);
	free(matC);
	return 0;
}

void printMatrix(double *mat, size_t size) {
	size_t i, j;
	
	for(i = 0; i < size; i++) {
		for(j = 0; j < size; j++) {
			printf("%2.0lf ", mat[i * size + j]);
		}
		printf("\n");
	}	
	return;
}

void zeroMatrix(double *mat, size_t size) {
	size_t i, j;
	for(i = 0; i < size; i++) {
		for(j = 0; j < size; j++) {
			mat[i * size + j] = 0;
		}
	}
	return;
}

void matrixMult0(double *mat1, double *mat2, double *outputMat, size_t size) {
	size_t i, j, k;
	double partialSum;
	
	for(i = 0; i < size; i++) {
		for(j = 0; j < size; j++) {
			partialSum = 0;
			for(k = 0; k < size; k++) {
				partialSum += mat1[i * size + k] * mat2[k * size + j];
			}
			outputMat[i * size + j] = partialSum;
		}
	}
	return;
}

void matrixMult1(double *mat1, double *mat2, double *outputMat, size_t size) {
	size_t i, k, j;
	
	for(i = 0; i < size; i++) {
		for(k = 0; k < size; k++) {
			for(j = 0; j < size; j++) {
				outputMat[i * size + j] += mat1[i * size + k] * mat2[k * size + j];
			}
		}
	}
	return;
}

void matrixMult2(double *mat1, double *mat2, double *outputMat, size_t size) {
	size_t i;
	
	#pragma omp parallel for
	for(i = 0; i < size; i++) {
		for(size_t k = 0; k < size; k++) {
			for(size_t j = 0; j < size; j++) {
				outputMat[i * size + j] += mat1[i * size + k] * mat2[k * size + j];
			}
		}
	}
	return;
}
