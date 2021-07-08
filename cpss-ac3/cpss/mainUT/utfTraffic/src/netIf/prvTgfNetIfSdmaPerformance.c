/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvTgfNetIfRxSdmaPerformance.c
*
* DESCRIPTION:
*      Rx SDMA performance testing
*
* FILE REVISION NUMBER:
*       $Revision: 12 $
*
*******************************************************************************/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/generic/pcl/cpssPcl.h>
#include <cpss/generic/cpssCommonDefs.h>


#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <utf/private/prvUtfHelpers.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfNetIfGen.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfPortGen.h>
#include <common/tgfCscdGen.h>
#include <netIf/prvTgfNetIfSdmaPerformance.h>

/* variables to save configuration before test and restore after it */
static CPSS_PORTS_BMP_STC savePortsMembers;
static GT_U32 savePcktSizeMru;

/* variables to save configuration */
static GT_U32 savePortMaxBuffLimit;
static GT_U32 savePortMaxDescrLimit;
static GT_U32 saveCpuPortMaxBuffLimit;
static GT_U32 saveCpuPortMaxDescrLimit;
static GT_U32  savePrvTgfPortsZero;

static  PRV_TGF_PORT_TX_Q_TAIL_DROP_PROF_TC_PARAMS saveTailDropProfileTc0Params;
static  PRV_TGF_PORT_TX_Q_TAIL_DROP_PROF_TC_PARAMS saveTailDropProfileTc7Params;
static  PRV_TGF_PORT_TX_Q_TAIL_DROP_PROF_TC_PARAMS saveCpuTailDropProfileTc0Params;

/* default VLAN Id */
#define PRV_TGF_DEF_VLANID_CNS  1

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS     3

/* Tx port number For Rx To Tx */
#define PRV_TGF_SEND_PORT_RX_TO_TX_IDX_CNS     0

/* maximal packet size for test */
#define PRV_TGF_MAX_PACKET_SIZE_CNS     10240

/* MRU default value */
#define PRV_TGF_DEF_MRU_VALUE_CNS 1518

/* Tail Drop Limits to support Jumbo packets up to 10256 (10240 + 16 eDSA)
   Each packet use 41 PP's buffer. WS UT use 30 packets.
   Set limit to comprize double buffers amount. */
#define PRV_TGF_TAIL_DROP_DESCR_LIMIT_CNS (60)
#define PRV_TGF_TAIL_DROP_BUFF_LIMIT_CNS (PRV_TGF_TAIL_DROP_DESCR_LIMIT_CNS * 41)

/* list of packet sizes for tests */
static  GT_U32  prvTgfNetIfSdmaPerformancePacketSizeArr[] = {64, 128, 256, 384, 512, 768, 1024, 1518, 2000, 4000, 8000, 10232, PRV_TGF_MAX_PACKET_SIZE_CNS};

/******************************* Test packet **********************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part = {
    {0x00, 0xAA, 0xBB, 0xCC, 0x12, 0x98},               /* daMac */
    {0x00, 0xFF, 0xEE, 0x55, 0x00, 0x01}                /* saMac */
};

/* packet's ethertype */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketEtherTypePart = {TGF_ETHERTYPE_0800_IPV4_TAG_CNS};

/* packet's IPv4 */
static TGF_PACKET_IPV4_STC prvTgfPacketIpv4Part = {
    4,                  /* version */
    5,                  /* headerLen */
    0,                  /* typeOfService */
    0,                  /* totalLen - set dynamically */
    0,                  /* id */
    0,                  /* flags */
    0,                  /* offset */
    0x40,               /* timeToLive */
    0x04,               /* protocol */
    TGF_PACKET_AUTO_CALC_CHECKSUM_CNS,     /* csum */
    { 1,  1,  1,  1},   /* srcAddr */
    { 1,  1,  1,  3}    /* dstAddr */
};

/* PAYLOAD part - allocated dynamically */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart = {
    0,                       /* dataLength */
    NULL                                /* dataPtr */
};

/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfPacketPartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacketIpv4Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};


/******************************************************************************\
 *                            Private test functions                          *
\******************************************************************************/

