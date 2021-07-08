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
* @file prvCpssDxChPort.h
*
* @brief Includes structures definition for the use of CPSS DxCh Port lib .
*
*
* @version   58
********************************************************************************
*/
#ifndef __prvCpssDxChPortStath
#define __prvCpssDxChPortStath

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>

/**
* @internal prvCpssDxChPortMacCountersIsSupportedCheck function
* @endinternal
*
* @brief   Checks if the counter is supported by current device and port.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] cntrName                 - counter name
* @param[in] portMacNum               - port MAC number
* @param[in] counterMode              - CPSS_DXCH_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_NORMAL_E -
*                                      MIB counter index 4 is sent deferred. MIB counter index 12 is Frames1024toMaxOctets
*                                      CPSS_DXCH_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_HISTOGRAM_1518_E -
*                                      MIB counter index 4 is Frames1024to1518Octets. MIB counter index 12 is Frames1519toMaxOctets
*
* @retval GT_TRUE                  - if counter is supported by current device and port
* @retval GT_FALSE                 - if counter is not supported by current device and port
*/
GT_BOOL prvCpssDxChPortMacCountersIsSupportedCheck
(
    IN  GT_U8                                                  devNum,
    IN  GT_U32                                                 portMacNum,
    IN  CPSS_PORT_MAC_COUNTERS_ENT                             cntrName,
    IN  CPSS_DXCH_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_ENT  counterMode
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssDxChPortStath */
