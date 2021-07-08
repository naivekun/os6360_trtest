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
* @file gtOsExc.h
*
* @brief Operating System wrapper. Exception Handling facility.
*
* @version   14
********************************************************************************
*/

#ifndef __gtOsExch
#define __gtOsExch

#ifdef __cplusplus
extern "C" {
#endif

/************* Includes ******************************************************/

#include <stdio.h>

#ifdef _VXWORKS

#include <vxWorks.h>
#include <excLib.h>
#include <taskLib.h>
#include <esf.h>

#if (CPU == PPC604) || (CPU == PPC603) || (CPU == PPC860) || (CPU == PPC85XX)
#define CPU_PPC_EXIST
#include <arch/ppc/esfPpc.h>
#endif
#if  ((CPU==ARMARCH7) || (CPU==ARMARCH5) || (CPU==ARMARCH5_T)) && (defined(CPU_946ES) || defined(CPU_926EJ))
#define CPU_ARM_EXIST
#include <arch/arm/esfArm.h>
#endif

#endif /* _VXWORKS */

#include <gtOs/gtGenTypes.h>

/************* Defines *******************************************************/
#define  FUNC_CALL_LENGTH   64
#define  MAX_UINT_8     255

#if defined(_WIN32) || defined(_linux) || defined(_uClinux) || defined(_FreeBSD)
#define  ESFPPC char
#define  ESF    char
#endif


/************* Enumoretion Types **********************************************/
typedef enum {
    OS_FATAL_RESET = 0,
    OS_FATAL_MONITOR,  /* TBD */
    OS_FATAL_WARNING
} OS_FATAL_ERROR_TYPE;

/************* Data structures ************************************************/
typedef struct  {
      unsigned int exc_vector; /* exception vector */
      char exc_name[30];  /* exception name */
} OS_exception_list_struct;

/* ptr to fatal error function  */
typedef void (*FATAL_FUNC_PTR)(OS_FATAL_ERROR_TYPE, char *);


/**
* @internal osFatalErrorInit function
* @endinternal
*
* @brief   Initalize the Fatal Error mechanism.
*
* @retval 0                        - success, 1 - fail.
*/
GT_STATUS osFatalErrorInit( FATAL_FUNC_PTR funcPtr );

/**
* @internal osFatalError function
* @endinternal
*
* @brief   Handling fatal error message.
*/
void osFatalError(
  OS_FATAL_ERROR_TYPE fatalErrorType,
  char * messageAttached
);

/**
* @internal osExceptionInit function
* @endinternal
*
* @brief   Replacing the OS exception handling.
*/
void osExceptionInit(void);


#ifdef CPU_PPC_EXIST
#undef CPU_PPC_EXIST
/*******************************************************************************
* osExceptionHandling  ( For PPC architecture )
*
* DESCRIPTION:
*       Handling the OS exceptions.
* INPUTS:
*       task - ID of offending task.
*       vecNum - Vector number.
*       stackPtr - Pointer to the main control architecture registers.
* OUTPUTS:
*       None
* RETURNS:
*       0 - success, 1 - fail.
* COMMENTS:
*       None
*
*******************************************************************************/
void osExceptionHandling(
  int      task,
  int      vecNum,
  ESFPPC  *stackPtr
);
#endif

#ifdef CPU_ARM_EXIST
#undef CPU_ARM_EXIST
/**
* @internal osExceptionHandling function
* @endinternal
*
* @brief   Handling the OS exceptions.
*
* @param[in] task                     - ID of offending task.
* @param[in] vecNum                   - Vector number.
* @param[in] stackPtr                 - Pointer to the main control architecture registers.
*
* @retval 0                        - success, 1 - fail.
*/
void osExceptionHandling(
  int      task,
  int      vecNum,
  ESF      *stackPtr
);
#endif


/**
* @internal osReset function
* @endinternal
*
* @brief   Reset the CPU
*
* @note Calling to specific function from mainExtDrv
*
*/
void  osReset(int type);

#ifdef __cplusplus
}
#endif

#endif  /* __gtOsExch */
/* Do Not Add Anything Below This Line */



