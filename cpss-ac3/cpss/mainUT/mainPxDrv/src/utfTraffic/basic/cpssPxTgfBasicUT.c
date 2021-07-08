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
* @file cpssPxTgfBasicUT.c
*
* @brief basic tests with traffic for the Pipe devices.
*
* @version   1
********************************************************************************
*/
/* includes */
#include <cpss/px/cpssPxTypes.h>
#include <utfTraffic/basic/cpssPxTgfBasicUT.h>
#include <cpss/px/ingress/cpssPxIngress.h>

#define UPLINK_PORT_NUM             12

typedef enum{
    PRINT_PACKET_TYPE_INGRESS_E,
    PRINT_PACKET_TYPE_ACTUAL_EGRESS_E,
    PRINT_PACKET_TYPE_EXPECTED_EGRESS_E
}PRINT_PACKET_TYPE_ENT;

/* print packet */
static void dumpFullPacket_print
(
    IN GT_SW_DEV_NUM                        devNum,
    IN GT_PHYSICAL_PORT_NUM                 portNum,
    IN PRINT_PACKET_TYPE_ENT                printType,
    OUT GT_U8                               *framePtr,
    OUT GT_U32                              frameLength
)
{
    GT_U32 iter;
    GT_U8*  bufferPtr = framePtr;
    GT_U32  length = frameLength;

    PRV_TGF_LOG4_MAC("[UTF]: device[%d] packet [%s] portNum [%d] frameLength[%d]\n",
        devNum,
        (char*)
            (printType == PRINT_PACKET_TYPE_INGRESS_E ? "to ingress" :
             printType == PRINT_PACKET_TYPE_ACTUAL_EGRESS_E ? "actual egress" :
                "expected to egress"),
        portNum,
        frameLength);

    if(length >= 256)
    {
        PRV_TGF_LOG1_MAC("print only first 256 bytes (instead of [%d]) \n", length);
        length = 256;
    }

    for(iter = 0; iter < length ; iter++)
    {
        if((iter & 0x0F) == 0)
        {
            PRV_TGF_LOG1_MAC("0x%4.4x :", iter);
        }

        PRV_TGF_LOG1_MAC(" %2.2x", bufferPtr[iter]);

        if((iter & 0x0F) == 0x0F)
        {
            PRV_TGF_LOG0_MAC("\n");
        }
    }/*iter*/
    PRV_TGF_LOG0_MAC("\n");

    return;
}

