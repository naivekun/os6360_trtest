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
* @file prvTgfAfCncIngressVlanClientIndexMode.c
*
* @brief CPSS DXCH Centralized counters (CNC) Technology facility implementation.
*
* @version   1
********************************************************************************
*/





#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/generic/cpssCommonDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpssCommon/private/prvCpssMath.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <utf/private/prvUtfHelpers.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfPortGen.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfCncGen.h>
#include <common/tgfCosGen.h>
#include <common/tgfAutoFlow.h>
#include <cnc/prvTgfAfCncIngressVlanClientIndexMode.h>


/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

#define PRV_TGF_CNC_BLOCKS_NUM_MAC()        prvTgfCncFineTuningBlockAmountGet()

#define PRV_TGF_CNC_BLOCK_ENTRIES_NUM_MAC() prvTgfCncFineTuningBlockSizeGet()

/* tested block index */
#define PRV_TGF_CNC_TESTED_BLOCK_NUM_MAC() (PRV_TGF_CNC_BLOCKS_NUM_MAC() - 1)


/* index of port in port-array for send port */
#define PRV_TGF_CNC_SEND_PORT_INDEX_CNS        1

/* index of port in port-array for receive (egress) port */
#define PRV_TGF_CNC_RECEIVE_PORT_INDEX_CNS     2

/* VLAN Id for ports */
#define PRV_TGF_VLANID_CNS  4

#define PRV_TGF_VLAN_TAG_0_CNS  5

#define PRV_TGF_VLAN_TAG_1_CNS  6

/* default number of packets to send */
static GT_U32        prvTgfBurstCount =   1;

static PRV_TGF_CNC_VLAN_INDEX_MODE_ENT prvTgfDefIngressVlanIndexMode;

static PRV_TGF_AF_TRAFFIC_UNT             trafficUn;
static PRV_TGF_AF_BRIDGE_UNT              bridgeUn;

static GT_U32  prvTgfPacketSize = 80;

/******************************* Test packet **********************************/

/* First VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTag0Part = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_VLAN_TAG_0_CNS                            /* pri, cfi, VlanId */
};

/* Second VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTag1Part = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_VLAN_TAG_1_CNS                           /* pri, cfi, VlanId */
};

/************************ Double tagged packet ********************************/

/******************************************************************************\
 *                            Implementation                          *
\******************************************************************************/



/******************************************************************************\
 *                            Private test functions                          *
\******************************************************************************/


/**
* @internal prvTgfAfCncIngressVlanBlockConfigure function
* @endinternal
*
* @brief   This function sets configuration of Cnc Block
*
* @param[in] blockNum                 - CNC block number
* @param[in] client                   - CNC client
*                                      valid range see in datasheet of specific device.
* @param[in] enable                   - the client  to update the block
*                                      GT_TRUE - enable, GT_FALSE - disable
*                                      It is forbidden to enable update the same
*                                      block by more then one client. When an
*                                      application enables some client it is responsible
*                                      to disable all other clients it enabled before
* @param[in] indexRangesBmp[]         - the counter index ranges bitmap
*                                      DxCh3 and DxChXcat devices have 8 ranges of counter indexes,
*                                      each 2048 indexes (0..(2K-1), 2K..(4K-1), ..., 14K..(16K-1))
*                                      the n-th bit 1 value maps the n-th index range
*                                      to the block (2048 counters)
*                                      The Lion devices has 64 ranges (512 indexes each).
*                                      Lion2 devices have 32 ranges (512 indexes each).
*                                      Allowed to map more then one range to the block
*                                      but it will cause updating the same counters via
*                                      different indexes.
* @param[in] format                   - CNC counter HW format,
*                                      relevant only for Lion and above
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_INITIALIZED       - trunk library was not initialized
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - some values are out of range
* @retval GT_BAD_PARAM             - on illegal parameters
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_ALREADY_EXIST         - one of the members already exists in another trunk
*/
GT_STATUS prvTgfAfCncIngressVlanBlockConfigure
(
    IN GT_U32                            blockNum,
    IN PRV_TGF_CNC_CLIENT_ENT            client,
    IN GT_BOOL                           enable,
    IN GT_U32                            indexRangesBmp[],
    IN PRV_TGF_CNC_COUNTER_FORMAT_ENT    format
)
{
    GT_STATUS                   rc;
    rc = prvTgfCncBlockClientEnableSet(
        blockNum, client, enable);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfCncBlockClientEnableSet");

    rc = prvTgfCncBlockClientRangesSet(
        blockNum, client, indexRangesBmp);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfCncBlockClientRangesSet");

    rc = prvTgfCncCounterFormatSet(
            blockNum, format);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfCncCounterFormatSet");
    return GT_OK;
}

/**
* @internal prvTgfAfCncIngressVlanSendPacketAndCheck function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port given packet:
*         Check that counter's packet count is equail to burstCount
* @param[in] blockNum                 -  CNC block number
* @param[in] counterIdx               -  index of counter to check
* @param[in] byteCountPattern         -  expecteded byte counter value
*                                      if 0xFFFFFFFF - the check skipped
*                                       None
*/

GT_VOID prvTgfAfCncIngressVlanSendPacketAndCheck
(
    IN GT_U32                         blockNum,
    IN GT_U32                         counterIdx,
    IN GT_U32                         byteCountPattern
)
{
    GT_STATUS                       rc           = GT_OK;
    PRV_TGF_CNC_COUNTER_STC         counter;
    GT_U32          portsCount   = prvTgfPortsNum;
    GT_U32          portIter;
    GT_U16          vlanIdx;
    PRV_UTF_LOG0_MAC("======= Generating Traffic =======\n");

    /* clear counter */
    counter.packetCount.l[0] = 0;
    counter.packetCount.l[1] = 0;
    counter.byteCount.l[0] = 0;
    counter.byteCount.l[1] = 0;

    for(vlanIdx=PRV_TGF_VLANID_CNS;vlanIdx<=PRV_TGF_VLAN_TAG_1_CNS;vlanIdx++)
    {
        rc = prvTgfCncCounterSet(
        blockNum, vlanIdx,
        PRV_TGF_CNC_COUNTER_FORMAT_MODE_0_E, &counter);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG2_MAC(
                "[TGF]: prvTgfCncCounterSet FAILED, counterIdx = %d, rc = [%d]",vlanIdx, rc);
            return;
        }
    }
    /* reset counters and force links UP */
    for (portIter = 0; portIter < portsCount; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* setup Packet */
    tgfAutoFlowTxSetup();
    /* send Packet */
    tgfAutoFlowTxStart();

    for(vlanIdx=PRV_TGF_VLANID_CNS;vlanIdx<=PRV_TGF_VLAN_TAG_1_CNS;vlanIdx++)
    {
        rc = prvTgfCncCounterGet(
        blockNum, vlanIdx,
        PRV_TGF_CNC_COUNTER_FORMAT_MODE_0_E, &counter);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCncCounterGet");
        if(vlanIdx==counterIdx)
        {
            UTF_VERIFY_EQUAL3_STRING_MAC(
                prvTgfBurstCount, counter.packetCount.l[0],
                "counter index %d : packet counter expected: %d received %d",
                vlanIdx,prvTgfBurstCount, counter.packetCount.l[0]);

            if (byteCountPattern != 0xFFFFFFFF)
            {
                UTF_VERIFY_EQUAL2_STRING_MAC(
                    byteCountPattern, counter.byteCount.l[0],
                    "byte counter expected: %d received %d",
                    byteCountPattern, counter.byteCount.l[0]);
            }
        }
        else
        {
             UTF_VERIFY_EQUAL2_STRING_MAC(
                0, counter.packetCount.l[0],
                "counter index %d : packet counter expected: 0 received %d",
                vlanIdx, counter.packetCount.l[0]);
        }


    }
}


/**
* @internal prvTgfAfCncIngressVlanNotZeroCountersDump function
* @endinternal
*
* @brief   This function Dumps all non zero counters
*/
GT_STATUS prvTgfAfCncIngressVlanNotZeroCountersDump
(
    GT_VOID
)
{
    GT_STATUS                   rc;
    GT_U32                      blockIdx;
    GT_U32                      blockAmount;
    GT_U32                      blockSize;
    GT_U32                      counterIdx;
    PRV_TGF_CNC_COUNTER_STC     counter;

    if (GT_TRUE == prvUtfSkipLongTestsFlagGet(UTF_ALL_FAMILY_E) &&
        GT_TRUE == prvUtfIsGmCompilation())
    {
        /* in the GM the looping over 32 blocks over 2K counters of CNC takes in win 32 more then 1 hour !!!
           and on linux server 111 seconds */
        return GT_OK;
    }

    blockAmount = PRV_TGF_CNC_BLOCKS_NUM_MAC();
    blockSize   = PRV_TGF_CNC_BLOCK_ENTRIES_NUM_MAC();

    for (blockIdx = 0; (blockIdx < blockAmount); blockIdx++)
    {
        for (counterIdx = 0; (counterIdx < blockSize); counterIdx ++)
        {
                /* AUTODOC: reset CNC counters for all index and blocks */
            rc = prvTgfCncCounterGet(
                blockIdx, counterIdx,
                PRV_TGF_CNC_COUNTER_FORMAT_MODE_0_E, &counter);
            PRV_UTF_VERIFY_GT_OK(rc, "prvTgfCncCounterGet");

            if ((counter.packetCount.l[0] != 0) || (counter.byteCount.l[0] != 0))
            {
                PRV_UTF_LOG4_MAC(
                    "CncBlock[%d] Index[%d]: packets[%d] bytes[%d]\n",
                    blockIdx, counterIdx, counter.packetCount.l[0], counter.byteCount.l[0]);
            }
        }
    }

    return GT_OK;
}



