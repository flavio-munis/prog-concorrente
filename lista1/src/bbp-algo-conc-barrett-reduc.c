/*-----------------------------------------------------------------*/
/**

  @file   bbp-algo-conc.c
  @author Flávio M.
  @brief  Implements BBP Algo Concurrently.
 */
/*-----------------------------------------------------------------*/

/*-----------------------------------------------------------------
                              Includes
  -----------------------------------------------------------------*/
#include <gmp.h>
#include <math.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <unistd.h>
#include "timer.h"
#include "error-handler.h"


/*-----------------------------------------------------------------
                            Definitions  -----------------------------------------------------------------*/
#define PRECISION 10
#define EPSILON 1e-17
#define TOTAL_ACC 15
#define DEBUG

/*-----------------------------------------------------------------
                              Structs
  -----------------------------------------------------------------*/

typedef struct {
    uint64_t hi;
    uint64_t lo;
} uint128_t;


/*-----------------------------------------------------------------
                          Global Variables -----------------------------------------------------------------*/
pthread_mutex_t counterMutex, accIndexMutex;
uint64_t d;
short int activeThreads;
uint64_t batchSize = 100000;

long double acc[TOTAL_ACC] = {0};
pthread_mutex_t accMutex[TOTAL_ACC];
int accIndex = 0;

MyTimer* total = NULL; 

/*-----------------------------------------------------------------
                   Internal Functions Signatures
  -----------------------------------------------------------------*/

/*-----------------------------------------------------------------*/
/**
   @brief Check if a String of Arguments is Valid.
   @param int             Total Arguments in String (argc).
   @param char*           String of Arguments (argv).
*/
/*-----------------------------------------------------------------*/
void checkArgs(int, char*[]);


/*-----------------------------------------------------------------*/
/**
   @brief  Execute BBP Algo Starting at d up to n Digits.
   @param  uint64_t   Starting Digit.
   @return long double Fractional Part Containing The Result.
*/
/*-----------------------------------------------------------------*/
long double bbpAlgo();


/*-----------------------------------------------------------------*/
/**
   @brief  Modular Exponentiation Algorithm (b^x mod n).
   @param  uint64_t Exponentiation base (b).
   @param  uint64_t Exponent (x).
   @param  uint64_t Modular Base (n).
   @return uint64_t Result of modular exp.
*/
/*-----------------------------------------------------------------*/
uint64_t modPow(uint64_t, uint64_t, uint64_t);


/*-----------------------------------------------------------------*/
/**
   @brief Print Result of BBP Algo (Base 16).
   @param long double Fraction Returned by bbpAlgo().
*/
/*-----------------------------------------------------------------*/
void ihex(long double);


/*-----------------------------------------------------------------*/
/**
   @brief Init All Threads, Mutexes and Conditions.
*/
/*-----------------------------------------------------------------*/
void initThreads();


long double lhs(int, uint64_t);


/*-----------------------------------------------------------------
                      Functions Implementation
  -----------------------------------------------------------------*/
void checkArgs(int argc, 
			   char* argv[]) {

	if (argc != 3) {
		invalidProgramCall(argv[0], "[inicio] [threads]");
	}

    d = strtoll(argv[1], NULL, 10);
    activeThreads = strtoll(argv[2], NULL, 10);

	if (d < 0) {
		invalidArgumentError("Argumento Inválido!\nInicio >= 0");
	}

	if (activeThreads < 1 || activeThreads > 65536) {
		invalidArgumentError("Invalid Numvber of Threads!\n1 < Threads < 65536");
	}
}

uint64_t barretReduction(uint64_t n,
                         uint64_t base,
                         uint64_t factor) {

	uint64_t q = ((__uint128_t)n * factor) >> 64;
	q = n - ((__uint128_t)q * base);
        
	while (q >= base)
		q -= base;

	return q;
}

uint64_t mod_mul(uint64_t a,
                 uint64_t b,
                 uint64_t mod,
                 uint64_t factor) {
	__uint128_t product = (__uint128_t)a * b;
	return barretReduction(product, mod, factor);
}

uint64_t modPowBarret(uint64_t n,
					  uint64_t exp,
					  uint64_t base) {

	uint64_t result = 1;
    uint64_t factor = UINT64_MAX / base;
	
	while (exp) {

		if (exp & 1) {
		    result = mod_mul(result, n, base, factor);
		}

		n = mod_mul(n, n, base, factor);
                
		exp >>= 1;
	}

	return result;
}        


