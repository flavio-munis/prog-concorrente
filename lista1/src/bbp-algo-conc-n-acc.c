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
#define TOTAL_ACC 10000
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
pthread_mutex_t qsMutex, stopMutex, activeMutex, prodMutex, counterMutex;
pthread_cond_t qsCond, stopCond;
bool stop = false;
short int activeThreads;
long long d;
Queue* work;
long long batchSize = 10000;

long double acc[TOTAL_ACC] = {0};
pthread_mutex_t accMutex[TOTAL_ACC];
int counter = 0;

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
   @brief  Init a New Queue Node.
   @param  int   Argument.
   @return Node* Pointer to New Node.
*/
/*-----------------------------------------------------------------*/
Node* initNode(int, long long);

/*-----------------------------------------------------------------*/
/**
   @brief  Free Memory Allocated to Node Struct
   @return Node* Pointer to Node;
*/
/*-----------------------------------------------------------------*/
void freeNode(Node*);


/*-----------------------------------------------------------------*/
/** (MUDAR)
   @brief Add a New Node to Queue
   @param Node*  Node to Be Added.
*/
/*-----------------------------------------------------------------*/
void enqueue(Node*);


/*-----------------------------------------------------------------*/
/**
   @brief  Remove a Node From Queue.
   @return Node* Pointer to Dequeued Node.
*/
/*-----------------------------------------------------------------*/
Node* dequeue();


/*-----------------------------------------------------------------*/
/**
   @brief Print All Info About Queue.
   @param Queue* Pointer to Queue.
*/
/*-----------------------------------------------------------------*/
void printQueue();


/*-----------------------------------------------------------------*/
/**
   @brief Init All Threads, Mutexes and Conditions.
*/
/*-----------------------------------------------------------------*/
void initThreads();


void executeWork(Node*);


void stopThreads();


long double lhs(int, long long);


/*-----------------------------------------------------------------
                      Functions Implementation
  -----------------------------------------------------------------*/
Queue* initQueue() {
	
	Queue* newQ = malloc(sizeof(Queue));
	checkNullPointer((void*) newQ);
	
	newQ -> front = NULL;
	newQ -> rear = NULL;
	newQ -> totalNodes = 0;

	return newQ;
}

Node* initNode(int arg1, long long arg2) {
	
	Node* newNode = malloc(sizeof(Node));
	checkNullPointer((void*) newNode);
    
	newNode -> arg1 = arg1;
	newNode -> arg2 = arg2;
	newNode -> next = NULL;

	return newNode;
}

void freeNode(Node* node) {

	if (node)
		free(node);
}

void enqueue(Node* newNode) {

	pthread_mutex_lock(&qsMutex);
	// If Queue is empty
	if (!work -> rear) {
	    work -> front = newNode;
		work -> rear = newNode;
		
	// If Queue already has elements
	} else {
		work -> rear -> next = newNode;
		work -> rear = newNode;
	}

    work -> totalNodes++;
	pthread_cond_signal(&qsCond);
	pthread_mutex_unlock(&qsMutex);
}

Node* dequeue() {

	Node* temp = NULL;
	
	if (!work || !work -> totalNodes)
		return NULL;

	temp = work -> front;
    work -> front = work -> front -> next;
	work -> totalNodes--;

	if (!work -> front)
		work -> rear = NULL;

	return temp;
}


void printQueue() {

	unsigned int i = 0;
	Node* aux;

	if (!work)
		return;

	aux = work -> front;

	while (aux) {
		printf("=== Node %d ===\n", i++);
	    
		printf("Arg 1 = %d\n", aux -> arg1);
	    printf("Arg 2 = %lld\n", aux -> arg2);

		puts("");
		aux = aux -> next;
	}
}


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


void executeWork(Node* n) {
    
	long double result;
	int accIndex;

	result = lhs(n -> arg1, n -> arg2);

	pthread_mutex_lock(&counterMutex);
	accIndex = counter;
	counter = (counter + 1) % TOTAL_ACC;
	pthread_mutex_unlock(&counterMutex);
	//printf("Acc: %d\n", accIndex);

	pthread_mutex_lock(accMutex + accIndex);
	acc[accIndex] += result;
	pthread_mutex_unlock(accMutex + accIndex);

	freeNode(n);
}

