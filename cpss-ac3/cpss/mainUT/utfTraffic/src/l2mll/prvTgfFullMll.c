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
* @file prvTgfFullMll.c
*
* @brief L2 MLL Full MLL table test
*
* @version   10
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/generic/cpssCommonDefs.h>
#include <cpss/generic/bridge/cpssGenBrgVlanTypes.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfL2MllGen.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfIpGen.h>
#include <common/tgfPortGen.h>


#include <l2mll/prvTgfL2MllUtils.h>
#include <l2mll/prvTgfFullMll.h>

/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/
/* default VLAN Id */
#define PRV_TGF_VLANID_CNS  1

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS     2

/* default number of packets to send */
static GT_U32  prvTgfBurstCount   = 1;

/* default value for tail-dropping for all packets based on the profile limits */
static GT_BOOL defaultPortTxTailDropUcEnable = GT_FALSE;
/* default Token Bucket rate shaping. */
static GT_BOOL defaultMultiTargetShaperEnable;
/* default Token Bucket Baseline */
static GT_U32  defaultMultiTargetShaperBaseline;
/* default Max Bucket Size */
static GT_U32  defaultMultiTargetMaxBucketSize;
/* default Token Bucket Mode*/
static CPSS_PORT_TX_DROP_SHAPER_MODE_ENT  defaultMultiTargetTokenBucketMode;
/* default Max Rate  */
static GT_U32  defaultMultiTargetMaxRate;


/******************************* Test packet **********************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part = {
    {0x01, 0x00, 0x00, 0x00, 0x00, 0x33},               /* dstMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x99}                /* srcMac */
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
    0x00, 0x00, 0x00, 0x00
};
/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart = {
    sizeof(prvTgfPayloadDataArr),                       /* dataLength */
    prvTgfPayloadDataArr                                /* dataPtr */
};

/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfPacketPartArray[] = {
    {TGF_PACKET_PART_L2_E,       &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E, &prvTgfPacketVlanTag0Part},
    {TGF_PACKET_PART_PAYLOAD_E,  &prvTgfPacketPayloadPart}
};

/* LENGTH of packet */
#define PRV_TGF_PACKET_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + sizeof(prvTgfPayloadDataArr)

/* LENGTH of packet with CRC */
#define PRV_TGF_PACKET_CRC_LEN_CNS  PRV_TGF_PACKET_LEN_CNS + TGF_CRC_LEN_CNS

