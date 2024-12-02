#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "timer.h"
#include "error-handler.h"


//#define DEBUG
#define PRECISION 10
#define EPSILON 1e-17


void checkArgs(int, char*[], uint64_t*);
long double bbpAlgo(uint64_t);


void checkArgs(int argc, 
			   char* argv[],
			   uint64_t* d) {

	uint64_t digit;

	if (argc != 2) {
		invalidProgramCall(argv[0], "[inicio]");
	}

    digit = strtoll(argv[1], NULL, 10);

	if (digit < 0) {
		invalidArgumentError("Argumento Inválido!\nInicio >= 0");
	}

	*d = digit;
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

uint64_t modMul(uint64_t a,
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
		    result = modMul(result, n, base, factor);
		}

		n = modMul(n, n, base, factor);
                
		exp >>= 1;
	}

	return result;
}

long double series(int j, uint64_t n) {

	long double sum = 0, temp, r;
#ifdef DEBUG
	MyTimer* left = NULL, *right = NULL;

	INIT_TIMER(left);
#endif

	for (uint64_t k = 0; k < n; k++) {
		r = 8.0L*k +j;
		temp = modPowBarret(16, n - k, r);
		sum = sum + temp / r;
		sum = fmodl(sum, 1.0L);
	}

#ifdef DEBUG
    END_TIMER(left);

	INIT_TIMER(right);
#endif

	for (uint64_t k = n; k <= n + 100; k++) {
		r = 8.0L*k +j;
		temp = powl(16.0L, (long double) n - k) / r;
		
		if (temp < EPSILON)
			break;

	    sum += temp;
		sum = fmodl(sum, 1.0L);
	}

#ifdef DEBUG
    END_TIMER(right);
	CALC_FINAL_TIME(left);
	CALC_FINAL_TIME(right);
	printf("Left: %.5fs\nRight: %.5fs\n", left -> totalTime, right -> totalTime);
	free(left);
	free(right);
#endif

	return sum;
}

long double bbpAlgo(uint64_t d) {
	
	long double s1, s2, s3, s4, result;
#ifdef DEBUG
	MyTimer* ts1 = NULL, *ts2 = NULL, *ts3 = NULL, *ts4 = NULL, *tresult = NULL;

	puts("Timing Series 1...");
	INIT_TIMER(ts1);
#endif

	s1 = series(1, d);

#ifdef DEBUG
	END_TIMER(ts1);
	CALC_FINAL_TIME(ts1);
	printf("Total Time: %.5fs\n\n", ts1 -> totalTime);
	free(ts1);

	puts("Timing Series 2...");
	INIT_TIMER(ts2);
#endif

	s2 = series(4, d);
	
#ifdef DEBUG
	END_TIMER(ts2);
	CALC_FINAL_TIME(ts2);
	printf("Total Time: %.5fs\n\n", ts2 -> totalTime);
	free(ts2);

	puts("Timing Series 3...");
	INIT_TIMER(ts3);
#endif

	s3 = series(5, d);

#ifdef DEBUG
	END_TIMER(ts3);
	CALC_FINAL_TIME(ts3);
	printf("Total Time: %.5fs\n\n", ts3 -> totalTime);
	free(ts3);

	puts("Timing Series 4...");
	INIT_TIMER(ts4);
#endif

	s4 = series(6, d);

#ifdef DEBUG
	END_TIMER(ts4);
	CALC_FINAL_TIME(ts4);
	printf("Total Time: %.5fs\n\n", ts4 -> totalTime);
	free(ts4);

	puts("Calc Final Result...");
	INIT_TIMER(tresult);
#endif	

	result = 4.0L *s1 - 2.0L * s2 - s4 - s3;
	result = fmodl(result, 1.0L);

#ifdef DEBUG
	END_TIMER(tresult);
	CALC_FINAL_TIME(tresult);
	printf("Total Time: %.5fs\n\n", tresult -> totalTime);
	free(tresult);
#endif

	return result;
}

void ihex (double x) {
  int i;
  double y;
  char hx[] = "0123456789ABCDEF";

  y = x;

  for (i = 0; i < PRECISION; i++){
    y = 16. * (y - floor (y));
    printf("%c", hx[(int) y]);
  }
}

int main(int argc, char* argv[]) {

	uint64_t d;
	long double result;
	MyTimer* tbbp = NULL;

	checkArgs(argc, argv, &d);

	INIT_TIMER(tbbp);

	result = bbpAlgo(d);

	END_TIMER(tbbp);
	CALC_FINAL_TIME(tbbp);
	printf("Total Runtime: %.5fs\n", tbbp -> totalTime);
	free(tbbp);

	printf("%d digits @ %ld = ", PRECISION, d);
	ihex(result);
	puts("");

	return 0;
}
