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
* @file prvCpssPxTgfPortTxTailDropDbaPortResources.c
*
* @brief "DBA Port Resource Allocation" enhanced UT for CPSS PX Port Tx
* Tail Drop APIs
*
* @version   1
********************************************************************************
*/

#include <utfTraffic/port/prvCpssPxTgfPortTxTailDropDbaPortResources.h>
#include <utf/utfMain.h>
#include <extUtils/trafficEngine/tgfTrafficGenerator.h>
#include <extUtils/trafficEngine/prvTgfLog.h>
#include <utfTraffic/common/cpssPxTgfCommon.h>
#include <utfTraffic/common/cpssPxTgfPortTxTailDrop.h>

#include <extUtils/trafficEngine/prvTgfLog.h>
#include <utf/private/prvUtfHelpers.h>
#include <cpss/px/cos/cpssPxCos.h>
#include <cpss/px/port/cpssPxPortCtrl.h>
#include <cpss/px/ingress/cpssPxIngress.h>
#include <cpss/px/port/cpssPxPortTxDebug.h>
#include <cpss/px/port/cpssPxPortTxTailDrop.h>


/* Configuration mode */
#define  PRV_TGF_TAIL_DROP_CONFIGURATION_MODE_SET_CNS       0
#define  PRV_TGF_TAIL_DROP_CONFIGURATION_MODE_RESTORE_CNS   1


/* profile used for tests */
#define  PRV_TGF_PROFILE_CNS                        CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_7_E
#define  PRV_TGF_DEFAULT_PROFILE_CNS                CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_0_E

/* traffic class */
#define  PRV_TGF_TRAFFIC_CLASS_CNS                  7
#define  PRV_TGF_DEFAULT_TRAFFIC_CLASS_CNS          0

/* count of transmitting */
#define  PRV_TGF_COUNT_OF_TRANSMITTING_CNS          6

/* count of transmitted frames */
#define  PRV_TGF_COUNT_OF_FRAMES_TO_TRANSMIT_CNS    1000

/* port to transmit */
#define  PRV_TGF_PORT_TO_TRANSMIT_CNS               prvTgfPxPortsArray[0]

/* port to receive */
#define  PRV_TGF_PORT_TO_RECEIVE_CNS                prvTgfPxPortsArray[1]

/* Packet type used for configuring CoS format entry        */
/* Packet type is source port number. appDemo configure it. */
#define  PRV_TGF_PACKET_TYPE_CNS                    prvTgfPxPortsArray[0]

/* Random entry index on DST Port Map Table */
#define  PRV_TGF_DST_IDX_CNS                        0x0123

/* Amount of buffers available for dynamic allocation */
#define  PRV_TGF_AMOUNT_OF_BUFFERS_FOR_DBA_CNS      1000


/* MAC DA used on tests */
static GT_U8 prvTgfTailDropMacDa[6] = {
    0x00, 0x01, 0x02, 0x03, 0xAA, 0x01
};

static GT_U32 prvTgfTailDropDbaPortDbaPortMaxBuffLimit[PRV_TGF_COUNT_OF_TRANSMITTING_CNS] = {
    600,    /* for Alpha ratio 0.0  ( 0.0% of dynamic buffers on this UT) */
    500,    /* for Alpha ratio 0.25 (20.0% of dynamic buffers on this UT) */
    400,    /* for Alpha ratio 0.5  (33.3% of dynamic buffers on this UT) */
    300,    /* for Alpha ratio 1.0  (50.0% of dynamic buffers on this UT) */
    200,    /* for Alpha ratio 2.0  (66.6% of dynamic buffers on this UT) */
    100     /* for Alpha ratio 4.0  (80.0% of dynamic buffers on this UT) */
};

/* Alpha for dynamic buffers allocation per port */
static CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT
                    prvTgfTailDropDbaPortDbaPortAlpha[PRV_TGF_COUNT_OF_TRANSMITTING_CNS] = {
    CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_0_E,
    CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_0_25_E,
    CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_0_5_E,
    CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_1_E,
    CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_2_E,
    CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_4_E,
};

/* struct for backup configuration before test start */
static struct
{
    GT_BOOL     isDbaModeEnabled;
    GT_U32      portMaxBuffLimit;
    GT_U32      portMaxDescLimit;
    GT_U32      defPortMaxBuffLimit;
    GT_U32      defPortMaxDescLimit;
    CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT     portAlpha;
    CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT     defPortAlpha;
    CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC  keyData;
    CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC  keyMask;
    CPSS_PX_COS_FORMAT_ENTRY_STC                cosFormatEntry;
    CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_TC_STC    params[CPSS_TC_RANGE_CNS];
    CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_TC_STC    defProfileParams[CPSS_TC_RANGE_CNS];
    CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT   profileBinding;
    CPSS_PX_PORTS_BMP                           targetPortsBmp;
    CPSS_PX_INGRESS_PORT_MAP_PACKET_TYPE_FORMAT_STC     packetTypeFormat;
    GT_U32      dbaAvailableBuffer;
} prvTgfTailDropDbaPortBackup;


/**
* @internal prvCpssPxTgfPortTxTailDropDbaPortAlphaOnReceivePortSet function
* @endinternal
*
* @brief   Set Tail Drop DBA configuration for receive port
*
* @param[in] numOfTransmit            - number of transmitting
*                                       None.
*/
static GT_STATUS prvCpssPxTgfPortTxTailDropDbaPortAlphaOnReceivePortSet
(
    IN  GT_U32  numOfTransmit
)
{
    return cpssPxPortTxTailDropProfileSet(prvTgfDevNum, PRV_TGF_PROFILE_CNS,
            prvTgfTailDropDbaPortDbaPortMaxBuffLimit[numOfTransmit],  /* portMaxBuffLimit */
            PRV_TGF_COUNT_OF_FRAMES_TO_TRANSMIT_CNS,                  /* portMaxDescLimit */
            prvTgfTailDropDbaPortDbaPortAlpha[numOfTransmit]);        /* portAlpha        */
}

/**
* @internal prvCpssPxTgfPortTxTailDropDbaPortExpectedAmountOfBuffers function
* @endinternal
*
* @brief   Calculate expected amount of available buffers (include dynamic
*         allocation).
* @param[in] numOfTransmit            - number of transmitting
*
* @param[out] expectedValue            - expected amount of available buffers
*                                       None.
*/
static GT_STATUS prvCpssPxTgfPortTxTailDropDbaPortExpectedAmountOfBuffers
(
    IN  GT_U32  numOfTransmit,
    OUT GT_U32  *expectedValue
)
{
    GT_U32      dynamicFactor, usedBuffers;
    GT_FLOAT32  ratio;

    /* Convert enum Alpha to float */
    PRV_CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_TO_RATIO_CONVERT_MAC(
            prvTgfTailDropDbaPortDbaPortAlpha[numOfTransmit], ratio);

    /* test uses guarantied number of buffers before DBA start to work */
    usedBuffers = prvTgfTailDropDbaPortDbaPortMaxBuffLimit[numOfTransmit];

    /*
       Dynamic factor calculated as:

                                                   Alpha
          DynamicFactor = FreeBuffers * ----------------------------
                                         1 + sumOfAllDestPortsRatio

       where sumOfAllDestPortsRatio - sum of Alphas of all destination ports.
       On this test we set Alphas for Tail Drop Profile 0 (default profile)
       to 0 - so we have Alpha0 on all ports except receive. On just one
       port (receive) we use Tail Drop Profile 7 with different Alphas (see
       variable prvTgfTailDropDbaPortDbaPortAlpha). So:

          sumOfAllDestPortsRatio = ratio    - because just one port (receive)
                                              will have non-zero Alpha
    */
    dynamicFactor = (GT_U32)((PRV_TGF_AMOUNT_OF_BUFFERS_FOR_DBA_CNS - usedBuffers) /* Free Buffers */
                    * (ratio / (1 + ratio)));              /* Alpha / (1 + sumOfAllDestPortsRatio) */

    /* Limit = Guaranteed Buffers + Dynamic Factor */
    *expectedValue = usedBuffers + dynamicFactor;

    return GT_OK;
}