/******************************************************************************\
 *                           configure,test,reset                          *
\******************************************************************************/
/**
* @internal prvTgfAfCncIngressVlanClientIndexModeConfigure function
* @endinternal
*
* @brief   This function configures Cnc Ingress Vlan Client Index Mode tests
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_INITIALIZED       - trunk library was not initialized
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - some values are out of range
* @retval GT_BAD_PARAM             - on illegal parameters
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_ALREADY_EXIST         - one of the members already exists in another trunk
*/
GT_VOID prvTgfAfCncIngressVlanClientIndexModeConfigure
(
    GT_VOID
)
{
    GT_STATUS                      rc=GT_OK;
    GT_U32                         indexRangesBmp[4];
    PRV_TGF_CNC_COUNTER_STC        counter;

    /* AUTODOC: SETUP CONFIGURATION: */

    /* AUTODOC: create VLAN 4 with untagged ports and pvid set for all ports*/
    bridgeUn.ingressVlan=PRV_TGF_VLANID_CNS;
    tgfAutoFlowBridgePreset(PRV_TGF_AF_BRIDGE_PARAM_INGRESS_VLAN_E,bridgeUn);
    /* AUTODOC: set packets to double tagged */
    trafficUn.vlanTagType=PRV_TGF_AF_VLAN_TAG_TYPE_DOUBLE_TAGGED_E;
    tgfAutoFlowTrafficPreset(PRV_TGF_AF_TRAFFIC_PARAM_VLAN_TAG_TYPE_E,&trafficUn);
    tgfAutoFlowRandSeed(1);
    tgfAutoFlowCreate();

    /* AUTODOC: create VLAN 5 with untagged ports */
    rc = prvTgfBrgDefVlanEntryWrite(PRV_TGF_VLAN_TAG_0_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc, "prvTgfBrgDefVlanEntryWrite");

    /* all ranges relevant to VID map into the block    */
    /* correct both for 512 and for 2048 counter blocks */
    indexRangesBmp[0] = 0xFF;
    indexRangesBmp[1] = 0;
    indexRangesBmp[2] = 0;
    indexRangesBmp[3] = 0;
    counter.byteCount.l[0] = 0;
    counter.byteCount.l[1] = 0;
    counter.packetCount.l[0] = 0;
    counter.packetCount.l[1] = 0;

    /* AUTODOC: bind INGRESS_VLAN Pass/Drop client to counter block */
    /* AUTODOC: for INGRESS_VLAN Pass/Drop client set index range to 0xFF */
    /* AUTODOC: set FORMAT_MODE_0 format of CNC counter */
    rc = prvTgfAfCncIngressVlanBlockConfigure(
        PRV_TGF_CNC_TESTED_BLOCK_NUM_MAC(),
        PRV_TGF_CNC_CLIENT_INGRESS_VLAN_PASS_DROP_E,
        GT_TRUE /*enable*/,
        indexRangesBmp,
        PRV_TGF_CNC_COUNTER_FORMAT_MODE_0_E);
    if (rc != GT_OK)
    {
        return;
    }

    rc = prvTgfCncVlanClientIndexModeGet(PRV_TGF_CNC_CLIENT_INGRESS_VLAN_PASS_DROP_E,
                                         &prvTgfDefIngressVlanIndexMode);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc,"prvTgfCncVlanClientIndexModeGet");

  /* AUTODOC: enable clear by read mode of CNC counters read operation */
    rc = prvTgfCncCounterClearByReadEnableSet(
        GT_TRUE /*enable*/);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc, "prvTgfCncCounterClearByReadEnableSet");

    /* AUTODOC: for FORMAT_MODE_0 set counter clear value 0 */
    rc = prvTgfCncCounterClearByReadValueSet(
        PRV_TGF_CNC_COUNTER_FORMAT_MODE_0_E, &counter);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc, "prvTgfCncCounterClearByReadValueSet");




    /* AUTODOC: for INGRESS_VLAN Pass/Drop CNC client set BYTE_COUNT_MODE_L2 mode */
    rc = prvTgfCncClientByteCountModeSet(
        PRV_TGF_CNC_CLIENT_INGRESS_VLAN_PASS_DROP_E,
        PRV_TGF_CNC_BYTE_COUNT_MODE_L2_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc, "prvTgfCncClientByteCountModeSet");

}