void* thPool(void* arg) {

	// Thread Loop
	while (1) {

		// Node For Work
		Node* workNode;

		// Locks Queue Mutex
		pthread_mutex_lock(&qsMutex);

		// Locks Stop Mutex For Reading
		pthread_mutex_lock(&stopMutex);

		// Checks if stop codition is already set and there's no more work
		if (stop && !work -> totalNodes) {
			pthread_mutex_unlock(&qsMutex);
			pthread_mutex_unlock(&stopMutex);
			break;
		}	

		// Unlocks stop mutex after check
		pthread_mutex_unlock(&stopMutex);

		// Checks if threre's work to be done, if not, waits signal
	    if (!work -> totalNodes) 
			pthread_cond_wait(&qsCond, &qsMutex);
		    
		// Gets Work from queue
		workNode = dequeue();

		// Unlocks queue mutex
		pthread_mutex_unlock(&qsMutex);
	    
		// Do the work
		if (workNode)
			executeWork(workNode);
	}

	// If thread finished execiution and there's no more work to be done
	pthread_mutex_lock(&activeMutex);
	activeThreads--;
	if (!activeThreads)
		pthread_cond_signal(&stopCond);
	pthread_mutex_unlock(&activeMutex);

	// End thread execution
	// Thread is detached so it will clean itself
	return NULL;
}

void initThreads() {

	pthread_t th[activeThreads];
	
	pthread_mutex_init(&qsMutex, NULL);
	pthread_mutex_init(&stopMutex, NULL);
	pthread_mutex_init(&activeMutex, NULL);
	pthread_mutex_init(&counterMutex, NULL);
	pthread_cond_init(&qsCond, NULL);
	pthread_cond_init(&stopCond, NULL);

	for (int i = 0; i < TOTAL_ACC; i++)
		pthread_mutex_init(accMutex + i, NULL);

	for (unsigned short i = 0; i < activeThreads; i++) {
		if (pthread_create(th + i, NULL, &thPool, NULL) != 0) {
			unexpectedError("Error Creating Threads!");
		}

		//printf("Thread %d created!\n", i);

		pthread_detach(th[i]);
	}
}

void stopThreads() {

	// Set stop condition to true
	pthread_mutex_lock(&stopMutex);
	stop = true;
	pthread_mutex_unlock(&stopMutex);

	// Awake all inactive threads (no new enqueue signals will be made)
	pthread_mutex_lock(&qsMutex);
	pthread_cond_broadcast(&qsCond);
	pthread_mutex_unlock(&qsMutex);

	// Awaits Signals that all threads have finished their work
	pthread_mutex_lock(&activeMutex);
	while (activeThreads) {
		pthread_cond_wait(&stopCond, &activeMutex);
	}
	pthread_mutex_unlock(&activeMutex);

	pthread_mutex_destroy(&qsMutex);
	pthread_mutex_destroy(&stopMutex);
	pthread_mutex_destroy(&activeMutex);
	pthread_cond_destroy(&qsCond);
	pthread_cond_destroy(&stopCond);
	pthread_mutex_destroy(&prodMutex);
	pthread_mutex_destroy(&counterMutex);

	for (int i = 0; i < TOTAL_ACC; i++)
		pthread_mutex_destroy(accMutex + i);
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
		temp = modPow(16, d - k, r);
	    //temp = expm((long double) d - k, r);
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

void* produceNodes(void* arg) {

	static long long k = 0;
	long long temp;

	while (true) {
		pthread_mutex_lock(&prodMutex);
		if (k >= d) {
			pthread_mutex_unlock(&prodMutex);
			break;
		}
		temp = k;
		k += batchSize;
		pthread_mutex_unlock(&prodMutex);
		    
		Node* s1, *s2, *s3, *s4;

		s1 = initNode(1, temp);	
		s2 = initNode(4, temp);
	    s3 = initNode(5, temp);
	    s4 = initNode(6, temp);
		enqueue(s1);
		enqueue(s2);
		enqueue(s3);
		enqueue(s4);
	} 

	return NULL;
}

long double bbpAlgo() { 

	long double result = 0;
	pthread_t producers[activeThreads];

	pthread_mutex_init(&prodMutex, NULL);

	// Produce Nodes
    for (int i = 0; i < activeThreads; i++) {
		if (pthread_create(producers + i, NULL, &produceNodes, NULL) != 0) {
			unexpectedError("Error Creating Threads!");
		}
	}

	for (int i = 0; i < activeThreads; i++) {
		if (pthread_join(producers[i], NULL) != 0) {
			unexpectedError("Error Joining Threads!");
		}
	}

	initThreads();


    stopThreads();

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
	work = initQueue();

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
	
	free(work);

    return 0;
}
