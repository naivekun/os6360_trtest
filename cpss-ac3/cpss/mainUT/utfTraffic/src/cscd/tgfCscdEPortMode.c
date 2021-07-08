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
* @file tgfCscdEPortMode.c
*
* @brief CPSS Cascade remote port remapping
*
* @version   4
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/generic/cpssCommonDefs.h>
#include <cpss/generic/bridge/cpssGenBrgVlanTypes.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>
#include <utf/private/prvUtfExtras.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>
#include <trafficEngine/private/prvTgfTrafficParser.h>

#include <common/tgfCommon.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfCscdGen.h>
#include <cscd/prvTgfCscd.h>

#include <cscd/tgfCscdEPortMode.h>


/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/
/* default VLAN0 Id */
#define PRV_TGF_VLANID_CNS                 5

/* number of ports */
#define PRV_TGF_PORT_COUNT_CNS             4

/* burst count */
#define PRV_TGF_BURST_COUNT_CNS            1

#define PRV_TGF_SEND_PORT_IDX_CNS          0
/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_NUM_CNS   prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]

/* dsa device */
#define PRV_TGF_DSA_DEV_CNS                2

/* dsa port */
#define PRV_TGF_DSA_PORT_CNS               3

/* port base ePort */
#define PRV_TGF_PORT_BASE_EPORT_CNS       15

/* trunk base ePort */
#define PRV_TGF_TRUNK_BASE_EPORT_CNS      60

/* src trunk lsb amount */
#define PRV_TGF_SRC_TRUNK_LSB_AMOUNT_CNS  5

/* src port lsb amount */
#define PRV_TGF_SRC_PORT_LSB_AMOUNT_CNS   4

/* src dev lsb amount */
#define PRV_TGF_SRC_DEV_LSB_AMOUNT_CNS    0


/* port number fdb learning expected */
#define PRV_TGF_FDB_PORT_NUM_CNS          18


/******************************* Test packet **********************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part =
{
    {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},   /* dstMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x02}    /* srcMac */
};

