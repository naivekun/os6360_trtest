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
* @file prvCpssPxTgfPortTxTailDropOnePortSharing.c
*
* @brief "One Port Sharing" enhanced UT for CPSS PX Port Tx Tail Drop APIs
*
* @version   1
********************************************************************************
*/


#include <utfTraffic/port/prvCpssPxTgfPortTxTailDropOnePortSharing.h>
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
#define  PRV_TGF_PROFILE_CNS                        CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_6_E
#define  PRV_TGF_RX_TEST_PORT_PROFILE_CNS           CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_0_E

/* traffic class */
#define  PRV_TGF_TRAFFIC_CLASS_CNS                  4
#define  PRV_TGF_RX_TEST_PORT_TRAFFIC_CLASS_CNS     0

/* count of transmitted frames */
#define  PRV_TGF_COUNT_OF_FRAMES_TO_TRANSMIT_CNS    400

/* index of port to transmit */
#define  PRV_TGF_PORT_TO_TRANSMIT_CNS               0

/* index of port to receive */
#define  PRV_TGF_PORT_TO_RECEIVE_CNS                1

/* Packet type used for configuring CoS format entry */
#define  PRV_TGF_PACKET_TYPE_CNS                    (prvTgfPxPortsArray[PRV_TGF_PORT_TO_TRANSMIT_CNS])

/* Random entry index on DST Port Map Table */
#define  PRV_TGF_DST_IDX_CNS                        0x0123

/* Shared pool number */
#define  PRV_TGF_SHARED_POOL_NUM_CNS                2

/* Shared pool buffers and descriptors limit */
#define  PRV_TGF_SHARED_POOL_LIMIT_CNS              (70)

/* number of preallocated descriptors in shared pools */
#define  PRV_TGF_SHARED_POOL_PRE_ALLOCATED_DESCR_CNS           (127)


/* MAC DA used on tests */
static GT_U8 prvTgfTailDropMacDa[6] = {
    0x00, 0x01, 0x02, 0x03, 0xAA, 0x01
};

/* limits per Drop precedence */
static GT_U32 prvTgfTailDropOnePortSharingDpLimits[] = {
    300,    /* DP0 (green)  buffers/descriptors limit */
    200,    /* DP1 (yellow) buffers/descriptors limit */
    100     /* DP2 (red)    buffers/descriptors limit */
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
    GT_U32                                          poolNum;
    CPSS_PX_PORT_TX_TAIL_DROP_SHARED_DP_MODE_ENT    poolEnableMode;
    GT_U32      sharedPoolDescLimit;
    GT_U32      sharedPoolBuffLimit;
} prvTgfTailDropOnePortSharingBackup;


/**
* @internal prvCpssPxTgfPortTxTailDropOnePortSharingDpSet function
* @endinternal
*
* @brief   Set Drop Precedence for ingress frames per Packet Type
*/
static GT_STATUS prvCpssPxTgfPortTxTailDropOnePortSharingDpSet
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
* @internal prvCpssPxTgfPortTxTailDropOnePortSharingSharedDpModeSet function
* @endinternal
*
* @brief   Set Shared Drop Precedence mode for test Tail Drop Profile and
*         Traffic Class.
*/
static GT_STATUS prvCpssPxTgfPortTxTailDropOnePortSharingSharedDpModeSet
(
    CPSS_PX_PORT_TX_TAIL_DROP_SHARED_DP_MODE_ENT    mode
)
{
    return cpssPxPortTxTailDropProfileTcSharingSet(prvTgfDevNum,
            PRV_TGF_PROFILE_CNS, PRV_TGF_TRAFFIC_CLASS_CNS,
            mode, PRV_TGF_SHARED_POOL_NUM_CNS);
}

