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


/*-----------------------------------------------------------------
                          Global Variables -----------------------------------------------------------------*/
uint64_t d;

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

	if (argc != 2) {
		invalidProgramCall(argv[0], "[inicio]");
	}

    d = strtoll(argv[1], NULL, 10);

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


long double lhsBell(int m, int j, int l) {

	long double r, sum = 0, sign, temp, exp;
    int64_t loopLimit = (int64_t) (4*d + l) / 10;
    
	for (int64_t k = 0; k < loopLimit; k++) {
		sign = (k % 2) ? -1 : 1;
		r = m * k + j;
		temp = modPowBarret(2, 4L*d + l - 10L*k, r);
		sum += sign * (temp / r);	
	    sum = fmodl(sum, 1.0L);
	}
        
    for (int64_t k = loopLimit; k < loopLimit + 100; k++) {
		sign = (k % 2) ? -1 : 1;
		r = m * k + j;
		exp =  (long double) 4*d + l - 10* k;
		temp = powl(2.0, exp);
		temp = (temp / r) *sign;
            
		if (fabsl(temp) < EPSILON)
			break;

		sum += temp;
    }

    sum = fmodl(sum, 1.0L);

	return sum;
}

long double bellardLfS() {

	long double result = 0;
        
	result -= lhsBell(4, 1, -1);
	result -= lhsBell(4, 3, -6);
	result += lhsBell(10, 1, 2);
	result -= lhsBell(10, 3, 0);
	result -= lhsBell(10, 5, -4);
	result -= lhsBell(10, 7, -4);
	result += lhsBell(10, 9, -6);
        
    result = fmodl(result, 1.0L);
        
	return result;
}

long double rhsBell(int m, int j, int l) {

	long double r, sum = 0, temp, sign;
	int64_t k = (int64_t)(4 * d + l) / 10 + 1;
        
    while (true) {
		sign = (k % 2) ? -1 : 1;
		r = m * k + j;
		temp = powl(2, (long double)4 * d + l - 10 * k) / r;
		temp *= sign;

		if (fabsl(temp) < EPSILON)
			break;

		sum += temp;
		sum = fmodl(sum, 1.0L);
		k++;		
	}

	return sum;
}

long double bellardRfs() {

	long double result = 0;

	result -= rhsBell(4, 1, -1);
	result -= rhsBell(4, 3, -6);
	result += rhsBell(10, 1, 2);
	result -= rhsBell(10, 3, 0);
	result -= rhsBell(10, 5, -4);
	result -= rhsBell(10, 7, -4);
	result += rhsBell(10, 9, -6);	
        
	return result;	
} 


long double bbpAlgo() { 

	long double result = 0;

	result += bellardLfS();
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


int main(int argc, char* argv[]) {

	long double result = 0;
        
#ifdef DEBUG
	INIT_TIMER(total);
#endif

	checkArgs(argc, argv);

	result = bbpAlgo();
	printf("%d digits @ %ld = ", PRECISION, d);
	ihex(result);
	puts("");
        
#ifdef DEBUG
    END_TIMER(total);
	CALC_FINAL_TIME(total);

    printf("Total Exec. Time: %.5fs\n", total -> totalTime);
	free(total);
#endif

    return 0;
}