/**
* @internal prvTgfNetIfRxSdmaPerformanceTestInit function
* @endinternal
*
* @brief   Various test initializations
*/
GT_VOID prvTgfNetIfRxSdmaPerformanceTestInit
(
    GT_VOID
)
{
    GT_STATUS rc;
    CPSS_PORTS_BMP_STC                  portsTagging;
    GT_BOOL                             isValid;
    PRV_TGF_BRG_VLAN_INFO_STC           vlanInfo;
    PRV_TGF_BRG_VLAN_PORTS_TAG_CMD_STC  portsTaggingCmd;
    CPSS_PORTS_BMP_STC                  portsMembers;
    GT_U32                              maxPayloadSize;
    GT_U32                              vlanMruIdx;
    GT_U32                              ii;
    PRV_TGF_PORT_TX_Q_TAIL_DROP_PROF_TC_PARAMS setTailDropProfileParams;
    CPSS_PORT_TX_DROP_PROFILE_SET_ENT   portTailDropProfile;
    CPSS_PORT_TX_DROP_PROFILE_SET_ENT   cpuPortTailDropProfile;

    /* AUTODOC: Reset ports counters */
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfEthCountersReset: %d", prvTgfDevNum);

    /* AUTODOC: TRAP unknown UC packets to CPU by use MIRROR_TO_CPU command and none port members */
    /* AUTODOC: Disable send NA to CPU in order to avoid influence on RX SDMA. */

    /* Read Vlan entry parametrs */
    rc = prvTgfBrgVlanEntryRead(prvTgfDevNum, PRV_TGF_DEF_VLANID_CNS,
                                &savePortsMembers,
                                &portsTagging,
                                &isValid,
                                &vlanInfo,
                                &portsTaggingCmd);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEntryRead %d", prvTgfDevNum);

    /* get MRU index */
    vlanMruIdx = vlanInfo.mruIdx;

    /* remove all ports from VLAN */
    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsMembers);

    /* send unknown packets only to CPU */
    vlanInfo.unkUcastCmd = CPSS_PACKET_CMD_MIRROR_TO_CPU_E;
    vlanInfo.naMsgToCpuEn = GT_FALSE;

    rc = prvTgfBrgVlanEntryWrite(prvTgfDevNum, PRV_TGF_DEF_VLANID_CNS,
                                &portsMembers,
                                &portsTagging,
                                &vlanInfo,
                                &portsTaggingCmd);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEntryWrite %d", prvTgfDevNum);

    /* allocate payload for packet */
    maxPayloadSize = PRV_TGF_MAX_PACKET_SIZE_CNS
                    - 12 /* MAC header */ - 4 /* etherType*/
                     - (prvTgfPacketIpv4Part.headerLen * 4);

    prvTgfPacketPayloadPart.dataPtr = cpssOsMalloc(maxPayloadSize);
    for (ii = 0; ii < maxPayloadSize; ii++)
    {
        prvTgfPacketPayloadPart.dataPtr[ii] = (GT_U8)(ii & 0xFF);
    }

    /* AUTODOC: enable Jumbo frames for all used ports */
    /* get default MRU for ports */
    rc = prvTgfPortMruGet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                          &savePcktSizeMru);
    UTF_VERIFY_EQUAL1_STRING_MAC(rc, GT_OK, "ERROR of prvTgfPortMruGet %d", GT_TRUE);

    /* AUTODOC: set MRU value 10240 for all ports */
    rc = prvTgfPortMruSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], PRV_TGF_MAX_PACKET_SIZE_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of prvTgfPortMruSet: %d",
                                 prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* AUTODOC: set MRU for each profiles to 10240) */
    rc = prvTgfBrgVlanMruProfileValueSet(vlanMruIdx, PRV_TGF_MAX_PACKET_SIZE_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of prvTgfBrgVlanMruProfileValueSet: %d",
                                 vlanMruIdx);

    /* set UTF Rx Packet callback to count packets only */
    tgfTrafficTableRxDoCountOnlySet(GT_TRUE);

    /* AUTODOC: enable tail drop by set HOL mode */
    /* AUTODOC: configure Tail Drop to support Jumbo packets */
    rc = prvTgfPortFcHolSysModeSet(prvTgfDevNum, PRV_TGF_PORT_PORT_HOL_E);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPortFcHolSysModeSet: %d, %d",
                                 prvTgfDevNum, PRV_TGF_PORT_PORT_HOL_E);
    /* get tail drop profile */
    rc = prvTgfPortTxBindPortToDpGet(prvTgfDevNum,
                                     prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                     &portTailDropProfile);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPortTxBindPortToDpGet: %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    rc = prvTgfPortTxBindPortToDpGet(prvTgfDevNum,
                                     CPSS_CPU_PORT_NUM_CNS,
                                     &cpuPortTailDropProfile);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPortTxBindPortToDpGet: %d, %d",
                                 prvTgfDevNum, CPSS_CPU_PORT_NUM_CNS);

    /* save profile settings */
    rc = prvTgfPortTxTailDropProfileWithoutAlphaGet(prvTgfDevNum, portTailDropProfile,
                                        &savePortMaxBuffLimit,
                                        &savePortMaxDescrLimit);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPortTxTailDropProfileWithoutAlphaGet: %d, %d",
                                 prvTgfDevNum, portTailDropProfile);

    /* save CPU port profile settings */
    rc = prvTgfPortTxTailDropProfileWithoutAlphaGet(prvTgfDevNum, cpuPortTailDropProfile,
                                        &saveCpuPortMaxBuffLimit,
                                        &saveCpuPortMaxDescrLimit);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPortTxTailDropProfileWithoutAlphaGet: %d, %d",
                                 prvTgfDevNum, portTailDropProfile);

    /* AUTODOC: set tail drop profile: */
    /* AUTODOC:   portMaxBuffLimit=60*41, portMaxDescrLimit=60 */
    rc = prvTgfPortTxTailDropProfileWithoutAlphaSet(prvTgfDevNum, portTailDropProfile,
        PRV_TGF_TAIL_DROP_BUFF_LIMIT_CNS, PRV_TGF_TAIL_DROP_DESCR_LIMIT_CNS);
    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc,
        "prvTgfPortTxTailDropProfileWithoutAlphaSet: %d, %d, %d, %d",
        prvTgfDevNum, portTailDropProfile,
        PRV_TGF_TAIL_DROP_BUFF_LIMIT_CNS, PRV_TGF_TAIL_DROP_DESCR_LIMIT_CNS);

    rc = prvTgfPortTxTailDropProfileWithoutAlphaSet(prvTgfDevNum, cpuPortTailDropProfile,
        PRV_TGF_TAIL_DROP_BUFF_LIMIT_CNS, PRV_TGF_TAIL_DROP_DESCR_LIMIT_CNS);
    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "prvTgfPortTxTailDropProfileWithoutAlphaSet: %d, %d, %d, %d",
                                 prvTgfDevNum, cpuPortTailDropProfile,
                                 PRV_TGF_TAIL_DROP_BUFF_LIMIT_CNS, PRV_TGF_TAIL_DROP_DESCR_LIMIT_CNS);

    /* store per TC and DP settings */
    rc = prvTgfPortTx4TcTailDropProfileGet(prvTgfDevNum, portTailDropProfile,
                                           0, &saveTailDropProfileTc0Params);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPortTx4TcTailDropProfileGet: %d, %d, %d",
                                 prvTgfDevNum, portTailDropProfile, 0);

    rc = prvTgfPortTx4TcTailDropProfileGet(prvTgfDevNum, portTailDropProfile,
                                           7, &saveTailDropProfileTc7Params);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPortTx4TcTailDropProfileGet: %d, %d, %d",
                                 prvTgfDevNum, portTailDropProfile, 7);


    /* save only TC0 for CPU port */
    rc = prvTgfPortTx4TcTailDropProfileGet(prvTgfDevNum, cpuPortTailDropProfile,
                                           0, &saveCpuTailDropProfileTc0Params);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPortTx4TcTailDropProfileGet: %d, %d, %d",
                                 prvTgfDevNum, cpuPortTailDropProfile, 0);

    cpssOsMemSet(&setTailDropProfileParams, 0, sizeof(setTailDropProfileParams));

    setTailDropProfileParams.dp0MaxBuffNum  =
    setTailDropProfileParams.dp1MaxBuffNum  =
    setTailDropProfileParams.dp2MaxBuffNum  =
    setTailDropProfileParams.tcMaxBuffNum   = PRV_TGF_TAIL_DROP_BUFF_LIMIT_CNS;

    setTailDropProfileParams.dp0MaxDescrNum =
    setTailDropProfileParams.dp1MaxDescrNum =
    setTailDropProfileParams.dp2MaxDescrNum =
    setTailDropProfileParams.tcMaxDescrNum  = PRV_TGF_TAIL_DROP_DESCR_LIMIT_CNS;

    /* AUTODOC: set tail drop profiles limits for TC 0: */
    /* AUTODOC:   all descriptor limits = 60  */
    /* AUTODOC:   all buffers limits = 60*41  */
    rc = prvTgfPortTx4TcTailDropProfileSet(prvTgfDevNum, portTailDropProfile,
                                           0, &setTailDropProfileParams);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPortTx4TcTailDropProfileSet: %d, %d, %d",
                                 prvTgfDevNum, portTailDropProfile, 0);

    /* AUTODOC: set tail drop profiles limits for TC 7: */
    /* AUTODOC:   all descriptor limits = 60  */
    /* AUTODOC:   all buffers limits = 60*41  */
    rc = prvTgfPortTx4TcTailDropProfileSet(prvTgfDevNum, portTailDropProfile,
                                           7, &setTailDropProfileParams);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPortTx4TcTailDropProfileSet: %d, %d, %d",
                                 prvTgfDevNum, portTailDropProfile, 7);

    /* AUTODOC: set CPU port tail drop profiles limits for TC 0: */
    /* AUTODOC:   all descriptor limits = 60  */
    /* AUTODOC:   all buffers limits = 60*41  */
    rc = prvTgfPortTx4TcTailDropProfileSet(prvTgfDevNum, cpuPortTailDropProfile,
                                           0, &setTailDropProfileParams);
    UTF_VERIFY_EQUAL3_STRING_MAC(rc, GT_OK, "prvTgfPortTx4TcTailDropProfileSet: %d, %d, %d",
                                 prvTgfDevNum, cpuPortTailDropProfile, 0);

    /* AUTODOC: Disable TX DMA interrups to avoid interrupts influence on test results */
    rc = cpssEventDeviceMaskSet(prvTgfDevNum, CPSS_PP_TX_END_E, CPSS_EVENT_MASK_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(rc, GT_OK, "cpssEventDeviceMaskSet %d", prvTgfDevNum);

    rc = cpssEventDeviceMaskSet(prvTgfDevNum, CPSS_PP_TX_ERR_QUEUE_E, CPSS_EVENT_MASK_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(rc, GT_OK, "cpssEventDeviceMaskSet %d", prvTgfDevNum);
}

/**
* @internal prvTgfNetIfRxToTxSdmaPerformanceTestInit function
* @endinternal
*
* @brief   Various test initializations
*/
GT_VOID prvTgfNetIfRxToTxSdmaPerformanceTestInit
(
    GT_VOID
)
{
    GT_STATUS rc;
    CPSS_INTERFACE_INFO_STC portInterface;

    /* AUTODOC: change port 0 to be 10G one */
    if (PRV_CPSS_SIP_5_CHECK_MAC(prvTgfDevNum))
    {
        savePrvTgfPortsZero = prvTgfPortsArray[PRV_TGF_SEND_PORT_RX_TO_TX_IDX_CNS];
        prvTgfPortsArray[PRV_TGF_SEND_PORT_RX_TO_TX_IDX_CNS] = 50;

        switch (PRV_CPSS_PP_MAC(prvTgfDevNum)->devType)
        {
            case CPSS_BOBK_CETUS_DEVICES_CASES_MAC:
                prvTgfPortsArray[PRV_TGF_SEND_PORT_RX_TO_TX_IDX_CNS] = 58;
                break;
            case CPSS_BOBK_CAELUM_DEVICES_CASES_MAC:
                prvTgfPortsArray[PRV_TGF_SEND_PORT_RX_TO_TX_IDX_CNS] = 56;
                break;
            case CPSS_ALDRIN_DEVICES_CASES_MAC:
                prvTgfPortsArray[PRV_TGF_SEND_PORT_RX_TO_TX_IDX_CNS] = 20;
                break;
            default:
                break;
        }
    }

    /* call common init */
    prvTgfNetIfRxSdmaPerformanceTestInit();

    /* set port for Tx */
    portInterface.type = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum = prvTgfDevNum;
    portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_SEND_PORT_RX_TO_TX_IDX_CNS];

    /*AUTODOC: configure port to Tx. Use untagged packet and automatic DSA definition. */
    rc = prvTgfTrafficGeneratorRxToTxInfoSet(&portInterface, 0, 0, 4094, GT_FALSE,
                                             GT_FALSE, /* ignored*/ TGF_DSA_1_WORD_TYPE_E);

    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTrafficGeneratorRxToTxInfoSet: %d", prvTgfDevNum);

    /*AUTODOC: enable Rx to Tx */
    tgfTrafficTableRxToTxSet(GT_TRUE);

    /* disable Do Counting only mode to get Rx to Tx processing */
    tgfTrafficTableRxDoCountOnlySet(GT_FALSE);
}

