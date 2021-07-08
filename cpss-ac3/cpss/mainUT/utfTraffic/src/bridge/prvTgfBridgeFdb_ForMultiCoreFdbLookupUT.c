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
* @file prvTgfBridgeFdb_ForMultiCoreFdbLookupUT.c
*
* @brief enhanced UT for Bridge FDB - for multi port group fdb lookup
*
* tests according to design doc in documentum :
* http://docil.marvell.com/webtop/drl/objectId/0900dd88800a0077
*
* @version   15
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfTrunkGen.h>
#include <bridge/prvTgfBasicDynamicLearning.h>
#include <trunk/prvTgfTrunk.h>
#include <common/tgfCscdGen.h>
#include <utf/private/prvUtfExtras.h>
#include <bridge/prvTgfBridgeFdb_ForMultiCoreFdbLookupUT.h>
#ifdef CHX_FAMILY
    #include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#endif /*CHX_FAMILY*/

#define INDEX_2_CNS     2
#define INDEX_3_CNS     3

/* include the AppDemo to get info about the infrastructure */
#include <appDemo/sysHwConfig/gtAppDemoSysConfig.h>

extern GT_STATUS appDemoLionTrunkSrcPortHashMappingSet_ForMultiCoreFdbLookup
(
    IN  GT_U8                       boardRevId
);
/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/
#define LAST_NETWORK_PORT_INDEX_CNS       (N1_INDEX_E + (PRV_CPSS_PP_MAC(prvTgfDevNum)->portGroupsInfo.numOfPortGroups - 1))
/* default VLAN Id */
#define PRV_TGF_VLANID_CNS             2

/* VLAN Id = 1 */
#define PRV_TGF_VLANID_1_CNS            1

/* Trunk Id to use */
#define PRV_TGF_TRUNK_ID_CNS            13

/* another Trunk Id to use */
#define PRV_TGF_ANOTHER_TRUNK_ID_CNS    26

static GT_U32   debug_burstNum = 0;
/* default number of packets to send */
static GT_U32       prvTgfBurstCount = 1;

/* array of source MACs for the tests */
static TGF_MAC_ADDR prvTgfSaMacArr[] = {
                                        {0x00, 0x00, 0x00, 0x00, 0x00, 0x01},
                                        {0x00, 0x00, 0x00, 0x00, 0x00, 0x02},
                                        {0x00, 0x00, 0x00, 0x00, 0x00, 0x03},
                                        {0x00, 0x00, 0x00, 0x00, 0x00, 0x04},
                                        {0x00, 0x00, 0x00, 0x00, 0x00, 0x05},
                                        {0x00, 0x00, 0x00, 0x00, 0x00, 0x06},
                                        {0x00, 0x00, 0x00, 0x00, 0x00, 0x07},
                                        {0x00, 0x00, 0x00, 0x00, 0x00, 0x08}
                                        };

/******************************* Test packet **********************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part = {
    {0x00, 0x12, 0x34, 0x56, 0x78, 0x9a},               /* dstMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x33}                /* srcMac */
};
/* VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTagPart = {
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
    0x00, 0x00, 0x00, 0x00
};
/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart = {
    sizeof(prvTgfPayloadDataArr),                       /* dataLength */
    prvTgfPayloadDataArr                                /* dataPtr */
};

/* PARTS of tagged packet */
static TGF_PACKET_PART_STC prvTgfPacketTaggedPartArray[] = {
    {TGF_PACKET_PART_L2_E,       &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E, &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_PAYLOAD_E,  &prvTgfPacketPayloadPart}
};

/* PARTS of untagged packet */
static TGF_PACKET_PART_STC prvTgfPacketUnTaggedPartArray[] = {
    {TGF_PACKET_PART_L2_E,       &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_PAYLOAD_E,  &prvTgfPacketPayloadPart}
};

/* LENGTH of tagged packet */
#define PRV_TGF_PACKET_TAGGED_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + sizeof(prvTgfPayloadDataArr)

/* LENGTH of untagged packet */
#define PRV_TGF_PACKET_UNTAGGED_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + sizeof(prvTgfPayloadDataArr)

/* tagged PACKET to send */
static TGF_PACKET_STC prvTgfPacketInfoTagged = {
    PRV_TGF_PACKET_TAGGED_LEN_CNS,                                      /* totalLen */
    sizeof(prvTgfPacketTaggedPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketTaggedPartArray                                        /* partsArray */
};

/* untagged PACKET to send */
static TGF_PACKET_STC prvTgfPacketInfoUnTagged = {
    PRV_TGF_PACKET_UNTAGGED_LEN_CNS,                                      /* totalLen */
    sizeof(prvTgfPacketUnTaggedPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketUnTaggedPartArray                                        /* partsArray */
};

/******************************************************************************/
/* vlans array */
static GT_U16   prvTgfVansArr[]={1 , 3, 555, 1234, 4094};

/* current index into prvTgfVansArr[] */
static GT_U32   prvTgfCurrVlanIndex = 0;

/* do we use tagged frames ? or untagged */
static GT_BOOL  prvTgfUseTaggedFrame = GT_FALSE;

/******************************************************************************\
 *                            Private variables                               *
\******************************************************************************/

/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

typedef enum{
    PRV_TGF_FDB_REGULAR_MODE_E,
    PRV_TGF_FDB_DOUBLE_MODE_E,
    PRV_TGF_FDB_QUAD_MODE_E
}PRV_TGF_FDB_MODE_ENT;

static  PRV_TGF_FDB_MODE_ENT fdbMode;

/* indication that flooding expected */
#define PRV_TGF_FLOODING_CNS    0xFFFFFFFF

/* test's specific ports array */
static GT_U8  prvTgfSpecificPortArray[] = {
                                           0   ,    18,    34,    48,/*uplinks in port groups 0..3*/
                                           2+64, 18+64, 34+64, 58+64,/*uplinks in port groups 4..7*/
                                           4   ,    22,    40,    50,/*network ports in port groups 0..3*/
                                           6+64, 22+64, 40+64, 52+64 /*network ports in port groups 4..7*/
                                           };

/* bmp of ports from prvTgfSpecificPortArray[] that are tested */
static GT_U32  testedPortsBmp = 0x00000F0F;

/* convert index from U1_INDEX_E..N8_INDEX_E to index in array of prvTgfPortsArray[] */
#define PORT_INDEX_CONVERT_MAC(index)    \
    (((index) < N1_INDEX_E) ? /*uplink*/ (index) : ((index) - N1_INDEX_E) + PRV_CPSS_PP_MAC(prvTgfDevNum)->portGroupsInfo.numOfPortGroups)

/**
* @internal tgfBridgeFdbRestore_ForMultiCoreFdbLookup function
* @endinternal
*
* @brief   Restore_ForMultiCoreFdbLookup test configuration
*/
static GT_VOID tgfBridgeFdbRestore_ForMultiCoreFdbLookup
(
    void
)
{
    GT_STATUS   rc;
    GT_TRUNK_ID trunkIdArr[2] = {PRV_TGF_TRUNK_ID_CNS,PRV_TGF_ANOTHER_TRUNK_ID_CNS};
    GT_U32  ii;
    GT_U32  portIter;
    GT_U32  numOfValid = 0 ;/* number of valid entries in the FDB */
    GT_U32  numOfSkip = 0; /* number of skipped entries in the FDB */
    GT_U32  numOfValidMinusSkip=0;/* number of (valid - skip) entries in the FDB*/
    GT_U32  maxFlush;/* up to 10 times flush FDB , due to still processing of AppDemo or Asic simulation */
#ifdef ASIC_SIMULATION
    maxFlush = 10;
#else
    maxFlush = 1;
#endif /*ASIC_SIMULATION*/

    PRV_UTF_LOG0_MAC("======= tgfBridgeFdbRestore_ForMultiCoreFdbLookup : flush FDB =======\n");

    currPortGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;

    do{
        /* delete mac addresses , include static */
        rc = prvTgfBrgFdbFlush(GT_TRUE);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

        /* Check FDB capacity (FDB should be empty) */
        /* get FDB counters */
        rc = prvTgfBrgFdbCount(&numOfValid,&numOfSkip,NULL,NULL,NULL);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

        numOfValidMinusSkip = numOfValid - numOfSkip;

        maxFlush--;

        if(numOfValidMinusSkip && maxFlush)
        {
            cpssOsTimerWkAfter(1000);
            utfPrintKeepAlive();
        }
    }while(numOfValidMinusSkip && maxFlush);

    UTF_VERIFY_EQUAL1_STRING_MAC(0, numOfValidMinusSkip,
                    "FDB not empty , got [%d] entries ",
                    numOfValidMinusSkip);

    PRV_UTF_LOG0_MAC("======= restore aging timeout =======\n");
    /* Restore_ForMultiCoreFdbLookup 300 seconds default */
    rc = prvTgfBrgFdbAgingTimeoutSet(300);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    PRV_UTF_LOG0_MAC("======= restore trigger action mode =======\n");
    /* set trigger aging */
    rc = prvTgfBrgFdbMacTriggerModeSet(PRV_TGF_ACT_TRIG_E);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    PRV_UTF_LOG0_MAC("======= restore trunks =======\n");
    /* clear the 2 trunks */
    for(ii = 0 ; ii < 2 ; ii++)
    {
        CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkIdArr[ii]);

        rc = prvTgfTrunkMembersSet(
            trunkIdArr[ii] /*trunkId*/,
            0 /*numOfEnabledMembers*/,
            (CPSS_TRUNK_MEMBER_STC*)NULL /*enabledMembersArray*/,
            0 /*numOfDisabledMembers*/,
            (CPSS_TRUNK_MEMBER_STC*)NULL  /*disabledMembersArray*/);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
    }

    PRV_UTF_LOG0_MAC("======= clear ports counters =======\n");
    /* clear also counters at end of test */
    for(portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevsArray[portIter],
                                    prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
    }

    PRV_UTF_LOG0_MAC("======= tgfBridgeFdbRestore_ForMultiCoreFdbLookup :end =======\n");
}


/**
* @internal prvTgfMultiPortGroupInit_ForMultiCoreFdbLookup function
* @endinternal
*
* @brief   function init the DB of the multi-port groups
*/
GT_VOID prvTgfMultiPortGroupInit_ForMultiCoreFdbLookup
(
    GT_VOID
)
{
    GT_U8   devNum = prvTgfDevsArray[0];
    GT_U32  portIter;
    GT_STATUS rc = GT_OK;
    GT_U32  portArray[32];
    GT_U8  jj;

    /* map input params */
    /* state that the test can use 'per port group' indication */
    usePortGroupsBmp = GT_TRUE;

    testedPortsBmp = (BIT_0 << PRV_CPSS_PP_MAC(devNum)->portGroupsInfo.numOfPortGroups) - 1;
    /* each port group tested with 2 ports */
    testedPortsBmp |= testedPortsBmp << N1_INDEX_E;

    jj = 0;

    for(portIter = 0 ; portIter < LAST_INDEX_E ; portIter++)
    {
        if((testedPortsBmp & (BIT_0 << portIter)) == 0)
        {
            continue;
        }
        portArray[jj++] = prvTgfSpecificPortArray[portIter];
    }

    /* set test's specific port array */
    rc = prvTgfDefPortsArraySet(portArray, jj);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    PRV_UTF_LOG0_MAC("======= prvTgfMultiPortGroupInit_ForMultiCoreFdbLookup : clear ports counters =======\n");

    /* clear counters at start of test (and at the end of test) */
    for(portIter = 0; portIter < prvTgfPortsNum ; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevsArray[portIter],
                                    prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
    }

    /* according to the number of unified port groups :
       the code supports Lion and Lion2 logic */
    switch(appDemoPpConfigList[devNum].portGroupsInfo[0].unifiedFdbPortGroupsBmp & 0xF)
    {
        case BIT_0:
            fdbMode = PRV_TGF_FDB_QUAD_MODE_E;
            break;
        case (BIT_0 | BIT_2):
            fdbMode = PRV_TGF_FDB_DOUBLE_MODE_E;
            break;
        default:

            /* !!! not supported for those UTs !!! */
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, GT_NOT_SUPPORTED);
            return;
    }

    PRV_UTF_LOG0_MAC("======= prvTgfMultiPortGroupInit_ForMultiCoreFdbLookup : end =======\n");

}

/**
* @internal tgfBridgeFdbAddDeleteEntryFromUplinkTrafficSend function
* @endinternal
*
* @brief   send packets and check that expected egress port got the traffic
*         (also check that other network ports not get the traffic)
* @param[in] senderIndex              - index in prvTgfDevPortArray2[] to state the port to sent traffic from
*                                      expectedIndex - index in prvTgfDevPortArray2[] to state the port expected
*                                      to receive the traffic
*                                      when value is PRV_TGF_FLOODING_CNS --> flooding to network ports
*                                       None
*/
static GT_VOID tgfBridgeFdbAddDeleteEntryFromUplinkTrafficSend
(
    IN GT_U32 senderIndex,
    IN GT_U32 expectedPortIndex
)
{
    GT_STATUS   rc;
    CPSS_PORT_MAC_COUNTER_SET_STC portCntrs;/* counters of port */
    GT_U32  portIter;
    GT_U32  startIndex,endIndex;
    GT_U32  expectedCounterValue = 0;  /* expected counter value */
    TGF_PACKET_STC  *packetInfoPtr;
    GT_U32  index;/*index to access prvTgfDevsArray[] and prvTgfPortsArray[] */

    /* reset needed network port counters */
    if(expectedPortIndex == PRV_TGF_FLOODING_CNS)
    {
        startIndex = N1_INDEX_E;
        endIndex = LAST_NETWORK_PORT_INDEX_CNS;
    }
    else
    {
        startIndex = expectedPortIndex;
        endIndex = startIndex;

    }

    PRV_UTF_LOG0_MAC("======= tgfBridgeFdbAddDeleteEntryFromUplinkTrafficSend : clear ports counters =======\n");
    for (portIter = startIndex; portIter <= endIndex; portIter++)
    {
        index = PORT_INDEX_CONVERT_MAC(portIter);

        rc = prvTgfResetCountersEth(prvTgfDevsArray[index], prvTgfPortsArray[index]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevsArray[index], prvTgfPortsArray[index]);
    }

    /* set source MAC address in Packet to defer between the uplinks  */
    cpssOsMemCpy(prvTgfPacketL2Part.saMac, prvTgfSaMacArr[PORT_INDEX_CONVERT_MAC(senderIndex)],
                 sizeof(prvTgfPacketL2Part.saMac));


    if(prvTgfUseTaggedFrame == GT_TRUE)
    {
        /* send packets in the needed vlan */
        prvTgfPacketVlanTagPart.vid = prvTgfVansArr[prvTgfCurrVlanIndex];
        packetInfoPtr = &prvTgfPacketInfoTagged;
    }
    else
    {
        packetInfoPtr = &prvTgfPacketInfoUnTagged;
    }

    PRV_UTF_LOG0_MAC("======= setup packets =======\n");
    /* setup Packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevsArray[PORT_INDEX_CONVERT_MAC(senderIndex)], packetInfoPtr, prvTgfBurstCount, 0, NULL);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d, %d",
            prvTgfDevsArray[PORT_INDEX_CONVERT_MAC(senderIndex)], prvTgfPortsArray[PORT_INDEX_CONVERT_MAC(senderIndex)]);

    PRV_UTF_LOG0_MAC("======= send packets =======\n");
    /* send Packet */
    rc = prvTgfStartTransmitingEth(prvTgfDevsArray[PORT_INDEX_CONVERT_MAC(senderIndex)], prvTgfPortsArray[PORT_INDEX_CONVERT_MAC(senderIndex)]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, &d",
                                 prvTgfDevsArray[PORT_INDEX_CONVERT_MAC(senderIndex)], prvTgfPortsArray[PORT_INDEX_CONVERT_MAC(senderIndex)]);


    PRV_UTF_LOG0_MAC("======= check expected counters on network ports =======\n");
    /* check expected counters on network ports */
    for (portIter = N1_INDEX_E; portIter <= LAST_NETWORK_PORT_INDEX_CNS; portIter++)
    {
        index = PORT_INDEX_CONVERT_MAC(portIter);

        rc = prvTgfReadPortCountersEth(prvTgfDevsArray[index], prvTgfPortsArray[index], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevsArray[index], prvTgfPortsArray[index]);

        if(portIter == expectedPortIndex ||
           expectedPortIndex == PRV_TGF_FLOODING_CNS)
        {
            /* this port should get packet */
            expectedCounterValue = prvTgfBurstCount;
        }
        else
        {
            /* this port should not get packet */
            expectedCounterValue = 0;
        }

        UTF_VERIFY_EQUAL2_STRING_MAC(expectedCounterValue, portCntrs.goodPktsSent.l[0],
                                     "expected [%d] in goodPktsSent but got [%d]",
                                     expectedCounterValue, portCntrs.goodPktsSent.l[0]);
    }

    PRV_UTF_LOG0_MAC("======= tgfBridgeFdbAddDeleteEntryFromUplinkTrafficSend : ended =======\n");
}

