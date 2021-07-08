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
* @file prvCpssPxTgfPortTxTailDropVariousPorts.c
*
* @brief "Various Ports" enhanced UT for CPSS PX Port Tx Tail Drop APIs
*
* @version   1
********************************************************************************
*/


#include <utfTraffic/port/prvCpssPxTgfPortTxTailDropVariousPorts.h>
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

/* Packet type used for configuring CoS format entry
   Packet type is source port number. appDemo configure it. */
#define  PRV_TGF_PACKET_TYPE_CNS                    (prvTgfPxPortsArray[PRV_TGF_PORT_TO_TRANSMIT_CNS])

/* Random entry index on DST Port Map Table */
#define  PRV_TGF_DST_IDX_CNS                        0x0123


/* MAC DA used on tests */
static GT_U8 prvTgfTailDropMacDa[6] = {
    0x00, 0x01, 0x02, 0x03, 0xAA, 0x01
};

/* index of ports to receive */
static GT_U32 prvTgfTailDropVariousPortsRcvPortIndex[PRV_TGF_COUNT_OF_TRANSMITTING_CNS] = {
    1, 2, 3
};

/* limits per Drop precedence */
static GT_U32 prvTgfTailDropVariousPortsDpLimits[PRV_TGF_COUNT_OF_TRANSMITTING_CNS] = {
    1000,   /* DP0 (green) buffers/descriptors limit */
    500,    /* DP1 (yellow) buffers/descriptors limit */
    200     /* DP2 (red) buffers/descriptors limit */
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
    CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT
                            profileBinding[PRV_TGF_COUNT_OF_TRANSMITTING_CNS];
    CPSS_PX_PORTS_BMP                                   targetPortsBmp;
    CPSS_PX_INGRESS_PORT_MAP_PACKET_TYPE_FORMAT_STC     packetTypeFormat;
} prvTgfTailDropVariousPortsBackup;


/**
* @internal prvCpssPxTgfPortTxTailDropVariousPortsDpSet function
* @endinternal
*
* @brief   Set Drop Precedence for ingress frames per Packet Type
*/
static GT_STATUS prvCpssPxTgfPortTxTailDropVariousPortsDpSet
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
* @internal prvCpssPxTgfPortTxTailDropVariousPortsTargetPortSet function
* @endinternal
*
* @brief   Disable forwarding to all ports except port <portNum>.
*
* @param[in] portNum                  - physical port number
*                                       None.
*/
static GT_STATUS prvCpssPxTgfPortTxTailDropVariousPortsTargetPortSet
(
    GT_PHYSICAL_PORT_NUM    portNum
)
{
    CPSS_PX_PORTS_BMP   targetPortsBmp;

    targetPortsBmp = 1 << portNum;

    /* set DST Port Map Table entry for target bitmap */
    return cpssPxIngressPortMapEntrySet(prvTgfDevNum,
            CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E,
            PRV_TGF_DST_IDX_CNS, targetPortsBmp, GT_FALSE);
}

