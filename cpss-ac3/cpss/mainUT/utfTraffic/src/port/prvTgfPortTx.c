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
* @file prvTgfPortTx.c
*
* @brief Port Tx Queue features testing
*
* @version   16
********************************************************************************
*/

/* the define of UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC must come from C files that
   already fixed the types of ports from GT_U8 !

   NOTE: must come before ANY include to H files !!!!
*/
#define UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/generic/cpssCommonDefs.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>
#include <utf/private/prvUtfExtras.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfPortGen.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfCosGen.h>
#include <common/tgfVntGen.h>
#include <port/prvTgfPortFWS.h>
#include <common/tgfCncGen.h>
#include <cnc/prvTgfCncVlanL2L3.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortTx.h>
#include <appDemo/boardConfig/appDemoBoardConfig.h>
/* default VLAN Id */
#define PRV_TGF_VLANID_CNS            223

/* ether type used for "vlan tag" */
#define TGF_ETHERTYPE_8100_VLAN_TAG_CNS 0x8100

/* egress port index */
#define PRV_TGF_EGR_PORT_IDX_0_CNS  0

/* maximal number of ingress ports that may be used in test */
#define PRV_TGF_MAX_INGR_PORTS_NUM_CNS  7

/* actual number of ingress ports used in test */
GT_U32 prvTgfPortTxSpIngrPortsNum = 4;

/* acceptable difference between calculated rate and counted one */
GT_U32 prvTgfWsDiffInPercent = 2;

/* debug mode */
GT_U32 prvTgfPortTxSpDebug = 0;

/* open log */
GT_U32 prvTgfPortTxOpenLog = 0;
#define PORT_TX_PRV_UTF_LOG0_MAC \
    if(prvTgfPortTxOpenLog) PRV_UTF_LOG0_MAC

#define PORT_TX_PRV_UTF_LOG1_MAC \
    if(prvTgfPortTxOpenLog) PRV_UTF_LOG1_MAC

#define PORT_TX_PRV_UTF_LOG2_MAC \
    if(prvTgfPortTxOpenLog) PRV_UTF_LOG2_MAC

GT_U32 prvTgfPortTxOpenLogSet(GT_U32 newVal)
{
    GT_U32 old = prvTgfPortTxOpenLog;
    prvTgfPortTxOpenLog = newVal;
    return  old;
}
/* test use 8 ports.
   7 ingress port and last one egress */
#define PRV_TGF_TX_TEST_PORT_NUM   8

/* PRV_TGF_DEVICE_TYPE_COUNT_E + bobcat3 10 Giga ports array */
#define PRV_TGF_TX_PORT_ARRAY_SIZE   PRV_TGF_DEVICE_TYPE_COUNT_E + 1

/* array of ports for test */
GT_U32   prvTgfPortTxPortsArray[PRV_TGF_TX_PORT_ARRAY_SIZE][PRV_TGF_TX_TEST_PORT_NUM] =
        {{0,  2,  5,  8, 11, 14, 18, 23}, /* 28 port devices */
         {0, 10, 18, 36, 58, 22, 40, 50}, /* 60 port devices,
                                 ports 1-4 should be in different port groups */
         {0,  8, 16, 36, 56, 73, 96, 116}, /* 128 port devices,
                                 ports 1-4 should be in different port groups */
         {56, 57, 58, 59, 64, 65, 66, 67}, /* Cetus devices port starting from 56 */
         {24, 40, 41, 42, 43, 25, 26, 27}};/*bobcat3 */
/* array of TC for test.
   values must be in ascent order  */
static GT_U32 tcArr[PRV_TGF_MAX_INGR_PORTS_NUM_CNS] = {1, 2, 3, 4, 5, 6, 7};

/* array of QoS Profile for test */
static GT_U8 qosArr[PRV_TGF_MAX_INGR_PORTS_NUM_CNS] = {1, 17, 23, 50, 77, 103, 127};

/* array of Ingress Ports indexes */
static GT_U32 inPortsIdxArr[PRV_TGF_MAX_INGR_PORTS_NUM_CNS] = {1, 2, 3, 4, 5, 6, 7};

/* array to store arbiter mode */
static CPSS_PORT_TX_Q_ARB_GROUP_ENT savePortTxArpArr[PRV_TGF_MAX_INGR_PORTS_NUM_CNS];

/* scheduler profile */
static GT_U32 prvPortTxProfile;

/* tail drop profile */
static CPSS_PORT_TX_DROP_PROFILE_SET_ENT prvPortTailDropProfile;

/* default number of packets to send */
static GT_U32   prvTgfBurstCount   = 1;

/* L2 part of packet LSB of MAC SA is changed in run time */
static TGF_PACKET_L2_STC prvTgfPacket1L2Part = {
    {0x00, 0xAA, 0xBB, 0xCC, 0xDD, 0x11},               /* dstMac */
    {0x00, 0x88, 0x99, 0x33, 0x44, 0x11}                /* srcMac */
};


/* First VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTag0Part = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_VLANID_CNS                            /* pri, cfi, VlanId */
};

/* DATA of packet */
static GT_U8 prvTgfPayloadDataArr[] = {
    0x55, 0x55, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x55, 0x55};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart = {
    sizeof(prvTgfPayloadDataArr),                       /* dataLength */
    prvTgfPayloadDataArr                                /* dataPtr */
};

/* PARTS of packet 1 */
static TGF_PACKET_PART_STC prvTgfPacket1PartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacket1L2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTag0Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};


/* LENGTH of packet */
#define PRV_TGF_PACKET_LEN_CNS \
    (TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + sizeof(prvTgfPayloadDataArr))

/* LENGTH of packet with CRC */
#define PRV_TGF_PACKET_CRC_LEN_CNS  PRV_TGF_PACKET_LEN_CNS + TGF_CRC_LEN_CNS


/* PACKET 1 to send */
static TGF_PACKET_STC prvTgfPacket1Info = {
    PRV_TGF_PACKET_LEN_CNS,                                      /* totalLen */
    sizeof(prvTgfPacket1PartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacket1PartArray                                        /* partsArray */
};


/* FDB MAC entry */
static PRV_TGF_BRG_MAC_ENTRY_STC prvTgfMacEntry;

/* variables to save configuration */
static GT_U32 savePortMaxBuffLimit;
static GT_U32 savePortMaxDescrLimit;


/**
* @internal prvTgfPortWsRateCheck function
* @endinternal
*
* @brief   Check rate by WS rate of port
*
* @param[in] portIdx                  - index of egress port
* @param[in] rate                     -  to check in packets per second
* @param[in] packetSize               - packet size in bytes including CRC
*
* @retval 0                        - input rate is WS
* @retval other                    - difference between input rate and WS one
*/
GT_U32 prvTgfPortWsRateCheck
(
    IN  GT_U32 portIdx,
    IN  GT_U32 rate,
    IN  GT_U32 packetSize
)
{
    GT_U32      packetRate; /* packet rate in packets per second */
    GT_U32      percent;     /* difference between ports WS rate
                               and input one in percent */

    /* get port's WS rate */
    packetRate = prvTgfCommonPortWsRateGet(portIdx,packetSize);

    /* calculate difference between ports WS rate and input one */
    percent = prvTgfCommonDiffInPercentCalc(packetRate, rate);

    if (percent > prvTgfWsDiffInPercent)
    {
        if (rate > packetRate)
        {
            /* BobK devices may generate rate more then port's speed configuration.
               This is OK for tests. */
            if(GT_FALSE == prvUtfDeviceTestNotSupport(prvTgfDevNum,
                                                      UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E))
            {
                return 0;
            }
        }
        /* big difference */
        return percent;
    }
    else
    {
        return 0;
    }
}

/**
* @internal prvTgfPortTxSpSchedulerStopSend function
* @endinternal
*
* @brief   Function stops sends packets
*/
GT_VOID prvTgfPortTxSpSchedulerStopSend
(
    IN  GT_VOID
)
{
    GT_STATUS       rc         = GT_OK;
    GT_U32          i;
    GT_U32           portNum;

    for (i = 0; i < prvTgfPortTxSpIngrPortsNum; i++)
    {
        portNum = prvTgfPortsArray[inPortsIdxArr[i]];

        /* stop send Packet from port portNum */
        rc = prvTgfStopTransmitingEth(prvTgfDevNum, portNum);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StopTransmitting: %d, %d",
                                     prvTgfDevNum, portNum);

        /* configure default mode */
        rc = prvTgfTxModeSetupEth(prvTgfDevNum, portNum,
                                  PRV_TGF_TX_MODE_SINGLE_BURST_E,
                                  PRV_TGF_TX_CONT_MODE_UNIT_WS_E,
                                  0);

        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfTxModeSetupEth: %d, %d",
                                     prvTgfDevNum, portNum);

    }

    /* wait some time to guaranty that traffic stopped */
    cpssOsTimerWkAfter(10);
}

