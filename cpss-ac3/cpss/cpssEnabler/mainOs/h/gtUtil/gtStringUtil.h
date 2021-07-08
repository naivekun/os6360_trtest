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
* @file gtStringUtil.h
*
* @brief Tool for tracing memory usage.
* Based on osMemGetHeapBytesAllocated function;
*
* @version   1
********************************************************************************
*/

#ifndef __gtStringUtilh
#define __gtStringUtilh

#ifdef __cplusplus
extern "C" {
#endif

/************* Includes *******************************************************/

#include <gtOs/gtGenTypes.h>
#include <gtOs/gtOsMem.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>

/**
* @internal gtStringUtilDecimalFromLine function
* @endinternal
*
* @brief   Extract value by key from parameter line.
*
* @param[in] line                     - string that contains keys and values
* @param[in] key                      - string used as name of value
*                                       value if found or 0.
*/
GT_U32 gtStringUtilDecimalFromLine
(
    IN GT_CHAR* line,
    IN GT_CHAR* key
);

#ifdef __cplusplus
}
#endif

#endif  /* __gtStringUtilh */
/* Do Not Add Anything Below This Line */