/**
* @internal prvTgfPxInjectToIngressPortAndCheckEgressSimple function
* @endinternal
*
* @brief   simple : send SINGLE packet to ingress port and check expected egress ports.
*         function will set :
*         1. link UP
*         2. MAC LOOPBACK
*         3. set 'forwarding' tables to direct the packet from CPU to the port.
*         3. send from cpu packet to egress the port and LOOPBACK will make it
*         ingress the port.
* @param[in] devNum                   - the device number
* @param[in] portNum                  - the ingress port num
* @param[in] framePtr                 - (pointer to) the frame bytes
* @param[in] frameLength              - frame length
* @param[in] numOfEgressPortsInfo     - number of egress ports info in array of egressPortsArr[]
*                                      if 0 --> not checking egress ports
* @param[in] simple_egressPortsArr[]  - array of egress ports to check for counters and packets
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong parameter.
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS prvTgfPxInjectToIngressPortAndCheckEgressSimple
(
    IN GT_SW_DEV_NUM        devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN GT_U8                *framePtr,
    IN GT_U32               frameLength,
    IN GT_U32               numOfEgressPortsInfo,
    IN TGF_EXPECTED_EGRESS_INFO_SIMPLE_STC simple_egressPortsArr[]
)
{
    GT_U32  ii;
    TGF_PACKET_STC              ingressPacketInfo;
    TGF_PACKET_PART_STC         ingressFullPart[1];
    TGF_PACKET_PAYLOAD_STC      ingressPayloadPart;

    static TGF_EXPECTED_EGRESS_INFO_STC egressPortsArr[32];
    static TGF_PACKET_STC           egress_packetInfo[32];
    static TGF_PACKET_PART_STC      egress_fullPart[32];
    static TGF_PACKET_PAYLOAD_STC   egress_payloadPart[32];
    GT_U32  burstCount = 1;

    if(numOfEgressPortsInfo >= 32)
    {
        return GT_BAD_PARAM;
    }

    PRV_TGF_LOG0_MAC("prvTgfPxInjectToIngressPortAndCheckEgressSimple : start \n");
    dumpFullPacket_print(devNum,portNum,
        PRINT_PACKET_TYPE_INGRESS_E,
        framePtr,frameLength);

    ingressPacketInfo.totalLen = TGF_PACKET_AUTO_CALC_LENGTH_CNS;
    ingressPacketInfo.numOfParts = 1;
    ingressPacketInfo.partsArray = &ingressFullPart[0];

    ingressFullPart[0].type = TGF_PACKET_PART_PAYLOAD_E;
    ingressFullPart[0].partPtr = &ingressPayloadPart;

    ingressPayloadPart.dataLength = frameLength;
    ingressPayloadPart.dataPtr    = framePtr;

    for(ii = 0 ; ii < numOfEgressPortsInfo ; ii++)
    {
        egressPortsArr[ii].portNum       = simple_egressPortsArr[ii].portNum;
        egressPortsArr[ii].burstCount    = (simple_egressPortsArr[ii].numBytes ? 1 : 0);
        egressPortsArr[ii].numBytes      = simple_egressPortsArr[ii].numBytes;
        if(simple_egressPortsArr[ii].pktInfo == NULL)
        {
            egressPortsArr[ii].pktInfo = NULL;
        }
        else
        {
            egress_packetInfo[ii].totalLen = TGF_PACKET_AUTO_CALC_LENGTH_CNS;
            egress_packetInfo[ii].numOfParts = 1;
            egress_packetInfo[ii].partsArray = &egress_fullPart[ii];

            egress_fullPart[ii].type = TGF_PACKET_PART_PAYLOAD_E;
            egress_fullPart[ii].partPtr = &egress_payloadPart[ii];

            egress_payloadPart[ii].dataLength = simple_egressPortsArr[ii].numBytes;
            egress_payloadPart[ii].dataPtr    = simple_egressPortsArr[ii].pktInfo;

            egressPortsArr[ii].pktInfo = &egress_packetInfo[ii];

            dumpFullPacket_print(devNum,simple_egressPortsArr[ii].portNum,
                PRINT_PACKET_TYPE_EXPECTED_EGRESS_E,
                simple_egressPortsArr[ii].pktInfo,
                simple_egressPortsArr[ii].numBytes);
        }
    }

    return tgfTrafficGeneratorPxTransmitPacketAndCheckResult(devNum,portNum,&ingressPacketInfo,
            burstCount,numOfEgressPortsInfo,egressPortsArr);
}

/**
* @internal prvTgfPxInjectToIngressPortAndCheckEgressSimpleList function
* @endinternal
*
* @brief   simple : send SINGLE packet (build of list of simple parts) to ingress
*         port and check expected egress ports.
*         function will set :
*         1. link UP
*         2. MAC LOOPBACK
*         3. set 'forwarding' tables to direct the packet from CPU to the port.
*         3. send from cpu packet to egress the port and LOOPBACK will make it
*         ingress the port.
* @param[in] devNum                   - the device number
* @param[in] portNum                  - the ingress port num
* @param[in] ingressFramePtr          - (pointer to) the frame parts bytes
* @param[in] numOfEgressPortsInfo     - number of egress ports info in array of egressPortsArr[]
*                                      if 0 --> not checking egress ports
* @param[in] simpleList_egressPortsArr[] - array of egress ports to check for counters and packets
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong parameter.
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS prvTgfPxInjectToIngressPortAndCheckEgressSimpleList
(
    IN GT_SW_DEV_NUM        devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN TGF_BYTES_INFO_LIST_STC *ingressFramePtr,
    IN GT_U32               numOfEgressPortsInfo,
    IN TGF_EXPECTED_EGRESS_INFO_SIMPLE_LIST_STC simpleList_egressPortsArr[]
)
{
    GT_U32  ii,jj;
    TGF_PACKET_STC              ingressPacketInfo;
    TGF_PACKET_PART_STC         ingressFullPart[TGF_MAX_PARTS];
    TGF_PACKET_PAYLOAD_STC      ingressPayloadPart[TGF_MAX_PARTS];

    static TGF_EXPECTED_EGRESS_INFO_STC egressPortsArr[32];
    static TGF_PACKET_STC           egress_packetInfo[32];
    static TGF_PACKET_PART_STC      egress_fullPart[32][TGF_MAX_PARTS];
    static TGF_PACKET_PAYLOAD_STC   egress_payloadPart[32][TGF_MAX_PARTS];
    TGF_BYTES_INFO_LIST_STC         *currEgressFramePtr;
    GT_U32  burstCount = 1;

    if(numOfEgressPortsInfo >= 32)
    {
        return GT_BAD_PARAM;
    }

    if(ingressFramePtr->numOfParts > TGF_MAX_PARTS)
    {
        return GT_BAD_PARAM;
    }

    ingressPacketInfo.totalLen = TGF_PACKET_AUTO_CALC_LENGTH_CNS;
    ingressPacketInfo.numOfParts = ingressFramePtr->numOfParts;
    ingressPacketInfo.partsArray = &ingressFullPart[0];


    for(ii = 0; ii < ingressFramePtr->numOfParts; ii++)
    {
        if(ingressFramePtr->partsInfo[ii].numOfBytes)
        {
            ingressFullPart[ii].type = TGF_PACKET_PART_PAYLOAD_E;
            ingressFullPart[ii].partPtr = &ingressPayloadPart[ii];

            ingressPayloadPart[ii].dataLength = ingressFramePtr->partsInfo[ii].numOfBytes;
            ingressPayloadPart[ii].dataPtr    = ingressFramePtr->partsInfo[ii].partBytesPtr;
        }
        else
        {
            ingressFullPart[ii].type = TGF_PACKET_PART_SKIP_E;
        }
    }

    for(ii = 0 ; ii < numOfEgressPortsInfo ; ii++)
    {
        egressPortsArr[ii].portNum       = simpleList_egressPortsArr[ii].portNum;
        egressPortsArr[ii].burstCount    = 1;

        currEgressFramePtr = &simpleList_egressPortsArr[ii].egressFrame;
        if(currEgressFramePtr->numOfParts > TGF_MAX_PARTS)
        {
            return GT_BAD_PARAM;
        }

        if(currEgressFramePtr->numOfParts == 1 &&
           currEgressFramePtr->partsInfo[0].partBytesPtr == NULL)
        {
            /*********************************************************************************/
            /* indication that only needs to check byte count counter and not packet content */
            /*********************************************************************************/
            egressPortsArr[ii].numBytes = currEgressFramePtr->partsInfo[0].numOfBytes;
            egressPortsArr[ii].pktInfo = NULL;

            if(egressPortsArr[ii].numBytes == 0)
            {
                /*******************************************************/
                /* indication that egress port MUST not get the packet */
                /*******************************************************/
                egressPortsArr[ii].burstCount = 0;
            }

            continue;
        }

        egressPortsArr[ii].pktInfo = &egress_packetInfo[ii];

        egress_packetInfo[ii].totalLen = TGF_PACKET_AUTO_CALC_LENGTH_CNS;
        egress_packetInfo[ii].numOfParts = currEgressFramePtr->numOfParts;
        egress_packetInfo[ii].partsArray = &egress_fullPart[ii][0];

        for(jj = 0 ; jj < currEgressFramePtr->numOfParts; jj++)
        {
            if(0 == currEgressFramePtr->partsInfo[jj].numOfBytes)
            {
                egress_fullPart[ii][jj].type = TGF_PACKET_PART_SKIP_E;
            }
            else
            {
                egress_fullPart[ii][jj].type = TGF_PACKET_PART_PAYLOAD_E;
                egress_fullPart[ii][jj].partPtr = &egress_payloadPart[ii][jj];

                egress_payloadPart[ii][jj].dataLength = currEgressFramePtr->partsInfo[jj].numOfBytes;
                egress_payloadPart[ii][jj].dataPtr    = currEgressFramePtr->partsInfo[jj].partBytesPtr;
            }
        }
    }

    return tgfTrafficGeneratorPxTransmitPacketAndCheckResult(devNum,portNum,&ingressPacketInfo,
            burstCount,numOfEgressPortsInfo,egressPortsArr);
}