/**
* @internal prvCpssPxTgfPortTxTailDropVariousPortsConfigurationBackup function
* @endinternal
*
* @brief   Backup configuration for "Various Ports" test.
*/
static GT_VOID prvCpssPxTgfPortTxTailDropVariousPortsConfigurationBackup
(
    GT_VOID
)
{
    GT_STATUS   rc;
    GT_U32      i;
    GT_PHYSICAL_PORT_NUM    portNum;
    GT_BOOL     bypassLagDesignatedBitmap;

    /* Backup packet type data/mask */
    rc = cpssPxIngressPacketTypeKeyEntryGet(prvTgfDevNum, PRV_TGF_PACKET_TYPE_CNS,
            &(prvTgfTailDropVariousPortsBackup.keyData),
            &(prvTgfTailDropVariousPortsBackup.keyMask));
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssPxIngressPacketTypeKeyEntryGet");

    /* Backup CoS format entry configuration */
    rc = cpssPxCosFormatEntryGet(prvTgfDevNum, PRV_TGF_PACKET_TYPE_CNS,
            &(prvTgfTailDropVariousPortsBackup.cosFormatEntry));
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssPxCosFormatEntryGet");

    /* Backup Packet Type Destination Format entry */
    rc = cpssPxIngressPortMapPacketTypeFormatEntryGet(prvTgfDevNum,
            CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E,
            PRV_TGF_PACKET_TYPE_CNS,
            &(prvTgfTailDropVariousPortsBackup.packetTypeFormat));
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
            "cpssPxIngressPortMapPacketTypeFormatEntryGet");

    /* Backup DST Port Map Table entry */
    rc = cpssPxIngressPortMapEntryGet(prvTgfDevNum,
            CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E,
            PRV_TGF_DST_IDX_CNS,
            &(prvTgfTailDropVariousPortsBackup.targetPortsBmp), 
            &bypassLagDesignatedBitmap);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssPxIngressPortMapEntryGet");

    for (i = 0; i < PRV_TGF_COUNT_OF_TRANSMITTING_CNS; i++)
    {
        portNum = prvTgfPxPortsArray[prvTgfTailDropVariousPortsRcvPortIndex[i]];

        /* Backup port to profile binding */
        rc = cpssPxPortTxTailDropProfileIdGet(prvTgfDevNum, portNum,
                &(prvTgfTailDropVariousPortsBackup.profileBinding[i]));
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssPxPortTxTailDropProfileIdGet");
    }

    /* Backup DBA Mode enable state */
    rc = cpssPxPortTxTailDropDbaModeEnableGet(prvTgfDevNum,
            &(prvTgfTailDropVariousPortsBackup.isDbaModeEnabled));
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssPxPortTxTailDropDbaModeEnableGet");

    /* Backup buffer limit, descriptor limit and alpha for profile */
    rc = cpssPxPortTxTailDropProfileGet(prvTgfDevNum, PRV_TGF_PROFILE_CNS,
            &(prvTgfTailDropVariousPortsBackup.portMaxBuffLimit),
            &(prvTgfTailDropVariousPortsBackup.portMaxDescLimit),
            &(prvTgfTailDropVariousPortsBackup.portAlpha));
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssPxPortTxTailDropProfileGet");

    /* Backup limits and alpha for traffic class and profile */
    rc = cpssPxPortTxTailDropProfileTcGet(prvTgfDevNum, PRV_TGF_PROFILE_CNS,
            PRV_TGF_TRAFFIC_CLASS_CNS, &(prvTgfTailDropVariousPortsBackup.params));
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssPxPortTxTailDropProfileTcGet");
}

