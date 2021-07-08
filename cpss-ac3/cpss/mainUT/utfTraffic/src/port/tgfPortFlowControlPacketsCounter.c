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
* @file tgfPortFlowControlPacketsCounter.c
*
* @brief Flow control packets counter check
*
* @version   2
********************************************************************************
*/

#include <utf/utfMain.h>
#include <common/tgfCommon.h>
#include <common/tgfPortGen.h>
#include <utf/private/prvUtfExtras.h>

/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

/* number of ports to send traffic to */
#define PRV_TGF_NUM_OF_PORTS_TO_SEND     4

/* default number of packets to send */
static GT_U32 prvTgfBurstCount = 2;


/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part =
{
    {0x01, 0x80, 0xC2, 0x00, 0x00, 0x01},    /* Flow Control daMac */
    {0x00, 0x00, 0x00, 0x00, 0xAA, 0x18}     /* saMac */
};

/* packet's ethertype */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketEtherTypePart =
{
    TGF_ETHERTYPE_8808_FC_TAG_CNS
};

/* IEEE 802.3bd/802.1Qbb PFC opcode */
/* opcode packet part should be proceed like ethertype*/
/* no need for additional UTF structures, funcs etc.*/
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketFCOpcodePart =
{
    TGF_PFC_OPCODE_0101_TAG_CNS
};

/* DATA of packet */
static GT_U8 prvTgfPayloadDataArr[] = {
    0x00, 0x01, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF,
};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart = {
    sizeof(prvTgfPayloadDataArr),                       /* dataLength */
    prvTgfPayloadDataArr                                /* dataPtr */
};

/* PARTS of FC packet */
static TGF_PACKET_PART_STC prvTgfPacketFCPartArray[] =
{
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherTypePart},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketFCOpcodePart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* LENGTH of FC packet */
#define PRV_TGF_PACKET_FC_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS*2/*opCode*/ \
                + sizeof(prvTgfPayloadDataArr)

/* FC PACKET to send */
static TGF_PACKET_STC prvTgfPacketFC =
{
    PRV_TGF_PACKET_FC_LEN_CNS,                                     /* totalLen */
    sizeof(prvTgfPacketFCPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketFCPartArray                                        /* partsArray */
};


/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/**
* @internal prvTgfPortFCTrafficGenerate function
* @endinternal
*
*/
GT_VOID prvTgfPortFCTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS rc = GT_OK;
    GT_U32    portIndex;

    PRV_UTF_LOG0_MAC("======= Sending FC packet =======\n");

    /* setup Packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfPacketFC, prvTgfBurstCount, 0, NULL);
    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d, %d",
                                 prvTgfDevNum, prvTgfBurstCount, 0, 0);

    
    /* transmit packet */
    for(portIndex = 0; portIndex < PRV_TGF_NUM_OF_PORTS_TO_SEND; portIndex++)
    {
        rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[portIndex]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIndex]);
    }
}

/**
* @internal prvTgfPortFCCounterCheck function
* @endinternal
*
*/
GT_VOID prvTgfPortFCCounterCheck
(
    GT_VOID
)
{
    GT_STATUS st;
    GT_U32    receivedCntr;
    GT_U32    droppedCntr; 

    st = prvTgfPortFCPacketsCntrsGet(&receivedCntr, &droppedCntr);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, 
            "ERROR of prvTgfPortFCPacketsCntrsGet: %d", st);

    if(GT_OK == st)
    {
        UTF_VERIFY_EQUAL2_STRING_MAC(prvTgfBurstCount * PRV_TGF_NUM_OF_PORTS_TO_SEND, 
                                     receivedCntr, 
                "Got wrong received counter value, expected [%d], got [%d]", 
                                     prvTgfBurstCount * PRV_TGF_NUM_OF_PORTS_TO_SEND, 
                                     receivedCntr);

#ifndef ASIC_SIMULATION
        /* dropped counter is not supported by simulation */
        UTF_VERIFY_EQUAL1_STRING_MAC(droppedCntr, 0, 
                "Got wrong dropped counter value, expected [0], got [%d]", droppedCntr);
#endif/* ASIC_SIMULATION */

    }

}




