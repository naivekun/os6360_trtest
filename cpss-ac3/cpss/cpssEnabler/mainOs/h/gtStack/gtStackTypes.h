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
* @file gtStackTypes.h
* @version   2
********************************************************************************
*/

#ifndef __gtStackTypes
#define __gtStackTypes

#ifdef __cplusplus
extern "C" {
#endif

/* File descriptor generic type */
#ifndef _WIN32
typedef int GT_FD ;
#else
typedef GT_UINTPTR GT_FD ;
#endif

#ifdef __cplusplus
}
#endif

#endif   /* __gtStackTypes */




