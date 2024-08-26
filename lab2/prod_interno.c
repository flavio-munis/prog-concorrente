/*-----------------------------------------------------------------*/
/**

  @file    prod_interno.c
  @author  Flávio M.
  @brief   Cálculo o Valor do Produto Interno de Dois Vetores
           Concorrentemente.
  @Materia Prog Concorrente (ICP361)

 */
/*-----------------------------------------------------------------*/

/*-----------------------------------------------------------------
                              Includes
  -----------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>

/*-----------------------------------------------------------------
                              Structs
  -----------------------------------------------------------------*/

typedef struct interval {
    float** start;
    float** end;
	int curr;
	int total;
} Interval;


/*-----------------------------------------------------------------
                  Internal Functions Declarations
  -----------------------------------------------------------------*/

/*-----------------------------------------------------------------*/
/**
   @brief  Init a Interval Struct.
   @param  int       Total Vectors Intervals.
   @return Interval* Pointer to New Interval Struct.
*/
/*-----------------------------------------------------------------*/
Interval* initInterval(int);


/*-----------------------------------------------------------------*/
/**
   @brief Add a Interval To the Struct.
   @param Interval* Pointer to Interval Struct.
   @param float*    Address to Start of The Interval.
   @param float*    Address to End of The Interval.
*/
/*-----------------------------------------------------------------*/
void addInterval(Interval*, float*, float*);


/*-----------------------------------------------------------------*/
/**
   @brief Free All Memory Allocated to Interval Struct
          (Except The Addressess Listed in start and end).
   @param Interval* Pointer to Interval Struct.
*/
/*-----------------------------------------------------------------*/
void freeInterval(Interval*);


/*-----------------------------------------------------------------*/
/**
   @brief Print all Intervals of Vectors in the Struct.
   @param Interval* Pointer to Interval Struct.
*/
/*-----------------------------------------------------------------*/
void printInterval(Interval*);


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
   @brief  Check if a String of Arguments is Valid.
   @param  int           Total Arguments in String (argc).
   @param  char*         String of Arguments (argv).
   @param  unsigned short* Pointer For Data Be Written.
   @return bool If Args  Are Valid.
*/
/*-----------------------------------------------------------------*/
bool checkArgs(int, char*[], unsigned short*);


/*-----------------------------------------------------------------*/
/**
   @brief  Pthread Function To Calc Internal Product In An Interval
   @param  void* Interval Struct.
   @return void* Parcial Result.
*/
/*-----------------------------------------------------------------*/
void* prodInterno(void*);


/*-----------------------------------------------------------------
                      Functions Implementation
  -----------------------------------------------------------------*/
Interval* initInterval(int total) {

	Interval* newInterval = (Interval*) malloc(sizeof(Interval));

	if(!newInterval) {
		perror("Error Allocating Interval");
		exit(-1);
	}

	newInterval -> start = (float**) malloc(sizeof(float*) * total);
	newInterval -> end = (float**) malloc(sizeof(float*) * total);

	if(!newInterval -> start || !newInterval -> end) {
		perror("Error Allocating Interval Start/End");
		exit(-1);
	}

	newInterval -> curr = 0;
	newInterval -> total = 2;
	
	return newInterval;
}


void addInterval(Interval* inter, float* start, float* end) {

	int curr = inter -> curr;
	
	if(curr == inter -> total)
		return ;
	
    inter -> start[curr] = start;
	inter -> end[curr] = end;

	inter -> curr++;
}

void freeInterval(Interval* inter) {

	if(inter){
		if(inter -> start)
			free(inter -> start);

		if(inter -> end)
			free(inter -> end);

		free(inter);
	}
}

void printInterval(Interval* inter) {

	float* start, *end;
	
	for(int i = 0; i < inter -> curr; i++) {

		start = inter -> start[i];
		end = inter -> end[i];

		printf("Interval %d: ", i);
		
		while(start != end){
			printf("%f ", *start);
			start++;
		}

		puts("");
	}

	puts("");					   
}

void printVec(float* vec, unsigned int size) {

	puts("\n=== Info ===");
	printf("Vec Size: %u\n", size);
	printf("Vec Elements: ");
	
	for(unsigned int i = 0; i < size; i++)
		printf("%f ", vec[i]);

	puts("");
}


