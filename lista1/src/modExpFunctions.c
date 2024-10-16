#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "timer.h"

#define PRECISION 1000000000000000LL // 10^15 for 15 decimal digits of precision

// Structure to represent 128-bit integer
typedef struct {
    uint64_t hi;
    uint64_t lo;
} uint128_t;

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

uint64_t modPow(uint64_t n,
				uint64_t exp,
				uint64_t base) {

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

int main() {

  MyTimer* normal = NULL, *multi_bits = NULL, *barrett = NULL;
  uint64_t n = 3;
  uint64_t exp = 644;
  uint64_t base = 1000000007;

  INIT_TIMER(normal);
  printf("(%ld)^%ld mod %ld: %ld (modPow)\n", n, exp, base, modPow(n, exp, base));
  END_TIMER(normal);

  INIT_TIMER(barrett);
  printf("(%ld)^%ld mod %ld: %ld (modPow)\n", n, exp, base, modPowBarret(n, exp, base));
  END_TIMER(barrett);
  
  INIT_TIMER(multi_bits);
  printf("(%ld)^%ld mod %ld: %ld (multi-bits)\n", n, exp, base,
         powmod(n, exp, base));
  END_TIMER(multi_bits);

  CALC_FINAL_TIME(normal);
  CALC_FINAL_TIME(multi_bits);
  CALC_FINAL_TIME(barrett);
  
  printf("Total Exec Time (ModPow): %.5fs\n", normal -> totalTime);
  printf("Total Exec Time (Multi-Bits): %.5fs\n", multi_bits -> totalTime);
  printf("Total Exec Time (ModPowBarrett): %.5fs\n", barrett->totalTime);

  free(barrett);
  free(normal);
  free(multi_bits);
  
  return 0;
}
