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
* @file prvCpssPxTgfPortTxTailDropOnePort.c
*
* @brief "One Port" enhanced UT for CPSS PX Port Tx Tail Drop APIs
*
* @version   1
********************************************************************************
*/


#include <utfTraffic/port/prvCpssPxTgfPortTxTailDropOnePort.h>
#include <utf/utfMain.h>
#include <extUtils/trafficEngine/tgfTrafficGenerator.h>
#include <utfTraffic/common/cpssPxTgfCommon.h>

#include <extUtils/trafficEngine/prvTgfLog.h>
#include <utf/private/prvUtfHelpers.h>
#include <cpss/common/config/private/prvCpssConfigTypes.h>
#include <cpss/px/cos/cpssPxCos.h>
#include <cpss/px/cos/private/prvCpssPxCoS.h>
#include <cpss/px/port/cpssPxPortCtrl.h>
#include <cpss/px/ingress/cpssPxIngress.h>
#include <cpss/px/port/cpssPxPortTxDebug.h>
#include <cpss/px/port/cpssPxPortTxTailDrop.h>


/* Configuration mode */
#define  PRV_TGF_TAIL_DROP_CONFIGURATION_MODE_SET_CNS       0
#define  PRV_TGF_TAIL_DROP_CONFIGURATION_MODE_RESTORE_CNS   1


/* profile used for tests */
#define  PRV_TGF_PROFILE_CNS                        CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_7_E
#define  PRV_TGF_RX_TEST_PORT_PROFILE_CNS           CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_0_E

/* traffic class */
#define  PRV_TGF_TRAFFIC_CLASS_CNS                  7
#define  PRV_TGF_RX_TEST_PORT_TRAFFIC_CLASS_CNS     0

/* count of transmitting */
#define  PRV_TGF_COUNT_OF_TRANSMITTING_CNS          3

/* count of transmitted frames */
#define  PRV_TGF_COUNT_OF_FRAMES_TO_TRANSMIT_CNS    1500

/* index of port to transmit */
#define  PRV_TGF_PORT_TO_TRANSMIT_CNS               0

/* index of port to receive */
#define  PRV_TGF_PORT_TO_RECEIVE_CNS                1

/* Packet type used for configuring CoS format entry
   Packet type is source port number. appDemo configure it. */
#define  PRV_TGF_PACKET_TYPE_CNS                    (prvTgfPxPortsArray[PRV_TGF_PORT_TO_TRANSMIT_CNS])

/* Random entry index on DST Port Map Table */
#define  PRV_TGF_DST_IDX_CNS                        0x0123


/* MAC DA used on tests */
static GT_U8 prvTgfTailDropMacDa[6] = {
    0x00, 0x01, 0x02, 0x03, 0xAA, 0x01
};

/* limits per Drop precedence */
static GT_U32 prvTgfTailDropOnePortDpLimits[PRV_TGF_COUNT_OF_TRANSMITTING_CNS] = {
    1000,   /* DP0 (green)  buffers/descriptors limit */
    500,    /* DP1 (yellow) buffers/descriptors limit */
    200     /* DP2 (red)    buffers/descriptors limit */
};

/* struct for backup configuration before test start */
static struct
{
    GT_BOOL     isDbaModeEnabled;
    GT_U32      portMaxBuffLimit;
    GT_U32      portMaxDescLimit;
    GT_U32      portAlpha;
    CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC  keyData;
    CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC  keyMask;
    CPSS_PX_COS_FORMAT_ENTRY_STC                cosFormatEntry;
    CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_TC_STC    params;
    CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT   profileBinding;
    CPSS_PX_PORTS_BMP                           targetPortsBmp;
    CPSS_PX_INGRESS_PORT_MAP_PACKET_TYPE_FORMAT_STC     packetTypeFormat;
} prvTgfTailDropOnePortBackup;