/**
* @internal prvCpssPxTgfPortTxTailDropVariousPortsConfigurationSetOrRestore function
* @endinternal
*
* @brief   Restore/Set configuration for "Various Ports" test.
*
* @param[in] mode                     - configuration mode
*                                       None.
*/
static GT_VOID prvCpssPxTgfPortTxTailDropVariousPortsConfigurationSetOrRestore
(
    IN  GT_U32          mode
)
{
    GT_STATUS               rc;
    GT_U32                  i;
    GT_PHYSICAL_PORT_NUM    portNum;

    GT_BOOL     isDbaModeEnabled;
    GT_U32      portMaxBuffLimit;
    GT_U32      portMaxDescLimit;
    GT_U32      portAlpha;
    CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC  keyData;
    CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC  keyMask;
    CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_TC_STC    params;
    CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT
                                profileBinding[PRV_TGF_COUNT_OF_TRANSMITTING_CNS];
    CPSS_PX_INGRESS_PORT_MAP_PACKET_TYPE_FORMAT_STC     packetTypeFormat;


    /*
        for RESTORE mode
    */
    if (mode == PRV_TGF_TAIL_DROP_CONFIGURATION_MODE_RESTORE_CNS)
    {
        /* Restore from backup packet type keyData and keyMask */
        cpssOsMemCpy(&keyData, &(prvTgfTailDropVariousPortsBackup.keyData),
                     sizeof(keyData));
        cpssOsMemCpy(&keyMask, &(prvTgfTailDropVariousPortsBackup.keyMask),
                     sizeof(keyMask));

        /* Restore from backup Packet Type Destination Format entry */
        cpssOsMemCpy(&packetTypeFormat,
                     &(prvTgfTailDropVariousPortsBackup.packetTypeFormat),
                     sizeof(packetTypeFormat));

        /* Restore from backup port to profile binding */
        cpssOsMemCpy(&profileBinding,
                     &(prvTgfTailDropVariousPortsBackup.profileBinding),
                     sizeof(profileBinding));

        /* Restore from backup DBA Mode enable state */
        isDbaModeEnabled = prvTgfTailDropVariousPortsBackup.isDbaModeEnabled;

        /* Restore from backup limits and alpha for traffic class and profile */
        cpssOsMemCpy(&params, &(prvTgfTailDropVariousPortsBackup.params),
                     sizeof(params));

        /* Restore from backup limits and alpha for profile */
        portMaxBuffLimit = prvTgfTailDropVariousPortsBackup.portMaxBuffLimit;
        portMaxDescLimit = prvTgfTailDropVariousPortsBackup.portMaxDescLimit;
        portAlpha        = prvTgfTailDropVariousPortsBackup.portAlpha;

        /* Restore from backup DST Port Map Table entry */
        rc = cpssPxIngressPortMapEntrySet(prvTgfDevNum,
                CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E,
                PRV_TGF_DST_IDX_CNS,
                prvTgfTailDropVariousPortsBackup.targetPortsBmp, GT_FALSE);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssPxIngressPortMapEntryGet");

        /* Restore from backup CoS format entry */
        rc = cpssPxCosFormatEntrySet(prvTgfDevNum, PRV_TGF_PACKET_TYPE_CNS,
                &(prvTgfTailDropVariousPortsBackup.cosFormatEntry));
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
        prvCpssPxTgfPortTxTailDropVariousPortsConfigurationBackup();

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

        for (i = 0; i < PRV_TGF_COUNT_OF_TRANSMITTING_CNS; i++)
        {
            /* Set profile binding */
            profileBinding[i] = PRV_TGF_PROFILE_CNS;
        }

        /* Set DBA Mode enable state */
        isDbaModeEnabled = GT_FALSE;

        /* Set Tail Drop profile configuration */
        params.dp0MaxBuffNum = prvTgfTailDropVariousPortsDpLimits[0];
        params.dp0MaxDescNum = prvTgfTailDropVariousPortsDpLimits[0];
        params.dp1MaxBuffNum = prvTgfTailDropVariousPortsDpLimits[1];
        params.dp1MaxDescNum = prvTgfTailDropVariousPortsDpLimits[1];
        params.dp2MaxBuffNum = prvTgfTailDropVariousPortsDpLimits[2];
        params.dp2MaxDescNum = prvTgfTailDropVariousPortsDpLimits[2];
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
            PRV_TGF_PACKET_TYPE_CNS,
            &packetTypeFormat);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
            "cpssPxIngressPortMapPacketTypeFormatEntryGet");

    for (i = 0; i < PRV_TGF_COUNT_OF_TRANSMITTING_CNS; i++)
    {
        portNum = prvTgfPxPortsArray[prvTgfTailDropVariousPortsRcvPortIndex[i]];

        /* Set/Restore port to profile binding */
        rc = cpssPxPortTxTailDropProfileIdSet(prvTgfDevNum, portNum, profileBinding[i]);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssPxPortTxTailDropProfileIdSet");
    }

    /* Set/Restore DBA Mode enable state */
    rc = cpssPxPortTxTailDropDbaModeEnableSet(prvTgfDevNum, isDbaModeEnabled);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssPxPortTxTailDropDbaModeEnableSet");

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
* @internal prvCpssPxTgfPortTxTailDropVariousPortsConfigurationRestore function
* @endinternal
*
* @brief   Restore backuped configuration for "Various Ports" test.
*/
GT_VOID prvCpssPxTgfPortTxTailDropVariousPortsConfigurationRestore
(
    GT_VOID
)
{
    prvCpssPxTgfPortTxTailDropVariousPortsConfigurationSetOrRestore(
            PRV_TGF_TAIL_DROP_CONFIGURATION_MODE_RESTORE_CNS);
}

/**
* @internal prvCpssPxTgfPortTxTailDropVariousPortsConfigurationSet function
* @endinternal
*
* @brief   Set configuration for "Various Ports" test.
*         1. Configure Packet Type entry (0) - all ingress frames with MAC DA
*         00:01:02:03:AA:01 will be classified as Packet Type 0.
*         2. Set index of Destination Port Map Table for Packet Type 0.
*         3. Bind egress ports (1/2/3) to Tail Drop Profile 7.
*         4. Disable DBA mode.
*         5. Set buffers/descriptors limits per Tail Drop Profile (7) and
*         Traffic Class (7):
*         - for Drop Precedence 0 (green) - 1000 buffers/descriptors;
*         - for Drop Precedence 1 (yellow) - 500 buffers/descriptors;
*         - for Drop Precedence 2 (red)  - 200 buffers/descriptors;
*         6. Set port limits for Tail Drop Profile 7 - 1700 buffers/descriptors;
*/
GT_VOID prvCpssPxTgfPortTxTailDropVariousPortsConfigurationSet
(
    GT_VOID
)
{
    prvCpssPxTgfPortTxTailDropVariousPortsConfigurationSetOrRestore(
            PRV_TGF_TAIL_DROP_CONFIGURATION_MODE_SET_CNS);
}