/* PACKET to send */
static TGF_PACKET_STC prvTgfPacketInfo = {
    PRV_TGF_PACKET_LEN_CNS,                                      /* totalLen */
    sizeof(prvTgfPacketPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketPartArray                                        /* partsArray */
};

/******************************************************************************\
 *                            Private test functions                          *
\******************************************************************************/


/**
* @internal prvTgfFullMllConfigurationSet function
* @endinternal
*
* @brief   Set configuration.
*/
GT_VOID prvTgfFullMllConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS rc;
    PRV_TGF_L2_MLL_LTT_ENTRY_STC         lttEntry;
    PRV_TGF_PAIR_READ_WRITE_FORM_ENT     mllPairWriteForm;
    PRV_TGF_L2_MLL_PAIR_STC              mllPairEntry;
    GT_U32                         mllsNumber;
    GT_U32                         mllsNumOfChunks;/* each chunck consist maximum 1000 mlls pairs */
    GT_U32                         lastChunkSize;/* the size of the last checnk migth be smaller then 1000 */
    GT_U32                         i=0;
    GT_U32                         j=0;

    /* AUTODOC: SETUP CONFIGURATION: */

    /* Save L2 MLL configurations */
    prvTgfL2MllUtilsGlobalConfigurationGetAndSave();

    /* AUTODOC: disable MLL lookup for all multi-target packets */
    rc = prvTgfL2MllLookupForAllEvidxEnableSet(prvTgfDevNum, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfL2MllLookupForAllEvidxEnableSet: enable %d",
                                 GT_FALSE);

    /* AUTODOC: add FDB entry with MAC 01:00:00:00:00:33, VLAN 1, eVIDX 4096 */
    rc = prvTgfBrgDefFdbMacEntryOnVidxSet(prvTgfPacketL2Part.daMac, PRV_TGF_VLANID_CNS,
                                          4097, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgDefFdbMacEntryOnVidxSet: eVidx 4096");

    /* create L2 MLL LTT entries */
    cpssOsMemSet(&lttEntry, 0, sizeof(PRV_TGF_L2_MLL_LTT_ENTRY_STC));

    lttEntry.mllPointer = 0;
    lttEntry.entrySelector = 0;

    /* AUTODOC: add L2 MLL LTT entry 1 with: */
    /* AUTODOC:   mllPointer=0, mllMaskProfileEnable=False, mllMaskProfile=0 */
    rc = prvTgfL2MllLttEntrySet(prvTgfDevNum, 1, &lttEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfL2MllLttEntrySet: index 1");

    /* get number of MLLs */
    mllsNumber = prvTgfL2MllNumberOfMllsGet();
    mllsNumOfChunks = mllsNumber/1000;
    lastChunkSize = mllsNumber%1000;
    mllsNumOfChunks += ((lastChunkSize != 0) ? 1 : 0);

    /* create L2 MLL entries */
    cpssOsMemSet(&mllPairEntry, 0, sizeof(PRV_TGF_L2_MLL_PAIR_STC));

    mllPairWriteForm=PRV_TGF_PAIR_READ_WRITE_WHOLE_E;

    for(j = 0 ; j < mllsNumOfChunks ; j++)
    {

        /* create a check of 1000 mll pairs */
        for(i=0; i < 999 ; i++)
        {
            /* last mll chunk can have less then 1000 MLL pairs */
            if((j==(mllsNumOfChunks-1))&&
               (lastChunkSize!=0) &&
               (i==(lastChunkSize-1)))
                break;

            mllPairEntry.nextPointer = (i+1)+(j*1000);
            mllPairEntry.entrySelector = PRV_TGF_L2_MLL_PAIR_ENTRY_SELECTOR_FIRST_MLL_E;

            mllPairEntry.firstMllNode.last = GT_FALSE;
            mllPairEntry.firstMllNode.egressInterface.type = CPSS_INTERFACE_PORT_E;
            mllPairEntry.firstMllNode.egressInterface.devPort.hwDevNum = prvTgfDevNum;
            mllPairEntry.firstMllNode.egressInterface.devPort.portNum = prvTgfPortsArray[1+2*(i%2)];

            mllPairEntry.secondMllNode.last = GT_FALSE;
            mllPairEntry.secondMllNode.egressInterface.type = CPSS_INTERFACE_PORT_E;
            mllPairEntry.secondMllNode.egressInterface.devPort.hwDevNum = prvTgfDevNum;
            mllPairEntry.secondMllNode.egressInterface.devPort.portNum = prvTgfPortsArray[1+2*((i+1)%2)];

            /* AUTODOC: add all possible L2 MLL entries with: */
            /* AUTODOC:   index=mllPtr */
            /* AUTODOC:   nextPointer=mllPtr+1, ports=[1,3,1,3,...] */
            rc = prvTgfL2MllPairWrite(prvTgfDevNum, (i+(j*1000)), mllPairWriteForm, &mllPairEntry);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfL2MllPairWrite: index %d",i);
        }

        mllPairEntry.nextPointer = 0;
        mllPairEntry.entrySelector = PRV_TGF_L2_MLL_PAIR_ENTRY_SELECTOR_FIRST_MLL_E;

        mllPairEntry.firstMllNode.last = GT_FALSE;
        mllPairEntry.firstMllNode.egressInterface.type = CPSS_INTERFACE_PORT_E;
        mllPairEntry.firstMllNode.egressInterface.devPort.hwDevNum = prvTgfDevNum;
        mllPairEntry.firstMllNode.egressInterface.devPort.portNum = prvTgfPortsArray[1+2*(i%2)];

        mllPairEntry.secondMllNode.last = GT_TRUE;
        mllPairEntry.secondMllNode.egressInterface.type = CPSS_INTERFACE_PORT_E;
        mllPairEntry.secondMllNode.egressInterface.devPort.hwDevNum = prvTgfDevNum;
        mllPairEntry.secondMllNode.egressInterface.devPort.portNum = prvTgfPortsArray[1+2*((i+1)%2)];

        /* AUTODOC: add last L2 MLL entry: */
        /* AUTODOC:   nextPointer=0, port=[1 or 3] */
        rc = prvTgfL2MllPairWrite(prvTgfDevNum, (i+(j*1000)), mllPairWriteForm, &mllPairEntry);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfL2MllPairWrite: index %d",i);
    }

    if (PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum) == GT_FALSE) CPSS_BOOKMARK_TBD_FALCON_TXQ
    {
        /* AUTODOC: keep default tail drop value for restore */
        rc = prvTgfPortTxTailDropUcEnableGet(prvTgfDevNum,&defaultPortTxTailDropUcEnable);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortTxTailDropUcEnableGet");
    }

    /* AUTODOC: disable tail drop */
    rc = prvTgfPortTxTailDropUcEnableSet(prvTgfDevNum,GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortTxTailDropUcEnableSet");

}

/**
* @internal prvTgfFullMllTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic.
*/
GT_VOID prvTgfFullMllTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS rc;
    GT_U32    portIter;
    CPSS_PORT_MAC_COUNTER_SET_STC portCntrs;
    GT_U32    expectedTxPacket;
    GT_U32    mllsNumber;
    GT_U32    mllsNumOfChunks;/* each chunck consist maximum 1000 mlls pairs */
    GT_U32    lastChunkSize;/* the size of the last checnk migth be smaller then 1000 */
    GT_U32    j=0;
    PRV_TGF_L2_MLL_LTT_ENTRY_STC         lttEntry;

    /* get number of MLLs pairs */
    mllsNumber = prvTgfL2MllNumberOfMllsGet();
    mllsNumOfChunks = mllsNumber/1000;
    lastChunkSize = mllsNumber%1000;
    mllsNumOfChunks += ((lastChunkSize != 0) ? 1 : 0);

    for(j = 0 ; j < mllsNumOfChunks ; j++)
    {
        /* create L2 MLL LTT entries */
        cpssOsMemSet(&lttEntry, 0, sizeof(PRV_TGF_L2_MLL_LTT_ENTRY_STC));

        lttEntry.mllPointer = (1000*j);
        lttEntry.entrySelector = 0;

        /* AUTODOC: add L2 MLL LTT entry 1 with: */
        /* AUTODOC:   mllPointer=0, mllMaskProfileEnable=False, mllMaskProfile=0 */
        rc = prvTgfL2MllLttEntrySet(prvTgfDevNum, 1, &lttEntry);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfL2MllLttEntrySet: index 1");

        /* AUTODOC: GENERATE TRAFFIC: */

        /* reset counters */
        prvTgfL2MllUtilsResetAllEthernetCounters();

        /* AUTODOC: send Ethernet packet from port 2 with: */
        /* AUTODOC:   DA=01:00:00:00:00:33, SA=00:00:00:00:00:99, VID=1 */
        prvTgfL2MllUtilsPacketSend(&prvTgfPacketInfo, prvTgfBurstCount, PRV_TGF_SEND_PORT_IDX_CNS);

    #ifdef ASIC_SIMULATION
        /* allow at least one packet to egress */
        cpssOsTimerWkAfter(100);
    #endif


        /* AUTODOC: verify received Ethernet packets: */
        /* AUTODOC:   ports [2] - 1 packet */
        /* AUTODOC:   ports [1,3] - each received mllsNumber packets */
        for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
        {
            /* read counters */
            rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                         prvTgfDevNum, prvTgfPortsArray[portIter]);

            /* check Rx counters */
            switch(portIter)
            {
                case 0:expectedTxPacket = 0;
                        break;
                case 1:expectedTxPacket = (j==(mllsNumOfChunks-1)) ? lastChunkSize : 1000;
                        break;
                case 2: expectedTxPacket = 1;
                        break;
                case 3: expectedTxPacket = (j==(mllsNumOfChunks-1)) ? lastChunkSize : 1000;;
                        /* do not use value from the counter as we already accumulated the counter */
                        /*portCntrs.goodPktsSent.l[0] = tempNumOfPackets;*/
                        break;
                default: expectedTxPacket = 0;
                        break;
            }

            UTF_VERIFY_EQUAL2_STRING_MAC(expectedTxPacket, portCntrs.goodPktsSent.l[0],
                                         "get another Tx packets than expected on port[%d] = %d",
                                         portIter, prvTgfPortsArray[portIter]);
        }

        /* reset counters */
        prvTgfL2MllUtilsResetAllEthernetCounters();
    }
}

/**
* @internal prvTgfFullMllConfigurationRestore function
* @endinternal
*
* @brief   Restore configuration.
*/
GT_VOID prvTgfFullMllConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS rc;
    PRV_TGF_L2_MLL_LTT_ENTRY_STC         lttEntry;
    PRV_TGF_PAIR_READ_WRITE_FORM_ENT     mllPairWriteForm;
    PRV_TGF_L2_MLL_PAIR_STC              mllPairEntry;
    GT_U32    mllsNumber;
    GT_U32    i;

    /* AUTODOC: RESTORE CONFIGURATION: */

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d", prvTgfDevNum, GT_TRUE);

    /* get number of MLLs */
    mllsNumber = prvTgfL2MllNumberOfMllsGet();

    /* clear L2 MLL LTT entry */
    cpssOsMemSet(&lttEntry, 0, sizeof(PRV_TGF_L2_MLL_LTT_ENTRY_STC));

    /* AUTODOC: clear L2 MLL and LTT entries */

    /* clear L2 MLL LTT entry */
    rc = prvTgfL2MllLttEntrySet(prvTgfDevNum, 1, &lttEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfL2MllLttEntrySet: index 1");

    /* clear L2 MLL entries */
    cpssOsMemSet(&mllPairEntry, 0, sizeof(PRV_TGF_L2_MLL_PAIR_STC));

    mllPairWriteForm = PRV_TGF_PAIR_READ_WRITE_WHOLE_E;

    mllPairEntry.firstMllNode.last = GT_TRUE;

    i = 0;
    rc = prvTgfL2MllPairWrite(prvTgfDevNum, i, mllPairWriteForm, &mllPairEntry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfL2MllPairWrite: index %d", i);

    mllPairEntry.firstMllNode.last = GT_FALSE;

    for ( i = 1 ; i < mllsNumber ; i++)
    {
        rc = prvTgfL2MllPairWrite(prvTgfDevNum, i, mllPairWriteForm, &mllPairEntry);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfL2MllPairWrite: index %d", i);
    }

    /* allow traffic to 'die' in case that device not finished processing all
       packets , by the time we 'restore' to default */
    cpssOsTimerWkAfter(1000);

    i = 0;
    /* at this point that we assume that traffic is dead , reset also index 0 */
    rc = prvTgfL2MllPairWrite(prvTgfDevNum, i, mllPairWriteForm, &mllPairEntry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfL2MllPairWrite: index %d", i);

    /* AUTODOC: restore default L2 MLL configurations */
    prvTgfL2MllUtilsGlobalConfigurationRestore();

    /* AUTODOC: restore tail-dropping for all packets based on the profile limits */
    rc = prvTgfPortTxTailDropUcEnableSet(prvTgfDevNum,defaultPortTxTailDropUcEnable);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortTxTailDropUcEnableSet");
}


/**
* @internal prvTgfFullMllOneListConfigurationSet function
* @endinternal
*
* @brief   Set configuration.
*/
GT_VOID prvTgfFullMllOneListConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS rc;
    PRV_TGF_L2_MLL_LTT_ENTRY_STC         lttEntry;
    PRV_TGF_PAIR_READ_WRITE_FORM_ENT     mllPairWriteForm;
    PRV_TGF_L2_MLL_PAIR_STC              mllPairEntry;
    GT_U32                               mllsNumber;
    GT_U32                               i;
    GT_U32                               maxRate;

    /* AUTODOC: SETUP CONFIGURATION: */

    /* Save L2 MLL configurations */
    prvTgfL2MllUtilsGlobalConfigurationGetAndSave();

    /* AUTODOC: disable MLL lookup for all multi-target packets */
    rc = prvTgfL2MllLookupForAllEvidxEnableSet(prvTgfDevNum, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfL2MllLookupForAllEvidxEnableSet: enable %d",
                                 GT_FALSE);

    /* AUTODOC: add FDB entry with MAC 01:00:00:00:00:33, VLAN 1, eVIDX 4096 */
    rc = prvTgfBrgDefFdbMacEntryOnVidxSet(prvTgfPacketL2Part.daMac, PRV_TGF_VLANID_CNS,
                                          4097, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgDefFdbMacEntryOnVidxSet: eVidx 4096");

    /* create L2 MLL LTT entries */
    cpssOsMemSet(&lttEntry, 0, sizeof(PRV_TGF_L2_MLL_LTT_ENTRY_STC));

    lttEntry.mllPointer = 0;
    lttEntry.entrySelector = 0;

    /* AUTODOC: add L2 MLL LTT entry 1 with: */
    /* AUTODOC:   mllPointer=0, mllMaskProfileEnable=False, mllMaskProfile=0 */
    rc = prvTgfL2MllLttEntrySet(prvTgfDevNum, 1, &lttEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfL2MllLttEntrySet: index 1");

    /* get number of MLLs */
    mllsNumber = prvTgfL2MllNumberOfMllsGet();

    /* create L2 MLL entries */
    cpssOsMemSet(&mllPairEntry, 0, sizeof(PRV_TGF_L2_MLL_PAIR_STC));

    mllPairWriteForm=PRV_TGF_PAIR_READ_WRITE_WHOLE_E;

    for(i = 0 ; i < mllsNumber - 1 ; i++ )
    {
        mllPairEntry.nextPointer = i+1;
        mllPairEntry.entrySelector = PRV_TGF_L2_MLL_PAIR_ENTRY_SELECTOR_FIRST_MLL_E;

        mllPairEntry.firstMllNode.last = GT_FALSE;
        mllPairEntry.firstMllNode.egressInterface.type = CPSS_INTERFACE_PORT_E;
        mllPairEntry.firstMllNode.egressInterface.devPort.hwDevNum = prvTgfDevNum;
        mllPairEntry.firstMllNode.egressInterface.devPort.portNum = prvTgfPortsArray[1+2*(i%2)];

        mllPairEntry.secondMllNode.last = GT_FALSE;
        mllPairEntry.secondMllNode.egressInterface.type = CPSS_INTERFACE_PORT_E;
        mllPairEntry.secondMllNode.egressInterface.devPort.hwDevNum = prvTgfDevNum;
        mllPairEntry.secondMllNode.egressInterface.devPort.portNum = prvTgfPortsArray[1+2*((i+1)%2)];

        /* AUTODOC: add all possible L2 MLL entries with: */
        /* AUTODOC:   index=mllPtr */
        /* AUTODOC:   nextPointer=mllPtr+1, ports=[1,3,1,3,...] */
        rc = prvTgfL2MllPairWrite(prvTgfDevNum, i, mllPairWriteForm, &mllPairEntry);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfL2MllPairWrite: index %d",i);
    }

    mllPairEntry.nextPointer = 0;
    mllPairEntry.entrySelector = PRV_TGF_L2_MLL_PAIR_ENTRY_SELECTOR_FIRST_MLL_E;

    mllPairEntry.firstMllNode.last = GT_FALSE;
    mllPairEntry.firstMllNode.egressInterface.type = CPSS_INTERFACE_PORT_E;
    mllPairEntry.firstMllNode.egressInterface.devPort.hwDevNum = prvTgfDevNum;
    mllPairEntry.firstMllNode.egressInterface.devPort.portNum = prvTgfPortsArray[1+2*(i%2)];

    mllPairEntry.secondMllNode.last = GT_TRUE;
    mllPairEntry.secondMllNode.egressInterface.type = CPSS_INTERFACE_PORT_E;
    mllPairEntry.secondMllNode.egressInterface.devPort.hwDevNum = prvTgfDevNum;
    mllPairEntry.secondMllNode.egressInterface.devPort.portNum = prvTgfPortsArray[1+2*((i+1)%2)];

    /* AUTODOC: add last L2 MLL entry: */
    /* AUTODOC:   nextPointer=0, port=[1 or 3] */
    rc = prvTgfL2MllPairWrite(prvTgfDevNum, i, mllPairWriteForm, &mllPairEntry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfL2MllPairWrite: index %d",i);


    /* AUTODOC: IP MLL Shaper configuration */

    /* AUTODOC: keep default tail drop value for restore */
    rc = prvTgfPortTxTailDropUcEnableGet(prvTgfDevNum,&defaultPortTxTailDropUcEnable);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortTxTailDropUcEnableGet");

    /* AUTODOC: disable tail drop */
    rc = prvTgfPortTxTailDropUcEnableSet(prvTgfDevNum,GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortTxTailDropUcEnableSet");

    /* AUTODOC: keep Token Bucket rate shaping value for restore */
    rc = prvTgfIpMllMultiTargetShaperEnableGet(prvTgfDevNum, &defaultMultiTargetShaperEnable);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpMllMultiTargetShaperEnableGet");

    /* AUTODOC: enable Token Bucket rate shaping */
    rc = prvTgfIpMllMultiTargetShaperEnableSet(prvTgfDevNum, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpMllMultiTargetShaperEnableSet");

    /* AUTODOC: keep Baseline value for restore */
    rc = prvTgfIpMllMultiTargetShaperBaselineGet(prvTgfDevNum, &defaultMultiTargetShaperBaseline);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpMllMultiTargetShaperBaselineGet");

    /* AUTODOC: set Baseline value to 0x600 */
    rc = prvTgfIpMllMultiTargetShaperBaselineSet(prvTgfDevNum, 0x600);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpMllMultiTargetShaperBaselineSet");

    /* AUTODOC: keep Token Bucket Mode for restore */
    rc = prvTgfIpMllMultiTargetShaperTokenBucketModeGet(prvTgfDevNum,&defaultMultiTargetTokenBucketMode);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpMllMultiTargetShaperTokenBucketModeGet");

    /* AUTODOC: set Token Bucket Mode to CPSS_PORT_TX_DROP_SHAPER_BYTE_MODE_E */
    rc = prvTgfIpMllMultiTargetShaperTokenBucketModeSet(prvTgfDevNum,CPSS_PORT_TX_DROP_SHAPER_BYTE_MODE_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpMllMultiTargetShaperTokenBucketModeSet");

    /* AUTODOC: keep shaper configuration values for restore */
    rc = prvTgfIpMllMultiTargetShaperConfigurationGet(prvTgfDevNum,
                                                      &defaultMultiTargetMaxBucketSize,
                                                      &defaultMultiTargetMaxRate);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpMllMultiTargetShaperConfigurationGet");


    /* AUTODOC: set shaper configuration values*/
    maxRate = 102000;
    rc = prvTgfIpMllMultiTargetShaperConfigurationSet(prvTgfDevNum,
                                                      1,
                                                      &maxRate);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpMllMultiTargetShaperConfigurationSet");
}

/**
* @internal prvTgfFullMllOneListTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic.
*/
GT_VOID prvTgfFullMllOneListTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS rc;
    GT_U32    portIter;
    CPSS_PORT_MAC_COUNTER_SET_STC portCntrs;
    GT_U32    expectedTxPacket;
    GT_U32    mllsNumber;
    GT_U32    tempNumOfPackets = 0;
#ifdef ASIC_SIMULATION
    /* NOTE: the GM should skip this test ! ... see GM_SKIP_TEST_DUE_TO_POOR_PERFORMANCE_MAC in prvTgfFullMll */
    GT_U32    maxIterator = 30;/* allow 30 seconds to process the 8K MLL entries --> 8K packets */
#else
    GT_U32    maxIterator = 1;/* allow 1 seconds to process the 8K MLL entries --> 8K packets */
#endif

    maxIterator++;

    /* AUTODOC: GENERATE TRAFFIC: */

    /* reset counters */
    prvTgfL2MllUtilsResetAllEthernetCounters();

    /* AUTODOC: send Ethernet packet from port 2 with: */
    /* AUTODOC:   DA=01:00:00:00:00:33, SA=00:00:00:00:00:99, VID=1 */
    prvTgfL2MllUtilsPacketSend(&prvTgfPacketInfo, prvTgfBurstCount, PRV_TGF_SEND_PORT_IDX_CNS);

#ifdef ASIC_SIMULATION
    /* allow at least one packet to egress */
    cpssOsTimerWkAfter(100);
#endif

    /* get number of MLLs pairs */
    mllsNumber = prvTgfL2MllNumberOfMllsGet();

    /* wait to see that portIter[3] got all it's packets  */
    expectedTxPacket = mllsNumber;
    portIter = 3;

    do
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_FALSE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        if(portCntrs.goodPktsSent.l[0] == 0)
        {
            /* not got any packet in the last interval ... so we can assume that we will get not more */
            break;
        }

        tempNumOfPackets += portCntrs.goodPktsSent.l[0];

        if(tempNumOfPackets >= expectedTxPacket)
        {
            /* got (at least) all that we expected */
            break;
        }

        cpssOsTimerWkAfter(1000);
    }while(maxIterator--);

    /* AUTODOC: verify received Ethernet packets: */
    /* AUTODOC:   ports [2] - 1 packet */
    /* AUTODOC:   ports [1,3] - each received mllsNumber packets */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
        /* check Rx counters */
        switch(portIter)
        {
            case 0: expectedTxPacket = 0;
                    break;
            case 1: expectedTxPacket = mllsNumber;
                    break;
            case 2: expectedTxPacket = 1;
                    break;
            case 3: expectedTxPacket = mllsNumber;
                    /* do not use value from the counter as we already accumulated the counter */
                    portCntrs.goodPktsSent.l[0] = tempNumOfPackets;
                    break;
            default: expectedTxPacket = 0;
                    break;
        }
        UTF_VERIFY_EQUAL2_STRING_MAC(expectedTxPacket, portCntrs.goodPktsSent.l[0],
                                     "get another Tx packets than expected on port[%d] = %d",
                                     portIter, prvTgfPortsArray[portIter]);
    }
    /* reset counters */
    prvTgfL2MllUtilsResetAllEthernetCounters();
}

/**
* @internal prvTgfFullMllOneListConfigurationRestore function
* @endinternal
*
* @brief   Restore configuration.
*/
GT_VOID prvTgfFullMllOneListConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS rc = GT_OK;

    prvTgfFullMllConfigurationRestore();

    /* AUTODOC: restore IP MLL Shaper configuration */

    rc = prvTgfPortTxTailDropUcEnableSet(prvTgfDevNum,defaultPortTxTailDropUcEnable);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortTxTailDropUcEnableSet");

    rc = prvTgfIpMllMultiTargetShaperEnableSet(prvTgfDevNum, defaultMultiTargetShaperEnable);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpMllMultiTargetShaperEnableSet");

    rc = prvTgfIpMllMultiTargetShaperBaselineSet(prvTgfDevNum, defaultMultiTargetShaperBaseline);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpMllMultiTargetShaperBaselineSet");

    rc = prvTgfIpMllMultiTargetShaperTokenBucketModeSet(prvTgfDevNum,defaultMultiTargetTokenBucketMode);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpMllMultiTargetShaperTokenBucketModeSet");

    rc = prvTgfIpMllMultiTargetShaperConfigurationSet(prvTgfDevNum,
                                                      defaultMultiTargetMaxBucketSize,
                                                      &defaultMultiTargetMaxRate);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpMllMultiTargetShaperConfigurationSet");
}