/**
* @internal prvCpssPxTgfPortTxTailDropOnePortSharingConfigurationBackup function
* @endinternal
*
* @brief   Backup configuration for "One Port Sharing" test.
*/
static GT_VOID prvCpssPxTgfPortTxTailDropOnePortSharingConfigurationBackup
(
    GT_VOID
)
{
    GT_STATUS   rc;
    GT_BOOL     bypassLagDesignatedBitmap;

    /* Backup packet type data/mask */
    rc = cpssPxIngressPacketTypeKeyEntryGet(prvTgfDevNum, PRV_TGF_PACKET_TYPE_CNS,
            &(prvTgfTailDropOnePortSharingBackup.keyData),
            &(prvTgfTailDropOnePortSharingBackup.keyMask));
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssPxIngressPacketTypeKeyEntryGet");

    /* Backup CoS format entry configuration */
    rc = cpssPxCosFormatEntryGet(prvTgfDevNum, PRV_TGF_PACKET_TYPE_CNS,
            &(prvTgfTailDropOnePortSharingBackup.cosFormatEntry));
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssPxCosFormatEntryGet");

    /* Backup Packet Type Destination Format entry */
    rc = cpssPxIngressPortMapPacketTypeFormatEntryGet(prvTgfDevNum,
            CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E,
            PRV_TGF_PACKET_TYPE_CNS,
            &(prvTgfTailDropOnePortSharingBackup.packetTypeFormat));
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
            "cpssPxIngressPortMapPacketTypeFormatEntryGet");

    /* Backup DST Port Map Table entry */
    rc = cpssPxIngressPortMapEntryGet(prvTgfDevNum,
            CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E,
            PRV_TGF_DST_IDX_CNS,
            &(prvTgfTailDropOnePortSharingBackup.targetPortsBmp), &bypassLagDesignatedBitmap);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssPxIngressPortMapEntryGet");

    /* Backup DBA Mode enable state */
    rc = cpssPxPortTxTailDropDbaModeEnableGet(prvTgfDevNum,
            &(prvTgfTailDropOnePortSharingBackup.isDbaModeEnabled));
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssPxPortTxTailDropDbaModeEnableGet");

    /* Backup Tail Drop profile configuration */
    rc = cpssPxPortTxTailDropProfileIdGet(prvTgfDevNum,
            prvTgfPxPortsArray[PRV_TGF_PORT_TO_RECEIVE_CNS],
            &(prvTgfTailDropOnePortSharingBackup.profileBinding));
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssPxPortTxTailDropProfileIdGet");

    rc = cpssPxPortTxTailDropProfileGet(prvTgfDevNum, PRV_TGF_PROFILE_CNS,
            &(prvTgfTailDropOnePortSharingBackup.portMaxBuffLimit),
            &(prvTgfTailDropOnePortSharingBackup.portMaxDescLimit),
            &(prvTgfTailDropOnePortSharingBackup.portAlpha));
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssPxPortTxTailDropProfileGet");

    rc = cpssPxPortTxTailDropProfileTcGet(prvTgfDevNum, PRV_TGF_PROFILE_CNS,
            PRV_TGF_TRAFFIC_CLASS_CNS,
            &(prvTgfTailDropOnePortSharingBackup.params));
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssPxPortTxTailDropProfileTcGet");

    /* Backup Shared Pool configuration */
    rc = cpssPxPortTxTailDropProfileTcSharingGet(prvTgfDevNum,
            PRV_TGF_PROFILE_CNS, PRV_TGF_TRAFFIC_CLASS_CNS,
            &(prvTgfTailDropOnePortSharingBackup.poolEnableMode),
            &(prvTgfTailDropOnePortSharingBackup.poolNum));
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
            "cpssPxPortTxTailDropProfileTcSharingGet");

    rc = cpssPxPortTxTailDropSharedPoolLimitsGet(prvTgfDevNum,
            PRV_TGF_SHARED_POOL_NUM_CNS,
            &(prvTgfTailDropOnePortSharingBackup.sharedPoolBuffLimit),
            &(prvTgfTailDropOnePortSharingBackup.sharedPoolDescLimit));
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
            "cpssPxPortTxTailDropSharedPoolLimitsGet");
}