/**
* @internal prvCpssPxTgfPortTxTailDropOnePortDpSet function
* @endinternal
*
* @brief   Set Drop Precedence for ingress frames per Packet Type
*/
static GT_STATUS prvCpssPxTgfPortTxTailDropOnePortDpSet
(
    CPSS_DP_LEVEL_ENT   dp
)
{
    CPSS_PX_COS_FORMAT_ENTRY_STC    cosFormatEntry;

    cpssOsMemSet(&cosFormatEntry, 0, sizeof(cosFormatEntry));

    cosFormatEntry.cosMode = CPSS_PX_COS_MODE_FORMAT_ENTRY_E;
    cosFormatEntry.cosAttributes.trafficClass   = PRV_TGF_TRAFFIC_CLASS_CNS;
    cosFormatEntry.cosAttributes.dropPrecedence = dp;

    return cpssPxCosFormatEntrySet(prvTgfDevNum, PRV_TGF_PACKET_TYPE_CNS, &cosFormatEntry);
}

/**
* @internal prvCpssPxTgfPortTxTailDropOnePortConfigurationBackup function
* @endinternal
*
* @brief   Backup configuration for "One Port" test.
*/
static GT_VOID prvCpssPxTgfPortTxTailDropOnePortConfigurationBackup
(
    GT_VOID
)
{
    GT_STATUS   rc;
    GT_BOOL     bypassLagDesignatedBitmap;

    /* Backup packet type data/mask */
    rc = cpssPxIngressPacketTypeKeyEntryGet(prvTgfDevNum, PRV_TGF_PACKET_TYPE_CNS,
            &(prvTgfTailDropOnePortBackup.keyData),
            &(prvTgfTailDropOnePortBackup.keyMask));
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssPxIngressPacketTypeKeyEntryGet");

    /* Backup CoS format entry configuration */
    rc = cpssPxCosFormatEntryGet(prvTgfDevNum, PRV_TGF_PACKET_TYPE_CNS,
            &(prvTgfTailDropOnePortBackup.cosFormatEntry));
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssPxCosFormatEntryGet");

    /* Backup Packet Type Destination Format entry */
    rc = cpssPxIngressPortMapPacketTypeFormatEntryGet(prvTgfDevNum,
            CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E,
            PRV_TGF_PACKET_TYPE_CNS,
            &(prvTgfTailDropOnePortBackup.packetTypeFormat));
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
            "cpssPxIngressPortMapPacketTypeFormatEntryGet");

    /* Backup DST Port Map Table entry */
    rc = cpssPxIngressPortMapEntryGet(prvTgfDevNum,
            CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E,
            PRV_TGF_DST_IDX_CNS,
            &(prvTgfTailDropOnePortBackup.targetPortsBmp), &bypassLagDesignatedBitmap);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssPxIngressPortMapEntryGet");

    /* Backup DBA Mode enable state */
    rc = cpssPxPortTxTailDropDbaModeEnableGet(prvTgfDevNum,
            &(prvTgfTailDropOnePortBackup.isDbaModeEnabled));
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssPxPortTxTailDropDbaModeEnableGet");

    /* Backup port to profile binding */
    rc = cpssPxPortTxTailDropProfileIdGet(prvTgfDevNum,
            prvTgfPxPortsArray[PRV_TGF_PORT_TO_RECEIVE_CNS],
            &(prvTgfTailDropOnePortBackup.profileBinding));
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssPxPortTxTailDropProfileIdGet");

    /* Backup buffer limit, descriptor limit and alpha for profile */
    rc = cpssPxPortTxTailDropProfileGet(prvTgfDevNum, PRV_TGF_PROFILE_CNS,
            &(prvTgfTailDropOnePortBackup.portMaxBuffLimit),
            &(prvTgfTailDropOnePortBackup.portMaxDescLimit),
            &(prvTgfTailDropOnePortBackup.portAlpha));
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssPxPortTxTailDropProfileGet");

    /* Backup limits and alpha for traffic class and profile */
    rc = cpssPxPortTxTailDropProfileTcGet(prvTgfDevNum, PRV_TGF_PROFILE_CNS,
            PRV_TGF_TRAFFIC_CLASS_CNS, &(prvTgfTailDropOnePortBackup.params));
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssPxPortTxTailDropProfileTcGet");
}