/**
* @internal prvTgfPortTxSpSchedulerFdbConfigurationSet function
* @endinternal
*
* @brief   Set FDB configuration.
*/
static GT_VOID prvTgfPortTxSpSchedulerFdbConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS                   rc;                 /* return code */

    /* clear entry */
    cpssOsMemSet(&prvTgfMacEntry, 0, sizeof(prvTgfMacEntry));

    prvTgfMacEntry.key.entryType                = PRV_TGF_FDB_ENTRY_TYPE_MAC_ADDR_E;
    prvTgfMacEntry.key.key.macVlan.vlanId       = PRV_TGF_VLANID_CNS;
    prvTgfMacEntry.dstInterface.type            = CPSS_INTERFACE_PORT_E;
    prvTgfMacEntry.dstInterface.devPort.hwDevNum  = prvTgfDevNum;
    prvTgfMacEntry.dstInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_0_CNS];
    prvTgfMacEntry.isStatic                     = GT_TRUE;
    prvTgfMacEntry.daCommand                    = PRV_TGF_PACKET_CMD_FORWARD_E;
    prvTgfMacEntry.saCommand                    = PRV_TGF_PACKET_CMD_FORWARD_E;
    prvTgfMacEntry.daRoute                      = GT_FALSE;
    prvTgfMacEntry.mirrorToRxAnalyzerPortEn     = GT_FALSE;
    prvTgfMacEntry.userDefined                  = 0;
    prvTgfMacEntry.daQosIndex                   = 0;
    prvTgfMacEntry.saQosIndex                   = 0;
    prvTgfMacEntry.daSecurityLevel              = 0;
    prvTgfMacEntry.saSecurityLevel              = 0;
    prvTgfMacEntry.appSpecificCpuCode           = GT_FALSE;
    prvTgfMacEntry.pwId                         = 0;
    prvTgfMacEntry.spUnknown                    = GT_FALSE;
    prvTgfMacEntry.sourceId                     = 1;

    cpssOsMemCpy(prvTgfMacEntry.key.key.macVlan.macAddr.arEther, prvTgfPacket1L2Part.daMac, 6);

    /* AUTODOC: add FDB entry with MAC 00:AA:BB:CC:DD:11, VLAN 223, Port 0 */
    rc = prvTgfBrgFdbMacEntrySet(&prvTgfMacEntry);
    UTF_VERIFY_EQUAL1_STRING_MAC(rc, GT_OK, "Error in prvTgfBrgFdbMacEntrySet %d", GT_TRUE);
}

/**
* @internal prvTgfPortTxSpSchedulerBrgConfigurationSet function
* @endinternal
*
* @brief   Set Bridge configuration.
*/
GT_VOID prvTgfPortTxSpSchedulerBrgConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS rc;

    /* AUTODOC: create VLAN 223 with all ports, tagCmd=TAG0  */
    rc = prvTgfBrgDefVlanEntryWriteWithTaggingCmd(PRV_TGF_VLANID_CNS,
                                                  PRV_TGF_BRG_VLAN_PORT_TAG0_CMD_E);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWriteWithTaggingCmd: %d, %d",
                                 prvTgfDevNum, PRV_TGF_VLANID_CNS);

    prvTgfPortTxSpSchedulerFdbConfigurationSet();
}

/**
* @internal prvTgfPortTxSpSchedulerConfigurationSet function
* @endinternal
*
* @brief   Set configuration.
*/
GT_VOID prvTgfPortTxSpSchedulerConfigurationSet
(
    GT_VOID
)
{
    GT_U32                      i;         /* iterator */
    GT_STATUS                   rc;        /* return code */
    GT_U32                       portNum;   /* port number */
    CPSS_QOS_ENTRY_STC          portQosCfg; /* port QoS configuration */
    PRV_TGF_COS_PROFILE_STC     qosProfile; /* QoS Profile */
    PRV_TGF_DEVICE_TYPE_ENT     devTypeIdx;

    /* AUTODOC: SETUP CONFIGURATION: */
    if(PRV_CPSS_PP_MAC(prvTgfDevNum)->numOfPorts > 63)
    {
        if(GT_FALSE != prvUtfDeviceTestNotSupport(prvTgfDevNum, UTF_CPSS_PP_E_ARCH_CNS))
        {
            devTypeIdx = PRV_TGF_128_PORT_DEVICE_TYPE_E;
        }
        else
        {
            /* Bobcat2 case - use 28 Ports setup to use only same speed ports - tri-speed */
            devTypeIdx = PRV_TGF_28_PORT_DEVICE_TYPE_E;
            switch (PRV_CPSS_PP_MAC(prvTgfDevNum)->devType)
            {
                case CPSS_BOBK_CETUS_DEVICES_CASES_MAC:
                    devTypeIdx = PRV_TGF_PORTS_FROM_56_DEVICE_TYPE_E;
                    break;
                default:
                    break;
            }
        }
    }
    else
    {
        /* calc device type index in port array */
        devTypeIdx = PRV_CPSS_IS_LION_STYLE_MULTI_PORT_GROUPS_DEVICE_MAC(prvTgfDevNum) ? PRV_TGF_64_PORT_DEVICE_TYPE_E :
                                                                          PRV_TGF_28_PORT_DEVICE_TYPE_E;
    }
    /* for bobcat3 need 10 Giga ports for WS*/
    if (PRV_CPSS_SIP_5_20_CHECK_MAC(prvTgfDevNum))
    {

        devTypeIdx = PRV_TGF_PORTS_FROM_56_DEVICE_TYPE_E + 1;
    }

    /* AUTODOC: set test's specific port array with 8 ports */
    rc = prvTgfDefPortsArraySet(&prvTgfPortTxPortsArray[devTypeIdx][0],
                                PRV_TGF_TX_TEST_PORT_NUM);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    /* AUTODOC: enable tail drop by set HOL mode */
    rc = prvTgfPortFcHolSysModeSet(prvTgfDevNum, PRV_TGF_PORT_PORT_HOL_E);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPortFcHolSysModeSet: %d, %d",
                                 prvTgfDevNum, PRV_TGF_PORT_PORT_HOL_E);

    if(GT_FALSE != prvUtfDeviceTestNotSupport(prvTgfDevNum, UTF_LION_E | UTF_LION2_E | UTF_CPSS_PP_E_ARCH_CNS))
    {
        /* disable global resource sharing */
        rc = prvTgfPortTxSharingGlobalResourceEnableSet(prvTgfDevNum, GT_FALSE);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortTxSharingGlobalResourceEnableSet");
    }

    /* get tail drop profile */
    rc = prvTgfPortTxBindPortToDpGet(prvTgfDevNum,
                                     prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_0_CNS],
                                     &prvPortTailDropProfile);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPortTxBindPortToDpGet: %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_0_CNS]);

    /* save profile settings */
    rc = prvTgfPortTxTailDropProfileWithoutAlphaGet(prvTgfDevNum, prvPortTailDropProfile,
                                        &savePortMaxBuffLimit,
                                        &savePortMaxDescrLimit);
    UTF_VERIFY_EQUAL2_STRING_MAC(rc, GT_OK, "prvTgfPortTxTailDropProfileWithoutAlphaGet: %d, %d",
                                 prvTgfDevNum, prvPortTailDropProfile);

    /* AUTODOC: set tail drop profile: */
    /* AUTODOC:   portMaxBuffLimit=200, portMaxDescrLimit=200 */
    rc = prvTgfPortTxTailDropProfileWithoutAlphaSet(prvTgfDevNum, prvPortTailDropProfile,
                                        200, 200);
    UTF_VERIFY_EQUAL4_STRING_MAC(rc, GT_OK, "prvTgfPortTxTailDropProfileWithoutAlphaSet: %d, %d, %d, %d",
                                 prvTgfDevNum, prvPortTailDropProfile,
                                 200, 200);

    /* get scheduler profile */
    rc = prvTgfPortTxSchedulerProfileIndexGet(prvTgfDevNum,
                                              prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_0_CNS],
                                              &prvPortTxProfile);

    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPortTxSchedulerProfileIndexGet: %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_0_CNS]);

    for (i = 0; i < prvTgfPortTxSpIngrPortsNum; i++)
    {
        /* store arbiter configuration */
        rc = prvTgfPortTxSchedulerArbGroupGet(prvTgfDevNum, prvPortTxProfile,
                                              tcArr[i], &(savePortTxArpArr[i]));
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPortTxSchedulerArbGroupGet: %d, %d",
                                     prvTgfDevNum, i);

        /* AUTODOC: set arbitration group to SP_ARB_GROUP for TC [1,2,3,4] */
        rc = prvTgfPortTxSchedulerArbGroupSet(prvTgfDevNum, prvPortTxProfile,
                                              tcArr[i], CPSS_PORT_TX_SP_ARB_GROUP_E);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPortTxSchedulerArbGroupSet: %d, %d",
                                     prvTgfDevNum, i);

        portNum = prvTgfPortsArray[inPortsIdxArr[i]];

        /* AUTODOC: set QoS Trust Mode as NO_TRUST for ports [1,2,3,4] */
        rc = prvTgfCosPortQosTrustModeSet(portNum, CPSS_QOS_PORT_NO_TRUST_E);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfCosPortQosTrustModeSet: %d, %d",
                                     prvTgfDevNum, portNum);

        /* Configure Port QoS attributes */
        portQosCfg.assignPrecedence = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E;
        portQosCfg.enableModifyDscp = CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
        portQosCfg.enableModifyUp   = CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
        portQosCfg.qosProfileId     = qosArr[i];

        /* AUTODOC: config QoS entries [1,17,23,50] for ports [1,2,3,4] with: */
        /* AUTODOC:   assignPrecedenc=SOFT */
        /* AUTODOC:   enableModifyUp=enableModifyDscp=DISABLE */
        rc = prvTgfCosPortQosConfigSet(portNum,
                                       &portQosCfg);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfCosPortQosConfigSet: %d", prvTgfDevNum);

        /* Set packets TC */
        qosProfile.dropPrecedence = 0;
        qosProfile.trafficClass   = tcArr[i];
        qosProfile.dscp           = 0;
        qosProfile.exp            = 0;
        qosProfile.userPriority   = 0;

        /* AUTODOC: config Cos profile entries [1,17,23,50] with: */
        /* AUTODOC:   DP=DSCP=EXP=UP=0, TC=[1,2,3,4] */
        rc = prvTgfCosProfileEntrySet(qosArr[i], &qosProfile);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCosProfileEntrySet");
    }

    prvTgfPortTxSpSchedulerBrgConfigurationSet();
}

