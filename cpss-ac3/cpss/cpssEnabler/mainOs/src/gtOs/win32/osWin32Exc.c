/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/
/**
********************************************************************************
* @file osWin32Exc.c
*
* @brief Windows Operating System wrapper. Exception handling and Fatal Error facilities.
*
* @version   3
********************************************************************************
*/

#include <gtOs/gtOsMem.h>
#include <gtOs/gtOsExc.h>

#include <string.h>


/************* Includes ******************************************************/

#define MAX_STR_LEN 300

/************* Defines ***********************************************/

/* External function definitions */
extern void SHOSTG_abort(void);


/**
* @internal osLoadFuctionCalls function
* @endinternal
*
* @brief   Loading the last function calls to a given buffer
*/
void  osLoadFuctionCalls(
  unsigned int * pc, 
  unsigned int * sp, 
  unsigned int * funcCallList
)
{
    return;
}

/************ Public Functions ************************************************/

/**
* @internal osReset function
* @endinternal
*
* @brief   Reset the CPU
*
* @note Calling to specific function from mainExtDrv
*
*/
void  osReset(int type)
{
    return;
}

/**
* @internal osFatalErrorInit function
* @endinternal
*
* @brief   Initalize the Fatal Error mechanism.
*
* @retval 0                        - success, 1 - fail.
*/
GT_STATUS osFatalErrorInit( FATAL_FUNC_PTR funcPtr )
{
    return(GT_OK);
}

/**
* @internal osFatalError function
* @endinternal
*
* @brief   Handling fatal error message.
*/
void osFatalError(
  OS_FATAL_ERROR_TYPE fatalErrorType,
  char * messageAttached
)
{
    /* screan print */
    printf("------------ Fatal Error ------------\n");
    printf(messageAttached);

    /* abort application */
    SHOSTG_abort();

}

/**
* @internal osExceptionInit function
* @endinternal
*
* @brief   Replacing the OS exception handling.
*/
void osExceptionInit(void)
{
    return;
}

        