/**
* @internal tgfBridgeFdbAddDeleteEntryRun_ForMultiCoreFdbLookup function
* @endinternal
*
* @brief   4.1    Test 3: check add/delete (by index/message) of FDB entry
*         (Covered API: FDB :
*         1.    cpssDxChBrgFdbPortGroupMacEntrySet
*         a.    cpssDxChBrgFdbPortGroupFromCpuAuMsgStatusGet
*         2.    cpssDxChBrgFdbPortGroupMacEntryDelete
*         a.    cpssDxChBrgFdbPortGroupFromCpuAuMsgStatusGet
*         3.    cpssDxChBrgFdbPortGroupMacEntryWrite
*         4.    cpssDxChBrgFdbPortGroupMacEntryRead
*         5.    cpssDxChBrgFdbPortGroupMacEntryInvalidate
*         )
*         1.    check add/delete (by index/message) of FDB entry :
*         a.    add the same FDB entry (mac + vlan) , but with different destination port to each port group , meaning:
*         i.    port group 0 - FDB entry , on port N1
*         ii.    port group 1 - FDB entry , on port N2
*         iii.    port group 2 - FDB entry , on port N3
*         iv.    port group 3 - FDB entry , on port N4
*         b.    Refresh FDB to check that add is ok.
*         c.    NOTEs:
*         i.    when add/delete 'by message' need to call cpssDxChBrgFdbPortGroupFromCpuAuMsgStatusGet to check that operation succeeded on the needed port groups
*         b.    send traffic from all uplinks to this mac address :
*         a.    In 128K mode : check that egress N1
*         b.    In 64K mode :
*         i.    when Uplink is U1,U2
*         1.    check that egress N1
*         ii.    else
*         1.    check that egress N3
*         c.    delete the entry from port group 0
*         d.    Refresh FDB to check that delete is ok.
*         e.    send traffic from all uplinks to this mac address :
*         a.    In 128K mode : check that egress N2
*         b.    In 64K mode :
*         i.    when Uplink is U1,U2
*         1.    check that egress N2
*         ii.    else
*         1.    check that egress N3
*         f.    delete the entry from port group 1
*         g.    Refresh FDB to check that delete is ok.
*         h.    send traffic from all uplinks to this mac address :
*         a.    In 128K mode : check that egress N3
*         b.    In 64K mode :
*         i.    when Uplink is U1,U2
*         1.    check that flooding
*         ii.    else
*         1.    check that egress N3
*         i.    delete the entry from port group 2
*         j.    Refresh FDB to check that delete is ok.
*         k.    send traffic from all uplinks to this mac address :
*         a.    In 128K mode : check that egress N4
*         b.    In 64K mode :
*         i.    when Uplink is U1,U2
*         1.    check that flooding
*         ii.    else
*         1.    check that egress N4
*         l.    delete the entry from port group 3
*         m.    Refresh FDB to check that delete is ok.
*         n.    send multi-destination traffic from all uplinks to this mac address , check flooding
*/
GT_VOID tgfBridgeFdbAddDeleteEntryRun_ForMultiCoreFdbLookup
(
    GT_VOID
)
{
    GT_STATUS rc;
    PRV_TGF_BRG_MAC_ENTRY_STC      macEntry = PRV_TGF_BRG_FDB_MAC_ENTRY_DEFAULT_MAC;
    GT_U32      portIndex = 0;/*port index*/
    GT_U32      uplinkIndex;/*uplink index*/
    GT_U32      expectedPortIndex;/*expected network port index*/
    GT_U32      ii;
    GT_U32      fdbIndex;/* index in FDB*/
    GT_U32      prevFdbIndex = 0;/* index in FDB*/
    PRV_TGF_BRG_MAC_ENTRY_STC      tmpMacEntry;
    GT_U32      index;
    GT_U32      numPortGroupsInChain;/* number of port groups in chain */

     /* Set the FDB entry mac address */
    cpssOsMemCpy(macEntry.key.key.macVlan.macAddr.arEther,
                 &prvTgfPacketL2Part.daMac,
                 sizeof(TGF_MAC_ADDR));

    macEntry.key.key.macVlan.vlanId         = PRV_TGF_VLANID_1_CNS;
    macEntry.dstInterface.type              = CPSS_INTERFACE_PORT_E;

    PRV_UTF_LOG0_MAC("======= tgfBridgeFdbAddDeleteEntryRun_ForMultiCoreFdbLookup : set FDB entries =======\n");
    /*
        a. add the same FDB entry (mac + vlan) , but with different destination port to each port group , meaning:
            i.   port group 0 - FDB entry , on port N1
            ii.  port group 1 - FDB entry , on port N2
            iii. port group 2 - FDB entry , on port N3
            iv.  port group 3 - FDB entry , on port N4
    */

    for(ii = 0 ; ii < PRV_CPSS_PP_MAC(prvTgfDevNum)->portGroupsInfo.numOfPortGroups ; ii++)
    {
        portIndex = ii + N1_INDEX_E;

        index = PORT_INDEX_CONVERT_MAC(portIndex);

        macEntry.dstInterface.devPort.hwDevNum    = prvTgfDevsArray[index];
        macEntry.dstInterface.devPort.portNum   = prvTgfPortsArray[index];

        currPortGroupsBmp = BIT_0 << ii;
        rc = prvTgfBrgFdbMacEntrySet(&macEntry);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
    }


    PRV_UTF_LOG0_MAC("======= : Refresh FDB to check that add is ok =======\n");
    /*Refresh FDB to check that add is ok*/
    for(ii = 0 ; ii < PRV_CPSS_PP_MAC(prvTgfDevNum)->portGroupsInfo.numOfPortGroups ; ii++)
    {
        portIndex = ii + N1_INDEX_E;

        index = PORT_INDEX_CONVERT_MAC(portIndex);

        currPortGroupsBmp = BIT_0 << ii;
        rc = prvTgfBrgFdbMacEntryFind(&macEntry.key,&fdbIndex,NULL,NULL,&tmpMacEntry);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

        UTF_VERIFY_EQUAL0_PARAM_MAC(CPSS_INTERFACE_PORT_E,
            tmpMacEntry.dstInterface.type);
        UTF_VERIFY_EQUAL0_PARAM_MAC(prvTgfDevsArray[index],
            tmpMacEntry.dstInterface.devPort.hwDevNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(prvTgfPortsArray[index],
            tmpMacEntry.dstInterface.devPort.portNum);

        if(ii == 0)
        {
            prevFdbIndex = fdbIndex;
        }
        else
        {
            UTF_VERIFY_EQUAL0_PARAM_MAC(fdbIndex, prevFdbIndex);
        }
    }

    /*
        b.        send traffic from all uplinks to this mac address :
            a.        In 128K mode : check that egress N1
            b.        In 64K mode :
                i.        when Uplink is U1,U2
                    1.        check that egress N1
                ii.        else
                    1.        check that egress N3
        c.        delete the entry from port group 0
        d.        Refresh FDB to check that delete is ok.
        e.        send traffic from all uplinks to this mac address :
            a.        In 128K mode : check that egress N2
            b.        In 64K mode :
                i.        when Uplink is U1,U2
                    1.        check that egress N2
                ii.        else
                    1.        check that egress N3
        f.        delete the entry from port group 1
        g.        Refresh FDB to check that delete is ok.
        h.        send traffic from all uplinks to this mac address :
            a.        In 128K mode : check that egress N3
            b.        In 64K mode :
                i.        when Uplink is U1,U2
                    1.        check that flooding
                ii.        else
                    1.        check that egress N3
        i.        delete the entry from port group 2
        j.        Refresh FDB to check that delete is ok.
        k.        send traffic from all uplinks to this mac address :
            a.        In 128K mode : check that egress N4
            b.        In 64K mode :
                i.        when Uplink is U1,U2
                    1.        check that flooding
                ii.        else
                    1.        check that egress N4
        l.        delete the entry from port group 3
        m.        Refresh FDB to check that delete is ok.
        n.        send multi-destination traffic from all uplinks to this mac address , check flooding
    */

    PRV_UTF_LOG0_MAC("======= : send traffic from all uplinks to this mac address =======\n");

    switch(fdbMode)
    {
        case PRV_TGF_FDB_QUAD_MODE_E:
            numPortGroupsInChain = 4;
            break;
        case PRV_TGF_FDB_DOUBLE_MODE_E:
            numPortGroupsInChain = 2;
            break;
        default:
            return;
    }

/* double FDB mode :

   uplink index :  0                    1                   2                   3                   4                   5                   6                   7

mac found in:0     SAME_PORT_GROUP_E    SAME_PORT_GROUP_E   SAME_PORT_GROUP_E   SAME_PORT_GROUP_E   SAME_PORT_GROUP_E   SAME_PORT_GROUP_E   SAME_PORT_GROUP_E   SAME_PORT_GROUP_E
             1     1                    SAME_PORT_GROUP_E   SAME_PORT_GROUP_E   SAME_PORT_GROUP_E   SAME_PORT_GROUP_E   SAME_PORT_GROUP_E   SAME_PORT_GROUP_E   SAME_PORT_GROUP_E
             2     FLOOD_E              FLOOD_E             SAME_PORT_GROUP_E   SAME_PORT_GROUP_E   SAME_PORT_GROUP_E   SAME_PORT_GROUP_E   SAME_PORT_GROUP_E   SAME_PORT_GROUP_E
             3     FLOOD_E              FLOOD_E             3                   SAME_PORT_GROUP_E   SAME_PORT_GROUP_E   SAME_PORT_GROUP_E   SAME_PORT_GROUP_E   SAME_PORT_GROUP_E
             4     FLOOD_E              FLOOD_E             FLOOD_E             FLOOD_E             SAME_PORT_GROUP_E   SAME_PORT_GROUP_E   SAME_PORT_GROUP_E   SAME_PORT_GROUP_E
             5     FLOOD_E              FLOOD_E             FLOOD_E             FLOOD_E             5                   SAME_PORT_GROUP_E   SAME_PORT_GROUP_E   SAME_PORT_GROUP_E
             6     FLOOD_E              FLOOD_E             FLOOD_E             FLOOD_E             FLOOD_E             FLOOD_E             SAME_PORT_GROUP_E   SAME_PORT_GROUP_E
             7     FLOOD_E              FLOOD_E             FLOOD_E             FLOOD_E             FLOOD_E             FLOOD_E             7                   SAME_PORT_GROUP_E

*/

/* quad FDB mode :

   uplink index :  0                    1                   2                   3                   4                   5                   6                   7

mac found in:0     SAME_PORT_GROUP_E    SAME_PORT_GROUP_E   SAME_PORT_GROUP_E   SAME_PORT_GROUP_E   SAME_PORT_GROUP_E   SAME_PORT_GROUP_E   SAME_PORT_GROUP_E   SAME_PORT_GROUP_E
             1     1                    SAME_PORT_GROUP_E   SAME_PORT_GROUP_E   SAME_PORT_GROUP_E   SAME_PORT_GROUP_E   SAME_PORT_GROUP_E   SAME_PORT_GROUP_E   SAME_PORT_GROUP_E
             2     2                    2                   SAME_PORT_GROUP_E   SAME_PORT_GROUP_E   SAME_PORT_GROUP_E   SAME_PORT_GROUP_E   SAME_PORT_GROUP_E   SAME_PORT_GROUP_E
             3     3                    3                   3                   SAME_PORT_GROUP_E   SAME_PORT_GROUP_E   SAME_PORT_GROUP_E   SAME_PORT_GROUP_E   SAME_PORT_GROUP_E
             4     FLOOD_E              FLOOD_E             FLOOD_E             FLOOD_E             SAME_PORT_GROUP_E   SAME_PORT_GROUP_E   SAME_PORT_GROUP_E   SAME_PORT_GROUP_E
             5     FLOOD_E              FLOOD_E             FLOOD_E             FLOOD_E             5                   SAME_PORT_GROUP_E   SAME_PORT_GROUP_E   SAME_PORT_GROUP_E
             6     FLOOD_E              FLOOD_E             FLOOD_E             FLOOD_E             6                   6                   SAME_PORT_GROUP_E   SAME_PORT_GROUP_E
             7     FLOOD_E              FLOOD_E             FLOOD_E             FLOOD_E             7                   7                   7                   SAME_PORT_GROUP_E

*/


    for(ii = 0 ; ii <= (PRV_CPSS_PP_MAC(prvTgfDevNum)->portGroupsInfo.numOfPortGroups + 1) ; ii++)
    {
        for(uplinkIndex = U1_INDEX_E ;
            uplinkIndex < (U1_INDEX_E + PRV_CPSS_PP_MAC(prvTgfDevNum)->portGroupsInfo.numOfPortGroups);
            uplinkIndex++)
        {
            switch(fdbMode)
            {
                case PRV_TGF_FDB_QUAD_MODE_E:
                    if(ii <= uplinkIndex)
                    {
                        /* traffic egress from same ingress port group */
                        expectedPortIndex = uplinkIndex + N1_INDEX_E;
                    }
                    else if((ii < 4 && uplinkIndex < 4) ||
                            (uplinkIndex >= 4 && ii >= 4 && ii < 8))
                    {
                        expectedPortIndex = ii + N1_INDEX_E;
                    }
                    else
                    {
                        expectedPortIndex = PRV_TGF_FLOODING_CNS;
                    }
                    break;
                case PRV_TGF_FDB_DOUBLE_MODE_E:
                    if(ii <= uplinkIndex)
                    {
                        /* traffic egress from same ingress port group */
                        expectedPortIndex = uplinkIndex + N1_INDEX_E;
                    }
                    else if((0 == (uplinkIndex & 1)) && (ii == (uplinkIndex + 1)))
                    {
                        /* match on the second port group with other port destination */
                        expectedPortIndex = ii + N1_INDEX_E;
                    }
                    else
                    {
                        expectedPortIndex = PRV_TGF_FLOODING_CNS;
                    }
                    break;
                default:
                    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, GT_NOT_SUPPORTED);
                    return;
            }

            /* send packets and check that expected egress port got the traffic
               (also check that other network ports not get the traffic) */
            tgfBridgeFdbAddDeleteEntryFromUplinkTrafficSend(uplinkIndex,expectedPortIndex);
        }

        if(ii < PRV_CPSS_PP_MAC(prvTgfDevNum)->portGroupsInfo.numOfPortGroups)
        {
            /* need to delete the FDB entry from the current port group */
            currPortGroupsBmp = BIT_0 << ii;
            rc = prvTgfBrgFdbMacEntryDelete(&macEntry.key);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

            /* check that entry not found any more */
            rc = prvTgfBrgFdbMacEntryFind(&macEntry.key,&fdbIndex,NULL,NULL,NULL);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_NOT_FOUND, rc);
        }

    }

    PRV_UTF_LOG0_MAC("======= tgfBridgeFdbAddDeleteEntryRun_ForMultiCoreFdbLookup : ended =======\n");


    TGF_PARAM_NOT_USED(numPortGroupsInChain);

    return;
}



/**
* @internal tgfBridgeFdbAddDeleteEntryRestore_ForMultiCoreFdbLookup function
* @endinternal
*
* @brief   Restore_ForMultiCoreFdbLookup test configuration
*/
GT_VOID tgfBridgeFdbAddDeleteEntryRestore_ForMultiCoreFdbLookup
(
    GT_VOID
)
{
    tgfBridgeFdbRestore_ForMultiCoreFdbLookup();
}