/**
* @internal prvTgfPortTxSpSchedulerRestore function
* @endinternal
*
* @brief   None
*/
GT_VOID prvTgfPortTxSpSchedulerRestore
(
    GT_VOID
)
{
    GT_U32                      i;         /* iterator */
    CPSS_QOS_ENTRY_STC          cosEntry;  /* QoS entry */
    PRV_TGF_COS_PROFILE_STC     qosProfile; /* QoS profile */
    GT_STATUS                   rc;         /* return code */
    GT_U32                       portNum;    /* port number */

    /* AUTODOC: RESTORE CONFIGURATION: */
    /* stop send packets */
    prvTgfPortTxSpSchedulerStopSend();

    /* AUTODOC: restore default FC mode */
    rc = prvTgfPortFcHolSysModeSet(prvTgfDevNum, PRV_TGF_PORT_PORT_FC_E);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPortFcHolSysModeSet: %d, %d",
                                 prvTgfDevNum, PRV_TGF_PORT_PORT_FC_E);

    if(GT_FALSE != prvUtfDeviceTestNotSupport(prvTgfDevNum, UTF_LION_E | UTF_LION2_E | UTF_CPSS_PP_E_ARCH_CNS))
    {
        /* enable global resource sharing */
        rc = prvTgfPortTxSharingGlobalResourceEnableSet(prvTgfDevNum, GT_TRUE);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortTxSharingGlobalResourceEnableSet");
    }


    /* AUTODOC: restore default tail drop profile */
    rc = prvTgfPortTxTailDropProfileWithoutAlphaSet(prvTgfDevNum, prvPortTailDropProfile,
                                        savePortMaxBuffLimit, savePortMaxDescrLimit);
    UTF_VERIFY_EQUAL4_STRING_MAC(rc, GT_OK, "prvTgfPortTxTailDropProfileWithoutAlphaSet: %d, %d, %d, %d",
                                 prvTgfDevNum, prvPortTailDropProfile,
                                 savePortMaxBuffLimit, savePortMaxDescrLimit);
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
                                 prvTgfDevNum, PRV_TGF_VLANID_CNS);

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d", prvTgfDevNum, GT_TRUE);

    for (i = 0; i < prvTgfPortTxSpIngrPortsNum; i++)
    {
        /* AUTODOC: restore arbitration group configuration */
        rc = prvTgfPortTxSchedulerArbGroupSet(prvTgfDevNum, prvPortTxProfile, tcArr[i], savePortTxArpArr[i]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPortTxSchedulerArbGroupSet: %d, %d",
                                     prvTgfDevNum, i);

        portNum = prvTgfPortsArray[inPortsIdxArr[i]];

        /* AUTODOC: restore default QoS Trust Mode for all ports */
        rc = prvTgfCosPortQosTrustModeSet(portNum, CPSS_QOS_PORT_TRUST_L2_L3_E);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfCosPortQosTrustModeSet: %d, %d",
                                     prvTgfDevNum, portNum);

        /* map send port to Qos Profile0 */
        cosEntry.qosProfileId     = 0;
        cosEntry.enableModifyDscp = CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
        cosEntry.enableModifyUp   = CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
        cosEntry.assignPrecedence = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E;

        /* AUTODOC: restore default QoS entries [1,17,23,50] */
        rc = prvTgfCosPortQosConfigSet(portNum, &cosEntry);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCosPortQosConfigSet");

        /* Reset packets TC */
        qosProfile.dropPrecedence = 0;
        qosProfile.trafficClass   = 0;
        qosProfile.dscp           = 0;
        qosProfile.exp            = 0;
        qosProfile.userPriority   = 0;

        /* AUTODOC: restore default Cos profile entries [1,17,23,50] */
        rc = prvTgfCosProfileEntrySet(qosArr[i], &qosProfile);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCosProfileEntrySet");
    }

    /* restore port configuration according to device family */
    prvTgfPortsArrayByDevTypeSet();
}

/**
* @internal prvTgfPortTxSpSchedulerPacketSend function
* @endinternal
*
* @brief   Function sends packet.
*/
static GT_VOID prvTgfPortTxSpSchedulerPacketSend
(
    OUT GT_U32  rateArr[PRV_TGF_MAX_INGR_PORTS_NUM_CNS]
)
{
    GT_STATUS       rc;                          /* return code*/
    GT_U32          portsCount = prvTgfPortsNum; /* number of used ports */
    GT_U32          portIter;                    /* iterator */
    TGF_PACKET_STC  packetInfo;                  /* packet info */
    GT_U32          i;                           /* iterator */
    GT_U32           portNum;                     /* port number */
    GT_U32          tmp;                         /* variable */

    /* Send packet Vlan Tagged */
    tgfTrafficGeneratorEtherTypeForVlanTagSet(TGF_ETHERTYPE_8100_VLAN_TAG_CNS);

    /* reset counters */
    for (portIter = 0; portIter < portsCount; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }
    packetInfo = prvTgfPacket1Info;
    for (i = 0; i < prvTgfPortTxSpIngrPortsNum; i++)
    {
        prvTgfPacket1L2Part.saMac[5] = (GT_U8)i;

        /* change length of packet if need.
           all ports has same packet length so far. */
        packetInfo.totalLen = prvTgfPacket1Info.totalLen;

        portNum = prvTgfPortsArray[inPortsIdxArr[i]];

        /* setup Packet */
        rc = prvTgfSetTxSetupEth(prvTgfDevNum, &packetInfo, prvTgfBurstCount, 0, NULL);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "prvTgfSetTxSetupEth: %d, %d",
                                     prvTgfDevNum, prvTgfBurstCount, 0, NULL);

        /* configure WS mode */
        rc = prvTgfTxModeSetupEth(prvTgfDevNum, portNum,
                                  PRV_TGF_TX_MODE_CONTINUOUS_E,
                                  PRV_TGF_TX_CONT_MODE_UNIT_WS_E,
                                  0);

        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfTxModeSetupEth: %d, %d",
                                     prvTgfDevNum, portNum);

        /* AUTODOC: send continuous wirespeed traffic from port 1 with: */
        /* AUTODOC:   DA=00:AA:BB:CC:DD:11, SA=00:88:99:33:44:00, VID=223 */
        /* AUTODOC: send continuous wirespeed traffic from port 2 with: */
        /* AUTODOC:   DA=00:AA:BB:CC:DD:11, SA=00:88:99:33:44:01, VID=223 */
        /* AUTODOC: send continuous wirespeed traffic from port 3 with: */
        /* AUTODOC:   DA=00:AA:BB:CC:DD:11, SA=00:88:99:33:44:02, VID=223 */
        /* AUTODOC: send continuous wirespeed traffic from port 4 with: */
        /* AUTODOC:   DA=00:AA:BB:CC:DD:11, SA=00:88:99:33:44:03, VID=223 */
        rc = prvTgfStartTransmitingEth(prvTgfDevNum, portNum);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, %d",
                                     prvTgfDevNum, portNum);
    }

    /* AUTODOC: get Rx rate by MAC MIB counters and check WS rate */
    prvTgfCommonPortsRxRateGet(inPortsIdxArr, prvTgfPortTxSpIngrPortsNum, 1000, rateArr);
    for (i = 0; i < prvTgfPortTxSpIngrPortsNum; i++)
    {
        tmp = prvTgfPortWsRateCheck(inPortsIdxArr[i], rateArr[i], PRV_TGF_PACKET_CRC_LEN_CNS);
        if (tmp)
        {
            /* the rate is not good .
               start again */
            PRV_UTF_LOG2_MAC("[TxSpScheduler]: wrong ingress port %d rate %d\n", prvTgfPortsArray[inPortsIdxArr[i]], rateArr[i]);

            packetInfo.totalLen = prvTgfPacket1Info.totalLen;
            prvTgfPacket1L2Part.saMac[5] = (GT_U8)i;

            portNum = prvTgfPortsArray[inPortsIdxArr[i]];

            /* setup Packet */
            rc = prvTgfSetTxSetupEth(prvTgfDevNum, &packetInfo, prvTgfBurstCount, 0, NULL);
            UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "prvTgfSetTxSetupEth: %d, %d",
                                         prvTgfDevNum, prvTgfBurstCount, 0, NULL);

            /* send Packet from port portNum */
            rc = prvTgfStartTransmitingEth(prvTgfDevNum, portNum);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, %d",
                                         prvTgfDevNum, portNum);
        }
    }
    /* get final rate values */
    prvTgfCommonPortsRxRateGet(inPortsIdxArr, prvTgfPortTxSpIngrPortsNum, 1000, rateArr);
}

static GT_U32 prvTgfPortTxSpExpectedDropRateGet
(
    IN GT_U32 txRate,
    IN GT_U32 rxRate
)
{
    /* SIP5 devices 10G ports may generate rate more then port's speed configuration.
       This is OK for tests. */
    if(GT_FALSE == prvUtfDeviceTestNotSupport(prvTgfDevNum,
                                              UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E))
    {
        if (rxRate < 1600000)
        {
            /* 1G/100M/10M ports */
            return 0;
        }

        return (rxRate > txRate)? (rxRate - txRate) : 0;
    }
    else
    {
        /* other devices should have 0 drop rate of highest priority */
        return 0;
    }
}