static GT_U8 frameArr[] = {
      0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,          /*mac da FF:FF:FF:FF:FF:FF */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x01,          /*mac sa 00:00:00:00:00:01 */
      0x81, 0x00, 0x0F, 0xFE,                      /*vlan 4094                */
      0x55, 0x55,                                  /*ethertype                */
      0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, /*payload  */
      0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
      0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23,
      0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
      0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B,
      0x3C, 0x3D, 0x3E, 0x3F};

GT_STATUS prvTgfPxTrafficSanity(
    IN GT_SW_DEV_NUM        devNum
)
{
    GT_STATUS   rc;
    GT_U32      ii;
    GT_PHYSICAL_PORT_NUM portNum = 7;
    GT_U32               frameLength;
    GT_U32               numOfEgressPortsInfo;
    TGF_EXPECTED_EGRESS_INFO_SIMPLE_STC simple_egressPortsArr[4];
    GT_U32               egressPorts[4]={0,4,5,UPLINK_PORT_NUM};

    frameLength = sizeof(frameArr);

    PRV_TGF_LOG0_MAC("prvTgfPxTrafficSanity : start \n");

    for(ii = 0; ii < 4 ; ii++)
    {
        simple_egressPortsArr[ii].portNum = egressPorts[ii];
        simple_egressPortsArr[ii].numBytes = frameLength + TGF_CRC_LEN_CNS;
        /* not checking yet egress packets */
        simple_egressPortsArr[ii].pktInfo = frameArr;
    }

    /* to show in example that only the egress_numBytes can be set without egress_framePtr */
    simple_egressPortsArr[2].pktInfo = NULL;


    numOfEgressPortsInfo = ii;

    rc = prvTgfPxInjectToIngressPortAndCheckEgressSimple(devNum,portNum,
        frameArr,frameLength,numOfEgressPortsInfo,simple_egressPortsArr);

    if(rc != GT_OK)
    {
        PRV_TGF_LOG0_MAC("prvTgfPxTrafficSanity : Test FAILED \n");
    }
    else
    {
        PRV_TGF_LOG0_MAC("prvTgfPxTrafficSanity : Test PASSED \n");
    }

    return rc;
}

