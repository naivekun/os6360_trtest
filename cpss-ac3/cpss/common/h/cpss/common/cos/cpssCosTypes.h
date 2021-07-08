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
* @file common/h/cpss/common/cos/cpssCosTypes.h
*
* @brief Includes structures definition of COS module.
*
*
* @version   4
********************************************************************************
*/

#ifndef __cpssCosTypes_h
#define __cpssCosTypes_h

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>

/* DSCP range limit */
#define CPSS_DSCP_RANGE_CNS  64

/* EXP range limit */
#define CPSS_EXP_RANGE_CNS  8

/* Traffic class range limit */
#define CPSS_TC_RANGE_CNS   8

/* User priority range limit */
#define CPSS_USER_PRIORITY_RANGE_CNS    8


/**
* @enum CPSS_DP_LEVEL_ENT
 *
 * @brief Enumeration of drop precedence levels.
*/
typedef enum{

    /** conforming */
    CPSS_DP_GREEN_E = 0,

    /** last conforming */
    CPSS_DP_YELLOW_E,

    /** non conforming */
    CPSS_DP_RED_E,

    /** last value (not to be used) */
    CPSS_DP_LAST_E

} CPSS_DP_LEVEL_ENT;



#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssCosTypes_h */


