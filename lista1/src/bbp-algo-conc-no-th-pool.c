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
typedef struct node {
    int arg1;
	long long arg2;
	struct node* next;
} Node;

typedef struct queue {
	Node* front;
	Node* rear;
	unsigned int totalNodes;
} Queue;


/*-----------------------------------------------------------------
                          Global Variables -----------------------------------------------------------------*/
pthread_mutex_t counterMutex, accIndexMutex;
long long d;
short int activeThreads;
long long batchSize = 100000;

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
   @param  long long   Starting Digit.
   @return long double Fractional Part Containing The Result.
*/
/*-----------------------------------------------------------------*/
long double bbpAlgo();


/*-----------------------------------------------------------------*/
/**
   @brief  Modular Exponentiation Algorithm (b^x mod n).
   @param  long long Exponentiation base (b).
   @param  long long Exponent (x).
   @param  long long Modular Base (n).
   @return long long Result of modular exp.
*/
/*-----------------------------------------------------------------*/
long long modPow(long long, long long, long long);


/*-----------------------------------------------------------------*/
/**
   @brief Print Result of BBP Algo (Base 16).
   @param long double Fraction Returned by bbpAlgo().
*/
/*-----------------------------------------------------------------*/
void ihex(long double);


/*-----------------------------------------------------------------*/
/**
   @brief  Init a New Queue.
   @return Queue* Pointer to New Queue.
*/
/*-----------------------------------------------------------------*/
Queue* initQueue();


/*-----------------------------------------------------------------*/
/**
   @brief  Free Memory Allocated to Node Struct
   @return Node* Pointer to Node;
*/
/*-----------------------------------------------------------------*/
void freeNode(Node*);


/*-----------------------------------------------------------------*/
/**
   @brief Init All Threads, Mutexes and Conditions.
*/
/*-----------------------------------------------------------------*/
void initThreads();


long double lhs(int, long long);


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

long long modPow(long long n, long long exp, long long base) {

	__uint128_t result = 1;
    __uint128_t temp = n;

	temp %= base;
	
	while (exp) {

		if (exp & 1)
			result = (result * temp) % base;
	
		temp = (temp * temp) % base;
		exp >>= 1;
	}

	return (long long) result;
}

long long modPowGMP(long long p, long long ak) {
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

    // Convert result to unsigned long long
    long long ret = mpz_get_ui(result);

    // Clear GMP variables
    mpz_clear(base);
    mpz_clear(exp);
    mpz_clear(mod);
    mpz_clear(result);

    return ret;
}

long long expm (long long p, long long ak)

/*  expm = 16^p mod ak.  This routine uses the left-to-right binary 
    exponentiation scheme. */

{
  int i, j;
  __uint128_t p1, pt, r;
#define ntp 37
  static __uint128_t tp[ntp];
  static int tp1 = 0;

/*  If this is the first call to expm, fill the power of two table tp. */

  if (tp1 == 0) {
    tp1 = 1;
    tp[0] = 1;

    for (i = 1; i < ntp; i++) tp[i] = 2 * tp[i-1];
  }

  if (ak == 1) return 0.;

/*  Find the greatest power of two less than or equal to p. */

  for (i = 0; i < ntp; i++) if (tp[i] > p) break;

  pt = tp[i-1];
  p1 = p;
  r = 1;

/*  Perform binary exponentiation algorithm modulo ak. */

  for (j = 1; j <= i; j++){
    if (p1 >= pt){
      r = 16 * r;
      r = r - (long long) (r / ak) * ak;
      p1 = p1 - pt;
    }
    pt /= 2;
    if (pt >= 1){
      r = r * r;
      r = r - (long long) (r / ak) * ak;
    }
  }

  return r;
}

long double lhs(int j, long long s) {

	long double r, sum = 0.0L, mult = -1, temp;
	long long loopLimit = s + batchSize;

	if (j == 1)
		mult = 4;
	else if (j == 4)
		mult = -2;

	if (loopLimit > d)
		loopLimit = batchSize;

	for (long long k = s; k < loopLimit; k++) {
		r = 8.0L*k +j;
		//temp = modPow(16, d - k, r);
	    temp = expm(d - k, r);
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

	for (long long k = d; k <= d + 100; k++) {
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

	static long long count = 0;
  
	while (true) {
		long long localCount;
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
	printf("%d digits @ %lld = ", PRECISION, d);
	ihex(result);
	puts("");

#ifdef DEBUG
    END_TIMER(total);
	CALC_FINAL_TIME(total);
    printTimers();
#endif

    return 0;
}
