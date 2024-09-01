/*-----------------------------------------------------------------*/
/**

  @file    gera_matrizes.c
  @author  Flávio M.
  @brief   Gera duas matrizes (M X N e N X M) com valores aleatorios e escreve em um
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
   @brief  Add New Elements to a Matrix.
   @param  float*      Pointer To Matrix.
   @param  unsigned int Total Rows.
   @param  unsigned int Total Columns.
*/
/*-----------------------------------------------------------------*/
void addElements(float**, unsigned int, unsigned int);


/*-----------------------------------------------------------------*/
/**
   @brief Print All Elements of a Matrix. 
   @param double*      Pointer To Matrix.
   @param  unsigned int Total Rows.
   @param  unsigned int Total Columns.
*/
/*-----------------------------------------------------------------*/
void printMatrix(float**, unsigned int, unsigned int);


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
   @brief Check if a String of Arguments is Valid.
   @param  int           Total Arguments in String (argc).
   @param  char*         String of Arguments (argv).
   @param  unsigned int* Pointer For Data Be Written.
   @param  unsigned int* Pointer For Data Be Written.
*/
/*-----------------------------------------------------------------*/
void checkArgs(int, char*[], unsigned int*, unsigned int*);


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

void addElements(float** matriz,
				 unsigned int rows,
				 unsigned int columns) {

	static bool seedIinitiliazed = false;
	int rangeOfNums = 10000.0;
	short int fator = 1;

    if(!seedIinitiliazed){
		srand(time(NULL));
		seedIinitiliazed = true;
	}
	
	for(unsigned int i = 0; i < rows; i++) {
		for(unsigned int j = 0; j < columns; j++) {
		    matriz[i][j] = (rand() % rangeOfNums)/7.0 * fator;
			fator *= -1;
		}
	}
}

void checkArgs(int argc,
			   char* argv[],
			   unsigned int* m,
			   unsigned int* n) {

	unsigned int rows, columns;

	if (argc != 4) {
		invalidArgumentError("Usage: \n  ./[program] [rows] [columns] [output_file]");
	}
	
    rows = atol(argv[1]);
	columns = atol(argv[2]);
	
	if(rows < 1 || columns < 1) {
		invalidArgumentError("Invalid Arguments! \nUsage: 1 < rows && columns < 4,294,967,295");
	}

	*m = rows;
	*n = columns;
}

void writeToFile(void* info, size_t size, size_t total, FILE* out) {

	size_t ret;

	//escreve os elementos do vetor
	ret = fwrite(info, size, total, out);

	if(ret < total) {
	    unexpectedError("Error in Writing To File!");
	}
}

int main(int argc, char* argv[]) {

	unsigned int m, n;
    float** matriz1, **matriz2;
	FILE* output;
	
    checkArgs(argc, argv, &m, &n);

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

	addElements(matriz1, m, n);
	addElements(matriz2, n, m);

	//printMatrix(matriz1, m, n);
	//printMatrix(matriz2, n, m);
	
	output = fopen(argv[3], "wb");
	checkNullFilePointer((void*) output);

	// Escreve a Quantindade de Linhas e Colunas
    writeToFile(&m, sizeof(unsigned int), 1, output);
	writeToFile(&n, sizeof(unsigned int), 1, output);
	
	// Escreve os Elementos das Matrizes
	for(unsigned int i = 0; i < m; i++) 
		writeToFile(matriz1[i], sizeof(float), n, output);
	
	for(unsigned int i = 0; i < n; i++) 
		writeToFile(matriz2[i], sizeof(float), m, output);

	// Close File Descriptor
	fclose(output);

	// Check if its a square matrix
	if(m != n) {
		for(unsigned int i = 0; i < m; i++)
			free(matriz1[i]);

		for(unsigned int i = 0; i < n; i++)
			free(matriz2[i]);
		
	} else{
		for(unsigned int i = 0; i < m; i++) {
			free(matriz1[i]);
			free(matriz2[i]);
		}
	}
	
	free(matriz1);
	free(matriz2);
	
	return 0;
}