/**
* @internal prvTgfPortTxSpSchedulerTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic and test results.
*/
GT_VOID prvTgfPortTxSpSchedulerTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS    rc;      /* return code */
    GT_U32       txRate;  /* Tx rate */
    GT_U32       tmp;     /* variable */
    GT_U32       ii, kk;  /* iterators */
    GT_U32       rateOk;  /* is rate OK */
    GT_U32        portNum; /* port number */
    GT_U32       rateArr[PRV_TGF_MAX_INGR_PORTS_NUM_CNS]; /* array of rates */
    GT_U32       expectRate[PRV_TGF_MAX_INGR_PORTS_NUM_CNS]; /* array of expected rates */
    GT_U32       actualRate[PRV_TGF_MAX_INGR_PORTS_NUM_CNS]; /* array of actual rates */
    GT_U32       expectDropRate[PRV_TGF_MAX_INGR_PORTS_NUM_CNS]; /* array of expected drop rates */
    GT_U32       actualDropRate[PRV_TGF_MAX_INGR_PORTS_NUM_CNS]; /* array of actual drop rates */
    GT_U32       expectHighPriDropRate; /* expected drop rate of highest priority */
    GT_U32       allowWsDiffInPercent; /* allowed difference between actual and expected rates */

    allowWsDiffInPercent = prvTgfWsDiffInPercent;
    if (prvTgfPortEgressCntIsCaelumErratumExist(prvTgfDevNum))
    {
        /* The test use CNC for rate measure for Cetus/Caelum A0 devices.
           CNC rate may have more deviation from port MAC one. */
        allowWsDiffInPercent = 10;
    }
    PRV_UTF_LOG1_MAC("Rate's comparison allowed deviation is %d percent\n", allowWsDiffInPercent);

    /* AUTODOC: GENERATE TRAFFIC: */

    /* send packets */
    prvTgfPortTxSpSchedulerPacketSend(rateArr);

    /* wait to get stable traffic */
    cpssOsTimerWkAfter(10);

    /* AUTODOC: verify that egress port get WS traffic */
    prvTgfCommonPortTxRateGet(PRV_TGF_EGR_PORT_IDX_0_CNS, 1000, &txRate);
    tmp = prvTgfPortWsRateCheck(PRV_TGF_EGR_PORT_IDX_0_CNS, txRate, PRV_TGF_PACKET_CRC_LEN_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(0, tmp, "wrong egress rate in WS percent %d, port %d",
                                     tmp, prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_0_CNS]);

    PRV_UTF_LOG0_MAC("\n");
    for (ii = 0; ii < prvTgfPortTxSpIngrPortsNum; ii++)
    {
        PRV_UTF_LOG2_MAC("Ingress port %d rate %d\n", prvTgfPortsArray[inPortsIdxArr[ii]], rateArr[ii]);
    }
    PRV_UTF_LOG2_MAC("Egress port %d rate %d\n", prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_0_CNS], txRate);

    /* provide time for print-outs before rate checks */
    cpssOsTimerWkAfter(200);

    /* index of ingress ports is priority
       maximal index take 100% of traffic */
    for (ii = 0; ii < (prvTgfPortTxSpIngrPortsNum -1); ii++)
    {
        expectRate[ii] = 0;
        expectDropRate[ii] = rateArr[ii];
    }

    /* highest priority */
    expectHighPriDropRate = prvTgfPortTxSpExpectedDropRateGet(txRate,rateArr[ii]);
    expectRate[ii] = txRate;
    expectDropRate[ii] = expectHighPriDropRate;

    for (ii = 0; ii < prvTgfPortTxSpIngrPortsNum; ii++)
    {
        PORT_TX_PRV_UTF_LOG1_MAC("[TxSpScheduler]: iteration %d\n", ii);
        /* AUTODOC: get rate for TC [1,2] by egress counters */
        prvTgfCommonPortTxTwoUcTcRateGet(PRV_TGF_EGR_PORT_IDX_0_CNS, tcArr[0], tcArr[1],
                                 1000, &actualRate[0], &actualRate[1],
                                 &actualDropRate[0], &actualDropRate[1]);
        /* AUTODOC: get rate for TC [3,4] by egress counters */
        prvTgfCommonPortTxTwoUcTcRateGet(PRV_TGF_EGR_PORT_IDX_0_CNS, tcArr[2], tcArr[3],
                                 1000, &actualRate[2], &actualRate[3],
                                 &actualDropRate[2], &actualDropRate[3]);

        if (prvTgfPortTxSpIngrPortsNum > 4)
        {
            prvTgfCommonPortTxTwoUcTcRateGet(PRV_TGF_EGR_PORT_IDX_0_CNS, tcArr[4], tcArr[5],
                                     1000, &actualRate[4], &actualRate[5],
                                     &actualDropRate[4], &actualDropRate[5]);
            if (prvTgfPortTxSpIngrPortsNum > 6)
            {
                prvTgfCommonPortTxTwoUcTcRateGet(PRV_TGF_EGR_PORT_IDX_0_CNS, tcArr[6], tcArr[6],
                                         1000, &actualRate[6], &actualRate[6],
                                         &actualDropRate[6], &actualDropRate[6]);
            }
        }

        /* AUTODOC: compare rates with expected rate (with permitted accuracy) */
        for (kk = 0; kk < prvTgfPortTxSpIngrPortsNum; kk++)
        {
            PORT_TX_PRV_UTF_LOG2_MAC("[TxSpScheduler]: compare rate %d drop %d\n", actualRate[kk], actualDropRate[kk]);
            if (expectRate[kk])
            {
                /* the TC counter rate is not exact.
                  get percentage of difference and compare with permitted accuracy.*/
                tmp = prvTgfCommonDiffInPercentCalc(expectRate[kk],actualRate[kk]);
                rateOk = (tmp <= allowWsDiffInPercent)? 1 : 0;
                UTF_VERIFY_EQUAL4_STRING_MAC(1, rateOk," wrong rate, iter %d idx %d expected %d actual %d\n",
                                             ii, kk, expectRate[kk],actualRate[kk]);
            }
            else
            {
                UTF_VERIFY_EQUAL1_STRING_MAC(0,actualRate[kk]," wrong zero rate, idx %d\n", kk);
            }

            if (expectDropRate[kk])
            {
                /* the TC counter rate is not exact.
                  get percentage of difference and compare with permitted accuracy.*/
                tmp = prvTgfCommonDiffInPercentCalc(expectDropRate[kk],actualDropRate[kk]);
                rateOk = (tmp <= allowWsDiffInPercent)? 1 : 0;
                UTF_VERIFY_EQUAL4_STRING_MAC(1, rateOk," wrong drop rate, iter %d idx %d expected %d actual %d\n",
                                             ii, kk, expectDropRate[kk],actualDropRate[kk]);
            }
            else
            {
                UTF_VERIFY_EQUAL2_STRING_MAC(0,actualDropRate[kk]," wrong zero drop rate, iter %d idx %d\n", ii, kk);
            }
        }

        /* disable highest priority traffic */
        tmp = prvTgfPortTxSpIngrPortsNum - 1 - ii;
        portNum = prvTgfPortsArray[inPortsIdxArr[tmp]];

        if (prvTgfPortTxSpDebug == 0)
        {
            /* stop send Packet from port portNum */
            rc = prvTgfStopTransmitingEth(prvTgfDevNum, portNum);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StopTransmitting: %d, %d",
                                         prvTgfDevNum, portNum);

            /* wait to stop traffic */
             cpssOsTimerWkAfter(10);
        }
        /* correct expected results */

        if (tmp)
        {
            /* change highest priotity traffic */
            expectRate[tmp-1] = txRate;
            expectRate[tmp] = 0;

            expectHighPriDropRate = prvTgfPortTxSpExpectedDropRateGet(txRate,rateArr[tmp-1]);
            expectDropRate[tmp-1] = expectHighPriDropRate;
            expectDropRate[tmp] = 0;
        }
    }

}


GT_VOID prvTgfPortTxTcRateDump
(
    IN  GT_U32 portIdx,
    IN  GT_U32 tc1,
    IN  GT_U32 tc2,
    IN  GT_U32 timeOut
)
{
    GT_U32 outRate1;
    GT_U32 outRate2;
    GT_U32 dropRate1;
    GT_U32 dropRate2;
    GT_U32 portRate;

    /* get rates */
    prvTgfCommonPortTxTwoUcTcRateGet( portIdx, tc1, tc2, timeOut,
                           &outRate1, &outRate2, &dropRate1, &dropRate2);

    prvTgfCommonPortTxRateGet(portIdx, timeOut,&portRate);

    /* print rates */
    PRV_UTF_LOG4_MAC("Rate Dump: port %d  TC1=%d  TC2=%d  timeout=%d\n",
                     prvTgfPortsArray[portIdx], tc1, tc2, timeOut);
    PRV_UTF_LOG3_MAC("OUT  Rate: %10d %10d %10d\n",
                     outRate1, outRate2, outRate1 + outRate2);
    PRV_UTF_LOG3_MAC("Drop Rate: %10d %10d %10d\n",
                     dropRate1, dropRate2, dropRate1 + dropRate2);

    PRV_UTF_LOG1_MAC("Port Rate: %10d\n", portRate);


}

GT_VOID prvTgfPortsRxRateDump
(
    IN  GT_U32 timeOut
)
{
    GT_U32 portIdx;
    GT_U32 rxRate;

    PRV_UTF_LOG1_MAC("Rx Rate Dump: timeout=%d\n", timeOut);
    for (portIdx = 0; portIdx < PRV_TGF_TX_TEST_PORT_NUM; portIdx++)
    {
        prvTgfCommonPortRxRateGet(portIdx, timeOut, &rxRate);
        PRV_UTF_LOG2_MAC(" port %3d  Rx rate=%10d\n",
                         prvTgfPortsArray[portIdx], rxRate);
    }
}