/**
* @internal prvCpssPxTgfPortTxTailDropDbaPortConfigurationBackup function
* @endinternal
*
* @brief   Backup configuration for "DBA Port Resources Allocation" test.
*/
static GT_VOID prvCpssPxTgfPortTxTailDropDbaPortConfigurationBackup
(
    GT_VOID
)
{
    GT_STATUS   rc;
    GT_U32      tc;
    GT_BOOL     bypassLagDesignatedBitmap;

    /* Backup packet type data/mask */
    rc = cpssPxIngressPacketTypeKeyEntryGet(prvTgfDevNum, PRV_TGF_PACKET_TYPE_CNS,
            &(prvTgfTailDropDbaPortBackup.keyData),
            &(prvTgfTailDropDbaPortBackup.keyMask));
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssPxIngressPacketTypeKeyEntryGet");

    /* Backup CoS format entry configuration */
    rc = cpssPxCosFormatEntryGet(prvTgfDevNum, PRV_TGF_PACKET_TYPE_CNS,
            &(prvTgfTailDropDbaPortBackup.cosFormatEntry));
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssPxCosFormatEntryGet");

    /* Backup Packet Type Destination Format entry */
    rc = cpssPxIngressPortMapPacketTypeFormatEntryGet(prvTgfDevNum,
            CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E,
            PRV_TGF_PACKET_TYPE_CNS,
            &(prvTgfTailDropDbaPortBackup.packetTypeFormat));
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
            "cpssPxIngressPortMapPacketTypeFormatEntryGet");

    /* Backup DST Port Map Table entry */
    rc = cpssPxIngressPortMapEntryGet(prvTgfDevNum,
            CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E,
            PRV_TGF_DST_IDX_CNS,
            &(prvTgfTailDropDbaPortBackup.targetPortsBmp), &bypassLagDesignatedBitmap);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssPxIngressPortMapEntryGet");

    /* Backup DBA Mode enable state */
    rc = cpssPxPortTxTailDropDbaModeEnableGet(prvTgfDevNum,
            &(prvTgfTailDropDbaPortBackup.isDbaModeEnabled));
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssPxPortTxTailDropDbaModeEnableGet");

    /* Backup port to profile binding */
    rc = cpssPxPortTxTailDropProfileIdGet(prvTgfDevNum, PRV_TGF_PORT_TO_RECEIVE_CNS,
            &(prvTgfTailDropDbaPortBackup.profileBinding));
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssPxPortTxTailDropProfileIdGet");

    /* Backup buffer limit, descriptor limit and alpha for profiles */
    rc = cpssPxPortTxTailDropProfileGet(prvTgfDevNum, PRV_TGF_PROFILE_CNS,
            &(prvTgfTailDropDbaPortBackup.portMaxBuffLimit),
            &(prvTgfTailDropDbaPortBackup.portMaxDescLimit),
            &(prvTgfTailDropDbaPortBackup.portAlpha));
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssPxPortTxTailDropProfileGet");

    rc = cpssPxPortTxTailDropProfileGet(prvTgfDevNum,
            PRV_TGF_DEFAULT_PROFILE_CNS,
            &(prvTgfTailDropDbaPortBackup.defPortMaxBuffLimit),
            &(prvTgfTailDropDbaPortBackup.defPortMaxDescLimit),
            &(prvTgfTailDropDbaPortBackup.defPortAlpha));
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssPxPortTxTailDropProfileGet");

    /* Backup limits and alpha for traffic class and profile */
    for (tc = 0; tc < CPSS_TC_RANGE_CNS; tc++)
    {
        rc = cpssPxPortTxTailDropProfileTcGet(prvTgfDevNum, PRV_TGF_PROFILE_CNS,
                PRV_TGF_TRAFFIC_CLASS_CNS,
                &(prvTgfTailDropDbaPortBackup.params[tc]));
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssPxPortTxTailDropProfileTcGet");

        rc = cpssPxPortTxTailDropProfileTcGet(prvTgfDevNum,
                PRV_TGF_DEFAULT_PROFILE_CNS,
                PRV_TGF_DEFAULT_TRAFFIC_CLASS_CNS,
                &(prvTgfTailDropDbaPortBackup.defProfileParams[tc]));
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssPxPortTxTailDropProfileTcGet");
    }
    /* Backup amount of buffers available for dynamic allocation */
    rc = cpssPxPortTxTailDropDbaAvailableBuffGet(prvTgfDevNum,
            &(prvTgfTailDropDbaPortBackup.dbaAvailableBuffer));
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
            "cpssPxPortTxTailDropDbaAvailableBuffGet");
}

/**
* @internal prvCpssPxTgfPortTxTailDropDbaPortConfigurationSetOrRestore function
* @endinternal
*
* @brief   Restore/Set configuration for "DBA Port Resources Allocation" test.
*
* @param[in] mode                     - Configuration mode
*                                       None.
*/
static GT_VOID prvCpssPxTgfPortTxTailDropDbaPortConfigurationSetOrRestore
(
    IN  GT_U32      mode
)
{
    GT_STATUS   rc;
    GT_U32      i;

    GT_BOOL     isDbaModeEnabled;
    GT_U32      dbaAvailableBuffer;
    GT_U32      defPortMaxBuffLimit;
    GT_U32      defPortMaxDescLimit;
    CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT     defPortAlpha;
    CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC  keyData;
    CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC  keyMask;
    CPSS_PX_COS_FORMAT_ENTRY_STC                cosFormatEntry;
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
        cpssOsMemCpy(&keyData, &(prvTgfTailDropDbaPortBackup.keyData),
                     sizeof(keyData));
        cpssOsMemCpy(&keyMask, &(prvTgfTailDropDbaPortBackup.keyMask),
                     sizeof(keyMask));

        /* Restore from backup Packet Type Destination Format entry */
        cpssOsMemCpy(&packetTypeFormat,
                     &(prvTgfTailDropDbaPortBackup.packetTypeFormat),
                     sizeof(packetTypeFormat));

        /* Restore from backup CoS format entry */
        cpssOsMemCpy(&cosFormatEntry,
                     &(prvTgfTailDropDbaPortBackup.cosFormatEntry),
                     sizeof(cosFormatEntry));

        /* Restore from backup DST Port Map Table entry */
        cpssOsMemCpy(&targetPortsBmp,
                     &(prvTgfTailDropDbaPortBackup.targetPortsBmp),
                     sizeof(targetPortsBmp));

        /* Restore from backup DBA Mode enable state */
        isDbaModeEnabled = prvTgfTailDropDbaPortBackup.isDbaModeEnabled;

        /* Restore from backup port to profile binding */
        cpssOsMemCpy(&profileBinding,
                     &(prvTgfTailDropDbaPortBackup.profileBinding),
                     sizeof(profileBinding));

        /* Restore from backup limits and alpha for profile */
        defPortMaxBuffLimit = prvTgfTailDropDbaPortBackup.defPortMaxBuffLimit;
        defPortMaxDescLimit = prvTgfTailDropDbaPortBackup.defPortMaxDescLimit;
        defPortAlpha        = prvTgfTailDropDbaPortBackup.defPortAlpha;

        /* Restore amount of buffers available for dynamic allocation */
        dbaAvailableBuffer = prvTgfTailDropDbaPortBackup.dbaAvailableBuffer;

        /* Restore from backup Tail Drop configuration for receive port */
        rc = cpssPxPortTxTailDropProfileSet(prvTgfDevNum, PRV_TGF_PROFILE_CNS,
                prvTgfTailDropDbaPortBackup.portMaxBuffLimit,
                prvTgfTailDropDbaPortBackup.portMaxDescLimit,
                prvTgfTailDropDbaPortBackup.portAlpha);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssPxPortTxTailDropProfileSet");

        /* Restore limits and alpha for traffic class and profile */
        for (i = 0; i < CPSS_TC_RANGE_CNS; i++)
        {
            rc = cpssPxPortTxTailDropProfileTcSet(prvTgfDevNum,
                    PRV_TGF_PROFILE_CNS,
                    i,
                    &(prvTgfTailDropDbaPortBackup.params[i]));
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                    "cpssPxPortTxTailDropProfileTcSet");

            rc = cpssPxPortTxTailDropProfileTcSet(prvTgfDevNum,
                    PRV_TGF_DEFAULT_PROFILE_CNS,
                    i,
                    &(prvTgfTailDropDbaPortBackup.defProfileParams[i]));
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                    "cpssPxPortTxTailDropProfileTcSet");
        }
    }
    /*
        for SET mode
    */
    else
    {
        /*
            BACKUP CONFIGURATION
        */
        prvCpssPxTgfPortTxTailDropDbaPortConfigurationBackup();

        /* Clear keyData & keyMask needed to configure packet type */
        cpssOsMemSet(&keyData, 0, sizeof(keyData));
        cpssOsMemSet(&keyMask, 0, sizeof(keyMask));

        /* use profile as ingress port */
        keyData.profileIndex = PRV_TGF_PORT_TO_TRANSMIT_CNS;
        keyMask.profileIndex = 0x7F;

        /* Set keyData & keyMask to classify frame's packet type by MAC DA */
        for (i = 0; i < 6; i++)
        {
            keyData.macDa.arEther[i] = prvTgfTailDropMacDa[i];
            keyMask.macDa.arEther[i] = 0xFF;
        }

        /* Set Packet Type Destination Format entry */
        cpssOsMemSet(&packetTypeFormat, 0, sizeof(packetTypeFormat));

        packetTypeFormat.indexMax   = BIT_12 - 1;
        packetTypeFormat.indexConst = PRV_TGF_DST_IDX_CNS;

        /* Set CoS format entry */
        cpssOsMemSet(&cosFormatEntry, 0, sizeof(cosFormatEntry));

        cosFormatEntry.cosMode = CPSS_PX_COS_MODE_FORMAT_ENTRY_E;
        cosFormatEntry.cosAttributes.trafficClass = PRV_TGF_TRAFFIC_CLASS_CNS;

        /* Disable forwarding to all ports except receive port */
        targetPortsBmp = 1 << PRV_TGF_PORT_TO_RECEIVE_CNS;

        /* Set profile binding */
        profileBinding = PRV_TGF_PROFILE_CNS;

        /* Set DBA Mode enable state */
        isDbaModeEnabled = GT_TRUE;

        /* Set Tail Drop profiles configuration - set Alpha to 0 */
        params.dp0MaxBuffNum = PRV_TGF_COUNT_OF_FRAMES_TO_TRANSMIT_CNS;
        params.dp1MaxBuffNum = PRV_TGF_COUNT_OF_FRAMES_TO_TRANSMIT_CNS;
        params.dp2MaxBuffNum = PRV_TGF_COUNT_OF_FRAMES_TO_TRANSMIT_CNS;
        params.dp0MaxDescNum = PRV_TGF_COUNT_OF_FRAMES_TO_TRANSMIT_CNS;
        params.dp1MaxDescNum = PRV_TGF_COUNT_OF_FRAMES_TO_TRANSMIT_CNS;
        params.dp2MaxDescNum = PRV_TGF_COUNT_OF_FRAMES_TO_TRANSMIT_CNS;
        params.tcMaxBuffNum  = 0;
        params.tcMaxDescNum  = 0;
        params.dp0QueueAlpha = CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_0_E;
        params.dp1QueueAlpha = CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_0_E;
        params.dp2QueueAlpha = CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_0_E;

        /* Set Tail Drop configuration per port */
        defPortMaxBuffLimit = PRV_TGF_COUNT_OF_FRAMES_TO_TRANSMIT_CNS;
        defPortMaxDescLimit = PRV_TGF_COUNT_OF_FRAMES_TO_TRANSMIT_CNS;
        defPortAlpha        = CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_0_E;

        /* Set amount of buffers available for dynamic allocation */
        dbaAvailableBuffer = PRV_TGF_AMOUNT_OF_BUFFERS_FOR_DBA_CNS;

        /* Set Alpha0 for Tail Drop configuration for Profiles 0 and 7 */
        for (i = 0; i < CPSS_TC_RANGE_CNS; i++)
        {
            rc = cpssPxPortTxTailDropProfileTcSet(prvTgfDevNum,
                    PRV_TGF_DEFAULT_PROFILE_CNS, i, &params);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                    "cpssPxPortTxTailDropProfileTcSet");

            rc = cpssPxPortTxTailDropProfileTcSet(prvTgfDevNum,
                    PRV_TGF_PROFILE_CNS, i, &params);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                    "cpssPxPortTxTailDropProfileTcSet");
        }
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

    /* Set/Restore CoS format entry */
    rc = cpssPxCosFormatEntrySet(prvTgfDevNum, PRV_TGF_PACKET_TYPE_CNS,
            &cosFormatEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssPxCosFormatEntryGet");

    /* Set/Restore DST Port Map Table entry for target bitmap */
    rc = cpssPxIngressPortMapEntrySet(prvTgfDevNum,
            CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E,
            PRV_TGF_DST_IDX_CNS, targetPortsBmp, GT_FALSE);

    /* Set/Restore DBA Mode enable state */
    rc = cpssPxPortTxTailDropDbaModeEnableSet(prvTgfDevNum, isDbaModeEnabled);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssPxPortTxTailDropDbaModeEnableSet");

    /* Set/Restore port to profile binding */
    rc = cpssPxPortTxTailDropProfileIdSet(prvTgfDevNum, PRV_TGF_PORT_TO_RECEIVE_CNS,
            profileBinding);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssPxPortTxTailDropProfileIdSet");

    /* Set/Restore buffer limit, descriptor limit and alpha for profile */
    rc = cpssPxPortTxTailDropProfileSet(prvTgfDevNum, PRV_TGF_DEFAULT_PROFILE_CNS,
            defPortMaxBuffLimit, defPortMaxDescLimit, defPortAlpha);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssPxPortTxTailDropProfileSet");

    /* Set/Restore amount of buffers available for dynamic allocation */
    rc = cpssPxPortTxTailDropDbaAvailableBuffSet(prvTgfDevNum, dbaAvailableBuffer);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssPxPortTxTailDropDbaAvailableBuffSet");
}

