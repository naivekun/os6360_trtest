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
* @file prvTgfCscdCentralizedChassisRemotePhysicalPortMap.c
*
* @brief Mapping of a target physical port to a remote physical port that
* resides over a DSA-tagged interface
*
* @version   2
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


/* port number to send traffic to */
#define PRV_TGF_SOURCE_CASCADE_PORT_NUM_CNS   65

#define PRV_TGF_TARGET_CASCADE_PORT_NUM_CNS   67

/* number of ports */
#define PRV_TGF_PORT_COUNT_CNS          4

/* port number to send traffic from */
#define PRV_TGF_SOURCE_REMOTE_PORT_NUM_CNS   608

/* port number to send traffic to */
#define PRV_TGF_REMOTE_PORT_TARGET_NUM_CNS   622

/* number of remote ports in test. 1 source remote port
   and 1 "target" by flodding remote port */
#define PRV_TGF_REMOTE_PORT_COUNT_CNS          2

/* source remote device */
#define PRV_TGF_SOURCE_REMOTE_DEV_CNS                2

/* source remote port */
#define PRV_TGF_SOURCE_REMOTE_LOCAL_PORT_CNS               3

/* target remote device */
#define PRV_TGF_TARGET_REMOTE_DEV_CNS         4

/* target remote port */
#define PRV_TGF_REMOTE_TARGET_PORT_CNS       2

/* physical port base for source remote port */
#define PRV_TGF_PHYSICAL_PORT_BASE_CNS    605

/* egress eport to receive traffic from */
#define PRV_TGF_TARGET_EPORT_CNS        65

/* burst count */
#define PRV_TGF_BURST_COUNT_CNS            1

/* vlan ID */
#define PRV_TGF_VLANID_CNS 5

/*************************** Restore config ***********************************/

/* parameters that is needed to be restored */
static struct
{
    CPSS_INTERFACE_INFO_STC                                  physicalInfo;
    CPSS_CSCD_PORT_TYPE_ENT                                  portType0;
    CPSS_CSCD_PORT_TYPE_ENT                                  portType1;
    PRV_TGF_CSCD_INGRESS_PHYSICAL_PORT_ASSIGNMENT_MODE_ENT   portAssignMode;
    GT_U32                                                   srcDevLsbAmount;
    GT_U32                                                   srcPortLsbAmount;
    GT_U32                                                   phyPortBase;
    GT_BOOL                                                  assignSrcDevEnable;
    CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_ENT               portLinkStatusState0;
    CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_ENT               portLinkStatusState1;
    GT_BOOL                                                  portMapEnable;
    GT_HW_DEV_NUM                                            remotePhysicalHwDevNum;
    GT_PHYSICAL_PORT_NUM                                     remotePhysicalPortNum;
    GT_U32                                                   vlanTpid;
    GT_U32                                                   HaCscadeEnable;
    GT_U32                                                   Ha4BytesFromCpu;
    GT_U32                                                   portsArray[PRV_TGF_MAX_PORTS_NUM_CNS];
} prvTgfRestoreCfg;

/******************************* Test packet **********************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part =
{
    {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},   /* dstMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x02}    /* srcMac */
};


/* DATA of packet */
static GT_U8 prvTgfPayloadDataArr[] =
{
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
    0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
    0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F
};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart =
{
    sizeof(prvTgfPayloadDataArr),                       /* dataLength */
    prvTgfPayloadDataArr                                /* dataPtr */
};


/********************************** DSA Tag configurations *********************************/


static TGF_PACKET_DSA_TAG_STC  prvTgfPacketDsaTagPart = {
    TGF_DSA_CMD_FORWARD_E ,/*dsaCommand*/
    TGF_DSA_1_WORD_TYPE_E ,/*dsaType*/

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



/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfPacketDsaPartArray[] =
{
    {TGF_PACKET_PART_L2_E,       &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_DSA_TAG_E,  &prvTgfPacketDsaTagPart},
    {TGF_PACKET_PART_PAYLOAD_E,  &prvTgfPacketPayloadPart}
};



/* LENGTH of packet */
#define PRV_TGF_PACKET_DSA_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_DSA_TAG_SIZE_CNS + sizeof(prvTgfPayloadDataArr)

/* PACKET to send */
static TGF_PACKET_STC prvTgfPacketDsaInfo =
{
    PRV_TGF_PACKET_DSA_LEN_CNS,                                      /* totalLen */
    sizeof(prvTgfPacketDsaPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketDsaPartArray                                        /* partsArray */
};



/********************************** VLAN Tag configurations *********************************/

static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTagPart0 = {
    0x8123,                    /* etherType ( TPID)*/
    0, 0, (PRV_TGF_VLANID_CNS +1)                            /* pri, cfi, VlanId */

};


static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTagPart1 = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType ( TPID)*/
    0, 0, PRV_TGF_VLANID_CNS                            /* pri, cfi, VlanId */

};

/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfPacketVlanPartArray[] =
{
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTagPart0},
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTagPart1},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};


/* LENGTH of packet */
#define PRV_TGF_PACKET_VLAN_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + sizeof(prvTgfPayloadDataArr) + TGF_CRC_LEN_CNS