/**
* @internal prvCpssPxTgfPortTxTailDropOnePortConfigurationSetOrRestore function
* @endinternal
*
* @brief   Restore/Set configuration for "One Port" test.
*
* @param[in] mode                     - configuration mode
*                                       None.
*/
static GT_VOID prvCpssPxTgfPortTxTailDropOnePortConfigurationSetOrRestore
(
    IN  GT_U32          mode
)
{
    GT_STATUS   rc;
    GT_U32      i;

    GT_BOOL     isDbaModeEnabled;
    GT_U32      portMaxBuffLimit;
    GT_U32      portMaxDescLimit;
    GT_U32      portAlpha;
    CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC  keyData;
    CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC  keyMask;
    CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_TC_STC    params;
    CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT   profileBinding;
    CPSS_PX_PORTS_BMP                           targetPortsBmp;
    CPSS_PX_INGRESS_PORT_MAP_PACKET_TYPE_FORMAT_STC     packetTypeFormat;

    /*
        for RESTORE mode
    */
    if (mode == PRV_TGF_TAIL_DROP_CONFIGURATION_MODE_RESTORE_CNS)
    {
        /* Restore from backup packet type keyData and keyMask */
        cpssOsMemCpy(&keyData, &(prvTgfTailDropOnePortBackup.keyData),
                     sizeof(keyData));
        cpssOsMemCpy(&keyMask, &(prvTgfTailDropOnePortBackup.keyMask),
                     sizeof(keyMask));

        /* Restore from backup Packet Type Destination Format entry */
        cpssOsMemCpy(&packetTypeFormat,
                     &(prvTgfTailDropOnePortBackup.packetTypeFormat),
                     sizeof(packetTypeFormat));

        /* Restore from backup DST Port Map Table entry */
        cpssOsMemCpy(&targetPortsBmp,
                     &(prvTgfTailDropOnePortBackup.targetPortsBmp),
                     sizeof(targetPortsBmp));

        /* Restore from backup DBA Mode enable state */
        isDbaModeEnabled = prvTgfTailDropOnePortBackup.isDbaModeEnabled;

        /* Restore from backup port to profile binding */
        cpssOsMemCpy(&profileBinding,
                     &(prvTgfTailDropOnePortBackup.profileBinding),
                     sizeof(profileBinding));

        /* Restore from backup limits and alpha for traffic class and profile */
        cpssOsMemCpy(&params, &(prvTgfTailDropOnePortBackup.params),
                     sizeof(params));

        /* Restore from backup limits and alpha for profile */
        portMaxBuffLimit = prvTgfTailDropOnePortBackup.portMaxBuffLimit;
        portMaxDescLimit = prvTgfTailDropOnePortBackup.portMaxDescLimit;
        portAlpha        = prvTgfTailDropOnePortBackup.portAlpha;

        /* Restore from backup CoS format entry */
        rc = cpssPxCosFormatEntrySet(prvTgfDevNum, PRV_TGF_PACKET_TYPE_CNS,
                &(prvTgfTailDropOnePortBackup.cosFormatEntry));
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssPxCosFormatEntryGet");
    }
    /*
        for SET mode
    */
    else
    {
        /*
            BACKUP CONFIGURATION
        */
        prvCpssPxTgfPortTxTailDropOnePortConfigurationBackup();

        /* Clear keyData & keyMask needed to configure packet type */
        cpssOsMemSet(&keyData, 0, sizeof(keyData));
        cpssOsMemSet(&keyMask, 0, sizeof(keyMask));

        /* use profile as ingress port */
        keyData.profileIndex = prvTgfPxPortsArray[PRV_TGF_PORT_TO_TRANSMIT_CNS];
        keyMask.profileIndex = 0x7F;

        /* Set keyData & keyMask to classify frame's packet type by MAC DA */
        for (i = 0; i < 6; i++)
        {
            keyData.macDa.arEther[i] = prvTgfTailDropMacDa[i];
            keyMask.macDa.arEther[i] = 0xFF;
        }

        /* Set Packet Type Destination Format entry */
        cpssOsMemSet(&packetTypeFormat, 0, sizeof(packetTypeFormat));

        packetTypeFormat.indexMax = BIT_12 - 1;
        packetTypeFormat.indexConst = PRV_TGF_DST_IDX_CNS;

        /* Set DBA Mode enable state */
        isDbaModeEnabled = GT_FALSE;

        /* Disable forwarding to all ports except receive port */
        targetPortsBmp = 1 << prvTgfPxPortsArray[PRV_TGF_PORT_TO_RECEIVE_CNS];

        /* Set profile binding */
        profileBinding = PRV_TGF_PROFILE_CNS;

        /* Set Tail Drop profile configuration */
        params.dp0MaxBuffNum = prvTgfTailDropOnePortDpLimits[0];
        params.dp0MaxDescNum = prvTgfTailDropOnePortDpLimits[0];
        params.dp1MaxBuffNum = prvTgfTailDropOnePortDpLimits[1];
        params.dp1MaxDescNum = prvTgfTailDropOnePortDpLimits[1];
        params.dp2MaxBuffNum = prvTgfTailDropOnePortDpLimits[2];
        params.dp2MaxDescNum = prvTgfTailDropOnePortDpLimits[2];
        params.tcMaxBuffNum  = 0;
        params.tcMaxDescNum  = 0;
        params.dp0QueueAlpha = CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_0_E;
        params.dp1QueueAlpha = CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_0_E;
        params.dp2QueueAlpha = CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_0_E;

        portMaxBuffLimit = params.dp0MaxBuffNum + params.dp1MaxBuffNum +
                           params.dp2MaxBuffNum;
        portMaxDescLimit = params.dp0MaxDescNum + params.dp1MaxDescNum +
                           params.dp2MaxDescNum;
        portAlpha = CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_0_E;
    }

    /*
        Set/Restore configuration
    */
    /* Set/Restore packet type entry data & mask */
    rc = cpssPxIngressPacketTypeKeyEntrySet(prvTgfDevNum, PRV_TGF_PACKET_TYPE_CNS,
            &keyData, &keyMask);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssPxIngressPacketTypeKeyEntrySet");

    /* Set/Restore Packet Type Destination Format entry */
    rc = cpssPxIngressPortMapPacketTypeFormatEntrySet(prvTgfDevNum,
            CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E,
            PRV_TGF_PACKET_TYPE_CNS, &packetTypeFormat);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
            "cpssPxIngressPortMapPacketTypeFormatEntryGet");

    /* Set/Restore DST Port Map Table entry for target bitmap */
    rc = cpssPxIngressPortMapEntrySet(prvTgfDevNum,
            CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E,
            PRV_TGF_DST_IDX_CNS, targetPortsBmp, GT_FALSE);

    /* Set/Restore DBA Mode enable state */
    rc = cpssPxPortTxTailDropDbaModeEnableSet(prvTgfDevNum, isDbaModeEnabled);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssPxPortTxTailDropDbaModeEnableSet");

    /* Set/Restore port to profile binding */
    rc = cpssPxPortTxTailDropProfileIdSet(prvTgfDevNum,
            prvTgfPxPortsArray[PRV_TGF_PORT_TO_RECEIVE_CNS], profileBinding);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssPxPortTxTailDropProfileIdSet");

    /* Set/Restore buffer limit, descriptor limit and alpha for profile */
    rc = cpssPxPortTxTailDropProfileSet(prvTgfDevNum, PRV_TGF_PROFILE_CNS,
            portMaxBuffLimit, portMaxDescLimit, portAlpha);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssPxPortTxTailDropProfileSet");

    rc = cpssPxPortTxTailDropProfileSet(prvTgfDevNum, PRV_TGF_RX_TEST_PORT_PROFILE_CNS,
            portMaxBuffLimit, portMaxDescLimit, portAlpha);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssPxPortTxTailDropProfileSet");

    /* Set/Restore limits and alpha for traffic class and profile */
    rc = cpssPxPortTxTailDropProfileTcSet(prvTgfDevNum, PRV_TGF_PROFILE_CNS,
            PRV_TGF_TRAFFIC_CLASS_CNS, &params);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssPxPortTxTailDropProfileTcSet");

    rc = cpssPxPortTxTailDropProfileTcSet(prvTgfDevNum, PRV_TGF_RX_TEST_PORT_PROFILE_CNS,
            PRV_TGF_RX_TEST_PORT_TRAFFIC_CLASS_CNS, &params);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssPxPortTxTailDropProfileTcSet");
    
}