/**
* @internal prvCpssPxTgfPortTxTailDropOnePortSharingConfigurationSetOrRestore function
* @endinternal
*
* @brief   Restore/Set configuration for "One Port Sharing" test.
*
* @param[in] mode                     - configuration mode
*                                       None.
*/
static GT_VOID prvCpssPxTgfPortTxTailDropOnePortSharingConfigurationSetOrRestore
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
    GT_U32      descLimit;
    GT_U32      buffLimit;

    /*
        for RESTORE mode
    */
    if (mode == PRV_TGF_TAIL_DROP_CONFIGURATION_MODE_RESTORE_CNS)
    {
        /* Restore packet type keyData and keyMask */
        cpssOsMemCpy(&keyData, &(prvTgfTailDropOnePortSharingBackup.keyData),
                     sizeof(keyData));
        cpssOsMemCpy(&keyMask, &(prvTgfTailDropOnePortSharingBackup.keyMask),
                     sizeof(keyMask));

        /* Restore Packet Type Destination Format entry */
        cpssOsMemCpy(&packetTypeFormat,
                     &(prvTgfTailDropOnePortSharingBackup.packetTypeFormat),
                     sizeof(packetTypeFormat));

        /* Restore DST Port Map Table entry */
        cpssOsMemCpy(&targetPortsBmp,
                     &(prvTgfTailDropOnePortSharingBackup.targetPortsBmp),
                     sizeof(targetPortsBmp));

        /* Restore from backup DBA Mode enable state */
        isDbaModeEnabled = prvTgfTailDropOnePortSharingBackup.isDbaModeEnabled;

        /* Restore Tail Drop profile configuration */
        cpssOsMemCpy(&profileBinding,
                     &(prvTgfTailDropOnePortSharingBackup.profileBinding),
                     sizeof(profileBinding));

        cpssOsMemCpy(&params, &(prvTgfTailDropOnePortSharingBackup.params),
                     sizeof(params));

        portMaxBuffLimit = prvTgfTailDropOnePortSharingBackup.portMaxBuffLimit;
        portMaxDescLimit = prvTgfTailDropOnePortSharingBackup.portMaxDescLimit;
        portAlpha        = prvTgfTailDropOnePortSharingBackup.portAlpha;

        /* Restore Shared Pool configuration */
        buffLimit = prvTgfTailDropOnePortSharingBackup.sharedPoolBuffLimit;
        descLimit = prvTgfTailDropOnePortSharingBackup.sharedPoolDescLimit;

        /* Restore CoS format entry */
        rc = cpssPxCosFormatEntrySet(prvTgfDevNum, PRV_TGF_PACKET_TYPE_CNS,
                &(prvTgfTailDropOnePortSharingBackup.cosFormatEntry));
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssPxCosFormatEntryGet");

        /* Restore Shared Pool configuration */
        rc = cpssPxPortTxTailDropProfileTcSharingSet(prvTgfDevNum,
                PRV_TGF_PROFILE_CNS, PRV_TGF_TRAFFIC_CLASS_CNS,
                prvTgfTailDropOnePortSharingBackup.poolEnableMode,
                prvTgfTailDropOnePortSharingBackup.poolNum);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                "cpssPxPortTxTailDropProfileTcSharingSet");
    }
    /*
        for SET mode
    */
    else
    {
        /*
            BACKUP CONFIGURATION
        */
        prvCpssPxTgfPortTxTailDropOnePortSharingConfigurationBackup();

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

        /* Set Tail Drop profile configuration */
        profileBinding = PRV_TGF_PROFILE_CNS;

        params.dp0MaxBuffNum = prvTgfTailDropOnePortSharingDpLimits[0];
        params.dp0MaxDescNum = prvTgfTailDropOnePortSharingDpLimits[0];
        params.dp1MaxBuffNum = prvTgfTailDropOnePortSharingDpLimits[1];
        params.dp1MaxDescNum = prvTgfTailDropOnePortSharingDpLimits[1];
        params.dp2MaxBuffNum = prvTgfTailDropOnePortSharingDpLimits[2];
        params.dp2MaxDescNum = prvTgfTailDropOnePortSharingDpLimits[2];
        params.tcMaxBuffNum  = 0;
        params.tcMaxDescNum  = 0;
        params.dp0QueueAlpha = CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_0_E;
        params.dp1QueueAlpha = CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_0_E;
        params.dp2QueueAlpha = CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_0_E;

        portMaxBuffLimit = params.dp0MaxBuffNum + params.dp1MaxBuffNum +
                           params.dp2MaxBuffNum + PRV_TGF_SHARED_POOL_LIMIT_CNS;
        portMaxDescLimit = params.dp0MaxDescNum + params.dp1MaxDescNum +
                           params.dp2MaxDescNum + PRV_TGF_SHARED_POOL_LIMIT_CNS;
        portAlpha = CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_0_E;

        /* Set Shared Pool configuration */
        buffLimit = PRV_TGF_SHARED_POOL_LIMIT_CNS + PRV_TGF_SHARED_POOL_PRE_ALLOCATED_DESCR_CNS;
        descLimit = PRV_TGF_SHARED_POOL_LIMIT_CNS + PRV_TGF_SHARED_POOL_PRE_ALLOCATED_DESCR_CNS;
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

    /* Set/Restore Tail Drop profile configuration */
    rc = cpssPxPortTxTailDropProfileIdSet(prvTgfDevNum,
            prvTgfPxPortsArray[PRV_TGF_PORT_TO_RECEIVE_CNS], profileBinding);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssPxPortTxTailDropProfileIdSet");

    rc = cpssPxPortTxTailDropProfileSet(prvTgfDevNum, PRV_TGF_PROFILE_CNS,
            portMaxBuffLimit, portMaxDescLimit, portAlpha);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssPxPortTxTailDropProfileSet");

    rc = cpssPxPortTxTailDropProfileSet(prvTgfDevNum, PRV_TGF_RX_TEST_PORT_PROFILE_CNS,
            portMaxBuffLimit, portMaxDescLimit, portAlpha);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssPxPortTxTailDropProfileSet");

    rc = cpssPxPortTxTailDropProfileTcSet(prvTgfDevNum, PRV_TGF_PROFILE_CNS,
            PRV_TGF_TRAFFIC_CLASS_CNS, &params);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssPxPortTxTailDropProfileTcSet");

    rc = cpssPxPortTxTailDropProfileTcSet(prvTgfDevNum, PRV_TGF_RX_TEST_PORT_PROFILE_CNS,
            PRV_TGF_RX_TEST_PORT_TRAFFIC_CLASS_CNS, &params);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssPxPortTxTailDropProfileTcSet");

    /* Set/Restore Shared Pool limits */
    rc = cpssPxPortTxTailDropSharedPoolLimitsSet(prvTgfDevNum,
            PRV_TGF_SHARED_POOL_NUM_CNS, buffLimit, descLimit);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
            "cpssPxPortTxTailDropSharedPoolLimitsGet");
}

