/*-----------------------------------------------------------------*/
/**

  @file    sum_one.c
  @author  Flávio M.
  @brief   Soma +1 a Cada Elemento do Array.
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
                              Structs
  -----------------------------------------------------------------*/

typedef struct vector {
	int* arr;
	size_t totalElements;
	unsigned int arrSize;
} Vector;


/*-----------------------------------------------------------------
                  Internal Functions Declarations
  -----------------------------------------------------------------*/

/*-----------------------------------------------------------------*/
/**
   @brief  Init a New Vector Struct.
   @param  unsigned int   Size of Array.
   @return Vector*        Pointer To Vector.
*/
/*-----------------------------------------------------------------*/
Vector* initVec(unsigned int);


/*-----------------------------------------------------------------*/
/**
   @brief  Free Memory Allocated To Vector.
   @param  Vector* Pointer to Vector
*/
/*-----------------------------------------------------------------*/
void freeVec(Vector*);


/*-----------------------------------------------------------------*/
/**
   @brief  Creates a Copy of a Vector.
   @param  Vector* Source Vector.
   @return Vector* Copy of Source Vector.
*/
/*-----------------------------------------------------------------*/
Vector* copyVec(Vector*);


/*-----------------------------------------------------------------*/
/**
   @brief Add a New Element to Vector.
   @param Vector*      Pointer To Vector.
   @param unsigned int Element to be Added.
*/
/*-----------------------------------------------------------------*/
void addElement(Vector*, unsigned int);


/*-----------------------------------------------------------------*/
/**
   @brief Print All Elements of Vector. 
   @param Vector* Pointer To Vector.
*/
/*-----------------------------------------------------------------*/
void printVec(Vector*);


/*-----------------------------------------------------------------*/
/**
   @brief Populated Vector With Random Values.
   @param Vector* Pointer to Vector.
*/
/*-----------------------------------------------------------------*/
void populateVec(Vector*);


/*-----------------------------------------------------------------*/
/**
   @brief  Function Executed By pthread, Sums +1 to All Positions.
   @param  Void* Pointer to Interval Struct Casted to Void*
*/
/*-----------------------------------------------------------------*/
void sum1ToVec(Vector*);


/*-----------------------------------------------------------------*/
/**
   @brief  Check if Problem Was Solved Correctly.
   @param  Vector* Original Vector.
   @param  Vector* Vector After Operations.
   @return bool    If Solution Is Right.
*/
/*-----------------------------------------------------------------*/
bool checkSolution(Vector*, Vector*);


/*-----------------------------------------------------------------
                      Functions Implementation
  -----------------------------------------------------------------*/

Vector* initVec(unsigned int size) {

	Vector* newVec = (Vector*) malloc(sizeof(Vector));
	if(!newVec)
		perror("Error in Creating Vector!");

	newVec -> arrSize = size;
	newVec -> totalElements = 0;
	newVec -> arr = (int*) malloc(sizeof(int) * size);
	if(!newVec -> arr)
		perror("Error in Allocating Array!");

	return newVec;
}

void freeVec(Vector* vec) {

	if(vec) {
		if(vec -> arr)
			free(vec -> arr);

		free(vec);
	}
}

void addElement(Vector* vec, unsigned int n) {
	
	vec -> arr[vec -> totalElements] = n;
	vec -> totalElements += 1;
}

Vector* copyVec(Vector* source) {

	Vector* cpyVec = NULL;
	
	if(source) {
		cpyVec = initVec(source -> arrSize);
		cpyVec -> totalElements = source -> totalElements;
		memcpy(cpyVec -> arr, source -> arr, source -> arrSize * sizeof(int));
	}

	return cpyVec;
}

void printVec(Vector* vec) {

	puts("\n=== Info ===");
	printf("Vec Size: %u\n", vec -> arrSize);
	printf("Vec Elements: %lu\n", vec -> totalElements);
	printf("Vec Elements: ");
	
	for(unsigned int i = 0; i < vec -> totalElements; i++)
		printf("%d ", vec -> arr[i]);

	puts("");
}

void populateVec(Vector* vec) {

	unsigned int seed = time(NULL);
	unsigned int totalElements = vec -> arrSize;
	int rangeOfNums = 100;
	
	for(unsigned int i = 0; i < totalElements; i++)
		addElement(vec, rand_r(&seed) % rangeOfNums);
}
	

void sum1ToVec(Vector* vec) {
    int* vecArr = vec -> arr;

	for(unsigned int i = 0; i < vec -> totalElements; i++)
		vecArr[i]++;
}

bool checkArgs(int argc,
			   char* argv[],
			   unsigned int* arrSize) {

	unsigned int arr;

	if (argc != 2) {
		puts("Uso: \n  ./sum_array [arr_size]");
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

bool checkSolution(Vector* solution, Vector* original) {

	int *solArr = solution -> arr;
	int *oriArr = original -> arr;
	
	for(unsigned int i = 0; i < original -> totalElements; i++)
		if (solArr[i] != (oriArr[i] + 1))
			return false;

	return true;
}

int main(int argc, char* argv[]) {

	unsigned int arrSize;
	Vector* vec = NULL, *vecCpy = NULL;
	
	if(!checkArgs(argc, argv, &arrSize))
		exit(-1);

	// Initiate Vector and Clone it For Checking Solution After
	vec = initVec(arrSize);
	populateVec(vec);

	vecCpy = copyVec(vec);

	sum1ToVec(vec);

	// Check Solution
	if(checkSolution(vec, vecCpy))
		puts("Solution Is Correct!");
	else
		puts("Solution Is Incorrect!");


	// Free Vectors
	freeVec(vec);
	freeVec(vecCpy);
	
	return 0;
}
