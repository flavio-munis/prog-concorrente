/*-----------------------------------------------------------------*/
/**

  @file    gera_vets.c
  @author  Flávio M.
  @brief   Gera dois vetores com valores aleatorios e escreve em um
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
#include <string.h>
#include <errno.h>
#include <time.h>


/*-----------------------------------------------------------------
                  Internal Functions Declarations
  -----------------------------------------------------------------*/


/*-----------------------------------------------------------------*/
/**
   @brief  Add a New Element to Vector.
   @param  double*      Pointer To Array.
   @param  unsigned int Total Element to be Added.
*/
/*-----------------------------------------------------------------*/
void addElements(float*, unsigned int);


/*-----------------------------------------------------------------*/
/**
   @brief Print All Elements of Array. 
   @param double*      Pointer To Array.
   @param unsigned int Size of array.
*/
/*-----------------------------------------------------------------*/
void printVec(float*, unsigned int);


/*-----------------------------------------------------------------*/
/**
   @brief  Function Executed By pthread, Sums +1 to All Positions.
   @param  double* Pointer to Array 1.
   @param  double* Pointer to Array 2.
   @return double  Internal Product Between the Two Vectors.
*/
/*-----------------------------------------------------------------*/
double internalProduct(float*, float*, unsigned int);


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
   @return bool If Args  Are Valid.
*/
/*-----------------------------------------------------------------*/
bool checkArgs(int, char*, unsigned int*);


/*-----------------------------------------------------------------
                      Functions Implementation
  -----------------------------------------------------------------*/

void printVec(float* vec, unsigned int size) {

	puts("\n=== Info ===");
	printf("Vec Size: %u\n", size);
	printf("Vec Elements: ");
	
	for(unsigned int i = 0; i < size; i++)
		printf("%f ", vec[i]);

	puts("");
}

void addElements(float* vec, unsigned int size) {

	static bool seedIinitiliazed = false;
	int rangeOfNums = 1000.0;
	short int fator = 1;

    if(!seedIinitiliazed){
		srand(time(NULL));
		seedIinitiliazed = true;
	}
	
	for(unsigned int i = 0; i < size; i++) {
	    vec[i] = (rand() % rangeOfNums)/3.0 * fator;
		fator *= -1;
	}
}
	
double internalProduct(float* vec1, float* vec2, unsigned int size) {

	double result = 0.0;

	for(unsigned int i = 0; i < size; i++)
		result += vec1[i] * vec2[i];
	
	return result;
}

bool checkArgs(int argc,
			   char* argv[],
			   unsigned int* arrSize) {

	unsigned int arr;

	if (argc != 3) {
		puts("Uso: \n  ./sum_array [arr_size] [output_file]");
	    return false;
	}
	
	arr = atol(argv[1]);
	if(arr < 1){
		puts("Invalid Array Size!");
	    return false;
	}

	*arrSize = arr;

	return true;
}

void writeToFile(void* info, size_t size, size_t total, FILE* out) {

	size_t ret;

	//escreve os elementos do vetor
	ret = fwrite(info, size, total, out);

	if(ret < total)
	    perror("Error in Writing To File");
}

int main(int argc, char* argv[]) {

	unsigned int arrSize;
    float* vec1, *vec2;
	double int_product;
	FILE* output;
	
	if(!checkArgs(argc, argv, &arrSize))
		exit(-1);

	vec1 = malloc(sizeof(float) * arrSize);
	vec2 = malloc(sizeof(float) * arrSize);
	
	if(!vec1 || !vec2)
		perror("Error Allocating Memory For Vector!");

	addElements(vec1, arrSize);
	addElements(vec2, arrSize);

	//printVec(vec1, arrSize);
	//printVec(vec2, arrSize);

	int_product = internalProduct(vec1, vec2, arrSize);
	printf("\nInternal Product: %f\n", int_product);
	
	output = fopen(argv[2], "wb");

	if(!output) 
	    perror("Error Opening in File!");

	// Escreve o Tamanho dos Vetores
    writeToFile(&arrSize, sizeof(long int), 1, output);

	// Escreve os Elementos dos Vetores
	writeToFile(vec1, sizeof(float) ,arrSize, output);
	writeToFile(vec2, sizeof(float), arrSize, output);
	
	// Escreve o Produto Interno
    writeToFile(&int_product, sizeof(double), 1, output);

	// Close File Descriptor
	fclose(output);
	
	// Free Vectors
	free(vec1);
	free(vec2);
	
	return 0;
}
