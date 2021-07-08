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
* @file cstBridge.h
*
* @brief Generic API for Bridge
*
* @version   70
********************************************************************************
*/
#ifndef __cstBridgeh
#define __cstBridgeh

#include <common/tgfTunnelGen.h>
#include <common/tgfInlifGen.h>

#include <cpss/generic/bridge/cpssGenBrgFdb.h>
#include <cpss/generic/bridge/cpssGenBrgGen.h>
#include <cpss/generic/bridge/cpssGenBrgVlanTypes.h>
#include <cpss/generic/bridge/cpssGenBrgSrcId.h>
#include <cpss/generic/bridge/cpssGenBrgSecurityBreachTypes.h>
#include <cpss/generic/ip/cpssIpTypes.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#ifdef CHX_FAMILY

/**
* @internal cstBrgDefVlanEntryWithPortsSet function
* @endinternal
*
* @brief   Set vlan entry, with the requested ports, tagging
*
* @param[in] devNum                   - device number
* @param[in] vlanId                   - the vlan id
* @param[in] portsArray[]             - array of ports
* @param[in] devArray[]               - array of devices (when NULL -> assume all ports belong to devNum)
* @param[in] tagArray[]               - array of tagging for the ports (when NULL -> assume all ports untagged)
* @param[in] numPorts                 - number of ports in the arrays
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on HW error
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS cstBrgDefVlanEntryWithPortsSet
(
    IN GT_U8                          devNum,
    IN GT_U16                         vlanId,
    IN GT_U32                          portsArray[],
    IN GT_U8                          devArray[],
    IN GT_U8                          tagArray[],
    IN GT_U32                         numPorts
);

/**
* @internal cstBrgFdbFlush function
* @endinternal
*
* @brief   Flush FDB table (with/without static entries).
*         function sets actDev and it's mask to 'dont care' so all entries can be
*         flushed (function restore default values at end of operation).
* @param[in] includeStatic            - include static entries
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
*/
GT_STATUS cstBrgFdbFlush
(
    IN GT_BOOL                        includeStatic
);

/**
* @internal cstBrgDefVlanEntryInvalidate function
* @endinternal
*
* @brief   This function invalidates VLAN entry and reset all it's content
*
* @param[in] devNum                   - device number
* @param[in] vlanId                   - VLAN id
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - otherwise
* @retval GT_BAD_PARAM             - wrong devNum or vid
*/
GT_STATUS cstBrgDefVlanEntryInvalidate
(
    IN GT_U8                devNum,
    IN GT_U16               vlanId
);



#endif /* CHX_FAMILY */
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __tgfBridgeGenh */