/**
* @internal prvCpssPxTgfPortTxTailDropDbaPortConfigurationRestore function
* @endinternal
*
* @brief   Restore backuped configuration for "DBA Port Resources Allocation" test.
*/
GT_VOID prvCpssPxTgfPortTxTailDropDbaPortConfigurationRestore
(
    GT_VOID
)
{
    prvCpssPxTgfPortTxTailDropDbaPortConfigurationSetOrRestore(
            PRV_TGF_TAIL_DROP_CONFIGURATION_MODE_RESTORE_CNS);
}

/**
* @internal prvCpssPxTgfPortTxTailDropDbaPortConfigurationSet function
* @endinternal
*
* @brief   Set configuration for "DBA Port Resources Allocation" test:
*         1. Configure Packet Type entry (0) - all ingress frames with MAC DA
*         00:01:02:03:AA:01 will be classified as Packet Type 0.
*         2. Set index of Destination Port Map Table entry for Packet Type 0.
*         3. Set Traffic Class (7) of ingress frames for Packet Type (0).
*         4. Set entry of Destination Port Map Table - disable forwarding to all
*         ports exclude receive port (1).
*         5. Bind egress port (1) to Tail Drop Profile 7.
*         6. Enable DBA mode.
*         7. Set Alpha0 for Tail Drop (TC,DP) configuration for Profiles 0 and 7.
*         8. Set Alpha0 for Tail Drop per port configuration for Profile (7).
*         9. Set amount of available for DBA buffers to 1000.
*/
GT_VOID prvCpssPxTgfPortTxTailDropDbaPortConfigurationSet
(
    GT_VOID
)
{
    prvCpssPxTgfPortTxTailDropDbaPortConfigurationSetOrRestore(
            PRV_TGF_TAIL_DROP_CONFIGURATION_MODE_SET_CNS);
}

