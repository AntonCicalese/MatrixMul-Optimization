#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <omp.h>

#define MAX_N 20000
#define MATRIX_VALUE_RANGE_MAX 10

#define DEFAULT_MATRIX_SIZE 1000
#define DEFAULT_FUNCTION_IDX 1
#define DEFAULT_THREADS_NUM 1
#define DEFAULT_VERBOSE_FLAG 0

#define NUM_OF_AVAILABLE_FUNCTIONS 3

typedef struct {
	size_t matSize;
	int functionIdx;
	int threadsNum;
	int verbose;
} Settings;

void printMatrix(double *mat, size_t size);
void printSettings(Settings cfg);
void matrixMult0(double *mat1, double *mat2, double *outputMat, size_t size);
void matrixMult1(double *mat1, double *mat2, double *outputMat, size_t size);
void matrixMult2(double *mat1, double *mat2, double *outputMat, size_t size);
void zeroMatrix(double *mat, size_t size);

/*TO DO
- Fix verbose setting with print on every major step
- Add --help command for info on how to use the program
- Fix output on non-verbose for use by execution scripts
- Modularize code for: Matrix random values filling, Code execution timing
*/

int main(int argc, char **argv) {
	Settings cfg;
	int i, j;
	double *matA, *matB, *matC;
	double elapsedTime = 0;
	
	srand(42);
	
	// Default config
	
	cfg.matSize = DEFAULT_MATRIX_SIZE;
	cfg.functionIdx = DEFAULT_FUNCTION_IDX;
	cfg.threadsNum = DEFAULT_THREADS_NUM;
	cfg.verbose = DEFAULT_VERBOSE_FLAG;
	
	// Program arguments read
	
	for(i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-n") == 0) {
			cfg.matSize = atoi(argv[++i]);
		} else if (strcmp(argv[i], "-f") == 0) {
			cfg.functionIdx = atoi(argv[++i]);
		} else if (strcmp(argv[i], "-t") == 0) {
			cfg.threadsNum = atoi(argv[++i]);
		} else if (strcmp(argv[i], "-v") == 0) {
			cfg.verbose = 1;
		}
	}
	
	if (cfg.verbose) printSettings(cfg);
	
	// Matrix setup
	
	matA = malloc(cfg.matSize*cfg.matSize*sizeof(*matA));
	matB = malloc(cfg.matSize*cfg.matSize*sizeof(*matB));
	matC = malloc(cfg.matSize*cfg.matSize*sizeof(*matC));
	
	for(i = 0; i < cfg.matSize; i++) {
		for(j = 0; j < cfg.matSize; j++) {
			matA[i * cfg.matSize + j] = rand() % MATRIX_VALUE_RANGE_MAX;
			matB[i * cfg.matSize + j] = rand() % MATRIX_VALUE_RANGE_MAX;
		}
	}
	
	zeroMatrix(matC, cfg.matSize);
	/*
	printf("Matrix A:\n");
	printMatrix(matA, n);
	printf("Matrix B:\n");
	printMatrix(matB, n);
	*/
	
	// Function Select
	/*printf("Available Functions:\n");
	printf("0 - IJK Solution\n");
	printf("1 - IKJ Solution\n");
	printf("2 - Parallel IKJ Solution\n");
	printf("Select a Valid Function: \n");
	*/
	
	// Operation
	
	switch(cfg.functionIdx) {
		case 0: // ijk solution
			elapsedTime = clock();
			matrixMult0(matA, matB, matC, cfg.matSize);
			elapsedTime = (clock() - elapsedTime)  / CLOCKS_PER_SEC;
			break;
		case 1: // ikj solution
			elapsedTime = clock();
			matrixMult1(matA, matB, matC, cfg.matSize);
			elapsedTime = (clock() - elapsedTime)  / CLOCKS_PER_SEC;
			break;
		case 2: // Parallel ikj solution
			omp_set_num_threads(cfg.threadsNum);
			elapsedTime = omp_get_wtime();
			matrixMult2(matA, matB, matC, cfg.matSize);
			elapsedTime = omp_get_wtime() - elapsedTime;
			break;
	}
	
	printf("Processor time taken: ");
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

void printSettings(Settings cfg) {
	printf("----------------------\n");
	printf("Execution settings: \n");
	printf("Matrix Size: %zu\n", cfg.matSize);
	printf("Function Index: %d\n", cfg.functionIdx);
	printf("Threads Num: %d\n", cfg.threadsNum);
	printf("Verbose: %d\n", cfg.verbose);
	printf("----------------------\n");
	return;
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