GT_VOID prvTgfPortsRatesDump
(
    IN  GT_U32 timeOut
)
{
    GT_U32 portIdx;
    GT_U32 rxRate;
    GT_U32 txRate;

    PRV_UTF_LOG1_MAC("Rx and Tx Rates Dump: timeout=%d\n", timeOut);
    for (portIdx = 0; portIdx < PRV_TGF_TX_TEST_PORT_NUM; portIdx++)
    {
        prvTgfCommonPortRxRateGet(portIdx, timeOut, &rxRate);
        prvTgfCommonPortTxRateGet(portIdx, timeOut, &txRate);
        PRV_UTF_LOG3_MAC(" port %3d Rx rate=%10d Tx rate=%10d\n",
                         prvTgfPortsArray[portIdx], rxRate, txRate);
    }
}

GT_VOID prvTgfNumOfPortsRatesDump
(
    IN  GT_U32 timeOut
)
{
    GT_U32 portIdxArr[]={1,2,3,4,5,6,7};
    GT_U32 numOfPorts = PRV_TGF_MAX_INGR_PORTS_NUM_CNS;
    GT_U32 rxRatesArr[8];
    GT_U32 portIdx;

    PRV_UTF_LOG1_MAC("Rx Rates Dump: timeout=%d\n", timeOut);

    /* get Rx rates */
    prvTgfCommonPortsRxRateGet(portIdxArr, numOfPorts, timeOut, rxRatesArr);
    for (portIdx = 0; portIdx < prvTgfPortTxSpIngrPortsNum; portIdx++)
    {
        PRV_UTF_LOG2_MAC(" port %3d Rx rate=%10d \n",
                         prvTgfPortsArray[portIdxArr[portIdx]], rxRatesArr[portIdx]);
    }
}

GT_VOID prvTgfTxStart(GT_U8 devNum, GT_U8 portNum, GT_U32 addBytes)
{

    GT_STATUS           rc;                  /* return code*/
    TGF_PACKET_STC      packetInfo;          /* packet info */

    packetInfo = prvTgfPacket1Info;
    prvTgfPacket1L2Part.saMac[5] = portNum;
    packetInfo.totalLen = prvTgfPacket1Info.totalLen + addBytes;

    /* setup Packet */
    rc = prvTgfSetTxSetupEth(devNum, &packetInfo, prvTgfBurstCount, 0, NULL);
    if (rc != GT_OK)
    {
        PRV_UTF_LOG2_MAC("prvTgfSetTxSetupEth: rc %d, dev %d\n", rc, devNum);
    }

    /* send Packet from port portNum */
    rc = prvTgfStartTransmitingEth(devNum, portNum);
    if (rc != GT_OK)
    {
        PRV_UTF_LOG2_MAC("ERROR of StartTransmitting: rc %d, dev %d\n", rc, devNum);
    }
}

CPSS_PORT_MAC_COUNTERS_ENT prvTgfPortRxCntrName = CPSS_GOOD_UC_PKTS_RCV_E;
CPSS_PORT_MAC_COUNTERS_ENT prvTgfPortTxCntrName = CPSS_GOOD_UC_PKTS_SENT_E;


GT_VOID prvTgfDevPortsMcRatesSet(GT_VOID)
{
    prvTgfPortRxCntrName = CPSS_MC_PKTS_RCV_E;
    prvTgfPortTxCntrName = CPSS_MC_PKTS_SENT_E;
}

GT_VOID prvTgfDevPortsRatesDump
(
    IN  GT_U8 devNum
)
{
    GT_U64      cntrValue;  /* value of counters */
    GT_PHYSICAL_PORT_NUM       portNum;              /* port number */
    GT_STATUS   rc;                   /* return code */
    GT_U32      startTimeStampRxArr[CPSS_MAX_PORTS_NUM_CNS]; /* VNT time stamp start for RX */
    GT_U32      startTimeStampTxArr[CPSS_MAX_PORTS_NUM_CNS]; /* VNT time stamp start for TX */
    GT_U32      endTimeStamp;   /* VNT time stamp start */
    GT_U32      rxRate;         /* VNT time stamp based RX rate */
    GT_U32      txRate;         /* VNT time stamp based TX rate */
    GT_U32      dumpCount = 0; /* number of ports for dump */
    GT_UL64     summaryRxRatePps = 0;  /* summary RX rate in PPS */
    GT_UL64     summaryTxRatePps = 0;  /* summary TX rate in PPS */
    GT_U32      summaryRxRateMbps;     /* summary RX rate in MBPS */
    GT_U32      summaryTxRateMbps;     /* summary TX rate in MBPS */
    GT_U32      packetSize = 64;  /* L2 packet size for bits rate calculation including FCS */

    rc = prvUtfNextMacPortReset(&portNum, devNum);
    if (rc != GT_OK)
    {
        PRV_UTF_LOG0_MAC("prvUtfNextMacPortReset failure 1 \n");
        return;
    }

    /* For device go over all available physical ports. */
    while(GT_OK == prvUtfNextMacPortGet(&portNum, GT_TRUE))
    {
        if (portNum >= (CPSS_MAX_PORTS_NUM_CNS-1))
        {
            PRV_UTF_LOG1_MAC("port too big  %d\n", portNum);
            return;
        }

        rc = prvTgfPortMacCounterGet(devNum, portNum,
                                      prvTgfPortRxCntrName,
                                      &cntrValue);
        if (rc != GT_OK)
        {
            PRV_UTF_LOG2_MAC("prvTgfPortMacCounterGet failure RX: dev %d port %d\n", devNum, portNum);
            return;
        }

        rc = prvTgfVntPortLastReadTimeStampGet(devNum, portNum, &startTimeStampRxArr[portNum]);
        if (rc != GT_OK)
        {
            PRV_UTF_LOG2_MAC("prvTgfPortMacCounterGet failure RX: dev %d port %d\n", devNum, portNum);
            return;
        }

        rc = prvTgfPortMacCounterGet(devNum, portNum,
                                      prvTgfPortTxCntrName,
                                      &cntrValue);
        if (rc != GT_OK)
        {
            PRV_UTF_LOG2_MAC("prvTgfPortMacCounterGet failure TX: dev %d port %d\n", devNum, portNum);
            return;
        }

        rc = prvTgfVntPortLastReadTimeStampGet(devNum, portNum, &startTimeStampTxArr[portNum]);
        if (rc != GT_OK)
        {
            PRV_UTF_LOG2_MAC("prvTgfPortMacCounterGet failure TX: dev %d port %d\n", devNum, portNum);
            return;
        }
    }

    /* sleep */
    cpssOsTimerWkAfter(1000);

    /* get counter */
    rc = prvUtfNextMacPortReset(&portNum, devNum);
    if (rc != GT_OK)
    {
        PRV_UTF_LOG0_MAC("prvUtfNextMacPortReset failure 2 \n");
        return;
    }

    /* For device go over all available physical ports. */
    while(GT_OK == prvUtfNextMacPortGet(&portNum, GT_TRUE))
    {
        if (portNum >= (CPSS_MAX_PORTS_NUM_CNS-1))
        {
            PRV_UTF_LOG1_MAC("port too big 2  %d\n", portNum);
            return;
        }

        rc =  prvTgfPortMacCounterGet(devNum, portNum,
                                      prvTgfPortRxCntrName, &cntrValue);
        if (rc != GT_OK)
        {
            PRV_UTF_LOG2_MAC("prvTgfPortMacCounterGet failure RX: dev %d port %d\n", devNum, portNum);
            return;
        }

        rc = prvTgfVntPortLastReadTimeStampGet(devNum, portNum, &endTimeStamp);
        if (rc != GT_OK)
        {
            PRV_UTF_LOG2_MAC("prvTgfPortMacCounterGet failure RX: dev %d port %d\n", devNum, portNum);
            return;
        }

        rxRate = prvTgfCommonVntTimeStampsRateGet(devNum, startTimeStampRxArr[portNum], endTimeStamp, cntrValue.l[0]);


        rc =  prvTgfPortMacCounterGet(devNum, portNum,
                                      prvTgfPortTxCntrName, &cntrValue);
        if (rc != GT_OK)
        {
            PRV_UTF_LOG2_MAC("prvTgfPortMacCounterGet failure TX: dev %d port %d\n", devNum, portNum);
            return;
        }

        rc = prvTgfVntPortLastReadTimeStampGet(devNum, portNum, &endTimeStamp);
        if (rc != GT_OK)
        {
            PRV_UTF_LOG2_MAC("prvTgfPortMacCounterGet failure TX: dev %d port %d\n", devNum, portNum);
            return;
        }

        txRate = prvTgfCommonVntTimeStampsRateGet(devNum, startTimeStampTxArr[portNum], endTimeStamp, cntrValue.l[0]);

        if (rxRate || txRate)
        {
            dumpCount++;
            PRV_UTF_LOG4_MAC(" dev %d port %2d RX Rate %9d TX Rate %9d\n", devNum, portNum, rxRate, txRate);
            summaryRxRatePps += rxRate;
            summaryTxRatePps += txRate;
        }
    }

    summaryRxRateMbps = (GT_U32)(summaryRxRatePps * (packetSize + 20) * 8 / 1000000);
    summaryTxRateMbps = (GT_U32)(summaryTxRatePps * (packetSize + 20) * 8 / 1000000);

    PRV_UTF_LOG1_MAC(" Ports number: %d\n", dumpCount);
    PRV_UTF_LOG2_MAC("== Summary RX rate in PPS %u in MBitsPS %u\n", summaryRxRatePps, summaryRxRateMbps);
    PRV_UTF_LOG2_MAC("== Summary TX rate in PPS %u in MBitsPS %u\n", summaryTxRatePps, summaryTxRateMbps);
}