/**
* @internal prvTgfNetIfRxSdmaPerformanceTestRestore function
* @endinternal
*
* @brief   Restore test configurations
*/
GT_VOID prvTgfNetIfRxSdmaPerformanceTestRestore
(
    GT_VOID
)
{
    GT_STATUS rc;
    CPSS_PORTS_BMP_STC                  portsMembers;
    CPSS_PORTS_BMP_STC                  portsTagging;
    GT_BOOL                             isValid;
    PRV_TGF_BRG_VLAN_INFO_STC           vlanInfo;
    PRV_TGF_BRG_VLAN_PORTS_TAG_CMD_STC  portsTaggingCmd;
    GT_PHYSICAL_PORT_NUM                portNum;
    GT_U32                              vlanMruIdx;
    CPSS_PORT_TX_DROP_PROFILE_SET_ENT   portTailDropProfile;
    CPSS_PORT_TX_DROP_PROFILE_SET_ENT   cpuPortTailDropProfile;

    /* free payload memory */
    cpssOsFree(prvTgfPacketPayloadPart.dataPtr);

    /* AUTODOC: Clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* AUTODOC: restore default VLAN entry unicast command, NA to CPU and ports */
    rc = prvTgfBrgVlanEntryRead(prvTgfDevNum, PRV_TGF_DEF_VLANID_CNS,
                                &portsMembers,
                                &portsTagging,
                                &isValid,
                                &vlanInfo,
                                &portsTaggingCmd);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEntryRead %d", prvTgfDevNum);

    vlanMruIdx = vlanInfo.mruIdx;

    /* set default values */
    vlanInfo.unkUcastCmd = CPSS_PACKET_CMD_FORWARD_E;
    vlanInfo.naMsgToCpuEn = GT_TRUE;

    rc = prvTgfBrgVlanEntryWrite(prvTgfDevNum, PRV_TGF_DEF_VLANID_CNS,
                                &savePortsMembers,
                                &portsTagging,
                                &vlanInfo,
                                &portsTaggingCmd);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEntryWrite %d", prvTgfDevNum);

    /* AUTODOC: restore default MRU for all ports */
    rc = prvTgfPortMruSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                          savePcktSizeMru);
    UTF_VERIFY_EQUAL1_STRING_MAC(rc, GT_OK, "ERROR of prvTgfPortMruSet %d", GT_TRUE);

    /* AUTODOC: restore MRU for default VLAN) */
    rc = prvTgfBrgVlanMruProfileValueSet(vlanMruIdx, PRV_TGF_DEF_MRU_VALUE_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfBrgVlanMruProfileValueSet");

    portNum = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS];

    /* configure default mode */
    rc = prvTgfTxModeSetupEth(prvTgfDevNum, portNum,
                              PRV_TGF_TX_MODE_SINGLE_BURST_E,
                              PRV_TGF_TX_CONT_MODE_UNIT_WS_E,
                              0);

    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfTxModeSetupEth: %d, %d",
                                 prvTgfDevNum, portNum);

    /* restore regular UTF processing of RX To CPU packets */
    tgfTrafficTableRxDoCountOnlySet(GT_FALSE);

    /* get tail drop profile */
    rc = prvTgfPortTxBindPortToDpGet(prvTgfDevNum,
                                     prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                     &portTailDropProfile);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPortTxBindPortToDpGet: %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    rc = prvTgfPortTxBindPortToDpGet(prvTgfDevNum,
                                     CPSS_CPU_PORT_NUM_CNS,
                                     &cpuPortTailDropProfile);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPortTxBindPortToDpGet: %d, %d",
                                 prvTgfDevNum, CPSS_CPU_PORT_NUM_CNS);

    /* AUTODOC: restore tail drop profile: */
    rc = prvTgfPortTxTailDropProfileWithoutAlphaSet(prvTgfDevNum, portTailDropProfile,
                                        savePortMaxBuffLimit, savePortMaxDescrLimit);
    UTF_VERIFY_EQUAL4_STRING_MAC(rc, GT_OK, "prvTgfPortTxTailDropProfileWithoutAlphaSet: %d, %d, %d, %d",
                                 prvTgfDevNum, portTailDropProfile,
                                 savePortMaxBuffLimit, savePortMaxDescrLimit);

    /* AUTODOC: restore CPU Port tail drop profile: */
    rc = prvTgfPortTxTailDropProfileWithoutAlphaSet(prvTgfDevNum, cpuPortTailDropProfile,
                                        saveCpuPortMaxBuffLimit, saveCpuPortMaxDescrLimit);
    UTF_VERIFY_EQUAL4_STRING_MAC(rc, GT_OK, "prvTgfPortTxTailDropProfileWithoutAlphaSet: %d, %d, %d, %d",
                                 prvTgfDevNum, cpuPortTailDropProfile,
                                 saveCpuPortMaxBuffLimit, saveCpuPortMaxDescrLimit);

    /* AUTODOC: restore tail drop profiles limits for TC 0: */
    rc = prvTgfPortTx4TcTailDropProfileSet(prvTgfDevNum, portTailDropProfile,
                                           0, &saveTailDropProfileTc0Params);
    UTF_VERIFY_EQUAL3_STRING_MAC(rc, GT_OK, "prvTgfPortTx4TcTailDropProfileSet: %d, %d, %d",
                                 prvTgfDevNum, portTailDropProfile, 0);

    /* AUTODOC: restore tail drop profiles limits for TC 7: */
    rc = prvTgfPortTx4TcTailDropProfileSet(prvTgfDevNum, portTailDropProfile,
                                           7, &saveTailDropProfileTc7Params);
    UTF_VERIFY_EQUAL3_STRING_MAC(rc, GT_OK, "prvTgfPortTx4TcTailDropProfileSet: %d, %d, %d",
                                 prvTgfDevNum, portTailDropProfile, 7);

    /* AUTODOC: restore CPU Port tail drop profiles limits for TC 0: */
    rc = prvTgfPortTx4TcTailDropProfileSet(prvTgfDevNum, cpuPortTailDropProfile,
                                           0, &saveCpuTailDropProfileTc0Params);
    UTF_VERIFY_EQUAL3_STRING_MAC(rc, GT_OK, "prvTgfPortTx4TcTailDropProfileSet: %d, %d, %d",
                                 prvTgfDevNum, cpuPortTailDropProfile, 0);

    /* AUTODOC: Enable TX DMA interrups as was */
    rc = cpssEventDeviceMaskSet(prvTgfDevNum, CPSS_PP_TX_END_E, CPSS_EVENT_UNMASK_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(rc, GT_OK, "cpssEventDeviceMaskSet %d", prvTgfDevNum);

    rc = cpssEventDeviceMaskSet(prvTgfDevNum, CPSS_PP_TX_ERR_QUEUE_E, CPSS_EVENT_UNMASK_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(rc, GT_OK, "cpssEventDeviceMaskSet %d", prvTgfDevNum);
}

/**
* @internal prvTgfNetIfRxToTxSdmaPerformanceTestRestore function
* @endinternal
*
* @brief   Restore test configurations for Rx To Tx test
*/
GT_VOID prvTgfNetIfRxToTxSdmaPerformanceTestRestore
(
    GT_VOID
)
{
    /* common restore procedure */
    prvTgfNetIfRxSdmaPerformanceTestRestore();

    /* AUTODOC: restore port 0 */
    if (PRV_CPSS_SIP_5_CHECK_MAC(prvTgfDevNum))
    {
        prvTgfPortsArray[PRV_TGF_SEND_PORT_RX_TO_TX_IDX_CNS] = savePrvTgfPortsZero;
    }

    /*AUTODOC: disable Rx to Tx */
    tgfTrafficTableRxToTxSet(GT_FALSE);
}
/**
* @internal prvTgfNetIfRxSdmaPerformanceMeasure function
* @endinternal
*
* @brief   Measure RX SDMA rate and print it
*
* @param[in] testedPacketSize         - packet size include CRC
* @param[in] dsaSizeInBytes           - DSA tag size
* @param[in] quickCheck               - GT_FALSE - use full check of rate during 1 second
*                                      GT_TRUE  - use quick check. Use minimal time for rate calculation.
*                                      test runs quick but results are not accurate
* @param[in] level                    - recursion level
* @param[in] printResults             - print results
* @param[in] rxToTxEnable             - GT_FALSE - do not perform calculations for Rx to Tx mode
*                                      GT_FALSE - perform calculations for Rx to Tx mode
*                                       None
*/
GT_VOID prvTgfNetIfRxSdmaPerformanceMeasure
(
    GT_U32  testedPacketSize,
    GT_U32  dsaSizeInBytes,
    GT_BOOL quickCheck,
    GT_U32  level,
    GT_BOOL printResults,
    GT_BOOL rxToTxEnable
)
{
    GT_U32 numOfPackets1;        /* number of RX SDMA packets before sleep */
    GT_U32 numOfPackets2;        /* number of RX SDMA packets after sleep */
    double numOfPacketsDiff;     /* number of RX SDMA packets during sleep */
    double packetRate;           /* RX SDMA packets rate in PPS */
    double rxSdmaBitRate;        /* RX SDMA packets bits rate in bps */
    GT_U32 rxSdmaPacketSize;     /* RX SDMA packets size */
    GT_U32 startSec, endSec;     /* start and end time seconds part */
    GT_U32 startNano, endNano;   /* start and end time nanoseconds part */
    double startMilli, endMilli; /* start and end time in milliseconds */
    double timeOutMilli;         /* timeout in milliseconds */
    GT_U32 packetRate_u32;       /* RX SDMA packets rate in PPS */
    GT_U32 rateCheckTimeout;     /* timeout to check rate */
    GT_U32 portRate = 0;         /* port rate */
    GT_U32 rateDiff;             /* rate difference in percent */

    if (quickCheck == GT_FALSE)
    {
        /* provide time for previous print outs */
        cpssOsTimerWkAfter(100);

        /* Full check use 1 second */
        rateCheckTimeout = 1000;
    }
    else
    {
        /* Quick check use 10 milisecond. Sometimes RX Handler does not get
           time during 10 mili sleep because of UART or other tasks.
           Check rate one more time with bigger sleep. */
        rateCheckTimeout = 10 + 100 * level;
    }

    numOfPackets1 = tgfTrafficTableRxCountGet();

    /* store start time */
    cpssOsTimeRT(&startSec,&startNano);

    cpssOsTimerWkAfter(rateCheckTimeout);

    numOfPackets2 = tgfTrafficTableRxCountGet();

    /* get end time */
    cpssOsTimeRT(&endSec,&endNano);

    startMilli = startSec * 1000 + startNano / 1000000;
    endMilli = endSec * 1000 + endNano / 1000000;

    timeOutMilli = (endMilli - startMilli);

    /* calculate difference between end and start */
    if (numOfPackets1 > numOfPackets2)
    {
       /* take care of wraparound of end time */
       numOfPacketsDiff = 0xFFFFFFFFU;
       numOfPacketsDiff = numOfPacketsDiff + 1 + numOfPackets2 - numOfPackets1;
    }
    else
       numOfPacketsDiff = numOfPackets2 - numOfPackets1;

    packetRate = (numOfPacketsDiff * 1000) / timeOutMilli;

    /* DMA Data transfers packet including DSA tag if enabled to the host buffer.
       The overall TO CPU Data Transfer includes Packet Bytes and  DSA.*/
    rxSdmaPacketSize = testedPacketSize + dsaSizeInBytes;
    rxSdmaBitRate = packetRate * 8 * rxSdmaPacketSize;
    packetRate_u32 = (GT_U32)packetRate;

    if (packetRate_u32 == 0)
    {
        if ((level > 3) || (quickCheck == GT_FALSE))
        {
            /* test failure */
            UTF_VERIFY_NOT_EQUAL2_STRING_MAC(packetRate_u32, 0, "Zero rate for packet size %d level %d\n", testedPacketSize, level);
        }
        else
        {
            /* Sometimes RX Handler does not get time during 10 mili sleep
              because of UART or other tasks. Check rate one more time with
              bigger sleep. */
            PRV_UTF_LOG2_MAC("Zero rate for packet size %d level %d: recheck\n", testedPacketSize, level);
            prvTgfNetIfRxSdmaPerformanceMeasure(testedPacketSize, dsaSizeInBytes, quickCheck, ++level, printResults, rxToTxEnable);
        }
    }
    else
    {
        if (rxToTxEnable)
        {
            /* AUTODOC: check rate on Tx port for Rx to Tx test */
           prvTgfCommonPortTxRateGet(PRV_TGF_SEND_PORT_RX_TO_TX_IDX_CNS, rateCheckTimeout, &portRate);
           rateDiff = prvTgfCommonDiffInPercentCalc(packetRate_u32, portRate);
           if (rateDiff > 10) /* allow 10% difference. 5% are appeared sometimes */
           {
               /* test failure */
               UTF_VERIFY_EQUAL1_STRING_MAC(0, rateDiff, "Port TX and Rx DMA rate different for packet size %d\n", testedPacketSize);
           }
        }

        if (printResults == GT_TRUE)
        {
            if (rxToTxEnable == GT_FALSE)
            {
                if (quickCheck == GT_FALSE)
                {
                    cpssOsPrintf(" %5d      %10.2f          %6.6f\n", testedPacketSize, packetRate, (rxSdmaBitRate/1000000));
                }
                else
                {
                    PRV_UTF_LOG1_MAC(" %6d", packetRate_u32);
                }
            }
            else
            {
                if (quickCheck == GT_FALSE)
                {
                    cpssOsPrintf(" %5d      %10.2f          %9d\n", testedPacketSize, packetRate, portRate);
                }
                else
                {
                    PRV_UTF_LOG1_MAC(" %6d", packetRate_u32);
                }
            }
        }
    }

    return;
}

/**
* @internal prvTgfNetIfRxSdmaPerformanceTest function
* @endinternal
*
* @brief   Rx SDMA performance test for specific packet size.
*
* @param[in] testedPacketSize         - packet size include CRC
* @param[in] dsaSizeInBytes           - DSA tag size
* @param[in] quickCheck               - GT_FALSE - use full check of rate during 1 second
*                                      GT_TRUE  - use quick check. Use minimal time for rate calculation.
*                                      test runs quick but results are not accurate
* @param[in] printResults             - print results
* @param[in] rxToTxEnable             - GT_FALSE - do not perform calculations for Rx to Tx mode
*                                      GT_FALSE - perform calculations for Rx to Tx mode
*                                       None
*/
GT_VOID prvTgfNetIfRxSdmaPerformanceTest
(
    GT_U32  testedPacketSize,
    GT_U32  dsaSizeInBytes,
    GT_BOOL quickCheck,
    GT_BOOL printResults,
    GT_BOOL rxToTxEnable
)
{
    GT_STATUS            rc;                 /* return code */
    GT_PHYSICAL_PORT_NUM portNum;            /* ingress port number */
    GT_U32               tmp;                /* tmp variable */
    GT_U32               portRate;           /* rate of port */
    GT_U32               rateCheckTimeout;   /* timeout to check rate of port */
    GT_U32               rateCheckTolerance; /* tolerance for port rate check in % */
    TGF_PACKET_STC packetInfo = {0, /* totalLen */
                sizeof(prvTgfPacketPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
                prvTgfPacketPartArray                                        /* partsArray */};

    portNum = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS];

    /* set payload size. Don't add CRC bytes. */
    prvTgfPacketPayloadPart.dataLength = (testedPacketSize - 4 /* CRC */)
                - 12 /* MAC header */ - 2 /* etherType*/
                 - (prvTgfPacketIpv4Part.headerLen * 4);

    /* set IP Total Length */
    prvTgfPacketIpv4Part.totalLen = (GT_U16)(prvTgfPacketPayloadPart.dataLength
                - 12 /* MAC header */ - 2 /* etherType*/);

    packetInfo.totalLen = (testedPacketSize - 4);

    /* setup Packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &packetInfo, 1, 0, NULL);
    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "prvTgfSetTxSetupEth: %d, %d",
                                 prvTgfDevNum, 1, 0, NULL);

    /* configure WS mode */
    rc = prvTgfTxModeSetupEth(prvTgfDevNum, portNum,
                              PRV_TGF_TX_MODE_CONTINUOUS_E,
                              PRV_TGF_TX_CONT_MODE_UNIT_WS_E,
                              0);

    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfTxModeSetupEth: %d, %d",
                                 prvTgfDevNum, portNum);

    /* AUTODOC: send continuous wirespeed traffic from port 3 */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, portNum);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, %d",
                                 prvTgfDevNum, portNum);

    if (quickCheck == GT_FALSE)
    {
        /* Full check use 1 second and 5 percent tolerance */
        rateCheckTimeout = 1000;
        rateCheckTolerance = 5;
    }
    else
    {
        /* Quick check use 10 milisecond and 30 percent tolerance */
        rateCheckTimeout = 10;
        rateCheckTolerance = 30;
    }

    /* AUTODOC: check WS on ingress port */
   prvTgfCommonPortRxRateGet(PRV_TGF_SEND_PORT_IDX_CNS, rateCheckTimeout, &portRate);

   /* check rate */
   tmp = prvTgfPortWsRateCheck(PRV_TGF_SEND_PORT_IDX_CNS, portRate, testedPacketSize);
   /* 5% is acceptable for WS advanced UT */
   if(tmp <= rateCheckTolerance)
       tmp = 0;
   UTF_VERIFY_EQUAL1_STRING_MAC(0, tmp, "prvTgfPortWsRateCheck: bad rate %d",
                                portRate);

   /* AUTODOC: measure RX SDMA rate */
   prvTgfNetIfRxSdmaPerformanceMeasure(testedPacketSize, dsaSizeInBytes, quickCheck,
                                       0 /* first level*/, printResults, rxToTxEnable);

   /* AUTODOC: stop continuous wirespeed traffic from port 3 */
   rc = prvTgfStopTransmitingEth(prvTgfDevNum, portNum);
   UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StopTransmitting: %d, %d",
                                prvTgfDevNum, portNum);

   if (quickCheck == GT_FALSE)
   {
       /* provide time to finish all previouse RX packets treatment */
       cpssOsTimerWkAfter(10);
   }
}