/**
* @internal prvTgfAfCncIngressVlanClientIndexModeTest function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port given packet:
*         Check counters.
*/
GT_VOID prvTgfAfCncIngressVlanClientIndexModeTest
(
    GT_VOID
)
{
    GT_STATUS                       rc           = GT_OK;
    /* AUTODOC: GENERATE TRAFFIC: */
    /* AUTODOC: set vlan tags packet size and burst counts for packets and ingress port idx for bridge */
    trafficUn.vlanTag0=prvTgfPacketVlanTag0Part;
    tgfAutoFlowTrafficPreset(PRV_TGF_AF_TRAFFIC_PARAM_VLAN_TAG_0_E,&trafficUn);
    trafficUn.vlanTag1=prvTgfPacketVlanTag1Part;
    tgfAutoFlowTrafficPreset(PRV_TGF_AF_TRAFFIC_PARAM_VLAN_TAG_1_E,&trafficUn);
    trafficUn.burstCount=prvTgfBurstCount;
    tgfAutoFlowTrafficPreset(PRV_TGF_AF_TRAFFIC_PARAM_BURST_COUNT_E,&trafficUn);
    trafficUn.packetSize=prvTgfPacketSize;
    tgfAutoFlowTrafficPreset(PRV_TGF_AF_TRAFFIC_PARAM_PACKET_SIZE_E,&trafficUn);
    bridgeUn.ingressPortIdx=PRV_TGF_CNC_SEND_PORT_INDEX_CNS;
    tgfAutoFlowBridgePreset(PRV_TGF_AF_BRIDGE_PARAM_INGRESS_PORT_IDX_E,bridgeUn);


    /*AUTODOC: set ingress VLAN client index mode to ORIGINAL_VID*/
    rc = prvTgfCncVlanClientIndexModeSet(PRV_TGF_CNC_CLIENT_INGRESS_VLAN_PASS_DROP_E,
                                         PRV_TGF_CNC_VLAN_INDEX_MODE_ORIGINAL_VID_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc, "prvTgfCncVlanClientIndexModeSet");


    /* AUTODOC: send double tagged packet from port 1 with: */
    prvTgfAfCncIngressVlanSendPacketAndCheck(
        PRV_TGF_CNC_TESTED_BLOCK_NUM_MAC(),
        PRV_TGF_VLAN_TAG_0_CNS,
        /* counting all packet bytes with CRC */
        ((prvTgfPacketSize + 4) * prvTgfBurstCount));
    /* AUTODOC: verify CNC counters: */
    /* AUTODOC:   byteCount = 64 */
    /* AUTODOC:   packetCount = 1 */
   rc = prvTgfAfCncIngressVlanNotZeroCountersDump();
   UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc, "prvTgfCncIngressVlanNotZeroCountersDump");


   /*AUTODOC: set ingress VLAN client index mode to EVID, and enable ForcePvid on port 1*/
    rc = prvTgfCncVlanClientIndexModeSet(PRV_TGF_CNC_CLIENT_INGRESS_VLAN_PASS_DROP_E,
                                                 PRV_TGF_CNC_VLAN_INDEX_MODE_EVID_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc, "prvTgfCncVlanClientIndexModeSet");

    rc = prvTgfBrgVlanPortForcePvidEnable(prvTgfDevNum,
        prvTgfPortsArray[PRV_TGF_CNC_SEND_PORT_INDEX_CNS],GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc, "prvTgfBrgVlanPortForcePvidEnable");

    /* AUTODOC: send double tagged packet from port 1 with: */
    prvTgfAfCncIngressVlanSendPacketAndCheck(
        PRV_TGF_CNC_TESTED_BLOCK_NUM_MAC(),
        PRV_TGF_VLANID_CNS,
        /* counting all packet bytes with CRC */
        ((prvTgfPacketSize + 4) * prvTgfBurstCount));
    /* AUTODOC: verify CNC counters: */
    /* AUTODOC:   byteCount = 64 */
    /* AUTODOC:   packetCount = 1 */
   rc = prvTgfAfCncIngressVlanNotZeroCountersDump();
   UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc, "prvTgfCncIngressVlanNotZeroCountersDump");

   /*AUTODOC: set ingress VLAN client index mode to TAG1_VID*/
    rc = prvTgfCncVlanClientIndexModeSet(PRV_TGF_CNC_CLIENT_INGRESS_VLAN_PASS_DROP_E,
                                         PRV_TGF_CNC_VLAN_INDEX_MODE_TAG1_VID_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc, "prvTgfCncVlanClientIndexModeGet");


    /* AUTODOC: send double tagged packet from port 1 with: */
    prvTgfAfCncIngressVlanSendPacketAndCheck(
        PRV_TGF_CNC_TESTED_BLOCK_NUM_MAC(),
        PRV_TGF_VLAN_TAG_1_CNS,
        /* counting all packet bytes with CRC */
        ((prvTgfPacketSize + 4) * prvTgfBurstCount));
    /* AUTODOC: verify CNC counters: */
    /* AUTODOC:   byteCount =  64*/
    /* AUTODOC:   packetCount = 1 */
   rc = prvTgfAfCncIngressVlanNotZeroCountersDump();
   UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc, "prvTgfCncIngressVlanNotZeroCountersDump");
}

