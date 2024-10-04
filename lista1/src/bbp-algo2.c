#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include "timer.h"
#include "error-handler.h"


#define DEBUG
#define PRECISION 10
#define EPSILON 1e-17


void checkArgs(int, char*[], long long*);
long double bbpAlgo(long long);


void checkArgs(int argc, 
			   char* argv[],
			   long long* d) {

	long long digit;

	if (argc != 2) {
		invalidProgramCall(argv[0], "[inicio]");
	}

    digit = strtoll(argv[1], NULL, 10);

	if (digit < 0) {
		invalidArgumentError("Argumento Inválido!\nInicio >= 0");
	}

	*d = digit;
}

long long modPow(long long n, long long exp, long long base) {

	unsigned long long result = 1;
	unsigned long long temp = n;

	temp %= base;
	
	while (exp) {

		if (exp % 2)
			result = (result * temp) % base;
	
		temp = (temp * temp) % base;
		exp >>= 1;
	}

	return result;
}

long double series(int j, long long n) {

	long double sum = 0, temp, r;
#ifdef DEBUG
	MyTimer* left = NULL, *right = NULL;

	INIT_TIMER(left);
#endif

	for (long long k = 0; k < n; k++) {
		r = 8.0L*k +j;
		temp = modPow(16, n - k, r);
		sum = sum + temp / r;
		sum = fmodl(sum, 1.0L);
	}

#ifdef DEBUG
    END_TIMER(left);

	INIT_TIMER(right);
#endif

	for (long long k = n; k <= n + 100; k++) {
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

long double series1(long long n) {

	long double temp, r, sum = 0.0L;

	for (long long k = 0; k < n; k++) {

		r = 8.0L*k + 1;
		temp = modPow(16, n - k, r);
		sum = sum + (4.0L * temp) / r;
		sum = fmodl(sum, 1.0L);
	}

	for (long long k = n; k <= n + 100; k++) {
		r = 8.0L*k + 1;
		temp = powl(16.0L, (long double) n - k) / r;
		
		if (temp < EPSILON)
			break;

	    sum += 4.0L * temp;
		sum = fmodl(sum, 1.0L);
	}

	return sum;
}

long double series2(int j, long long n) {

	long double temp, r, sum = 0.0L;
    long double multiplier = -1;

	if (j == 1)
		multiplier = 4;
	else if (j == 4)
		multiplier = -2;

	for (long long k = 0; k < n; k++) {
		r = 8.0L*k + j;
		temp = modPow(16, n - k, r);
		sum += (multiplier * temp) / r;
		sum = fmodl(sum, 1.0L);
	}

	for (long long k = n; k <= n + 100; k++) {
		r = 8.0L*k + j;
		temp = powl(16.0L, (long double) n - k) / r;
		
		if (temp < EPSILON)
			break;

	    sum += multiplier * temp;
		sum = fmodl(sum, 1.0L);
	}

	return sum;
}

long double bbpAlgo(long long d) {
	
	long double result = 0.0L;
#ifdef DEBUG
	MyTimer* ts1 = NULL, *ts2 = NULL, *ts3 = NULL, *ts4 = NULL, *tresult = NULL;

	puts("Timing Series 1...");
	INIT_TIMER(ts1);
#endif

	result += series2(4, d);
	result = fmodl(result, 1.0L);

	//s1 = series(1, d);
    result += series2(1, d);
	result = fmodl(result, 1.0L);
	printf("(1) Parcial Result %Lf\n", result);

#ifdef DEBUG
	END_TIMER(ts1);
	CALC_FINAL_TIME(ts1);
	printf("Total Time: %.5fs\n\n", ts1 -> totalTime);
	free(ts1);

	puts("Timing Series 2...");
	INIT_TIMER(ts2);
#endif

	//s2 = series(4, d);
	

#ifdef DEBUG
	END_TIMER(ts2);
	CALC_FINAL_TIME(ts2);
	printf("Total Time: %.5fs\n\n", ts2 -> totalTime);
	free(ts2);

	puts("Timing Series 3...");
	INIT_TIMER(ts3);
#endif

	//s3 = series(5, d);
	result += series2(5, d);
	result = fmodl(result, 1.0L);

#ifdef DEBUG
	END_TIMER(ts3);
	CALC_FINAL_TIME(ts3);
	printf("Total Time: %.5fs\n\n", ts3 -> totalTime);
	free(ts3);

	puts("Timing Series 4...");
	INIT_TIMER(ts4);
#endif

	//s4 = series(6, d);
	result += series2(6, d);
	result = fmodl(result, 1.0L);

#ifdef DEBUG
	END_TIMER(ts4);
	CALC_FINAL_TIME(ts4);
	printf("Total Time: %.5fs\n\n", ts4 -> totalTime);
	free(ts4);

	puts("Calc Final Result...");
	INIT_TIMER(tresult);
#endif	

	//result = 4.0L *s1 - 2.0L * s2 - s4 - s3;
	//result = s1 - 2.0L * s2 - s3 - s4;
	//result = fmodl(result, 1.0L);

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

	long long d;
	long double result;
#ifdef DEBUG
	MyTimer* tbbp = NULL;
#endif

	checkArgs(argc, argv, &d);

#ifdef DEBUG
	puts("Timing Total Algo Runtime...\n");
	INIT_TIMER(tbbp);
#endif

	result = bbpAlgo(d);

#ifdef DEBUG
	END_TIMER(tbbp);
	CALC_FINAL_TIME(tbbp);
	printf("Total Runtime: %.5fs\n\n", tbbp -> totalTime);
	free(tbbp);
#endif

	printf("%d digits @ %lld = ", PRECISION, d);
	ihex(result);
	puts("");

	return 0;
}
