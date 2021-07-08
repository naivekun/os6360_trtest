/******************************************************************************
*              (c), Copyright 2017, Marvell International Ltd.                *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.  *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE       *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.    *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,      *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.  *
*******************************************************************************
*/
/**
********************************************************************************
* @file cpssPxTgfIngressIpFiltering.h
*
* @brief Enhanced UT for CPSS PX Ip filtering
*
* @version   1
********************************************************************************
*/
#ifndef __cpssPxTgfIngressIpFiltering_h__
#define __cpssPxTgfIngressIpFiltering_h__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>

/**
* @internal prvTgfPxIpClassificationConfig function
* @endinternal
*
* @brief   Save or restore IP classification original configuration
*
* @param[in] dev                      - device number
* @param[in] configure                - GT_TRUE - save configuration
*                                      GT_FALSE - restore configuration
*                                       None
*/
void prvTgfPxIpClassificationConfig
(
    GT_U8   dev,
    GT_BOOL configure
);

/**
* @internal prvTgfPxIpClassificationExecute function
* @endinternal
*
* @brief   Execute IP address classification test.
*
* @param[in] dev                      - device number
*                                       None
*/
void prvTgfPxIpClassificationExecute
(
    GT_U8 dev
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssPxTgfIngressIpFiltering_h__ */

