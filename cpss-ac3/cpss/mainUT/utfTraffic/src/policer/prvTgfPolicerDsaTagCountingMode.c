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
* @file prvTgfPolicerDsaTagCountingMode.c
*
* @brief DSA Tag Counting Mode test.
*
* @version   6
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/generic/pcl/cpssPcl.h>
#include <cpss/generic/cpssCommonDefs.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>
#include <utf/private/prvUtfExtras.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <policer/prvTgfPolicerDsaTagCountingMode.h>

#include <common/tgfCommon.h>
#include <common/tgfPolicerGen.h>
#include <common/tgfPolicerStageGen.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfCscdGen.h>

/* VID */
#define PRV_TGF_VLANID_CNS 5

/* traffic generation sending port */
#define ING_PORT_IDX_CNS    0

/* target port */
#define EGR_PORT_IDX_CNS    1

/* final target device & port */
#define PRV_TGF_DSA_DEV_CNS     16
#define PRV_TGF_DSA_PORT_CNS    0x01

#define CBS_CNS    65 /*0x41*/
#define EBS_CNS    81 /*0x51*/

static GT_U32       currentMru = 0;
static GT_U32       currentCbs = CBS_CNS;
static GT_U32       currentEbs = EBS_CNS;
/* default number of packets to send */
static GT_U32   prvTgfBurstCount = 1;

/******************************* Test packet **********************************/


static TGF_PACKET_L2_STC prvTgfPacketL2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x17},                /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x08}                 /* saMac */
};

/********************** DSA tag  ***************************/
static TGF_DSA_DSA_FORWARD_STC  prvTgfPacketDsaTagPart_forward = {
    PRV_TGF_BRG_VLAN_PORT_UNTAGGED_CMD_E , /*srcIsTagged*/
    3,/*srcHwDev*/
    GT_FALSE,/* srcIsTrunk */
    /*union*/
    {
        /*trunkId*/
        10/*portNum*/ /* Set in runtime to avoid BE init problem for the union */
    },/*source;*/

    0,/*srcId*/

    GT_FALSE,/*egrFilterRegistered*/
    GT_FALSE,/*wasRouted*/
    0,/*qosProfileIndex*/

    /*CPSS_INTERFACE_INFO_STC         dstInterface*/
    {
        CPSS_INTERFACE_PORT_E,/*type*/

        /*struct*/{
            0,/*devNum*/
            1/*portNum*/
        },/*devPort*/

        0,/*trunkId*/
        0, /*vidx*/
        1,/*vlanId*/
        16,/*devNum*/
        0,/*fabricVidx*/
        0 /*index*/
    },/*dstInterface*/
    GT_FALSE,/*isTrgPortValid*/
    1,/*dstEport*/
    0,/*TPID Index*/
    GT_FALSE,/*origSrcPhyIsTrunk*/
    /*union*/
    {
        /*trunkId*/
        0/*portNum*/ /* Set in runtime to avoid BE init problem for the union */
    },/*origSrcPhy;*/
    GT_FALSE,/*phySrcMcFilterEnable*/
    0, /* hash */
    GT_TRUE /*skipFdbSaLookup*/
};

static TGF_PACKET_DSA_TAG_STC  prvTgfPacketDsaTagPart = {
    TGF_DSA_CMD_FORWARD_E ,/*dsaCommand*/
    TGF_DSA_4_WORD_TYPE_E ,/*dsaType*/

    /*TGF_DSA_DSA_COMMON_STC*/
    {
        0,/*vpt*/
        0,/*cfiBit*/
        PRV_TGF_VLANID_CNS,/*vid*/
        GT_FALSE,/*dropOnSource*/
        GT_FALSE/*packetIsLooped*/
    }/*commonParams*/

/*    union{
        TGF_DSA_DSA_TO_CPU_STC         toCpu;
        TGF_DSA_DSA_FROM_CPU_STC       fromCpu;
        TGF_DSA_DSA_TO_ANALYZER_STC    toAnalyzer;
        TGF_DSA_DSA_FORWARD_STC        forward;
    }dsaInfo;*/
};

/* DATA of packet */
static GT_U8 prvTgfPayloadDataArr[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
    0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f
};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart = {
    sizeof(prvTgfPayloadDataArr),                       /* dataLength */
    prvTgfPayloadDataArr                                /* dataPtr */
};

/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfPacketPartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_DSA_TAG_E,   &prvTgfPacketDsaTagPart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* LENGTH of packet */
#define PRV_TGF_PACKET_LEN_CNS \
 (TGF_L2_HEADER_SIZE_CNS + TGF_eDSA_TAG_SIZE_CNS + sizeof(prvTgfPayloadDataArr))

/* PACKET to send info */
static TGF_PACKET_STC prvTgfPacketInfo = {
    PRV_TGF_PACKET_LEN_CNS,                                     /* totalLen */
    sizeof(prvTgfPacketPartArray) / sizeof(TGF_PACKET_PART_STC),/* numOfParts */
    prvTgfPacketPartArray                                       /* partsArray */
};

/**********************/
/* Restore parameters */
/**********************/

/* save for restore stage metering enable */
static GT_BOOL saveStageMeterEnable[PRV_TGF_POLICER_STAGE_NUM];

/* save for restore port metering enable */
static GT_BOOL savePortMeterEnable[PRV_TGF_POLICER_STAGE_NUM];

/* save for restore meter mode: port or flow */
static PRV_TGF_POLICER_STAGE_METER_MODE_ENT
                    saveMeterMode[PRV_TGF_POLICER_STAGE_NUM];

/* save for restore counting mode */
static PRV_TGF_POLICER_COUNTING_MODE_ENT
                    saveCountMode[PRV_TGF_POLICER_STAGE_NUM];

/* save for restore MRU value */
static GT_U32 saveMru[PRV_TGF_POLICER_STAGE_NUM];

static CPSS_INTERFACE_INFO_STC                            savePhyInfo;
static CPSS_CSCD_LINK_TYPE_STC                            cascadeLinkForRestore;
static PRV_TGF_CSCD_TRUNK_LINK_HASH_ENT                   srcPortTrunkHashEnForRestore;
static PRV_TGF_PCL_EGRESS_TARGET_PORT_SELECTION_MODE_ENT  portSelectionModeForRestore;
static CPSS_POLICER_PACKET_SIZE_MODE_ENT                  packetSizeForRestore;

/**
* @internal mruSet function
* @endinternal
*
* @brief   set needed MRU for the test .
*
* @param[in] stage                    - policer  (iplr0/1/eplr)
* @param[in] mru                      - policer MRU
*                                       None
*/
static void mruSet(
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT stage,
    IN GT_U32  mru
)
{
    GT_STATUS rc;

    currentMru = mru;

    if(PRV_CPSS_SIP_5_15_CHECK_MAC(prvTgfDevNum))
    {
        /* AUTODOC: statistical metering :

            NOTE: we can not get yellow/red if currentMru will be 0 !!!

           to differentiate between 'with dsa' and 'without dsa' counting
           we need more then single packet

           first packet will be green on both cases , but second pocket will be :
           1. green  - for 'without dsa' mode
           2. yellow - for 'with dsa'


           expected behavior:
           scenario for 2 packets:
           MRU = 0x40
           CBS = 0x41 + 0x40
           EBS = 0x51 + 0x40

           ==================
           'without dsa' counting mode :
                   ==================
                   cpu set the entry:
                   CBS,EBS

                   (the pp) update signs :
                       sign0 = true , sign1 = true : because TB0 and TB1 more then MRU
                       (meaning next packet considered green)
                   ==================
                   first packet processed:
                        1. (the pp) consider as green because sign0 = true
                        2. (the pp) update token buckets:
                            a. TB0 = 0x41 + 0x40 - 0x40 = 0x41
                            b. TB1 = no change = 0x51 + 0x40
                        3. (the pp) update signs :
                            sign0 = true , sign1 = true : because TB0 and TB1 more then MRU
                            (meaning next packet considered green)
                   ==================
                   second packet processed:
                        1. (the pp) consider as green because sign0 = true
                        2. (the pp) update token buckets:
                            a. TB0 = 0x41 - 0x40 = 0x1
                            b. TB1 = no change = 0x51 + 0x40
                        3. (the pp) update signs :
                            sign0 = false , sign1 = true : because TB0 and TB1 more then MRU
                            (meaning next packet considered yellow)

           ==================
           'with dsa' counting mode :
                   ==================
same as 'without' : cpu set the entry:
same as 'without' : CBS,EBS

same as 'without' : (the pp) update signs :
same as 'without' :     sign0 = true , sign1 = true : because TB0 and TB1 more then MRU
same as 'without' :     (meaning next packet considered green)
                   ==================
                   first packet processed:
same as 'without' :     1. (the pp) consider as green because sign0 = true
                        2. (the pp) update token buckets:
!!!diff!!!                  a. TB0 = 0x41 + 0x40 - 0x50 = 0x31
                            b. TB1 = no change = 0x51 + 0x40 = 0x91
                        3. (the pp) update signs :
!!!diff!!!                  sign0 = false , sign1 = true : because TB0 lower then MRU and TB1 more then MRU
!!!diff!!!                  (meaning next packet considered yellow)
                   ==================
                   second packet processed:
!!!diff!!!              1. (the pp) consider as yellow because sign0 = false
                        2. (the pp) update token buckets:
!!!diff!!!                  a. TB0 = no change = 0x31
!!!diff!!!                  b. TB1 = 0x91 - 0x40 = 0x51
                        3. (the pp) update signs :
!!!diff!!!                  sign0 = false , sign1 = true : because TB0 lower then MRU and TB1 more then MRU
                            (meaning next packet considered yellow)

        */

        prvTgfBurstCount = 2;
        currentMru = CBS_CNS - 1;
        /* add to 'basic' CBS the length of packets (minus 'first packet')*/
        currentCbs = CBS_CNS + (prvTgfBurstCount-1) * PRV_TGF_PACKET_LEN_CNS;
        currentEbs = EBS_CNS + (prvTgfBurstCount-1) * PRV_TGF_PACKET_LEN_CNS;
    }


    /* AUTODOC: set MRU value */
    rc = prvTgfPolicerMruSet(stage, currentMru);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPolicerMruSet: %d, %d, %d",
                                 prvTgfDevNum, stage, currentMru);


    return;
}