/**
* @internal prvCpssPxTgfPortTxTailDropOnePortSharingConfigurationRestore function
* @endinternal
*
* @brief   Restore backuped configuration for "One Port Sharing" test.
*/
GT_VOID prvCpssPxTgfPortTxTailDropOnePortSharingConfigurationRestore
(
    GT_VOID
)
{
    prvCpssPxTgfPortTxTailDropOnePortSharingConfigurationSetOrRestore(
            PRV_TGF_TAIL_DROP_CONFIGURATION_MODE_RESTORE_CNS);
}

/**
* @internal prvCpssPxTgfPortTxTailDropOnePortSharingConfigurationSet function
* @endinternal
*
* @brief   Set configuration for "One Port Sharing" test.
*         1. Configure Packet Type entry (0) - all ingress frames with MAC DA
*         00:01:02:03:AA:01 will be classified as Packet Type 0.
*         2. Set index of Destination Port Map Table entry for Packet Type 0.
*         3. Set entry of Destination Port Map Table - disable forwarding to all
*         ports except receive port (1).
*         4. Bind egress port (1) to Tail Drop Profile 7.
*         5. Disable DBA mode.
*         6. Set buffers/descriptors limits per Tail Drop Profile (7) and
*         Traffic Class (7):
*         - for Drop Precedence 0 (green) - 300 buffers/descriptors;
*         - for Drop Precedence 1 (yellow) - 200 buffers/descriptors;
*         - for Drop Precedence 2 (red)  - 100 buffers/descriptors;
*         7. Set port limits for Tail Drop Profile 7 - 400 buffers/descriptors.
*         8. Set limits for Shared Pool 2 - 70 buffers/descriptors.
*/
GT_VOID prvCpssPxTgfPortTxTailDropOnePortSharingConfigurationSet
(
    GT_VOID
)
{
    prvCpssPxTgfPortTxTailDropOnePortSharingConfigurationSetOrRestore(
            PRV_TGF_TAIL_DROP_CONFIGURATION_MODE_SET_CNS);
}