/**
* @internal tgfBridgeFdbAgingAppDemoRun_ForMultiCoreFdbLookup function
* @endinternal
*
* @brief   4.2    Test 4: check aging of mac addresses - AppDemo behavior
*         (Covered API: FDB :
*         1.    cpssDxChBrgFdbPortGroupMacEntrySet
*         a.    cpssDxChBrgFdbPortGroupFromCpuAuMsgStatusGet
*         2.    cpssDxChBrgFdbPortGroupMacEntryDelete
*         b.    cpssDxChBrgFdbPortGroupFromCpuAuMsgStatusGet
*         3.    cpssDxChBrgFdbPortGroupMacEntryWrite
*         4.    cpssDxChBrgFdbPortGroupMacEntryRead
*         )
*         1.    check aging of mac addresses.
*         a.    Set aging to 'trigger mode'
*         b.    Set aging timeout to 10 seconds (2 loops are 20 seconds)
*         c.    Flush FDB (trigger action 'delete') ' done on all port groups
*         d.    Add (by message /index) FDB entry on port N1 to all 4 port groups
*         i.    BUT on port group 0 (where N1 is 'local port') , set as 'dynamic' (not 'static')
*         ii.    On ALL other port groups , set as 'static' !!!
*         e.    Add another 3 FDB entries to each port group that associated with ports N2,N3,N4 , with same logic as done from N1 :
*         iii.    on port N2 to all 4 port groups
*         1.    BUT on port group 1 (where N2 is 'local port') , set as 'dynamic' (not 'static')
*         2.    On ALL other port groups , set as 'static' !!!
*         iv.    on port N3 to all 4 port groups
*         3.    BUT on port group 2 (where N3 is 'local port') , set as 'dynamic' (not 'static')
*         4.    On ALL other port groups , set as 'static' !!!
*         v.    on port N4 to all 4 port groups
*         5.    BUT on port group 3 (where N4 is 'local port') , set as 'dynamic' (not 'static')
*         6.    On ALL other port groups , set as 'static' !!!
*         f.    Set age without removal
*         g.    Set aging to 'auto aging'
*         h.    Wait 22 seconds ((2 age time) + 10%)
*         i.    Check that entry deleted from all port groups !!! (even the 3 port groups that hold 'static') ' because the AppDemo delete 'aged out' entry from all port groups !
*         j.    Repeat the test with next change :
*         a.    The entries that should be set as 'static' are ignored. (Not to set as dynamic or static)
* @param[in] useStatic                - when GT_TRUE --> Run_ForMultiCoreFdbLookup test as described above.
*                                      when GT_FALSE -->
*                                      j.        Run_ForMultiCoreFdbLookup the test with next change :
*                                      a.        The entries that should be set as 'static' are ignored. (Not to set as dynamic or static)
* @param[in] forceLessThenHalfSleepTime - when GT_FALSE - do full aging wait of 22 seconds
*                                      when GT_TRUE  - do less then half sleep time of 9 seconds
*                                      check that entries not deleted.
*                                       None
*/
GT_VOID tgfBridgeFdbAgingAppDemoRun_ForMultiCoreFdbLookup
(
    IN  GT_BOOL useStatic,
    IN  GT_BOOL forceLessThenHalfSleepTime
)
{
    GT_STATUS rc;
    PRV_TGF_BRG_MAC_ENTRY_STC      macEntry = PRV_TGF_BRG_FDB_MAC_ENTRY_DEFAULT_MAC;
    GT_U32      portIndex = 0;/*port index*/
    GT_U32      ii,jj;
    GT_U32      fdbIndex;/* index in FDB*/
    GT_U32      agingTime = 10;/* 10 seconds */
    GT_U32      sleepTime = (agingTime * 2 * 11) / 10;/*sleepTime = ((2 * age time) + 10%)*/
    GT_U32      lessThenHalfSleepTime = (agingTime * 9) / 10;/* less then half sleepTime = ((age time) - 10%)*/
    GT_U32      index;

    utfGeneralStateMessageSave(INDEX_2_CNS,"useStatic=[%d],forceLessThenHalfSleepTime[%d]",
        useStatic , forceLessThenHalfSleepTime);

    if(forceLessThenHalfSleepTime == GT_TRUE)
    {
        sleepTime = lessThenHalfSleepTime;
    }

    PRV_UTF_LOG0_MAC("======= tgfBridgeFdbAgingAppDemoRun_ForMultiCoreFdbLookup : set trigger mode =======\n");

    /*a.        Set aging to 'trigger mode'*/
    rc = prvTgfBrgFdbMacTriggerModeSet(PRV_TGF_ACT_TRIG_E);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    PRV_UTF_LOG0_MAC("======= : set aging time =======\n");
    /*b.        Set aging timeout to 10 seconds (2 loops are 20 seconds)*/
    rc = prvTgfBrgFdbAgingTimeoutSet(agingTime);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    PRV_UTF_LOG0_MAC("======= : flush FDB =======\n");
    /*c.        Flush FDB (trigger action 'delete') ' done on all port groups*/
    rc = prvTgfBrgFdbFlush(GT_FALSE);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    /*d.        Add (by message /index) FDB entry on port N1 to all 4 port groups
        i.        BUT on port group 0 (where N1 is 'local port') , set as 'dynamic' (not 'static')
        ii.        On ALL other port groups , set as 'static' !!!
      e.        Add another 3 FDB entries to each port group that associated with ports N2,N3,N4 , with same logic as done from N1 :
        iii.        on port N2 to all 4 port groups
            1.        BUT on port group 1 (where N2 is 'local port') , set as 'dynamic' (not 'static')
            2.        On ALL other port groups , set as 'static' !!!
        iv.        on port N3 to all 4 port groups
            3.        BUT on port group 2 (where N3 is 'local port') , set as 'dynamic' (not 'static')
            4.        On ALL other port groups , set as 'static' !!!
        v.        on port N4 to all 4 port groups
            5.        BUT on port group 3 (where N4 is 'local port') , set as 'dynamic' (not 'static')
            6.        On ALL other port groups , set as 'static' !!!
    */

    macEntry.key.key.macVlan.vlanId         = PRV_TGF_VLANID_1_CNS;
    macEntry.dstInterface.type              = CPSS_INTERFACE_PORT_E;

    PRV_UTF_LOG0_MAC("======= : Add (by message /index) FDB entry on port N1 to all 4 port groups =======\n");

    for(jj = 0 ; jj < PRV_CPSS_PP_MAC(prvTgfDevNum)->portGroupsInfo.numOfPortGroups; jj++)
    {
         /* Set the FDB entry mac address */
        cpssOsMemCpy(macEntry.key.key.macVlan.macAddr.arEther,
                     &prvTgfSaMacArr[jj],
                     sizeof(TGF_MAC_ADDR));

        portIndex = jj + N1_INDEX_E;

        index = PORT_INDEX_CONVERT_MAC(portIndex);

        /* set destination port */
        macEntry.dstInterface.devPort.hwDevNum    = prvTgfDevsArray[index];
        macEntry.dstInterface.devPort.portNum   = prvTgfPortsArray[index];

        for(ii = 0 ; ii < PRV_CPSS_PP_MAC(prvTgfDevNum)->portGroupsInfo.numOfPortGroups ; ii++)
        {
            if(useStatic == GT_TRUE)
            {
                macEntry.isStatic = (ii == jj) ? GT_FALSE : GT_TRUE;
            }
            else if(ii != jj)
            {
                continue;
            }

            currPortGroupsBmp = BIT_0 << ii;
            rc = prvTgfBrgFdbMacEntrySet(&macEntry);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

            /* check that entry exists */
            rc = prvTgfBrgFdbMacEntryFind(&macEntry.key,&fdbIndex,NULL,NULL,NULL);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
        }
    }


    if(forceLessThenHalfSleepTime == GT_TRUE)
    {
        PRV_UTF_LOG0_MAC("======= : trigger aging loop by HW =======\n");
        /*d. trigger aging loop by HW*/
        rc = prvTgfBrgFdbActionStart(PRV_TGF_FDB_ACTION_AGE_WITHOUT_REMOVAL_E);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

        PRV_UTF_LOG0_MAC("======= : wait for trigger action to end =======\n");
        /* wait for trigger action to end */
        rc = prvTgfBrgFdbActionDoneWait(GT_FALSE);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

        /* indication that not 'Auto aging' used */
        sleepTime = 0;
    }
    else
    {
        PRV_UTF_LOG0_MAC("======= : Set age without removal =======\n");
        /*f.        Set age without removal*/
        rc = prvTgfBrgFdbActionModeSet(PRV_TGF_FDB_ACTION_AGE_WITHOUT_REMOVAL_E);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

        PRV_UTF_LOG0_MAC("======= : Set aging to 'auto aging' =======\n");
        /*g.        Set aging to 'auto aging'*/
        rc = prvTgfBrgFdbMacTriggerModeSet(PRV_TGF_ACT_AUTO_E);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
    }


    /*h.        Wait 22 seconds ((2 * age time) + 10%)*/
#ifdef ASIC_SIMULATION /* simulation fixed the aging calculation */
    if(forceLessThenHalfSleepTime == GT_FALSE)
    {
        /* in simulation the single cycle hold up to 50% divert */
        sleepTime = (sleepTime * 15) / 10;
    }
#endif /*ASIC_SIMULATION*/


    if(sleepTime)
    {
        PRV_UTF_LOG1_MAC("======= : start sleep for [%d] seconds for aging testing =======\n",sleepTime);

        for(ii = 0 ; ii < sleepTime ; ii++)
        {
            cpssOsTimerWkAfter(1000);
            utfPrintKeepAlive();
        }

        PRV_UTF_LOG0_MAC("======= : sleep ended =======\n");
        PRV_UTF_LOG0_MAC("======= : Check that entry deleted from all port groups !!! =======\n");
    }
    else
    {
        /* single cycle ended (not 2 cycles) so all FDB entries should still exist */
        PRV_UTF_LOG0_MAC("======= : Check that entry NOT deleted from any port group !!! =======\n");
    }

    /*i.        Check that entry deleted from all port groups !!!
                (even the 3 port groups that hold 'static') -->
                because the AppDemo delete 'aged out' entry from all port groups !
    */

    for(jj = 0 ; jj < PRV_CPSS_PP_MAC(prvTgfDevNum)->portGroupsInfo.numOfPortGroups; jj++)
    {
         /* Set the FDB entry mac address */
        cpssOsMemCpy(macEntry.key.key.macVlan.macAddr.arEther,
                     &prvTgfSaMacArr[jj],
                     sizeof(TGF_MAC_ADDR));

        for(ii = 0 ; ii < PRV_CPSS_PP_MAC(prvTgfDevNum)->portGroupsInfo.numOfPortGroups ; ii++)
        {
            currPortGroupsBmp = BIT_0 << ii;
            rc = prvTgfBrgFdbMacEntryFind(&macEntry.key,&fdbIndex,NULL,NULL,NULL);
            if(forceLessThenHalfSleepTime == GT_TRUE)
            {
                if((useStatic == GT_TRUE) || (ii == jj))
                {
                    /* check that entry exists */
                    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
                }
                else
                {
                    /* check that entry NOT exists */
                    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_NOT_FOUND, rc);
                }
            }
            else
            {
                /* check that entry NOT exists */
                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_NOT_FOUND, rc);
            }
        }
    }

    PRV_UTF_LOG0_MAC("======= tgfBridgeFdbAgingAppDemoRun_ForMultiCoreFdbLookup: ended !!! =======\n");
    return;
}


/**
* @internal tgfBridgeFdbAgingAppDemoRestore_ForMultiCoreFdbLookup function
* @endinternal
*
* @brief   Restore_ForMultiCoreFdbLookup test configuration
*/
GT_VOID tgfBridgeFdbAgingAppDemoRestore_ForMultiCoreFdbLookup
(
    GT_VOID
)
{
    tgfBridgeFdbRestore_ForMultiCoreFdbLookup();
}