GT_VOID prvTgfDevPortsDescrDump
(
    IN  GT_U8 devNum
)
{
    GT_PHYSICAL_PORT_NUM       portNum;              /* port number */
    GT_STATUS   rc;                   /* return code */
    GT_U32      dumpCount = 0; /* number of ports for dump */
    GT_U16      descrNum;

    rc = prvUtfNextMacPortReset(&portNum, devNum);
    if (rc != GT_OK)
    {
        PRV_UTF_LOG0_MAC("prvUtfNextMacPortReset failure 1 \n");
        return;
    }

    /* For device go over all available physical ports. */
    while(GT_OK == prvUtfNextMacPortGet(&portNum, GT_TRUE))
    {
        if (portNum >= (CPSS_MAX_PORTS_NUM_CNS-1))
        {
            PRV_UTF_LOG1_MAC("port too big  %d\n", portNum);
            return;
        }

        rc = cpssDxChPortTxDescNumberGet(devNum, portNum, &descrNum);
        if (rc != GT_OK)
        {
            PRV_UTF_LOG2_MAC("cpssDxChPortTxDescNumberGet failure: dev %d port %d\n", devNum, portNum);
            return;
        }

        if (descrNum)
        {
            dumpCount++;
            PRV_UTF_LOG3_MAC(" dev %d port %2d TX Desc %5d\n", devNum, portNum, descrNum);
        }
    }

    PRV_UTF_LOG1_MAC(" Ports number: %d\n", dumpCount);
}

GT_VOID prvTgfDevPortsRxBuffDump
(
    IN  GT_U8 devNum
)
{
    GT_PHYSICAL_PORT_NUM       portNum;              /* port number */
    GT_STATUS   rc;                   /* return code */
    GT_U32      dumpCount = 0; /* number of ports for dump */
    GT_U32      rxBuffsNum;
    GT_U32      rxBuffsSum = 0;

    rc = prvUtfNextMacPortReset(&portNum, devNum);
    if (rc != GT_OK)
    {
        PRV_UTF_LOG0_MAC("prvUtfNextMacPortReset failure 1 \n");
        return;
    }

    /* For device go over all available physical ports. */
    while(GT_OK == prvUtfNextMacPortGet(&portNum, GT_TRUE))
    {
        if (portNum >= (CPSS_MAX_PORTS_NUM_CNS-1))
        {
            PRV_UTF_LOG1_MAC("port too big  %d\n", portNum);
            return;
        }

        rc = cpssDxChPortRxBufNumberGet(devNum, portNum, &rxBuffsNum);
        if (rc != GT_OK)
        {
            PRV_UTF_LOG2_MAC("cpssDxChPortRxBufNumberGet failure: dev %d port %d\n", devNum, portNum);
            return;
        }

        if (rxBuffsNum)
        {
            rxBuffsSum += rxBuffsNum;
            dumpCount++;
            PRV_UTF_LOG3_MAC(" dev %d port %2d RX Buffers %5d\n", devNum, portNum, rxBuffsNum);
        }
    }

    PRV_UTF_LOG1_MAC(" Ports number: %d\n", dumpCount);
    PRV_UTF_LOG1_MAC(" Summary of all ports: %d\n", rxBuffsSum);

    rc = cpssDxChPortGlobalPacketNumberGet(devNum, &rxBuffsNum);
    if (rc != GT_OK)
    {
        PRV_UTF_LOG1_MAC("cpssDxChPortGlobalPacketNumberGet failure: dev %d\n", devNum);
        return;
    }

    PRV_UTF_LOG1_MAC(" Number of Packets in MPPM: %d\n", rxBuffsNum);
}

GT_VOID prvTgfDevPortUpdateVlan
(
    IN GT_U8 devNum,
    IN GT_U16 vlanId,
    IN GT_U32 port
)
{
    GT_STATUS                   rc        = GT_OK;
    CPSS_PORTS_BMP_STC          portsMembers;
    CPSS_PORTS_BMP_STC          portsTagging;
    PRV_TGF_BRG_VLAN_INFO_STC   vlanInfo;
    PRV_TGF_BRG_VLAN_PORTS_TAG_CMD_STC portsTaggingCmd;

    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsMembers);
    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsTagging);

    cpssOsMemSet(
        &vlanInfo, 0, sizeof(PRV_TGF_BRG_VLAN_INFO_STC));
    /* clear portsTaggingCmd */
    cpssOsMemSet(
        &portsTaggingCmd, 0, sizeof(PRV_TGF_BRG_VLAN_PORTS_TAG_CMD_STC));

    /* Create VLAN with two ports */
    CPSS_PORTS_BMP_PORT_SET_MAC(&portsMembers, 0);
    portsTaggingCmd.portsCmd[0] = PRV_TGF_BRG_VLAN_PORT_UNTAGGED_CMD_E;
    CPSS_PORTS_BMP_PORT_SET_MAC(&portsMembers, port);
    portsTaggingCmd.portsCmd[port] = PRV_TGF_BRG_VLAN_PORT_UNTAGGED_CMD_E;

    rc = prvTgfBrgVlanEntryWrite(devNum,
                                 vlanId,
                                 &portsMembers,
                                 &portsTagging,
                                 &vlanInfo,
                                 &portsTaggingCmd);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEntryWrite: %d", vlanId);
}