static TGF_PACKET_DSA_TAG_STC  prvTgfPacketDsaTagPart = {
    TGF_DSA_CMD_FORWARD_E ,/*dsaCommand*/
    TGF_DSA_2_WORD_TYPE_E ,/*dsaType*/

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
static GT_U8 prvTgfPayloadDataArr[] =
{
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF,
};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart =
{
    sizeof(prvTgfPayloadDataArr),                       /* dataLength */
    prvTgfPayloadDataArr                                /* dataPtr */
};

/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfPacketPartArray[] =
{
    {TGF_PACKET_PART_L2_E,       &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_DSA_TAG_E,  &prvTgfPacketDsaTagPart},
    {TGF_PACKET_PART_PAYLOAD_E,  &prvTgfPacketPayloadPart}
};


/******************************************************/

/* LENGTH of packet */
#define PRV_TGF_PACKET_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_DSA_TAG_SIZE_CNS + sizeof(prvTgfPayloadDataArr)

/* PACKET to send */
static TGF_PACKET_STC prvTgfPacketInfo =
{
    PRV_TGF_PACKET_LEN_CNS,                                      /* totalLen */
    sizeof(prvTgfPacketPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketPartArray                                        /* partsArray */
};

/*************************** Restore config ***********************************/

/* parameters that is needed to be restored */
static struct
{
    CPSS_CSCD_PORT_TYPE_ENT     portType;
    GT_BOOL                     centralizedChassisModeEnable;
    GT_U32                      portBaseEport;
    GT_U32                      trunkBaseEport;
    GT_U32                      srcTrunkLsbAmount;
    GT_U32                      srcPortLsbAmount;
    GT_U32                      srcDevLsbAmount;
    GT_U32                      portsArray[PRV_TGF_MAX_PORTS_NUM_CNS];
} prvTgfRestoreCfg;


/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/**
* @internal tgfCscdEPortModeConfigurationSet function
* @endinternal
*
* @brief   Set test configuration:
*         configure Port 0 as DSA port,
*         configure port with:
*         port base ePort
*         trunk base ePort
*         src trunk lsb amount
*         src port lsb amount
*         src dev lsb amount
*/
GT_VOID tgfCscdEPortModeConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS   rc;
    GT_U32      portIter;


    /* AUTODOC: save prvTgfPortsArray */
    cpssOsMemCpy(prvTgfRestoreCfg.portsArray, prvTgfPortsArray,
                 sizeof(prvTgfRestoreCfg.portsArray));

    /* AUTODOC: re-arrange prvTgfPortsArray to ensure some ports (will be
     * configured cascade) are not remote ports */
    if (UTF_CPSS_DXCH_XCAT3X_CHECK_MAC(prvTgfDevNum))
    {
        rc = prvTgfDefPortsArrange(GT_FALSE, PRV_TGF_SEND_PORT_IDX_CNS, -1);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortsArray rearrangement can't be done");
    }

    /* set port type */
    rc = prvTgfCscdPortTypeGet(prvTgfDevNum, PRV_TGF_SEND_PORT_NUM_CNS, CPSS_PORT_DIRECTION_RX_E,
                                                        &prvTgfRestoreCfg.portType);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdPortTypeGet");

    rc = prvTgfCscdPortTypeSet(prvTgfDevNum, CPSS_PORT_DIRECTION_RX_E, PRV_TGF_SEND_PORT_NUM_CNS,
                                                        CPSS_CSCD_PORT_DSA_MODE_EXTEND_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdPortTypeSet");


    /* Centralized Chassis Mode Enable */
    rc = prvTgfCscdCentralizedChassisModeEnableGet(prvTgfDevNum, PRV_TGF_SEND_PORT_NUM_CNS,
                                            &prvTgfRestoreCfg.centralizedChassisModeEnable);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdCentralizedChassisModeEnableGet");
    rc = prvTgfCscdCentralizedChassisModeEnableSet(prvTgfDevNum, PRV_TGF_SEND_PORT_NUM_CNS, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdCentralizedChassisModeEnableSet");


    /* configure default ePort mapping assignment on ingress centralized chassis enabled ports, */
    /* when packets are received from line-card port/trunk accordingly. */
    rc = prvTgfCscdCentralizedChassisLineCardDefaultEportBaseGet(prvTgfDevNum, PRV_TGF_SEND_PORT_NUM_CNS,
                                            &prvTgfRestoreCfg.portBaseEport, &prvTgfRestoreCfg.trunkBaseEport);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdCentralizedChassisLineCardDefaultEportBaseGet");

    rc = prvTgfCscdCentralizedChassisLineCardDefaultEportBaseSet(prvTgfDevNum, PRV_TGF_SEND_PORT_NUM_CNS,
                                          PRV_TGF_PORT_BASE_EPORT_CNS, PRV_TGF_TRUNK_BASE_EPORT_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdCentralizedChassisLineCardDefaultEportBaseSet");


    /* configure the amount of least significant bits taken from DSA tag
       for assigning a default source ePort on CC ports,
       for packets received from line-card device trunks/physical ports accordingly. */
    rc = prvTgfCscdCentralizedChassisMappingDsaSrcLsbAmountGet(&prvTgfRestoreCfg.srcTrunkLsbAmount,
                                                               &prvTgfRestoreCfg.srcPortLsbAmount,
                                                               &prvTgfRestoreCfg.srcDevLsbAmount);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdCentralizedChassisMappingDsaSrcLsbAmountGet");

    rc = prvTgfCscdCentralizedChassisMappingDsaSrcLsbAmountSet(PRV_TGF_SRC_TRUNK_LSB_AMOUNT_CNS,
                                                               PRV_TGF_SRC_PORT_LSB_AMOUNT_CNS,
                                                               PRV_TGF_SRC_DEV_LSB_AMOUNT_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdCentralizedChassisMappingDsaSrcLsbAmountSet");


    /*AUTODOC:  create VLAN with untagged ports [0,1,2,3] */
    rc = prvTgfBrgDefVlanEntryWrite(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWrite: %d", PRV_TGF_VLANID_CNS);

    /*AUTODOC:  clear counters */
    for (portIter = 0; portIter < PRV_TGF_PORT_COUNT_CNS; portIter++)
    {
        /* reset ethernet counters */
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }
}


/**
* @internal tgfCscdEPortModeTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port[0] packet:
*         macDa = FF:FF:FF:FF:FF:FF,
*         macSa = 00:00:00:00:00:02,
*         Success Criteria:
*         fdb learning return match on port 18
*/
GT_VOID tgfCscdEPortModeTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS                   rc = GT_OK;
    GT_BOOL                     isOk;
    PRV_TGF_MAC_ENTRY_KEY_STC   macEntryKey;
    PRV_TGF_BRG_MAC_ENTRY_STC   prvTgfMacEntry;

    prvTgfPacketDsaTagPart.dsaInfo.forward.srcHwDev         = PRV_TGF_DSA_DEV_CNS;
    prvTgfPacketDsaTagPart.dsaInfo.forward.source.portNum = PRV_TGF_DSA_PORT_CNS;

    /* AUTODOC: setup Packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfPacketInfo, PRV_TGF_BURST_COUNT_CNS, 0, NULL);
    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d, %d",
                                 prvTgfDevNum, PRV_TGF_BURST_COUNT_CNS, 0, NULL);

    /* AUTODOC: clear table */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* AUTODOC: send Ethernet packet from port 0 with: */
    /* AUTODOC: DA=FF:FF:FF:FF:FF:FF, SA=00:00:00:00:00:02, VID=5 */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, PRV_TGF_SEND_PORT_NUM_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, %d",
                                 prvTgfDevNum, PRV_TGF_SEND_PORT_NUM_CNS);

    /* check FDB learning */
    macEntryKey.entryType = PRV_TGF_FDB_ENTRY_TYPE_MAC_ADDR_E;

    /* packet received untagged on line card port - treat it as if it was received untagged locally */
    macEntryKey.key.macVlan.vlanId = 1;

    cpssOsMemCpy(macEntryKey.key.macVlan.macAddr.arEther, prvTgfPacketL2Part.saMac, sizeof(TGF_MAC_ADDR));
    cpssOsMemSet(&prvTgfMacEntry, 0, sizeof(prvTgfMacEntry));

    rc = prvTgfBrgFdbMacEntryGet(&macEntryKey, &prvTgfMacEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbMacEntryGet failed");

    if (GT_OK == rc)
    {
        isOk = (prvTgfMacEntry.dstInterface.devPort.hwDevNum  == prvTgfDevNum) &&
               (prvTgfMacEntry.dstInterface.devPort.portNum == PRV_TGF_FDB_PORT_NUM_CNS);

        UTF_VERIFY_EQUAL0_STRING_MAC(GT_TRUE, isOk, "got wrong fdb learning results");
    }
}

/**
* @internal tgfCscdEPortModeConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID tgfCscdEPortModeConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS   rc;

    /* restore port type */
    rc = prvTgfCscdPortTypeSet(prvTgfDevNum, CPSS_PORT_DIRECTION_RX_E, PRV_TGF_SEND_PORT_NUM_CNS,
                                                    prvTgfRestoreCfg.portType);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdPortTypeSet config restore");

    /* Centralized Chassis Mode Enable */
    rc = prvTgfCscdCentralizedChassisModeEnableSet(prvTgfDevNum, PRV_TGF_SEND_PORT_NUM_CNS,
                                            prvTgfRestoreCfg.centralizedChassisModeEnable);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdCentralizedChassisModeEnableSet config restore");

    /* restore default ePort mapping assignment on ingress centralized chassis enabled ports, */
    rc = prvTgfCscdCentralizedChassisLineCardDefaultEportBaseSet(prvTgfDevNum, PRV_TGF_SEND_PORT_NUM_CNS,
                                            prvTgfRestoreCfg.portBaseEport, prvTgfRestoreCfg.trunkBaseEport);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdCentralizedChassisLineCardDefaultEportBaseSet config restore");

    /* restore configure the amount of least significant bits taken from DSA tag
       for assigning a default source ePort on CC ports,
       for packets received from line-card device trunks/physical ports accordingly. */
    rc = prvTgfCscdCentralizedChassisMappingDsaSrcLsbAmountSet(prvTgfRestoreCfg.srcTrunkLsbAmount,
                                                               prvTgfRestoreCfg.srcPortLsbAmount,
                                                               prvTgfRestoreCfg.srcDevLsbAmount);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdCentralizedChassisMappingDsaSrcLsbAmountSet config restore");


    /* invalidate VLAN entry */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d", PRV_TGF_VLANID_CNS);

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d", GT_TRUE);

    /* AUTODOC: restore prvTgfPortsArray */
    cpssOsMemCpy(prvTgfPortsArray, prvTgfRestoreCfg.portsArray,
                 sizeof(prvTgfRestoreCfg.portsArray));

}