/**
* @internal fdbLearningFromPort function
* @endinternal
*
* @brief   function send incremental mac addresses from a port for learning.
*         and check that at least 99% of those mac addresses learned
* @param[in] learnByTraffic           - learning by traffic or by 'set entry'
*                                      GT_TRUE - by traffic
*                                      GT_FALSE - by 'set entry'
* @param[in] portIter                 - index of port in prvTgfDevPortArray2[...] , the port to
*                                      send learning from it learn
* @param[in] burstNum                 - number of frames in the burst
* @param[in] saMacPtr                 - pointer to the specific mac address
*                                      when NULL use the mac of the portIndex
*
* @param[out] actualLearnedMacs[/CPSS_MAX_PORT_GROUPS_CNS/] - array of actual learned entries on the port group
*                                       None
*/
static GT_VOID fdbLearningFromPort
(
    IN GT_BOOL  learnByTraffic,
    IN GT_U32   portIter,
    IN GT_U32   burstNum,
    OUT GT_U32  actualLearnedMacs[/*CPSS_MAX_PORT_GROUPS_CNS*/],
    IN TGF_MAC_ADDR    *saMacPtr,
    IN CPSS_INTERFACE_INFO_STC *dstInterfacePtr
)
{
    GT_STATUS   rc;
    GT_U8   devNum = prvTgfDevsArray[0];
    GT_U32  ii,iiMax;
    GT_U32  jj;
    GT_U32  kk;
    GT_U32  numOfValid = 0 ;/* number of valid entries in the FDB */
    GT_U32  numOfSkip = 0; /* number of skipped entries in the FDB */
    GT_U32  numOfValidMinusSkip=0;/* number of (valid - skip) entries in the FDB*/
    CPSS_PORT_MAC_COUNTER_SET_STC portCntrs;/* counters of port */
    GT_U32  expectedCounterValue;  /* expected counter value */
    PRV_TGF_BRG_MAC_ENTRY_STC      macEntry = PRV_TGF_BRG_FDB_MAC_ENTRY_DEFAULT_MAC;
    TGF_VFD_INFO_STC    vfd;
    GT_U32  index;/*index to access prvTgfDevsArray[] and prvTgfPortsArray[] */

    /* clear VFD array */
    cpssOsMemSet((GT_VOID*) &vfd, 0, sizeof(vfd));

    index = PORT_INDEX_CONVERT_MAC(portIter);

    if(learnByTraffic == GT_FALSE)
    {
        if(saMacPtr)
        {
            cpssOsMemCpy(macEntry.key.key.macVlan.macAddr.arEther,
                         saMacPtr,
                         sizeof(TGF_MAC_ADDR));
        }
        else
        {
            /* Set the FDB entry mac address */
            macEntry.key.key.macVlan.macAddr.arEther[0] = 0;
            macEntry.key.key.macVlan.macAddr.arEther[1] = 0;
            macEntry.key.key.macVlan.macAddr.arEther[2] =
            macEntry.key.key.macVlan.macAddr.arEther[3] =
            macEntry.key.key.macVlan.macAddr.arEther[4] =
            macEntry.key.key.macVlan.macAddr.arEther[5] = (GT_U8)prvTgfPortsArray[index];
        }

        macEntry.key.key.macVlan.vlanId         = PRV_TGF_VLANID_1_CNS;
        macEntry.dstInterface              = *dstInterfacePtr;

        /* the the info for the increment of the mac */
        vfd.cycleCount = 6;
        vfd.mode = TGF_VFD_MODE_INCREMENT_E;
        vfd.modeExtraInfo = 0;
        vfd.offset = 0;
        cpssOsMemCpy(vfd.patternPtr, macEntry.key.key.macVlan.macAddr.arEther, sizeof(TGF_MAC_ADDR));

    }

    PRV_UTF_LOG3_MAC("======= fdbLearningFromPort : portIter[%d],burstNum[%d] ,useMac[%d] =======\n",
        portIter,burstNum,saMacPtr?1:0);

    if(portIter < N1_INDEX_E)
    {
        /* error */
        UTF_VERIFY_EQUAL2_STRING_MAC(N1_INDEX_E, portIter, " portIter=[%d] less then [%d]",
                                     portIter,N1_INDEX_E);
        return;
    }

    PRV_UTF_LOG3_MAC("=======  : clear the sender counters =======\n",
        portIter,burstNum,saMacPtr?1:0);
    /* clear the sender counters */
    rc = prvTgfResetCountersEth(prvTgfDevsArray[index],
                                prvTgfPortsArray[index]);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    if(learnByTraffic == GT_TRUE)
    {
        PRV_UTF_LOG0_MAC("=======  : send up to 4 times to make sure learning was done =======\n");
        /* send up to 4 times to make sure learning was done */
        iiMax = 4;
    }
    else
    {
        iiMax = 1;
    }

    for(ii = 0; ii < iiMax ; ii++)
    {
        if(learnByTraffic == GT_TRUE)
        {
            PRV_UTF_LOG1_MAC("=======  : iteration[%d] send =======\n",ii);

            if(saMacPtr == NULL)
            {
                prvTgfCommonIncrementalSaMacByPortNumSend(
                        prvTgfDevsArray[index],
                        prvTgfPortsArray[index],
                        burstNum);
            }
            else
            {
                prvTgfCommonIncrementalSaMacSend(
                        prvTgfDevsArray[index],
                        prvTgfPortsArray[index],
                        saMacPtr,
                        burstNum);
            }

            PRV_UTF_LOG1_MAC("=======  : iteration[%d] check counters =======\n",ii);
            /* check counters */
            rc = prvTgfReadPortCountersEth(prvTgfDevsArray[index], prvTgfPortsArray[index], GT_TRUE, &portCntrs);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                         prvTgfDevsArray[index],
                                         prvTgfPortsArray[index]);

            /* this port should get packets */
            expectedCounterValue = burstNum;

            UTF_VERIFY_EQUAL4_STRING_MAC(expectedCounterValue, portCntrs.goodPktsSent.l[0],
                                         "dev[%d],port[%d] ,expected [%d] in goodPktsSent but got [%d]"
                                         ,prvTgfDevsArray[index]
                                         ,prvTgfPortsArray[index]
                                         ,expectedCounterValue
                                         ,portCntrs.goodPktsSent.l[0]
                                         );

            /* port in loop back should be the same value */
            UTF_VERIFY_EQUAL4_STRING_MAC(portCntrs.goodPktsRcv.l[0], portCntrs.goodPktsSent.l[0],
                                         "dev[%d],port[%d] ,goodPktsRcv [%d] but goodPktsSent [%d]"
                                         ,prvTgfDevsArray[index]
                                         ,prvTgfPortsArray[index]
                                         ,portCntrs.goodPktsRcv.l[0]
                                         ,portCntrs.goodPktsSent.l[0]
                                         );
        }
        else
        {
            PRV_UTF_LOG1_MAC("=======  : iteration[%d] learn by message =======\n",ii);

            macEntry.dstInterface.devPort.hwDevNum    = prvTgfDevsArray[index];
            macEntry.dstInterface.devPort.portNum   = prvTgfPortsArray[index];

            /* bmp to set the entry to is according to AppDemo's logic */
            currPortGroupsBmp =
                appDemoPpConfigList[devNum].
                    portGroupsInfo[portIter & 3].unifiedFdbPortGroupsBmp;
            for(jj = 0 ; jj < burstNum; jj++)
            {
                /* increment the mac address */
                vfd.modeExtraInfo = jj;

                if((jj % 1000) == 999)
                {
                    /* show sign of aliveness */
                    utfPrintKeepAlive();
                }

                /* over ride the buffer with VFD info */
                rc = tgfTrafficEnginePacketVfdApply(&vfd,
                        macEntry.key.key.macVlan.macAddr.arEther,
                        sizeof(TGF_MAC_ADDR));
                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

                rc = prvTgfBrgFdbMacEntrySet(&macEntry);
                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
            }
        }

        PRV_UTF_LOG1_MAC("=======  : iteration[%d] check FDB capacity =======\n",ii);
        /* c.        Check FDB capacity (FDB should be full on all port groups) */
        /* DO it BEFORE we send the traffic from the Uplinks because we not want to
           see the 4 mac addresses , of the Uplinks */
        /* get FDB counters */
        rc = prvTgfBrgFdbPortGroupCount(devNum,(portIter - N1_INDEX_E),
            &numOfValid,&numOfSkip,NULL,NULL,NULL);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

        numOfValidMinusSkip = numOfValid - numOfSkip;

        /* expected is the current burst and entries already learned on this port group */
        expectedCounterValue = burstNum ;
        for(kk = 0 ;
            (GT_U32)(BIT_0 << kk) <= appDemoPpConfigList[devNum].portGroupsInfo[portIter - N1_INDEX_E].unifiedFdbPortGroupsBmp;
            kk ++)
        {
            if(kk >= CPSS_MAX_PORT_GROUPS_CNS)
            {
                /* should not happen */
                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, GT_BAD_STATE);
                return;
            }

            if((BIT_0 << kk) & appDemoPpConfigList[devNum].portGroupsInfo[portIter - N1_INDEX_E].unifiedFdbPortGroupsBmp)
            {
                /* this port group hold mac addresses of this group */
                expectedCounterValue += actualLearnedMacs[kk];
            }
        }

        if(expectedCounterValue < numOfValidMinusSkip)
        {
            /* we have more entries then expected */
            UTF_VERIFY_EQUAL2_STRING_MAC(expectedCounterValue , numOfValidMinusSkip,
                                "FDB learning: expected [%d] is less then received [%d] ",
                                         expectedCounterValue , numOfValidMinusSkip);
        }
        /* we not expect all burst be learned due to hash collisions ,
        but 99% of it is reasonable for incremental addresses */
        else if(0 == ((expectedCounterValue - numOfValidMinusSkip) > (expectedCounterValue / 100)))
        {
            /* no need new sending of burst */
            break;
        }
        /* we expect at least 33% to be learned */
        else if(numOfValidMinusSkip < (expectedCounterValue / 3))
        {
            /* no need new sending of burst */
            UTF_VERIFY_EQUAL2_STRING_MAC(expectedCounterValue / 3, numOfValidMinusSkip,
                                "FDB learning: less then 33 percent learned : expected [%d] , got [%d] ",
                                         expectedCounterValue / 3, numOfValidMinusSkip);
        }
    }/*loop on ii*/

    /* expected is the current burst and entries already learned on this port group */
    expectedCounterValue = burstNum ;
    for(kk = 0 ;
        (GT_U32)(BIT_0 << kk) <= appDemoPpConfigList[devNum].portGroupsInfo[portIter - N1_INDEX_E].unifiedFdbPortGroupsBmp;
        kk ++)
    {
        if(kk >= CPSS_MAX_PORT_GROUPS_CNS)
        {
            /* should not happen */
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, GT_BAD_STATE);
            return;
        }

        if((BIT_0 << kk) & appDemoPpConfigList[devNum].portGroupsInfo[portIter - N1_INDEX_E].unifiedFdbPortGroupsBmp)
        {
            /* this port group hold mac addresses of this group */
            expectedCounterValue += actualLearnedMacs[kk];
        }
    }

    /* we not expect all burst be learned due to hash collisions ,
    but 99% of it is reasonable for incremental addresses */
    if((expectedCounterValue - numOfValidMinusSkip) > (expectedCounterValue / 100))
    {
        /* diff more then 1% */
        UTF_VERIFY_EQUAL2_STRING_MAC(expectedCounterValue, numOfValidMinusSkip,
                            "FDB learning: more then 1 percent difference : expected [%d] , got [%d] ",
                                     expectedCounterValue, numOfValidMinusSkip);
    }

    /* save the actual number of entries learned */
    actualLearnedMacs[portIter - N1_INDEX_E] = numOfValidMinusSkip;
    for(kk = 0 ;
        (GT_U32)(BIT_0 << kk) <= appDemoPpConfigList[devNum].portGroupsInfo[portIter - N1_INDEX_E].unifiedFdbPortGroupsBmp;
        kk ++)
    {
        if(kk == (portIter - N1_INDEX_E))
        {
            continue;
        }

        if((BIT_0 << kk) & appDemoPpConfigList[devNum].portGroupsInfo[portIter - N1_INDEX_E].unifiedFdbPortGroupsBmp)
        {
            /* this port group hold mac addresses of this group */
            actualLearnedMacs[portIter - N1_INDEX_E] -= actualLearnedMacs[kk];
        }
    }

    PRV_UTF_LOG0_MAC("=======  : clear the sender counters =======\n");
    /* clear the sender counters */
    rc = prvTgfResetCountersEth(prvTgfDevsArray[index],
                                prvTgfPortsArray[index]);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    PRV_UTF_LOG0_MAC("=======  fdbLearningFromPort : ended =======\n");

    return;
}
/**
* @internal tgfBridgeFdbLearningFromNetworkPortsRun_ForMultiCoreFdbLookup function
* @endinternal
*
* @brief   4.3    Test 5: check learning of 128K/64K from network ports
*         (Covered API: FDB :
*         1. cpssDxChBrgFdbPortGroupMacEntrySet
*         c. cpssDxChBrgFdbPortGroupFromCpuAuMsgStatusGet
*         2. cpssDxChBrgFdbPortGroupMacEntryRead
*         )
*         1. check learning of 128K/64K from network ports
*         a. Do learning from network ports (N1,N2,N3,N4).
*         a. In 128K mode --> 32K from each port
*         b. In 64K mode --> 16K from each port
*         b. Check that ALL uplinks see 128K/64K addresses. (send traffic to those learned macs)
*         c. Check FDB capacity (FDB should be full on all port groups)
* @param[in] learnByTraffic           - learning by traffic or by 'set entry'
*                                      GT_TRUE - by traffic
*                                      GT_FALSE - by 'set entry'
*                                       None
*/
GT_VOID tgfBridgeFdbLearningFromNetworkPortsRun_ForMultiCoreFdbLookup
(
    IN GT_BOOL  learnByTraffic
)
{
    GT_STATUS   rc;
    GT_U32  portIter,ii;
    GT_U32  burstNum;
    GT_U32  numOfValid ;/* number of valid entries in the FDB */
    GT_U32  numOfSkip ; /* number of skipped entries in the FDB */
    GT_U32  numOfValidMinusSkip=0;/* number of (valid - skip) entries in the FDB*/
    CPSS_PORT_MAC_COUNTER_SET_STC portCntrs;/* counters of port */
    GT_U32  expectedCounterValue;  /* expected counter value */
    GT_U32  actualLearnedMacs[CPSS_MAX_PORT_GROUPS_CNS] = {0};
    GT_BOOL origByMessageMustSucceed = GT_TRUE;/* original value of 'By Message' Must Succeed */
    CPSS_INTERFACE_INFO_STC interfaceInfo = PRV_TGF_INTERFACE_PORT_MAC(0,0);
    GT_U32  index;/*index to access prvTgfDevsArray[] and prvTgfPortsArray[] */
    GT_U32  fdbNumEntriesPerPortGroup;/* number of FDB entries in single port group*/
    GT_U32  burstModeFactor;/* burst mode factor */

    utfGeneralStateMessageSave(INDEX_2_CNS,"learnByTraffic[%d]",
        learnByTraffic);

    /* Check FDB capacity (FDB should be empty) */
    /* get FDB counters */
    rc = prvTgfBrgFdbCount(&numOfValid,&numOfSkip,NULL,NULL,NULL);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    numOfValidMinusSkip = numOfValid - numOfSkip;
    UTF_VERIFY_EQUAL1_STRING_MAC(0, numOfValidMinusSkip,
                    "FDB not empty , got [%d] entries ",
                    numOfValidMinusSkip);


    switch(PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily)
    {
        case CPSS_PP_FAMILY_DXCH_LION_E:
            fdbNumEntriesPerPortGroup = _32K;
            break;
        case CPSS_PP_FAMILY_DXCH_LION2_E:
            fdbNumEntriesPerPortGroup = _64K;
            break;
        default:
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, GT_NOT_SUPPORTED);
            return;
    }
/*
    LionB:
    a.        Do learning from network ports (N1..N4).
        a.        In 128K mode --> 32K from each port
        b.        In 64K mode  --> 16K from each port
    Lion2:
    a.        Do learning from network ports (N1..N8).
        a.        In 256K mode --> 32K from each port
        b.        In 128K mode --> 16K from each port
*/

    switch(fdbMode)
    {
        case PRV_TGF_FDB_QUAD_MODE_E:
            burstModeFactor = 4;
            break;
        case PRV_TGF_FDB_DOUBLE_MODE_E:
            burstModeFactor = 2;
            break;
        default:
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, GT_NOT_SUPPORTED);
            return;
    }

    burstNum = (fdbNumEntriesPerPortGroup / PRV_CPSS_PP_MAC(prvTgfDevNum)->portGroupsInfo.numOfPortGroups) * burstModeFactor;

    if(debug_burstNum)
    {
        burstNum = debug_burstNum;
    }

    PRV_UTF_LOG0_MAC("======= tgfBridgeFdbLearningFromNetworkPortsRun_ForMultiCoreFdbLookup: send traffic from network ports with incremental SA macs that starts from"
       "the network port --> for learning =======\n");

    /* send traffic from network ports with incremental SA macs that starts from
       the network port --> for learning */
    for (portIter = N1_INDEX_E; portIter <= LAST_NETWORK_PORT_INDEX_CNS; portIter++)
    {
        if(learnByTraffic == GT_FALSE)
        {
            /* state the 'by message operations' may not succeed ---> due to bucket full */
            origByMessageMustSucceed = prvTgfBrgFdbEntryByMessageMustSucceedSet(GT_FALSE);
        }

        index = PORT_INDEX_CONVERT_MAC(portIter);

        interfaceInfo.devPort.hwDevNum = prvTgfDevsArray[index];
        interfaceInfo.devPort.portNum = prvTgfPortsArray[index];

        fdbLearningFromPort(learnByTraffic,portIter,burstNum,actualLearnedMacs,NULL,&interfaceInfo);

        if(learnByTraffic == GT_FALSE)
        {
            /* restore value */
            prvTgfBrgFdbEntryByMessageMustSucceedSet(origByMessageMustSucceed);
        }
    }

    PRV_UTF_LOG0_MAC("======= : Check FDB capacity (FDB should be full on all port groups)=======\n");

    /* c.        Check FDB capacity (FDB should be full on all port groups) */
    /* DO it BEFORE we send the traffic from the Uplinks because we not want to
       see the 4 mac addresses , of the Uplinks */
    /* get FDB counters */
    rc = prvTgfBrgFdbCount(&numOfValid,&numOfSkip,NULL,NULL,NULL);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    numOfValidMinusSkip = numOfValid - numOfSkip;
    numOfValidMinusSkip /= PRV_CPSS_PP_MAC(prvTgfDevNum)->portGroupsInfo.numOfPortGroups;
    numOfValidMinusSkip *= burstModeFactor;

    /* check the FDB counters with the expected values */
    expectedCounterValue = fdbNumEntriesPerPortGroup * burstModeFactor;

    if(debug_burstNum)
    {
        expectedCounterValue = debug_burstNum *  PRV_CPSS_PP_MAC(prvTgfDevNum)->portGroupsInfo.numOfPortGroups;
    }


    /* we not expect all burst be learned due to hash collisions ,
    but 99% of it is reasonable for incremental addresses */
    if((expectedCounterValue - numOfValidMinusSkip) > (expectedCounterValue / 100))
    {
        UTF_VERIFY_EQUAL2_STRING_MAC(expectedCounterValue, numOfValidMinusSkip,
                            "FDB learning: more then 1 percent difference : expected [%d] , got [%d] ",
                                     expectedCounterValue, numOfValidMinusSkip);
    }

    PRV_UTF_LOG0_MAC("======= : Check that ALL uplinks see 128K/64K addresses. (send traffic to those learned macs)=======\n");
    /* b. Check that ALL uplinks see 128K/64K addresses. (send traffic to those learned macs)*/

    /* send traffic from uplinks with incremental DA macs that starts from the
       relevant network port */
    for (portIter = U1_INDEX_E; portIter < PRV_CPSS_PP_MAC(prvTgfDevNum)->portGroupsInfo.numOfPortGroups; portIter++)
    {

        index = PORT_INDEX_CONVERT_MAC(portIter);

        PRV_UTF_LOG1_MAC("=======  : portIter[%d] send DA burst =======\n",portIter);
        prvTgfCommonIncrementalDaMacByPortNumSend(
                prvTgfDevsArray[index],
                prvTgfPortsArray[index],
                prvTgfPortsArray[PORT_INDEX_CONVERT_MAC(portIter + N1_INDEX_E)],
                burstNum);

        for (ii = N1_INDEX_E; ii <= LAST_NETWORK_PORT_INDEX_CNS; ii++)
        {
            index = PORT_INDEX_CONVERT_MAC(ii);

            PRV_UTF_LOG2_MAC("=======  : portIter[%d] ii[%d] check port counters =======\n",portIter,ii);
            /* check counters */
            rc = prvTgfReadPortCountersEth(prvTgfDevsArray[index], prvTgfPortsArray[index], GT_TRUE, &portCntrs);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                         prvTgfDevsArray[index],
                                         prvTgfPortsArray[index]);

            if((portIter + N1_INDEX_E) == ii)
            {
                /* this port should get all the burst (known UC and flooding) */
                expectedCounterValue = burstNum;
            }
            else
            {
                /* this port should get only flooded packets */
                /* the amount of flooded is according to the number of actual
                   learned entries in FDB */
                expectedCounterValue = burstNum - actualLearnedMacs[portIter];
            }

            UTF_VERIFY_EQUAL6_STRING_MAC(expectedCounterValue, portCntrs.goodPktsSent.l[0],
                                         "receiver is: dev[%d],port[%d] ,expected [%d] in goodPktsSent but got [%d]"
                                         "(sender is : dev[%d],port[%d])"
                                         /* receiver */
                                         ,prvTgfDevsArray[index]
                                         ,prvTgfPortsArray[index]
                                         /* counters of receiver */
                                         ,expectedCounterValue
                                         ,portCntrs.goodPktsSent.l[0]
                                         /* sender */
                                         ,prvTgfDevsArray[PORT_INDEX_CONVERT_MAC(portIter)]
                                         ,prvTgfPortsArray[PORT_INDEX_CONVERT_MAC(portIter)]
                                         );
        }/* loop on ii --> network ports */
    }/*loop on portIter --> uplinks */