/**
* @internal prvCpssPxTgfPortTxTailDropOnePortSharingTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic for "One port" test:
*         1. Go over all Shared DP modes (disable/dp0/dp0-dp1/all).
*         1.1. Set Shared DP mode and Pool Number (2) for test Tail Drop Profile
*         and Traffic Class.
*         1.2. Clear MAC counter for receive port (1).
*         1.3. Go over all Drop Precedence.
*         1.3.1. Block TxQ (7) for receive port (1).
*         1.3.2. Set Drop Precedence (green/yellow/red) and Traffic Class (7) of
*         ingress frames for Packet Type (0).
*         1.3.3. Transmit 400 frames (with MAC DA 00:01:02:03:AA:01) to port 0.
*         1.3.4. Check global buffers/descriptors allocation.
*         1.3.5. Release TxQ (7) for receive port (1).
*         1.4. Check MAC counter on receive port (1).
*/
GT_VOID prvCpssPxTgfPortTxTailDropOnePortSharingTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS   rc;
    GT_U32      i;
    GT_U32      dpIndex;
    GT_U8       frame[256];
    GT_U32      frameLength;
    GT_U32      expectedResourceAlloc;
    GT_U32      expectedCounter;
    GT_U32      countOfAllocatedBuffers;
    GT_U32      countOfAllocatedDescriptors;
    GT_PHYSICAL_PORT_NUM    rcvPortNum;
    GT_PHYSICAL_PORT_NUM    sendPortNum;
    CPSS_DP_LEVEL_ENT       dp;
    CPSS_PX_PORT_TX_TAIL_DROP_SHARED_DP_MODE_ENT sharedDpMode;
    CPSS_PX_TGF_PORT_MAC_COUNTERS_STC   portCounters;
    TGF_PACKET_STC              ingressPacketInfo;
    TGF_PACKET_PART_STC         ingressFullPart[1];
    TGF_PACKET_PAYLOAD_STC      ingressPayloadPart;
    GT_SW_DEV_NUM           devNum = prvTgfDevNum;

    CPSS_PX_PORT_TX_TAIL_DROP_SHARED_DP_MODE_ENT sharedDpModeList[] = {
        CPSS_PX_PORT_TX_TAIL_DROP_SHARED_DP_MODE_DISABLE_E,
        CPSS_PX_PORT_TX_TAIL_DROP_SHARED_DP_MODE_DP0_E,
        CPSS_PX_PORT_TX_TAIL_DROP_SHARED_DP_MODE_DP0_DP1_E,
        CPSS_PX_PORT_TX_TAIL_DROP_SHARED_DP_MODE_ALL_E
    };

    CPSS_DP_LEVEL_ENT dpList[] = {
        CPSS_DP_GREEN_E,
        CPSS_DP_YELLOW_E,
        CPSS_DP_RED_E
    };


    /* set transmit and receive ports */
    sendPortNum = prvTgfPxPortsArray[PRV_TGF_PORT_TO_TRANSMIT_CNS];
    rcvPortNum  = prvTgfPxPortsArray[PRV_TGF_PORT_TO_RECEIVE_CNS];

    /* create frame */
    rc = cpssPxTgfEthSimpleFrameWithRandomPayloadCreate(
            prvTgfTailDropMacDa,  NULL,         /* MAC DA, MAC SA  */
            GT_FALSE, 0, 0, 0,  0x5555,         /* VLAN, EtherType */
            64,                                 /* payload length  */
            frame, &frameLength);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
            "cpssPxTgfEthSimpleFrameWithRandomPayloadCreate");

    /* go over Shared DP modes */
    for (i = 0; i < (sizeof(sharedDpModeList) / sizeof(sharedDpModeList[0])); i++)
    {
        sharedDpMode = sharedDpModeList[i];
        expectedCounter = 0;

        /* Set Shared DP mode */
        rc = prvCpssPxTgfPortTxTailDropOnePortSharingSharedDpModeSet(
                sharedDpMode);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                "prvCpssPxTgfPortTxTailDropOnePortSharingSharedDpModeSet");

        /* clear MAC counters for receive port */
        rc = cpssPxTgfEthCountersReset(devNum, rcvPortNum);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssPxTgfEthCountersReset");

        /* transmit traffic with different Drop Precedence */
        for (dpIndex = 0; dpIndex < (sizeof(dpList) / sizeof(dpList[0])); dpIndex++)
        {
            dp = dpList[dpIndex];

            /* print info */
            switch (sharedDpMode)
            {
                case CPSS_PX_PORT_TX_TAIL_DROP_SHARED_DP_MODE_DISABLE_E:
                    PRV_UTF_LOG0_MAC("SHARED DP MODE:  DISABLED\n");
                    break;
                case CPSS_PX_PORT_TX_TAIL_DROP_SHARED_DP_MODE_DP0_E:
                    PRV_UTF_LOG0_MAC("SHARED DP MODE:  DP0\n");
                    break;
                case CPSS_PX_PORT_TX_TAIL_DROP_SHARED_DP_MODE_DP0_DP1_E:
                    PRV_UTF_LOG0_MAC("SHARED DP MODE:  DP0-DP1\n");
                    break;
                default:
                    PRV_UTF_LOG0_MAC("SHARED DP MODE:  ALL\n");
            }

            PRV_TGF_LOG1_MAC("TRAFFIC DP:      DP%d\n", dpIndex);

            /* block TxQ 7 for receive port */
            rc = cpssPxPortTxDebugQueueTxEnableSet(devNum, rcvPortNum,
                    PRV_TGF_TRAFFIC_CLASS_CNS, GT_FALSE);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                    "cpssPxPortTxDebugQueueTxEnableSet");

            /* set Drop Precedence for frames that we will transmit */
            rc = prvCpssPxTgfPortTxTailDropOnePortSharingDpSet(dp);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                    "prvCpssPxTgfPortTxTailDropOnePortSharingDpSet");

            /* transmit frames to send port */
            ingressPacketInfo.totalLen = TGF_PACKET_AUTO_CALC_LENGTH_CNS;
            ingressPacketInfo.numOfParts = 1;
            ingressPacketInfo.partsArray = &ingressFullPart[0];

            ingressFullPart[0].type = TGF_PACKET_PART_PAYLOAD_E;
            ingressFullPart[0].partPtr = &ingressPayloadPart;

            ingressPayloadPart.dataLength = frameLength;
            ingressPayloadPart.dataPtr    = frame;
            rc = tgfTrafficGeneratorPxTransmitPacketAndCheckResult(devNum,sendPortNum,&ingressPacketInfo,
                                                                   (PRV_TGF_COUNT_OF_FRAMES_TO_TRANSMIT_CNS),0,NULL);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorPxTransmitPacketAndCheckResult");


            /* get count of allocated buffers and descriptors */
            rc = cpssPxPortTxTailDropGlobalBuffNumberGet(devNum,
                    &countOfAllocatedBuffers);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                    "cpssPxPortTxTailDropGlobalBuffNumberGet");

            rc = cpssPxPortTxTailDropGlobalDescNumberGet(devNum,
                    &countOfAllocatedDescriptors);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                    "cpssPxPortTxTailDropGlobalDescNumberGet");

            PRV_UTF_LOG2_MAC("Count of allocated buffers:      %d\n"
                             "Count of allocated descriptors:  %d\n",
                             countOfAllocatedBuffers,
                             countOfAllocatedDescriptors);

            /* Calc expected count of allocated resources */
            switch (sharedDpMode)
            {
                case CPSS_PX_PORT_TX_TAIL_DROP_SHARED_DP_MODE_ALL_E:
                    if (dp == CPSS_DP_RED_E)
                    {
                        /* use DP0 limits when ALL sharing mode */
                        expectedResourceAlloc =
                                prvTgfTailDropOnePortSharingDpLimits[0] +
                                PRV_TGF_SHARED_POOL_LIMIT_CNS;
                        break;
                    }

                case CPSS_PX_PORT_TX_TAIL_DROP_SHARED_DP_MODE_DP0_DP1_E:
                    if (dp == CPSS_DP_YELLOW_E)
                    {
                        /* use DP0 limits when DP0_DP1 sharing mode */
                        expectedResourceAlloc =
                                prvTgfTailDropOnePortSharingDpLimits[0] +
                                PRV_TGF_SHARED_POOL_LIMIT_CNS;
                        break;
                    }

                case CPSS_PX_PORT_TX_TAIL_DROP_SHARED_DP_MODE_DP0_E:
                    if (dp == CPSS_DP_GREEN_E)
                    {
                        expectedResourceAlloc =
                                prvTgfTailDropOnePortSharingDpLimits[dpIndex] +
                                PRV_TGF_SHARED_POOL_LIMIT_CNS;
                        break;
                    }

                default:
                    expectedResourceAlloc =
                            prvTgfTailDropOnePortSharingDpLimits[dpIndex];
            }

            /* verify resource allocation */
            UTF_VERIFY_EQUAL0_STRING_MAC(expectedResourceAlloc,
                    countOfAllocatedBuffers,
                    "got other count of buffers than expected");
            UTF_VERIFY_EQUAL0_STRING_MAC(expectedResourceAlloc,
                    countOfAllocatedDescriptors,
                    "got other count of descriptors than expected");

            /* release TxQ 7 for receive port */
            rc = cpssPxPortTxDebugQueueTxEnableSet(devNum, rcvPortNum,
                    PRV_TGF_TRAFFIC_CLASS_CNS, GT_TRUE);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssPxPortTxDebugQueueTxEnableSet");

            /* wait */
            cpssOsTimerWkAfter(10);

            expectedCounter += expectedResourceAlloc;

            if ((i == 0) && (dpIndex == 0))
            {
                /* Disable printing info about frame content */
                cpssPxTgfEthTransmitPacketPrintInfoEnableSet(GT_FALSE);
            }
        }

        /* read counters */
        rc = cpssPxTgfEthCountersRead(devNum, rcvPortNum, &portCounters);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssPxTgfEthCountersRead");

        /* check counters */
        UTF_VERIFY_EQUAL3_STRING_MAC(expectedCounter, portCounters.goodPktsSent,
                "Got other counter value than expected on port %d:\n"
                "   Tx counter:  %d\n"
                "   Tx expected: %d",
                rcvPortNum, portCounters.goodPktsSent, expectedCounter);
    }

    /* Enable printing info about frame content */
    cpssPxTgfEthTransmitPacketPrintInfoEnableSet(GT_TRUE);
}