/**
* @internal prvCpssPxTgfPortTxTailDropOnePortConfigurationRestore function
* @endinternal
*
* @brief   Restore backuped configuration for "One Port" test.
*/
GT_VOID prvCpssPxTgfPortTxTailDropOnePortConfigurationRestore
(
    GT_VOID
)
{
    prvCpssPxTgfPortTxTailDropOnePortConfigurationSetOrRestore(
            PRV_TGF_TAIL_DROP_CONFIGURATION_MODE_RESTORE_CNS);
}

/**
* @internal prvCpssPxTgfPortTxTailDropOnePortConfigurationSet function
* @endinternal
*
* @brief   Set configuration for "One Port" test.
*         1. Configure Packet Type entry (0) - all ingress frames with MAC DA
*         00:01:02:03:AA:01 will be classified as Packet Type 0.
*         2. Set index of Destination Port Map Table entry for Packet Type 0.
*         3. Set entry of Destination Port Map Table - disable forwarding to all
*         ports except receive port (1).
*         4. Bind egress port (1) to Tail Drop Profile 7.
*         5. Disable DBA mode.
*         6. Set buffers/descriptors limits per Tail Drop Profile (7) and
*         Traffic Class (7):
*         - for Drop Precedence 0 (green) - 1000 buffers/descriptors;
*         - for Drop Precedence 1 (yellow) - 500 buffers/descriptors;
*         - for Drop Precedence 2 (red)  - 200 buffers/descriptors;
*         7. Set port limits for Tail Drop Profile 7 - 1700 buffers/descriptors.
*/
GT_VOID prvCpssPxTgfPortTxTailDropOnePortConfigurationSet
(
    GT_VOID
)
{
    prvCpssPxTgfPortTxTailDropOnePortConfigurationSetOrRestore(
            PRV_TGF_TAIL_DROP_CONFIGURATION_MODE_SET_CNS);
}

