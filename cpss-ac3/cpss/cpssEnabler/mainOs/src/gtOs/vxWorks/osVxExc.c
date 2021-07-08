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
* @file osVxExc.c
*
* @brief VxWorks Operating System wrapper. Exception handling and Fatal Error facilities.
*
* @version   6
********************************************************************************
*/

#include <gtOs/gtOsMem.h>
#include <gtOs/gtOsExc.h>

#include <gtOs/gtOsIntr.h>
#include <gtOs/gtOsIo.h>
#include <gtOs/gtOsMem.h>
#include <gtOs/gtOsTask.h>

#include <time.h>
#include <intLib.h>
#include <string.h>
#include <taskLib.h>
#include <esf.h>

/************* Includes ******************************************************/
#if (CPU == PPC604) || (CPU == PPC603) || (CPU == PPC860) || (CPU == PPC85XX)
#include <arch/ppc/regsPpc.h>
#endif

#if  ((CPU==ARMARCH7) || (CPU==ARMARCH5) || (CPU==ARMARCH5_T)) && (defined(CPU_946ES) || defined(CPU_926EJ))
#include <arch/arm/regsArm.h>
#endif



/* Binding the default function handling the fatal error */
FATAL_FUNC_PTR currentFatalErrorFunc = NULL;
/* In case the user would like to define his own fatal Error handling function */
FATAL_FUNC_PTR usrDefinedFatalErrorFunc = NULL;


/* External function definitions */
extern GT_STATUS extDrvReset();

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
  int i;
  unsigned int * spLcl = sp;
  /* Set the first address as the program counter */
  funcCallList[0] = (unsigned int)pc;
  for (i=1; (spLcl != 0) && (i < FUNC_CALL_LENGTH) ; i++ )
  {
    funcCallList[i] = *((unsigned int *)((unsigned int)spLcl + 4));
    spLcl = (unsigned int *)(*spLcl);
    if (spLcl == (unsigned int *)0xeeeeeeee)
      break;
  }
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
#ifndef DUNE_FE_IMAGE
  extDrvReset();
  while (1);
#endif
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

  /* Binding the default function handling the fatal error */
  currentFatalErrorFunc = (FATAL_FUNC_PTR)osFatalError;

  /* Binding the user defined function handling the fatal error - if declerd */
  if ( funcPtr != (FATAL_FUNC_PTR)NULL )
    usrDefinedFatalErrorFunc = funcPtr;

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
    printf("%s", messageAttached);

    /* abort application */
    exit(0);
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