/**
* @internal prvTgfAfCncIngressVlanClientIndexModeReset function
* @endinternal
*
* @brief   reset all configurations
*/
GT_VOID prvTgfAfCncIngressVlanClientIndexModeReset
(
    GT_VOID
)
{
    GT_STATUS                   rc;
    GT_U32                      blockIdx;
    GT_U32                      blockAmount;
    GT_U32                      blockSize;
    PRV_TGF_CNC_CLIENT_ENT      client=PRV_TGF_CNC_CLIENT_INGRESS_VLAN_PASS_DROP_E;
    GT_U32                      clientRange[4];
    GT_U32                      counterIdx;
    PRV_TGF_CNC_COUNTER_STC     counter;

    /* AUTODOC: RESTORE CONFIGURATION: */

   /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc, "prvTgfBrgFdbFlush");

    /* AUTODOC: restore default vlanId to all ports */
    rc = prvTgfBrgVlanPortForcePvidEnable(prvTgfDevNum,
                                          prvTgfPortsArray[PRV_TGF_CNC_SEND_PORT_INDEX_CNS],GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc, "prvTgfBrgVlanPortForcePvidEnable");

    tgfAutoFlowDel();
    /* invalidate manualy set vlan entry (and reset vlan entry) */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLAN_TAG_0_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK,rc, "prvTgfBrgDefVlanEntryInvalidate: %d",PRV_TGF_VLAN_TAG_0_CNS);

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc, "tgfTrafficTableRxPcktTblClear");

    rc = prvTgfCncVlanClientIndexModeSet(PRV_TGF_CNC_CLIENT_INGRESS_VLAN_PASS_DROP_E,
                                         prvTgfDefIngressVlanIndexMode);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc, "prvTgfCncVlanClientIndexModeSet");

    blockAmount = PRV_TGF_CNC_BLOCKS_NUM_MAC();
    blockSize   = PRV_TGF_CNC_BLOCK_ENTRIES_NUM_MAC();
    clientRange[0] = 0;
    clientRange[1] = 0;
    clientRange[2] = 0;
    clientRange[3] = 0;
    counter.byteCount.l[0] = 0;
    counter.byteCount.l[1] = 0;
    counter.packetCount.l[0] = 0;
    counter.packetCount.l[1] = 0;

    for (blockIdx = 0; (blockIdx < blockAmount); blockIdx++)
    {
            /* AUTODOC: unbind client from all counter blocks */
            rc = prvTgfCncBlockClientEnableSet(
                blockIdx, client, GT_FALSE);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc, "prvTgfCncBlockClientEnableSet");

            /* AUTODOC: reset index ranges for all CNC clients and blocks */
            rc = prvTgfCncBlockClientRangesSet(
                blockIdx, client, clientRange);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc, "prvTgfCncBlockClientRangesSet");
    }

    /* clear the block used in test */
    blockIdx = PRV_TGF_CNC_TESTED_BLOCK_NUM_MAC();
    for (counterIdx = 0; (counterIdx < blockSize); counterIdx ++)
    {
        /* AUTODOC: reset CNC counters for all index and blocks */
        rc = prvTgfCncCounterSet(
            blockIdx, counterIdx,
            PRV_TGF_CNC_COUNTER_FORMAT_MODE_0_E, &counter);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc, "prvTgfCncCounterSet");
    }

}





