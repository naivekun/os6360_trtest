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
* @file prvTgfPclBc2.h
*
* @brief SIP5 specific PCL features testing
*
* @version   9
********************************************************************************
*/
#ifndef __prvTgfPclBc2h
#define __prvTgfPclBc2h

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @internal prvTgfPclBc2Mac2meTest function
* @endinternal
*
* @brief   Main test function
*/
GT_VOID prvTgfPclBc2Mac2meTest
(
    GT_VOID
);

/**
* @internal prvTgfPclBc2Mac2meTestRestore function
* @endinternal
*
* @brief   Restore after test configuration
*/
GT_VOID prvTgfPclBc2Mac2meTestRestore
(
    GT_VOID
);

/**
* @internal prvTgfPclBc2SetMacSaTest function
* @endinternal
*
* @brief   Set test configuration
*/
GT_VOID prvTgfPclBc2SetMacSaTest
(
    GT_VOID
);

/**
* @internal prvTgfPclBc2SetMacSaTestRestore function
* @endinternal
*
* @brief   Restore after test configuration
*/
GT_VOID prvTgfPclBc2SetMacSaTestRestore
(
    GT_VOID
);

/**
* @internal prvTgfPclBc2MapMplsChannelToOamOpcodeTest function
* @endinternal
*
* @brief   Set test configuration and traffic checking
*/
GT_VOID prvTgfPclBc2MapMplsChannelToOamOpcodeTest
(
    GT_VOID
);

/**
* @internal prvTgfPclBc2MapMplsChannelToOamOpcodeReset function
* @endinternal
*
* @brief   Set reset test configuration
*/
GT_VOID prvTgfPclBc2MapMplsChannelToOamOpcodeReset
(
    GT_VOID
);

/**
* @internal prvTgfPclBc2VidTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*
* @param[in] prvTgfPacketInfoPtr      - (pointer to) packet info
*                                       None
*/
GT_VOID prvTgfPclBc2VidTrafficGenerate
(
    IN TGF_PACKET_STC *prvTgfPacketInfoPtr
);

/**
* @internal prvTgfPclBc2TrafficEgressVidCheck function
* @endinternal
*
* @brief   Checks traffic egress VID in the Tag
*
* @param[in] portIndex                - port index
* @param[in] egressVid                - VID found in egressed packets VLAN Tag
* @param[in] checkMsb                 - to check High bits of VID and UP
*                                       None
*/
GT_VOID prvTgfPclBc2TrafficEgressVidCheck
(
    IN GT_U32  portIndex,
    IN GT_U16  egressVid,
    IN GT_BOOL checkMsb
);

/**
* @internal prvTgfPclBc2Mac2meSetBaseConfiguration function
* @endinternal
*
* @brief   Setup base configuration for Mac2me Test
*/
GT_VOID prvTgfPclBc2Mac2meSetBaseConfiguration
(
    GT_VOID
);

/**
* @internal prvTgfPclBc2Mac2meCheckMac2MeWithPclRule function
* @endinternal
*
* @brief   Setup PCL rule to modify packet VID if mac2me is set
*/
GT_VOID prvTgfPclBc2Mac2meCheckMac2MeWithPclRule
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfPclBc2h */


