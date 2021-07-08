#include <Copyright.h>
/********************************************************************************
* debug.c
*
* DESCRIPTION:
*       Debug message display routine
*
* DEPENDENCIES:
*       OS Dependent
*
* FILE REVISION NUMBER:
*******************************************************************************/
#include <msApiTypes.h>
#include <gtUtils.h>

#ifdef PRV_CPSS_DEBUG_QD
#if defined(LINUX)
 #ifdef __KERNEL__
  #include "kernel.h"
 #else
  #include "stdio.h"
  #include "stdarg.h"
 #endif
#endif

/*******************************************************************************
* prvCpssDrvGtDbgPrint
*
* DESCRIPTION:
*       .
*
* INPUTS:
*       None
*
* OUTPUTS:
*       None
*
* RETURNS:
*
* COMMENTS:
*       None
*
*******************************************************************************/
#if defined(LINUX)
void prvCpssDrvGtDbgPrint(char* format, ...)
{
    va_list argP;
    char dbgStr[1000] = "";

    va_start(argP, format);

    vsprintf(dbgStr, format, argP);
#if defined(LINUX)
 #ifdef __KERNEL__
    printk("%s",dbgStr);
 #else
    printf("%s",dbgStr);
 #endif
#endif
    return;
}
#else
void prvCpssDrvGtDbgPrint(char* format, ...)
{
    GT_UNUSED_PARAM(format);
}
#endif
#else /* PRV_CPSS_DEBUG_QD not defined */
void prvCpssDrvGtDbgPrint(char* format, ...)
{
    GT_UNUSED_PARAM(format);
}
#endif /* PRV_CPSS_DEBUG_QD */

