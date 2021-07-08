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
* @file common/h/cpss/common/networkIf/cpssGenNetIfTypes.h
* @version   9
********************************************************************************
*/

#ifndef __cpssGenNetIfTypes_h
#define __cpssGenNetIfTypes_h

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/************ Includes ********************************************************/
#include <cpss/common/cpssTypes.h>
#include <cpss/common/cos/cpssCosTypes.h>
#include <cpss/common/config/private/prvCpssGenIntDefs.h>

/**
* @enum CPSS_NET_CPU_PORT_MODE_ENT
 *
 * @brief Defines the possible CPU network interface modes
*/
typedef enum{

    /** CPU Tx/Rx by SDMA */
    CPSS_NET_CPU_PORT_MODE_SDMA_E,

    /** CPU Tx/Rx by MII */
    CPSS_NET_CPU_PORT_MODE_MII_E,

    /** This device isn't connected to CPU */
    CPSS_NET_CPU_PORT_MODE_NONE_E

} CPSS_NET_CPU_PORT_MODE_ENT;

/**
* @struct CPSS_NET_SDMA_RX_ERROR_COUNTERS_STC
 *
 * @brief information about Rx Error counters -- per device
*/
typedef struct{

    GT_U32 counterArray[NUM_OF_SDMA_QUEUES];

} CPSS_NET_SDMA_RX_ERROR_COUNTERS_STC;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif   /* __cpssGenNetIfTypes_h */



