#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

#define RESET   "\033[0m"
#define BOLD    "\033[1m"
#define RED     "\033[31m"
#define CYAN    "\033[36m"

#define MATRIX_VALUE_RANGE_MAX 10

#define DEFAULT_MATRIX_SIZE 1000
#define DEFAULT_FUNCTION_IDX 1
#define DEFAULT_THREADS_NUM 1
#define DEFAULT_BLOCK_SIZE 40
#define DEFAULT_PARALLEL_FLAG 0
#define DEFAULT_VERBOSE_FLAG 0
#define DEFAULT_HELP_SELECTED_FLAG 0

typedef struct {
	size_t matSize;
	int functionIdx;
	int threadsNum;
	size_t blockSize;
	int parallel;
	int verbose;
	int helpSelected;
} Settings;

void run(Settings cfg, double *matA, double *matB, double *matC);
void matrixMult0(double *mat1, double *mat2, double *outputMat, size_t size);
void matrixMult0parallel(double *mat1, double *mat2, double *outputMat, size_t size);
void matrixMult1(double *mat1, double *mat2, double *outputMat, size_t size);
void matrixMult1parallel(double *mat1, double *mat2, double *outputMat, size_t size);
void matrixMult2(double *mat1, double *mat2, double *outputMat, size_t size, size_t blockSize);
void matrixMult2parallel(double *mat1, double *mat2, double *outputMat, size_t size, size_t blockSize);
void matrixMult3(double *mat1, double *mat2, double *outputMat, size_t size, size_t blockSize);
void matrixMult3parallel(double *mat1, double *mat2, double *outputMat, size_t size, size_t blockSize);
void printMatrix(double *mat, size_t size);
void printSettings(Settings cfg);
void printHelp();
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
	cfg.blockSize = DEFAULT_BLOCK_SIZE;
	cfg.parallel = DEFAULT_PARALLEL_FLAG;
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
		} else if (strcmp(argv[i], "-b") == 0) {
			cfg.blockSize = atoi(argv[++i]);
		} else if (strcmp(argv[i], "-v") == 0) {
			cfg.verbose = 1;
		} else if (strcmp(argv[i], "-p") == 0) {
			cfg.parallel = 1;
		} else if (strcmp(argv[i], "--help") == 0) {
			cfg.helpSelected = 1;
			printHelp();
		}
	}
	
	if(!cfg.helpSelected) {
		matA = malloc(cfg.matSize*cfg.matSize*sizeof(*matA));
		matB = malloc(cfg.matSize*cfg.matSize*sizeof(*matB));
		matC = malloc(cfg.matSize*cfg.matSize*sizeof(*matC));
		if (matA != NULL && matB != NULL && matC != NULL)
			run(cfg, matA, matB, matC);
		else
			printf(RED "Memory Allocation Error! MatSize: %zu\n" RESET, cfg.matSize);
	}
	
	return 0;
}

void run(Settings cfg, double *matA, double *matB, double *matC) {
	int errorDetected = 0;
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
			omp_set_num_threads(cfg.threadsNum);
			elapsedTime = omp_get_wtime();
			if (cfg.parallel)
				matrixMult0parallel(matA, matB, matC, cfg.matSize);
			else
				matrixMult0(matA, matB, matC, cfg.matSize);
			elapsedTime = omp_get_wtime() - elapsedTime;
			break;
		case 1: // ikj solution
			omp_set_num_threads(cfg.threadsNum);
			elapsedTime = omp_get_wtime();
			if (cfg.parallel)
				matrixMult1parallel(matA, matB, matC, cfg.matSize);
			else
				matrixMult1(matA, matB, matC, cfg.matSize);
			elapsedTime = omp_get_wtime() - elapsedTime;
			break;
		case 2: // Blocked iikkjj-ikj solution
			omp_set_num_threads(cfg.threadsNum);
			elapsedTime = omp_get_wtime();
			if (cfg.parallel)
				matrixMult2parallel(matA, matB, matC, cfg.matSize, cfg.blockSize);
			else
				matrixMult2(matA, matB, matC, cfg.matSize, cfg.blockSize);
			elapsedTime = omp_get_wtime() - elapsedTime;
			break;
		case 3: // Blocked iijjkk-ikj solution
			omp_set_num_threads(cfg.threadsNum);
			elapsedTime = omp_get_wtime();
			if (cfg.parallel)
				matrixMult3parallel(matA, matB, matC, cfg.matSize, cfg.blockSize);
			else
				matrixMult3(matA, matB, matC, cfg.matSize, cfg.blockSize);
			elapsedTime = omp_get_wtime() - elapsedTime;
			break;
		default:
			errorDetected = 1;
			printf(RED "Function %d not available!\n" RESET, cfg.functionIdx);
	}
	
	if (!errorDetected) {
		if (cfg.verbose) printf("Execution complete!\n" CYAN "Time taken: ");

		if (cfg.verbose) printf("%lf seconds\n" RESET, elapsedTime);
		else printf("%lf\n", elapsedTime);
	}
	
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

