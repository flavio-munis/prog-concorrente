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

typedef enum {
	BBP_ORIGINAL,
	BELLARD  
}Algorithm;


/*-----------------------------------------------------------------
                          Global Variables -----------------------------------------------------------------*/
Algorithm algoInUse;
uint64_t upperBound;
long double (*leftSum) (uint64_t);
long double (*rightSum) ();
int64_t upperBoundNeg1, upperBoundNeg6, upperBoundNeg4, upperBound0, upperBound2;


short int activeThreads;
uint64_t d;
uint64_t batchSize = 10000;
pthread_mutex_t counterMutex, accIndexMutex;
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


long double bbpAlgoOriginalLfS(uint64_t);


/*-----------------------------------------------------------------
                      Functions Implementation
  -----------------------------------------------------------------*/
void checkArgs(int argc, 
			   char* argv[]) {

	if (argc != 4) {
		invalidProgramCall(argv[0], "[inicio] [threads] [batchSize]");
	}

    d = strtoll(argv[1], NULL, 10);
    activeThreads = strtoll(argv[2], NULL, 10);
    batchSize = strtoll(argv[3], NULL, 10);
    
	if (d < 0) {
		invalidArgumentError("Argumento Inválido!\nInicio >= 0");
	}

	if (activeThreads < 1 || activeThreads > 65536) {
		invalidArgumentError("Invalid Numvber of Threads!\n1 < Threads < 65536");
	}
	
	if (d < 0) {
		invalidArgumentError("Argumento Inválido!\nInicio >= 0");
	}
}