bool checkArgs(int argc,
			   char* argv[],
			   unsigned short* n_threads) {

	unsigned int threads;

	if (argc != 3) {
		puts("Uso: \n  ./sum_array [n_threds] [output_file]");
	    return false;
	}
    
    threads = atol(argv[1]);
	if(threads < 1 || threads > 32767) {
		puts("Invalid Number of Threads! [1 - 32767]");
	    return false;
	}

	*n_threads = threads;

	return true;
}

void readFromFile(void* dest, size_t size, size_t total, FILE* fd) {

	size_t ret;

	//escreve os elementos do vetor
	ret = fread(dest, size, total, fd);

	if(ret < total) {
	    perror("Error in Reading From File");
		exit(-1);
	}
}

void* prodInterno(void* arg) {

	Interval* inter = (Interval*) arg;
	double* parcialResult;
    float* start1, *end1, *start2;

	parcialResult = (double*) malloc(sizeof(double));

	if(!parcialResult) {
		perror("Error Allocating Memory!");
		exit(-1);
	}

	(*parcialResult) = 0.0;
	start1 = inter -> start[0];
	end1 = inter -> end[0];
	start2 = inter -> start[1];
	//end2 = inter -> end[1];
	
	while(start1 != end1){
		(*parcialResult) += (*start1) * (*start2);
		start1++;
		start2++;
	}

	freeInterval(inter);
		
	return (void*) parcialResult;
}

int main(int argc, char* argv[]) {

	unsigned int size;
	unsigned short n_threads;
    float* vec1, *vec2;
	double int_product, result = 0.0;
	FILE* input;
	
	if(!checkArgs(argc, argv, &n_threads))
		exit(-1);

	// Open File in Reading Mode
    input = fopen(argv[2], "rb");

	if(!input) 
	    perror("Error Opening in File!");

	// Read Vector Size
	readFromFile(&size, sizeof(unsigned int), 1, input);

	// Malloc and Read Vectors
	vec1 = malloc(sizeof(float) * size);
	vec2 = malloc(sizeof(float) * size);

	if(!vec1 || !vec2)
		perror("Error Allocating Memory For Vector!");
	
	readFromFile(vec1, sizeof(float), size, input);
	readFromFile(vec2, sizeof(float), size, input);

	// Read Internal Product From File
	readFromFile(&int_product, sizeof(double), 1, input);

	// Close File Descriptor
	fclose(input);
	
	//printVec(vec1, size);
	//printVec(vec2, size);

	// Check if there's more threads than elements
	if (n_threads > size) {
		n_threads = size;
		printf("More Threads Than Elements in Vector!\n Total Threads Executed %d\n", size);
	}

	pthread_t th[n_threads];
	unsigned int sizePerPart = size / n_threads;
	unsigned int start;
	unsigned int end;
	
	for(unsigned short i = 0; i < n_threads; i++) {

		// Create4 Interval Struct
		Interval* inter = initInterval(2);

		// Calc Current Chunk To Send To Thread
		start = sizePerPart * i;

		if (i == n_threads - 1)
			end = size;
		else
			end = sizePerPart * (i + 1);

		// Add The Interval In Both Vectors
		addInterval(inter, vec1 + start, vec1 + end);
		addInterval(inter, vec2 + start, vec2 + end);

		//printInterval(inter);

		// Create Threads
		if(pthread_create(th + i, NULL, &prodInterno, (void*) inter) != 0)
			perror("Error Creating Threads!");
	}

	// Join All Threads
	for(int i = 0; i < n_threads; i++) {

		double* parcialResult;
		
		if(pthread_join(th[i], (void**) &parcialResult) != 0)
			perror("Error Creating Threads!");

		result += *parcialResult;
		free(parcialResult);
	}

	// Print Results
	printf("Internal Product File: %f\nConcurrent: %f\nVariação Relativa: %f\n", int_product, result, (int_product - result)/ int_product);
	
	// Free Vectors
	free(vec1);
	free(vec2);
	
	return 0;
}