void matrixMult0parallel(double *mat1, double *mat2, double *outputMat, size_t size) {
	size_t i, j, k;
	double partialSum;
	
	#pragma omp parallel for collapse(2) private(k, partialSum)
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

void matrixMult1parallel(double *mat1, double *mat2, double *outputMat, size_t size) {
	size_t i, k, j;
	
	#pragma omp parallel for private(k, j)
	for(i = 0; i < size; i++) {
		for(k = 0; k < size; k++) {
			for(j = 0; j < size; j++) {
				outputMat[i * size + j] += mat1[i * size + k] * mat2[k * size + j];
			}
		}
	}
	return;
}

void matrixMult2(double *mat1, double *mat2, double *outputMat, size_t size, size_t blockSize) {
	size_t ii, kk, jj, i, k ,j, iMax, kMax, jMax;
	
	for(ii = 0; ii < size; ii += blockSize) {
		if(ii + blockSize < size) iMax = ii + blockSize; else iMax = size;
		for(kk = 0; kk < size; kk += blockSize) {
			if(kk + blockSize < size) kMax = kk + blockSize; else kMax = size;
			for(jj = 0; jj < size; jj += blockSize) {
				if(jj + blockSize < size) jMax = jj + blockSize; else jMax = size;
				
				for(i = ii; i < iMax; i++) {
					for(k = kk; k < kMax; k++) {
						for(j = jj; j < jMax; j++) {
							outputMat[i * size + j] += mat1[i * size + k] * mat2[k * size + j];
						}
					}						
				}
				
			}				
		}	
	}
	return;
}

void matrixMult2parallel(double *mat1, double *mat2, double *outputMat, size_t size, size_t blockSize) {
	size_t ii, kk, jj, i, k ,j, iMax, kMax, jMax;
	
	#pragma omp parallel for private(kk, jj, i, k, j, iMax, kMax, jMax)
	for(ii = 0; ii < size; ii += blockSize) {
		if(ii + blockSize < size) iMax = ii + blockSize; else iMax = size;
		for(kk = 0; kk < size; kk += blockSize) {
			if(kk + blockSize < size) kMax = kk + blockSize; else kMax = size;
			for(jj = 0; jj < size; jj += blockSize) {
				if(jj + blockSize < size) jMax = jj + blockSize; else jMax = size;
				
				for(i = ii; i < iMax; i++) {
					for(k = kk; k < kMax; k++) {
						for(j = jj; j < jMax; j++) {
							outputMat[i * size + j] += mat1[i * size + k] * mat2[k * size + j];
						}
					}						
				}
				
			}				
		}	
	}
	return;
}

void matrixMult3(double *mat1, double *mat2, double *outputMat, size_t size, size_t blockSize) {
	size_t ii, jj, kk, i, k ,j, iMax, kMax, jMax;
	
	for(ii = 0; ii < size; ii += blockSize) {
		if(ii + blockSize < size) iMax = ii + blockSize; else iMax = size;
		for(jj = 0; jj < size; jj += blockSize) {
			if(jj + blockSize < size) jMax = jj + blockSize; else jMax = size;
			for(kk = 0; kk < size; kk += blockSize) {
				if(kk + blockSize < size) kMax = kk + blockSize; else kMax = size;
				
				for(i = ii; i < iMax; i++) {
					for(k = kk; k < kMax; k++) {
						for(j = jj; j < jMax; j++) {
							outputMat[i * size + j] += mat1[i * size + k] * mat2[k * size + j];
						}
					}						
				}
				
			}				
		}	
	}
	return;
}

void matrixMult3parallel(double *mat1, double *mat2, double *outputMat, size_t size, size_t blockSize) {
	size_t ii, jj, kk, i, k ,j, iMax, kMax, jMax;
	
	#pragma omp parallel for collapse(2) private(kk, i, k, j, iMax, kMax, jMax)
	for(ii = 0; ii < size; ii += blockSize) {
		for(jj = 0; jj < size; jj += blockSize) {
			if(ii + blockSize < size) iMax = ii + blockSize; else iMax = size;
			if(jj + blockSize < size) jMax = jj + blockSize; else jMax = size;
			for(kk = 0; kk < size; kk += blockSize) {
				if(kk + blockSize < size) kMax = kk + blockSize; else kMax = size;
				
				for(i = ii; i < iMax; i++) {
					for(k = kk; k < kMax; k++) {
						for(j = jj; j < jMax; j++) {
							outputMat[i * size + j] += mat1[i * size + k] * mat2[k * size + j];
						}
					}						
				}
				
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
	printf("Block Size: %zu\n", cfg.blockSize);
	printf("Parallel: %d\n", cfg.parallel);
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
	printf("║  " CYAN "-b [int]" RED " | Set Block Size                    ║\n");
	printf("║  " CYAN "-p      " RED " | Parallel Execution                ║\n");
	printf("║  " CYAN "-v      " RED " | Verbose Execution                 ║\n");
	printf("╚═══════════════════════════════════════════════╝\n");
	printf("╔════════════" CYAN BOLD " AVAILABLE FUNCTIONS " RESET RED "══════════════╗\n");
	printf("║  " CYAN "0" RED " | IJK Solution                             ║\n");
	printf("║  " CYAN "1" RED " | IKJ Solution                             ║\n");
	printf("║  " CYAN "2" RED " | Blocked IIKKJJ-IKJ Solution              ║\n");
	printf("║  " CYAN "3" RED " | Blocked IIJJKK-IKJ Solution              ║\n");
	printf("╚═══════════════════════════════════════════════╝\n" RESET);
	return;
}