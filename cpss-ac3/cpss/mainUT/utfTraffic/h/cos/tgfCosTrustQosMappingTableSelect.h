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
* @file tgfCosTrustQosMappingTableSelect.h
*
* @brief Use User Priority as Mapping Table Index.
*
* @version   2
********************************************************************************
*/

#ifndef __tgfCosTrustQosMappingTableSelect
#define __tgfCosTrustQosMappingTableSelect

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @internal tgfCosTrustQosMappingTableSelectBridgeConfigSet function
* @endinternal
*
* @brief   Set Bridge Configuration
*/
GT_VOID tgfCosTrustQosMappingTableSelectBridgeConfigSet
(
     GT_VOID
);

/**
* @internal tgfCosTrustQosMappingTableSelectBuildPacket function
* @endinternal
*
* @brief   Build Packet
*
* @note Packet description:
*       TGF_PACKET_PART_L2_E
*       TGF_PACKET_PART_VLAN_TAG_E
*       TGF_PACKET_PART_IPV4_E
*       TGF_PACKET_PART_PAYLOAD_E
*
*/
GT_VOID tgfCosTrustQosMappingTableSelectBuildPacket
(
   GT_VOID
);

/**
* @internal tgfCosTrustQosMappingTableSelectCaseConfigurationSet function
* @endinternal
*
* @brief   Set specific configuration for two tests
*
* @param[in] testNumber               0 - test uses mapping table index
*                                      1 - user priority is used to select table
*                                       None
*/
GT_VOID tgfCosTrustQosMappingTableSelectCaseConfigurationSet
(
    IN GT_U32 testNumber
);

/**
* @internal tgfCosTrustQosMappingTableSelectCommonConfigurationSet function
* @endinternal
*
* @brief   Set Route Configuration
*/
GT_VOID tgfCosTrustQosMappingTableSelectCommonConfigurationSet
(
    GT_VOID
);

/**
* @internal tgfCosTrustQosMappingTableSelectTrafficOnPort function
* @endinternal
*
* @brief   Check traffic on egress port
*/
GT_VOID tgfCosTrustQosMappingTableSelectTrafficOnPort
(
    GT_U32 capturedPort
);

/**
* @internal tgfCosTrustQosMappingTableSelectTrafficGenerate function
* @endinternal
*
* @brief   Generane Traffic
*/
GT_VOID tgfCosTrustQosMappingTableSelectTrafficGenerate
(
    GT_VOID
);

/**
* @internal tgfCosTrustQosMappingTableSelectConfigurationRestore function
* @endinternal
*
* @brief   Restore Base Configuration
*/
GT_VOID tgfCosTrustQosMappingTableSelectConfigurationRestore
(
    GT_VOID
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __tgfCosTrustQosMappingTableSelect */