/**
* @internal prvTgfNetIfTxSdmaPerformanceTest function
* @endinternal
*
* @brief   Tx SDMA performance test for specific packet size.
*
* @param[in] testedPacketSize         - packet size include CRC
* @param[in] burstCount               - number of packets to use for test
* @param[in] portNum                  - port number
* @param[in] dsaSizeInBytes           - DSA tag size in bytes
*
* @param[out] timeMilliPtr             - pointer to test duration in milliseconds
*                                       None
*/
GT_VOID prvTgfNetIfTxSdmaPerformanceTest
(
    IN GT_U32                  testedPacketSize,
    IN GT_U32                  burstCount,
    IN GT_PHYSICAL_PORT_NUM    portNum,
    IN GT_U32                  dsaSizeInBytes,
    OUT double                *timeMilliPtr
)
{
    GT_STATUS                       rc;           /* return code*/
    CPSS_INTERFACE_INFO_STC         portInterface; /* port interface */
    double                          timeMilli;     /* TX duration in milliseconds */
    double                          rate;          /* TX rate in packets per second */
    GT_U32                          sendFail;      /* number of packests failed to be sent */
    double                          txSdmaBitRate;     /* TX SDMA packets bits rate in bps */
    GT_U32                          txSdmaPacketSize;  /* TX SDMA packets size */
    GT_U64                          counter;           /* Port TX Counter */
    double                          counterDouble;     /* double representation of counter */
    double                          expectedCounter;   /* expected value for counter */
    double                          expectedCounterMinus1;   /* expected value for counter minus 1 packet */
    TGF_PACKET_STC packetInfo = {0, /* totalLen */
                sizeof(prvTgfPacketPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
                prvTgfPacketPartArray                                        /* partsArray */};

    /* provide time for previous print outs */
    cpssOsTimerWkAfter(100);

    /* set payload size. Don't add CRC bytes. */
    prvTgfPacketPayloadPart.dataLength = (testedPacketSize - 4 /* CRC */)
                - 12 /* MAC header */ - 2 /* etherType*/
                 - (prvTgfPacketIpv4Part.headerLen * 4);

    /* set IP Total Length */
    prvTgfPacketIpv4Part.totalLen = (GT_U16)(prvTgfPacketPayloadPart.dataLength
                - 12 /* MAC header */ - 2 /* etherType*/);

    packetInfo.totalLen = (testedPacketSize - 4);

    /* fill portInterface struct */
    portInterface.type            = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = prvTgfDevNum;
    portInterface.devPort.portNum = portNum;

    sendFail = 0;
    timeMilli = 0;
    if (timeMilliPtr)
    {
        *timeMilliPtr = 0;
    }

    /* reset Tx counter by read it */
    rc = prvTgfPortMacCounterGet(prvTgfDevNum, portNum, CPSS_GOOD_OCTETS_SENT_E, &counter);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPortMacCounterGet %d", portNum);

    /* send packets */
    rc = tgfTrafficGeneratorPortTxEthBurstTransmit(&portInterface, &packetInfo,
                                                   burstCount,GT_FALSE, TGF_DSA_2_WORD_TYPE_E,
                                                   &timeMilli,&sendFail);
    if (rc != GT_OK)
    {
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorPortTxEthBurstTransmit %d", prvTgfDevNum);
    }
    else
    {
        /* read Tx counter */
        rc = prvTgfPortMacCounterGet(prvTgfDevNum, portNum, CPSS_GOOD_OCTETS_SENT_E, &counter);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPortMacCounterGet %d", portNum);

        if (sendFail != 0)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(0, sendFail, "BurstTransmit Send Fail %d", testedPacketSize);
        }
        else if (timeMilli == 0)
        {
            UTF_VERIFY_NOT_EQUAL1_STRING_MAC(0, (GT_U32)timeMilli, "BurstTransmit Time Fail %d", testedPacketSize);
        }
        else
        {
            /* check that port got all bytes */
            counterDouble = ((double)(counter.l[1])) * 4294967296.0 /* 0x100000000*/ + counter.l[0];
            expectedCounter = burstCount * testedPacketSize;
            expectedCounterMinus1 = expectedCounter - testedPacketSize;

            /* the number of Output Octets must be equal to number of octets
               in sent packets. The one packet less is acceptable also. Because
               it's take a time to process packet. And Jumbo packet takes more
               time then CPU get TX Complete and exist from
               tgfTrafficGeneratorPortTxEthBurstTransmit */
            if (UTF_CPSS_DXCH_XCAT3X_CHECK_MAC(prvTgfDevNum) &&
                prvCpssDxChPortRemotePortCheck(prvTgfDevNum,prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]))
            {
                /* The SDMA engine have more performance than remote port one */
            }
            else
            {
                if ((counterDouble != expectedCounter) && (counterDouble != expectedCounterMinus1))
                {
                    UTF_VERIFY_EQUAL1_STRING_MAC((GT_U32) expectedCounter, (GT_U32)counterDouble, "Wrong TX MAC Count %d", burstCount);
                    UTF_VERIFY_EQUAL1_STRING_MAC((GT_U32) expectedCounterMinus1, (GT_U32)counterDouble, "Wrong TX MAC Count %d", burstCount);
                }
            }

            if (timeMilliPtr)
            {
                *timeMilliPtr = timeMilli;
            }

            rate = burstCount * 1000 / timeMilli;

            /* DMA transfers packet including DSA tag if enabled from the host buffer.
               The overall FROM CPU Data Transfer includes Packet Bytes - CRC,
               DSA tag bytes. */
            txSdmaPacketSize = testedPacketSize - 4 + dsaSizeInBytes;
            txSdmaBitRate = rate * 8 * txSdmaPacketSize;
            cpssOsPrintf(" %5d      %10.2f          %6.6f\n", testedPacketSize, rate, (txSdmaBitRate/1000000));
        }
    }

    return;
}

