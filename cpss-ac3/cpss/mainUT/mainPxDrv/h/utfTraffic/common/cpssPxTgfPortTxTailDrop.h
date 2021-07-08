/*******************************************************************************
*              (c), Copyright 2017, Marvell International Ltd.                 *
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
* @file cpssPxTgfPortTxTailDrop.h
*
* @brief CPSS PX Port Tx Tail Drop traffic helper API for enhanced UTs
*
* @version   1
********************************************************************************
*/
#ifndef __cpssPxTgfPortTxTailDrop_h__
#define __cpssPxTgfPortTxTailDrop_h__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpss/px/port/cpssPxPortTxTailDrop.h>


/* Convert Tail Drop DBA Alpha to ratio value (GT_FLOAT32) */
#define PRV_CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_TO_RATIO_CONVERT_MAC(alpha, ratio) \
    do {                                                                    \
        switch (alpha)                                                      \
        {                                                                   \
            case CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_0_E:                   \
                ratio = (0.0);                                              \
                break;                                                      \
            case CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_0_25_E:                \
                ratio = (0.25);                                             \
                break;                                                      \
            case CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_0_5_E:                 \
                ratio = (0.5);                                              \
                break;                                                      \
            case CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_1_E:                   \
                ratio = (1.0);                                              \
                break;                                                      \
            case CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_2_E:                   \
                ratio = (2.0);                                              \
                break;                                                      \
            case CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_4_E:                   \
                ratio = (4.0);                                              \
                break;                                                      \
            default:                                                        \
                return GT_BAD_PARAM;                                        \
        }                                                                   \
    } while (0)


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssPxTgfPortTxTailDrop_h__ */

