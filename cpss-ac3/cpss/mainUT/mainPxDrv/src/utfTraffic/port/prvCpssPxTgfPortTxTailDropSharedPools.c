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
* @file prvCpssPxTgfPortTxTailDropSharedPools.c
*
* @brief "Shared Pools" enhanced UT for CPSS PX Port Tx Tail Drop APIs
*
* @version   1
********************************************************************************
*/


#include <utfTraffic/port/prvCpssPxTgfPortTxTailDropSharedPools.h>
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


/* profiles used on test */
#define  PRV_TGF_PROFILE_CNS                            CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_7_E
#define  PRV_TGF_RX_TEST_PORT_PROFILE_CNS               CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_0_E

/* frame's Drop Precedence */
#define  PRV_TGF_DROP_PRECEDENCE_CNS                    CPSS_DP_GREEN_E

/* count of transmitted frames */
#define  PRV_TGF_COUNT_OF_FRAMES_TO_TRANSMIT_CNS        300

/* traffic class for Rx port */
#define  PRV_TGF_RX_TEST_PORT_TRAFFIC_CLASS_CNS         0

/* index of port to transmit */
#define  PRV_TGF_PORT_TO_TRANSMIT_CNS                   2

/* index of port to receive */
#define  PRV_TGF_PORT_TO_RECEIVE_CNS                    0

/* Random entry index on DST Port Map Table */
#define  PRV_TGF_DST_IDX_CNS                            0x0123

/* use 8 packet types starting from ingress port number.
   it guaranty proper matching with one of 8 rules according to MAC_DA[5]  */
#define  PRV_TGF_PACKET_TYPE_BASE_CNS                   (PRV_TGF_PORT_TO_TRANSMIT_CNS)

/* Buffers/descriptors limit per Drop Precedence */
#define  PRV_TGF_DROP_PRECEDENCE_LIMIT_CNS              200

/*
 * Base shared pool buffers and descriptors limit.
 * For each Traffic Class shared pool limits will be calculated as:
 *
 *    buffLimit[tc] = PRV_TGF_SHARED_POOL_LIMIT_BASE_CNS + tc;
 *    descLimit[tc] = PRV_TGF_SHARED_POOL_LIMIT_BASE_CNS + tc;
 */
#define  PRV_TGF_SHARED_POOL_LIMIT_BASE_CNS             70

/* number of preallocated descriptors in shared pools */
#define  PRV_TGF_SHARED_POOL_PRE_ALLOCATED_DESCR_CNS    127


/*
 * Base MAC DA. "Shared Pools" test needed 8 destination addresses -
 * one MAC DA per Traffic Class. MAC DA will be calculated as:
 *
 *    cpssOsMemCpy(MAC_DA, prvTgfTailDropMacDaBase, 5);
 *    MAC_DA[5] = prvTgfTailDropMacDaBase[5] + trafficClass;
 */
static GT_U8 prvTgfTailDropMacDaBase[6] = {
    0x00, 0x01, 0x02, 0x03, 0xAA, 0x00
};

/* struct for backup configuration before test start */
static struct
{
    GT_BOOL     isDbaModeEnabled;
    GT_U32      portMaxBuffLimit;
    GT_U32      rxPortMaxBuffLimit;
    GT_U32      portMaxDescLimit;
    GT_U32      rxPortMaxDescLimit;
    GT_U32      portAlpha;
    GT_U32      rxPortAlpha;
    CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC  keyData[CPSS_TC_RANGE_CNS];
    CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC  keyMask[CPSS_TC_RANGE_CNS];
    CPSS_PX_COS_FORMAT_ENTRY_STC                cosFormatEntry[CPSS_TC_RANGE_CNS];
    CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_TC_STC    params;
    CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_TC_STC    rxPortParams;
    CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT   profileBinding;
    CPSS_PX_PORTS_BMP                           dstTargetPortsBmp;
    CPSS_PX_INGRESS_PORT_MAP_PACKET_TYPE_FORMAT_STC     dstPacketTypeFormat[CPSS_TC_RANGE_CNS];
    GT_U32                                          poolNum[CPSS_TC_RANGE_CNS];
    CPSS_PX_PORT_TX_TAIL_DROP_SHARED_DP_MODE_ENT    poolEnableMode[CPSS_TC_RANGE_CNS];
    GT_U32      sharedPoolDescLimit[CPSS_TC_RANGE_CNS];
    GT_U32      sharedPoolBuffLimit[CPSS_TC_RANGE_CNS];
} prvTgfTailDropSharedPoolsBackup;