#ifdef ASIC_SIMULATION
    /* allow the simulation to be done with the major burst sent , and that
       the traffic should finish processing */
    cpssOsTimerWkAfter(1000);
#endif /*ASIC_SIMULATION*/

    PRV_UTF_LOG0_MAC("======= tgfBridgeFdbLearningFromNetworkPortsRun_ForMultiCoreFdbLookup : ended =======\n");
    return;
}

/**
* @internal tgfBridgeFdbLearningFromNetworkPortsRestore_ForMultiCoreFdbLookup function
* @endinternal
*
* @brief   Restore_ForMultiCoreFdbLookup test configuration
*/
GT_VOID tgfBridgeFdbLearningFromNetworkPortsRestore_ForMultiCoreFdbLookup
(
    GT_VOID
)
{
    tgfBridgeFdbRestore_ForMultiCoreFdbLookup();
}

/**
* @internal tgfBridgeFdbLearningFromNetworkTrunksRun_ForMultiCoreFdbLookup function
* @endinternal
*
* @brief   4.4    Test 6: check learning of 128K/64K from trunk ports
*         (Covered API: FDB :
*         1. cpssDxChBrgFdbPortGroupMacEntrySet
*         d. cpssDxChBrgFdbPortGroupFromCpuAuMsgStatusGet
*         2. cpssDxChBrgFdbPortGroupMacEntryRead
*         )
*         1. check learning of 128K/64K from trunk ports
*         a. Add N1,N3 to trunk Tr1 and N2,N4 to trunk Tr2.
*         b. Do learning from those ports (N1,N2,N3,N4).
*         a. In 128K mode --> 32K from each port
*         b. In 64K mode --> 16K from each port
*         c. Check that ALL uplinks see 128K/64K addresses. (send traffic to those learned macs)
*         d. Check FDB capacity (FDB should be full on all port groups)
* @param[in] learnByTraffic           - learning by traffic or by 'set entry'
*                                      GT_TRUE - by traffic
*                                      GT_FALSE - by 'set entry'
*                                      learnMixedOnTrunkPorts - do we learn addresses from single port in trunk
*                                       None
*/
GT_VOID tgfBridgeFdbLearningFromNetworkTrunksRun_ForMultiCoreFdbLookup
(
    IN GT_BOOL  learnByTraffic,
    IN GT_BOOL  learnFromSinglePortInTrunk
)
{
    GT_STATUS   rc;
    GT_U32  portIter,senderIndex,ii;
    GT_U32  burstNum;
    GT_U32  numOfValid;/* number of valid entries in the FDB */
    GT_U32  numOfSkip; /* number of skipped entries in the FDB */
    GT_U32  numOfValidMinusSkip=0;/* number of (valid - skip) entries in the FDB*/
    GT_TRUNK_ID trunkIdArr[2] = {PRV_TGF_TRUNK_ID_CNS,PRV_TGF_ANOTHER_TRUNK_ID_CNS};
    CPSS_TRUNK_MEMBER_STC    trunkMember;/* trunk member */
    PRV_TGF_TRUNK_EXPECTED_LOAD_BALANCE_MODE_ENT mode;
    TGF_MAC_ADDR    saMac;
    GT_U32 portNum;
    GT_U32  expectedCounterValue;  /* expected counter value */
    GT_U32  tolerance;/* tolerance expected due to flooding */
    GT_U32  actualLearnedMacs[CPSS_MAX_PORT_GROUPS_CNS] = {0};
    GT_U32  actualLearnedMacs_forSrcPort[CPSS_MAX_PORT_GROUPS_CNS] = {0};
    CPSS_PORT_MAC_COUNTER_SET_STC portCntrs;/* counters of port */
    GT_U32  tmpValue;
    GT_BOOL origByMessageMustSucceed = GT_TRUE;/* original value of 'By Message' Must Succeed */
    CPSS_INTERFACE_INFO_STC interfaceInfo = PRV_TGF_INTERFACE_TRUNK_MAC(0);
    GT_U32  index;/*index to access prvTgfDevsArray[] and prvTgfPortsArray[] */
    GT_U32  fdbNumEntriesPerPortGroup;/* number of FDB entries in single port group*/
    GT_U32  burstModeFactor;/* burst mode factor */


    if(learnFromSinglePortInTrunk == GT_TRUE && learnByTraffic == GT_FALSE)
    {
        /* when using learnByTraffic == GT_FALSE , there is no meaning to running with :
        learnFromSinglePortInTrunk == GT_TRUE , because has no meaning. */

        return;
    }

    utfGeneralStateMessageSave(INDEX_2_CNS,"learnByTraffic[%d],learnFromSinglePortInTrunk[%d]",
        learnByTraffic,learnFromSinglePortInTrunk);

    /* Check FDB capacity (FDB should be empty) */
    /* get FDB counters */
    rc = prvTgfBrgFdbCount(&numOfValid,&numOfSkip,NULL,NULL,NULL);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    numOfValidMinusSkip = numOfValid - numOfSkip;
    UTF_VERIFY_EQUAL1_STRING_MAC(0, numOfValidMinusSkip,
                    "FDB not empty , got [%d] entries ",
                    numOfValidMinusSkip);

    PRV_UTF_LOG0_MAC("======= tgfBridgeFdbLearningFromNetworkTrunksRun_ForMultiCoreFdbLookup : clear the 2 trunks =======\n");

    /* clear the 2 trunks */
    for(ii = 0 ; ii < 2 ; ii++)
    {
        CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkIdArr[ii]);

        rc = prvTgfTrunkMembersSet(
            trunkIdArr[ii] /*trunkId*/,
            0 /*numOfEnabledMembers*/,
            NULL,
            0 /*numOfDisabledMembers*/,
            NULL  /*disabledMembersArray*/);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
    }

    PRV_UTF_LOG0_MAC("======= : Add N1,N3 to trunk Tr1 and N2,N4 to trunk Tr2 =======\n");
    /*a. Add N1,N3 to trunk Tr1 and N2,N4 to trunk Tr2.*/
    for (portIter = N1_INDEX_E; portIter <= LAST_NETWORK_PORT_INDEX_CNS; portIter++)
    {
        index = PORT_INDEX_CONVERT_MAC(portIter);

        trunkMember.hwDevice = prvTgfDevsArray[index];
        trunkMember.port = prvTgfPortsArray[index];

        rc = prvTgfTrunkMemberAdd(trunkIdArr[portIter & 1], &trunkMember);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
    }

    PRV_UTF_LOG0_MAC("======= : Do learning from network ports (N1,N2,N3,N4) =======\n");
    switch(PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily)
    {
        case CPSS_PP_FAMILY_DXCH_LION_E:
            fdbNumEntriesPerPortGroup = _32K;
            break;
        case CPSS_PP_FAMILY_DXCH_LION2_E:
            fdbNumEntriesPerPortGroup = _64K;
            break;
        default:
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, GT_NOT_SUPPORTED);
            return;
    }
/*
    LionB:
    a.        Do learning from network ports (N1..N4).
        a.        In 128K mode --> 32K from each port
        b.        In 64K mode  --> 16K from each port
    Lion2:
    a.        Do learning from network ports (N1..N8).
        a.        In 256K mode --> 32K from each port
        b.        In 128K mode --> 16K from each port
*/

    switch(fdbMode)
    {
        case PRV_TGF_FDB_QUAD_MODE_E:
            burstModeFactor = 4;
            break;
        case PRV_TGF_FDB_DOUBLE_MODE_E:
            burstModeFactor = 2;
            break;
        default:
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, GT_NOT_SUPPORTED);
            return;
    }

    burstNum = (fdbNumEntriesPerPortGroup / PRV_CPSS_PP_MAC(prvTgfDevNum)->portGroupsInfo.numOfPortGroups) * burstModeFactor;

    if(debug_burstNum)
    {
        burstNum = debug_burstNum;
    }

    if(learnFromSinglePortInTrunk == GT_TRUE)
    {
        if(burstNum > _16K)
        {
            /* in this mode we send traffic from only 2 ports N1,N2 but for
               traffic 'associated' with all 4 ports */
            burstNum = _16K;
        }
    }

    PRV_UTF_LOG0_MAC("======= : send traffic from network ports with incremental SA macs that starts from"
       "the network port --> for learning =======\n");
    /* send traffic from network ports with incremental SA macs that starts from
       the network port --> for learning */
    for (portIter = N1_INDEX_E; portIter <= LAST_NETWORK_PORT_INDEX_CNS; portIter++)
    {
        index = PORT_INDEX_CONVERT_MAC(portIter);

        interfaceInfo.trunkId = trunkIdArr[portIter & 1];

        if(learnFromSinglePortInTrunk == GT_FALSE)
        {
            if(learnByTraffic == GT_FALSE)
            {
                /* state the 'by message operations' may not succeed ---> due to bucket full */
                origByMessageMustSucceed = prvTgfBrgFdbEntryByMessageMustSucceedSet(GT_FALSE);
            }

            fdbLearningFromPort(learnByTraffic,portIter,burstNum,actualLearnedMacs_forSrcPort,NULL,&interfaceInfo);

            if(learnByTraffic == GT_FALSE)
            {
                /* restore value */
                prvTgfBrgFdbEntryByMessageMustSucceedSet(origByMessageMustSucceed);
            }
        }
        else
        {
            /* build the source mac address */

            /* the SA is according to the 'real' network port , although we sent
               it from other port in the trunk ... because should be learned on
               the 'trunk' and not on the port  */

            /* the sender is N1 or N2 */
            senderIndex = N1_INDEX_E + (portIter & 1);
            /* the mac is according to N1/N2/N3/N4 */
            portNum = prvTgfPortsArray[index];
            saMac[0] = 0;
            saMac[1] = 0;
            saMac[2] =
            saMac[3] =
            saMac[4] =
            saMac[5] = (GT_U8)portNum;

            /* save value before the update of fdbLearningFromPort()*/
            tmpValue = actualLearnedMacs[senderIndex - N1_INDEX_E];
            /* send from single (first) port in trunk */

            if(learnByTraffic == GT_FALSE)
            {
                /* state the 'by message operations' may not succeed ---> due to bucket full */
                origByMessageMustSucceed = prvTgfBrgFdbEntryByMessageMustSucceedSet(GT_FALSE);
            }

            fdbLearningFromPort(learnByTraffic,senderIndex,burstNum,actualLearnedMacs,&saMac,&interfaceInfo);

            if(learnByTraffic == GT_FALSE)
            {
                /* restore value */
                prvTgfBrgFdbEntryByMessageMustSucceedSet(origByMessageMustSucceed);
            }

            /* set the actual number of addresses learned in this burst */
            actualLearnedMacs_forSrcPort[portIter - N1_INDEX_E] =
                actualLearnedMacs[senderIndex - N1_INDEX_E] - tmpValue;
        }
    }

    PRV_UTF_LOG0_MAC("======= : Check FDB capacity (FDB should be full on all port groups) =======\n");
    /* c.        Check FDB capacity (FDB should be full on all port groups) */
    /* DO it BEFORE we send the traffic from the Uplinks because we not want to
       see the 4 mac addresses , of the Uplinks */
    /* get FDB counters */
    rc = prvTgfBrgFdbCount(&numOfValid,&numOfSkip,NULL,NULL,NULL);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    numOfValidMinusSkip = numOfValid - numOfSkip;
    numOfValidMinusSkip /= PRV_CPSS_PP_MAC(prvTgfDevNum)->portGroupsInfo.numOfPortGroups;
    numOfValidMinusSkip *= burstModeFactor;

    /* check the FDB counters with the expected values */
    expectedCounterValue = fdbNumEntriesPerPortGroup * burstModeFactor;

    if(debug_burstNum)
    {
        expectedCounterValue = debug_burstNum *  PRV_CPSS_PP_MAC(prvTgfDevNum)->portGroupsInfo.numOfPortGroups;
    }

    /* we not expect all burst be learned due to hash collisions ,
    but 99% of it is reasonable for incremental addresses */
    if((expectedCounterValue - numOfValidMinusSkip) > (expectedCounterValue / 100))
    {
        UTF_VERIFY_EQUAL2_STRING_MAC(expectedCounterValue, numOfValidMinusSkip,
                            "FDB learning: more then 1 percent difference : expected [%d] , got [%d] ",
                                     expectedCounterValue, numOfValidMinusSkip);
    }

    PRV_UTF_LOG0_MAC("======= : Check that ALL uplinks see 128K/64K addresses. (send traffic to those learned macs) =======\n");
    /* b. Check that ALL uplinks see 128K/64K addresses. (send traffic to those learned macs)*/

    /* send traffic from uplinks with incremental DA macs that starts from the
       relevant network port */
    for (portIter = U1_INDEX_E; portIter < PRV_CPSS_PP_MAC(prvTgfDevNum)->portGroupsInfo.numOfPortGroups; portIter++)
    {
        index = PORT_INDEX_CONVERT_MAC(portIter);

        rc = prvTgfResetCountersEth(prvTgfDevsArray[index], prvTgfPortsArray[index]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevsArray[index],
                                     prvTgfPortsArray[index]);


        PRV_UTF_LOG1_MAC("=======  : portIter[%d] send DA burst =======\n",portIter);
        prvTgfCommonIncrementalDaMacByPortNumSend(
                prvTgfDevsArray[index],
                prvTgfPortsArray[index],
                prvTgfPortsArray[PORT_INDEX_CONVERT_MAC(portIter + N1_INDEX_E)],
                burstNum);

        PRV_UTF_LOG1_MAC("=======  : portIter[%d] check counters =======\n",ii);
        /* check counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevsArray[index], prvTgfPortsArray[index], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevsArray[index],
                                     prvTgfPortsArray[index]);

        /* this port should get packets */
        expectedCounterValue = burstNum;

        UTF_VERIFY_EQUAL4_STRING_MAC(expectedCounterValue, portCntrs.goodPktsSent.l[0],
                                     "dev[%d],port[%d] ,expected [%d] in goodPktsSent but got [%d]"
                                     ,prvTgfDevsArray[index]
                                     ,prvTgfPortsArray[index]
                                     ,expectedCounterValue
                                     ,portCntrs.goodPktsSent.l[0]
                                     );

        /* port in loop back should be the same value */
        UTF_VERIFY_EQUAL4_STRING_MAC(portCntrs.goodPktsRcv.l[0], portCntrs.goodPktsSent.l[0],
                                     "dev[%d],port[%d] ,goodPktsRcv [%d] but goodPktsSent [%d]"
                                     ,prvTgfDevsArray[index]
                                     ,prvTgfPortsArray[index]
                                     ,portCntrs.goodPktsRcv.l[0]
                                     ,portCntrs.goodPktsSent.l[0]
                                     );

        mode = PRV_TGF_TRUNK_EXPECTED_LOAD_BALANCE_MODE_EVEN_E;

        /* this trunk should get all the burst (known UC and flooding) */
        expectedCounterValue = burstNum;
        /* the flooding may come on other port ! because it travels between port groups !!!! */
        tolerance = burstNum - actualLearnedMacs_forSrcPort[portIter];
        prvTgfTrunkLoadBalanceCheck(trunkIdArr[portIter & 1],mode,
            expectedCounterValue,
            tolerance,
            NULL,NULL);/* traffic not sent from within the checked trunk */

        /* this trunk should get only flooded packets */
        /* the amount of flooded is according to the number of actual
           learned entries in FDB */
        expectedCounterValue = burstNum - actualLearnedMacs_forSrcPort[portIter];
        prvTgfTrunkLoadBalanceCheck(trunkIdArr[1 - (portIter & 1)],
            mode,
            expectedCounterValue,
            0,
            NULL,NULL);/* traffic not sent from within the checked trunk */

    }/*loop on portIter --> uplinks */

    PRV_UTF_LOG0_MAC("======= tgfBridgeFdbLearningFromNetworkTrunksRun_ForMultiCoreFdbLookup : ended =======\n");
    return;
}

/**
* @internal tgfBridgeFdbLearningFromNetworkTrunksRestore_ForMultiCoreFdbLookup function
* @endinternal
*
* @brief   Restore_ForMultiCoreFdbLookup test configuration
*/
GT_VOID tgfBridgeFdbLearningFromNetworkTrunksRestore_ForMultiCoreFdbLookup
(
    GT_VOID
)
{
    tgfBridgeFdbRestore_ForMultiCoreFdbLookup();
}

/**
* @internal tgfBridgeFdbStationMovementRun_ForMultiCoreFdbLookup function
* @endinternal
*
* @brief   4.5    Test 7: check station movement
*         (Covered API: FDB :
*         1. cpssDxChBrgFdbPortGroupMacEntrySet
*         e. cpssDxChBrgFdbPortGroupFromCpuAuMsgStatusGet
*         2. cpssDxChBrgFdbPortGroupMacEntryRead
*         )
*         1. check station movement (port to port)
*         a. learn mac A from port N1 in port group 0
*         b. In 128K mode:
*         a. check that FDB of port group 0 has it , other FDBs don't
*         c. In 64 K mode:
*         a. check that FDB of port group 0,2 has it , other FDBs don't
*         d. learn same mac A from port N2 in port group 1
*         e. In 128K mode:
*         a. check that FDB of port group 1 has it , other FDBs don't
*         f. In 64 K mode:
*         a. check that FDB of port group 1,3 has it , other FDBs don't
*         g. learn same mac A from port N3 in port group 2
*         h. In 128K mode:
*         a. check that FDB of port group 2 has it , other FDBs don't
*         i. In 64 K mode:
*         a. check that FDB of port group 0,2 has it , other FDBs don't
*         j. learn same mac A from port N4 in port group 3
*         k. In 128K mode:
*         a. check that FDB of port group 3 has it , other FDBs don't
*         l. In 64 K mode:
*         a. check that FDB of port group 1,3 has it , other FDBs don't
*         2. check address movement on trunk ports (in same trunk)
*         a. Add the network ports to trunk.
*         b. learn mac A from port N1 in port group 0
*         c. In 128K mode:
*         a. check that FDB of port group 0 has it , other FDBs don't
*         d. In 64 K mode:
*         a. check that FDB of port group 0,2 has it , other FDBs don't
*         e. learn same mac A from port N2 in port group 1
*         f. In 128K mode:
*         a. check that FDB of port group 1 has it , other FDBs don't
*         g. In 64 K mode:
*         a. check that FDB of port group 1,3 has it , other FDBs don't
*         h. learn same mac A from port N3 in port group 2
*         i. In 128K mode:
*         a. check that FDB of port group 2 has it , other FDBs don't
*         j. In 64 K mode:
*         a. check that FDB of port group 0,2 has it , other FDBs don't
*         k. learn same mac A from port N4 in port group 3
*         l. In 128K mode:
*         a. check that FDB of port group 3 has it , other FDBs don't
*         m. In 64 K mode:
*         a. check that FDB of port group 1,3 has it , other FDBs don't
*         3. check station movement from trunk to trunk
*         a. set N1,N3 in trunk A
*         b. set N2,N4 in trunk B
*         c. learn mac A from port N1 in port group 0
*         d. In 128K mode:
*         a. check that FDB of port group 0 has it , other FDBs don't' on Tr_A
*         e. In 64 K mode:
*         a. check that FDB of port group 0,2 has it , other FDBs don't' on Tr_A
*         f. learn same mac A from port N2 in port group 1
*         g. In 128K mode:
*         a. check that FDB of port group 1 has it , other FDBs don't' on Tr_B
*         h. In 64 K mode:
*         a. check that FDB of port group 1,3 has it , other FDBs don't' on Tr_B
*         i. learn same mac A from port N3 in port group 2
*         j. In 128K mode:
*         a. check that FDB of port group 2 has it , other FDBs don't' on Tr_A
*         k. In 64 K mode:
*         a. check that FDB of port group 0,2 has it , other FDBs don't' on Tr_A
*         l. learn same mac A from port N4 in port group 3
*         m. In 128K mode:
*         a. check that FDB of port group 3 has it , other FDBs don't' on Tr_B
*         n. In 64 K mode:
*         a. check that FDB of port group 1,3 has it , other FDBs don't' on Tr_B
*         4. check station movement from trunk to port , and port to trunk
*         a. set N1,N3 in trunk A
*         b. learn mac A from port N1 in port group 0
*         c. In 128K mode:
*         a. check that FDB of port group 0 has it , other FDBs don't' on Tr_A
*         d. In 64 K mode:
*         a. check that FDB of port group 0,2 has it , other FDBs don't' on Tr_A
*         e. learn same mac A from port N2 in port group 1
*         f. In 128K mode:
*         a. check that FDB of port group 1 has it , other FDBs don't' on port N2
*         g. In 64 K mode:
*         a. check that FDB of port group 1,3 has it , other FDBs don't' on port N2
*         h. learn same mac A from port N3 in port group 2
*         i. In 128K mode:
*         a. check that FDB of port group 2 has it , other FDBs don't' on Tr_A
*         j. In 64 K mode:
*         a. check that FDB of port group 0,2 has it , other FDBs don't' on Tr_A
*         k. learn same mac A from port N4 in port group 3
*         l. In 128K mode:
*         a. check that FDB of port group 3 has it , other FDBs don't' on port N4
*         m. In 64 K mode:
*         a. check that FDB of port group 1,3 has it , other FDBs don't' on port N4
* @param[in] mode                     - station movement  to test
*                                       None
*/
GT_VOID tgfBridgeFdbStationMovementRun_ForMultiCoreFdbLookup
(
    IN PRV_TGF_BRG_FDB_STATION_MOVEMENT_MODE_ENT     mode
)
{
    GT_STATUS   rc;
    GT_TRUNK_ID trunkIdArr[2] = {PRV_TGF_TRUNK_ID_CNS,PRV_TGF_ANOTHER_TRUNK_ID_CNS};
    CPSS_TRUNK_MEMBER_STC    trunkMember;/* trunk member */
    GT_U32  portIndexArr[8] = {N1_INDEX_E,N3_INDEX_E,N5_INDEX_E,N7_INDEX_E,
                               N2_INDEX_E,N4_INDEX_E,N6_INDEX_E,N8_INDEX_E};
    GT_U32  portIndex;
    TGF_MAC_ADDR    saMac = {0x00,0x01,0x23,0x45,0x67,0x89};
    PRV_TGF_MAC_ENTRY_KEY_STC searchKey;
    PRV_TGF_BRG_MAC_ENTRY_STC macEntry;
    GT_PORT_GROUPS_BMP portGroupsBmp;
    GT_U32      ii;
    GT_U32  index;/*index to access prvTgfDevsArray[] and prvTgfPortsArray[] */

    switch(mode)
    {
        case PRV_TGF_BRG_FDB_STATION_MOVEMENT_MODE_PORT_TO_PORT_E:
            utfGeneralStateMessageSave(INDEX_2_CNS,"mode=[%s]",
                "PRV_TGF_BRG_FDB_STATION_MOVEMENT_MODE_PORT_TO_PORT_E");
            break;
        case PRV_TGF_BRG_FDB_STATION_MOVEMENT_MODE_PORT_TO_TRUNK_E:
            utfGeneralStateMessageSave(INDEX_2_CNS,"mode=[%s]",
                "PRV_TGF_BRG_FDB_STATION_MOVEMENT_MODE_PORT_TO_TRUNK_E");
            break;
        case PRV_TGF_BRG_FDB_STATION_MOVEMENT_MODE_TRUNK_TO_PORT_E:
            utfGeneralStateMessageSave(INDEX_2_CNS,"mode=[%s]",
                "PRV_TGF_BRG_FDB_STATION_MOVEMENT_MODE_TRUNK_TO_PORT_E");
            break;
        case PRV_TGF_BRG_FDB_STATION_MOVEMENT_MODE_TRUNK_TO_TRUNK_E:
            utfGeneralStateMessageSave(INDEX_2_CNS,"mode=[%s]",
                "PRV_TGF_BRG_FDB_STATION_MOVEMENT_MODE_TRUNK_TO_TRUNK_E");
            break;
        case PRV_TGF_BRG_FDB_STATION_MOVEMENT_MODE_IN_THE_TRUNK_E:
            utfGeneralStateMessageSave(INDEX_2_CNS,"mode=[%s]",
                "PRV_TGF_BRG_FDB_STATION_MOVEMENT_MODE_IN_THE_TRUNK_E");
            break;
        default:
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, GT_BAD_PARAM);
            return;

    }

    searchKey.entryType = PRV_TGF_FDB_ENTRY_TYPE_MAC_ADDR_E;
    cpssOsMemCpy(searchKey.key.macVlan.macAddr.arEther,saMac,sizeof(saMac));
    searchKey.key.macVlan.vlanId = prvTgfVansArr[prvTgfCurrVlanIndex];

    PRV_UTF_LOG0_MAC("======= tgfBridgeFdbStationMovementRun_ForMultiCoreFdbLookup : clear the 2 trunks =======\n");

    /* clear the 2 trunks */
    for(ii = 0 ; ii < 2 ; ii++)
    {
        CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkIdArr[ii]);

        rc = prvTgfTrunkMembersSet(
            trunkIdArr[ii] /*trunkId*/,
            0 /*numOfEnabledMembers*/,
            NULL,
            0 /*numOfDisabledMembers*/,
            NULL  /*disabledMembersArray*/);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
    }


    PRV_UTF_LOG0_MAC("======= : add ports to trunks =======\n");
    /* add ports to trunks */
    switch(mode)
    {
        case PRV_TGF_BRG_FDB_STATION_MOVEMENT_MODE_PORT_TO_TRUNK_E:
        case PRV_TGF_BRG_FDB_STATION_MOVEMENT_MODE_TRUNK_TO_PORT_E:
        case PRV_TGF_BRG_FDB_STATION_MOVEMENT_MODE_TRUNK_TO_TRUNK_E:
        case PRV_TGF_BRG_FDB_STATION_MOVEMENT_MODE_IN_THE_TRUNK_E:
            /* ports N1,N3 in trunk A */
            for (ii = 0; ii < (PRV_CPSS_PP_MAC(prvTgfDevNum)->portGroupsInfo.numOfPortGroups / 2); ii++)
            {
                portIndex = portIndexArr[ii];

                index = PORT_INDEX_CONVERT_MAC(portIndex);

                trunkMember.hwDevice = prvTgfDevsArray[index];
                trunkMember.port = prvTgfPortsArray[index];

                rc = prvTgfTrunkMemberAdd(trunkIdArr[0], &trunkMember);
                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
            }
            break;
        default:
            break;
    }

    /* add ports to trunks */
    switch(mode)
    {
        case PRV_TGF_BRG_FDB_STATION_MOVEMENT_MODE_IN_THE_TRUNK_E:
            /* ports N2,N4 in trunk A */
            for (ii = 0; ii < (PRV_CPSS_PP_MAC(prvTgfDevNum)->portGroupsInfo.numOfPortGroups / 2); ii++)
            {
                portIndex = portIndexArr[ii + 4];

                index = PORT_INDEX_CONVERT_MAC(portIndex);

                trunkMember.hwDevice = prvTgfDevsArray[index];
                trunkMember.port = prvTgfPortsArray[index];

                rc = prvTgfTrunkMemberAdd(trunkIdArr[0], &trunkMember);
                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
            }
            break;
        default:
            break;
    }

    /* add ports to trunks */
    switch(mode)
    {
        case PRV_TGF_BRG_FDB_STATION_MOVEMENT_MODE_TRUNK_TO_TRUNK_E:
            /* ports N2,N4 in trunk B*/
            for (ii = 0; ii < (PRV_CPSS_PP_MAC(prvTgfDevNum)->portGroupsInfo.numOfPortGroups / 2); ii++)
            {
                portIndex = portIndexArr[ii + 4];

                index = PORT_INDEX_CONVERT_MAC(portIndex);

                trunkMember.hwDevice = prvTgfDevsArray[index];
                trunkMember.port = prvTgfPortsArray[index];

                rc = prvTgfTrunkMemberAdd(trunkIdArr[1], &trunkMember);
                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
            }
            break;
        default:
            break;
    }

    PRV_UTF_LOG0_MAC("======= : send frame with SA from the port =======\n");

    for (portIndex = N1_INDEX_E; portIndex <= LAST_NETWORK_PORT_INDEX_CNS; portIndex++)
    {
        index = PORT_INDEX_CONVERT_MAC(portIndex);

        /* send frame with SA from the port */
        prvTgfCommonIncrementalSaMacSend(
            prvTgfDevsArray[index],
            prvTgfPortsArray[index],
            &saMac,1);

        /* check that the FDB learned the mac on this port :
           in 128K mode : in the proper port group (and not exists in others)
           in 64K mode  : in 2 proper port groups (and not exists in others)
        */

        switch(fdbMode)
        {
            case PRV_TGF_FDB_QUAD_MODE_E:
                portGroupsBmp = BIT_0 <<
                    (U32_GET_FIELD_MAC(prvTgfPortsArray[index],4,2));
                break;
            case PRV_TGF_FDB_DOUBLE_MODE_E:
                portGroupsBmp = BIT_0 <<
                    (U32_GET_FIELD_MAC(prvTgfPortsArray[index],4,2));
                switch(portGroupsBmp & 0xF)
                {
                    case BIT_1:
                    case BIT_3:
                        portGroupsBmp = BIT_1 | BIT_3;
                        break;
                    default:
                        portGroupsBmp = BIT_0 | BIT_2;
                        break;
                }
                break;
            default:
                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, GT_NOT_SUPPORTED);
                return;
        }

        if(PRV_CPSS_PP_MAC(prvTgfDevNum)->portGroupsInfo.numOfPortGroups == 8)
        {
            /* the first 4 cores hold the same values as the next 4 cores */
            portGroupsBmp |= portGroupsBmp << 4;
        }

        for(currPortGroupsBmp = BIT_0 ;
            currPortGroupsBmp <= (GT_U32)(BIT_0 << (PRV_CPSS_PP_MAC(prvTgfDevNum)->portGroupsInfo.numOfPortGroups - 1)) ;
            currPortGroupsBmp <<= 1)
        {
            rc = prvTgfBrgFdbMacEntryFind(&searchKey,NULL,NULL,NULL,&macEntry);
            if(currPortGroupsBmp & portGroupsBmp)
            {
                /* the entry must exists in the FDB on the port */
                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

                switch(mode)
                {
                    case PRV_TGF_BRG_FDB_STATION_MOVEMENT_MODE_PORT_TO_PORT_E:
                        UTF_VERIFY_EQUAL0_PARAM_MAC(CPSS_INTERFACE_PORT_E, macEntry.dstInterface.type);
                        UTF_VERIFY_EQUAL0_PARAM_MAC(prvTgfDevsArray[index],
                                                    macEntry.dstInterface.devPort.hwDevNum);
                        UTF_VERIFY_EQUAL0_PARAM_MAC(prvTgfPortsArray[index],
                                                    macEntry.dstInterface.devPort.portNum);
                        break;
                    case PRV_TGF_BRG_FDB_STATION_MOVEMENT_MODE_IN_THE_TRUNK_E:
                        UTF_VERIFY_EQUAL0_PARAM_MAC(CPSS_INTERFACE_TRUNK_E, macEntry.dstInterface.type);
                        UTF_VERIFY_EQUAL0_PARAM_MAC(trunkIdArr[0],macEntry.dstInterface.trunkId);
                        break;
                    case PRV_TGF_BRG_FDB_STATION_MOVEMENT_MODE_TRUNK_TO_TRUNK_E:
                        UTF_VERIFY_EQUAL0_PARAM_MAC(CPSS_INTERFACE_TRUNK_E, macEntry.dstInterface.type);
                        UTF_VERIFY_EQUAL0_PARAM_MAC(trunkIdArr[portIndex & 1],macEntry.dstInterface.trunkId);
                        break;
                    case PRV_TGF_BRG_FDB_STATION_MOVEMENT_MODE_PORT_TO_TRUNK_E:
                    case PRV_TGF_BRG_FDB_STATION_MOVEMENT_MODE_TRUNK_TO_PORT_E:
                        if(portIndex & 1)
                        {
                            UTF_VERIFY_EQUAL0_PARAM_MAC(CPSS_INTERFACE_PORT_E, macEntry.dstInterface.type);
                            UTF_VERIFY_EQUAL0_PARAM_MAC(prvTgfDevsArray[index],
                                                        macEntry.dstInterface.devPort.hwDevNum);
                            UTF_VERIFY_EQUAL0_PARAM_MAC(prvTgfPortsArray[index],
                                                        macEntry.dstInterface.devPort.portNum);
                        }
                        else
                        {
                            UTF_VERIFY_EQUAL0_PARAM_MAC(CPSS_INTERFACE_TRUNK_E, macEntry.dstInterface.type);
                            UTF_VERIFY_EQUAL0_PARAM_MAC(trunkIdArr[0],macEntry.dstInterface.trunkId);
                        }

                        break;
                    default:
                        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, GT_NOT_SUPPORTED);
                        return;
                }

            }
            else
            {
                /* the entry must NOT exists in the FDB */
                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_NOT_FOUND, rc);
            }
        }/*currPortGroupsBmp*/

    }/*loop portIndex*/

    /* test ended */
    PRV_UTF_LOG0_MAC("======= tgfBridgeFdbStationMovementRun_ForMultiCoreFdbLookup : ended =======\n");

    return;



}

/**
* @internal tgfBridgeFdbStationMovementRestore_ForMultiCoreFdbLookup function
* @endinternal
*
* @brief   Restore_ForMultiCoreFdbLookup test configuration
*/
GT_VOID tgfBridgeFdbStationMovementRestore_ForMultiCoreFdbLookup
(
    GT_VOID
)
{
    tgfBridgeFdbRestore_ForMultiCoreFdbLookup();
}

/**
* @internal tgfBridgeFdbAgeBitRun function
* @endinternal
*
* @brief   4.7    Test 9: check FDB mac entry's 'Age bit' on specific port group
*         (Covered API: FDB :
*         1. cpssDxChBrgFdbPortGroupMacEntrySet
*         a. cpssDxChBrgFdbPortGroupFromCpuAuMsgStatusGet
*         2. cpssDxChBrgFdbPortGroupMacEntryAgeBitSet
*         )
*         1. check FDB mac entry's 'Age bit' on specific port group
*         a. Add (by message /index) FDB entry (not static) on port N1 to all 4 port groups
*         b. set age bit to GT_FALSE on port groups 1,2,3 (not 0)
*         c. read the FDB and check that the entry on port group 0 with GT_TRUE , and others with GT_FALSE
*         d. trigger aging loop by HW
*         e. read the FDB and check that the entry on ALL port groups with 'Age bit' = 'GT_FALSE
*         f. trigger aging loop by HW
*         g. read the FDB and check that the entry removed from all port groups
*         h. ...
*         i. Add (by message /index) FDB entry (not static) on port N1 to all 4 port groups
*         j. set age bit to GT_FALSE on port groups 0,1,3 (not 2)
*         k. read the FDB and check that the entry on port group 2 with 'Age bit' = GT_TRUE , and others with 'Age bit' = GT_FALSE
*         l. trigger aging loop by HW
*         m. read the FDB and check that the entry removed from all port groups
*         i. because the port N1 associated with port group 0 , and the age bit was GT_FALSE prior to the trigger aging , the AA was sent to the AppDemo , that in turn deleted it from all the port groups.
* @param[in] hemisphereId             - hemisphere ID
*                                       None
*/
static GT_VOID tgfBridgeFdbAgeBitRun
(
    IN  GT_U32  hemisphereId
)
{
    GT_STATUS   rc;
    GT_U8   devNum = prvTgfDevsArray[0];
    GT_U32      index;/* fdb index */
    GT_U32      ii;
    PRV_TGF_BRG_MAC_ENTRY_STC      macEntry = PRV_TGF_BRG_FDB_MAC_ENTRY_DEFAULT_MAC;
    GT_BOOL age;/* age bit */
    GT_BOOL                      tmp_valid;
    GT_BOOL                      tmp_skip;
    GT_BOOL                      tmp_aged;
    GT_HW_DEV_NUM                tmp_hwDevNum;
    PRV_TGF_BRG_MAC_ENTRY_STC    tmp_macEntry;
    GT_U32                       portGroupsArr[]={0,2,5,7};
    GT_U32                       *portGroupsPtr;
    GT_U32                       portIndexArr[]={N1_INDEX_E,N6_INDEX_E};
    GT_U32                       portIndex;

    if(hemisphereId >= 2)
    {
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, GT_NOT_IMPLEMENTED);
        return;
    }

    portGroupsPtr = &portGroupsArr[hemisphereId * 2];
    portIndex = portIndexArr[hemisphereId];


    PRV_UTF_LOG0_MAC("======= tgfBridgeFdbAgeBitRun : Set aging to 'trigger mode' =======\n");

    /* Set aging to 'trigger mode'*/
    rc = prvTgfBrgFdbMacTriggerModeSet(PRV_TGF_ACT_TRIG_E);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    PRV_UTF_LOG0_MAC("======= : Set age without removal =======\n");
    /* Set age without removal*/
    rc = prvTgfBrgFdbActionModeSet(PRV_TGF_FDB_ACTION_AGE_WITHOUT_REMOVAL_E);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

     /* Set the FDB entry mac address */
    cpssOsMemCpy(macEntry.key.key.macVlan.macAddr.arEther,
                 &prvTgfPacketL2Part.daMac,
                 sizeof(TGF_MAC_ADDR));

    macEntry.key.key.macVlan.vlanId         = prvTgfVansArr[prvTgfCurrVlanIndex];
    macEntry.dstInterface.type              = CPSS_INTERFACE_PORT_E;

    PRV_UTF_LOG0_MAC("======= : calculate the index of the inserted mac address =======\n");
    /* calculate the index of the inserted mac address */
    rc = prvTgfBrgFdbMacEntryIndexFind(&macEntry.key,&index);
    /* the mac address should not be found in the FDB at this stage ...*/
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_NOT_FOUND, rc);

    PRV_UTF_LOG0_MAC("======= : Add (by message /index) FDB entry (not static) on port N1 to all 4 port groups =======\n");
    /*a. Add (by message /index) FDB entry (not static) on port N1 to all 4 port groups*/
    macEntry.dstInterface.devPort.hwDevNum    = prvTgfDevsArray[PORT_INDEX_CONVERT_MAC(portIndex)];
    macEntry.dstInterface.devPort.portNum   = prvTgfPortsArray[PORT_INDEX_CONVERT_MAC(portIndex)];

    currPortGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
    rc = prvTgfBrgFdbMacEntrySet(&macEntry);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    PRV_UTF_LOG0_MAC("======= : check that entry exists =======\n");

    rc = prvTgfBrgFdbMacEntryRead(index, &tmp_valid, &tmp_skip, &tmp_aged,
                                  &tmp_hwDevNum, &tmp_macEntry);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_TRUE, tmp_valid);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_FALSE, tmp_skip);
    UTF_VERIFY_EQUAL0_PARAM_MAC(devNum, tmp_hwDevNum);
    if(0 != prvTgfBrgFdbMacCompare(&tmp_macEntry,&macEntry))
    {
        /* entries not match !? */
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_FALSE, GT_TRUE);
        return;
    }

    PRV_UTF_LOG0_MAC("======= : set age bit to GT_FALSE on port groups 1,2,3 (not 0) =======\n");
    /*b. set age bit to GT_FALSE on port groups 1,2,3 (not 0)*/
    for(ii = 0 ; ii < PRV_CPSS_PP_MAC(prvTgfDevNum)->portGroupsInfo.numOfPortGroups ; ii++)
    {
        currPortGroupsBmp = BIT_0 << ii;

        if(ii == portGroupsPtr[0])
        {
            age = GT_TRUE;
        }
        else
        {
            age = GT_FALSE;
        }

        rc = prvTgfBrgFdbMacEntryAgeBitSet(index,age);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
    }

    PRV_UTF_LOG0_MAC("======= : read the FDB and check that the entry on port group 0 with GT_TRUE , and others with GT_FALSE =======\n");
    /*c. read the FDB and check that the entry on port group 0 with GT_TRUE , and others with GT_FALSE*/
    for(ii = 0 ; ii < PRV_CPSS_PP_MAC(prvTgfDevNum)->portGroupsInfo.numOfPortGroups ; ii++)
    {
        currPortGroupsBmp = BIT_0 << ii;

        if(ii == portGroupsPtr[0])
        {
            age = GT_TRUE;
        }
        else
        {
            age = GT_FALSE;
        }

        rc = prvTgfBrgFdbMacEntryRead(index, &tmp_valid, &tmp_skip, &tmp_aged,
                                      &tmp_hwDevNum, &tmp_macEntry);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_TRUE, tmp_valid);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_FALSE, tmp_skip);
        UTF_VERIFY_EQUAL0_PARAM_MAC(age, tmp_aged); /* !!! compare the age bit !!! */
        UTF_VERIFY_EQUAL0_PARAM_MAC(devNum, tmp_hwDevNum);
        if(0 != prvTgfBrgFdbMacCompare(&tmp_macEntry,&macEntry))
        {
            /* entries not match !? */
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_FALSE, GT_TRUE);
            return;
        }
    }

    PRV_UTF_LOG0_MAC("======= : trigger aging loop by HW =======\n");
    /*d. trigger aging loop by HW*/
    rc = prvTgfBrgFdbActionStart(PRV_TGF_FDB_ACTION_AGE_WITHOUT_REMOVAL_E);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    PRV_UTF_LOG0_MAC("======= : wait for trigger action to end =======\n");
    /* wait for trigger action to end */
    rc = prvTgfBrgFdbActionDoneWait(GT_FALSE);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    PRV_UTF_LOG0_MAC("======= : read the FDB and check that the entry on ALL port groups with 'Age bit' = 'GT_FALSE =======\n");
    /*e. read the FDB and check that the entry on ALL port groups with 'Age bit' = 'GT_FALSE*/
    age = GT_FALSE;
    for(ii = 0 ; ii < PRV_CPSS_PP_MAC(prvTgfDevNum)->portGroupsInfo.numOfPortGroups ; ii++)
    {
        currPortGroupsBmp = BIT_0 << ii;

        rc = prvTgfBrgFdbMacEntryRead(index, &tmp_valid, &tmp_skip, &tmp_aged,
                                      &tmp_hwDevNum, &tmp_macEntry);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_TRUE, tmp_valid);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_FALSE, tmp_skip);
        UTF_VERIFY_EQUAL0_PARAM_MAC(age, tmp_aged); /* !!! compare the age bit !!! */
        UTF_VERIFY_EQUAL0_PARAM_MAC(devNum, tmp_hwDevNum);
        if(0 != prvTgfBrgFdbMacCompare(&tmp_macEntry,&macEntry))
        {
            /* entries not match !? */
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_FALSE, GT_TRUE);
            return;
        }
    }

    PRV_UTF_LOG0_MAC("======= : trigger aging loop by HW =======\n");
    /*f. trigger aging loop by HW*/
    rc = prvTgfBrgFdbActionStart(PRV_TGF_FDB_ACTION_AGE_WITHOUT_REMOVAL_E);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    PRV_UTF_LOG0_MAC("======= : wait for trigger action to end =======\n");
    /* wait for trigger action to end */
    rc = prvTgfBrgFdbActionDoneWait(GT_FALSE);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);


    PRV_UTF_LOG0_MAC("======= : read the FDB and check that the entry removed from all port groups =======\n");
    /*g. read the FDB and check that the entry removed from all port groups*/
    for(ii = 0 ; ii < PRV_CPSS_PP_MAC(prvTgfDevNum)->portGroupsInfo.numOfPortGroups ; ii++)
    {
        currPortGroupsBmp = BIT_0 << ii;

        rc = prvTgfBrgFdbMacEntryRead(index, &tmp_valid, &tmp_skip, &tmp_aged,
                                      &tmp_hwDevNum, &tmp_macEntry);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

        /* entry can be NOT relevant if one of the next:
           1. valid = false
           2. valid = true and skip = true
           */

        if(tmp_valid == GT_TRUE)
        {
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_TRUE, tmp_skip); /* SKIP must be true */
        }
    }


    PRV_UTF_LOG0_MAC("======= : Add (by message /index) FDB entry (not static) on port N1 to all 4 port groups =======\n");
    /*i. Add (by message /index) FDB entry (not static) on port N1 to all 4 port groups*/
    currPortGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
    rc = prvTgfBrgFdbMacEntrySet(&macEntry);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    PRV_UTF_LOG0_MAC("======= : set age bit to GT_FALSE on port groups 0,1,3 (not 2) =======\n");
    /*j. set age bit to GT_FALSE on port groups 0,1,3 (not 2)*/
    for(ii = 0 ; ii < PRV_CPSS_PP_MAC(prvTgfDevNum)->portGroupsInfo.numOfPortGroups ; ii++)
    {
        currPortGroupsBmp = BIT_0 << ii;

        if(ii == portGroupsPtr[1])
        {
            age = GT_TRUE;
        }
        else
        {
            age = GT_FALSE;
        }

        rc = prvTgfBrgFdbMacEntryAgeBitSet(index,age);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
    }

    PRV_UTF_LOG0_MAC("======= : read the FDB and check that the entry on port group 2 with 'Age bit' = GT_TRUE ,"
        "and others with 'Age bit' = GT_FALSE =======\n");
    /*k. read the FDB and check that the entry on port group 2 with 'Age bit' = GT_TRUE ,
        and others with 'Age bit' = GT_FALSE*/
    for(ii = 0 ; ii < PRV_CPSS_PP_MAC(prvTgfDevNum)->portGroupsInfo.numOfPortGroups ; ii++)
    {
        currPortGroupsBmp = BIT_0 << ii;

        if(ii == portGroupsPtr[1])
        {
            age = GT_TRUE;
        }
        else
        {
            age = GT_FALSE;
        }

        rc = prvTgfBrgFdbMacEntryRead(index, &tmp_valid, &tmp_skip, &tmp_aged,
                                      &tmp_hwDevNum, &tmp_macEntry);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_TRUE, tmp_valid);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_FALSE, tmp_skip);
        UTF_VERIFY_EQUAL0_PARAM_MAC(age, tmp_aged); /* !!! compare the age bit !!! */
        UTF_VERIFY_EQUAL0_PARAM_MAC(devNum, tmp_hwDevNum);
        if(0 != prvTgfBrgFdbMacCompare(&tmp_macEntry,&macEntry))
        {
            /* entries not match !? */
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_FALSE, GT_TRUE);
            return;
        }
    }

    PRV_UTF_LOG0_MAC("======= : trigger aging loop by HW =======\n");
    /*l. trigger aging loop by HW*/
    rc = prvTgfBrgFdbActionStart(PRV_TGF_FDB_ACTION_AGE_WITHOUT_REMOVAL_E);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    PRV_UTF_LOG0_MAC("======= : wait for trigger action to end =======\n");
    /* wait for trigger action to end */
    rc = prvTgfBrgFdbActionDoneWait(GT_FALSE);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    PRV_UTF_LOG0_MAC("======= : read the FDB and check that the entry removed from all port groups =======\n");
    /*m. read the FDB and check that the entry removed from all port groups
        i. because the port N1 associated with port group 0 , and the age bit
            was GT_FALSE prior to the trigger aging , the AA was sent to the AppDemo ,
            that in turn deleted it from all the port groups.
    */
    for(ii = 0 ; ii < PRV_CPSS_PP_MAC(prvTgfDevNum)->portGroupsInfo.numOfPortGroups ; ii++)
    {
        currPortGroupsBmp = BIT_0 << ii;

        rc = prvTgfBrgFdbMacEntryRead(index, &tmp_valid, &tmp_skip, &tmp_aged,
                                      &tmp_hwDevNum, &tmp_macEntry);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

        /* entry can be NOT relevant if one of the next:
           1. valid = false
           2. valid = true and skip = true
           */

        if(tmp_valid == GT_TRUE)
        {
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_TRUE, tmp_skip); /* SKIP must be true */
        }
    }

    PRV_UTF_LOG0_MAC("======= tgfBridgeFdbAgeBitRun : ended =======\n");

}
/**
* @internal tgfBridgeFdbAgeBitRun_ForMultiCoreFdbLookup function
* @endinternal
*
* @brief   4.7    Test 9: check FDB mac entry's 'Age bit' on specific port group
*         (Covered API: FDB :
*         1. cpssDxChBrgFdbPortGroupMacEntrySet
*         a. cpssDxChBrgFdbPortGroupFromCpuAuMsgStatusGet
*         2. cpssDxChBrgFdbPortGroupMacEntryAgeBitSet
*         )
*         1. check FDB mac entry's 'Age bit' on specific port group
*         a. Add (by message /index) FDB entry (not static) on port N1 to all 4 port groups
*         b. set age bit to GT_FALSE on port groups 1,2,3 (not 0)
*         c. read the FDB and check that the entry on port group 0 with GT_TRUE , and others with GT_FALSE
*         d. trigger aging loop by HW
*         e. read the FDB and check that the entry on ALL port groups with 'Age bit' = 'GT_FALSE
*         f. trigger aging loop by HW
*         g. read the FDB and check that the entry removed from all port groups
*         h. ...
*         i. Add (by message /index) FDB entry (not static) on port N1 to all 4 port groups
*         j. set age bit to GT_FALSE on port groups 0,1,3 (not 2)
*         k. read the FDB and check that the entry on port group 2 with 'Age bit' = GT_TRUE , and others with 'Age bit' = GT_FALSE
*         l. trigger aging loop by HW
*         m. read the FDB and check that the entry removed from all port groups
*         i. because the port N1 associated with port group 0 , and the age bit was GT_FALSE prior to the trigger aging , the AA was sent to the AppDemo , that in turn deleted it from all the port groups.
*/
GT_VOID tgfBridgeFdbAgeBitRun_ForMultiCoreFdbLookup
(
    GT_VOID
)
{
    tgfBridgeFdbAgeBitRun(0);
    if(PRV_CPSS_PP_MAC(prvTgfDevNum)->portGroupsInfo.numOfPortGroups > 4)
    {
        tgfBridgeFdbAgeBitRun(1);
    }
}