/* PACKET to send */
static TGF_PACKET_STC prvTgfPacketVlanInfo =
{
    PRV_TGF_PACKET_VLAN_LEN_CNS,                                      /* totalLen */
    sizeof(prvTgfPacketVlanPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketVlanPartArray                                        /* partsArray */
};

/********************************** E Tag configurations *********************************/

static TGF_ETHER_TYPE prvTgfetherTypeEtagPart = 0x893f;

/* DATA of packet */
static GT_U8 prvTgfEtagDataArr[] = 
{
   0x00, 0x00, 0x00, 0x03, 0x00, 0x00
};


/* E-Tag part */
static TGF_PACKET_PAYLOAD_STC prvTgfEtagPart =
{
    sizeof(prvTgfEtagDataArr),                       /* dataLength */
    prvTgfEtagDataArr                                /* dataPtr */
};



/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfPacketEtagPartArray[] =
{
    {TGF_PACKET_PART_L2_E,         &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_ETHERTYPE_E,  &prvTgfetherTypeEtagPart},
    {TGF_PACKET_PART_PAYLOAD_E,    &prvTgfEtagPart},
    {TGF_PACKET_PART_VLAN_TAG_E,   &prvTgfPacketVlanTagPart1},
    {TGF_PACKET_PART_PAYLOAD_E,    &prvTgfPacketPayloadPart}
};



/* LENGTH of packet */
#define PRV_TGF_PACKET_ETAG_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS +sizeof(prvTgfEtagDataArr) + TGF_VLAN_TAG_SIZE_CNS + sizeof(prvTgfPayloadDataArr)


/* PACKET to send */
static TGF_PACKET_STC prvTgfPacketEtagInfo =
{
    PRV_TGF_PACKET_ETAG_LEN_CNS,                                      /* totalLen */
    sizeof(prvTgfPacketEtagPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketEtagPartArray                                        /* partsArray */
};



/**************************************************************/
/*               Common functions for Remote Ports            */
/**************************************************************/


/**
* @internal 
*           prvTgfRemoteTargetPhysicalPortConfigurationSet
*           function
* @endinternal
*
* @brief   Set test target remote port configuration for target 
*          (by flooding) remote ports and cascades.
*/
static GT_VOID prvTgfRemoteTargetPhysicalPortConfigurationSet
(
   IN GT_U32                remotePortNum,
   IN GT_U32                localRemotePortNum,
   IN GT_BOOL               isTagged
)
{
    GT_STATUS       rc;

    /* add remote port to the vlan*/
    rc = prvTgfBrgVlanMemberAdd(prvTgfDevNum,PRV_TGF_VLANID_CNS,remotePortNum, isTagged );
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
            "prvTgfBrgVlanMemberAdd: %d %d", prvTgfDevNum, remotePortNum);

    /*for the 'unaware application' about 'link down filter': force link up */
    rc = cpssDxChBrgEgrFltPortLinkEnableSet (prvTgfDevNum, remotePortNum,
        CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_FORCE_LINK_UP_E);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "cpssDxChBrgEgrFltPortLinkEnableSet: %d %d", prvTgfDevNum,remotePortNum);

    /* enable map between target port to remote local physical port */
    rc = prvTgfCscdCentralizedChassisRemotePhysicalPortMapEnableGet(prvTgfDevNum,
                                                                    remotePortNum,
                                                                    &(prvTgfRestoreCfg.portMapEnable));
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfCscdCentralizedChassisRemotePhysicalPortMapEnableGet: %d", prvTgfDevNum);

    rc = prvTgfCscdCentralizedChassisRemotePhysicalPortMapEnableSet(prvTgfDevNum,
                                                                    remotePortNum,
                                                                    GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfCscdCentralizedChassisRemotePhysicalPortMapEnableSet: %d", prvTgfDevNum);


    /* map between target port to remote local physical port */
    rc = prvTgfCscdCentralizedChassisRemotePhysicalPortMapGet(prvTgfDevNum,
                                                              remotePortNum,
                                                              &(prvTgfRestoreCfg.remotePhysicalHwDevNum),
                                                              &(prvTgfRestoreCfg.remotePhysicalPortNum));
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfCscdCentralizedChassisRemotePhysicalPortMapGet: %d", PRV_TGF_TARGET_REMOTE_DEV_CNS);

    rc = prvTgfCscdCentralizedChassisRemotePhysicalPortMapSet(prvTgfDevNum,
                                                              remotePortNum,
                                                              PRV_TGF_TARGET_REMOTE_DEV_CNS,
                                                              localRemotePortNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfCscdCentralizedChassisRemotePhysicalPortMapSet: %d", PRV_TGF_TARGET_REMOTE_DEV_CNS);

}

/**
* @internal 
*           prvTgfRemotePhysicalPortMapConfigurationSet function
* @endinternal
*
* @brief   Set test configuration: 
*          1. cvreatet vlan group with two remote port ( one is
*          the source port) and two local ports
*         2. set configuration for cascades ports
*         3. set tti and mapping configuration for physical
*         ports and cascade ports.
*/
GT_VOID prvTgfCommonRemotePhysicalPortMapConfigurationSet
(
    IN GT_U32                         phyPortBase,
    IN GT_U8                          *tagArray,
    IN GT_BOOL                        isTagged
)
{

    GT_STATUS                   rc;
    CPSS_INTERFACE_INFO_STC     physicalInfo;

    /* AUTODOC: SETUP CONFIGURATION: */

    /* AUTODOC: save prvTgfPortsArray */
    cpssOsMemCpy(prvTgfRestoreCfg.portsArray, prvTgfPortsArray,
                 sizeof(prvTgfRestoreCfg.portsArray));

    /* add the cascade port to tests ports array to set the port in link up before transmit the packet*/
    prvTgfPortsArray[2] = PRV_TGF_SOURCE_CASCADE_PORT_NUM_CNS;
    prvTgfPortsArray[3] = PRV_TGF_TARGET_CASCADE_PORT_NUM_CNS;

    /*1. prvTgfPortsArray:  58, 12, 65, 67. VLAN configurations:*/

    /* AUTODOC: create VLAN 5 with untagged ports 58, 12 */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_VLANID_CNS,
            prvTgfPortsArray, NULL, tagArray, 2);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "prvTgfBrgDefVlanEntryWithPortsSet: %d", prvTgfDevNum);

    /* add source remote port to the vlan*/
    rc = prvTgfBrgVlanMemberAdd(prvTgfDevNum,PRV_TGF_VLANID_CNS, PRV_TGF_SOURCE_REMOTE_PORT_NUM_CNS, isTagged );
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
            "prvTgfBrgVlanMemberAdd: %d %d", prvTgfDevNum, PRV_TGF_SOURCE_REMOTE_PORT_NUM_CNS);

    /*for the 'unaware application' about 'link down filter': force link up */
    rc = cpssDxChBrgEgrFltPortLinkEnableSet (prvTgfDevNum, PRV_TGF_SOURCE_REMOTE_PORT_NUM_CNS,
        CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_FORCE_LINK_UP_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChBrgEgrFltPortLinkEnableSet: %d", prvTgfDevNum);


    /* to limit errors by application that added it to vlan */
    rc = cpssDxChBrgEgrFltPortLinkEnableGet(prvTgfDevNum, PRV_TGF_SOURCE_CASCADE_PORT_NUM_CNS ,
       &(prvTgfRestoreCfg.portLinkStatusState0));
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChBrgEgrFltPortLinkEnableSet");

    rc = cpssDxChBrgEgrFltPortLinkEnableSet(prvTgfDevNum, PRV_TGF_SOURCE_CASCADE_PORT_NUM_CNS ,
        CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_FORCE_LINK_DOWN_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChBrgEgrFltPortLinkEnableSet");


    /* to limit errors by application that added it to vlan */
    rc = cpssDxChBrgEgrFltPortLinkEnableGet(prvTgfDevNum, PRV_TGF_TARGET_CASCADE_PORT_NUM_CNS ,
       &(prvTgfRestoreCfg.portLinkStatusState1));
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChBrgEgrFltPortLinkEnableSet");

    rc = cpssDxChBrgEgrFltPortLinkEnableSet(prvTgfDevNum, PRV_TGF_TARGET_CASCADE_PORT_NUM_CNS ,
        CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_FORCE_LINK_DOWN_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChBrgEgrFltPortLinkEnableSet");



    PRV_UTF_LOG0_MAC("======= Setting tti Configuration: Base, assignment mode and port type =======\n");

    /*2. prvTgfPortsArray:  58, 65, 67, 0. cascade configurations:*/

    /* set ingress physical port base */
    rc = prvTgfCscdMyPhysicalPortBaseGet(prvTgfDevNum, PRV_TGF_SOURCE_CASCADE_PORT_NUM_CNS,
                                                  &(prvTgfRestoreCfg.phyPortBase));
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdMyPhysicalPortBaseGet");

    rc = prvTgfCscdMyPhysicalPortBaseSet(prvTgfDevNum, PRV_TGF_SOURCE_CASCADE_PORT_NUM_CNS,
                                                phyPortBase);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdMyPhysicalPortBaseSet");


    /* set enable the assignment of the packet source device to be the local device number */
    rc = prvTgfCscdMyPhysicalPortAssignSrcDevEnableGet(prvTgfDevNum, PRV_TGF_SOURCE_CASCADE_PORT_NUM_CNS,
                                                  &(prvTgfRestoreCfg.assignSrcDevEnable));
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdMyPhysicalPortAssignSrcDevEnableGet");

    rc = prvTgfCscdMyPhysicalPortAssignSrcDevEnableSet(prvTgfDevNum,
                                PRV_TGF_SOURCE_CASCADE_PORT_NUM_CNS, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdMyPhysicalPortAssignSrcDevEnableSet");

    /*3. prvTgfPortsArray:  58, 65, 67, 0. remote Port configurations:*/

    /* set the mapping table */
    rc = prvTgfBrgEportToPhysicalPortTargetMappingTableGet(prvTgfDevNum,
                                                           PRV_TGF_TARGET_EPORT_CNS,
                                                           &(prvTgfRestoreCfg.physicalInfo));
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgEportToPhysicalPortTargetMappingTableGet: %d", prvTgfDevNum);

    physicalInfo.type = CPSS_INTERFACE_PORT_E;
    physicalInfo.devPort.hwDevNum = prvTgfDevNum;
    physicalInfo.devPort.portNum = PRV_TGF_SOURCE_REMOTE_PORT_NUM_CNS;
    rc = prvTgfBrgEportToPhysicalPortTargetMappingTableSet(prvTgfDevNum,
                                                           PRV_TGF_TARGET_EPORT_CNS,
                                                           &physicalInfo);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgEportToPhysicalPortTargetMappingTableSet: %d", prvTgfDevNum);


    /* the cpu should send the packet to target eport and not to physical port*/
    rc = tgfStateTrgEPortInsteadPhyPort(PRV_TGF_SOURCE_REMOTE_PORT_NUM_CNS, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfStateTrgEPortInsteadPhyPort");
}


/**
* @internal 
*           prvTgfRemotePhysicalPortMapCheck function
* @endinternal
*
* @brief Send packet with tag (dsa, vlan or etag) to cascade 
*       port in loopback; identifythe return packet as receive
*       from remote port which connected to the cascade.
*       flooding the packet to all vlan members: regular local
*       ports and one remote ports ( connect to diffrent
*       cascade). trap the packets to cpu check ethernet
*       counters and FDB entry.
*/
GT_VOID prvTgfCommonRemotePhysicalPortMapCheck
(
    GT_U32 floodingLocalTargetPortsPacketLength,
    GT_U32 floodingCascadeTargetPortsPacketLength,
    GT_U32 sourceCascadePortsPacketLength
)
{
    GT_U32                      portIter;
    CPSS_INTERFACE_INFO_STC         portInterface;
    PRV_TGF_MAC_ENTRY_KEY_STC   macEntryKey;
    PRV_TGF_BRG_MAC_ENTRY_STC   prvTgfMacEntry;
    CPSS_PORT_MAC_COUNTER_SET_STC   portCntrs;
    CPSS_PORT_MAC_COUNTER_SET_STC   expectedCntrs;
    GT_STATUS                       rc;


    for (portIter = 0; portIter < PRV_TGF_PORT_COUNT_CNS; portIter++)
    {
        /* reset ethernet counters */
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* setup target portInterface for capturing */
    portInterface.type            = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = prvTgfDevNum;
    portInterface.devPort.portNum = PRV_TGF_TARGET_CASCADE_PORT_NUM_CNS;
    /* enable capture on target port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                 prvTgfDevNum, PRV_TGF_TARGET_CASCADE_PORT_NUM_CNS);

    /* AUTODOC: clear table */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* AUTODOC: send Ethernet packet from port 608 with: */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, PRV_TGF_SOURCE_CASCADE_PORT_NUM_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, %d",
                                 prvTgfDevNum, PRV_TGF_SOURCE_CASCADE_PORT_NUM_CNS);


    /* disable capture on target port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                 prvTgfDevNum, PRV_TGF_TARGET_CASCADE_PORT_NUM_CNS);

    /* check FDB learning */
    macEntryKey.entryType = PRV_TGF_FDB_ENTRY_TYPE_MAC_ADDR_E;
    macEntryKey.key.macVlan.vlanId = PRV_TGF_VLANID_CNS;

    cpssOsMemCpy(macEntryKey.key.macVlan.macAddr.arEther, prvTgfPacketL2Part.saMac, sizeof(TGF_MAC_ADDR));
    cpssOsMemSet(&prvTgfMacEntry, 0, sizeof(prvTgfMacEntry));

    rc = prvTgfBrgFdbMacEntryGet(&macEntryKey, &prvTgfMacEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbMacEntryGet failed");

    UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfDevNum, prvTgfMacEntry.dstInterface.devPort.hwDevNum, "prvTgfBrgFdbMacEntryGet dev num failed");
    UTF_VERIFY_EQUAL0_STRING_MAC(PRV_TGF_SOURCE_REMOTE_PORT_NUM_CNS, prvTgfMacEntry.dstInterface.devPort.portNum,"prvTgfBrgFdbMacEntryGet port num failed");

    /* prvTgfPortsArray:  58, 65, 67, 0. remote Port configurations:*/

    /* AUTODOC: verify traffic */
    for (portIter = 0; portIter < PRV_TGF_PORT_COUNT_CNS; portIter++)
    {
        GT_BOOL isOk;

        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d\n",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        /* check Tx and Rx counters */
        switch (prvTgfPortsArray[portIter])
        {
            case PRV_TGF_SOURCE_CASCADE_PORT_NUM_CNS:
                expectedCntrs.goodOctetsSent.l[0] = sourceCascadePortsPacketLength * PRV_TGF_BURST_COUNT_CNS;
                expectedCntrs.goodPktsSent.l[0]   = PRV_TGF_BURST_COUNT_CNS;
                expectedCntrs.ucPktsSent.l[0]     = 0;
                expectedCntrs.brdcPktsSent.l[0]   = PRV_TGF_BURST_COUNT_CNS;
                expectedCntrs.mcPktsSent.l[0]     = 0;
                expectedCntrs.goodOctetsRcv.l[0]  = sourceCascadePortsPacketLength * PRV_TGF_BURST_COUNT_CNS;
                expectedCntrs.goodPktsRcv.l[0]    = PRV_TGF_BURST_COUNT_CNS;
                expectedCntrs.ucPktsRcv.l[0]      = 0;
                expectedCntrs.brdcPktsRcv.l[0]    = PRV_TGF_BURST_COUNT_CNS;
                expectedCntrs.mcPktsRcv.l[0]      = 0;
                break;

            case PRV_TGF_TARGET_CASCADE_PORT_NUM_CNS:
                expectedCntrs.goodOctetsSent.l[0] = floodingCascadeTargetPortsPacketLength * PRV_TGF_BURST_COUNT_CNS;
                expectedCntrs.goodPktsSent.l[0]   = PRV_TGF_BURST_COUNT_CNS;
                expectedCntrs.ucPktsSent.l[0]     = 0;
                expectedCntrs.brdcPktsSent.l[0]   = PRV_TGF_BURST_COUNT_CNS;
                expectedCntrs.mcPktsSent.l[0]     = 0;
                expectedCntrs.goodOctetsRcv.l[0]  = floodingCascadeTargetPortsPacketLength * PRV_TGF_BURST_COUNT_CNS;
                expectedCntrs.goodPktsRcv.l[0]    = PRV_TGF_BURST_COUNT_CNS;
                expectedCntrs.ucPktsRcv.l[0]      = 0;
                expectedCntrs.brdcPktsRcv.l[0]    = PRV_TGF_BURST_COUNT_CNS;
                expectedCntrs.mcPktsRcv.l[0]      = 0;
                break;

            default:
                /* for other ports, regular local physical ports, not cascade */
                expectedCntrs.goodOctetsSent.l[0] = floodingLocalTargetPortsPacketLength * PRV_TGF_BURST_COUNT_CNS;
                expectedCntrs.goodPktsSent.l[0]   = PRV_TGF_BURST_COUNT_CNS;
                expectedCntrs.ucPktsSent.l[0]     = 0;
                expectedCntrs.brdcPktsSent.l[0]   = PRV_TGF_BURST_COUNT_CNS;
                expectedCntrs.mcPktsSent.l[0]     = 0;
                expectedCntrs.goodOctetsRcv.l[0]  = 0;
                expectedCntrs.goodPktsRcv.l[0]    = 0;
                expectedCntrs.ucPktsRcv.l[0]      = 0;
                expectedCntrs.brdcPktsRcv.l[0]    = 0;
                expectedCntrs.mcPktsRcv.l[0]      = 0;
                break;
        }

        isOk =
            portCntrs.goodOctetsSent.l[0] == expectedCntrs.goodOctetsSent.l[0] &&
            portCntrs.goodPktsSent.l[0]   == expectedCntrs.goodPktsSent.l[0] &&
            portCntrs.ucPktsSent.l[0]     == expectedCntrs.ucPktsSent.l[0] &&
            portCntrs.brdcPktsSent.l[0]   == expectedCntrs.brdcPktsSent.l[0] &&
            portCntrs.mcPktsSent.l[0]     == expectedCntrs.mcPktsSent.l[0] &&
            portCntrs.goodOctetsRcv.l[0]  == expectedCntrs.goodOctetsRcv.l[0] &&
            portCntrs.goodPktsRcv.l[0]    == expectedCntrs.goodPktsRcv.l[0] &&
            portCntrs.ucPktsRcv.l[0]      == expectedCntrs.ucPktsRcv.l[0] &&
            portCntrs.brdcPktsRcv.l[0]    == expectedCntrs.brdcPktsRcv.l[0] &&
            portCntrs.mcPktsRcv.l[0]      == expectedCntrs.mcPktsRcv.l[0];

        UTF_VERIFY_EQUAL0_STRING_MAC(isOk, GT_TRUE, "get another counters values.");

        /* print expected values if bug */
        if (isOk != GT_TRUE) {
            PRV_UTF_LOG0_MAC("Expected values:\n");
            PRV_UTF_LOG1_MAC(" goodOctetsSent = %d\n", expectedCntrs.goodOctetsSent.l[0]);
            PRV_UTF_LOG1_MAC(" goodPktsSent = %d\n", expectedCntrs.goodPktsSent.l[0]);
            PRV_UTF_LOG1_MAC(" ucPktsSent = %d\n", expectedCntrs.ucPktsSent.l[0]);
            PRV_UTF_LOG1_MAC(" brdcPktsSent = %d\n", expectedCntrs.brdcPktsSent.l[0]);
            PRV_UTF_LOG1_MAC(" mcPktsSent = %d\n", expectedCntrs.mcPktsSent.l[0]);
            PRV_UTF_LOG1_MAC(" goodOctetsRcv = %d\n", expectedCntrs.goodOctetsRcv.l[0]);
            PRV_UTF_LOG1_MAC(" goodPktsRcv = %d\n", expectedCntrs.goodPktsRcv.l[0]);
            PRV_UTF_LOG1_MAC(" ucPktsRcv = %d\n", expectedCntrs.ucPktsRcv.l[0]);
            PRV_UTF_LOG1_MAC(" brdcPktsRcv = %d\n", expectedCntrs.brdcPktsRcv.l[0]);
            PRV_UTF_LOG1_MAC(" mcPktsRcv = %d\n", expectedCntrs.mcPktsRcv.l[0]);
            PRV_UTF_LOG0_MAC("\n");
        }
    }

}


/**
* @internal 
*           prvTgfRemotePhysicalPortMapConfigurationRestore
*           function
* @endinternal
*
* @brief   Restore configuration
*/
GT_VOID prvTgfCommonRemotePhysicalPortMapConfigurationRestore
(
    GT_VOID
)
{

    GT_STATUS                 rc;
    CPSS_INTERFACE_INFO_STC                 portInterface;

    /* AUTODOC: RESTORE CONFIGURATION: */

    PRV_UTF_LOG0_MAC("======= Restoring Configuration =======\n");

     /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");


    rc = prvTgfCscdPortTypeSet(prvTgfDevNum, CPSS_PORT_DIRECTION_RX_E, PRV_TGF_SOURCE_CASCADE_PORT_NUM_CNS,
                                                        prvTgfRestoreCfg.portType0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdPortTypeSet");

    rc = prvTgfCscdPortTypeSet(prvTgfDevNum, CPSS_PORT_DIRECTION_TX_E, PRV_TGF_TARGET_CASCADE_PORT_NUM_CNS,
                                                        prvTgfRestoreCfg.portType1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdPortTypeSet");


    rc = prvTgfCscdMyPhysicalPortAssignmentModeSet(prvTgfDevNum, PRV_TGF_SOURCE_CASCADE_PORT_NUM_CNS,
                                                    prvTgfRestoreCfg.portAssignMode);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdMyPhysicalPortAssignmentModeSet");


    rc = prvTgfCscdMyPhysicalPortBaseSet(prvTgfDevNum, PRV_TGF_SOURCE_CASCADE_PORT_NUM_CNS,
                                                  prvTgfRestoreCfg.phyPortBase);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdMyPhysicalPortBaseSet");


    rc = prvTgfCscdMyPhysicalPortAssignSrcDevEnableSet(prvTgfDevNum, PRV_TGF_SOURCE_CASCADE_PORT_NUM_CNS,
                                                  prvTgfRestoreCfg.assignSrcDevEnable);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdMyPhysicalPortAssignSrcDevEnableSet");


    rc = prvTgfBrgEportToPhysicalPortTargetMappingTableSet(prvTgfDevNum,
                                                            PRV_TGF_TARGET_EPORT_CNS,
                                                            &(prvTgfRestoreCfg.physicalInfo));
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgEportToPhysicalPortTargetMappingTableSet: %d", prvTgfDevNum);


    /* to limit errors by application that added it to vlan */
    rc = cpssDxChBrgEgrFltPortLinkEnableSet(prvTgfDevNum, PRV_TGF_SOURCE_CASCADE_PORT_NUM_CNS ,
                                            prvTgfRestoreCfg.portLinkStatusState0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChBrgEgrFltPortLinkEnableSet");

     /* to limit errors by application that added it to vlan */
    rc = cpssDxChBrgEgrFltPortLinkEnableSet(prvTgfDevNum, PRV_TGF_TARGET_CASCADE_PORT_NUM_CNS ,
                                            prvTgfRestoreCfg.portLinkStatusState1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChBrgEgrFltPortLinkEnableSet");


    /* return the cpu from send the packet to target eport to send to phy port */
    rc = tgfStateTrgEPortInsteadPhyPort(PRV_TGF_SOURCE_REMOTE_PORT_NUM_CNS, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfStateTrgEPortInsteadPhyPort");

    /* disable force link pass */
    portInterface.type = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum = prvTgfDevNum;
    portInterface.devPort.portNum = PRV_TGF_SOURCE_CASCADE_PORT_NUM_CNS;
    rc = tgfTrafficGeneratorPortForceLinkUpEnableSet(&portInterface, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ForceLinkUpEnableSet: %d",PRV_TGF_SOURCE_CASCADE_PORT_NUM_CNS);



    /* enable map between target port to remote local physical port */
    rc = prvTgfCscdCentralizedChassisRemotePhysicalPortMapEnableSet(prvTgfDevNum,
                                                                    PRV_TGF_REMOTE_PORT_TARGET_NUM_CNS,
                                                                    prvTgfRestoreCfg.portMapEnable);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfCscdCentralizedChassisRemotePhysicalPortMapEnableSet: %d", prvTgfDevNum);

    /* map between target port to remote local physical port */
    rc = prvTgfCscdCentralizedChassisRemotePhysicalPortMapSet(prvTgfDevNum,
                                                              PRV_TGF_REMOTE_PORT_TARGET_NUM_CNS,
                                                              prvTgfRestoreCfg.remotePhysicalHwDevNum,
                                                              prvTgfRestoreCfg.remotePhysicalPortNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfCscdCentralizedChassisRemotePhysicalPortMapSet: %d", PRV_TGF_TARGET_REMOTE_DEV_CNS);


    /* flush the FDB table */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d", prvTgfDevNum);

    /* delete VLAN 5 */
    rc = prvTgfBrgVlanEntryInvalidate(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEntryInvalidate: %d, %d",
                                 prvTgfDevNum, PRV_TGF_VLANID_CNS);

    /* AUTODOC: restore prvTgfPortsArray */
    cpssOsMemCpy(prvTgfPortsArray, prvTgfRestoreCfg.portsArray,
                 sizeof(prvTgfRestoreCfg.portsArray));
}







/**************************************************************/
/*               Remote Ports By DSA Tag                     */
/**************************************************************/


/**
* @internal 
*           prvTgfDSARemotePhysicalPortMapConfigurationSet
*           function
* @endinternal
*
* @brief   Set test configuration for remote port over DSA tag.
*/
GT_VOID prvTgfDSARemotePhysicalPortMapConfigurationSet
(
    GT_VOID
)
{
    GT_U32 value = 1;
    GT_STATUS rc;

    /* set port type */
    rc = prvTgfCscdPortTypeGet(prvTgfDevNum, PRV_TGF_SOURCE_CASCADE_PORT_NUM_CNS, CPSS_PORT_DIRECTION_RX_E,
                                                        &(prvTgfRestoreCfg.portType0));
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdPortTypeGet");

    rc = prvTgfCscdPortTypeSet(prvTgfDevNum, CPSS_PORT_DIRECTION_RX_E, PRV_TGF_SOURCE_CASCADE_PORT_NUM_CNS,
                                                        CPSS_CSCD_PORT_DSA_MODE_1_WORD_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdPortTypeSet");


     /* set port type */
    rc = prvTgfCscdPortTypeGet(prvTgfDevNum, PRV_TGF_TARGET_CASCADE_PORT_NUM_CNS, CPSS_PORT_DIRECTION_TX_E,
                                                        &(prvTgfRestoreCfg.portType1));
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdPortTypeGet");

    rc = prvTgfCscdPortTypeSet(prvTgfDevNum, CPSS_PORT_DIRECTION_TX_E, PRV_TGF_TARGET_CASCADE_PORT_NUM_CNS,
                                                        CPSS_CSCD_PORT_DSA_MODE_1_WORD_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdPortTypeSet");


    prvTgfCommonRemotePhysicalPortMapConfigurationSet(PRV_TGF_PHYSICAL_PORT_BASE_CNS,NULL,GT_FALSE);

    /* set source port assignment mode */
    rc = prvTgfCscdMyPhysicalPortAssignmentModeGet(prvTgfDevNum, PRV_TGF_SOURCE_CASCADE_PORT_NUM_CNS,
                                                    &(prvTgfRestoreCfg.portAssignMode));
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdMyPhysicalPortAssignmentModeGet");

    rc = prvTgfCscdMyPhysicalPortAssignmentModeSet(prvTgfDevNum, PRV_TGF_SOURCE_CASCADE_PORT_NUM_CNS,
                            PRV_TGF_CSCD_INGRESS_PHYSICAL_PORT_ASSIGNMENT_MODE_DSA_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdMyPhysicalPortAssignmentModeSet");


    prvTgfRemoteTargetPhysicalPortConfigurationSet(PRV_TGF_REMOTE_PORT_TARGET_NUM_CNS,PRV_TGF_REMOTE_TARGET_PORT_CNS,GT_FALSE);

    /* global - per device - take only bits that relevant to the port number in the DSA tag and not to the dev number.*/
    rc = cpssDxChCscdCentralizedChassisMyPhysicalPortMappingDsaSrcLsbAmountGet(
        prvTgfDevNum, &(prvTgfRestoreCfg.srcDevLsbAmount) , &(prvTgfRestoreCfg.srcPortLsbAmount));
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChCscdCentralizedChassisMyPhysicalPortMappingDsaSrcLsbAmountGet: %d", prvTgfDevNum);

    rc = cpssDxChCscdCentralizedChassisMyPhysicalPortMappingDsaSrcLsbAmountSet(
        prvTgfDevNum, 0 /*srcDevLsbAmount*/, 4 /*srcPortLsbAmount*/);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChCscdCentralizedChassisMyPhysicalPortMappingDsaSrcLsbAmountSet: %d", prvTgfDevNum);

    rc = prvCpssDxChReadTableEntryField(
        prvTgfDevNum,
        CPSS_DXCH_SIP5_TABLE_HA_PHYSICAL_PORT_2_E,
        PRV_TGF_REMOTE_PORT_TARGET_NUM_CNS,
        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
        SIP5_HA_PHYSICAL_PORT_TABLE_2_FIELDS_CASCADE_PORT_ENABLE_E,
        PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
         &(prvTgfRestoreCfg.HaCscadeEnable));
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "SIP5_HA_PHYSICAL_PORT_TABLE_2_FIELDS_CASCADE_PORT_ENABLE_E: %d", PRV_TGF_TARGET_REMOTE_DEV_CNS);


    rc = prvCpssDxChWriteTableEntryField(
        prvTgfDevNum,
        CPSS_DXCH_SIP5_TABLE_HA_PHYSICAL_PORT_2_E,
        PRV_TGF_REMOTE_PORT_TARGET_NUM_CNS,
        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
        SIP5_HA_PHYSICAL_PORT_TABLE_2_FIELDS_CASCADE_PORT_ENABLE_E,
        PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
        value);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "SIP5_HA_PHYSICAL_PORT_TABLE_2_FIELDS_CASCADE_PORT_ENABLE_E: %d", PRV_TGF_TARGET_REMOTE_DEV_CNS);

    rc = prvCpssDxChReadTableEntryField(
        prvTgfDevNum,
        CPSS_DXCH_SIP5_TABLE_HA_PHYSICAL_PORT_2_E,
        PRV_TGF_REMOTE_PORT_TARGET_NUM_CNS,
        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
        SIP5_HA_PHYSICAL_PORT_TABLE_2_FIELDS_TRANS_FWD_TO_4B_FROM_CPU_DSA_E,
        PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
         &(prvTgfRestoreCfg.Ha4BytesFromCpu));
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "SIP5_HA_PHYSICAL_PORT_TABLE_2_FIELDS_TRANS_FWD_TO_4B_FROM_CPU_DSA_E: %d", PRV_TGF_TARGET_REMOTE_DEV_CNS);


    rc = prvCpssDxChWriteTableEntryField(
        prvTgfDevNum,
        CPSS_DXCH_SIP5_TABLE_HA_PHYSICAL_PORT_2_E,
        PRV_TGF_REMOTE_PORT_TARGET_NUM_CNS,
        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
        SIP5_HA_PHYSICAL_PORT_TABLE_2_FIELDS_TRANS_FWD_TO_4B_FROM_CPU_DSA_E,
        PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
        value);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "SIP5_HA_PHYSICAL_PORT_TABLE_2_FIELDS_TRANS_FWD_TO_4B_FROM_CPU_DSA_E: %d", PRV_TGF_TARGET_REMOTE_DEV_CNS);

}






/**
* @internal 
*           prvTgfDSARemotePhysicalPortMapTrafficGenerate
*           function
* @endinternal
*
* @brief   Generate and check traffic. 
*/
GT_VOID prvTgfDSARemotePhysicalPortMapTrafficGenerate
(
    GT_VOID
)
{

    GT_STATUS                   rc;
    CPSS_INTERFACE_INFO_STC iface          = {0};
    TGF_PACKET_STC          *packetInfoPtr = NULL;
    GT_U32                  actualNumOfPackets;
    GT_U32                  skipList[1] = {13};

    /* packetSize is changed , the forward DSA tag that was built in the packet removed before the flooding, 4 bytes CRC*/
    GT_U32 floodingTargetPortsPacketLength = (PRV_TGF_PACKET_DSA_LEN_CNS -4 + TGF_CRC_LEN_CNS);
    /*added: 4 bytes DSA tag (1 word) for target remote ports, removed: 4 bytes DSA tag of source remote port ( 4-4 = 0), 4 bytes CRC*/
    GT_U32 floodingCascadeTargetPortsPacketLength = (PRV_TGF_PACKET_DSA_LEN_CNS +TGF_CRC_LEN_CNS);
    /* the packet will be with crc (4 bytes)*/
    GT_U32 sourceCascadePortsPacketLength = (PRV_TGF_PACKET_DSA_LEN_CNS +TGF_CRC_LEN_CNS);

    cpssOsMemSet(&iface, 0, sizeof(iface));
    /* AUTODOC: GENERATE TRAFFIC: */
    PRV_UTF_LOG0_MAC("======= Setting packet with DSA tag 'forward' =======\n");

    prvTgfPacketDsaTagPart.dsaInfo.forward.srcHwDev          = PRV_TGF_SOURCE_REMOTE_DEV_CNS;
    prvTgfPacketDsaTagPart.dsaInfo.forward.source.portNum    = PRV_TGF_SOURCE_REMOTE_LOCAL_PORT_CNS;
    prvTgfPacketDsaTagPart.dsaInfo.forward.isTrgPhyPortValid = GT_FALSE;

    /* AUTODOC: setup Packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfPacketDsaInfo, PRV_TGF_BURST_COUNT_CNS, 0, NULL);
    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d, %d",
                                 prvTgfDevNum, PRV_TGF_BURST_COUNT_CNS, 0, NULL);

    PRV_UTF_LOG1_MAC("======= check packets from cascade port %d =======\n",PRV_TGF_TARGET_CASCADE_PORT_NUM_CNS);
    /* AUTODOC: Check trigger counters */
    prvTgfCommonRemotePhysicalPortMapCheck(floodingTargetPortsPacketLength, floodingCascadeTargetPortsPacketLength, sourceCascadePortsPacketLength);

    iface.type              = CPSS_INTERFACE_PORT_E;
    iface.devPort.hwDevNum  = prvTgfDevNum;
    iface.devPort.portNum   = PRV_TGF_TARGET_CASCADE_PORT_NUM_CNS;

    cpssOsMemSet(&prvTgfPacketDsaTagPart, 0, sizeof(prvTgfPacketDsaTagPart));
 
    prvTgfPacketDsaTagPart.dsaInfo.fromCpu.dstInterface.type = CPSS_INTERFACE_PORT_E;
    prvTgfPacketDsaTagPart.dsaInfo.fromCpu.dstInterface.devPort.portNum = PRV_TGF_REMOTE_TARGET_PORT_CNS;
    prvTgfPacketDsaTagPart.dsaInfo.fromCpu.dstInterface.devPort.hwDevNum = PRV_TGF_TARGET_REMOTE_DEV_CNS;
    prvTgfPacketDsaTagPart.commonParams.vid =       PRV_TGF_VLANID_CNS;

    prvTgfPacketDsaTagPart.dsaCommand = TGF_DSA_CMD_FROM_CPU_E;
    prvTgfPacketDsaTagPart.dsaType= TGF_DSA_1_WORD_TYPE_E;

    /* AUTODOC: check the captured packet */
    packetInfoPtr = &prvTgfPacketDsaInfo;

    rc = tgfTrafficGeneratorPortTxEthCaptureCompare(
        &iface,
        packetInfoPtr,
        (PRV_TGF_BURST_COUNT_CNS),/*numOfPackets*/
        0/* vfdNum */,
        NULL /* vfdArray */,
        skipList, /* bytesNum's skip list */
        1,    /* length of skip list */
        &actualNumOfPackets,
        NULL/* onFirstPacketNumTriggersBmpPtr */);

    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                        "tgfTrafficGeneratorPortTxEthCaptureCompare:"
                        " port = %d, rc = 0x%02X\n", iface.devPort.portNum, rc);

}


/**
* @internal 
*           prvTgfDSARemotePhysicalPortMapConfigurationRestore
*           function
* @endinternal
*
* @brief   Restore configuration
*/
GT_VOID prvTgfDSARemotePhysicalPortMapConfigurationRestore
(
    GT_VOID
)
{

    GT_STATUS                 rc;

    prvTgfCommonRemotePhysicalPortMapConfigurationRestore();

    rc = cpssDxChCscdCentralizedChassisMyPhysicalPortMappingDsaSrcLsbAmountSet(
        prvTgfDevNum, prvTgfRestoreCfg.srcDevLsbAmount , prvTgfRestoreCfg.srcPortLsbAmount);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChCscdCentralizedChassisMyPhysicalPortMappingDsaSrcLsbAmountSet: %d", prvTgfDevNum);

    rc = prvCpssDxChWriteTableEntryField(
        prvTgfDevNum,
        CPSS_DXCH_SIP5_TABLE_HA_PHYSICAL_PORT_2_E,
        PRV_TGF_REMOTE_PORT_TARGET_NUM_CNS,
        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
        SIP5_HA_PHYSICAL_PORT_TABLE_2_FIELDS_CASCADE_PORT_ENABLE_E,
        PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
        prvTgfRestoreCfg.HaCscadeEnable);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "SIP5_HA_PHYSICAL_PORT_TABLE_2_FIELDS_CASCADE_PORT_ENABLE_E: %d", PRV_TGF_TARGET_REMOTE_DEV_CNS);

    rc = prvCpssDxChWriteTableEntryField(
        prvTgfDevNum,
        CPSS_DXCH_SIP5_TABLE_HA_PHYSICAL_PORT_2_E,
        PRV_TGF_REMOTE_PORT_TARGET_NUM_CNS,
        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
        SIP5_HA_PHYSICAL_PORT_TABLE_2_FIELDS_TRANS_FWD_TO_4B_FROM_CPU_DSA_E,
        PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
        prvTgfRestoreCfg.Ha4BytesFromCpu);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "SIP5_HA_PHYSICAL_PORT_TABLE_2_FIELDS_TRANS_FWD_TO_4B_FROM_CPU_DSA_E: %d", PRV_TGF_TARGET_REMOTE_DEV_CNS);

}






/**************************************************************/
/*               Remote Ports By Vlan Tag                     */
/**************************************************************/



/**
* @internal 
*           prvTgfVlanRemotePhysicalPortMapConfigurationSet
*           function
* @endinternal
*
* @brief  Set test configuration for remote port over VLAN tag.
*/
GT_VOID prvTgfVlanRemotePhysicalPortMapConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS rc;
    GT_U8 tagArray[2]= {1, 1};
    prvTgfCommonRemotePhysicalPortMapConfigurationSet((PRV_TGF_PHYSICAL_PORT_BASE_CNS -3),&tagArray[0], GT_TRUE);

    /* set source port assignment mode */
    rc = prvTgfCscdMyPhysicalPortAssignmentModeGet(prvTgfDevNum, PRV_TGF_SOURCE_CASCADE_PORT_NUM_CNS,
                                                    &(prvTgfRestoreCfg.portAssignMode));
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdMyPhysicalPortAssignmentModeGet");

    rc = prvTgfCscdMyPhysicalPortAssignmentModeSet(prvTgfDevNum, PRV_TGF_SOURCE_CASCADE_PORT_NUM_CNS,
                            PRV_TGF_CSCD_INGRESS_PHYSICAL_PORT_ASSIGNMENT_MODE_VLAN_TAG_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdMyPhysicalPortAssignmentModeSet");

    rc = prvTgfRemotePhysicalPortVlanTagTpidGet(prvTgfDevNum, &(prvTgfRestoreCfg.vlanTpid));
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfRemotePhysicalPortVlanTagTpidGet");

    rc = prvTgfRemotePhysicalPortVlanTagTpidSet(prvTgfDevNum,prvTgfPacketVlanTagPart0.etherType);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfRemotePhysicalPortVlanTagTpidSet");

    prvTgfRemoteTargetPhysicalPortConfigurationSet(PRV_TGF_REMOTE_PORT_TARGET_NUM_CNS,PRV_TGF_REMOTE_TARGET_PORT_CNS,GT_TRUE);

}




/**
* @internal 
*           prvTgfVlanRemotePhysicalPortMapTrafficGenerate
*           function
* @endinternal
*
* @brief   Generate and check traffic
*/
GT_VOID prvTgfVlanRemotePhysicalPortMapTrafficGenerate
(
    GT_VOID
)
{

    GT_STATUS                   rc;
    /* packetSize is changed , the forward DSA tag that was built in the packet removed before the flooding, 4 bytes CRC*/
    GT_U32 floodingTargetPortsPacketLength = (PRV_TGF_PACKET_VLAN_LEN_CNS - TGF_VLAN_TAG_SIZE_CNS + TGF_CRC_LEN_CNS);
    /*added: 4 bytes DSA tag (1 word) for target remote ports, removed: 4 bytes DSA tag of source remote port ( 4-4 = 0), 4 bytes CRC*/
    GT_U32 floodingCascadeTargetPortsPacketLength = (PRV_TGF_PACKET_VLAN_LEN_CNS - TGF_VLAN_TAG_SIZE_CNS +TGF_CRC_LEN_CNS);
    /* the packet will be with crc (4 bytes)*/
    GT_U32 sourceCascadePortsPacketLength = (PRV_TGF_PACKET_VLAN_LEN_CNS +TGF_CRC_LEN_CNS);


    /* AUTODOC: GENERATE TRAFFIC: */
     /* use different 'etherType for vlan tagging' instead of 0x8100 */
    /* this will make our packet that has '0x8100' to be recognized as 'untagged'
      by the 'traffic generator' and also the CPSS will not override the ethertype
      of tag0 with the DSA tag */
    tgfTrafficGeneratorEtherTypeForVlanTagSet(TGF_ETHERTYPE_NON_VALID_TAG_CNS);

    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfPacketVlanInfo, PRV_TGF_BURST_COUNT_CNS, 0, NULL);
    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d, %d",
                                 prvTgfDevNum, PRV_TGF_BURST_COUNT_CNS, 0, NULL);


    PRV_UTF_LOG1_MAC("======= check packets from cascade port %d =======\n",PRV_TGF_TARGET_CASCADE_PORT_NUM_CNS);
    /* AUTODOC: Check trigger counters */
    /* the outmust vlan tag is ppoped for remote port assignment*/
    prvTgfCommonRemotePhysicalPortMapCheck(floodingTargetPortsPacketLength, floodingCascadeTargetPortsPacketLength, sourceCascadePortsPacketLength);

        /* restore default ethertype */
    tgfTrafficGeneratorEtherTypeForVlanTagSet(TGF_ETHERTYPE_8100_VLAN_TAG_CNS);

}


/**
* @internal 
*           prvTgfVlanRemotePhysicalPortMapConfigurationRestore
*           function
* @endinternal
*
* @brief   Restore configuration
*/
GT_VOID prvTgfVlanRemotePhysicalPortMapConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS rc;

    rc = prvTgfRemotePhysicalPortVlanTagTpidSet(prvTgfDevNum, prvTgfRestoreCfg.vlanTpid);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfRemotePhysicalPortVlanTagTpidSet");

    prvTgfCommonRemotePhysicalPortMapConfigurationRestore();

}


/**************************************************************/
/*               Remote Ports By E Tag                     */
/**************************************************************/



/**
* @internal 
*           prvTgfEtagRemotePhysicalPortMapConfigurationSet
*           function
* @endinternal
*
* @brief  Set test configuration for remote port over E tag.
*/
GT_VOID prvTgfEtagRemotePhysicalPortMapConfigurationSet
(
    GT_VOID
)
{

    GT_STATUS rc;
    GT_U8 tagArray[2]= {1, 1};

    prvTgfCommonRemotePhysicalPortMapConfigurationSet((PRV_TGF_PHYSICAL_PORT_BASE_CNS),tagArray,GT_TRUE);

    /* set source port assignment mode */
    rc = prvTgfCscdMyPhysicalPortAssignmentModeGet(prvTgfDevNum, PRV_TGF_SOURCE_CASCADE_PORT_NUM_CNS,
                                                    &(prvTgfRestoreCfg.portAssignMode));
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdMyPhysicalPortAssignmentModeGet");

    rc = prvTgfCscdMyPhysicalPortAssignmentModeSet(prvTgfDevNum, PRV_TGF_SOURCE_CASCADE_PORT_NUM_CNS,
                            PRV_TGF_CSCD_INGRESS_PHYSICAL_PORT_ASSIGNMENT_MODE_E_TAG_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdMyPhysicalPortAssignmentModeSet");


    prvTgfRemoteTargetPhysicalPortConfigurationSet(PRV_TGF_REMOTE_PORT_TARGET_NUM_CNS, PRV_TGF_REMOTE_TARGET_PORT_CNS, GT_TRUE);

}






/**
* @internal 
*           prvTgfEtagRemotePhysicalPortMapTrafficGenerate
*           function
* @endinternal
*
* @brief   Generate and check traffic
*/
GT_VOID prvTgfEtagRemotePhysicalPortMapTrafficGenerate
(
    GT_VOID
)
{

    GT_STATUS                   rc;
    /* the packet will be without the etag (6 bytes) and ether type ( 2 bytes) parts, with crc (4 bytes)*/
    GT_U32 floodingTargetPortsPacketLength = (PRV_TGF_PACKET_ETAG_LEN_CNS -6 -TGF_ETHERTYPE_SIZE_CNS +TGF_CRC_LEN_CNS);
    /* the packet will be with crc (4 bytes)*/
    GT_U32 sourceCascadePortsPacketLength = (PRV_TGF_PACKET_ETAG_LEN_CNS + TGF_CRC_LEN_CNS);

    /* AUTODOC: GENERATE TRAFFIC: */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfPacketEtagInfo, PRV_TGF_BURST_COUNT_CNS, 0, NULL);
    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d, %d",
                                 prvTgfDevNum, PRV_TGF_BURST_COUNT_CNS, 0, NULL);

    PRV_UTF_LOG1_MAC("======= check packets from cascade port %d =======\n",PRV_TGF_TARGET_CASCADE_PORT_NUM_CNS);

    /* AUTODOC: Check trigger counters */
    prvTgfCommonRemotePhysicalPortMapCheck(floodingTargetPortsPacketLength/* local ports*/,floodingTargetPortsPacketLength/* remote ports*/, sourceCascadePortsPacketLength );

}


/**
* @internal 
*           prvTgfEtagRemotePhysicalPortMapConfigurationRestore
*           function
* @endinternal
*
* @brief   Restore configuration
*/
GT_VOID prvTgfEtagRemotePhysicalPortMapConfigurationRestore
(
    GT_VOID
)
{
    prvTgfCommonRemotePhysicalPortMapConfigurationRestore();
}
