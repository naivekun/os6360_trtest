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
* @file prvTgfBasicTests.h
*
* @brief Basic tests for IPFIX declarations
*
* @version   5
********************************************************************************
*/
#ifndef __prvTgfBasicTestsh
#define __prvTgfBasicTestsh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#if (defined CHX_FAMILY)
#include <cpss/dxCh/dxChxGen/ipfix/cpssDxChIpfix.h>
#endif

#include <utf/private/prvUtfExtras.h>

/* default VLAN0 Id */
#define PRV_TGF_VLANID_CNS  5

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS     1

/**
* @struct PRV_TGF_TEST_CONFIG_STC
 *
 * @brief Some test specific configuration parameters
*/
typedef struct{

    /** pcl rule index */
    GT_U32 ruleIndex;

    /** IPFIX entry index */
    GT_U32 ipfixIndex;

    /** num of packets to send */
    GT_U32 burstCount;

    /** each packet payload length */
    GT_U32 payloadLen;

    /** Command done on sampled packets. */
    PRV_TGF_IPFIX_SAMPLING_ACTION_ENT samplingAction;

    /** Sampling base method. */
    PRV_TGF_IPFIX_SAMPLING_MODE_ENT samplingMode;

    /** @brief Determines the random offset used for the
     *  reference value calculation in sampling.
     */
    PRV_TGF_IPFIX_SAMPLING_DIST_ENT randomFlag;

    /** @brief Window used for Reference value calculation.
     *  The units of this window are according to
     *  samplingMode.
     */
    GT_U64 samplingWindow;

    /** @brief 2 lsb bits of the cpu code for sampled packets.
     *  Relevant only when samplingAction is Mirror.
     */
    GT_U32 cpuSubCode;

} PRV_TGF_TEST_CONFIG_STC;

#ifndef PRV_TGF_TEST_DEFINITIONS

/* port bitmap VLAN members */
extern CPSS_PORTS_BMP_STC localPortsVlanMembers;

extern PRV_TGF_TEST_CONFIG_STC basicTestConf[5];

extern PRV_TGF_POLICER_STAGE_TYPE_ENT testedStage;

/******************************* Test packet **********************************/

/* L2 part of packet */
extern TGF_PACKET_L2_STC prvTgfPacketL2Part;

/* First VLAN_TAG part */
extern TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTag0Part;

/* DATA of packet */
extern GT_U8 prvTgfPayloadDataArr[];

/* PAYLOAD part */
extern TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart;

/* PARTS of packet */
extern TGF_PACKET_PART_STC prvTgfPacketPartArray[];

/* PACKET to send */
extern TGF_PACKET_STC prvTgfPacketInfo;

#endif /* PRV_TGF_TEST_DEFINITIONS */

/**
* @internal prvTgfIpfixStagesParamsSaveAndReset function
* @endinternal
*
* @brief   Save stages counting mode and enable metering state for later restore
*         and set stages to disable counting and metering.
*/
GT_VOID prvTgfIpfixStagesParamsSaveAndReset
(
    GT_VOID
);

/**
* @internal prvTgfIpfixStagesParamsRestore function
* @endinternal
*
* @brief   Restore stages counting mode and enable metering state.
*/
GT_VOID prvTgfIpfixStagesParamsRestore
(
    GT_VOID
);

/**
* @internal prvTgfIpfixIngressTestInit function
* @endinternal
*
* @brief   Set test configuration:
*         Enable the Ingress Policy Engine set Pcl rule.
*/
GT_VOID prvTgfIpfixIngressTestInit
(
    GT_VOID
);

/**
* @internal prvTgfIpfixFirstIngressTestInit function
* @endinternal
*
* @brief   Set test configuration:
*         Enable the Ingress Policy Engine set Pcl rule.
*/
GT_VOID prvTgfIpfixFirstIngressTestInit
(
    GT_VOID
);

/**
* @internal prvTgfIpfixFirstUseAllIngressTestInit function
* @endinternal
*
* @brief   Set test configuration:
*         Enable the Ingress Policy Engine set Pcl rule.
*/
GT_VOID prvTgfIpfixFirstUseAllIngressTestInit
(
    GT_VOID
);

/**
* @internal prvTgfIpfixSecondUseAllIngressTestInit function
* @endinternal
*
* @brief   Set test configuration:
*         Enable the Ingress Policy Engine set Pcl rule.
*/
GT_VOID prvTgfIpfixSecondUseAllIngressTestInit
(
    GT_VOID
);

/**
* @internal prvTgfIpfixTimestampVerificationTestInit function
* @endinternal
*
* @brief   Set test configuration:
*         Enable the Ingress Policy Engine set Pcl rule.
*/
GT_VOID prvTgfIpfixTimestampVerificationTestInit
(
    GT_VOID
);

/**
* @internal prvTgfIpfixIngressTestTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfIpfixIngressTestTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfIpfixTimestampVerificationTestTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfIpfixTimestampVerificationTestTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfIpfixIngressTestRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfIpfixIngressTestRestore
(
    GT_VOID
);

/**
* @internal prvTgfIpfixTrafficGenManager function
* @endinternal
*
* @brief   Perform the test traffic generation and checks for all port groups
*         (unaware mode) and again for first core.
*/
GT_VOID prvTgfIpfixTrafficGenManager
(
    GT_VOID (*initFuncPtr)(GT_VOID),
    GT_VOID (*trafficGenFuncPtr)(GT_VOID),
    GT_VOID (*restoreFuncPtr)(GT_VOID)
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfBasicTestsh */

