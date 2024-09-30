/*-----------------------------------------------------------------*/
/**

  @file   error-handler.h
  @author Flávio M.

 */
/*-----------------------------------------------------------------*/
#ifndef ERROR_HANDLER_HEADER_FILE
#define ERROR_HANDLER_HEADER_FILE


/*-----------------------------------------------------------------
                              Includes
  -----------------------------------------------------------------*/
#include <errno.h>


/*-----------------------------------------------------------------
                          Macros Definitions
  -----------------------------------------------------------------*/

/*-----------------------------------------------------------------*/
/**
   @brief Halts Program and Prints An Custom Error Message And
          System Menssage.
   @param char* Any String That Will Be Printed as Custum Error
 */
/*-----------------------------------------------------------------*/
#define FATAL_SYS_ERROR(err) \
    perror(err);	\
	exit(EXIT_FAILURE);


/*-----------------------------------------------------------------*/
/**
   @brief Halts Program and Prints An Custom Error Message
   @param char* Any String That Will Be Printed as Custum Error
 */
/*-----------------------------------------------------------------*/
#define FATAL_USR_ERROR(err) \
	fprintf(stderr, "\n%s\n", err);	\
	exit(EXIT_FAILURE);


/*-----------------------------------------------------------------*/
/**
   @brief Checks if a Given Pointer is NULL
   @param p Is Any Pointer;

   If Pointer is NULL, halts Program and prints an error message to user.
 */
/*-----------------------------------------------------------------*/
#define checkNullPointer(p) if(!p) { FATAL_SYS_ERROR("Pointer Couldn't Be Allocated!"); } 


/*-----------------------------------------------------------------*/
/**
   @brief Checks if a Given File Pointer is NULL
   @param p Is A File Pointer;

   If Pointer is NULL, halts Program and prints an error message to user.
 */
/*-----------------------------------------------------------------*/
#define checkNullFilePointer(p) if(!p) { FATAL_SYS_ERROR("Invalid File Pointer!"); } 


/*-----------------------------------------------------------------*/
/**
   @brief Throws an Error Message to User and Halts Program
   @param char* Custom Error Message
 */
/*-----------------------------------------------------------------*/
#define invalidArgumentError(err) { FATAL_USR_ERROR(err); }


/*-----------------------------------------------------------------*/
/**
   @brief Throws an Error Message to User
   @param char* Custom Error Message
 */
/*-----------------------------------------------------------------*/
#define invalidArgumentException(exp) fprintf(stderr, "Invalid Argument Exception!\n%s\n", exp); 


#define invalidProgramCall(progName, args) \
	fprintf(stderr, "Usage: \n ./%s %s\n", progName, args); \
    exit(EXIT_FAILURE);


/*-----------------------------------------------------------------*/
/**
   @brief Unexpected Error in Code
   @param char* Custom Error Message
 */
/*-----------------------------------------------------------------*/
#define unexpectedError(err) { FATAL_SYS_ERROR(err); }

#endif