/* the RX SDMA All test prints only results of Nth packet size.
  The constant defines this N. Each Nth packet size results will be printed out */
#define PRV_TGF_NET_PRINT_RATE_CNS 32

/**
* @internal prvTgfNetIfRxSdmaPerformanceAllSizeTest function
* @endinternal
*
* @brief   Rx SDMA performance test for predefined list of packet sizes
*         or range of packet sizes.
* @param[in] firstPacketSize          - first packet size, 0 - use predefined values below
* @param[in] lastPacketSize           - last packet size, used when firstPacketSize > 0
* @param[in] step                     -  of packet size values
* @param[in] quickCheck               - GT_FALSE - use full check of rate during 1 second
*                                      GT_TRUE  - use quick check. Use minimal time for rate calculation.
*                                      test runs quick but results are not accurate
* @param[in] rxToTxEnable             - GT_FALSE - do not perform calculations for Rx to Tx mode
*                                      GT_FALSE - perform calculations for Rx to Tx mode
*                                       None
*/
GT_VOID prvTgfNetIfRxSdmaPerformanceAllSizeTest
(
    GT_U32  firstPacketSize,
    GT_U32  lastPacketSize,
    GT_U32  step,
    GT_BOOL quickCheck,
    GT_BOOL rxToTxEnable
)
{
    GT_U32  packetSize;           /* size of packet on Network Port */
    GT_U32  sizeIteratNum;        /* number of iterations - size of packetSizeArr */
    GT_U32  ii;                   /* iterator */
    GT_BOOL printEnable;          /* enable flag of TX events printing */
    GT_U32  dsaSizeInBytes;       /* DSA tag size */
    CPSS_CSCD_PORT_TYPE_ENT cpuPortType; /* CPU Port DSA Type */
    GT_STATUS rc;                        /* return code */
    GT_U32  firstIter;              /* first iterator */
    GT_U32  lastIter;               /* last iterator */
    GT_U32  incrementIter;          /* increment of iterator */
    GT_U32  kk;                     /* iterator */
    GT_BOOL printResults;           /* print results */
    GT_U32  printCount;             /* number of printed results */

    sizeIteratNum = sizeof(prvTgfNetIfSdmaPerformancePacketSizeArr) / sizeof(prvTgfNetIfSdmaPerformancePacketSizeArr[0]);

    /* get CPU port DSA type */
    rc = prvTgfCscdPortTypeGet(prvTgfDevNum, CPSS_CPU_PORT_NUM_CNS,
                               CPSS_PORT_DIRECTION_TX_E, &cpuPortType);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfCscdPortTypeGet: %d, %d, %d",
                                 CPSS_CPU_PORT_NUM_CNS,
                                 CPSS_PORT_DIRECTION_TX_E,
                                 cpuPortType);

    switch (cpuPortType)
    {
        case CPSS_CSCD_PORT_DSA_MODE_1_WORD_E:
            dsaSizeInBytes = 4;
            break;
        case CPSS_CSCD_PORT_DSA_MODE_2_WORDS_E:
            dsaSizeInBytes = 8;
            break;
        case CPSS_CSCD_PORT_NETWORK_E:
            dsaSizeInBytes = 0;
            break;
        case CPSS_CSCD_PORT_DSA_MODE_3_WORDS_E:
            dsaSizeInBytes = 12;
            break;
        default: /* CPSS_CSCD_PORT_DSA_MODE_4_WORDS_E */
            dsaSizeInBytes = 16;
            break;
    }

    /*disable TX packet printing to get clean result table */
    printEnable = prvTgfTrafficPrintPacketTxEnableSet(GT_FALSE);

    printResults = GT_TRUE;
    if (quickCheck == GT_FALSE)
    {
        if (rxToTxEnable == GT_FALSE)
        {
            cpssOsPrintf("RX SDMA performance results\n");
            cpssOsPrintf("RX SDMA Data Bits Rate includes packet's bytes\n");
            cpssOsPrintf("and %d DSA bytes.\n", dsaSizeInBytes);
            cpssOsPrintf("Packet Size RX SDMA Packet Rate RX SDMA Data Bits Rate\n");
            cpssOsPrintf("   Bytes    Packets Per Second  Mega Bits Per Second\n");
        }
        else
        {
            cpssOsPrintf("RX to TX SDMA performance results\n");
            cpssOsPrintf("Ingress port %d, From CPU Tx Target port %d\n", prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], prvTgfPortsArray[PRV_TGF_SEND_PORT_RX_TO_TX_IDX_CNS]);
            cpssOsPrintf("Packet Size RX SDMA Packet Rate TX Port Packet Rate\n");
            cpssOsPrintf("   Bytes    Packets Per Second  Packets Per Second\n");
        }
    }
    else
    {
        if (rxToTxEnable == GT_FALSE)
        {
            PRV_UTF_LOG0_MAC("RX SDMA Wire Speed check for range of packet sizes:\n");
        }
        else
        {
            PRV_UTF_LOG0_MAC("RX To TX SDMA Wire Speed check for range of packet sizes:\n");
        }
        PRV_UTF_LOG1_MAC("start of range, bytes: %d\n", firstPacketSize);
        PRV_UTF_LOG1_MAC("end of range,   bytes: %d\n", lastPacketSize);
        PRV_UTF_LOG1_MAC("step,           bytes: %d\n", step);
        PRV_UTF_LOG1_MAC("Print each %dth packet size\n", PRV_TGF_NET_PRINT_RATE_CNS);
    }

    if (firstPacketSize == 0)
    {
        firstIter = 0;
        lastIter  = sizeIteratNum;
        incrementIter = 1;
    }
    else
    {
        firstIter = firstPacketSize;
        lastIter  = lastPacketSize + 1;
        incrementIter = step;
    }

    kk = printCount = 0;
    for (ii = firstIter; ii < lastIter; ii += incrementIter, kk++)
    {
        if (firstPacketSize != 0)
        {
            packetSize = ii;
        }
        else
        {
            packetSize = prvTgfNetIfSdmaPerformancePacketSizeArr[ii];
        }

        if (quickCheck == GT_TRUE)
        {
            /* enable print of results only for specific packet sizes */
            printResults = GT_FALSE;

            if (printCount == 5)
            {
                /* move to new string each 5 prints */
                PRV_UTF_LOG0_MAC("\n");
                printCount = 0;
            }
        }

        if (UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(prvTgfDevNum))
        {
            if(UTF_CPSS_SIP5_0_ONLY_CHECK_MAC(prvTgfDevNum))
            {
                PRV_CPSS_RX_DESC_LIST_STC       *rxDescList;
                GT_U32                           buffSize;

                /* get buffer size. All buffers have same size.
                   Use first descriptor for Get Packet. */
                rxDescList  = PRV_CPSS_PP_MAC(prvTgfDevNum)->intCtrl.rxDescList;
                buffSize = RX_DESC_GET_BUFF_SIZE_FIELD_MAC(rxDescList->next2Receive->rxDesc);

                /* bobcat2 A0 does not support such packet sizes */
                if (((packetSize >= 249) && (packetSize <= 264)) ||
                    ((packetSize >= (249 + buffSize)) && (packetSize <= (264 + buffSize))))
                {
                    if (quickCheck == GT_TRUE)
                    {
                        if ((kk % PRV_TGF_NET_PRINT_RATE_CNS) == 0)
                        {
                            /* notify skipped sizes */
                            PRV_UTF_LOG1_MAC("%6d:  skip ", packetSize);
                            printCount++;
                        }
                    }
                    continue;
                }
            }

            if (UTF_CPSS_DXCH_XCAT3X_CHECK_MAC(prvTgfDevNum) && 
                prvCpssDxChPortRemotePortCheck(prvTgfDevNum,prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]) &&
                (packetSize > (10240 - 4)))
            {
                /* remote ports support 10240 - Single DSA size */
                continue;
            }
        }
        else
        {
            /* SIP4 device support FROM CPU packets with size <= 10240.
              8 bytes is added by DSA tag. The maximal size for application
              is 10232.*/
            if (packetSize > 10232)
            {
                continue;
            }
        }

        if (quickCheck == GT_TRUE)
        {
            if ((kk % PRV_TGF_NET_PRINT_RATE_CNS) == 0)
            {
                PRV_UTF_LOG1_MAC("%6d:", packetSize);
                printCount++;
                printResults = GT_TRUE;
            }
        }

        prvTgfNetIfRxSdmaPerformanceTest(packetSize, dsaSizeInBytes, quickCheck, printResults, rxToTxEnable);
    }

    if (quickCheck == GT_TRUE)
    {
        /* provide time to finish all previouse RX packets treatment */
        cpssOsTimerWkAfter(10);
    }

    /* restore TX Packet printing*/
    prvTgfTrafficPrintPacketTxEnableSet(printEnable);
}