/**
* @internal prvCpssPxTgfPortTxTailDropSharedPoolsConfigurationBackup function
* @endinternal
*
* @brief   Backup configuration for "Shared Pools" test.
*/
static GT_VOID prvCpssPxTgfPortTxTailDropSharedPoolsConfigurationBackup
(
    GT_VOID
)
{
    GT_STATUS   rc;
    GT_U32      i;
    CPSS_PX_PACKET_TYPE     packetType;
    GT_BOOL     bypassLagDesignatedBitmap;

    for (i = 0; i < CPSS_TC_RANGE_CNS; i++)
    {
        packetType = PRV_TGF_PACKET_TYPE_BASE_CNS + i;

        /* Backup packet type data/mask */
        rc = cpssPxIngressPacketTypeKeyEntryGet(prvTgfDevNum,
                packetType,
                &(prvTgfTailDropSharedPoolsBackup.keyData[i]),
                &(prvTgfTailDropSharedPoolsBackup.keyMask[i]));
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                "cpssPxIngressPacketTypeKeyEntryGet");

        /* Backup CoS format entry configuration */
        rc = cpssPxCosFormatEntryGet(prvTgfDevNum,
                packetType,
                &(prvTgfTailDropSharedPoolsBackup.cosFormatEntry[i]));
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssPxCosFormatEntryGet");

        /* Backup Packet Type Destination Format entry */
        rc = cpssPxIngressPortMapPacketTypeFormatEntryGet(prvTgfDevNum,
                CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E,
                packetType,
                &(prvTgfTailDropSharedPoolsBackup.dstPacketTypeFormat[i]));
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                "cpssPxIngressPortMapPacketTypeFormatEntryGet");
    }

    /* Backup DST Port Map Table entry */
    rc = cpssPxIngressPortMapEntryGet(prvTgfDevNum,
            CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E,
            PRV_TGF_DST_IDX_CNS,
            &(prvTgfTailDropSharedPoolsBackup.dstTargetPortsBmp), &bypassLagDesignatedBitmap);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssPxIngressPortMapEntryGet");

    /* Backup DBA mode enable state */
    rc = cpssPxPortTxTailDropDbaModeEnableGet(prvTgfDevNum,
            &(prvTgfTailDropSharedPoolsBackup.isDbaModeEnabled));
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssPxPortTxTailDropDbaModeEnableGet");

    /* Backup port to profile binding */
    rc = cpssPxPortTxTailDropProfileIdGet(prvTgfDevNum,
            prvTgfPxPortsArray[PRV_TGF_PORT_TO_RECEIVE_CNS],
            &(prvTgfTailDropSharedPoolsBackup.profileBinding));
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssPxPortTxTailDropProfileIdGet");

    /* Backup buffer limit, descriptor limit and alpha for profiles */
    rc = cpssPxPortTxTailDropProfileGet(prvTgfDevNum, PRV_TGF_PROFILE_CNS,
            &(prvTgfTailDropSharedPoolsBackup.portMaxBuffLimit),
            &(prvTgfTailDropSharedPoolsBackup.portMaxDescLimit),
            &(prvTgfTailDropSharedPoolsBackup.portAlpha));
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssPxPortTxTailDropProfileGet");

    rc = cpssPxPortTxTailDropProfileGet(prvTgfDevNum, PRV_TGF_RX_TEST_PORT_PROFILE_CNS,
            &(prvTgfTailDropSharedPoolsBackup.rxPortMaxBuffLimit),
            &(prvTgfTailDropSharedPoolsBackup.rxPortMaxDescLimit),
            &(prvTgfTailDropSharedPoolsBackup.rxPortAlpha));
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssPxPortTxTailDropProfileGet");

    for (i = 0; i < CPSS_TC_RANGE_CNS; i++)
    {
        /* Backup limits and alpha for traffic class and profile */
        rc = cpssPxPortTxTailDropProfileTcGet(prvTgfDevNum, PRV_TGF_PROFILE_CNS,
                i, /* tc */
                &(prvTgfTailDropSharedPoolsBackup.params));
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssPxPortTxTailDropProfileTcGet");

        /* Backup Shared Pool configuration */
        rc = cpssPxPortTxTailDropProfileTcSharingGet(prvTgfDevNum,
                PRV_TGF_PROFILE_CNS,
                i, /* tc */
                &(prvTgfTailDropSharedPoolsBackup.poolEnableMode[i]),
                &(prvTgfTailDropSharedPoolsBackup.poolNum[i]));
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                "cpssPxPortTxTailDropProfileTcSharingGet");

        rc = cpssPxPortTxTailDropSharedPoolLimitsGet(prvTgfDevNum,
                i, /* poolNum */
                &(prvTgfTailDropSharedPoolsBackup.sharedPoolBuffLimit[i]),
                &(prvTgfTailDropSharedPoolsBackup.sharedPoolDescLimit[i]));
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                "cpssPxPortTxTailDropSharedPoolLimitsGet");
    }

    /* Backup limits and alpha for traffic class and profile */
    rc = cpssPxPortTxTailDropProfileTcGet(prvTgfDevNum,
            PRV_TGF_RX_TEST_PORT_PROFILE_CNS,
            PRV_TGF_RX_TEST_PORT_TRAFFIC_CLASS_CNS,
            &(prvTgfTailDropSharedPoolsBackup.rxPortParams));
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssPxPortTxTailDropProfileTcGet");
}

