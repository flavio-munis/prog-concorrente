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
	void* (*func)(void*);
	void* arg;
	void* retValue;
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
pthread_cond_t qsCond, stopCond;
bool stop = false;
short int activeThreads;
Queue* work;


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
void checkArgs(int, char*[], long long*);


/*-----------------------------------------------------------------*/
/**
   @brief  Execute BBP Algo Starting at d up to n Digits.
   @param  long long   Starting Digit.
   @return long double Fractional Part Containing The Result.
*/
/*-----------------------------------------------------------------*/
long double bbpAlgo(long long);


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
   @param  void* (*f)(void*) Pointer to Node Function.
   @param  void*             Argument to Function.
   @return Node*             Pointer to New Node.
*/
/*-----------------------------------------------------------------*/
Node* initNode(void* (*func)(void*), void*);

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
   @param Queue*           Pointer to Queue.
   @param void* (*f)(void*) Pointer to Node Function.
   @param void*             Argument to Function.
*/
/*-----------------------------------------------------------------*/
void enqueue(Queue*, Node*);


/*-----------------------------------------------------------------*/
/**
   @brief  Remove a Node From Queue.
   @return Node* Pointer to Dequeued Node.
*/
/*-----------------------------------------------------------------*/
Node* dequeue(Queue*);


/*-----------------------------------------------------------------*/
/**
   @brief Print All Info About Queue.
   @param Queue* Pointer to Queue.
*/
/*-----------------------------------------------------------------*/
void printQueue(Queue*);


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

Node* initNode(void* (*func)(void*), void* arg) {
	
	Node* newNode = malloc(sizeof(Node));
	checkNullPointer((void*) newNode);
	
	newNode -> func = func;
	newNode -> arg = arg;
	newNode -> retValue = NULL;
	newNode -> next = NULL;

	return newNode;
}

void freeNode(Node* node) {

	if (node) {		
		if (node -> arg)
			free(node -> arg);

		if (node -> retValue)
			free(node -> retValue);

		free(node);
	}
}

void enqueue(Queue* q, Node* newNode) {
	
	// If queue was not initialized yet
	if (!q)
		return;

	pthread_mutex_lock(&qsMutex);
	// If Queue is empty
	if (!q -> rear) {
	    q -> front = newNode;
		q -> rear = newNode;
		
	// If Queue already has elements
	} else {
		q -> rear -> next = newNode;
		q -> rear = newNode;
	}

    q -> totalNodes++;
	pthread_cond_signal(&qsCond);
	pthread_mutex_unlock(&qsMutex);
}

Node* dequeue(Queue* q) {

	Node* temp = NULL;
	
	if (!q || !q -> totalNodes)
		return NULL;

	temp = q -> front;
	q -> front = q -> front -> next;
	q -> totalNodes--;

	if (!q -> front)
		q -> rear = NULL;

	return temp;
}


void printQueue(Queue* q) {

	unsigned int i = 0;
	Node* aux;

	if (!q)
		return;

	aux = q -> front;

	while (aux) {
		printf("=== Node %d ===\n", i++);
		
		if (aux -> arg)
			printf("Arg = %d\n", *((int*)aux -> arg));
		
		if (aux -> retValue)
			printf("RetValue = %Lf\n", *((long double*)aux -> retValue));

		puts("");
		aux = aux -> next;
	}
}


void checkArgs(int argc, 
			   char* argv[],
			   long long* d) {

	long long digit;

	if (argc != 3) {
		invalidProgramCall(argv[0], "[inicio] [threads]");
	}

    digit = strtoll(argv[1], NULL, 10);
    activeThreads = strtoll(argv[2], NULL, 10);

	if (digit < 0) {
		invalidArgumentError("Argumento Inválido!\nInicio >= 0");
	}

	if (activeThreads < 1 || activeThreads > 65536) {
		invalidArgumentError("Invalid Numvber of Threads!\n1 < Threads < 65536");
	}

	*d = digit;
}


void* sqrt2(void* arg) {
	int num = *((int*) arg);
	int* result = (int*) malloc(sizeof(int));

	*result = sqrt(num);
	return (void*) result;
}

void retFunction(Node* n) {
	int arg, retValue;

	arg = *((int*) n -> arg);
    retValue = *((int*) n -> retValue);

	printf("%d * 2 = %d\n", arg, retValue);
}

void executeWork(Node* n) {
	void* result; 
    
	result = n -> func(n -> arg);
    printf("Result: %d\n", *((int*) result));
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
		workNode = dequeue(work);

		// Unlocks queue mutex
		pthread_mutex_unlock(&qsMutex);
	    
		// Do the work
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
	pthread_cond_init(&qsCond, NULL);
	pthread_cond_init(&stopCond, NULL);
	
	//pthread_mutex_init(&totalOp, NULL);

	for (unsigned short i = 0; i < activeThreads; i++) {
		if (pthread_create(th + i, NULL, &thPool, NULL) != 0) {
			unexpectedError("Error Creating Threads!");
		}

		printf("Thread %d created!\n", i);

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
}

int main(int argc, char* argv[]) {

	long long d;
	long double result;

	work = initQueue();
	//results = initQueue();

	checkArgs(argc, argv, &d);
	initThreads();

	srand(time(NULL));
	for (int i = 0; i < 10; i++) {
		Node* newNode;
		int* n = (int*) malloc(sizeof(int));
		checkNullPointer((void*) n);

		*n = rand() % 100;
		newNode = initNode(&sqrt2, (void*) n);
		enqueue(work, newNode);
		//printQueue(work);
	}

	stopThreads();

	/* result = bbpAlgo(d); */
	/* printf("%d digits @ %lld = ", PRECISION, d); */
	/* ihex(result); */
	/* puts(""); */

	/* pthread_mutex_destroy(&workQueue); */
	/* pthread_mutex_destroy(&resQueue); */
	/* pthread_cond_destroy(&condWork); */
	/* pthread_cond_destroy(&condRes); */
	/* free(work); */
	/* free(res); */

	pthread_exit(0);
}