long double lhs(int j, uint64_t s) {

	long double r, sum = 0.0L, mult = -1, temp;
	uint64_t loopLimit = s + batchSize;

	if (j == 1)
		mult = 4;
	else if (j == 4)
		mult = -2;

	if (loopLimit > d)
		loopLimit = batchSize;

	for (uint64_t k = s; k < loopLimit; k++) {
		r = 8.0L * k + j;
		temp = modPowBarret(16, d - k, r);
		sum += (mult * temp) / r;
	    sum = fmodl(sum, 1.0L);
	}

	return sum;
}

long double rhs(int j) {
	
	long double sum = 0.0L, temp, r;
    long double mult = -1.0;

	if (j == 1)
		mult = 4.0L;
	else if (j == 4)
		mult = -2.0L;

	for (uint64_t k = d; k <= d + 100; k++) {
		r = 8.0L*k + j;
		temp = powl(16.0L, (long double) d - k) / r;
		
		if (temp < EPSILON)
			break;

		sum += mult * temp;
	    sum = fmodl(sum, 1.0L);
	}
	
	return sum;
}

void* thPool(void* arg) {

    static uint64_t count = 0;
  
	while (true) {
		uint64_t localCount;
		int localIndex;
      
		pthread_mutex_lock(&counterMutex);
		if (count >= d) {
			pthread_mutex_unlock(&counterMutex);
			break;
		}

		localCount = count;
		count += batchSize;
                
		pthread_mutex_unlock(&counterMutex);

		pthread_mutex_lock(&accIndexMutex);
		localIndex = accIndex;
		accIndex = (accIndex + 1) % TOTAL_ACC;
		pthread_mutex_unlock(&accIndexMutex);
                
		pthread_mutex_lock(accMutex + localIndex);
		acc[localIndex] += lhs(1, localCount);
		acc[localIndex] += lhs(4, localCount);
		acc[localIndex] += lhs(5, localCount);
		acc[localIndex] += lhs(6, localCount);		
		pthread_mutex_unlock(accMutex + localIndex);
	}
	
	return NULL;
}

void initThreads() {

	pthread_t producers[activeThreads];
  
	pthread_mutex_init(&counterMutex, NULL);
	pthread_mutex_init(&accIndexMutex, NULL);
        
	for (int i = 0; i < TOTAL_ACC; i++)
		pthread_mutex_init(accMutex + i, NULL);	
			
        
	// Produce Nodes
    for (int i = 0; i < activeThreads; i++) {
		if (pthread_create(producers + i, NULL, &thPool, NULL) != 0) {
			unexpectedError("Error Creating Threads!");
		}
	}

	for (int i = 0; i < activeThreads; i++) {
		if (pthread_join(producers[i], NULL) != 0) {
			unexpectedError("Error Joining Threads!");
		}
	}

	pthread_mutex_destroy(&counterMutex);
	pthread_mutex_destroy(&accIndexMutex);
        
	for (int i = 0; i < TOTAL_ACC; i++)
		pthread_mutex_destroy(accMutex + i);
}


long double bbpAlgo() { 

	long double result = 0;

	initThreads();
        
	for (int i = 0; i < TOTAL_ACC; i++)
		result += acc[i];
        
	result += rhs(1);
	result = fmodl(result, 1.0L);
	result += rhs(4);
    result = fmodl(result, 1.0L);
	result += rhs(5);
	result = fmodl(result, 1.0L);
    result += rhs(6);
	result = fmodl(result, 1.0L);

	return result;
}


void ihex (long double x) {
	int i;
	long double y;
	char hx[] = "0123456789ABCDEF";
	
	y = x;

	for (i = 0; i < PRECISION; i++){
		y = 16. * (y - floor (y));
		printf("%c", hx[(int) y]);
	}
}

void printTimers() {

	printf("Total Exec. Time: %.5fs\n", total -> totalTime);
	free(total);
}


int main(int argc, char* argv[]) {

	long double result;

#ifdef DEBUG
	INIT_TIMER(total);
#endif

	checkArgs(argc, argv);

	if (d < batchSize)
		batchSize = d;

	result = bbpAlgo(d);
	printf("%d digits @ %ld = ", PRECISION, d);
	ihex(result);
	puts("");
        
#ifdef DEBUG
    END_TIMER(total);
	CALC_FINAL_TIME(total);
    printTimers();
#endif

    return 0;
}