/**
* @internal prvCpssPxTgfPortTxTailDropSharedPoolsConfigurationSetOrRestore function
* @endinternal
*
* @brief   Restore/Set configuration for "Shared Pools" test.
*
* @param[in] mode                     - configuration mode
*                                       None.
*/
static GT_VOID prvCpssPxTgfPortTxTailDropSharedPoolsConfigurationSetOrRestore
(
    IN  GT_U32          mode
)
{
    GT_STATUS   rc;
    GT_U32      i;
    GT_U32      j;
    CPSS_PX_PACKET_TYPE     packetType;

    GT_BOOL     isDbaModeEnabled;
    GT_U32      portMaxBuffLimit;
    GT_U32      rxPortMaxBuffLimit;
    GT_U32      portMaxDescLimit;
    GT_U32      rxPortMaxDescLimit;
    GT_U32      portAlpha;
    GT_U32      rxPortAlpha;
    CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC  keyData[CPSS_TC_RANGE_CNS];
    CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC  keyMask[CPSS_TC_RANGE_CNS];
    CPSS_PX_COS_FORMAT_ENTRY_STC                cosFormatEntry[CPSS_TC_RANGE_CNS];
    CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_TC_STC    params;
    CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_TC_STC    rxPortParams;
    CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT   profileBinding;
    CPSS_PX_PORTS_BMP                           dstTargetPortsBmp;
    CPSS_PX_INGRESS_PORT_MAP_PACKET_TYPE_FORMAT_STC     dstPacketTypeFormat[CPSS_TC_RANGE_CNS];
    CPSS_PX_PORT_TX_TAIL_DROP_SHARED_DP_MODE_ENT        poolEnableMode[CPSS_TC_RANGE_CNS];
    GT_U32      poolNum[CPSS_TC_RANGE_CNS];
    GT_U32      buffLimit[CPSS_TC_RANGE_CNS];
    GT_U32      descLimit[CPSS_TC_RANGE_CNS];


    /*
        for RESTORE mode
    */
    if (mode == PRV_TGF_TAIL_DROP_CONFIGURATION_MODE_RESTORE_CNS)
    {
        /* Restore packet type keyData and keyMask */
        cpssOsMemCpy(&keyData, &(prvTgfTailDropSharedPoolsBackup.keyData),
                     sizeof(keyData));

        cpssOsMemCpy(&keyMask, &(prvTgfTailDropSharedPoolsBackup.keyMask),
                     sizeof(keyMask));

        /* Restore CoS format entry */
        cpssOsMemCpy(&cosFormatEntry,
                     &(prvTgfTailDropSharedPoolsBackup.cosFormatEntry),
                     sizeof(cosFormatEntry));

        /* Restore Packet Type Destination Format entries */
        cpssOsMemCpy(&dstPacketTypeFormat,
                     &(prvTgfTailDropSharedPoolsBackup.dstPacketTypeFormat),
                     sizeof(dstPacketTypeFormat));

        /* Restore DST Port Map Table entry */
        cpssOsMemCpy(&dstTargetPortsBmp,
                     &(prvTgfTailDropSharedPoolsBackup.dstTargetPortsBmp),
                     sizeof(dstTargetPortsBmp));

        /* Restore DBA enable mode state */
        isDbaModeEnabled = prvTgfTailDropSharedPoolsBackup.isDbaModeEnabled;

        /* Restore port to profile binding */
        cpssOsMemCpy(&profileBinding,
                     &(prvTgfTailDropSharedPoolsBackup.profileBinding),
                     sizeof(profileBinding));

        /* Restore limits and alpha for traffic class and profiles */
        cpssOsMemCpy(&params, &(prvTgfTailDropSharedPoolsBackup.params),
                     sizeof(params));

        cpssOsMemCpy(&rxPortParams, &(prvTgfTailDropSharedPoolsBackup.rxPortParams),
                     sizeof(rxPortParams));

        /* Restore limits and alpha for profile */
        portMaxBuffLimit = prvTgfTailDropSharedPoolsBackup.portMaxBuffLimit;
        portMaxDescLimit = prvTgfTailDropSharedPoolsBackup.portMaxDescLimit;
        portAlpha        = prvTgfTailDropSharedPoolsBackup.portAlpha;

        rxPortMaxBuffLimit = prvTgfTailDropSharedPoolsBackup.rxPortMaxBuffLimit;
        rxPortMaxDescLimit = prvTgfTailDropSharedPoolsBackup.rxPortMaxDescLimit;
        rxPortAlpha        = prvTgfTailDropSharedPoolsBackup.rxPortAlpha;

        /* Restore Shared Pool configuration */
        cpssOsMemCpy(&poolEnableMode,
                     &(prvTgfTailDropSharedPoolsBackup.poolEnableMode),
                     sizeof(poolEnableMode));

        cpssOsMemCpy(&poolNum,
                     &(prvTgfTailDropSharedPoolsBackup.poolNum),
                     sizeof(poolNum));

        cpssOsMemCpy(&buffLimit,
                     &(prvTgfTailDropSharedPoolsBackup.sharedPoolBuffLimit),
                     sizeof(buffLimit));

        cpssOsMemCpy(&descLimit,
                     &(prvTgfTailDropSharedPoolsBackup.sharedPoolDescLimit),
                     sizeof(descLimit));
    }
    /*
        for SET mode
    */
    else
    {
        /*
            BACKUP CONFIGURATION
        */
        prvCpssPxTgfPortTxTailDropSharedPoolsConfigurationBackup();

        /* Clear structs */
        cpssOsMemSet(&keyData, 0, sizeof(keyData));
        cpssOsMemSet(&keyMask, 0, sizeof(keyMask));
        cpssOsMemSet(&cosFormatEntry, 0, sizeof(cosFormatEntry));
        cpssOsMemSet(&dstPacketTypeFormat, 0, sizeof(dstPacketTypeFormat));
        cpssOsMemSet(&params, 0, sizeof(params));
        cpssOsMemSet(&rxPortParams, 0, sizeof(rxPortParams));

        /* Disable forwarding to all ports except receive port */
        dstTargetPortsBmp = 1 << prvTgfPxPortsArray[PRV_TGF_PORT_TO_RECEIVE_CNS];

        /* Set profile binding */
        profileBinding = PRV_TGF_PROFILE_CNS;

        /* Disable DBA mode */
        isDbaModeEnabled = GT_FALSE;

        /* Configurate limits and alpha for traffic class and profile */
        params.dp0MaxBuffNum = PRV_TGF_DROP_PRECEDENCE_LIMIT_CNS;
        params.dp0MaxDescNum = PRV_TGF_DROP_PRECEDENCE_LIMIT_CNS;
        params.dp1MaxBuffNum = PRV_TGF_DROP_PRECEDENCE_LIMIT_CNS;
        params.dp1MaxDescNum = PRV_TGF_DROP_PRECEDENCE_LIMIT_CNS;
        params.dp2MaxBuffNum = PRV_TGF_DROP_PRECEDENCE_LIMIT_CNS;
        params.dp2MaxDescNum = PRV_TGF_DROP_PRECEDENCE_LIMIT_CNS;
        params.tcMaxBuffNum  = 0;
        params.tcMaxDescNum  = 0;
        params.dp0QueueAlpha = CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_0_E;
        params.dp1QueueAlpha = CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_0_E;
        params.dp2QueueAlpha = CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_0_E;

        /* For Rx port we will no use Shared Pools so we need to allocate  */
        /* more buffers than for Tail Drop Profile 7                       */
        rxPortParams.dp0MaxBuffNum = PRV_TGF_DROP_PRECEDENCE_LIMIT_CNS +
                                     PRV_TGF_SHARED_POOL_LIMIT_BASE_CNS +
                                     PRV_TGF_SHARED_POOL_PRE_ALLOCATED_DESCR_CNS +
                                     CPSS_TC_RANGE_CNS;
        rxPortParams.dp0MaxDescNum = PRV_TGF_DROP_PRECEDENCE_LIMIT_CNS +
                                     PRV_TGF_SHARED_POOL_LIMIT_BASE_CNS +
                                     PRV_TGF_SHARED_POOL_PRE_ALLOCATED_DESCR_CNS +
                                     CPSS_TC_RANGE_CNS;
        rxPortParams.dp1MaxBuffNum = PRV_TGF_DROP_PRECEDENCE_LIMIT_CNS +
                                     PRV_TGF_SHARED_POOL_LIMIT_BASE_CNS +
                                     PRV_TGF_SHARED_POOL_PRE_ALLOCATED_DESCR_CNS +
                                     CPSS_TC_RANGE_CNS;
        rxPortParams.dp1MaxDescNum = PRV_TGF_DROP_PRECEDENCE_LIMIT_CNS +
                                     PRV_TGF_SHARED_POOL_LIMIT_BASE_CNS +
                                     PRV_TGF_SHARED_POOL_PRE_ALLOCATED_DESCR_CNS +
                                     CPSS_TC_RANGE_CNS;
        rxPortParams.dp2MaxBuffNum = PRV_TGF_DROP_PRECEDENCE_LIMIT_CNS +
                                     PRV_TGF_SHARED_POOL_LIMIT_BASE_CNS +
                                     PRV_TGF_SHARED_POOL_PRE_ALLOCATED_DESCR_CNS +
                                     CPSS_TC_RANGE_CNS;
        rxPortParams.dp2MaxDescNum = PRV_TGF_DROP_PRECEDENCE_LIMIT_CNS +
                                     PRV_TGF_SHARED_POOL_LIMIT_BASE_CNS +
                                     PRV_TGF_SHARED_POOL_PRE_ALLOCATED_DESCR_CNS +
                                     CPSS_TC_RANGE_CNS;
        rxPortParams.tcMaxBuffNum  = 0;
        rxPortParams.tcMaxDescNum  = 0;
        rxPortParams.dp0QueueAlpha = CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_0_E;
        rxPortParams.dp1QueueAlpha = CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_0_E;
        rxPortParams.dp2QueueAlpha = CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_0_E;

        /* Configurate limits and alpha for profile */
        portMaxBuffLimit = PRV_TGF_COUNT_OF_FRAMES_TO_TRANSMIT_CNS;
        portMaxDescLimit = PRV_TGF_COUNT_OF_FRAMES_TO_TRANSMIT_CNS;
        portAlpha = CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_0_E;

        rxPortMaxBuffLimit = PRV_TGF_COUNT_OF_FRAMES_TO_TRANSMIT_CNS +
                             PRV_TGF_SHARED_POOL_LIMIT_BASE_CNS +
                             PRV_TGF_SHARED_POOL_PRE_ALLOCATED_DESCR_CNS +
                             CPSS_TC_RANGE_CNS;
        rxPortMaxDescLimit = PRV_TGF_COUNT_OF_FRAMES_TO_TRANSMIT_CNS +
                             PRV_TGF_SHARED_POOL_LIMIT_BASE_CNS +
                             PRV_TGF_SHARED_POOL_PRE_ALLOCATED_DESCR_CNS +
                             CPSS_TC_RANGE_CNS;
        rxPortAlpha = CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_0_E;

        for (i = 0; i < CPSS_TC_RANGE_CNS; i++)
        {
            /* Set keyData & keyMask to classify frame's packet type by MAC DA */
            for (j = 0; j < 6; j++)
            {
                keyData[i].macDa.arEther[j] = prvTgfTailDropMacDaBase[j];
                keyMask[i].macDa.arEther[j] = 0xFF;
            }
            keyData[i].macDa.arEther[5] += i;

            /* use profile as ingress port to avoid traffic from CPU port to be caught by rule */
            keyData[i].profileIndex = prvTgfPxPortsArray[PRV_TGF_PORT_TO_TRANSMIT_CNS];
            keyMask[i].profileIndex = 0x7F;

            /* Set CoS format entry */
            cosFormatEntry[i].cosMode = CPSS_PX_COS_MODE_FORMAT_ENTRY_E;
            cosFormatEntry[i].cosAttributes.trafficClass   = i;
            cosFormatEntry[i].cosAttributes.dropPrecedence = PRV_TGF_DROP_PRECEDENCE_CNS;

            /* Set Packet Type Destination Format entry */
            dstPacketTypeFormat[i].indexMax   = BIT_12 - 1;
            dstPacketTypeFormat[i].indexConst = PRV_TGF_DST_IDX_CNS;

            /* Set Shared Pool configuration */
            poolEnableMode[i] = CPSS_PX_PORT_TX_TAIL_DROP_SHARED_DP_MODE_ALL_E;
            poolNum[i] = i;

            buffLimit[i] = PRV_TGF_SHARED_POOL_LIMIT_BASE_CNS + i +
                           PRV_TGF_SHARED_POOL_PRE_ALLOCATED_DESCR_CNS;
            descLimit[i] = PRV_TGF_SHARED_POOL_LIMIT_BASE_CNS + i +
                           PRV_TGF_SHARED_POOL_PRE_ALLOCATED_DESCR_CNS;
        }
    }

    /*
        Set/Restore configuration
    */
    for (i = 0; i < CPSS_TC_RANGE_CNS; i++)
    {
        packetType = PRV_TGF_PACKET_TYPE_BASE_CNS + i;

        /* Set/Restore packet type entry data & mask */
        rc = cpssPxIngressPacketTypeKeyEntrySet(prvTgfDevNum, packetType,
                &(keyData[i]), &(keyMask[i]));
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssPxIngressPacketTypeKeyEntrySet");

        /* Set/Restore from backup CoS format entry */
        rc = cpssPxCosFormatEntrySet(prvTgfDevNum, packetType, &(cosFormatEntry[i]));
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssPxCosFormatEntryGet");

        /* Set/Restore Packet Type Destination Format entries */
        rc = cpssPxIngressPortMapPacketTypeFormatEntrySet(prvTgfDevNum,
                CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E,
                packetType, &(dstPacketTypeFormat[i]));
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                "cpssPxIngressPortMapPacketTypeFormatEntryGet");
    }

    /* Set/Restore DST Port Map Table entry for target bitmap */
    rc = cpssPxIngressPortMapEntrySet(prvTgfDevNum,
            CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E,
            PRV_TGF_DST_IDX_CNS, dstTargetPortsBmp, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssPxIngressPortMapEntrySet");

    /* Set/Restore DBA Mode enable state */
    rc = cpssPxPortTxTailDropDbaModeEnableSet(prvTgfDevNum, isDbaModeEnabled);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssPxPortTxTailDropDbaModeEnableSet");

    /* Set/Restore port to profile binding */
    rc = cpssPxPortTxTailDropProfileIdSet(prvTgfDevNum,
            prvTgfPxPortsArray[PRV_TGF_PORT_TO_RECEIVE_CNS], profileBinding);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssPxPortTxTailDropProfileIdSet");

    /* Set/Restore buffer limit, descriptor limit and alpha for profiles */
    rc = cpssPxPortTxTailDropProfileSet(prvTgfDevNum, PRV_TGF_PROFILE_CNS,
            portMaxBuffLimit, portMaxDescLimit, portAlpha);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssPxPortTxTailDropProfileSet");

    rc = cpssPxPortTxTailDropProfileSet(prvTgfDevNum,
            PRV_TGF_RX_TEST_PORT_PROFILE_CNS, rxPortMaxBuffLimit,
            rxPortMaxDescLimit, rxPortAlpha);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssPxPortTxTailDropProfileSet");

    for (i = 0; i < CPSS_TC_RANGE_CNS; i++)
    {
        /* Set/Restore limits and alpha for traffic class and profile */
        rc = cpssPxPortTxTailDropProfileTcSet(prvTgfDevNum, PRV_TGF_PROFILE_CNS,
                i, /* tc */
                &params);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssPxPortTxTailDropProfileTcGet");

        /* Set/Restore Shared Pool configuration */
        rc = cpssPxPortTxTailDropProfileTcSharingSet(prvTgfDevNum,
                PRV_TGF_PROFILE_CNS,
                i, /* tc */
                poolEnableMode[i], poolNum[i]);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                "cpssPxPortTxTailDropProfileTcSharingGet");

        rc = cpssPxPortTxTailDropSharedPoolLimitsSet(prvTgfDevNum,
                i, /* poolNum */
                buffLimit[i], descLimit[i]);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                "cpssPxPortTxTailDropSharedPoolLimitsGet");
    }

    /* Set/Restore limits and alpha for traffic class and profile */
    rc = cpssPxPortTxTailDropProfileTcSet(prvTgfDevNum,
            PRV_TGF_RX_TEST_PORT_PROFILE_CNS,
            PRV_TGF_RX_TEST_PORT_TRAFFIC_CLASS_CNS, &rxPortParams);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssPxPortTxTailDropProfileTcGet");
}