/**
* @internal tgfBridgeFdbAgeBitRestore_ForMultiCoreFdbLookup function
* @endinternal
*
* @brief   Restore_ForMultiCoreFdbLookup test configuration
*/
GT_VOID tgfBridgeFdbAgeBitRestore_ForMultiCoreFdbLookup
(
    GT_VOID
)
{
    tgfBridgeFdbRestore_ForMultiCoreFdbLookup();
}


/**
* @internal tgfBridgeFdbValidAndSkipBitsRun function
* @endinternal
*
* @brief   4.8    Test 10: check FDB mac entry Get valid bit , skip bit on specific port group
*         (Covered API: FDB :
*         1. cpssDxChBrgFdbPortGroupMacEntrySet
*         a. cpssDxChBrgFdbPortGroupFromCpuAuMsgStatusGet
*         2. cpssDxChBrgFdbPortGroupMacEntryStatusGet
*         )
*         1. check FDB mac entry Get valid bit , skip bit on specific port group
*         a. Add by (message / by index) FDB entry on port N1 to all port groups
*         b. Check status of entry on each port group : cpssDxChBrgFdbPortGroupMacEntryStatusGet , should be:
*         i. Valid = GT_TRUE
*         ii. Skip = GT_FALSE
*         c. Delete by message the entry from port group 0
*         d. Check status of entry on port group 0: cpssDxChBrgFdbPortGroupMacEntryStatusGet , should be:
*         i. Valid = GT_TRUE
*         ii. Skip = GT_TRUE
*         e. Delete by message the entry from port group 3
*         f. Check status of entry on port group 3: cpssDxChBrgFdbPortGroupMacEntryStatusGet , should be:
*         i. Valid = GT_TRUE
*         ii. Skip = GT_TRUE
*         g. Delete by index the entry from port group 2
*         h. Check status of entry on port group 2: cpssDxChBrgFdbPortGroupMacEntryStatusGet , should be:
*         i. Valid = GT_FALSE
*         ii. Skip = GT_FALSE
*         i. Check status of entry on port group 1 (no change done on it): cpssDxChBrgFdbPortGroupMacEntryStatusGet , should be:
*         i. Valid = GT_TRUE
*         ii. Skip = GT_FALSE
* @param[in] hemisphereId             - hemisphere ID
*                                       None
*/
static GT_VOID tgfBridgeFdbValidAndSkipBitsRun
(
    IN  GT_U32  hemisphereId
)
{
    GT_STATUS   rc;
    GT_U8   devNum = prvTgfDevsArray[0];
    GT_U32      index;/* fdb index */
    GT_U32      ii;
    PRV_TGF_BRG_MAC_ENTRY_STC      macEntry = PRV_TGF_BRG_FDB_MAC_ENTRY_DEFAULT_MAC;
    GT_BOOL                      tmp_valid;
    GT_BOOL                      tmp_skip;
    GT_BOOL                      tmp_aged;
    GT_HW_DEV_NUM                tmp_hwDevNum;
    PRV_TGF_BRG_MAC_ENTRY_STC    tmp_macEntry;
    GT_U32                       portGroupsArr[]={0,3,2,1,5,6,7,4};
    GT_U32                       *portGroupsPtr;
    GT_U32                       portIndexArr[]={N1_INDEX_E,N6_INDEX_E};
    GT_U32                       portIndex;

    if(hemisphereId >= 2)
    {
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, GT_NOT_IMPLEMENTED);
        return;
    }

    portGroupsPtr = &portGroupsArr[hemisphereId * 4];
    portIndex = portIndexArr[hemisphereId];

     /* Set the FDB entry mac address */
    cpssOsMemCpy(macEntry.key.key.macVlan.macAddr.arEther,
                 &prvTgfPacketL2Part.daMac,
                 sizeof(TGF_MAC_ADDR));

    macEntry.key.key.macVlan.vlanId         = prvTgfVansArr[prvTgfCurrVlanIndex];
    macEntry.dstInterface.type              = CPSS_INTERFACE_PORT_E;

    PRV_UTF_LOG0_MAC("======= tgfBridgeFdbValidAndSkipBitsRun : calculate the index of the inserted mac address =======\n");

    /* calculate the index of the inserted mac address */
    rc = prvTgfBrgFdbMacEntryIndexFind(&macEntry.key,&index);
    /* the mac address should not be found in the FDB at this stage ...*/
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_NOT_FOUND, rc);

    PRV_UTF_LOG0_MAC("======= : Add by (message / by index) FDB entry on port N1 to all port groups =======\n");
    /*a. Add by (message / by index) FDB entry on port N1 to all port groups*/
    macEntry.dstInterface.devPort.hwDevNum    = prvTgfDevsArray[PORT_INDEX_CONVERT_MAC(portIndex)];
    macEntry.dstInterface.devPort.portNum   = prvTgfPortsArray[PORT_INDEX_CONVERT_MAC(portIndex)];

    currPortGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
    rc = prvTgfBrgFdbMacEntrySet(&macEntry);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);


    PRV_UTF_LOG0_MAC("======= : check that entry exists =======\n");
    rc = prvTgfBrgFdbMacEntryRead(index, &tmp_valid, &tmp_skip, &tmp_aged,
                                  &tmp_hwDevNum, &tmp_macEntry);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_TRUE, tmp_valid);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_FALSE, tmp_skip);
    UTF_VERIFY_EQUAL0_PARAM_MAC(devNum, tmp_hwDevNum);
    if(0 != prvTgfBrgFdbMacCompare(&tmp_macEntry,&macEntry))
    {
        /* entries not match !? */
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_FALSE, GT_TRUE);
        return;
    }

    PRV_UTF_LOG0_MAC("======= : Check status of entry on each port group : cpssDxChBrgFdbPortGroupMacEntryStatusGet =======\n");
    /*b. Check status of entry on each port group : cpssDxChBrgFdbPortGroupMacEntryStatusGet , should be:
        i. Valid = GT_TRUE
        ii. Skip = GT_FALSE*/
    for(ii = 0 ; ii < PRV_CPSS_PP_MAC(prvTgfDevNum)->portGroupsInfo.numOfPortGroups ; ii++)
    {
        currPortGroupsBmp = BIT_0 << ii;

        rc = prvTgfBrgFdbMacEntryStatusGet(index,&tmp_valid,&tmp_skip);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_TRUE, tmp_valid);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_FALSE, tmp_skip);
    }

    PRV_UTF_LOG0_MAC("======= : Delete by message the entry from port group 0 =======\n");
    /*c. Delete by message the entry from port group 0*/
    currPortGroupsBmp = BIT_0 << portGroupsPtr[0];
    rc = prvTgfBrgFdbMacEntryDelete(&macEntry.key);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    PRV_UTF_LOG0_MAC("======= : Check status of entry on each port group : cpssDxChBrgFdbPortGroupMacEntryStatusGet =======\n");
    /*d. Check status of entry on port group 0: cpssDxChBrgFdbPortGroupMacEntryStatusGet , should be:
        i. Valid = GT_TRUE
        ii. Skip = GT_TRUE*/
    rc = prvTgfBrgFdbMacEntryStatusGet(index,&tmp_valid,&tmp_skip);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_TRUE, tmp_valid);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_TRUE, tmp_skip);
    /*e. Delete by message the entry from port group 3*/
    currPortGroupsBmp = BIT_0 << portGroupsPtr[1];
    rc = prvTgfBrgFdbMacEntryDelete(&macEntry.key);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    PRV_UTF_LOG0_MAC("======= : Check status of entry on each port group : cpssDxChBrgFdbPortGroupMacEntryStatusGet =======\n");
    /*f. Check status of entry on port group 3: cpssDxChBrgFdbPortGroupMacEntryStatusGet , should be:
        i. Valid = GT_TRUE
        ii. Skip = GT_TRUE*/
    rc = prvTgfBrgFdbMacEntryStatusGet(index,&tmp_valid,&tmp_skip);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_TRUE, tmp_valid);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_TRUE, tmp_skip);

    /*g. Delete by index the entry from port group 2*/
    currPortGroupsBmp = BIT_0 << portGroupsPtr[2];
    rc = prvTgfBrgFdbMacEntryInvalidate(index);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    PRV_UTF_LOG0_MAC("======= : Check status of entry on each port group : cpssDxChBrgFdbPortGroupMacEntryStatusGet =======\n");
    /*h. Check status of entry on port group 2: cpssDxChBrgFdbPortGroupMacEntryStatusGet , should be:
        i. Valid = GT_FALSE
        ii. Skip = GT_FALSE*/
    rc = prvTgfBrgFdbMacEntryStatusGet(index,&tmp_valid,&tmp_skip);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_FALSE, tmp_valid);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_FALSE, tmp_skip);

    PRV_UTF_LOG0_MAC("======= : Check status of entry on each port group : cpssDxChBrgFdbPortGroupMacEntryStatusGet =======\n");
    /*i. Check status of entry on port group 1 (no change done on it): cpssDxChBrgFdbPortGroupMacEntryStatusGet , should be:
        i. Valid = GT_TRUE
        ii. Skip = GT_FALSE*/
    currPortGroupsBmp = BIT_0 << portGroupsPtr[3];

    rc = prvTgfBrgFdbMacEntryStatusGet(index,&tmp_valid,&tmp_skip);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_TRUE, tmp_valid);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_FALSE, tmp_skip);

    PRV_UTF_LOG0_MAC("======= tgfBridgeFdbValidAndSkipBitsRun : ended =======\n");
    return;
}
/**
* @internal tgfBridgeFdbValidAndSkipBitsRun_ForMultiCoreFdbLookup function
* @endinternal
*
* @brief   4.8    Test 10: check FDB mac entry Get valid bit , skip bit on specific port group
*         (Covered API: FDB :
*         1. cpssDxChBrgFdbPortGroupMacEntrySet
*         a. cpssDxChBrgFdbPortGroupFromCpuAuMsgStatusGet
*         2. cpssDxChBrgFdbPortGroupMacEntryStatusGet
*         )
*         1. check FDB mac entry Get valid bit , skip bit on specific port group
*         a. Add by (message / by index) FDB entry on port N1 to all port groups
*         b. Check status of entry on each port group : cpssDxChBrgFdbPortGroupMacEntryStatusGet , should be:
*         i. Valid = GT_TRUE
*         ii. Skip = GT_FALSE
*         c. Delete by message the entry from port group 0
*         d. Check status of entry on port group 0: cpssDxChBrgFdbPortGroupMacEntryStatusGet , should be:
*         i. Valid = GT_TRUE
*         ii. Skip = GT_TRUE
*         e. Delete by message the entry from port group 3
*         f. Check status of entry on port group 3: cpssDxChBrgFdbPortGroupMacEntryStatusGet , should be:
*         i. Valid = GT_TRUE
*         ii. Skip = GT_TRUE
*         g. Delete by index the entry from port group 2
*         h. Check status of entry on port group 2: cpssDxChBrgFdbPortGroupMacEntryStatusGet , should be:
*         i. Valid = GT_FALSE
*         ii. Skip = GT_FALSE
*         i. Check status of entry on port group 1 (no change done on it): cpssDxChBrgFdbPortGroupMacEntryStatusGet , should be:
*         i. Valid = GT_TRUE
*         ii. Skip = GT_FALSE
*/
GT_VOID tgfBridgeFdbValidAndSkipBitsRun_ForMultiCoreFdbLookup
(
    GT_VOID
)
{
    GT_STATUS   rc;

    tgfBridgeFdbValidAndSkipBitsRun(0);
    if(PRV_CPSS_PP_MAC(prvTgfDevNum)->portGroupsInfo.numOfPortGroups > 4)
    {
        PRV_UTF_LOG0_MAC("======= : flush FDB =======\n");
        /*c.        Flush FDB (trigger action 'delete') ' done on all port groups*/
        rc = prvTgfBrgFdbFlush(GT_FALSE);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

        tgfBridgeFdbValidAndSkipBitsRun(1);
    }
}


/**
* @internal tgfBridgeFdbValidAndSkipBitsRestore_ForMultiCoreFdbLookup function
* @endinternal
*
* @brief   Restore_ForMultiCoreFdbLookup test configuration
*/
GT_VOID tgfBridgeFdbValidAndSkipBitsRestore_ForMultiCoreFdbLookup
(
    GT_VOID
)
{
    tgfBridgeFdbRestore_ForMultiCoreFdbLookup();
}

/**
* @internal tgfBridgeFdbDebugBurstNumSet function
* @endinternal
*
* @brief   debug function to run test of learn 128K/64K with smaller burst -->
*         less number of entries to the FDB.
*         to to allow fast debug of error in test and not wait till end of 128K/64K
*         learn...
*/
GT_STATUS tgfBridgeFdbDebugBurstNumSet
(
    IN GT_U32   burstNum
)
{
    debug_burstNum = burstNum;
    return GT_OK;
}

/**
* @internal tgfBridgeFdbDeviceMapTableModeSet_ForMultiCoreFdbLookup function
* @endinternal
*
* @brief   Set lookup mode for accessing the Device Map table.
*         and call to adjust the device map table.
* @param[in] mode                     - device Map lookup mode
*                                       none
*/
GT_VOID tgfBridgeFdbDeviceMapTableModeSet_ForMultiCoreFdbLookup
(
    IN PRV_TGF_DEV_MAP_LOOKUP_MODE_ENT   mode
)
{
    GT_STATUS   rc;

    PRV_UTF_LOG0_MAC("======= tgfBridgeFdbDeviceMapTableModeSet_ForMultiCoreFdbLookup : set new mode in device map table =======\n");
    rc = prvTgfCscdDevMapLookupModeSet(mode);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
#ifndef EXMXPM_FAMILY
    PRV_UTF_LOG0_MAC("======= : re-config trunk src port hash and the device map table: appDemoTrunkSrcPortHashMappingSet_ForMultiCoreFdbLookup =======\n");
    rc = appDemoLionTrunkSrcPortHashMappingSet_ForMultiCoreFdbLookup(0);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
#endif/*EXMXPM_FAMILY*/

    PRV_UTF_LOG0_MAC("======= tgfBridgeFdbDeviceMapTableModeSet_ForMultiCoreFdbLookup : ended =======\n");
    return;
}

/**
* @internal tgfBridgeFdbDeviceMapTableModeRestore_ForMultiCoreFdbLookup function
* @endinternal
*
* @brief   Restore_ForMultiCoreFdbLookup test configuration
*/
GT_VOID tgfBridgeFdbDeviceMapTableModeRestore_ForMultiCoreFdbLookup
(
    GT_VOID
)
{
    /* restore default value of table mode */
    tgfBridgeFdbDeviceMapTableModeSet_ForMultiCoreFdbLookup(PRV_TGF_DEV_MAP_LOOKUP_MODE_TRG_DEV_E);

    tgfBridgeFdbRestore_ForMultiCoreFdbLookup();
}


