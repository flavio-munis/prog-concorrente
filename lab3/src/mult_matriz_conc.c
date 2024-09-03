/*-----------------------------------------------------------------*/
/**

  @file    mult_matriz_conc.c
  @author  Flávio M.
  @brief   Multiplica Duas Matrizes (M X N) e (N X M) de forma concorrente
           e escreve a matriz resultado num arquivo binário.
  @Materia Prog Concorrente (ICP361)

 */
/*-----------------------------------------------------------------*/

/*-----------------------------------------------------------------
                              Includes
  -----------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <time.h>
#include <pthread.h>
#include "timer.h"
#include "error-handler.h"


/*-----------------------------------------------------------------
                              Structs
  -----------------------------------------------------------------*/
typedef struct multInfo {
    unsigned int startRow;
    unsigned int endRow;
	unsigned int n;
	unsigned int m;
	float** m1;
	float** m2;
	float** result;
} MultInfo;


/*-----------------------------------------------------------------
                  Internal Functions Declarations
  -----------------------------------------------------------------*/


/*-----------------------------------------------------------------*/
/**
   @brief Print All Elements of a Matrix. 
   @param double*      Pointer To Matrix.
   @param unsigned int Total Rows.
   @param unsigned int Total Columns.
*/
/*-----------------------------------------------------------------*/
void printMatrix(float**, unsigned int, unsigned int);


/*-----------------------------------------------------------------*/
/**
   @brief Get Data From a Binary Input File. 
   @param char*         Input File Path.
   @param unsigned int* Pointer for Rows Info be Written.
   @param unsigned int* Pointer for Columns Info be Written.
   @param float***      Pointer for Matrix Info be Written.
   @param float***      Pointer for Matrix Info be Written.
*/
/*-----------------------------------------------------------------*/
void getInputData(char*,
				  unsigned int*,
				  unsigned int*,
				  float***,
				  float***);


/*-----------------------------------------------------------------*/
/**
   @brief Write Data to Binary File 
   @param char*        Output File Path.
   @param unsigned int Columns/Rows as The Result Matrix Will Always
                       Be Squared.
   @param float**      Result Matrix.
*/
/*-----------------------------------------------------------------*/
void writeOutput(char*, unsigned int, float**);


/*-----------------------------------------------------------------*/
/**
   @brief Write a Information To a File.
   @param void*  Info To Be Written.
   @param size_t Data Type Size.
   @param size_t Total Elements to Be Written.
   @param FILE*  File Descriptor.
*/
/*-----------------------------------------------------------------*/
void writeToFile(void*, size_t, size_t, FILE*);


/*-----------------------------------------------------------------*/
/**
   @brief Read Information From a Fd.
   @param void*  Destination (Must Have Enogh Memory Allocated).
   @param size_t Size of Data To Be Read.
   @oaram size_t Total Elements to Br Read.
   @param FILE*  File Descriptor.
*/
/*-----------------------------------------------------------------*/
void readFromFile(void*, size_t, size_t, FILE*);


/*-----------------------------------------------------------------*/
/**
   @brief Check if a String of Arguments is Valid.
   @param int             Total Arguments in String (argc).
   @param char*           String of Arguments (argv).
   @param unsigned short* Pointer to Threads.
*/
/*-----------------------------------------------------------------*/
void checkArgs(int, char*[], unsigned short*);


/*-----------------------------------------------------------------*/
/**
   @brief Multiply Two Matrices (M x N) and (N x M)
   @param  float**      Matrix (M x N).
   @param  float**      Matrix (N x M).
   @param  unsigned int M.
   @param  unsigned int N.
   @return float**      Result Matrix (M x M).
*/
/*-----------------------------------------------------------------*/
void* multMatrix(void*);


/*-----------------------------------------------------------------
                      Functions Implementation
  -----------------------------------------------------------------*/

MultInfo* initMultInfo() {

	MultInfo* newInfo = (MultInfo*) malloc(sizeof(MultInfo));
	checkNullPointer((void*) newInfo);

	newInfo -> m1 = NULL;
	newInfo -> m2 = NULL;
	newInfo -> result = NULL;
	
	return newInfo;
}

void printInfo(MultInfo* info) {

	unsigned int start = info -> startRow;
	unsigned int end = info -> endRow;
	unsigned int n = info -> n;
	unsigned int m = info -> m;
	float** m1 = info -> m1;
	float** m2 = info -> m2;
	unsigned int inter = end - start;
	
	puts("\n=== Info ===");
	printf("M1 Interval Size: %u\n", inter);
    puts("Result Matrix: ");

	
    for(unsigned int i = start; i < end; i++) {

		float* currM1 = m1[i];

		if(i == start)
			printf("[");
		
		for(unsigned int j = 0; j < m; j++) {
			if(!j)
				printf("[");

			for(unsigned int k = 0; k < n; k++) {
				if(k == n - 1)
					printf("%f * %f", currM1[k], m2[k][j]);
				else
					printf("%f * %f + ", currM1[k], m2[k][j]);
			}

			if(j == m - 1)
				printf("]");
			else
				printf(", ");

			if(i != end - 1)
				printf("\n");
		}

		if(i == end - 1)
			printf("]\n");
	}

	puts("");
}

