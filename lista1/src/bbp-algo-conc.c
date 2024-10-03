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


/*-----------------------------------------------------------------
                              Structs
  -----------------------------------------------------------------*/
typedef struct node {
	long double (*func)(int, long long, long double);
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
pthread_mutex_t qsMutex, stopMutex, activeMutex;
pthread_mutex_t s1Mutex, s2Mutex, s3Mutex, s4Mutex;
pthread_cond_t qsCond, stopCond;
bool stop = false;
short int activeThreads;
long long d;
Queue* work;

long double s1 = 0.0L, s2 = 0.0L, s3 = 0.0L, s4 = 0.0L;


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
Node* initNode(long double (*func)(int, long long, long double), 
			   int, 
			   long long);

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

Node* initNode(long double (*func)(int, long long, long double), 
			   int arg1, 
			   long long arg2) {
	
	Node* newNode = malloc(sizeof(Node));
	checkNullPointer((void*) newNode);
    
	newNode -> func = func;
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
    
	long double sum, result;

	switch (n -> arg1) {
		case 1:
			pthread_mutex_lock(&s1Mutex);
			sum = s1;
			result = n -> func(n -> arg1, n -> arg2, sum);
			
			if (result > EPSILON)
				s1 = result;

			pthread_mutex_unlock(&s1Mutex);
			break;
		case 4:
		    pthread_mutex_lock(&s2Mutex);
			sum = s2;
			result = n -> func(n -> arg1, n -> arg2, sum);
			
			if (result > EPSILON)
				s2 = result;

			pthread_mutex_unlock(&s2Mutex);
			break;
	    case 5:
			pthread_mutex_lock(&s3Mutex);
			sum = s3;
			result = n -> func(n -> arg1, n -> arg2, sum);
		    
			if (result > EPSILON)
				s3 = result;

			pthread_mutex_unlock(&s3Mutex);
			break;
	    default:
		    pthread_mutex_lock(&s4Mutex);
			sum = s4;
			result = n -> func(n -> arg1, n -> arg2, sum);
		    
			if (result > EPSILON)
				s4 = result;
			
			pthread_mutex_unlock(&s4Mutex);
			break;
	}

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
	pthread_mutex_init(&s1Mutex, NULL);
	pthread_mutex_init(&s2Mutex, NULL);
	pthread_mutex_init(&s3Mutex, NULL);
	pthread_mutex_init(&s4Mutex, NULL);
	pthread_cond_init(&qsCond, NULL);
	pthread_cond_init(&stopCond, NULL);

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
	pthread_mutex_destroy(&s1Mutex);
	pthread_mutex_destroy(&s2Mutex);
	pthread_mutex_destroy(&s3Mutex);
	pthread_mutex_destroy(&s4Mutex);
	pthread_cond_destroy(&qsCond);
	pthread_cond_destroy(&stopCond);
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

long double lhs(int j, long long k, long double sum) {

	long double r;

	r = 8.0L*k +j;
	sum = sum + modPow(16, d - k, r) / r;
	sum = fmodl(sum, 1.0L);

	//printf("(j=%d)Lhs = %Lf\n", j, sum);

	return sum;
}

long double rhs(int j, long long k, long double sum) {
	
	long double r;

	r = 8.0L*k +j;
	sum = sum + powl(16.0L, (long double) d - k) / r;
	sum = fmodl(sum, 1.0L);

	//printf("(j=%d)Rhs = %Lf\n", j, sum);
	
	return sum;
}

long double bbpAlgo() { 

	long double result;

	initThreads();

	for (long long k = 0; k < d; k++) {
		Node* s1, *s2, *s3, *s4;

		s1 = initNode(lhs, 1, k);
		enqueue(s1);
		
		s2 = initNode(lhs, 4, k);
		enqueue(s2);

		s3 = initNode(lhs, 5, k);
		enqueue(s3);
		
		s4 = initNode(lhs, 6, k);
		enqueue(s4);
	} 

	for (long long k = d; k <= d + 100; k++) {
		Node* s1, *s2, *s3, *s4;

		s1 = initNode(rhs, 1, k);
		enqueue(s1);
		
		s2 = initNode(rhs, 4, k);
		enqueue(s2);

		s3 = initNode(rhs, 5, k);
		enqueue(s3);
		
		s4 = initNode(rhs, 6, k);
		enqueue(s4);
	}

	stopThreads();

	printf("s1: %Lf\ns2: %Lf\ns3: %Lf\ns4: %Lf\n", s1, s2, s3, s4);

	result = 4.0L *s1 - 2.0L * s2 - s3 - s4;
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


int main(int argc, char* argv[]) {

	long double result;

	work = initQueue();
	//results = initQueue();

	checkArgs(argc, argv);

	result = bbpAlgo(d);
	printf("%d digits @ %lld = ", PRECISION, d);
	ihex(result);
	puts("");

	free(work);

    return 0;
}
