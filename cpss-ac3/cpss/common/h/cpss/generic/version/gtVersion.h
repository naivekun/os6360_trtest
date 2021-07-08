/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
*/
/**
********************************************************************************
* @file gtVersion.h
*
* @brief Includes definitions for math function.
*
* @version   1
********************************************************************************
*/
#ifndef __gtVersionh
#define __gtVersionh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/extServices/os/gtOs/gtGenTypes.h>


#define VERSION_MAX_LEN 30
/**
* @struct GT_VERSION
 *
 * @brief This struct holds the package version.
*/
typedef struct{

    GT_U8 version[VERSION_MAX_LEN];

} GT_VERSION;

/* use definitions from 'common' */
#define CPSS_VERSION_MAX_LEN_CNS    VERSION_MAX_LEN
typedef GT_VERSION                  CPSS_VERSION_INFO_STC;



#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __gtVersionh */