/**
* @internal prvTgfPortTxMapPhy2TxQCheck function
* @endinternal
*
* @brief   Map physical and TxQ ports and test results.
*/
GT_VOID prvTgfPortTxMapPhy2TxQCheck
(
    IN  GT_VOID
)
{
    static CPSS_DXCH_PORT_MAP_STC  portMapArray[] =
    {
        {0,  CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,   0,        0,    0,   GT_FALSE,   GT_NA}
       ,{12, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,   0,       12,    1,   GT_FALSE,   GT_NA}
       ,{63, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E,       0,    GT_NA,   95,   GT_FALSE,   GT_NA}
    };

    GT_U8 devNum = prvTgfDevNum;
    GT_STATUS rc;
    GT_U32 txPort;
    GT_U32 dq;
    GT_U32 mac;
    CPSS_PORTS_BMP_STC portsBmp;
    CPSS_DXCH_DETAILED_PORT_MAP_STC portMap;
    CPSS_PORT_MAC_COUNTER_SET_STC portCntrs;
    GT_U32 portNum;
    GT_U32 regAddr;
    CPSS_PORT_SPEED_ENT            speed;
    CPSS_PORT_INTERFACE_MODE_ENT   ifMode;

    typedef struct
    {
        CPSS_PORT_SPEED_ENT            speed;
        CPSS_PORT_INTERFACE_MODE_ENT   ifMode;
    } CPSS_PORT_INTERFACE_STC;

    CPSS_PORT_INTERFACE_STC portIfData[256];

    cpssOsMemSet(
        &portIfData, 0, 256 * sizeof(CPSS_PORT_INTERFACE_STC));

    rc = prvUtfNextMacPortReset(&portNum, devNum);
    if (rc != GT_OK)
    {
        PRV_UTF_LOG0_MAC("prvUtfNextMacPortReset failure 1 \n");
        return;
    }

    /* AUTODOC: 1.1. For all active devices go over all available physical ports */
    while(GT_OK == prvUtfNextMacPortGet(&portNum, GT_TRUE))
    {
        rc = cpssDxChPortPhysicalPortDetailedMapGet(devNum, portNum, &portMap);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                     "cpssDxChPortPhysicalPortDetailedMapGet");

        portIfData[portNum].ifMode = CPSS_PORT_INTERFACE_MODE_NA_E;
        portIfData[portNum].speed = CPSS_PORT_SPEED_NA_E;

        if (portMap.valid == GT_FALSE)
        {
            continue;
        }

        if (portMap.portMap.mappingType != CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E)
        {
            rc = cpssDxChPortSpeedGet(devNum, portNum,/*OUT*/&speed);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChPortSpeedGet");

            rc = cpssDxChPortInterfaceModeGet(devNum, portNum,/*OUT*/&ifMode);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChPortInterfaceModeGet");

            if (ifMode != CPSS_PORT_INTERFACE_MODE_NA_E)
            {
                CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsBmp);
                CPSS_PORTS_BMP_PORT_SET_MAC(&portsBmp, portNum);

                /* AUTODOC: Disable physical port */
                rc = cpssDxChPortModeSpeedSet(devNum, &portsBmp, GT_FALSE,
                                              ifMode, speed);
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                             "tgfTrafficGeneratorPortForceLinkUpEnableSet FAIL : cpssDxChPortModeSpeedSet port[%d], power up [GT_FALSE]\n",
                                              portNum);

                portIfData[portNum].ifMode = ifMode;
                portIfData[portNum].speed = speed;
            }
        }
    }

    /* AUTODOC: 1.2 Enable physical port[0] */
    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsBmp);
    CPSS_PORTS_BMP_PORT_SET_MAC(&portsBmp, portMapArray[0].physicalPortNumber);
    rc = cpssDxChPortModeSpeedSet(devNum, &portsBmp, GT_TRUE,
                                  CPSS_PORT_INTERFACE_MODE_KR_E,
                                  CPSS_PORT_SPEED_10000_E);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
                                 "tgfTrafficGeneratorPortForceLinkUpEnableSet FAIL : cpssDxChPortModeSpeedSet port[%d],newIfMode[%d],newSpeed[%d]\n",
                                 portMapArray[1].interfaceNum, 0, 0);

    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        /* no TXQ-DQ of sip5 devices */
    }
    else
    {
        /* AUTODOC: 2.1 Iterate over all DQ units for all TxQ ports */
        for (dq = 0; dq < 6; dq++)
        {
            mac = ((dq + 1) * 12) - 1;
            portMapArray[1].physicalPortNumber = mac;
            portMapArray[1].interfaceNum = portMapArray[1].physicalPortNumber;

            /* AUTODOC: 2.2 Update VLAN with ports to send/recieve traffic  */
            prvTgfDevPortUpdateVlan(devNum, PRV_TGF_VLANID_CNS, portMapArray[1].physicalPortNumber);

            /* Disable CPU TxQ port on DQ > 0 */
            if (dq != 0)
            {
                regAddr = PRV_DXCH_REG1_UNIT_TXQ_DQ_INDEX_MAC(devNum, dq).global.globalDQConfig.globalDequeueConfig;
                rc = prvCpssHwPpSetRegField(devNum, regAddr, 0, 1, 0);
                if (rc != GT_OK)
                {
                    PRV_UTF_LOG1_MAC("Disable CPU TxQ port on DQ[%d] failure\n", dq);
                    return;
                }
            }

            for (txPort = 0; txPort < 96; txPort++)
            {
                portMapArray[1].txqPortNumber = txPort + dq * 96;

                /* Skip port 0 */
                if (dq == 0 && portMapArray[1].txqPortNumber == 0)
                {
                    continue;
                }
                /* Skip CPU port */
                if (portMapArray[1].txqPortNumber == 95)
                {
                    continue;
                }

                /* AUTODOC: 2.3 Map current physical port to TxQ port */
                rc = cpssDxChPortPhysicalPortMapSet(devNum, 3, portMapArray);
                UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc,
                                                "FAIL : cpssDxChPortPhysicalPortMapSet(%d,%d,%d) - rc[%d]\n",
                                                devNum, portMapArray[1].interfaceNum, portMapArray[1].txqPortNumber, rc);

                CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsBmp);
                CPSS_PORTS_BMP_PORT_SET_MAC(&portsBmp, portMapArray[1].physicalPortNumber);

                /* AUTODOC: 2.4 Enable physical port to recieve traffic */
                rc = cpssDxChPortModeSpeedSet(devNum, &portsBmp, GT_TRUE,
                                                CPSS_PORT_INTERFACE_MODE_KR_E,
                                                CPSS_PORT_SPEED_10000_E);
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                               "tgfTrafficGeneratorPortForceLinkUpEnableSet FAIL : cpssDxChPortModeSpeedSet port[%d],power up [GT_TRUE]\n",
                                                portMapArray[1].interfaceNum);

                /* AUTODOC: GENERATE TRAFFIC: */

                /* AUTODOC: reset counters on Tx port */
                rc = prvTgfResetCountersEth(devNum, portMapArray[1].physicalPortNumber);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                               "prvTgfResetCountersEth: %d, %d", devNum, 0);

                /* AUTODOC: setup Packet */
                rc = prvTgfSetTxSetupEth(devNum, &prvTgfPacket1Info, 1, 0, NULL);
                UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc,
                                               "prvTgfSetTxSetupEth: %d, %d, %d, %d", devNum, 1, 0, NULL);

                /* AUTODOC: send Packet to port 0 */
                rc = prvTgfStartTransmitingEth(devNum, portMapArray[0].physicalPortNumber);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                                "ERROR of StartTransmitting: %d, %d",
                                                devNum, 1);
                /* AUTODOC: check counters on egress port */
                rc = prvTgfReadPortCountersEth(devNum, portMapArray[1].physicalPortNumber, GT_TRUE, &portCntrs);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                                "prvTgfReadPortCountersEth: %d, %d", devNum, 0);
                UTF_VERIFY_EQUAL2_STRING_MAC(1, portCntrs.goodPktsSent.l[0],
                                                "get another goodPktsRcv counter than expected, DQ[%d], TxQ port[%d] ", dq,  portMapArray[1].txqPortNumber);

                /* AUTODOC: Disable port */
                rc = cpssDxChPortModeSpeedSet(devNum, &portsBmp, GT_FALSE,
                                                CPSS_PORT_INTERFACE_MODE_KR_E,
                                                CPSS_PORT_SPEED_10000_E);
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                                "tgfTrafficGeneratorPortForceLinkUpEnableSet FAIL : cpssDxChPortModeSpeedSet port[%d], power up [GT_FALSE]\n",
                                                portMapArray[1].interfaceNum);
            }

            /* Enable CPU TxQ port on DQ > 0 */
            if (dq != 0)
            {
                regAddr = PRV_DXCH_REG1_UNIT_TXQ_DQ_INDEX_MAC(devNum, dq).global.globalDQConfig.globalDequeueConfig;
                rc = prvCpssHwPpSetRegField(devNum, regAddr, 0, 1, 1);
                if (rc != GT_OK)
                {
                    PRV_UTF_LOG1_MAC("Disable CPU TxQ port on DQ[%d] failure\n", dq);
                    return;
                }
            }
        }
    }

    /* AUTODOC: 2.3 Restore original configurations */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
                                 devNum, PRV_TGF_VLANID_CNS);

    rc = prvUtfNextMacPortReset(&portNum, devNum);
    if (rc != GT_OK)
    {
        PRV_UTF_LOG0_MAC("prvUtfNextMacPortReset failure 1 \n");
        return;
    }

    /* Enable all available ports */
    while(GT_OK == prvUtfNextMacPortGet(&portNum, GT_TRUE))
    {
        rc = cpssDxChPortPhysicalPortDetailedMapGet(devNum, portNum, &portMap);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                     "cpssDxChPortPhysicalPortDetailedMapGet");

        if (portMap.valid == GT_FALSE)
        {
            continue;
        }

        if (portMap.portMap.mappingType != CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E &&
            portIfData[portNum].ifMode != CPSS_PORT_INTERFACE_MODE_NA_E &&
            portIfData[portNum].speed != CPSS_PORT_SPEED_NA_E)
        {

            CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsBmp);
            CPSS_PORTS_BMP_PORT_SET_MAC(&portsBmp, portNum);

            rc = cpssDxChPortModeSpeedSet(devNum, &portsBmp, GT_TRUE,
                                          portIfData[portNum].ifMode,
                                          portIfData[portNum].speed);
            if (rc != GT_NOT_INITIALIZED)
            {
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                             "tgfTrafficGeneratorPortForceLinkUpEnableSet FAIL : cpssDxChPortModeSpeedSet port[%d], power up [GT_FALSE]\n",
                                              portNum);
            }
        }
    }
}


static GT_VOID prvTgfPortTxTcTailDropProfileWrRd
(
    IN  GT_VOID
)
{
   GT_STATUS                                 rc;
   GT_U32                                    maxIter = 1000;
   GT_U32                                    ii;
   GT_U8                                     trafficClass;
   CPSS_PORT_TX_DROP_PROFILE_SET_ENT         profileSet ;
   CPSS_PORT_TX_Q_TAIL_DROP_PROF_TC_PARAMS   tailDropProfileParamsGet;
   GT_U32                                    mcastMaxDescNum;
   GT_U32                                    osTimeStart, osTime;
   GT_U32                                    noResourceCnt;
   GT_U32                                    retryMax = 10;
   GT_U32                                    retryCnt;

   PRV_UTF_LOG0_MAC("======= : start prvTgfPortTxTcTailDropProfileWrRd =======\n");

   cpssOsMemSet(&tailDropProfileParamsGet, 0, sizeof(tailDropProfileParamsGet));
   cpssOsTimerWkAfter(100);
   noResourceCnt = 0;
   osTimeStart = cpssOsTime();
   for (ii = 0 ; ii < maxIter ; ii++)
   {
        /* each tailDropProfile table has 16 profile. each profile has 8 traffic class.
           scan all indexes (16 * 8) */
        for(profileSet = CPSS_PORT_TX_DROP_PROFILE_1_E;
             profileSet <= CPSS_PORT_TX_DROP_PROFILE_16_E;
             profileSet++)
        {
            for (trafficClass = 0; trafficClass < CPSS_TC_RANGE_CNS; trafficClass++)
            {
                for (retryCnt = 0; (retryCnt < retryMax); retryCnt++)
                {
                    rc = cpssDxChPortTx4TcTailDropProfileGet(
                        prvTgfDevNum, profileSet, trafficClass, &tailDropProfileParamsGet);
                    /* DBA firmware not ready */
                    if ((rc == GT_NO_RESOURCE) || (rc == GT_TIMEOUT))
                    {
                        noResourceCnt++;
                        PRV_UTF_LOG0_MAC("DBA Read: No Resource or Timeout\n");
                        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChPortTx4TcTailDropProfileGet\n");
                        cpssOsDelay(10000); /* time in nanoseconds */
                        continue;
                    }
                    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChPortTx4TcTailDropProfileGet\n");

                    rc = cpssDxChPortTx4TcTailDropProfileSet(prvTgfDevNum,
                                                             profileSet,
                                                             trafficClass,
                                                             &tailDropProfileParamsGet);
                    /* DBA firmware not ready */
                    if ((rc == GT_NO_RESOURCE) || (rc == GT_TIMEOUT))
                    {
                        noResourceCnt++;
                        PRV_UTF_LOG0_MAC("DBA Write: No Resource or Timeout\n");
                        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChPortTx4TcTailDropProfileSet\n");
                        cpssOsDelay(10000); /* time in nanoseconds */
                        continue;
                    }
                    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChPortTx4TcTailDropProfileSet\n");

                    break; /* continue inside only can cause to repeat */
                }
            }
        }
        if ((ii & 0x3f) == 0)
        {
            osTime = cpssOsTime();
            if ((osTime - osTimeStart) >= 2) break;
        }
    }
   PRV_UTF_LOG1_MAC("======= : noResourceCnt [%d]=======\n", noResourceCnt);
   PRV_UTF_LOG0_MAC("======= : end prvTgfPortTxTcTailDropProfileWrRd =======\n");
   PRV_UTF_LOG0_MAC("======= : start prvTgfPortTxMcastPcktDescrLimitWrRd =======\n");
   noResourceCnt = 0;
   osTimeStart = cpssOsTime();
   for (ii = 0 ; ii < maxIter ; ii++)
   {
       for (retryCnt = 0; (retryCnt < retryMax); retryCnt++)
       {
           rc = cpssDxChPortTxMcastPcktDescrLimitGet(prvTgfDevNum, &mcastMaxDescNum);
           /* DBA firmware not ready */
           if (rc == GT_NO_RESOURCE)
           {
               noResourceCnt++;
               PRV_UTF_LOG0_MAC("DBA Read: No Resource\n");
               UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChPortTxMcastPcktDescrLimitGet\n");
               cpssOsDelay(10000); /* time in nanoseconds */
               continue;
           }
           if (rc == GT_TIMEOUT)
           {
               noResourceCnt++;
               PRV_UTF_LOG0_MAC("DBA Read: Timeout\n");
               UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChPortTxMcastPcktDescrLimitGet\n");
               cpssOsDelay(10000); /* time in nanoseconds */
               continue;
           }
           UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChPortTxMcastPcktDescrLimitGet\n");
           rc = cpssDxChPortTxMcastPcktDescrLimitSet(prvTgfDevNum, mcastMaxDescNum);
           /* DBA firmware not ready */
           if ((rc == GT_NO_RESOURCE) || (rc == GT_TIMEOUT))
           {
               noResourceCnt++;
               PRV_UTF_LOG0_MAC("DBA Write: No Resource or Timeout\n");
               UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChPortTxMcastPcktDescrLimitSet\n");
               cpssOsDelay(10000); /* time in nanoseconds */
               continue;
           }
           UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChPortTxMcastPcktDescrLimitSet\n");

           break; /* continue inside only can cause to repeat */
       }
       if ((ii & 0xff) == 0)
       {
           osTime = cpssOsTime();
           if ((osTime - osTimeStart) >= 2) break;
       }
   }
   PRV_UTF_LOG1_MAC("======= : noResourceCnt [%d]=======\n", noResourceCnt);
   PRV_UTF_LOG0_MAC("======= : end prvTgfPortTxMcastPcktDescrLimitWrRd =======\n");
}