GT_STATUS prvTgfPxTrafficSanityList(
    IN GT_SW_DEV_NUM        devNum
)
{
    GT_STATUS   rc;
    GT_U32      ii,jj,totalBytes;
    GT_PHYSICAL_PORT_NUM portNum = 7;
    GT_U32               frameLength;
    GT_U32               numOfEgressPortsInfo;
    TGF_EXPECTED_EGRESS_INFO_SIMPLE_LIST_STC simpleList_egressPortsArr[4];
    TGF_BYTES_INFO_LIST_STC ingressFrame;
    TGF_BYTES_INFO_LIST_STC *currEgressFramePtr;
    TGF_BYTES_PART_STC       *currBytePart;
    GT_U32               egressPorts[4]={1,2,7,UPLINK_PORT_NUM};

    frameLength = sizeof(frameArr);

    /***************************************/
    /* set the ingress packet as '2' parts */
    /***************************************/
    ingressFrame.numOfParts = 2;
    ingressFrame.partsInfo[0].numOfBytes = 21;
    ingressFrame.partsInfo[0].partBytesPtr = &frameArr[0];
    ingressFrame.partsInfo[1].numOfBytes = frameLength - ingressFrame.partsInfo[0].numOfBytes;
    ingressFrame.partsInfo[1].partBytesPtr = &frameArr[ingressFrame.partsInfo[0].numOfBytes];

    PRV_TGF_LOG0_MAC("prvTgfPxTrafficSanityList : start \n");

    /***************************************************/
    /* set the egress packet as '3/4/5' parts */
    /***************************************************/


    for(ii = 0; ii < 4 ; ii++)
    {
        simpleList_egressPortsArr[ii].portNum = egressPorts[ii];
        currEgressFramePtr = &simpleList_egressPortsArr[ii].egressFrame;

        currEgressFramePtr->numOfParts = ii+3;

        totalBytes = 0;

        for(jj = 0; jj < currEgressFramePtr->numOfParts; jj++)
        {
            currBytePart = &currEgressFramePtr->partsInfo[jj];

            currBytePart->numOfBytes   = (frameLength / currEgressFramePtr->numOfParts);
            currBytePart->partBytesPtr =  &frameArr[totalBytes];

            totalBytes += currBytePart->numOfBytes;
        }

        /*********************************/
        /* update bytes in the last part */
        /*********************************/
        currBytePart = &currEgressFramePtr->partsInfo[jj-1];
        if(totalBytes >= frameLength)
        {
            currBytePart->numOfBytes -= (totalBytes - frameLength);
        }
        else
        {
            currBytePart->numOfBytes += (frameLength - totalBytes);
        }

        currBytePart->numOfBytes += TGF_CRC_LEN_CNS;

    }

    /* to show in example that only the egress_numBytes can be set without egress_framePtr */
    simpleList_egressPortsArr[2].egressFrame.numOfParts = 1;
    simpleList_egressPortsArr[2].egressFrame.partsInfo[0].numOfBytes = frameLength + TGF_CRC_LEN_CNS;
    simpleList_egressPortsArr[2].egressFrame.partsInfo[0].partBytesPtr = NULL;

    numOfEgressPortsInfo = ii;

    rc = prvTgfPxInjectToIngressPortAndCheckEgressSimpleList(devNum,portNum,
        &ingressFrame,numOfEgressPortsInfo,simpleList_egressPortsArr);

    if(rc != GT_OK)
    {
        PRV_TGF_LOG0_MAC("prvTgfPxTrafficSanityList : Test FAILED \n");
    }
    else
    {
        PRV_TGF_LOG0_MAC("prvTgfPxTrafficSanityList : Test PASSED \n");
    }

    return rc;
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Sanity Test - to check that a test can send packet with single pointer,
 * and expect egress ports to match egress packet (as single pointer)
 */
UTF_TEST_CASE_MAC(prvTgfPxTrafficSanity)
{
    GT_STATUS   st;
    GT_U8       dev = 0;

    /* Prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvTgfPxTrafficSanity(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Sanity Test - to check that a test can send packet with list of pointers
 * (for example: for case of 'base packet' with ETag and with Vlan tag ,
    that is expected to egress different port with/without ETag and/or Vlan tag)
 * and expect egress ports to match egress packet (with list of pointers)
 */
UTF_TEST_CASE_MAC(prvTgfPxTrafficSanityList)
{
    GT_STATUS   st;
    GT_U8       dev = 0;

    /* Prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvTgfPxTrafficSanityList(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }
}


/*----------------------------------------------------------------------------*/
/*
 * Configuration of cpssPxTgfBasic suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssPxTgfBasic)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPxTrafficSanity)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPxTrafficSanityList)
UTF_SUIT_END_TESTS_MAC(cpssPxTgfBasic)


