/*-----------------------------------------------------------------*/
/**

  @file   timer.h
  @author Flávio M.

 */
/*-----------------------------------------------------------------*/

#ifndef MY_TIMER_HEADER_FILE
#define MY_TIMER_HEADER_FILE

/*-----------------------------------------------------------------
                              Includes
  -----------------------------------------------------------------*/
#include <sys/time.h>
#include <stdlib.h>
#include "error-handler.h"


/*-----------------------------------------------------------------
                              Structs
  -----------------------------------------------------------------*/
typedef struct mytimer {
    struct timespec start;
    struct timespec end;
	double totalTime;
}MyTimer;


/*-----------------------------------------------------------------
                          Macros Definitions
  -----------------------------------------------------------------*/

/*-----------------------------------------------------------------*/
/**
   @brief Allocate Memory for MyTimer Struct
   @param MyTimer* Pointer to MyTimer Struct.
 */
/*-----------------------------------------------------------------*/
#define MALLOC_TIMER(timer)						\
	timer = (MyTimer*) malloc(sizeof(MyTimer)); \
	checkNullPointer((void*) timer);


/*-----------------------------------------------------------------*/
/**
   @brief Malloc and Adds Init To Timer Struct
   @param MyTimer* Pointer to MyTimer Struct.
 */
/*-----------------------------------------------------------------*/
#define INIT_TIMER(timer) {  \
		MALLOC_TIMER(timer); \
	    clock_gettime(CLOCK_MONOTONIC_RAW, &(timer -> start));	\
}


/*-----------------------------------------------------------------*/
/**
   @brief Calculate Total Time Elapsed in Seconds.
   @param MyTimer* Pointer to MyTimer Struct.
 */
/*-----------------------------------------------------------------*/
#define CALC_FINAL_TIME(timer) \
	timer -> totalTime = ((timer -> end).tv_nsec - (timer -> start).tv_nsec) / 1000000000.0 + ((timer -> end).tv_sec - (timer -> start).tv_sec);


/*-----------------------------------------------------------------*/
/**
   @brief Add Stop To Timer Struct.
   @param MyTimer* Pointer to MyTimer Struct.
 */
/*-----------------------------------------------------------------*/
#define END_TIMER(timer) clock_gettime(CLOCK_MONOTONIC_RAW, &(timer -> end));


#endif