void printMatrix(float** matriz,
				 unsigned int rows,
				 unsigned int columns) {

	puts("\n=== Info ===");
	printf("Matrix Size: %ux%u\n", rows, columns);
	printf("Matrix Total Elments: %u\n", rows * columns);
    puts("Matrix Elments: ");
	
	for(unsigned int i = 0; i < rows; i++) {

		if(!i)
			printf("[");

		for(unsigned int j = 0; j < columns; j++) {
			if(!j)
				printf("[");

			if(j == columns - 1)
				printf("%f]", matriz[i][j]);
			else
				printf("%f, ", matriz[i][j]);
		}
		
		if(i == rows - 1)
			printf("]");
		
		puts("");
	}

	puts("");
}

void readFromFile(void* dest, size_t size, size_t total, FILE* fd) {

	size_t ret;

	//escreve os elementos do vetor
	ret = fread(dest, size, total, fd);

	if(ret < total) {
	    unexpectedError("Error in Reading From File");
	}
}

void writeToFile(void* info, size_t size, size_t total, FILE* out) {

	size_t ret;

	//escreve os elementos do vetor
	ret = fwrite(info, size, total, out);

	if(ret < total) {
	    unexpectedError("Error in Writing To File!");
	}
}

void checkArgs(int argc,
			   char* argv[],
			   unsigned short* threads) {

	unsigned int th;
	
	if (argc != 4) {
		invalidArgumentError("Usage: \n  ./[program] [input_file] [output_file] [threads]");
	}

	th = atoi(argv[3]);
	
	if(th < 1 || th > 65536) {
		invalidArgumentError("Invalid Number of Threads! 1 < Threads < 65536");
	}

	*threads = th;
}

void getInputData(char* inputPath,
				  unsigned int* mSize,
				  unsigned int* nSize,
				  float*** refMatriz1,
				  float*** refMatriz2) {

	float** matriz1, **matriz2;
	unsigned int m, n;
	FILE* input;
	
	input = fopen(inputPath, "rb");
	checkNullFilePointer((void*) input);

	// Le Linhas e Colunas
	readFromFile(&m, sizeof(unsigned int), 1, input);
	readFromFile(&n, sizeof(unsigned int), 1, input);

    // Aloca Memória Para as Matrizes
    matriz1 = (float**) malloc(sizeof(float*) * m);
	matriz2 = (float**) malloc(sizeof(float*) * n);
	
	checkNullPointer((void*) matriz1);
	checkNullPointer((void*) matriz2);

	// Check if it's a square matrix
	if(m != n) {
		for(unsigned int i = 0; i < m; i++) {
			matriz1[i] = (float*) malloc(sizeof(float) * n);
			checkNullPointer((void*) matriz1[i]);
		}
	
		for(unsigned int i = 0; i < n; i++) {
			matriz2[i] = (float*) malloc(sizeof(float) * m);
			checkNullPointer((void*) matriz2[i]);
		}
		
	} else {
		for(unsigned int i = 0; i < m; i++) {
			matriz1[i] = (float*) malloc(sizeof(float) * n);
			matriz2[i] = (float*) malloc(sizeof(float) * n);
			
			checkNullPointer((void*) matriz1[i]);
			checkNullPointer((void*) matriz2[i]);
		}
	}

	// Escreve os Elementos das Matrizes
	for(unsigned int i = 0; i < m; i++)
	    readFromFile(matriz1[i], sizeof(float), n, input);
	
	for(unsigned int i = 0; i < n; i++)
	    readFromFile(matriz2[i], sizeof(float), m, input);
	
	fclose(input);
	
	*mSize = m;
	*nSize = n;
	*refMatriz1 = matriz1;
	*refMatriz2 = matriz2;
}

void writeOutput(char* outputPath,  unsigned int m, float** result) {

	FILE* output = fopen(outputPath, "wb");
	checkNullFilePointer((void*) output);

	// Escreve a Quantindade de Linhas e Colunas
    writeToFile(&m, sizeof(unsigned int), 1, output);
	writeToFile(&m, sizeof(unsigned int), 1, output);

	//Escreve Elementos da Matriz
	for(unsigned int i = 0; i < m; i++)
		writeToFile(result[i], sizeof(float), m, output);

	// Close File Descriptor
	fclose(output);
}

