/*-----------------------------------------------------------------*/
/**

  @file    gera_matrizes.c
  @author  Flávio M.
  @brief   Gera duas matrizes com valores aleatorios e escreve em um
           arquivo binário.
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
#include "error-handler.h"


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
   @param  int             Total Arguments in String (argc).
   @param  char*           String of Arguments (argv).
   @param  unsigned short* Pointer For Data Be Written.
*/
/*-----------------------------------------------------------------*/
void checkArgs(int, char*[], unsigned short*);


/*-----------------------------------------------------------------
                      Functions Implementation
  -----------------------------------------------------------------*/

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
			   unsigned short* n_threads) {

	unsigned int threads;

	if (argc != 4) {
		invalidArgumentError("Usage: \n  ./[program] [input_file] [output_file] [threads]");
	}
	
    threads = atoi(argv[3]);
	
	if(threads < 1) {
		invalidArgumentError("Invalid Arguments! \nUsage: 1 < threads < 65535");
	}

	*n_threads = threads;
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

float** multMatrix(float** matriz1,
				   float** matriz2,
				   unsigned int m,
				   unsigned int n) {
	float** result;

	result = (float**) malloc(sizeof(float*) * m);
	checkNullPointer((void*) result);

	for(unsigned int i = 0; i < m; i++) {
		result[i] = (float*) malloc(sizeof(float) * m);
		checkNullPointer((void*) result);
	}

	for(unsigned int i = 0; i < m; i++) {
		for(unsigned int j = 0; j < m; j++) {
			result[i][j] = 0;
			
			for(unsigned int k = 0; k < n; k++) {

				result[i][j] += matriz1[i][k] * matriz2[k][j];
			}
		}
	}
	
	return result;
}

int main(int argc, char* argv[]) {

	unsigned int m, n;
	unsigned short threads;
    float** matriz1, **matriz2;
	float** result;
	
    checkArgs(argc, argv, &threads);

    getInputData(argv[1], &m, &n, &matriz1, &matriz2);

	result = multMatrix(matriz1, matriz2, m, n);

	writeOutput(argv[2], m, result);
	
	//printMatrix(matriz1, m, n);
	//printMatrix(matriz2, n, m);
	//printMatrix(result, m, m);

    // Check if its a square matrix
	if(m != n) {
		for(unsigned int i = 0; i < m; i++){
			free(matriz1[i]);
			free(result);
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
	
	return 0;
}