/**
* @internal prvCpssPxTgfPortTxTailDropOnePortTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic for "One port" test:
*         1. Clear MAC counters.
*         2. Block TxQ (7) for receive port (1).
*         3. Go over all Drop Precedence (green/yellow/red).
*         3.1. Set Drop Precedence and Traffic Class (7) of ingress frames for
*         Packet Type (0).
*         3.2. Transmit 1100 frames (with MAC DA 00:01:02:03:AA:01) to port 0.
*         3.3. Check count of global allocated buffers and descriptors - expected
*         values 200 / 500 / 1000.
*         4. Release TxQ (7) for receive port (1).
*         5. Check MAC counters - expected Tx counter on port 1 is 1000 frames.
*/
GT_VOID prvCpssPxTgfPortTxTailDropOnePortTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS   rc;
    GT_U32      i;
    GT_U8       frame[256];
    GT_U32      frameLength;
    GT_U32      expectedValue;
    GT_U32      countOfAllocatedBuffers;
    GT_U32      countOfAllocatedDescriptors;
    CPSS_DP_LEVEL_ENT       dp;
    GT_PHYSICAL_PORT_NUM    rcvPortNum;
    GT_PHYSICAL_PORT_NUM    sendPortNum;
    CPSS_PX_TGF_PORT_MAC_COUNTERS_STC   portCounters;
    TGF_PACKET_STC              ingressPacketInfo;
    TGF_PACKET_PART_STC         ingressFullPart[1];
    TGF_PACKET_PAYLOAD_STC      ingressPayloadPart;
    GT_SW_DEV_NUM               devNum;

    /* set transmit and receive ports */
    sendPortNum = prvTgfPxPortsArray[PRV_TGF_PORT_TO_TRANSMIT_CNS];
    rcvPortNum  = prvTgfPxPortsArray[PRV_TGF_PORT_TO_RECEIVE_CNS];
    devNum = prvTgfDevNum;

    /* create frame */
    rc = cpssPxTgfEthSimpleFrameWithRandomPayloadCreate(
            prvTgfTailDropMacDa,  NULL,         /* MAC DA, MAC SA  */
            GT_FALSE, 0, 0, 0,  0x5555,         /* VLAN, EtherType */
            64,                                 /* payload length  */
            frame, &frameLength);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
            "cpssPxTgfEthSimpleFrameWithRandomPayloadCreate");

    /* clear MAC counters for receive port */
    rc = cpssPxTgfEthCountersReset(devNum, rcvPortNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssPxTgfEthCountersReset");

    /* block TxQ 7 for receive port */
    rc = cpssPxPortTxDebugQueueTxEnableSet(devNum, rcvPortNum,
            PRV_TGF_TRAFFIC_CLASS_CNS, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssPxPortTxDebugQueueTxEnableSet");

    expectedValue = 0;

    for (i = 0; i < PRV_TGF_COUNT_OF_TRANSMITTING_CNS; i++)
    {
        /* tail drop algorithm use the following logic per TC
         Queue Counter + P > {Queue (Q),DP } Dynamic Limit / WRED
         So start with lowest number (DP2)
        */
        dp = (CPSS_DP_LEVEL_ENT) (PRV_TGF_COUNT_OF_TRANSMITTING_CNS - 1 - i);
        expectedValue = prvTgfTailDropOnePortDpLimits[dp];

        /* set Drop Precedence for frames that we will transmit */
        rc = prvCpssPxTgfPortTxTailDropOnePortDpSet(dp);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                "prvCpssPxTgfPortTxTailDropOnePortDpSet");

        /* transmit frames to send port */
        ingressPacketInfo.totalLen = TGF_PACKET_AUTO_CALC_LENGTH_CNS;
        ingressPacketInfo.numOfParts = 1;
        ingressPacketInfo.partsArray = &ingressFullPart[0];

        ingressFullPart[0].type = TGF_PACKET_PART_PAYLOAD_E;
        ingressFullPart[0].partPtr = &ingressPayloadPart;

        ingressPayloadPart.dataLength = frameLength;
        ingressPayloadPart.dataPtr    = frame;
        rc = tgfTrafficGeneratorPxTransmitPacketAndCheckResult(devNum,sendPortNum,&ingressPacketInfo,
                                                               (PRV_TGF_COUNT_OF_FRAMES_TO_TRANSMIT_CNS - i),0,NULL);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorPxTransmitPacketAndCheckResult");

        /* get count of allocated buffers */
        rc = cpssPxPortTxTailDropGlobalBuffNumberGet(devNum,
                &countOfAllocatedBuffers);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                "cpssPxPortTxTailDropGlobalBuffNumberGet");

        /* get count of allocated descriptors */
        rc = cpssPxPortTxTailDropGlobalDescNumberGet(devNum,
                &countOfAllocatedDescriptors);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                "cpssPxPortTxTailDropGlobalDescNumberGet");

        /* print count of allocated resources */
        PRV_UTF_LOG2_MAC("Count of allocated buffers:      %d\n"
                         "Count of allocated descriptors:  %d\n",
                         countOfAllocatedBuffers, countOfAllocatedDescriptors);

        /* verify resource allocation */
        UTF_VERIFY_EQUAL0_STRING_MAC(expectedValue, countOfAllocatedBuffers,
                "got other count of buffers than expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(expectedValue, countOfAllocatedDescriptors,
                "got other count of descriptors than expected");
    }

    /* release TxQ 7 for receive port */
    rc = cpssPxPortTxDebugQueueTxEnableSet(devNum, rcvPortNum,
            PRV_TGF_TRAFFIC_CLASS_CNS, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssPxPortTxDebugQueueTxEnableSet");

    /* wait */
    cpssOsTimerWkAfter(10);

    /* read counters */
    rc = cpssPxTgfEthCountersRead(devNum, rcvPortNum, &portCounters);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssPxTgfEthCountersRead");

    /* expected counters value: dp0Limit - maximal value */
    expectedValue = prvTgfTailDropOnePortDpLimits[0];

    /* check counters */
    UTF_VERIFY_EQUAL3_STRING_MAC(expectedValue, portCounters.goodPktsSent,
            "Got other counter value than expected on port %d:\n"
            "   Tx counter:  %d\n"
            "   Tx expected: %d\n",
            rcvPortNum, portCounters.goodPktsSent, expectedValue);
}