/**
* @internal prvPortTxFwsTcTailDropProfileWrRd function
* @endinternal
*
* @brief   test tail drop tables under FWS
*/
GT_VOID prvPortTxFwsTcTailDropProfileWrRd
(
    IN  GT_VOID
)
{
     GT_U32                  origSkipPortFwsStatus = 0;
     CPSS_PORTS_BMP_STC      origSkipPortBmap = {{0}};
     GT_U32                  portNum;

    PRV_UTF_LOG0_MAC("=============== :  start to do WR/RD without FWS traffic before FWS: =======\n");
    /* ---------- 1) WR/RD without traffic ----------  */
    prvTgfPortTxTcTailDropProfileWrRd();


    if (PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN_E)
    {
        /*configuration*/
        prvTgfPortFwsSkipPortsGet(&origSkipPortFwsStatus);
        prvTgfPortFwsSkipPortsBmpGet(&origSkipPortBmap);

        prvTgfPortFwsSkipPortsSet(0);

        for (portNum = 0; portNum < 16; portNum ++)
        {
          prvTgfPortFWSSkipPort(portNum, 0);
        }

        for (portNum = 16; portNum < 33; portNum ++)
        {
          prvTgfPortFWSSkipPort(portNum, 1);
        }
    }
    else if (PRV_CPSS_DXCH_CAELUM_CHECK_MAC(prvTgfDevNum))
    {
        prvTgfPortFWSSkipPort(56,0);
        prvTgfPortFWSSkipPort(57,0);
        prvTgfPortFWSSkipPort(59,0);
        prvTgfPortFWSSkipPort(64,0);
        prvTgfPortFWSSkipPort(67,0);
        prvTgfPortFWSSkipPort(70,0);
    }

    PRV_UTF_LOG0_MAC("============== :  start to do WR/RD with FWS traffic while unicast: =======\n");
    /* ---------- 2.1) WR/RD with FWS traffic, unicast ---------- */

    /*generate traffic and try to read and write from TxQ*/
    prvTgfFWSUseFloodingSet(0);
    prvTgfFwsGenericTestUnderFws(prvTgfPortTxTcTailDropProfileWrRd);
    prvTgfFWSRestore();

    PRV_UTF_LOG0_MAC("============== :  start to do WR/RD with FWS traffic while flooding: =======\n");
    /* ---------- 2.1) WR/RD with FWS traffic, flooding ---------- */

    /*vlan includes more than 2 ports*/
    prvTgfFWSUseFloodingSet(1);

    /*generate traffic and try to read and write from TxQ*/
    prvTgfFwsGenericTestUnderFws(prvTgfPortTxTcTailDropProfileWrRd);
    prvTgfFWSRestore();

    /* ---------- 3) WR/RD without traffic ---------- */
    PRV_UTF_LOG0_MAC("============== :  start to do WR/RD without FWS traffic after FWS: =======\n");

    prvTgfPortTxTcTailDropProfileWrRd();

    PRV_UTF_LOG0_MAC("==============:  start to do restore FWS: =======\n");
    /* stop traffic and restore port/vlan config */

    prvTgfFWSUseFloodingSet(0);
    if (PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN_E)
    {
        prvTgfPortFwsSkipPortsSet(origSkipPortFwsStatus);
        prvTgfPortFwsSkipPortsBmpSet(&origSkipPortBmap);
    }
    else if (PRV_CPSS_DXCH_CAELUM_CHECK_MAC(prvTgfDevNum))
    {
        prvTgfPortFWSSkipPort(56,1);
        prvTgfPortFWSSkipPort(57,1);
        prvTgfPortFWSSkipPort(59,1);
        prvTgfPortFWSSkipPort(64,1);
        prvTgfPortFWSSkipPort(67,1);
        prvTgfPortFWSSkipPort(70,1);
    }
}

/**
* @internal prvPortTxFullBandwidthTcTailDropProfileWrRd function
* @endinternal
*
* @brief   test tail drop tables under full bandwidth traffic
*/
GT_VOID prvPortTxFullBandwidthTcTailDropProfileWrRd
(
    IN  GT_VOID
)
{
    GT_U32                  origSkipPortFwsStatus = 0;
    CPSS_PORTS_BMP_STC      origSkipPortBmap = {{0}};
    GT_U32                  portNum;

    /*configuration*/
    if (PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN_E)
    {
        prvTgfPortFwsSkipPortsGet(&origSkipPortFwsStatus);
        prvTgfPortFwsSkipPortsBmpGet(&origSkipPortBmap);

        prvTgfPortFwsSkipPortsSet(0);

        for (portNum = 0; portNum < 16; portNum ++)
        {
          prvTgfPortFWSSkipPort(portNum, 0);
        }

        for (portNum = 16; portNum < 33; portNum ++)
        {
          prvTgfPortFWSSkipPort(portNum, 1);
        }
    }
    else if (PRV_CPSS_DXCH_CAELUM_CHECK_MAC(prvTgfDevNum))
    {
        prvTgfPortFWSSkipPort(56,0);
        prvTgfPortFWSSkipPort(57,0);
        prvTgfPortFWSSkipPort(59,0);
        prvTgfPortFWSSkipPort(64,0);
        prvTgfPortFWSSkipPort(67,0);
        prvTgfPortFWSSkipPort(70,0);
    }

    PRV_UTF_LOG0_MAC("=============== :  start to do WR/RD without FWS traffic before FWS: =======\n");
    /* ---------- 1) WR/RD without traffic ----------  */
    prvTgfPortTxTcTailDropProfileWrRd();


    PRV_UTF_LOG0_MAC("============== :  start to do WR/RD with FWS traffic while unicast: =======\n");
    /* ---------- 2.1) WR/RD with FWS traffic, unicast ---------- */

    /*generate traffic and try to read and write from TxQ*/
    prvTgfFWSUseFloodingSet(0);
    prvTgfFwsGenericFullBandwidthTest(prvTgfPortTxTcTailDropProfileWrRd);
    prvTgfFWSRestore();

    PRV_UTF_LOG0_MAC("============== :  start to do WR/RD with FWS traffic while flooding: =======\n");
    /* ---------- 2.1) WR/RD with FWS traffic, flooding ---------- */

    /*vlan includes more than 2 ports*/
    prvTgfFWSUseFloodingSet(1);

    /*generate traffic and try to read and write from TxQ*/
    prvTgfFwsGenericFullBandwidthTest(prvTgfPortTxTcTailDropProfileWrRd);
    prvTgfFWSRestore();

    /* ---------- 3) WR/RD without traffic ---------- */
    PRV_UTF_LOG0_MAC("============== :  start to do WR/RD without FWS traffic after FWS: =======\n");

    prvTgfPortTxTcTailDropProfileWrRd();

    PRV_UTF_LOG0_MAC("==============:  start to do restore FWS: =======\n");
    /* stop traffic and restore port/vlan config */

    prvTgfFWSUseFloodingSet(0);

    if (PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN_E)
    {
        prvTgfPortFwsSkipPortsSet(origSkipPortFwsStatus);
        prvTgfPortFwsSkipPortsBmpSet(&origSkipPortBmap);
    }
    else if (PRV_CPSS_DXCH_CAELUM_CHECK_MAC(prvTgfDevNum))
    {
        prvTgfPortFWSSkipPort(56,1);
        prvTgfPortFWSSkipPort(57,1);
        prvTgfPortFWSSkipPort(59,1);
        prvTgfPortFWSSkipPort(64,1);
        prvTgfPortFWSSkipPort(67,1);
        prvTgfPortFWSSkipPort(70,1);
    }
}