void* multMatrix(void* arg) {
	MultInfo* info = (MultInfo*) arg;
	unsigned int start = info -> startRow;
	unsigned int end = info -> endRow;
	unsigned int n = info -> n;
	unsigned int m = info -> m;
	float** m1 = info -> m1;
	float** m2 = info -> m2;
	unsigned int inter = end - start;
	float** result;
	float soma = 0.0;
	
	result = (float**) malloc(sizeof(float*) * inter);
	checkNullPointer((void*) result);

	for(unsigned int i = 0; i < inter; i++) {
		result[i] = (float*) malloc(sizeof(float) * m);
		checkNullPointer((void*) result);
	}
	
    for(unsigned int i = start; i < end; i++) {

		float* currM1 = m1[i];
		
		for(unsigned int j = 0; j < m; j++) {

			soma = 0.0;
			
			for(unsigned int k = 0; k < n; k++)
				soma += currM1[k] * m2[k][j];

			result[i - start][j] = soma;
		}
	}

	info -> result = result;
	
	return (void*) info;
}

int main(int argc, char* argv[]) {

	unsigned int m, n;
	unsigned short threads;
    float** matriz1, **matriz2;
	float** result;
	MyTimer* timerIORead, *timerIOWrite, *timerMult;
	
    checkArgs(argc, argv, &threads);

	INIT_TIMER(timerIORead);
    getInputData(argv[1], &m, &n, &matriz1, &matriz2);
    END_TIMER(timerIORead);

	if(threads > m) {
		invalidArgumentError("More Threads Than Rows, Insert A Valid Number of Threads!");
	}
		
	pthread_t th[threads];
	unsigned int rowsPerPart = m / threads;
	unsigned int start, end;

	//printMatrix(matriz1, m, n);
	//printMatrix(matriz2, n, m);

	INIT_TIMER(timerMult);
	
	for(unsigned short i = 0; i < threads; i++) {

		MultInfo* info = initMultInfo();

	    // Calc Current Chunk To Send To Thread
		start = rowsPerPart * i;

		if (i == threads - 1)
			end = m;
		else
			end = rowsPerPart * (i + 1);

		info -> startRow = start;
		info -> endRow = end;
		info -> n = n;
		info -> m = m;
		info -> m1 = matriz1;
		info -> m2 = matriz2;

		//printInfo(info);
		
		if(pthread_create(th + i, NULL, &multMatrix, (void*) info) != 0) {
			unexpectedError("Error Creating Threads!");
		}
	}

	result = (float**) malloc(sizeof(float*) * m);
	checkNullPointer((void*) result);
	
	for(unsigned short i = 0; i < threads; i++) {

	    MultInfo* retInfo;
	    float** tempResult;
		unsigned int startInfo, endInfo;
		
		if(pthread_join(th[i], (void**) &retInfo) != 0) {
			unexpectedError("Error Joining Threads!");
		}

		startInfo = retInfo -> startRow;
		endInfo = retInfo -> endRow;
		tempResult = retInfo -> result;
		
		for(unsigned int j = startInfo; j < endInfo; j++) 
			result[j] = tempResult[j - startInfo];
		

		free(tempResult);
		free(retInfo);
	}

	END_TIMER(timerMult);
	//printMatrix(result, m, m);
	
	INIT_TIMER(timerIOWrite);
	writeOutput(argv[2], m, result);
	END_TIMER(timerIOWrite);
	
	// Check if its a square matrix
	if(m != n) {
		for(unsigned int i = 0; i < m; i++){
			free(matriz1[i]);
			free(result[i]);
		}
		
		for(unsigned int i = 0; i < n; i++)
			free(matriz2[i]);
		
	} else{
		for(unsigned int i = 0; i < m; i++) {
			free(matriz1[i]);
			free(matriz2[i]);
			free(result[i]);
		}
	}

	free(matriz1);
	free(matriz2);
	free(result);

	CALC_FINAL_TIME(timerIORead);
	CALC_FINAL_TIME(timerIOWrite);
	CALC_FINAL_TIME(timerMult);
	
	puts("=== Time Elapsed ===\n");
	printf("Read IO: %.5fs\n", timerIORead -> totalTime);
	printf("Mult.: %.5fs\n", timerMult -> totalTime);
	printf("Write IO: %.5fs\n", timerIOWrite -> totalTime);
	printf("Total Time: %.5fs\n", timerIOWrite -> totalTime +
		   timerIORead -> totalTime +
		   timerMult -> totalTime);
	
    
	free(timerIORead);
	free(timerMult);
	free(timerIOWrite);
	
	return 0;
}