/**
* @internal prvCpssPxTgfPortTxTailDropVariousPortsTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic for "Various ports" test:
*         1. Set Drop Precedence (green/yellow/red) and Traffic Class (7) of
*         ingress frames for Packet Type (0).
*         2. Disable forwarding to all ports except receive port (1/2/3).
*         3. Block TxQ (7) for receive port (1/2/3).
*         4. Transmit 1500 frames (with MAC DA 00:01:02:03:AA:01) to port 0.
*         5. Release TxQ (7) for receive port (1/2/3).
*         6. Check MAC counters - expected Tx counter on port 1/2/3 is
*         1000/500/200 frames.
*/
GT_VOID prvCpssPxTgfPortTxTailDropVariousPortsTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS   rc;
    GT_U32      i;
    GT_U8       frame[256];
    GT_U32      frameLength;
    GT_U32      expectedCounterValue;
    CPSS_DP_LEVEL_ENT       dp;
    GT_PHYSICAL_PORT_NUM    rcvPortNum;
    GT_PHYSICAL_PORT_NUM    sendPortNum;
    CPSS_PX_TGF_PORT_MAC_COUNTERS_STC   portCounters;
    TGF_PACKET_STC              ingressPacketInfo;
    TGF_PACKET_PART_STC         ingressFullPart[1];
    TGF_PACKET_PAYLOAD_STC      ingressPayloadPart;

    /* set transmit port */
    sendPortNum = prvTgfPxPortsArray[PRV_TGF_PORT_TO_TRANSMIT_CNS];

    /* create frame */
    rc = cpssPxTgfEthSimpleFrameWithRandomPayloadCreate(
            prvTgfTailDropMacDa,  NULL,         /* MAC DA, MAC SA  */
            GT_FALSE, 0, 0, 0,  0x5555,         /* VLAN, EtherType */
            64,                                 /* payload length  */
            frame, &frameLength);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
            "cpssPxTgfEthSimpleFrameWithRandomPayloadCreate");

    for (i = 0; i < PRV_TGF_COUNT_OF_TRANSMITTING_CNS; i++)
    {
        dp = (CPSS_DP_LEVEL_ENT) i;
        rcvPortNum = prvTgfPxPortsArray[prvTgfTailDropVariousPortsRcvPortIndex[i]];
        expectedCounterValue = prvTgfTailDropVariousPortsDpLimits[i];

        /* clear MAC counters for receive port */
        rc = cpssPxTgfEthCountersReset(prvTgfDevNum, rcvPortNum);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssPxTgfEthCountersReset");

        /* set Drop Precedence for frames that we will transmit */
        rc = prvCpssPxTgfPortTxTailDropVariousPortsDpSet(dp);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                "prvCpssPxTgfPortTxTailDropVariousPortsDpSet");

        /* set forwarding to just 1 port */
        rc = prvCpssPxTgfPortTxTailDropVariousPortsTargetPortSet(rcvPortNum);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                "prvCpssPxTgfPortTxTailDropVariousPortsTargetPortSet");

        /* block TxQ 7 for receive port */
        rc = cpssPxPortTxDebugQueueTxEnableSet(prvTgfDevNum, rcvPortNum,
                PRV_TGF_TRAFFIC_CLASS_CNS, GT_FALSE);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssPxPortTxDebugQueueTxEnableSet");

        /* transmit frames to send port */
        ingressPacketInfo.totalLen = TGF_PACKET_AUTO_CALC_LENGTH_CNS;
        ingressPacketInfo.numOfParts = 1;
        ingressPacketInfo.partsArray = &ingressFullPart[0];

        ingressFullPart[0].type = TGF_PACKET_PART_PAYLOAD_E;
        ingressFullPart[0].partPtr = &ingressPayloadPart;

        ingressPayloadPart.dataLength = frameLength;
        ingressPayloadPart.dataPtr    = frame;
        rc = tgfTrafficGeneratorPxTransmitPacketAndCheckResult(prvTgfDevNum,sendPortNum,&ingressPacketInfo,
                                                               (PRV_TGF_COUNT_OF_FRAMES_TO_TRANSMIT_CNS - i),0,NULL);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorPxTransmitPacketAndCheckResult");

        /* wait */
        cpssOsTimerWkAfter(10);

        /* release TxQ 7 for receive port */
        rc = cpssPxPortTxDebugQueueTxEnableSet(prvTgfDevNum, rcvPortNum,
                PRV_TGF_TRAFFIC_CLASS_CNS, GT_TRUE);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssPxPortTxDebugQueueTxEnableSet");

        /* wait */
        cpssOsTimerWkAfter(10);

        /* read counters */
        rc = cpssPxTgfEthCountersRead(prvTgfDevNum, rcvPortNum, &portCounters);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssPxTgfEthCountersRead");

        /* check counters */
        UTF_VERIFY_EQUAL3_STRING_MAC(expectedCounterValue, portCounters.goodPktsSent,
                "Got other counter value than expected on port %d:\n"
                "   Tx counter:  %d\n"
                "   Tx expected: %d\n",
                rcvPortNum, portCounters.goodPktsSent, expectedCounterValue);
    }
}

