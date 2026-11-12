#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <omp.h>

#define RESET   "\033[0m"
#define BOLD    "\033[1m"
#define RED     "\033[31m"
#define CYAN    "\033[36m"

#define MAX_N 20000
#define MATRIX_VALUE_RANGE_MAX 10

#define DEFAULT_MATRIX_SIZE 1000
#define DEFAULT_FUNCTION_IDX 1
#define DEFAULT_THREADS_NUM 1
#define DEFAULT_VERBOSE_FLAG 0
#define DEFAULT_HELP_SELECTED_FLAG 0

#define NUM_OF_AVAILABLE_FUNCTIONS 3

typedef struct {
	size_t matSize;
	int functionIdx;
	int threadsNum;
	int verbose;
	int helpSelected;
} Settings;

void run(Settings cfg, double *matA, double *matB, double *matC);
void printMatrix(double *mat, size_t size);
void printSettings(Settings cfg);
void printHelp();
void matrixMult0(double *mat1, double *mat2, double *outputMat, size_t size);
void matrixMult1(double *mat1, double *mat2, double *outputMat, size_t size);
void matrixMult2(double *mat1, double *mat2, double *outputMat, size_t size);
void randomFillMatrix(double *mat, size_t size);
void zeroMatrix(double *mat, size_t size);

/*TO DO

*/

int main(int argc, char **argv) {
	Settings cfg;
	int i;
	double *matA, *matB, *matC;
	
	srand(42);
	
	// Default config
	
	cfg.matSize = DEFAULT_MATRIX_SIZE;
	cfg.functionIdx = DEFAULT_FUNCTION_IDX;
	cfg.threadsNum = DEFAULT_THREADS_NUM;
	cfg.verbose = DEFAULT_VERBOSE_FLAG;
	cfg.helpSelected = DEFAULT_HELP_SELECTED_FLAG;
	
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
		} else if (strcmp(argv[i], "--help") == 0) {
			cfg.helpSelected = 1;
			printHelp();
		}
	}
	
	if(!cfg.helpSelected) {
		matA = malloc(cfg.matSize*cfg.matSize*sizeof(*matA));
		matB = malloc(cfg.matSize*cfg.matSize*sizeof(*matB));
		matC = malloc(cfg.matSize*cfg.matSize*sizeof(*matC));
		run(cfg, matA, matB, matC);
	}
	
	return 0;
}

void run(Settings cfg, double *matA, double *matB, double *matC) {
	double elapsedTime = 0;
	
	if (cfg.verbose) printSettings(cfg);
	
	// Matrix setup
	
	if (cfg.verbose) printf("Setting up matrices of size %zu ...\n", cfg.matSize);
	
	randomFillMatrix(matA, cfg.matSize);
	randomFillMatrix(matB, cfg.matSize);
	
	zeroMatrix(matC, cfg.matSize);
	
	if (cfg.verbose) printf("Matrices set up!\n");
	
	// Operation
	if (cfg.verbose) printf("Executing function: %d ...\n", cfg.functionIdx);
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
	
	if (cfg.verbose) printf("Execution complete!\n" CYAN "Processor time taken: ");

	if (cfg.verbose) printf("%lf seconds\n" RESET, elapsedTime);
	else printf("%lf\n", elapsedTime);
	
	free(matA);
	free(matB);
	free(matC);
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

void randomFillMatrix(double *mat, size_t size) {
	size_t i, j;
	for(i = 0; i < size; i++) {
		for(j = 0; j < size; j++) {
			mat[i * size + j] = rand() % MATRIX_VALUE_RANGE_MAX;
		}
	}
	return;
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

void printHelp() {
	printf(CYAN" __  __       _   __  __       _  ___        _\n");
	printf("|  \\/  | __ _| |_|  \\/  |_   _| |/ _ \\ _ __ | |_\n");
	printf("| |\\/| |/ _` | __| |\\/| | | | | | | | | '_ \\| __|\n");
	printf("| |  | | (_| | |_| |  | | |_| | | |_| | |_) | |_\n");
	printf("|_|  |_|\\__,_|\\__|_|  |_|\\__,_|_|\\___/| .__/ \\__|\n");
	printf("                                      |_|\n");
	printf(RED "╔═══════════════" CYAN BOLD" PROGRAM OPTIONS " RESET RED "═══════════════╗\n");
	printf("║  " CYAN "-n [int]" RED " | Set Matrix Size                   ║\n");
	printf("║  " CYAN "-f [int]" RED " | Select Function Index             ║\n");
	printf("║  " CYAN "-t [int]" RED " | Set Thread Amount                 ║\n");
	printf("║  " CYAN "-v      " RED " | Verbose Execution                 ║\n");
	printf("╚═══════════════════════════════════════════════╝\n");
	printf("╔════════════" CYAN BOLD " AVAILABLE FUNCTIONS " RESET RED "══════════════╗\n");
	printf("║  " CYAN "0" RED " | IJK Solution                             ║\n");
	printf("║  " CYAN "1" RED " | IKJ Solution                             ║\n");
	printf("║  " CYAN "2" RED " | Parallel IKJ Solution                    ║\n");
	printf("╚═══════════════════════════════════════════════╝\n" RESET);
	return;
}

