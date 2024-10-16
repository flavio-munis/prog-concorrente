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

// 64-bit multiplication, result in 128 bits
uint128_t mul64(uint64_t a, uint64_t b) {
    uint128_t result;
    result.lo = a * b;
    result.hi = ((__uint128_t)a * b) >> 64;
    return result;
}

// 128-bit by 64-bit division
uint64_t div128_64(uint128_t a, uint64_t b) {
    __uint128_t dividend = ((__uint128_t)a.hi << 64) | a.lo;
    return dividend % b;
}

// Modular exponentiation function
uint64_t powmod(uint64_t a,
                uint64_t b,
                uint64_t k) {
    uint64_t s = 1;
    uint128_t temp;

    while (b > 0) {
        if (b & 1) {
            // s = (s * a) % k
            temp = mul64(s, a);
            s = div128_64(temp, k);
        }
        // a = (a * a) % k
        temp = mul64(a, a);
        a = div128_64(temp, k);
        b >>= 1;
    }

    return s;
}

uint64_t modPow(uint64_t n, uint64_t exp, uint64_t base) {

	__uint128_t result = 1;
    __uint128_t temp = n;

	temp %= base;
	
	while (exp) {

		if (exp & 1)
			result = (result * temp) % base;
	
		temp = (temp * temp) % base;
		exp >>= 1;
	}

	return (uint64_t) result;
}

uint64_t modPowGMP(uint64_t p, uint64_t ak) {
    mpz_t base, exp, mod, result;
    mpz_init(base);
    mpz_init(exp);
    mpz_init(mod);
    mpz_init(result);

    // Set base to 16
    mpz_set_ui(base, 16);

    // Set exponent to p (absolute value)
    mpz_set_si(exp, p < 0 ? -p : p);

    // Set modulus to ak
    mpz_set_si(mod, ak);

    // Compute (16^p) mod ak
    mpz_powm(result, base, exp, mod);

    // Convert result to unsigned uint64_t
    uint64_t ret = mpz_get_ui(result);

    // Clear GMP variables
    mpz_clear(base);
    mpz_clear(exp);
    mpz_clear(mod);
    mpz_clear(result);

    return ret;
}

uint64_t expm(uint64_t p, uint64_t ak) {
  
  int i, j;
  __uint128_t p1, pt, r;
#define ntp 45
  static __uint128_t tp[ntp];
  static int tp1 = 0;
  static int largest = 0;
  
/*  If this is the first call to expm, fill the power of two table tp. */

  if (tp1 == 0) {
    tp1 = 1;
    tp[0] = 1;

    //for (i = 1; i < ntp; i++) tp[i] = 2 * tp[i-1];
	while (tp[largest] < d) {

		tp[largest + 1] = tp[largest] * 2;
		largest++;
	}
  }        

  if (ak == 1) return 0.;

/*  Find the greatest power of two less than or equal to p. */

  for (i = 0; i < largest; i++) if (tp[i] > p) break;

  pt = tp[i-1];
  p1 = p;
  r = 1;

/*  Perform binary exponentiation algorithm modulo ak. */

  for (j = 1; j <= i; j++) {

	  unsigned int temp;
	
	  if (p1 >= pt){
		  r = 16 * r;
		  temp = (r / ak);
		  /* if (temp >= 4200000) */
		  /* 	printf("Temp %lld\n", temp);		   */
		  r = r - temp * ak;
		  p1 = p1 - pt;
	  }
          
	  pt /= 2;

	  if (pt >= 1){
		  r = r * r;
		  temp = (r / ak);
		  /* if (temp >= 4200000) */
		  /* 	printf("Temp %lld\n", temp); */

		  r = r - temp * ak;
	  }
  }

  return r;
}

uint64_t barretReduction(uint64_t n,
                         uint64_t base,
                         uint64_t factor) {

	uint64_t q = ((__uint128_t)n * factor) >> 64;
	q = n - q * base;
        
	if (q >= base)
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
		//temp = modPowBarret(16, d - k, r);
		temp = powmod(16, d - k, r);
		//temp = modPow(16, d - k, r);
	    //temp = expm(d - k, r);
		//temp = modPowGMP(d - k, r);
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
    //static int j = 1;
  
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

	//printf("s1: %Lf\ns2: %Lf\ns3: %Lf\ns4: %Lf\n", s1, s2, s3, s4);

	//result = 4.0L *s1 - 2.0L * s2 - s3 - s4;
	//result = fmodl(result, 1.0L);

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