/**
* @internal prvCpssPxTgfPortTxTailDropSharedPoolsConfigurationRestore function
* @endinternal
*
* @brief   Restore backuped configuration for "Shared Pools" test.
*/
GT_VOID prvCpssPxTgfPortTxTailDropSharedPoolsConfigurationRestore
(
    GT_VOID
)
{
    prvCpssPxTgfPortTxTailDropSharedPoolsConfigurationSetOrRestore(
            PRV_TGF_TAIL_DROP_CONFIGURATION_MODE_RESTORE_CNS);
}

/**
* @internal prvCpssPxTgfPortTxTailDropSharedPoolsConfigurationSet function
* @endinternal
*
* @brief   Set configuration for "Shared Pools" test:
*         1. Go over all Traffic Classes (0..7).
*         1.1. Configure Packet Type entry (2..9) - ingress frames will be
*         classified by MAC DA.
*         Note: SRC Port Map Table entry for egress port (1) by default
*         configured to disable forwarding for frames with
*         Packet Type 1 (same Packet Type as number of port).
*         So I use Packet Type entries > 1.
*         1.2. Configure CoS format entry - set Traffic Class (0..7) of ingress
*         frames for current Packet Type (2..9).
*         1.3. Set index of Destination Port Map Table Entry for Packet Type.
*         2. Set entry of Destination Port Map Table - disable forwarding to all
*         ports except receive port (1).
*         3. Disable DBA mode.
*         4. Bind egress port (1) to Tail Drop Profile 7.
*         5. Set port limits for Tail Drop Profile 7 - 300 buffers/descriptors.
*         6. Go over all Traffic Classes (0..7).
*         6.1. Set buffers/descriptors limits per Tail Drop Profile (7) and
*         Traffic Class (0..7): 200 buffers/descriptors for DP0/DP1/DP2.
*         6.2. Bind Traffic Class (0..7) to Shared Pool (0..7).
*         6.3. Set buffers/descriptors limit per Shared Pool: 70 + (0..7).
*/
GT_VOID prvCpssPxTgfPortTxTailDropSharedPoolsConfigurationSet
(
    GT_VOID
)
{
    prvCpssPxTgfPortTxTailDropSharedPoolsConfigurationSetOrRestore(
            PRV_TGF_TAIL_DROP_CONFIGURATION_MODE_SET_CNS);
}