/**
* @internal prvTgfNetIfTxSdmaPerformanceListTest function
* @endinternal
*
* @brief   Tx SDMA performance test for list of packet sizes.
*/
GT_VOID prvTgfNetIfTxSdmaPerformanceListTest
(
    GT_VOID
)
{
    GT_U32      packetSize;     /* packet size */
    GT_U32      sizeIteratNum;  /* number of iterations */
    GT_U32      ii;             /* iterator */
    GT_BOOL     printEnable;    /* enable Tx packet print flag */
    GT_STATUS   rc;             /* return code */
    GT_U32      dsaSizeInBytes; /* DSA tag size in bytes */
    double      timeMilli;      /* TX duration in milliseconds */
    GT_U32      burstSize;      /* number of packets used for measurement */

    sizeIteratNum = sizeof(prvTgfNetIfSdmaPerformancePacketSizeArr) / sizeof(prvTgfNetIfSdmaPerformancePacketSizeArr[0]);

    /* AUTODOC: disable TX packet printing to get clean result table */
    printEnable = prvTgfTrafficPrintPacketTxEnableSet(GT_FALSE);

    /* AUTODOC: disable packet trace */
    rc = tgfTrafficTracePacketByteSet(GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(rc, GT_OK, "Error in tgfTrafficTracePacketByteSet %d", GT_FALSE);

    dsaSizeInBytes = UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(prvTgfDevNum) ? 16 : 8;
    cpssOsPrintf("TX SDMA performance results\n");
    cpssOsPrintf("TX SDMA Data Bits Rate includes packet's bytes minus 4 bytes CRC\n");
    cpssOsPrintf("and plus %d DSA bytes.\n", dsaSizeInBytes);
    cpssOsPrintf("Packet Size TX SDMA Packet Rate TX SDMA Data Bits Rate\n");
    cpssOsPrintf("   Bytes    Packets Per Second  Mega Bits Per Second\n");
    burstSize = 200000;
    for (ii = 0; ii < sizeIteratNum; ii++)
    {
        packetSize = prvTgfNetIfSdmaPerformancePacketSizeArr[ii];
        if (!UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(prvTgfDevNum))
        {
            /* SIP4 device support FROM CPU packets with size <= 10240.
              8 bytes is added by DSA tag. The maximal size for application
              is 10232.*/
            if (packetSize > 10232)
            {
                continue;
            }
        }

        if (UTF_CPSS_DXCH_XCAT3X_CHECK_MAC(prvTgfDevNum) && 
            prvCpssDxChPortRemotePortCheck(prvTgfDevNum,prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]) &&
            (packetSize > (10240 - 4)))
        {
            /* remote ports support 10240 - Single DSA size */
            continue;
        }

        /* AUTODOC: Send burst of packets and measure performance. */
        prvTgfNetIfTxSdmaPerformanceTest(packetSize, burstSize, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                         dsaSizeInBytes, &timeMilli);
        if ((timeMilli > 2000) && (packetSize < 10232))
        {
            /* previous test lasts too much time. Reduce burst size. */
            burstSize /= 2;
        }
    }

    /* AUTODOC: enable packet trace */
    rc = tgfTrafficTracePacketByteSet(GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(rc, GT_OK, "Error in tgfTrafficTracePacketByteSet %d", GT_TRUE);

    /* AUTODOC: restore TX Packet printing*/
    prvTgfTrafficPrintPacketTxEnableSet(printEnable);
}


void testCpuDram(GT_U32 numOfLoops, GT_U32 useDma)
{
    PRV_CPSS_TX_DESC_STC *txDesc;

    GT_U32 startSec, endSec;     /* start and end time seconds part */
    GT_U32 startNano, endNano;   /* start and end time nanoseconds part */
    double start, end; /* start and end time in milliseconds */
    double timeOut;         /* timeout in milliseconds */
    double rate;
    PRV_CPSS_TX_DESC_STC  localDescr;
    GT_U32 loopIndex = numOfLoops;

    if (useDma)
    {
        txDesc = cpssOsCacheDmaMalloc(sizeof(PRV_CPSS_TX_DESC_STC));
    }
    else
    {
        txDesc = &localDescr;
    }

    cpssOsMemSet(txDesc, 0, sizeof(PRV_CPSS_TX_DESC_STC));
    TX_DESC_SET_OWN_BIT_MAC(txDesc,TX_DESC_DMA_OWN);

    cpssOsTimeRT(&startSec,&startNano);

        /* Wait until PP sent the packet. Or HW error if while block
                   run more then loopIndex times */
        while (loopIndex && (TX_DESC_GET_OWN_BIT_MAC(devNum,txDesc) == TX_DESC_DMA_OWN))
        {
            loopIndex--;
        }

    /* get end time */
    cpssOsTimeRT(&endSec,&endNano);

    start = ((double)startSec) + ((double)startNano) / 1000000000;
    end = ((double)endSec) + ((double)endNano) / 1000000000;
    timeOut = end - start;
    rate = ((double)numOfLoops) / timeOut;
    cpssOsPrintf("CPU Performance: loops %d, time %f, rate %f\n", numOfLoops,timeOut,rate);
    cpssOsPrintf("CPU Performance: Use DMA %d loop index %d\n", useDma, loopIndex);

}

typedef struct
{
    GT_CHAR_PTR namePtr;
    GT_U32 perTc;
    GT_U32  baseAddr;
    GT_U32  tcOffset;
}DMA_REG_INFO_STC;

void prvTgfRxSdmaDump(GT_U8 devNum, GT_U32 tc)
{

    DMA_REG_INFO_STC regArr[] =
        {{"Status and Command<n>", 1,  0x00002600, 0x10},
         {"Buffer Pointer LW<n>", 1,  0x00002604, 0x10},
         {"Next Descriptor Pointer <n>", 1, 0x00002608, 0x10},
         {"Receive SDMA Current Descriptor Pointer<n>", 1, 0x0000260C, 0x10},
         {"Receive SDMA Queue Command", 0, 0x00002680, 0},
         {"SDMA Configuration", 0, 0x00002800, 0},
         {"Receive SDMA Status", 0, 0x0000281C, 0},
         {"Receive SDMA<n> Packet Count", 1, 0x00002820, 4},
         {"Receive SDMA<n> Byte Count", 1, 0x00002840, 4},
         {"Receive SDMA Resource Error Count_And_Mode_<n>", 1, 0x00002860, 4},
         {"Receive SDMA Interrupt Cause1", 0, 0x00002890, 0}
         };
    GT_U32 numOfItems;
    GT_U32 regAddr;
    GT_U32 regData;
    GT_U32 ii;
    PRV_CPSS_RX_DESC_LIST_STC       *rxDescList;
    PRV_CPSS_SW_RX_DESC_STC*         swDescrPtr;
    GT_U32                         *descrAddr;

    numOfItems = sizeof(regArr) / sizeof(regArr[0]);

    cpssOsPrintf("RX SDMA registers for device %d and queue %d\n", devNum, tc);
    /* dump registers */
    for (ii = 0; ii < numOfItems; ii++)
    {
        if (regArr[ii].perTc)
        {
            regAddr = regArr[ii].baseAddr + tc * regArr[ii].tcOffset;
            if (regAddr == 0x00002870)
            {
                regAddr +=8;
            }
        }
        else
        {
            regAddr = regArr[ii].baseAddr;
        }
        prvCpssDrvHwPpReadRegister(devNum, regAddr, &regData);
        cpssOsPrintf("%s\n", regArr[ii].namePtr);
        cpssOsPrintf("address 0x%X value 0x%08X\n", regAddr, regData);
    }

    rxDescList  = PRV_CPSS_PP_MAC(devNum)->intCtrl.rxDescList;
    swDescrPtr = rxDescList[tc].swRxDescBlock;
    numOfItems = rxDescList[tc].maxDescNum;
    cpssOsPrintf("RX SDMA %d descriptors starting from 0x%08X\n", numOfItems, swDescrPtr[0].rxDesc);
    for (ii = 0; ii < numOfItems; ii++)
    {
        descrAddr = (GT_U32*)swDescrPtr[ii].rxDesc;
        cpssOsPrintf(" 0x%08X 0x%08X 0x%08X 0x%08X\n", *(descrAddr), *(descrAddr+1), *(descrAddr+2), *(descrAddr+3));
    }

    return;
}