/**
* @internal setEgressPortCascade function
* @endinternal
*
* @brief   set/unset the egress port as cascade with DSA tag .
*
* @param[in] egressPort               - egress port to be DSA tagged
* @param[in] enable                   - enable/disable the cascade port.
*                                       None
*/
static GT_VOID setEgressPortCascade
(
    IN GT_PHYSICAL_PORT_NUM     egressPort,
    IN GT_BOOL                  enable
)
{
    GT_STATUS rc;

    /* AUTODOC: set the egress port to be DSA tagged */
    rc = tgfTrafficGeneratorEgressCscdPortEnableSet(
        prvTgfDevNum, egressPort, enable,
        enable ? CPSS_CSCD_PORT_DSA_MODE_4_WORDS_E : CPSS_CSCD_PORT_NETWORK_E);

    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                         "tgfTrafficGeneratorEgressCscdPortEnableSet: [%d] \n");
}
/**
* @internal setIngressPortCascade function
* @endinternal
*
* @brief   set/unset the ingress port as cascade with DSA tag .
*/
static GT_VOID setIngressPortCascade
(
    IN GT_PHYSICAL_PORT_NUM     ingressPort,
    IN GT_BOOL                  enable
)
{
    GT_STATUS rc;

    /* AUTODOC: set the ingress port to be DSA tagged */
    rc = tgfTrafficGeneratorIngressCscdPortEnableSet(
                                             prvTgfDevNum, ingressPort, enable);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                        "tgfTrafficGeneratorIngressCscdPortEnableSet: [%d] \n");
}
/**
* @internal prvTgfVlanEDsaTagRedirectMapEport2PhySet function
* @endinternal
*
* @brief   Maps ePort to physical interface
*
* @param[in] devNum                   - devNum
* @param[in] portNum                  -  ePort
* @param[in] phyInfoPtr               -  (pointer to) inteface to map
* @param[in] savePhyInfoPtr           -  (pointer to) save inteface
*
* @retval GT_OK                    - on success
*/
static GT_STATUS prvTgfVlanEDsaTagRedirectMapEport2PhySet
(
    IN  GT_U8                            devNum,
    IN  GT_PORT_NUM                      portNum,
    IN  CPSS_INTERFACE_INFO_STC          *phyInfoPtr,
    OUT CPSS_INTERFACE_INFO_STC          *savePhyInfoPtr
)
{
    GT_STATUS rc;

    if (savePhyInfoPtr != NULL)
    {
        rc = prvTgfBrgEportToPhysicalPortTargetMappingTableGet(
            devNum, portNum, savePhyInfoPtr);
        PRV_UTF_VERIFY_GT_OK(
            rc, "prvTgfBrgEportToPhysicalPortTargetMappingTableGet");
    }

    rc = prvTgfBrgEportToPhysicalPortTargetMappingTableSet(
        devNum, portNum, phyInfoPtr);
    PRV_UTF_VERIFY_GT_OK(
        rc, "prvTgfBrgEportToPhysicalPortTargetMappingTableSet");

    return GT_OK;
}

/**
* @internal prvTgfPolicerManagementCountersCheck function
* @endinternal
*
* @brief   Check the value of specified Management Counters
*/
static GT_VOID prvTgfPolicerManagementCountersCheck
(
    IN  PRV_TGF_POLICER_STAGE_TYPE_ENT      policerStage,
    IN  PRV_TGF_POLICER_MNG_CNTR_TYPE_ENT   mngCntrType,
    IN  GT_U32                              packetMngCntr
)
{
    GT_STATUS rc = GT_OK;
    PRV_TGF_POLICER_MNG_CNTR_ENTRY_STC prvTgfMngCntr;

    /* get Management Counters */
    rc = prvTgfPolicerManagementCountersGet(prvTgfDevNum, policerStage,
                                            PRV_TGF_POLICER_MNG_CNTR_SET0_E,
                                            mngCntrType,
                                            &prvTgfMngCntr);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                        "prvTgfPolicerManagementCountersGet: %d", prvTgfDevNum);

    /* check counters */
    UTF_VERIFY_EQUAL3_STRING_MAC(
        packetMngCntr, prvTgfMngCntr.packetMngCntr,
      "prvTgfMngCntr.packetMngCntr = %d, policerStage = %d, mngCntrType = %d\n",
        prvTgfMngCntr.packetMngCntr, policerStage, mngCntrType);
}

/**
* @internal prvTgfPolicerDsaTagCountingModeInitEntrySet function
* @endinternal
*
* @brief   Initial metering entry setting
*
* @param[in] stage                    - Policer stage
* @param[in] cbs                      - committed burst size
* @param[in] ebs                      - excess burst size
* @param[in] dsaTagCountingMode       - DSA tag counting mode
*                                       None
*/
static GT_VOID prvTgfPolicerDsaTagCountingModeInitEntrySet
(
    IN  PRV_TGF_POLICER_STAGE_TYPE_ENT            stage,
    IN  GT_U32                                    cbs,
    IN  GT_U32                                    ebs,
    IN  PRV_TGF_POLICER_DSA_TAG_COUNTING_MODE_ENT dsaTagCountingMode
)
{
    GT_STATUS rc;
    PRV_TGF_POLICER_ENTRY_STC   meterEntry;
    PRV_TGF_POLICER_METER_TB_PARAMS_UNT tbParams;

    cpssOsMemSet(&meterEntry, 0, sizeof(meterEntry));

    /* AUTODOC: Clear management counters */
    prvTgfPolicerManagementCountersReset(stage, PRV_TGF_POLICER_MNG_CNTR_SET0_E);

    meterEntry.policerEnable = GT_TRUE;
    meterEntry.counterEnable = GT_TRUE;
    meterEntry.mngCounterSet = PRV_TGF_POLICER_MNG_CNTR_SET0_E;
    meterEntry.meterMode = PRV_TGF_POLICER_METER_MODE_SR_TCM_E;

    meterEntry.tunnelTerminationPacketSizeMode = 0;
    meterEntry.timeStampCountingMode = 0;
    if (PRV_TGF_POLICER_STAGE_EGRESS_E == stage)
    {
        meterEntry.countingEntryIndex = prvTgfPortsArray[EGR_PORT_IDX_CNS];
    }
    else
    {
        meterEntry.countingEntryIndex = prvTgfPortsArray[ING_PORT_IDX_CNS];
    }
    meterEntry.tbParams.srTcmParams.cir = 1; /* not used in simulation */
    meterEntry.tbParams.srTcmParams.cbs = cbs;
    meterEntry.tbParams.srTcmParams.ebs = ebs;
    meterEntry.dsaTagCountingMode = dsaTagCountingMode;

    /* AUTODOC: Metering according to L2 */
    meterEntry.byteOrPacketCountingMode =
                                       PRV_TGF_POLICER_METER_RESOLUTION_BYTES_E;
    meterEntry.packetSizeMode = CPSS_POLICER_PACKET_SIZE_L2_INCLUDE_E;

    if (PRV_TGF_POLICER_STAGE_EGRESS_E == stage)
    {
        meterEntry.modifyDscp = PRV_TGF_POLICER_MODIFY_DSCP_DISABLE_E;
        meterEntry.modifyUp   = PRV_TGF_POLICER_MODIFY_UP_DISABLE_E;
        meterEntry.modifyDp   = CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
        meterEntry.modifyExp  = CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
        meterEntry.modifyTc   = CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;

        /* AUTODOC: Setting the initial metering entry */
        rc = prvTgfPolicerEntrySet(stage,
                                   prvTgfPortsArray[EGR_PORT_IDX_CNS],
                                   &meterEntry,
                                   &tbParams);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPolicerEntrySet: %d, %d",
                                     stage,
                                     prvTgfPortsArray[EGR_PORT_IDX_CNS]);
    }
    else
    {
        /* AUTODOC: Setting the initial metering entry */
        rc = prvTgfPolicerEntrySet(stage,
                                   prvTgfPortsArray[ING_PORT_IDX_CNS],
                                   &meterEntry,
                                   &tbParams);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPolicerEntrySet: %d, %d",
                                     stage,
                                     prvTgfPortsArray[ING_PORT_IDX_CNS]);
    }

    rc = prvTgfPolicerPacketSizeModeSet (stage,
                                         CPSS_POLICER_PACKET_SIZE_L2_INCLUDE_E);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                 "prvTgfPolicerPacketSizeModeSet: %d, %d",
                                 stage,
                                 CPSS_POLICER_PACKET_SIZE_L2_INCLUDE_E);

}

/**
* @internal prvTgfPolicerDsaTagCountingModeConfigurationSet function
* @endinternal
*
* @brief   Basic DSA Tag Counting Mode test configurations
*
* @param[in] stage                    - Policer stage
*                                       None
*/
static GT_VOID prvTgfPolicerDsaTagCountingModeConfigurationSet
(
    PRV_TGF_POLICER_STAGE_TYPE_ENT stage
)
{
    GT_STATUS                        rc;
    CPSS_INTERFACE_INFO_STC          physicalInfo;
    CPSS_CSCD_LINK_TYPE_STC          cascadeLink;
    PRV_TGF_CSCD_TRUNK_LINK_HASH_ENT srcPortTrunkHashEn;
    PRV_TGF_BRG_MAC_ENTRY_STC        prvTgfMacEntry;

    /* AUTODOC: SETUP CONFIGURATION: */
    cpssOsMemSet(&physicalInfo, 0, sizeof(physicalInfo));

    physicalInfo.type = CPSS_INTERFACE_PORT_E;
    physicalInfo.devPort.hwDevNum = prvTgfDevNum;
    physicalInfo.devPort.portNum = prvTgfPortsArray[EGR_PORT_IDX_CNS];

    /* AUTODOC: Save stage metering enable for restore */
    rc = prvTgfPolicerMeteringEnableGet(prvTgfDevNum,
                                        stage,
                                        &saveStageMeterEnable[stage]);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
                                 "prvTgfPolicerMeteringEnableGet: %d, %d, %d",
                                 prvTgfDevNum,
                                 stage,
                                 saveStageMeterEnable[stage]);

    /* AUTODOC: Enable stage for metering */
    rc = prvTgfPolicerMeteringEnableSet(stage, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                 "prvTgfPolicerMeteringEnableSet: %d, %d",
                                 stage, GT_TRUE);

    /* AUTODOC: Save port metering enable for restore */
    rc = prvTgfPolicerPortMeteringEnableGet(prvTgfDevNum,
                                            stage,
                                            prvTgfPortsArray[EGR_PORT_IDX_CNS],
                                            &savePortMeterEnable[stage]);
    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc,
                           "prvTgfPolicerPortMeteringEnableGet: %d, %d, %d, %d",
                                 prvTgfDevNum,
                                 stage,
                                 prvTgfPortsArray[EGR_PORT_IDX_CNS],
                                 savePortMeterEnable[stage]);

    /* AUTODOC: Enable port for metering */
    if (PRV_TGF_POLICER_STAGE_EGRESS_E == stage)
    {
        rc = prvTgfPolicerPortMeteringEnableSet(prvTgfDevNum,
                                                stage,
                                             prvTgfPortsArray[EGR_PORT_IDX_CNS],
                                                GT_TRUE);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc,
                           "prvTgfPolicerPortMeteringEnableSet: %d, %d, %d, %d",
                                     prvTgfDevNum,
                                     stage,
                                     prvTgfPortsArray[EGR_PORT_IDX_CNS],
                                     GT_TRUE);
    }
    else
    {
        rc = prvTgfPolicerPortMeteringEnableSet(prvTgfDevNum,
                                             stage,
                                             prvTgfPortsArray[ING_PORT_IDX_CNS],
                                             GT_TRUE);
    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc,
                           "prvTgfPolicerPortMeteringEnableSet: %d, %d, %d, %d",
                                 prvTgfDevNum,
                                 stage,
                                 prvTgfPortsArray[ING_PORT_IDX_CNS],
                                 GT_TRUE);
    }

    /* AUTODOC: Save metering mode for restore */
    rc = prvTgfPolicerStageMeterModeGet(prvTgfDevNum,
                                        stage,
                                        &saveMeterMode[stage]);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
                                 "prvTgfPolicerMeteringEnableGet: %d, %d, %d",
                                 prvTgfDevNum,
                                 stage,
                                 saveMeterMode[stage]);

    /* AUTODOC: Set metering mode for "port" mode */
    rc = prvTgfPolicerStageMeterModeSet(stage,
                                       PRV_TGF_POLICER_STAGE_METER_MODE_PORT_E);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                 "prvTgfPolicerMeteringEnableSet: %d, %d",
                                 stage,
                                 PRV_TGF_POLICER_STAGE_METER_MODE_PORT_E);

    /* AUTODOC: Save counting mode for restore */
    rc = prvTgfPolicerCountingModeGet(prvTgfDevNum,
                                      stage,
                                      &saveCountMode[stage]);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
                                 "prvTgfPolicerCountingModeGet: %d, %d, %d",
                                 prvTgfDevNum, stage, saveCountMode[stage]);

    /* AUTODOC: Configure counting mode to billing */
    rc = prvTgfPolicerCountingModeSet(prvTgfDevNum, stage,
                                      PRV_TGF_POLICER_COUNTING_BILLING_IPFIX_E);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
                                 "prvTgfPolicerCountingModeSet: %d, %d %d",
                                 prvTgfDevNum, stage,
                                 PRV_TGF_POLICER_COUNTING_BILLING_IPFIX_E);

    /* AUTODOC: Save MRU for restore */
    rc = prvTgfPolicerMruGet(prvTgfDevNum, stage, &saveMru[stage]);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPolicerMruGet: %d, %d, %d",
                                 prvTgfDevNum, stage, saveMru[stage]);

    /* AUTODOC: set MRU value to be zero */
    mruSet(stage, 0);

    rc = prvTgfPolicerCountingColorModeSet(stage,
                                           PRV_TGF_POLICER_COLOR_COUNT_CL_E);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                "prvTgfPolicerCountingColorModeSet: %d, %d, %d",
                                prvTgfDevNum, stage);



    /* AUTODOC: set enable bridge bypass for ingress port*/
    rc = prvTgfCscdPortBridgeBypassEnableSet(prvTgfDevNum,
                                             prvTgfPortsArray[ING_PORT_IDX_CNS],
                                             GT_TRUE);

    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                   "prvTgfCscdPortBridgeBypassEnableSet: %d %d",
                              prvTgfDevNum, prvTgfPortsArray[ING_PORT_IDX_CNS]);

    /* AUTODOC: save target port selection mode for restore */
    rc = prvTgfPclEgressTargetPortSelectionModeGet(prvTgfDevNum,
                                                  &portSelectionModeForRestore);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                             "prvTgfPclEgressTargetPortSelectionModeGet: %d %d",
                             prvTgfDevNum, portSelectionModeForRestore);

    /* AUTODOC: set target port selection mode to LOCAL */
    rc = prvTgfPclEgressTargetPortSelectionModeSet(
                        PRV_TGF_PCL_EGRESS_TARGET_PORT_SELECTION_MODE_LOCAL_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                 "prvTgfPclEgressTargetPortSelectionModSet: %d",
                         PRV_TGF_PCL_EGRESS_TARGET_PORT_SELECTION_MODE_LOCAL_E);

    if (PRV_TGF_POLICER_STAGE_EGRESS_E == stage)
    {
        cpssOsMemSet(&prvTgfMacEntry, 0, sizeof(prvTgfMacEntry));

        /* AUTODOC: Set FDB entry with local ePort */
        prvTgfMacEntry.key.key.macVlan.vlanId = PRV_TGF_VLANID_CNS;
        prvTgfMacEntry.isStatic = GT_TRUE;
        prvTgfMacEntry.dstInterface.type = CPSS_INTERFACE_PORT_E;
        prvTgfMacEntry.dstInterface.devPort.portNum = PRV_TGF_DSA_PORT_CNS;
        cpssOsMemCpy(prvTgfMacEntry.key.key.macVlan.macAddr.arEther,
                     prvTgfPacketL2Part.daMac, sizeof(TGF_MAC_ADDR));

        rc = prvTgfBrgFdbMacEntrySet(&prvTgfMacEntry);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbMacEntrySet");
    }

    /* AUTODOC: map ePort to physical port 2 */
    prvTgfVlanEDsaTagRedirectMapEport2PhySet( prvTgfDevNum,PRV_TGF_DSA_PORT_CNS,
                                           &physicalInfo, &savePhyInfo);

    /* save the current cascade map table */
    rc = prvTgfCscdDevMapTableGet(prvTgfDevNum, PRV_TGF_DSA_DEV_CNS,
                                      PRV_TGF_DSA_PORT_CNS,
                                  &cascadeLinkForRestore,
                                  &srcPortTrunkHashEnForRestore);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,"prvTgfCscdDevMapTableGet: %d,%d,%d",
                                 prvTgfDevNum, PRV_TGF_DSA_DEV_CNS,
                                 PRV_TGF_DSA_PORT_CNS);

    cascadeLink.linkNum = prvTgfPortsArray[EGR_PORT_IDX_CNS];
    cascadeLink.linkType = CPSS_CSCD_LINK_TYPE_PORT_E;
    srcPortTrunkHashEn = PRV_TGF_CSCD_TRUNK_LINK_HASH_IS_SRC_PORT_E;
    /* AUTODOC: set cascade mapping for remote device\port */
    rc = prvTgfCscdDevMapTableSet(prvTgfDevNum, PRV_TGF_DSA_DEV_CNS,
                                  PRV_TGF_DSA_PORT_CNS, &cascadeLink,
                                  srcPortTrunkHashEn, GT_FALSE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,"prvTgfCscdDevMapTableSet: %d,%d,%d",
                                 prvTgfDevNum, PRV_TGF_DSA_DEV_CNS,
                                 PRV_TGF_DSA_PORT_CNS);

    if (PRV_TGF_POLICER_STAGE_EGRESS_E == stage)
    {
        setEgressPortCascade(  prvTgfPortsArray[EGR_PORT_IDX_CNS], GT_TRUE );
        setIngressPortCascade( prvTgfPortsArray[ING_PORT_IDX_CNS], GT_TRUE );
    }
    else
    {
        setIngressPortCascade( prvTgfPortsArray[ING_PORT_IDX_CNS], GT_TRUE );
    }

    rc = prvTgfPolicerPacketSizeModeGet (prvTgfDevNum, stage,
                                         &packetSizeForRestore);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                               "prvTgfPolicerPacketSizeModeGet: %d, %d", stage);

}

/**
* @internal prvTgfPolicerDsaTagCountingModeRestore function
* @endinternal
*
* @brief   Basic DSA Tag Counting Mode test restore configurations
*
* @param[in] stage                    - Policer stage
*                                       None
*/
static GT_VOID prvTgfPolicerDsaTagCountingModeRestore
(
    PRV_TGF_POLICER_STAGE_TYPE_ENT stage
)
{
    GT_STATUS rc;
    PRV_TGF_POLICER_ENTRY_STC   meterEntry;
    PRV_TGF_POLICER_METER_TB_PARAMS_UNT tbParams;

    /* AUTODOC: Invalidate metering entry (by clear)*/
    cpssOsMemSet(&meterEntry, 0, sizeof(meterEntry));
    meterEntry.packetSizeMode = CPSS_POLICER_PACKET_SIZE_L3_ONLY_E;

    if (PRV_TGF_POLICER_STAGE_EGRESS_E == stage)
    {
        meterEntry.modifyDscp = PRV_TGF_POLICER_MODIFY_DSCP_DISABLE_E;
        meterEntry.modifyUp   = PRV_TGF_POLICER_MODIFY_UP_DISABLE_E;
        meterEntry.modifyDp   = CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
        meterEntry.modifyExp  = CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
        meterEntry.modifyTc   = CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
    }

    setEgressPortCascade(  prvTgfPortsArray[EGR_PORT_IDX_CNS], GT_FALSE );
    setIngressPortCascade( prvTgfPortsArray[ING_PORT_IDX_CNS], GT_FALSE );

    rc = prvTgfPolicerPacketSizeModeSet (stage, packetSizeForRestore);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                 "prvTgfPolicerPacketSizeModeSet: %d, %d",
                                 stage, packetSizeForRestore);

    rc = prvTgfPolicerEntrySet(stage,
                               prvTgfPortsArray[EGR_PORT_IDX_CNS],
                               &meterEntry,
                               &tbParams);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPolicerEntrySet: %d, %d",
                                 stage,
                                 prvTgfPortsArray[EGR_PORT_IDX_CNS]);

    /* AUTODOC: Restore stage metering enable */
    rc = prvTgfPolicerMeteringEnableSet(stage,
                                        saveStageMeterEnable[stage]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                 "prvTgfPolicerMeteringEnableSet: %d, %d",
                                 stage,
                                 saveStageMeterEnable[stage]);

    /* AUTODOC: Restore port metering enable */
    rc = prvTgfPolicerPortMeteringEnableSet(prvTgfDevNum,
                                            stage,
                                            prvTgfPortsArray[EGR_PORT_IDX_CNS],
                                            savePortMeterEnable[stage]);
    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc,
                           "prvTgfPolicerPortMeteringEnableSet: %d, %d, %d, %d",
                                 prvTgfDevNum,
                                 stage,
                                 prvTgfPortsArray[EGR_PORT_IDX_CNS],
                                 savePortMeterEnable[stage]);

    /* AUTODOC: Restore metering mode */
    rc = prvTgfPolicerStageMeterModeSet(stage,
                                        saveMeterMode[stage]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                 "prvTgfPolicerMeteringEnableSet: %d, %d",
                                 stage,
                                 saveMeterMode[stage]);

    /* AUTODOC: Restore counting mode */
    rc = prvTgfPolicerCountingModeSet(prvTgfDevNum, stage,saveCountMode[stage]);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
                                 "prvTgfPolicerCountingModeSet: %d, %d %d",
                                 prvTgfDevNum, stage, saveCountMode[stage]);

    /* AUTODOC: restore MRU  */
    rc = prvTgfPolicerMruSet(stage, saveMru[stage]);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPolicerMruSet: %d, %d, %d",
                                 prvTgfDevNum, stage, saveMru[stage]);

    /* AUTODOC: restore source port selection mode */
    rc = prvTgfPclEgressTargetPortSelectionModeSet(portSelectionModeForRestore);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                 "prvTgfPclEgressTargetPortSelectionModSet: %d",
                                 portSelectionModeForRestore);

    /* AUTODOC: map ePort to physical port 2 */
    prvTgfVlanEDsaTagRedirectMapEport2PhySet( prvTgfDevNum,PRV_TGF_DSA_PORT_CNS,
                                           &savePhyInfo, NULL);

    /* AUTODOC: restore cascade mapping */
    rc = prvTgfCscdDevMapTableSet(prvTgfDevNum, PRV_TGF_DSA_DEV_CNS,
                                  PRV_TGF_DSA_PORT_CNS, &cascadeLinkForRestore,
                                  srcPortTrunkHashEnForRestore, GT_FALSE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,"prvTgfCscdDevMapTableSet: %d,%d,%d",
                                 prvTgfDevNum,
                                 PRV_TGF_DSA_DEV_CNS, PRV_TGF_DSA_PORT_CNS);

    currentMru = 0;
    currentCbs = CBS_CNS;
    currentEbs = EBS_CNS;
    prvTgfBurstCount = 1;
}


/**
* @internal prvTgfVlanEDsaTagTestPacketSend function
* @endinternal
*
* @brief   Function sends packet.
*
* @param[in] portInterfacePtr         - (pointer to) port interface
* @param[in] packetInfoPtr            - (pointer to) the packet info
*                                       None
*/
static GT_VOID prvTgfVlanEDsaTagTestPacketSend
(
    IN CPSS_INTERFACE_INFO_STC  *portInterfacePtr,
    IN TGF_PACKET_STC           *packetInfoPtr
)
{
    GT_STATUS rc          = GT_OK;
    GT_U32    portsCount  = prvTgfPortsNum;
    GT_U32    portIter    = 0;

    /* Set union fields in the DSA structure */
    prvTgfPacketDsaTagPart_forward.source.portNum = 10;
    prvTgfPacketDsaTagPart_forward.origSrcPhy.portNum = 0;

    /* bind the DSA tag FORWARD part */
    prvTgfPacketDsaTagPart.dsaInfo.forward = prvTgfPacketDsaTagPart_forward;

    /* use different 'etherType for vlan tagging' instead of 0x8100 */
    /*this will make our packet that has '0x8100' to be recognized as 'untagged'
      by the 'traffic generator' and also the CPSS will not override the
      ethertype of tag0 with the DSA tag */
    tgfTrafficGeneratorEtherTypeForVlanTagSet(TGF_ETHERTYPE_NON_VALID_TAG_CNS);

    /* AUTODOC: reset counters */
    for (portIter = 0; portIter < portsCount; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                     "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* setup Packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, packetInfoPtr, prvTgfBurstCount,
                             0, NULL);
    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d, %d",
                                 prvTgfDevNum, prvTgfBurstCount, 0, 0);

    /* AUTODOC: clear table */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* AUTODOC: Start capturing Tx packets */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(portInterfacePtr,
                                                TGF_CAPTURE_MODE_MIRRORING_E,
                                                GT_TRUE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
                           "tgfTrafficGeneratorPortTxEthCaptureSet: %d, %d, %d",
                                 portInterfacePtr->devPort.hwDevNum,
                                 portInterfacePtr->devPort.portNum,
                                 GT_TRUE);

    /* AUTODOC: send packet */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum,
                                   prvTgfPortsArray[ING_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                 "ERROR of StartTransmitting: %d, %d",
                                 prvTgfDevNum,
                                 prvTgfPortsArray[ING_PORT_IDX_CNS]);

    /* AUTODOC: wait for packets come to CPU */
    (void) tgfTrafficGeneratorRxInCpuNumWait(prvTgfBurstCount, 500, NULL);

    /* AUTODOC: Stop capturing Tx packets */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(portInterfacePtr,
                                                TGF_CAPTURE_MODE_MIRRORING_E,
                                                GT_FALSE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
                           "tgfTrafficGeneratorPortTxEthCaptureSet: %d, %d, %d",
                                 portInterfacePtr->devPort.hwDevNum,
                                 portInterfacePtr->devPort.portNum,
                                 GT_FALSE);

    PRV_UTF_LOG1_MAC("Port [%d] capturing:\n",
                     portInterfacePtr->devPort.portNum);

    /* AUTODOC: restore default ethertype */
    tgfTrafficGeneratorEtherTypeForVlanTagSet(TGF_ETHERTYPE_8100_VLAN_TAG_CNS);
}

/**
* @internal prvTgfPolicerDsaTagCountingModeTestGenerateTraffic function
* @endinternal
*
* @brief   Test generate traffic and check results
*/
GT_VOID prvTgfPolicerDsaTagCountingModeTestGenerateTraffic
(
    PRV_TGF_POLICER_STAGE_TYPE_ENT          stage
)
{
    GT_STATUS                       rc;
    CPSS_INTERFACE_INFO_STC         portInterface;
    CPSS_PORT_MAC_COUNTER_SET_STC   portCntrs;
    GT_U32                          expMngCounters;
    GT_U32                          cbs;
    GT_U32                          ebs;

    /* AUTODOC: set port for packet capture */
    portInterface.type = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum = prvTgfDevNum;
    portInterface.devPort.portNum = prvTgfPortsArray[EGR_PORT_IDX_CNS];

    cbs = currentCbs;
    ebs = currentEbs;

    if(PRV_CPSS_SIP_5_15_CHECK_MAC(prvTgfDevNum))
    {
        /* do not allow different EPLR calculations as the IPLR and EPLR
           calculations done according to 'correct' device behavior , without
           need for 'CRC' compensation .

           the IPLR hold 4 bytes CRC and so is the EPLR.
        */
    }
    else
    if(stage == PRV_TGF_POLICER_STAGE_EGRESS_E &&
       (GT_FALSE == prvTgfPolicerEgressIsByteCountCrcExclude()))
    {
        /* the EPLR will add 4 bytes CRC */
        cbs += 4;
        ebs += 4;
    }

    prvTgfPolicerDsaTagCountingModeInitEntrySet(stage, cbs, ebs,
                           PRV_TGF_POLICER_DSA_TAG_COUNTING_MODE_COMPENSATED_E);

    /* AUTODOC: send packet. */
    prvTgfVlanEDsaTagTestPacketSend(&portInterface, &prvTgfPacketInfo);

    /* AUTODOC: check counters */

    /* read Rx counter */
    rc = prvTgfReadPortCountersEth(prvTgfDevNum,
                                   prvTgfPortsArray[ING_PORT_IDX_CNS],
                                   GT_TRUE, &portCntrs);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                 "prvTgfReadPortCountersEth: %d, %d, %d",
                                 prvTgfDevNum,
                                 prvTgfPortsArray[ING_PORT_IDX_CNS]);

    UTF_VERIFY_EQUAL1_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsRcv.l[0],
                 "get another goodPktsRcv counter than expected on port %d, %d",
                            prvTgfPortsArray[ING_PORT_IDX_CNS]);

    /* Expected management counters */
    expMngCounters = prvTgfBurstCount;/*PRV_CPSS_SIP_5_15_CHECK_MAC(prvTgfDevNum) more then 1*/
    PRV_UTF_LOG1_MAC("expecting %d GREEN packets \n",
                     expMngCounters);

    prvTgfPolicerManagementCountersCheck(stage,PRV_TGF_POLICER_MNG_CNTR_GREEN_E,
                                         expMngCounters);

    prvTgfPolicerDsaTagCountingModeInitEntrySet(stage, cbs, ebs,
                              PRV_TGF_POLICER_DSA_TAG_COUNTING_MODE_INCLUDED_E);

    /* AUTODOC: send packet. */
    prvTgfVlanEDsaTagTestPacketSend(&portInterface, &prvTgfPacketInfo);

    /* AUTODOC: check counters */

    /* read Rx counter */
    rc = prvTgfReadPortCountersEth(prvTgfDevNum,
                                   prvTgfPortsArray[ING_PORT_IDX_CNS],
                                   GT_TRUE, &portCntrs);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                 "prvTgfReadPortCountersEth: %d, %d, %d",
                                 prvTgfDevNum,
                                 prvTgfPortsArray[ING_PORT_IDX_CNS]);

    UTF_VERIFY_EQUAL1_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsRcv.l[0],
                 "get another goodPktsRcv counter than expected on port %d, %d",
                            prvTgfPortsArray[ING_PORT_IDX_CNS]);

    /* Expected management counters */
    if(PRV_CPSS_SIP_5_15_CHECK_MAC(prvTgfDevNum))
    {
        expMngCounters = prvTgfBurstCount - 1;
        PRV_UTF_LOG1_MAC("expecting %d GREEN packets and 1 YELLOW packet \n",
                         expMngCounters);
        prvTgfPolicerManagementCountersCheck(stage,
                                             PRV_TGF_POLICER_MNG_CNTR_GREEN_E,
                                             expMngCounters);
    }
    else
    {
        PRV_UTF_LOG0_MAC("expecting one YELLOW packet \n");
    }

    expMngCounters = 1;
    prvTgfPolicerManagementCountersCheck(stage,
                                         PRV_TGF_POLICER_MNG_CNTR_YELLOW_E,
                                         expMngCounters);

}

/**
* @internal prvTgfPolicerDsaTagCountingModeTest function
* @endinternal
*
* @brief   Basic DSA Tag Counting Mode entry test
*
* @param[in] stage                    - Policer stage
*                                       None
*/
GT_VOID prvTgfPolicerDsaTagCountingModeTest
(
    PRV_TGF_POLICER_STAGE_TYPE_ENT stage
)
{
    if(stage > PRV_TGF_POLICER_STAGE_EGRESS_E)
    {
        /* will never happen ... but need to fix clockwork issues */
        return;
    }

    /* AUTODOC: Test configurations */
    prvTgfPolicerDsaTagCountingModeConfigurationSet(stage);

    /* AUTODOC: Test traffic and checks */
    prvTgfPolicerDsaTagCountingModeTestGenerateTraffic(stage);

    /* AUTODOC: Restore configurations */
    prvTgfPolicerDsaTagCountingModeRestore(stage);
}

/**
* @internal prvTgfPolicerDsaTagCountingModeGlobalConfigurationSet function
* @endinternal
*
* @brief   Generic global configurations for DSA Tag Counting Mode test
*/
GT_VOID prvTgfPolicerDsaTagCountingModeGlobalConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS   rc;

    /* AUTODOC: Enable "eArch" metering entry format parsing */
    rc = prvTgfPolicerEntryUseEarchEnable(GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                 "prvTgfPolicerEntryUseEarchEnable: %d",
                                 GT_TRUE);

    /* AUTODOC: Create vlan on all ports */
    rc = prvTgfBrgDefVlanEntryWrite(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEntryWrite: %d, %d",
                                 prvTgfDevNum, PRV_TGF_VLANID_CNS);
}

/**
* @internal prvTgfPolicerDsaTagCountingModeGlobalConfigurationRestore function
* @endinternal
*
* @brief   Restore Global test configuration
*/
GT_VOID prvTgfPolicerDsaTagCountingModeGlobalConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;

    /****************/
    /* Tables Reset */
    /****************/

    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* AUTODOC: Flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d",
                                 prvTgfDevNum, GT_TRUE);

    /* Invalidate vlan entries (and reset vlan entry) */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                 "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
                                 prvTgfDevNum, PRV_TGF_VLANID_CNS);

    /* AUTODOC: Disable "eArch" metering entry format parsing */
    rc = prvTgfPolicerEntryUseEarchEnable(GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                 "prvTgfPolicerEntryUseEarchEnable: %d",
                                 GT_FALSE);
}