/**
* @internal prvCpssPxTgfPortTxTailDropSharedPoolsTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic for "Shared Pools" test.
*         1. Go over all Traffic Class (0..7).
*         1.1. Set MAC DA for transmitted frames
*         (00:01:02:03:AA:00 .. 00:01:02:03:AA:07)
*         1.2. Clear MAC counters for receive port.
*         1.3. Block TxQ (0..7) for receive port (1).
*         1.4. Transmit 300 frames to send port (0).
*         1.5. Release TxQ (0..7) for receive port (1).
*         1.6. Read MAC counters for receive port (1).
*         Expected value for Tx counters: 270 + (0..7) frames.
*/
GT_VOID prvCpssPxTgfPortTxTailDropSharedPoolsTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS   rc;
    GT_U32      i;
    GT_U8       frame[256];
    GT_U32      frameLength;
    GT_U8       macDa[6];
    GT_U32      expectedCounter;
    GT_PHYSICAL_PORT_NUM    rcvPortNum;
    GT_PHYSICAL_PORT_NUM    sendPortNum;
    TGF_PACKET_STC          ingressPacketInfo;
    TGF_PACKET_PART_STC     ingressFullPart[1];
    TGF_PACKET_PAYLOAD_STC  ingressPayloadPart;
    CPSS_PX_TGF_PORT_MAC_COUNTERS_STC   portCounters;


    /* set transmit and receive ports */
    sendPortNum = prvTgfPxPortsArray[PRV_TGF_PORT_TO_TRANSMIT_CNS];
    rcvPortNum  = prvTgfPxPortsArray[PRV_TGF_PORT_TO_RECEIVE_CNS];

    /* Copy MAC DA from base value */
    cpssOsMemCpy(&macDa, &prvTgfTailDropMacDaBase, sizeof(macDa));

    /* go over all Traffic Classes */
    for (i = 0; i < CPSS_TC_RANGE_CNS; i++)
    {
        macDa[5] = prvTgfTailDropMacDaBase[5] + i;

        expectedCounter = PRV_TGF_DROP_PRECEDENCE_LIMIT_CNS +
                          (PRV_TGF_SHARED_POOL_LIMIT_BASE_CNS + i);

        /* clear MAC counters for receive port */
        rc = cpssPxTgfEthCountersReset(prvTgfDevNum, rcvPortNum);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssPxTgfEthCountersReset");

        /* block TxQ[i] for receive port */
        rc = cpssPxPortTxDebugQueueTxEnableSet(prvTgfDevNum, rcvPortNum,
                i, /* Traffic Class */
                GT_FALSE);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                "cpssPxPortTxDebugQueueTxEnableSet");

        /* create frame */
        rc = cpssPxTgfEthSimpleFrameWithRandomPayloadCreate(
                macDa,  NULL,                   /* MAC DA, MAC SA  */
                GT_FALSE, 0, 0, 0,  0x5555,     /* VLAN, EtherType */
                64,                             /* payload length  */
                frame, &frameLength);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                "cpssPxTgfEthSimpleFrameWithRandomPayloadCreate");

        /* transmit frames to send port */
        ingressPacketInfo.totalLen   = TGF_PACKET_AUTO_CALC_LENGTH_CNS;
        ingressPacketInfo.numOfParts = 1;
        ingressPacketInfo.partsArray = &ingressFullPart[0];

        ingressFullPart[0].type    = TGF_PACKET_PART_PAYLOAD_E;
        ingressFullPart[0].partPtr = &ingressPayloadPart;

        ingressPayloadPart.dataLength = frameLength;
        ingressPayloadPart.dataPtr    = frame;

        rc = tgfTrafficGeneratorPxTransmitPacketAndCheckResult(prvTgfDevNum,
                sendPortNum, &ingressPacketInfo,
                PRV_TGF_COUNT_OF_FRAMES_TO_TRANSMIT_CNS, 0, NULL);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                "tgfTrafficGeneratorPxTransmitPacketAndCheckResult");

        /* read counters */
        rc = cpssPxTgfEthCountersRead(prvTgfDevNum, rcvPortNum, &portCounters);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssPxTgfEthCountersRead");

        /* check counters - nothing should be sent because queue is closed */
        UTF_VERIFY_EQUAL1_STRING_MAC(0, portCounters.goodPktsSent,
                "Got other counter value than expected on port %d\n", rcvPortNum);

        /* release TxQ[i] for receive port */
        rc = cpssPxPortTxDebugQueueTxEnableSet(prvTgfDevNum, rcvPortNum,
                i, /* Traffic Class */
                GT_TRUE);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssPxPortTxDebugQueueTxEnableSet");

        /* wait */
        cpssOsTimerWkAfter(10);

        /* read counters */
        rc = cpssPxTgfEthCountersRead(prvTgfDevNum, rcvPortNum, &portCounters);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssPxTgfEthCountersRead");

        if (i == 0)
        {
            /* the TC0 for shared pool may be damaged by previous tests runs in DBA mode. */
            if (!((expectedCounter == portCounters.goodPktsSent) ||
                (PRV_TGF_DROP_PRECEDENCE_LIMIT_CNS == portCounters.goodPktsSent)))
            {
                /* check counters */
                UTF_VERIFY_EQUAL1_STRING_MAC(expectedCounter, portCounters.goodPktsSent,
                        "Got other counter value than expected on port %d\n", rcvPortNum);

            }
        }
        else
        {
            /* check counters */
            UTF_VERIFY_EQUAL1_STRING_MAC(expectedCounter, portCounters.goodPktsSent,
                    "Got other counter value than expected on port %d\n", rcvPortNum);
        }
    }
}