uint64_t barretReduction(__uint128_t n,
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

	if (loopLimit > upperBound)
		loopLimit = upperBound;

	for (uint64_t k = s; k < loopLimit; k++) {
		r = 8.0L * k + j;
		temp = modPowBarret(16, upperBound - k, r);
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

	for (uint64_t k = upperBound; k <= upperBound + 100; k++) {
		r = 8.0L*k + j;
		temp = powl(16.0L, (long double) upperBound - k) / r;
		
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
		if (count >= upperBound) {
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
		acc[localIndex] += leftSum(localCount);	
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


long double bbpAlgoOriginalLfS(uint64_t s) {

	long double result;

	result = lhs(1, s);
	result += lhs(4, s);
	result += lhs(5, s);
	result += lhs(6, s);

	return result;
}

long double bbpAlgoOriginalRfS() {

    long double result;

    result = rhs(1);
	result = fmodl(result, 1.0L);
	result += rhs(4);
    result = fmodl(result, 1.0L);
	result += rhs(5);
	result = fmodl(result, 1.0L);
    result += rhs(6);
	result = fmodl(result, 1.0L);
        
	return result;
}

uint64_t newtonInv(uint64_t n) {
        
	// Usign unsigned type modulo of 2^64 is not nedded as a overflow would lead to 0
	uint64_t inv = (3 * n) ^ 2;

	//printf("inv: %lu\n", inv);
	    
	for (int i = 0; i < 4; i++){
		inv *= (2 -  n * inv);
		//printf("inv: %lu\n", inv);
	}
	return inv;
}

uint64_t montgomeryMul(uint64_t a, uint64_t b, uint64_t base) {

	uint64_t factor = -newtonInv(base);
	__uint128_t C = (__uint128_t) a * b, temp;
	uint64_t q = ((__uint128_t) (factor * C));
	temp = (__uint128_t) q * base;	
	C = ((__uint128_t) C + temp) >> 64;
        
    if (C >= base)
		C -= base;

	return (uint64_t) C;
}

uint64_t modPowMontgomery(uint64_t exp, uint64_t base) {

	__uint64_t res;
  
	if (exp < 65)
	   return modPowBarret(2, exp, base);
        
	exp -= 64;
	res = ((__uint128_t) 1 << 65) % base;
        
    for (int64_t i = 63 - __builtin_clzll(exp) - 1; i >= 0; i--) {

		res = montgomeryMul(res, res, base);

		res <<= (exp >> i) & 1;

		if (res >= base)
			res -= base;
		
    }

	return res;
}

long double lhsBell(int m, int j, int l, uint64_t s, int64_t upperBoundl) {

	long double r, sum = 0, sign, temp;
	int64_t loopLimit = s + batchSize;

	if (s >= upperBoundl)
		return 0;
        
	if (loopLimit > upperBoundl)
		loopLimit = upperBoundl;

	for (uint64_t k = s; k < loopLimit; k++) {
		sign = (k % 2) ? -1 : 1;
		r = m * k + j;
		//temp = modPowMontgomery(4*d + l - 10*k, r);
		temp = modPowBarret(2, 4*d + l - 10*k, r);
		sum += sign * (temp / r);
	    sum = fmodl(sum, 1.0L);
	}

	return sum;
}

long double bellardLfS(uint64_t s) {

	long double result = 0;

    result -= lhsBell(4, 1, -1, s, upperBoundNeg1);
	result -= lhsBell(4, 3, -6, s, upperBoundNeg6);
	result += lhsBell(10, 1, 2, s, upperBound2);
	result -= lhsBell(10, 3, 0, s, upperBound0);
	result -= lhsBell(10, 5, -4, s, upperBoundNeg4);
	result -= lhsBell(10, 7, -4, s, upperBoundNeg4);
	result += lhsBell(10, 9, -6, s, upperBoundNeg6);

	fmodl(result, 1.0);
        
	return result;
}

long double rhsBell(int m, int j, int l, int64_t upperBoundl) {

	long double r, sum = 0, sign, temp, exp;

	for (uint64_t k = upperBoundl; k <= upperBoundl + 100; k++) {
		sign = (k % 2) ? -1 : 1;
		r = m * k + j;
		exp = (long double) 4*d + l - 10* k;
		temp = powl(2.0, exp);
		temp = (temp / r) * sign;

		if (fabsl(temp) < EPSILON)
			break;

		sum += temp;
	}

	sum = fmodl(sum, 1.0L);		
        
	return sum;
}

long double bellardRfs() {

	long double result = 0;
	
    result -= rhsBell(4, 1, -1, upperBoundNeg1);
	result -= rhsBell(4, 3, -6, upperBoundNeg6);
	result += rhsBell(10, 1, 2, upperBound2);
	result -= rhsBell(10, 3, 0, upperBound0);
	result -= rhsBell(10, 5, -4, upperBoundNeg4);
	result -= rhsBell(10, 7, -4, upperBoundNeg4);
	result += rhsBell(10, 9, -6, upperBoundNeg6);

	fmodl(result, 1.0L);
        
	return result;	
}


long double bbpAlgo() { 

	long double result = 0;

	initThreads();
        
	for (int i = 0; i < TOTAL_ACC; i++)
		result += acc[i];

	result += rightSum();
	fmodl(result, 1.0L);	
        
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


void configAlgorithm(Algorithm userAlgo) {

	int64_t helper = 4 * d;
  
	switch (userAlgo) {

        case BBP_ORIGINAL:
			algoInUse = BBP_ORIGINAL;
			leftSum = bbpAlgoOriginalLfS;
			rightSum = bbpAlgoOriginalRfS;
			upperBound = d;
			break;

        case BELLARD:
			algoInUse = BELLARD;
			leftSum = bellardLfS;
			rightSum = bellardRfs;

			helper = 4 * d;

			upperBound0 = (int64_t) helper / 10;
			upperBound2 = (int64_t) (helper + 2) / 10;
			upperBoundNeg1 = (int64_t )(helper - 1) / 10;
            upperBoundNeg4 = (int64_t) (helper - 4) / 10;
            upperBoundNeg6 = (int64_t) (helper - 6) / 10;
			upperBound = upperBound2;
            break;
	}
  
	if (upperBound < batchSize)
		batchSize = upperBound;
}


int main(int argc, char* argv[]) {

	long double result;
	Algorithm userAlgo = BELLARD;
        
#ifdef DEBUG
	INIT_TIMER(total);
#endif

	checkArgs(argc, argv);

	configAlgorithm(userAlgo);

	result = bbpAlgo();
	//printf("%d digits @ %ld = ", PRECISION, d);
	//ihex(result);
	//puts("");
        
#ifdef DEBUG
    END_TIMER(total);
	CALC_FINAL_TIME(total);

    printf("Total Exec. Time: %.5fs\n", total -> totalTime);
	free(total);
#endif

    return 0;
}