/**
* @internal prvCpssPxTgfPortTxTailDropDbaPortTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic for "DBA Port Resources Allocation" test:
*         1. Clear MAC counters.
*         2. Go over all Alphas.
*         2.1. Set portAlpha for receive port.
*         2.1. Get expected amount of available buffers (expectedValue).
*         2.3. Block TxQ (7) for receive port (1).
*         2.4. Transmit 1000 frames (with MAC DA 00:01:02:03:AA:01) to port 0.
*         2.5. Release TxQ (7) for receive port (1).
*         2.6. Read MAC counters. Expected that amount of sent frames will be
*         in range [ expectedValue-10 .. expectedValue+10 ].
*/
GT_VOID prvCpssPxTgfPortTxTailDropDbaPortTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS   rc;
    GT_U32      i;
    GT_U8       frame[256];
    GT_U32      frameLength;
    GT_U32      expectedValue;
    CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT  alpha;
    CPSS_PX_TGF_PORT_MAC_COUNTERS_STC        portCounters;
    TGF_PACKET_STC              ingressPacketInfo;
    TGF_PACKET_PART_STC         ingressFullPart[1];
    TGF_PACKET_PAYLOAD_STC      ingressPayloadPart;

    /* create frame */
    rc = cpssPxTgfEthSimpleFrameWithRandomPayloadCreate(
            prvTgfTailDropMacDa,  NULL,         /* MAC DA, MAC SA  */
            GT_FALSE, 0, 0, 0,  0x5555,         /* VLAN, EtherType */
            64,                                 /* payload length  */
            frame, &frameLength);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
            "cpssPxTgfEthSimpleFrameWithRandomPayloadCreate");

    /* clear MAC counters for receive port */
    rc = cpssPxTgfEthCountersReset(prvTgfDevNum, PRV_TGF_PORT_TO_RECEIVE_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssPxTgfEthCountersReset");

    expectedValue = 0;

    for (i = 0; i < PRV_TGF_COUNT_OF_TRANSMITTING_CNS; i++)
    {
        alpha = prvTgfTailDropDbaPortDbaPortAlpha[i];

        PRV_TGF_LOG1_MAC("ALPHA:   %s\n",
                (alpha == CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_0_E)    ? "0.0"  :
                (alpha == CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_0_25_E) ? "0.25" :
                (alpha == CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_0_5_E)  ? "0.5"  :
                (alpha == CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_1_E)    ? "1.0"  :
                (alpha == CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_2_E)    ? "2.0"  :
                (alpha == CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_4_E)    ? "4.0"  :
                "BAD PARAMETER");

        /* Get expected amount of available buffers */
        rc = prvCpssPxTgfPortTxTailDropDbaPortExpectedAmountOfBuffers(i,
                &expectedValue);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                "prvCpssPxTgfPortTxTailDropDbaPortExpectedAmountOfBuffers");

        /* set portAlpha for receive port */
        rc = prvCpssPxTgfPortTxTailDropDbaPortAlphaOnReceivePortSet(i);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                "prvCpssPxTgfPortTxTailDropDbaPortAlphaOnReceivePortSet");

        /* block TxQ 7 for receive port */
        rc = cpssPxPortTxDebugQueueTxEnableSet(prvTgfDevNum,
                PRV_TGF_PORT_TO_RECEIVE_CNS, PRV_TGF_TRAFFIC_CLASS_CNS,
                GT_FALSE);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                "cpssPxPortTxDebugQueueTxEnableSet");

        /* transmit frames to send port */
        ingressPacketInfo.totalLen = TGF_PACKET_AUTO_CALC_LENGTH_CNS;
        ingressPacketInfo.numOfParts = 1;
        ingressPacketInfo.partsArray = &ingressFullPart[0];

        ingressFullPart[0].type = TGF_PACKET_PART_PAYLOAD_E;
        ingressFullPart[0].partPtr = &ingressPayloadPart;

        ingressPayloadPart.dataLength = frameLength;
        ingressPayloadPart.dataPtr    = frame;

        rc = tgfTrafficGeneratorPxTransmitPacketAndCheckResult(prvTgfDevNum,
                PRV_TGF_PORT_TO_TRANSMIT_CNS, &ingressPacketInfo,
                PRV_TGF_COUNT_OF_FRAMES_TO_TRANSMIT_CNS, 0, NULL);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                "tgfTrafficGeneratorPxTransmitPacketAndCheckResult");

        /* release TxQ 7 for receive port */
        rc = cpssPxPortTxDebugQueueTxEnableSet(prvTgfDevNum,
                PRV_TGF_PORT_TO_RECEIVE_CNS, PRV_TGF_TRAFFIC_CLASS_CNS,
                GT_TRUE);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                "cpssPxPortTxDebugQueueTxEnableSet");

        /* wait */
        cpssOsTimerWkAfter(10);

        /* read counters */
        rc = cpssPxTgfEthCountersRead(prvTgfDevNum,
                PRV_TGF_PORT_TO_RECEIVE_CNS, &portCounters);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssPxTgfEthCountersRead");

        /* verify counters */
        if ((portCounters.goodPktsSent < (expectedValue - 10)) ||
            (portCounters.goodPktsSent > (expectedValue + 10)))
        {
            UTF_VERIFY_EQUAL3_STRING_MAC(expectedValue, portCounters.goodPktsSent,
                    "Got other counter value than expected:\n"
                    "   Tx counter:      %d\n"
                    "   Expected range:  [%d..%d]\n",
                    portCounters.goodPktsSent,
                    expectedValue - 10, expectedValue + 10);
        }
        else
        {
            PRV_TGF_LOG3_MAC("Got Tx counter value as expected:\n"
                             "   Tx counter:      %d\n"
                             "   Expected range:  [%d..%d]\n",
                             portCounters.goodPktsSent,
                             expectedValue - 10, expectedValue + 10);
        }
    }
}

