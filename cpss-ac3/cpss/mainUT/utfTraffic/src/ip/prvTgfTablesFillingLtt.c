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
* @file prvTgfTablesFillingLtt.c
*
* @brief Low Level IP table filling test
*
* @version   12
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/generic/cpssCommonDefs.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>
#include <utf/private/prvUtfExtras.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfIpGen.h>
#include <common/tgfPortGen.h>

#include <ip/prvTgfTablesFillingLtt.h>


/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

/* default VLAN Id */
#define PRV_TGF_SEND_VLANID_CNS            5

/* nexthope VLAN for the routing */
#define PRV_TGF_NEXTHOPE_VLANID_CNS        7

/* number of ports */
#define PRV_TGF_PORT_COUNT_CNS             4

/* port index to send traffic to */
#define PRV_TGF_TX_0_IDX_CNS               0

/* port index to receive traffic from */
#define PRV_TGF_RX_1_IDX_CNS               1

/* Base Indexes for the Route, ARP */
static GT_U32        prvTgfRouteEntryBaseIndex = 5;
static GT_U32        prvTgfArpEntryBaseIndex   = 3;

/* Router ARP Table Configuration */
static TGF_MAC_ADDR  prvTgfArpMac = {0x00, 0x00, 0x00, 0x11, 0x22, 0x00};

/* Default Destination IPv4 address for the prefixes with full length */
static TGF_IPV4_ADDR prvTgfDefDipv4 = {0x80, 0, 0, 0};

/* Default Destination IPv6 address for the prefixes with full length */
static TGF_IPV6_ADDR prvTgfDefDipv6 =
{
    0x8122, 0, 0, 0, 0, 0, 0, 0
};

/* Default Destination IPv4 address for the prefixes with various lengths */
static TGF_IPV4_ADDR prvTgfDefDipv4Various = {0, 0, 0, 2};

/* Default Destination IPv6 address for the prefixes with various lengths */
static TGF_IPV6_ADDR prvTgfDefDipv6Various =
{
    0, 0, 0, 0, 0, 0, 0, 2
};

/* buffer for the current IPv4 or IPv6 address */
static GT_U8    prvTgfIpCur[16] = {0};

/* number of wrong (not matched) packets to send */
static GT_U32   prvTgfWrongPackets = 10;

/* the LPM DB id for LPM Table */
static GT_U32   prvTgfLpmDBId = 0;

/* flag to indicate if needs to logging generated prefixes */
static GT_BOOL  prvTgfEnablePrefixLog = GT_FALSE;

/**************************** Test IPv4 packet ********************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part =
{
    {0x00, 0x00, 0x00, 0x00, 0xAA, 0x00},               /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0xAA, 0x18}                /* saMac */
};

/* packet's ethertype */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketEtherTypePart =
{
    TGF_ETHERTYPE_0800_IPV4_TAG_CNS
};

/* packet's IPv4 */
static TGF_PACKET_IPV4_STC prvTgfPacketIpv4Part =
{
    4,                                 /* version */
    5,                                 /* headerLen */
    0,                                 /* typeOfService */
    0x31,                              /* totalLen */
    0,                                 /* id */
    0,                                 /* flags */
    0,                                 /* offset */
    0x40,                              /* timeToLive */
    0xFF,                              /* protocol */
    TGF_PACKET_AUTO_CALC_CHECKSUM_CNS, /* csum */
    { 2,  2,  2,  4},                  /* srcAddr */
    { 1,  1,  1,  1}                   /* dstAddr */
};

/* DATA of packet */
static GT_U8 prvTgfPayloadDataArr[] =
{
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d
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
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacketIpv4Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* LENGTH of packet */
#define PRV_TGF_PACKET_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS + \
    TGF_IPV4_HEADER_SIZE_CNS + sizeof(prvTgfPayloadDataArr)

/* PACKET IPv4 to send */
static TGF_PACKET_STC prvTgfPacketInfo =
{
    PRV_TGF_PACKET_LEN_CNS,                                      /* totalLen */
    sizeof(prvTgfPacketPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketPartArray                                        /* partsArray */
};

/**************************** Test IPv6 packet ********************************/

/* packet's ethertype */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketIpv6EtherTypePart =
{
    TGF_ETHERTYPE_86DD_IPV6_TAG_CNS
};

/* packet's IPv6 */
static TGF_PACKET_IPV6_STC prvTgfPacketIpv6Part =
{
    6,                                       /* version */
    0,                                       /* trafficClass */
    0,                                       /* flowLabel */
    0x02,                                    /* payloadLen */
    0x3b,                                    /* nextHeader */
    0x40,                                    /* hopLimit */
    {0x6545, 0, 0, 0, 0, 0, 0x0000, 0x3212}, /* TGF_IPV6_ADDR srcAddr */
    {0x1122, 0, 0, 0, 0, 0, 0xccdd, 0xeeff}  /* TGF_IPV6_ADDR dstAddr */
};

/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfPacketIpv6PartArray[] =
{
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketIpv6EtherTypePart},
    {TGF_PACKET_PART_IPV6_E,      &prvTgfPacketIpv6Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* LENGTH of packet */
#define PRV_TGF_PACKET_IPV6_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS + \
    TGF_IPV6_HEADER_SIZE_CNS + sizeof(prvTgfPayloadDataArr)

/* PACKET IPv6 to send */
static TGF_PACKET_STC prvTgfPacketIpv6Info =
{
    PRV_TGF_PACKET_IPV6_LEN_CNS,                                     /* totalLen */
    sizeof(prvTgfPacketIpv6PartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketIpv6PartArray                                        /* partsArray */
};

/*************************** Restore config ***********************************/

/* parameters that is needed to be restored */
static struct
{
    GT_U16 vid;

    /* IPv4 UC default prefix */
    GT_U32 routeDefUcIdx;
    GT_U32 rowUcIpv4;
    GT_U32 colUcIpv4;

    /* IPv6 UC default prefix */
    GT_U32 rowUcIpv6;
    GT_U32 colUcIpv6;

    /* IPv4 MC default prefix */
    GT_U32 routeDefMcIdx;
    GT_U32 rowMcGrpIpv4;
    GT_U32 colMcGrpIpv4;
    GT_U32 rowMcSrcIpv4;
    GT_U32 colMcSrcIpv4;

    /* IPv6 MC default prefix */
    GT_U32 rowMcGrpIpv6;
    GT_U32 colMcSrcIpv6;
} prvTgfRestoreCfg;

/******************************************************************************\
 *                           Private declarations                             *
\******************************************************************************/

/* enumeration that allow select a mode for next IP address */
typedef enum
{
    PRV_TGF_NEXT_IP_MODE_INC_E,
    PRV_TGF_NEXT_IP_MODE_SHL_E,
    PRV_TGF_NEXT_IP_MODE_INC_VAL_E,
    PRV_TGF_NEXT_IP_MODE_RANDOM_E,
    PRV_TGF_NEXT_IP_MODE_LAST_E
} PRV_TGF_NEXT_IP_MODE_ENT;

/* available modes to calculate IP addresses in the test */
static PRV_TGF_NEXT_IP_MODE_ENT prvTgfNextIpMode = PRV_TGF_NEXT_IP_MODE_INC_E;

/******************************************************************************\
 *                            Private test functions                          *
\******************************************************************************/

/**
* @internal prvTgfTablesFillingLttIpFromPtr function
* @endinternal
*
* @brief   Copy IPv4 or IPv6 address from the ipPtr
*
* @param[in] protocol                 - type of IP stack used
* @param[in] ipPtr                    - (pointer to) IP address
*
* @param[out] ipv4Addr                 - IPv4 address
* @param[out] ipv6Addr                 - IPv6 address
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfTablesFillingLttIpFromPtr
(
    IN  CPSS_IP_PROTOCOL_STACK_ENT protocol,
    OUT TGF_IPV4_ADDR              ipv4Addr,
    OUT TGF_IPV6_ADDR              ipv6Addr,
    IN  GT_U8                      *ipPtr
)
{
    GT_U32 offset = 0;

    /* check input parameters */
    CPSS_NULL_PTR_CHECK_MAC(ipPtr);

    /* copy IP address from the ipPtr */
    if (CPSS_IP_PROTOCOL_IPV4_E == protocol)
    {
        cpssOsMemCpy(ipv4Addr, ipPtr, sizeof(TGF_IPV4_ADDR));
    }
    else
    {
        for (offset = 0; offset < 8; offset++)
        {
            ipv6Addr[offset] = (GT_U16) (ipPtr[offset << 1] << 8);
            ipv6Addr[offset] |= ipPtr[(offset << 1) + 1];
        }
    }

    return GT_OK;
}

/**
* @internal prvTgfTablesFillingLttIpToPtr function
* @endinternal
*
* @brief   Copy IPv4 or IPv6 address to the ipPtr
*
* @param[in] protocol                 - type of IP stack used
* @param[in] ipv4Addr                 - IPv4 address
* @param[in] ipv6Addr                 - IPv6 address
*
* @param[out] ipPtr                    - (pointer to) IP address
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfTablesFillingLttIpToPtr
(
    IN  CPSS_IP_PROTOCOL_STACK_ENT protocol,
    IN  TGF_IPV4_ADDR              ipv4Addr,
    IN  TGF_IPV6_ADDR              ipv6Addr,
    OUT GT_U8                      *ipPtr
)
{
    GT_U32 offset = 0;

    /* check input parameters */
    CPSS_NULL_PTR_CHECK_MAC(ipPtr);

    /* copy IP address to the ipPtr */
    if (CPSS_IP_PROTOCOL_IPV4_E == protocol)
    {
        cpssOsMemCpy(ipPtr, ipv4Addr, sizeof(TGF_IPV4_ADDR));
    }
    else
    {
        for (offset = 0; offset < 8; offset++)
        {
            ipPtr[offset << 1]       = (GT_U8)(ipv6Addr[offset] >> 8);
            ipPtr[(offset << 1) + 1] = (GT_U8) ipv6Addr[offset];
        }
    }

    return GT_OK;
}

/**
* @internal prvTgfTablesFillingLttIpInc function
* @endinternal
*
* @brief   Increment IPv4 or IPv6 address
*
* @param[in] protocol                 - type of IP stack used
* @param[in,out] ipPtr                    - (pointer to) IP address
* @param[in,out] ipPtr                    - (pointer to) IP address + 1
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfTablesFillingLttIpInc
(
    IN    CPSS_IP_PROTOCOL_STACK_ENT protocol,
    INOUT GT_U8                      *ipPtr
)
{
    GT_U8 offset = 0;

    /* check input parameters */
    CPSS_NULL_PTR_CHECK_MAC(ipPtr);

    /* set offset for IPv4\IPv6 addr */
    offset = (GT_U8) ((CPSS_IP_PROTOCOL_IPV4_E == protocol) ? 4 : 16);

    /* increment each byte in IP addr */
    while (offset--)
    {
        if (++ipPtr[offset])
        {
            break;
        }
    }

    return GT_OK;
}

/**
* @internal prvTgfTablesFillingLttIpShl function
* @endinternal
*
* @brief   Shift Left IPv4 or IPv6 address
*
* @param[in] protocol                 - type of IP stack used
* @param[in,out] ipPtr                    - (pointer to) IP address
* @param[in,out] ipPtr                    - (pointer to) IP address << 1
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfTablesFillingLttIpShl
(
    IN    CPSS_IP_PROTOCOL_STACK_ENT protocol,
    INOUT GT_U8                      *ipPtr
)
{
    GT_U8 offset    = 0;
    GT_U8 carrySave = 0;
    GT_U8 carry     = 0;

    /* check input parameters */
    CPSS_NULL_PTR_CHECK_MAC(ipPtr);

    /* set offset for IPv4\IPv6 addr */
    offset = (GT_U8) ((CPSS_IP_PROTOCOL_IPV4_E == protocol) ? 4 : 16);

    /* shift each byte in IP addr */
    while (offset--)
    {
        carry = carrySave;
        carrySave = (GT_U8) ((ipPtr[offset] & (1 << 7)) ? 1 : 0);
        ipPtr[offset] <<= 1;
        ipPtr[offset] = (GT_U8) (ipPtr[offset] + carry);
    }

    return GT_OK;
}

/**
* @internal prvTgfTablesFillingLttNextAddrGet function
* @endinternal
*
* @brief   Get Next IP Address
*
* @param[in] protocol                 - type of IP stack used
* @param[in,out] ipPtr                    - (pointer to) current IP address
* @param[in,out] ipPtr                    - (pointer to) next IP address
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfTablesFillingLttNextAddrGet
(
    IN    CPSS_IP_PROTOCOL_STACK_ENT protocol,
    INOUT GT_U8                      *ipPtr
)
{
    GT_STATUS   rc = GT_OK;

    /* check input parameters */
    CPSS_NULL_PTR_CHECK_MAC(ipPtr);

    /* choose IP update mode */
    switch (prvTgfNextIpMode)
    {
        case PRV_TGF_NEXT_IP_MODE_INC_E:
            rc = prvTgfTablesFillingLttIpInc(protocol, ipPtr);
            PRV_UTF_VERIFY_GT_OK(rc, "prvTgfTablesFillingLttIpInc");

            break;

        case PRV_TGF_NEXT_IP_MODE_SHL_E:
            rc = prvTgfTablesFillingLttIpShl(protocol, ipPtr);
            PRV_UTF_VERIFY_GT_OK(rc, "prvTgfTablesFillingLttIpShl");

            break;

        case PRV_TGF_NEXT_IP_MODE_INC_VAL_E:
        case PRV_TGF_NEXT_IP_MODE_RANDOM_E:
            return GT_NOT_IMPLEMENTED;
            break;

        default:
            return GT_BAD_PARAM;
    }

    return rc;
}

/**
* @internal prvTgfTablesFillingLttCapturedPacketGet function
* @endinternal
*
* @brief   In each call function gets next captured packet from captured table
*         on specific port interface
* @param[in] portInterfacePtr         - (pointer to) port interface
* @param[in,out] packetLenPtr             - (pointer to) length of the allocated gtBuf
*
* @param[out] packetBufPtr             - (pointer to) packet's buffer (pre allocated by the user)
* @param[in,out] packetLenPtr             - (pointer to) original length of the captured packet
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - wrong interface
* @retval GT_NO_MORE               - end of table reached
* @retval GT_BAD_PTR               - on NULL pointer
*
* @note The 'captured' (emulation of capture) must be set on this port
*       see tgfTrafficGeneratorPortTxEthCaptureSet.
*       All packets from another port interfaces will be skipped, to retrieve
*       those packets it is needed to set appropriate port interface.
*       Each function call gets next captured packet from captured table till
*       GT_NO_MORE reached
*
*/
static GT_STATUS prvTgfTablesFillingLttCapturedPacketGet
(
    IN    CPSS_INTERFACE_INFO_STC       *portInterfacePtr,
    OUT   GT_U8                         *packetBufPtr,
    INOUT GT_U32                        *packetLenPtr
)
{
    GT_STATUS       rc       = GT_OK;
    GT_U8           devNum   = 0;
    GT_U8           queue    = 0;
    GT_U32          origPacketLen = 0;
    TGF_NET_DSA_STC rxParam  = {0};
static GT_BOOL      getFirst = GT_TRUE;

    /* check input parameters */
    CPSS_NULL_PTR_CHECK_MAC(portInterfacePtr);

    if (portInterfacePtr->type != CPSS_INTERFACE_PORT_E)
    {
        return GT_BAD_PARAM;
    }

    /* get next entry from rxNetworkIf table - skip all not expected packets */
    rc = tgfTrafficGeneratorRxInCpuFromExpectedInterfaceGet(portInterfacePtr,
                                        TGF_PACKET_TYPE_CAPTURE_E,
                                       getFirst, GT_FALSE, packetBufPtr,
                                       packetLenPtr, &origPacketLen,
                                       &devNum, &queue, &rxParam);

    getFirst = GT_FALSE; /* now we get the next */

    (*packetLenPtr)--;

    /* check if we reach the end of table */
    if (GT_NO_MORE == rc)
    {
        getFirst = GT_TRUE;
    }

    return rc;
}

/**
* @internal prvTgfTablesFillingLttCapturedTableCheck function
* @endinternal
*
* @brief   scan whole captured table until GT_NO_MORE
*         to check all burstCount expected packet
* @param[in] protocol                 - type of IP stack used
* @param[in] packetIter               - number already checked packets in the test
* @param[in] portInterfacePtr         - (pointer to) port for capturing
* @param[in] burstCount               - number of packet to expect in capturing table
* @param[in] vfdPtr                   - (pointer to) pattern for checking
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_VOID prvTgfTablesFillingLttCapturedTableCheck
(
    IN CPSS_IP_PROTOCOL_STACK_ENT protocol,
    IN GT_U32                     packetIter,
    IN CPSS_INTERFACE_INFO_STC    *portInterfacePtr,
    IN GT_U32                     burstCount,
    IN TGF_VFD_INFO_STC           *vfdPtr
)
{
    GT_STATUS   rc        = GT_OK;
    GT_U32      packetLen = 0;
    GT_U32      burstIter = 0;
    GT_U8       packetBuff[TGF_RX_BUFFER_MAX_SIZE_CNS] = {0};

    /* scan whole captured table to check all burstCount expected packet */
    for (burstIter = 0; burstIter <= burstCount; burstIter++)
    {
        /* get next captured packet in specified interface */
        packetLen = TGF_RX_BUFFER_MAX_SIZE_CNS;

        rc = prvTgfTablesFillingLttCapturedPacketGet(portInterfacePtr, packetBuff, &packetLen);

        /* finish the loop if there are no more packets in captured table */
        if (GT_NO_MORE == rc)
        {
            /* trace expected DIP */
            if (burstIter != burstCount)
            {
                GT_U8 *p = vfdPtr->patternPtr;

                PRV_UTF_LOG1_MAC("\nexpected DIP for the packet# %d: ",packetIter + burstIter);
                while (p < vfdPtr->patternPtr + vfdPtr->cycleCount)
                {
                    PRV_UTF_LOG1_MAC(" %2.2x", *p++);
                }
            }

            /* check if we have burstCount packets in the capturing table */
            UTF_VERIFY_EQUAL0_STRING_MAC(burstCount, burstIter,
                    "\n   Capturing table has less packets than expected.\n");

            break;
        }

        /* check if we've really got a packet in the buffer */
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
                "\n   tgfTrafficGeneratorCapturedPacketGet: %d, %d, %d\n",
                portInterfacePtr->devPort.hwDevNum, portInterfacePtr->devPort.portNum,
                packetLen);

        /* the capturing table must have no more than burstCount packets */
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_TRUE, burstIter < burstCount,
                "\n   Capturing table has more packets than expected.\n");

        /* check the packet in the buffer if we've got it */
        if (GT_OK == rc)
        {
            /* check whether captured packet has the proper IP address */
            rc = (0 == cpssOsMemCmp(packetBuff + vfdPtr->offset, vfdPtr->patternPtr,
                                    vfdPtr->cycleCount)) ? GT_OK : GT_FAIL;

            /* trace a BAD packet if it differs from an expected one */
            if (GT_OK != rc)
            {
                GT_U8 *p = vfdPtr->patternPtr;

                PRV_UTF_LOG1_MAC("packet# %d, expected DIP: ", packetIter + burstIter);
                while (p < vfdPtr->patternPtr + vfdPtr->cycleCount)
                {
                    PRV_UTF_LOG1_MAC(" %2.2x", *p++);
                }
                PRV_UTF_LOG0_MAC("\n");

                /* print packet */
                rc = tgfTrafficTracePacket(packetBuff, packetLen, GT_TRUE);
                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTracePacket");
            }

            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "\n   captured packet has wrong DIP");
        }

        /* get next address */
        rc = prvTgfTablesFillingLttNextAddrGet(protocol, vfdPtr->patternPtr);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTablesFillingLttNextAddrGet\n");
    }
}

/**
* @internal prvTgfTablesFillingLttWrite function
* @endinternal
*
* @brief   Fill LTT by entries in range
*
* @param[in] rowFirst                 - from this row
* @param[in] rowLast                  - to this row
* @param[in] colFirst                 - from this column
* @param[in] colLast                  - to this column
* @param[in] routeEntryBaseIndex      - the entry base index in Route Table
*
* @param[out] numErrorsPtr             - number of errors caused with prvTgfIpLttWriteIPv4 call
* @param[out] firstErrorPtr            - rc of prvTgfIpLttWriteIPv4 call when first error happen
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfTablesFillingLttWrite
(
    IN  GT_U32 rowFirst,
    IN  GT_U32 rowLast,
    IN  GT_U32 colFirst,
    IN  GT_U32 colLast,
    IN  GT_U32 routeEntryBaseIndex,
    OUT GT_U32 *numErrorsPtr,
    OUT GT_U32 *firstErrorPtr
)
{
    GT_STATUS   rc        = GT_OK;
    GT_STATUS   firstRc   = GT_OK;
    GT_U32      numErrors = 0;
    GT_U32      row, col  = 0;
    PRV_TGF_IP_LTT_ENTRY_STC lttEntry = {0};

    lttEntry.routeEntryBaseIndex = routeEntryBaseIndex;
    lttEntry.routeType = PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_MAC;

    PRV_UTF_LOG1_MAC("Write %d LTT entries to the LTT\n",
                     ((rowLast - rowFirst + 1) * (colLast - colFirst + 1)));

    /* reset error counter */
    numErrors      = 0;
    *firstErrorPtr = 0;

    /* iterate thru all rows and cols */
    for (row = rowFirst; row <= rowLast; row++)
    {
        for (col = colFirst; col <= colLast; col++)
        {
            /* write LTT entry to the Router LookUp Translation Table */
            rc = prvTgfIpLttWrite(row, col, &lttEntry);

            /* accumulate errors in numErrors */
            if (GT_OK != rc)
            {
                /* capture first error */
                if (0 == *firstErrorPtr)
                {
                    *firstErrorPtr = row * 4 + col;
                    firstRc        = rc;
                }

                numErrors++;
            }
        }
    }

    *numErrorsPtr = numErrors;

    return firstRc;
}

/**
* @internal prvTgfTablesFillingLttIpv4FullLenSet function
* @endinternal
*
* @brief   Fill Router TCAM Table by UC IPv4 prefixes with full length in range
*
* @param[in] rowFirst                 - from this row
* @param[in] rowLast                  - to this row
* @param[in] numSkipped               - number of skipped entries at the beginning of the range
*
* @param[out] numErrorsPtr             - number of errors caused with prvTgfIpv4PrefixSet call
* @param[out] firstErrorPtr            - rc of prvTgfIpv4PrefixSet call when first error happen
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfTablesFillingLttIpv4FullLenSet
(
    IN  GT_U32 rowFirst,
    IN  GT_U32 rowLast,
    IN  GT_U32 numSkipped,
    OUT GT_U32 *numErrorsPtr,
    OUT GT_U32 *firstErrorPtr
)
{
    GT_STATUS rc        = GT_OK;
    GT_STATUS firstRc   = GT_OK;
    GT_U32    numErrors = 0;
    GT_U32    row, col  = 0;
    PRV_TGF_IPV4_PREFIX_STC ipv4Pref, ipv4Mask;

    PRV_UTF_LOG1_MAC("Set %d IPv4 prefixes with full length\n",
                     ((rowLast - rowFirst + 1) * 4 - numSkipped));

    /* set default IP address for the prefixes with full lengths */
    cpssOsMemSet(&ipv4Pref, 0, sizeof(ipv4Pref));

    /* fill IPv4 prefix with default DIP */
    rc = prvTgfTablesFillingLttIpToPtr(CPSS_IP_PROTOCOL_IPV4_E, prvTgfDefDipv4,
                                       prvTgfDefDipv6, ipv4Pref.ipAddr.arIP);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfTablesFillingLttIpToPtr");

    /* set default IP mask for the prefixes with full lengths */
    cpssOsMemSet(&ipv4Mask, 0, sizeof(ipv4Mask));
    cpssOsMemSet(ipv4Mask.ipAddr.arIP, 0xFF, sizeof(ipv4Mask.ipAddr.arIP));

    /* reset error counter */
    numErrors      = 0;
    *firstErrorPtr = 0;

    /* iterate thru all rows and cols */
    for (row = rowFirst; row <= rowLast; row++)
    {
        for (col = 0; col < 4; col++)
        {
            /* skip numSkipped prefixes for other action in future */
            if (numSkipped)
            {
                numSkipped--;
                continue;
            }

            /* print generated prefix */
            if (GT_TRUE == prvTgfEnablePrefixLog)
            {
                PRV_UTF_LOG2_MAC("row=%d, col=%d: ", row, col);

                rc = prvTgfPrefixPrint(CPSS_IP_PROTOCOL_IPV4_E,
                                       ipv4Pref.ipAddr.arIP,
                                       ipv4Mask.ipAddr.arIP,
                                       sizeof(ipv4Pref.ipAddr.arIP) * 8);
                PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPrefixPrint");
            }

            /* set prefix in Router TCAM Table */
            rc = prvTgfIpv4PrefixSet(row, col, &ipv4Pref, &ipv4Mask);

            /* accumulate errors in numErrors */
            if (GT_OK != rc)
            {
                /* capture first error */
                if (0 == *firstErrorPtr)
                {
                    *firstErrorPtr = row * 4 + col;
                    firstRc        = rc;
                }

                numErrors++;
            }

            /* increase ip */
            rc = prvTgfTablesFillingLttIpInc(CPSS_IP_PROTOCOL_IPV4_E, ipv4Pref.ipAddr.arIP);
            PRV_UTF_VERIFY_GT_OK(rc, "prvTgfTablesFillingLttIpInc");
        }
    }

    *numErrorsPtr = numErrors;

    return firstRc;
}

/**
* @internal prvTgfTablesFillingLttIpv6FullLenSet function
* @endinternal
*
* @brief   Fill Router TCAM Table by UC IPv6 prefixes with full length in range
*
* @param[in] rowFirst                 - from this row
* @param[in] rowLast                  - to this row
* @param[in] numSkipped               - number of skipped entries at the beginning of the range
*
* @param[out] numErrorsPtr             - number of errors caused with prvTgfIpv6PrefixSet call
* @param[out] firstErrorPtr            - rc of prvTgfIpv6PrefixSet call when first error happen
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfTablesFillingLttIpv6FullLenSet
(
    IN  GT_U32 rowFirst,
    IN  GT_U32 rowLast,
    IN  GT_U32 numSkipped,
    OUT GT_U32 *numErrorsPtr,
    OUT GT_U32 *firstErrorPtr
)
{
    GT_STATUS rc        = GT_OK;
    GT_STATUS firstRc   = GT_OK;
    GT_U32    numErrors = 0;
    GT_U32    row       = 0;
    PRV_TGF_IPV6_PREFIX_STC ipv6Pref, ipv6Mask;

    PRV_UTF_LOG1_MAC("Set %d IPv6 prefixes with full length\n",
                     ((rowLast - rowFirst + 1) - numSkipped));

    /* set default IP address for the prefixes with full lengths */
    cpssOsMemSet(&ipv6Pref, 0, sizeof(ipv6Pref));

    /* fill IPv6 prefix with default DIP */
    rc = prvTgfTablesFillingLttIpToPtr(CPSS_IP_PROTOCOL_IPV6_E, prvTgfDefDipv4,
                                       prvTgfDefDipv6, ipv6Pref.ipAddr.arIP);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfTablesFillingLttIpToPtr");

    /* set default IP mask for the prefixes with full lengths */
    cpssOsMemSet(&ipv6Mask, 0, sizeof(ipv6Mask));
    cpssOsMemSet(ipv6Mask.ipAddr.arIP, 0xFF, sizeof(ipv6Mask.ipAddr.arIP));

    /* reset error counter */
    numErrors      = 0;
    *firstErrorPtr = 0;

    for (row = rowFirst; row <= rowLast; row++)
    {
        /* skip numSkipped prefixes for other action in future */
        if (numSkipped)
        {
            numSkipped--;
            continue;
        }

        /* print generated prefix */
        if (GT_TRUE == prvTgfEnablePrefixLog)
        {
            PRV_UTF_LOG1_MAC("row=%d: ", row);

            rc = prvTgfPrefixPrint(CPSS_IP_PROTOCOL_IPV6_E,
                                   ipv6Pref.ipAddr.arIP,
                                   ipv6Mask.ipAddr.arIP,
                                   sizeof(ipv6Pref.ipAddr.arIP) * 8);
            PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPrefixPrint");
        }

        /* set prefix in Router TCAM Table */
        rc = prvTgfIpv6PrefixSet(row, &ipv6Pref, &ipv6Mask);

        /* accumulate errors in numErrors */
        if (GT_OK != rc)
        {
            /* capture first error */
            if (0 == *firstErrorPtr)
            {
                *firstErrorPtr = row + 1;
                firstRc        = rc;
            }

            numErrors++;
        }

        /* increase ip */
        rc = prvTgfTablesFillingLttIpInc(CPSS_IP_PROTOCOL_IPV6_E, ipv6Pref.ipAddr.arIP);
        PRV_UTF_VERIFY_GT_OK(rc, "prvTgfTablesFillingLttIpInc");
    }

    *numErrorsPtr = numErrors;

    return firstRc;
}

/**
* @internal prvTgfTablesFillingLttIpv4VarLenSet function
* @endinternal
*
* @brief   Fill Router TCAM Table by UC IPv4 prefixes with various length in range
*
* @param[in] numVarPrefixes           - number of prefixes to set
*
* @param[out] numErrorsPtr             - number of errors caused with prvTgfIpv4PrefixSet call
* @param[out] firstErrorPtr            - rc of prvTgfIpv4PrefixSet call when first error happen
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfTablesFillingLttIpv4VarLenSet
(
    IN  GT_U32 numVarPrefixes,
    OUT GT_U32 *numErrorsPtr,
    OUT GT_U32 *firstErrorPtr
)
{
    GT_STATUS rc        = GT_OK;
    GT_STATUS firstRc   = GT_OK;
    GT_U32    numErrors = 0;
    GT_U32    row, col  = 0;
    PRV_TGF_IPV4_PREFIX_STC ipv4Pref, ipv4Mask;

    PRV_UTF_LOG1_MAC("Set %d IPv4 prefixes with various length\n", numVarPrefixes);

    /* set default IP address for the prefixes with full lengths */
    cpssOsMemSet(&ipv4Pref, 0, sizeof(ipv4Pref));

    rc = prvTgfTablesFillingLttIpToPtr(CPSS_IP_PROTOCOL_IPV4_E, prvTgfDefDipv4Various,
                                       prvTgfDefDipv6Various, ipv4Pref.ipAddr.arIP);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfTablesFillingLttIpToPtr");

    /* set default IP mask for the prefixes with variable lengths (start_lenth = 31) */
    cpssOsMemSet(&ipv4Mask, 0xFF, sizeof(ipv4Mask));
    ipv4Mask.ipAddr.arIP[3] = 254;

    /* reset error counter */
    numErrors      = 0;
    *firstErrorPtr = 0;

    /* iterate thru table */
    for (row = 0; numVarPrefixes; row++)
    {
        for (col = 0; col < 4 && numVarPrefixes; col++, numVarPrefixes--)
        {
            /* print generated prefix */
            if (GT_TRUE == prvTgfEnablePrefixLog)
            {
                PRV_UTF_LOG2_MAC("row=%d, col=%d: ", row, col);

                rc = prvTgfPrefixPrint(CPSS_IP_PROTOCOL_IPV4_E,
                                       ipv4Pref.ipAddr.arIP,
                                       ipv4Mask.ipAddr.arIP,
                                       sizeof(ipv4Pref.ipAddr.arIP) * 8);
                PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPrefixPrint");
            }

            /* set prefix in Router TCAM Table */
            rc = prvTgfIpv4PrefixSet(row, col, &ipv4Pref, &ipv4Mask);

            /* accumulate errors in numErrors */
            if (GT_OK != rc)
            {
                /* capture first error */
                if (0 == *firstErrorPtr)
                {
                    *firstErrorPtr = row + 1;
                    firstRc        = rc;
                }

                numErrors++;
            }

            /* shift prefix */
            rc = prvTgfTablesFillingLttIpShl(CPSS_IP_PROTOCOL_IPV4_E, ipv4Pref.ipAddr.arIP);
            PRV_UTF_VERIFY_GT_OK(rc, "prvTgfTablesFillingLttIpShl");

            /* shift mask */
            rc = prvTgfTablesFillingLttIpShl(CPSS_IP_PROTOCOL_IPV4_E, ipv4Mask.ipAddr.arIP);
            PRV_UTF_VERIFY_GT_OK(rc, "prvTgfTablesFillingLttIpShl");
        }
    }

    *numErrorsPtr = numErrors;

    return firstRc;
}

/**
* @internal prvTgfTablesFillingLttIpv6VarLenSet function
* @endinternal
*
* @brief   Fill Router TCAM Table by UC IPv6 prefixes with various length in range
*
* @param[out] numErrorsPtr             - number of errors caused with prvTgfIpv6PrefixSet call
* @param[out] firstErrorPtr            - rc of prvTgfIpv6PrefixSet call when first error happen
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfTablesFillingLttIpv6VarLenSet
(
    IN  GT_U32 numPrefixes,
    OUT GT_U32 *numErrorsPtr,
    OUT GT_U32 *firstErrorPtr
)
{
    GT_STATUS rc        = GT_OK;
    GT_STATUS firstRc   = GT_OK;
    GT_U32    numErrors = 0;
    GT_U32    row       = 0;
    PRV_TGF_IPV6_PREFIX_STC ipv6Pref, ipv6Mask;

    PRV_UTF_LOG1_MAC("Set %d IPv6 prefixes with various length\n", numPrefixes);

    /* set default IP address for the prefixes with full lengths */
    cpssOsMemSet(&ipv6Pref, 0, sizeof(ipv6Pref));

    /* set default destination */
    rc = prvTgfTablesFillingLttIpToPtr(CPSS_IP_PROTOCOL_IPV6_E, prvTgfDefDipv4Various,
                                       prvTgfDefDipv6Various, ipv6Pref.ipAddr.arIP);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfTablesFillingLttIpToPtr");

    /* set default IP mask for the prefixes with variable lengths (start_lenth = 127) */
    cpssOsMemSet(&ipv6Mask, 0, sizeof(ipv6Mask));
    cpssOsMemSet(ipv6Mask.ipAddr.arIP, 0xFF, sizeof(ipv6Mask.ipAddr.arIP));
    ipv6Mask.ipAddr.arIP[15] = 254;

    /* reset error counter */
    numErrors      = 0;
    *firstErrorPtr = 0;

    for (row = 0; numPrefixes; row++, numPrefixes--)
    {
        /* print generated prefix */
        if (GT_TRUE == prvTgfEnablePrefixLog)
        {
            PRV_UTF_LOG1_MAC("row=%d: ", row);

            rc = prvTgfPrefixPrint(CPSS_IP_PROTOCOL_IPV6_E,
                                   ipv6Pref.ipAddr.arIP,
                                   ipv6Mask.ipAddr.arIP,
                                   sizeof(ipv6Pref.ipAddr.arIP) * 8);
            PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPrefixPrint");
        }

        /* set prefix in Router TCAM Table */
        rc = prvTgfIpv6PrefixSet(row, &ipv6Pref, &ipv6Mask);

        /* accumulate errors in numErrors */
        if (GT_OK != rc)
        {
            /* capture first error */
            if (0 == *firstErrorPtr)
            {
                *firstErrorPtr = row + 1;
                firstRc        = rc;
            }

            numErrors++;
        }

        /* shift prefix */
        rc = prvTgfTablesFillingLttIpShl(CPSS_IP_PROTOCOL_IPV6_E, ipv6Pref.ipAddr.arIP);
        PRV_UTF_VERIFY_GT_OK(rc, "prvTgfTablesFillingLttIpShl");

        /* shift mask */
        rc = prvTgfTablesFillingLttIpShl(CPSS_IP_PROTOCOL_IPV6_E, ipv6Mask.ipAddr.arIP);
        PRV_UTF_VERIFY_GT_OK(rc, "prvTgfTablesFillingLttIpShl");
    }

    *numErrorsPtr = numErrors;

    return firstRc;
}

/**
* @internal prvTgfTablesFillingLttIpv4Invalidate function
* @endinternal
*
* @brief   Invalidate Router TCAM UC IPv4 prefix in all columns
*         for each row from rowFirst till rowLast
* @param[in] rowFirst                 - from this row
* @param[in] rowLast                  - to this row
*
* @param[out] numErrorsPtr             - number of errors caused with
*                                      prvTgfIpv4PrefixInvalidate call
* @param[out] firstErrorPtr            - rc of prvTgfIpv4PrefixInvalidate call
*                                      when first error happen
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfTablesFillingLttIpv4Invalidate
(
    IN  GT_U32 rowFirst,
    IN  GT_U32 rowLast,
    OUT GT_U32 *numErrorsPtr,
    OUT GT_U32 *firstErrorPtr
)
{
    GT_STATUS rc        = GT_OK;
    GT_STATUS firstRc   = GT_OK;
    GT_U32    numErrors = 0;
    GT_U32    row, col  = 0;

    /* reset error counter */
    numErrors      = 0;
    *firstErrorPtr = 0;

    /* iterate thru table */
    for (col = 0; col < 4; col++)
    {
        for (row = rowFirst; row <= rowLast; row++)
        {
            /* do not delete the default prefix in the last row, col */
            if (row == rowLast && col == 3)
            {
                break;
            }

            /* Invalidate prefix */
            rc = prvTgfIpv4PrefixInvalidate(row, col);

            /* accumulate errors in numErrors */
            if (GT_OK != rc)
            {
                /* capture first error */
                if (0 == *firstErrorPtr)
                {
                    *firstErrorPtr = row + 1;
                    firstRc        = rc;
                }

                numErrors++;
            }
        }
    }

    *numErrorsPtr = numErrors;

    return firstRc;
}

/**
* @internal prvTgfTablesFillingLttIpv6Invalidate function
* @endinternal
*
* @brief   Invalidate Router TCAM UC IPv6 prefix
*         for each row from rowFirst till rowLast
* @param[in] rowFirst                 - from this row
* @param[in] rowLast                  - to this row
*
* @param[out] numErrorsPtr             - number of errors caused with
*                                      prvTgfIpv6PrefixInvalidate call
* @param[out] firstErrorPtr            - rc of prvTgfIpv6PrefixInvalidate call
*                                      when first error happen
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfTablesFillingLttIpv6Invalidate
(
    IN  GT_U32 rowFirst,
    IN  GT_U32 rowLast,
    OUT GT_U32 *numErrorsPtr,
    OUT GT_U32 *firstErrorPtr
)
{
    GT_STATUS rc        = GT_OK;
    GT_STATUS firstRc   = GT_OK;
    GT_U32    numErrors = 0;
    GT_U32    row       = 0;

    /* reset error counter */
    numErrors      = 0;
    *firstErrorPtr = 0;

    /* do not delete the default prefix in the last row */
    for (row = rowFirst; row < rowLast; row++)
    {
        /* Invalidate prefix */
        rc = prvTgfIpv6PrefixInvalidate(row);

        /* accumulate errors in numErrors */
        if (GT_OK != rc)
        {
            /* capture first error */
            if (0 == *firstErrorPtr)
            {
                *firstErrorPtr = row + 1;
                firstRc        = rc;
            }

            numErrors++;
        }
    }

    *numErrorsPtr = numErrors;

    return firstRc;
}

/**
* @internal prvTgfTablesFillingLttFullLenSet function
* @endinternal
*
* @brief   Fill Router TCAM Table by UC prefixes with full length in range
*
* @param[in] protocol                 - type of IP stack used
* @param[in] rowFirst                 - from this row
* @param[in] rowLast                  - to this row
* @param[in] numSkipped               - number of skipped entries at the beginning of the range
*
* @param[out] numErrorsPtr             - number of errors
* @param[out] firstErrorPtr            - rc when first error happen
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfTablesFillingLttFullLenSet
(
    IN  CPSS_IP_PROTOCOL_STACK_ENT protocol,
    IN  GT_U32 rowFirst,
    IN  GT_U32 rowLast,
    IN  GT_U32 numSkipped,
    OUT GT_U32 *numErrorsPtr,
    OUT GT_U32 *firstErrorPtr
)
{
    GT_STATUS   rc = GT_OK;

    /* fill table depending on protocol type */
    switch (protocol)
    {
        case CPSS_IP_PROTOCOL_IPV4_E:
            /* Fill Router TCAM Table by IPv4 prefixes with full length */
            rc = prvTgfTablesFillingLttIpv4FullLenSet(rowFirst, rowLast, numSkipped,
                                                      numErrorsPtr, firstErrorPtr);
            PRV_UTF_VERIFY_GT_OK(rc, "prvTgfTablesFillingLttIpv4FullLenSet");

            break;

        case CPSS_IP_PROTOCOL_IPV6_E:
            /* Fill Router TCAM Table by IPv6 prefixes with full length */
            rc = prvTgfTablesFillingLttIpv6FullLenSet(rowFirst, rowLast, numSkipped,
                                                      numErrorsPtr, firstErrorPtr);
            PRV_UTF_VERIFY_GT_OK(rc, "prvTgfTablesFillingLttIpv6FullLenSet");

            break;

        default:
            return GT_BAD_PARAM;
    }

    return rc;
}

/**
* @internal prvTgfTablesFillingLttVarLenSet function
* @endinternal
*
* @brief   Fill Router TCAM Table by UC prefixes with various length in range
*
* @param[in] protocol                 - type of IP stack used
* @param[in] numVarPrefixes           - number of prefixes to set
*
* @param[out] numErrorsPtr             - number of errors
* @param[out] firstErrorPtr            - rc when first error happen
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfTablesFillingLttVarLenSet
(
    IN  CPSS_IP_PROTOCOL_STACK_ENT protocol,
    IN  GT_U32 numVarPrefixes,
    OUT GT_U32 *numErrorsPtr,
    OUT GT_U32 *firstErrorPtr
)
{
    GT_STATUS   rc = GT_OK;

    /* fill table depending on protocol type */
    switch (protocol)
    {
        case CPSS_IP_PROTOCOL_IPV4_E:
            /* Fill Router TCAM Table by IPv4 prefixes with variable length */
            rc = prvTgfTablesFillingLttIpv4VarLenSet(numVarPrefixes, numErrorsPtr,firstErrorPtr);
            PRV_UTF_VERIFY_GT_OK(rc, "prvTgfTablesFillingLttIpv4VarLenSet");

            break;

        case CPSS_IP_PROTOCOL_IPV6_E:
            /* Fill Router TCAM Table by IPv6 prefixes with variable length */
            rc = prvTgfTablesFillingLttIpv6VarLenSet(numVarPrefixes, numErrorsPtr, firstErrorPtr);
            PRV_UTF_VERIFY_GT_OK(rc, "prvTgfTablesFillingLttIpv6VarLenSet");

            break;

        default:
            return GT_BAD_PARAM;
    }

    return rc;
}

/**
* @internal prvTgfTablesFillingLttInvalidate function
* @endinternal
*
* @brief   Invalidate Router TCAM UC IPv4 prefix in all columns
*         for each row from rowFirst till rowLast
* @param[in] protocol                 - type of IP stack used
* @param[in] rowFirst                 - from this row
* @param[in] rowLast                  - to this row
*
* @param[out] numErrorsPtr             - number of errors
* @param[out] firstErrorPtr            - rc when first error happen
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfTablesFillingLttInvalidate
(
    IN  CPSS_IP_PROTOCOL_STACK_ENT protocol,
    IN  GT_U32 rowFirst,
    IN  GT_U32 rowLast,
    OUT GT_U32 *numErrorsPtr,
    OUT GT_U32 *firstErrorPtr
)
{
    GT_STATUS rc = GT_OK;

    /* invalidate prefixes depending on protocol type */
    switch (protocol)
    {
        case CPSS_IP_PROTOCOL_IPV4_E:
            /* Invalidate IPv4 prefixes */
            rc = prvTgfTablesFillingLttIpv4Invalidate(rowFirst, rowLast,
                                                      numErrorsPtr, firstErrorPtr);
            PRV_UTF_VERIFY_GT_OK(rc, "prvTgfTablesFillingLttIpv4Invalidate");

            break;

        case CPSS_IP_PROTOCOL_IPV6_E:
            /* Invalidate IPv4 prefixes */
            rc = prvTgfTablesFillingLttIpv6Invalidate(rowFirst, rowLast,
                                                      numErrorsPtr, firstErrorPtr);
            PRV_UTF_VERIFY_GT_OK(rc, "prvTgfTablesFillingLttIpv6Invalidate");

            break;

        default:
            return GT_BAD_PARAM;
    }

    return rc;
}

/**
* @internal prvTgfTablesFillingLttDefaultUcPrefixSet function
* @endinternal
*
* @brief   Set Default UC prefix in row, col position
*
* @param[in] col                      - column index
* @param[in] row                      -  index
* @param[in] col                      - column index
* @param[in] routeEntryBaseIndex      - the entry base index in Route Table
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfTablesFillingLttDefaultUcPrefixSet
(
    IN  CPSS_IP_PROTOCOL_STACK_ENT protocol,
    IN  GT_U32 row,
    IN  GT_U32 col,
    IN  GT_U32 routeEntryBaseIndex
)
{
    GT_STATUS                   rc = GT_OK;
    PRV_TGF_IP_LTT_ENTRY_STC    lttEntry = {0};
    PRV_TGF_IPV4_PREFIX_STC     ipv4Pref;
    PRV_TGF_IPV4_PREFIX_STC     ipv4Mask;
    PRV_TGF_IPV6_PREFIX_STC     ipv6Pref;
    PRV_TGF_IPV6_PREFIX_STC     ipv6Mask;

    /* set default parameters */
    cpssOsMemSet(&ipv4Pref, 0, sizeof(ipv4Pref));
    cpssOsMemSet(&ipv4Mask, 0, sizeof(ipv4Mask));
    cpssOsMemSet(&ipv6Pref, 0, sizeof(ipv6Pref));
    cpssOsMemSet(&ipv6Mask, 0, sizeof(ipv6Mask));

    /* set default IPv4\IPv6 UC prefixes */
    switch (protocol)
    {
        case CPSS_IP_PROTOCOL_IPV4_E:
            PRV_UTF_LOG2_MAC("Mark 1 IPv4 UC prefix as default in row = %d, col = %d\n",
                             row, col);

            /* set UC IPv4 default prefix */
            ipv4Mask.vrId                = 0x0FFF;
            ipv4Mask.isMcSource          = GT_TRUE;
            ipv4Mask.mcGroupIndexRow     = 0x1FFF;
            ipv4Mask.mcGroupIndexColumn  = 3;
            lttEntry.routeEntryBaseIndex = routeEntryBaseIndex;
            lttEntry.routeType = PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_MAC;

            rc = prvTgfIpv4PrefixSet(row, col, &ipv4Pref, &ipv4Mask);
            PRV_UTF_VERIFY_GT_OK(rc, "prvTgfIpv4PrefixSet");

            rc = prvTgfIpLttWrite(row, col, &lttEntry);
            PRV_UTF_VERIFY_GT_OK(rc, "prvTgfIpLttWrite");

            break;

        case CPSS_IP_PROTOCOL_IPV6_E:
            PRV_UTF_LOG1_MAC("Mark 1 IPv6 UC prefix as default in row = %d\n", row);

            /* set UC IPv6 default prefix */
            ipv6Mask.vrId                = 0x0FFF;
            ipv6Mask.isMcSource          = GT_TRUE;
            ipv6Mask.mcGroupIndexRow     = 0x0FFF;
            lttEntry.routeEntryBaseIndex = routeEntryBaseIndex;
            lttEntry.routeType = PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_MAC;

            rc = prvTgfIpv6PrefixSet(row, &ipv6Pref, &ipv6Mask);
            PRV_UTF_VERIFY_GT_OK(rc, "prvTgfIpv6PrefixSet");

            rc = prvTgfIpLttWrite(row, col, &lttEntry);
            PRV_UTF_VERIFY_GT_OK(rc, "prvTgfIpLttWrite");

            break;

        default:
            return GT_BAD_PARAM;
    }

    return rc;
}

/**
* @internal prvTgfTablesFillingLttDefaultMcPrefixSet function
* @endinternal
*
* @brief   Set Default MC prefix in row, col position
*
* @param[in] col                      - column index
* @param[in] row                      -  index
* @param[in] col                      - column index
* @param[in] routeEntryBaseIndex      - the entry base index in Route Table
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfTablesFillingLttDefaultMcPrefixSet
(
    IN  CPSS_IP_PROTOCOL_STACK_ENT protocol,
    IN  GT_U32 row,
    IN  GT_U32 col,
    IN  GT_U32 routeEntryBaseIndex
)
{
    GT_STATUS                   rc = GT_OK;
    PRV_TGF_IP_LTT_ENTRY_STC    lttEntry = {0};
    PRV_TGF_IPV4_PREFIX_STC     ipv4Pref;
    PRV_TGF_IPV4_PREFIX_STC     ipv4Mask;
    PRV_TGF_IPV6_PREFIX_STC     ipv6Pref;
    PRV_TGF_IPV6_PREFIX_STC     ipv6Mask;

    /* set default parameters */
    cpssOsMemSet(&ipv4Pref, 0, sizeof(ipv4Pref));
    cpssOsMemSet(&ipv4Mask, 0, sizeof(ipv4Mask));
    cpssOsMemSet(&ipv6Pref, 0, sizeof(ipv6Pref));
    cpssOsMemSet(&ipv6Mask, 0, sizeof(ipv6Mask));

    /* set default IPv4\IPv6 MC prefixes */
    switch (protocol)
    {
        case CPSS_IP_PROTOCOL_IPV4_E:
            PRV_UTF_LOG2_MAC("Mark 1 IPv4 MC prefix as default in row = %d, col = %d\n",
                             row, col);

            /* set UC IPv4 default prefix */
            ipv4Mask.vrId                = 0x0FFF;
            ipv4Mask.isMcSource          = GT_TRUE;
            ipv4Mask.mcGroupIndexRow     = 0x1FFF;
            ipv4Mask.mcGroupIndexColumn  = 3;
            lttEntry.routeEntryBaseIndex = routeEntryBaseIndex;
            lttEntry.routeType = PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_MAC;
            ipv4Pref.ipAddr.arIP[0] = 224;
            ipv4Mask.ipAddr.arIP[0] = 0xF0;

            rc = prvTgfIpv4PrefixSet(row, col, &ipv4Pref, &ipv4Mask);
            PRV_UTF_VERIFY_GT_OK(rc, "prvTgfIpv4PrefixSet");

            rc = prvTgfIpLttWrite(row, col, &lttEntry);
            PRV_UTF_VERIFY_GT_OK(rc, "prvTgfIpLttWrite");

            break;

        case CPSS_IP_PROTOCOL_IPV6_E:
            PRV_UTF_LOG1_MAC("Mark 1 IPv6 UC prefix as default in row = %d\n", row);

            /* set UC IPv6 default prefix */
            ipv6Mask.vrId                = 0x0FFF;
            ipv6Mask.isMcSource          = GT_TRUE;
            ipv6Mask.mcGroupIndexRow     = 0x0FFF;
            lttEntry.routeEntryBaseIndex = routeEntryBaseIndex;
            lttEntry.routeType = PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_MAC;
            ipv6Pref.ipAddr.arIP[0] = 255;
            ipv6Mask.ipAddr.arIP[0] = 0xFF;

            rc = prvTgfIpv6PrefixSet(row, &ipv6Pref, &ipv6Mask);
            PRV_UTF_VERIFY_GT_OK(rc, "prvTgfIpv6PrefixSet");

            rc = prvTgfIpLttWrite(row, col, &lttEntry);
            PRV_UTF_VERIFY_GT_OK(rc, "prvTgfIpLttWrite");

            break;

        default:
            return GT_BAD_PARAM;
    }

    return rc;
}

/**
* @internal prvTgfTablesFillingLttBaseTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*
* @param[in] protocol                 - type of IP stack used
*                                      numEntries       - general number of prefixes to check
* @param[in] numVarPrefixes           - number of prefixes with variable length
* @param[in] burstCount               - number of packets to transmit in 1 burst
* @param[in] numRoutedPackets         - expected number of packets in capturing table
*                                       None
*/
static GT_VOID prvTgfTablesFillingLttBaseTrafficGenerate
(
    IN CPSS_IP_PROTOCOL_STACK_ENT protocol,
    IN GT_U32                     numPrefixes,
    IN GT_U32                     numVarPrefixes,
    IN GT_U32                     burstCount,
    IN GT_U32                     numRoutedPackets
)
{
    GT_STATUS               rc          = GT_OK;
    GT_U32                  burstIter   = 0;
    GT_U32                  packetIter  = 0;
    GT_U32                  numPackets  = 0;
    TGF_VFD_INFO_STC        vfdArray[1] = {{0}};
    TGF_PACKET_STC          *packetInfoPtr;
    CPSS_INTERFACE_INFO_STC portInterface;
    GT_U32                  timeActuallyWait;/*time waited for all packets to get to the CPU*/
    GT_U32                  numPacketsToCpu;/*number of packets to the CPU*/


    PRV_UTF_LOG1_MAC("\nEach dot corresponds to %d successfully checked prefixes.", burstCount);

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* setup nexthope portInterface for capturing */
    portInterface.type            = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = prvTgfDevNum;
    portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_RX_1_IDX_CNS];

    /* set mode to get new IP address */
    vfdArray[0].mode = (PRV_TGF_NEXT_IP_MODE_INC_E == prvTgfNextIpMode) ?
                        TGF_VFD_MODE_INCREMENT_E : TGF_VFD_MODE_STATIC_E;

    /* set vfdArray for incrementing dip */
    if (CPSS_IP_PROTOCOL_IPV6_E == protocol)
    {
        packetInfoPtr          = &prvTgfPacketIpv6Info;
        vfdArray[0].cycleCount = sizeof(prvTgfPacketIpv6Part.dstAddr);
        vfdArray[0].offset     = TGF_L2_HEADER_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS +
                                 TGF_IPV6_HEADER_SIZE_CNS - sizeof(prvTgfPacketIpv6Part.dstAddr);
    }
    else
    {
        packetInfoPtr          = &prvTgfPacketInfo;
        vfdArray[0].cycleCount = sizeof(prvTgfPacketIpv4Part.dstAddr);
        vfdArray[0].offset     = TGF_L2_HEADER_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS +
                                 TGF_IPV4_HEADER_SIZE_CNS - sizeof(prvTgfPacketIpv4Part.dstAddr);
    }

    /* setup transmit parameters */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, packetInfoPtr, burstCount, 1, vfdArray);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfSetTxSetupEth: %d", prvTgfDevNum);

    /* reserve space in the end of LTT for numVarPrefixes prefixes */
    numPackets = (PRV_TGF_NEXT_IP_MODE_INC_E == prvTgfNextIpMode) ?
                 numPrefixes - numVarPrefixes + prvTgfWrongPackets :
                 numVarPrefixes;

    for (packetIter = 0; packetIter < numPackets; packetIter += burstCount)
    {
        /* fix the last burstCount for transmitting */
        if (packetIter + burstCount > numPackets)
        {
            burstCount = numPackets - packetIter;

            rc = prvTgfSetTxSetupEth(prvTgfDevNum, packetInfoPtr, burstCount, 1, vfdArray);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfSetTxSetupEth: %d", prvTgfDevNum);
        }

        /* progress indicator - new line per each 40 bursts */
        if (0 == (packetIter % (40 * burstCount)))
        {
            PRV_UTF_LOG2_MAC("\nChecked prefix %5d from %d:", packetIter, numPackets);
        }

        /* progress indicator - space per each 10 bursts */
        if (0 == (packetIter % (10 * burstCount)))
        {
            PRV_UTF_LOG0_MAC(" ");
        }

        /* progress indicator - dot per each 1 burst */
        PRV_UTF_LOG0_MAC(".");

        /* set current IP from the ipPtr to the packet */
        rc = prvTgfTablesFillingLttIpFromPtr(protocol,
                                             prvTgfPacketIpv4Part.dstAddr,
                                             prvTgfPacketIpv6Part.dstAddr,
                                             prvTgfIpCur);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,"prvTgfTablesFillingLttIpFromPtr");

        /* copy current IP to the vfdArray */
        rc = prvTgfTablesFillingLttIpToPtr(protocol,
                                           prvTgfPacketIpv4Part.dstAddr,
                                           prvTgfPacketIpv6Part.dstAddr,
                                           vfdArray[0].patternPtr);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,"prvTgfTablesFillingLttIpToPtr");

        /* enable capture on nexthope port */
        rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface,
                                                    TGF_CAPTURE_MODE_MIRRORING_E,
                                                    GT_TRUE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, %d\n",
               portInterface.devPort.hwDevNum, portInterface.devPort.portNum);

        /* disable packet trace */
        rc = tgfTrafficTracePacketByteSet(GT_FALSE);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTracePacketByteSet");

        /* transmit burstCount packets */
        rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_TX_0_IDX_CNS]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfStartTransmitingEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[PRV_TGF_TX_0_IDX_CNS]);

        /* enable packet trace */
        rc = tgfTrafficTracePacketByteSet(GT_TRUE);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTracePacketByteSet");

        /* allow the appdemo task to process the last captured packets */
        (void)tgfTrafficGeneratorCaptureLoopTimeSet(200);

        if (packetIter + burstCount <= numRoutedPackets)
        {
            numPacketsToCpu = burstCount;
        }
        else if (packetIter < numRoutedPackets)
        {
            numPacketsToCpu = numRoutedPackets - packetIter;
        }
        else
        {
            numPacketsToCpu = 0;
        }

        if(numPacketsToCpu)
        {
            rc = tgfTrafficGeneratorRxInCpuNumWait(numPacketsToCpu,1000,&timeActuallyWait);
            if(rc == GT_OK)
            {
                /*don't let the function tgfTrafficGeneratorPortTxEthCaptureSet(...) do any extra sleep */
                (void)tgfTrafficGeneratorCaptureLoopTimeSet(1);

                if(numPacketsToCpu > 1 && prvTgfTrafficPrintPacketTxEnableGet() == GT_TRUE)
                {
                    /*cpssOsPrintf*/
                    PRV_UTF_LOG2_MAC
                        ("[TGF]: prvTgfTablesFillingLttBaseTrafficGenerate [%d] packets to CPU in [%d] milliseconds \n", numPacketsToCpu,timeActuallyWait);
                }
            }
            else
            {
                /*cpssOsPrintf*/
                PRV_UTF_LOG1_MAC
                    ("[TGF]: tgfTrafficGeneratorRxInCpuNumWait : failed with rc [%d] \n", rc);
            }
        }

        /* disable capture on nexthope port */
        rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface,
                                                    TGF_CAPTURE_MODE_MIRRORING_E,
                                                    GT_FALSE);
        /* restore value */
        (void)tgfTrafficGeneratorCaptureLoopTimeSet(0);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorPortTxEthCaptureSet: %d, %d\n",
                                     portInterface.devPort.hwDevNum, portInterface.devPort.portNum);

        if (packetIter + burstCount <= numRoutedPackets)
        {
            /* scan whole captured table and check burstCount expected packet */
            prvTgfTablesFillingLttCapturedTableCheck(protocol, packetIter,
                                                     &portInterface, burstCount,
                                                     &vfdArray[0]);
        }
        else if (packetIter < numRoutedPackets)
        {
            PRV_UTF_LOG1_MAC("\nCheck last %d routed packets: ", numRoutedPackets - packetIter);

            /* prvTgfWrongPackets last packets are wrong (non routed) packets.
             * There are no IP prefixes for them. They not appear in captured table.
             * fix burstCount for them.
             */
            prvTgfTablesFillingLttCapturedTableCheck(protocol, packetIter,
                    &portInterface, numRoutedPackets - packetIter, &vfdArray[0]);
        }

        /* clear capturing RxPcktTable */
        rc = tgfTrafficTableRxPcktTblClear();
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

        /* get next address */
        for (burstIter = 0; burstIter < burstCount; burstIter++)
        {
            rc = prvTgfTablesFillingLttNextAddrGet(protocol, prvTgfIpCur);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTablesFillingLttNextAddrGet\n");
        }
    }

    PRV_UTF_LOG0_MAC("\n");
}

/**
* @internal prvTgfTablesFillingLttCheckCounters function
* @endinternal
*
* @brief   Generate traffic
*
* @param[in] numEntries               - general number of prefixes to check
* @param[in] numExpRoutedPackets      - expected number of routed packets
* @param[in] numExpNonRoutedPackets   - expected number of non routed packets
*                                       None
*/
static GT_VOID prvTgfTablesFillingLttCheckCounters
(
    IN GT_U32 numEntries,
    IN GT_U32 numExpRoutedPackets,
    IN GT_U32 numExpNonRoutedPackets
)
{
    GT_STATUS rc = GT_OK;
    GT_U32    portIdx;
    GT_U32    numExpectTx[PRV_TGF_PORT_COUNT_CNS] = {0};
    GT_U32    numExpectRx[PRV_TGF_PORT_COUNT_CNS] = {0};
    CPSS_PORT_MAC_COUNTER_SET_STC portCntrs[PRV_TGF_PORT_COUNT_CNS];
    PRV_TGF_IP_COUNTER_SET_STC    ipCounters;

    /*--------------------------------------------------------------------------
     * read Eth counters for all receive ports
     */
    for (portIdx = 0; portIdx < PRV_TGF_PORT_COUNT_CNS; portIdx++)
    {
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIdx],
                                       GT_FALSE, &portCntrs[portIdx]);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth");
    }

    /* set number of expected packets */
    numExpectRx[0] = numEntries + prvTgfWrongPackets;
    numExpectTx[0] = numEntries + prvTgfWrongPackets;
    numExpectRx[1] = numExpRoutedPackets;
    numExpectTx[1] = numExpRoutedPackets;

    /* compare expected and received results */
    rc = (portCntrs[0].goodPktsRcv.l[0]  == numExpectRx[0]) &&
         (portCntrs[1].goodPktsRcv.l[0]  == numExpectRx[1]) &&
         (portCntrs[2].goodPktsRcv.l[0]  == numExpectRx[2]) &&
         (portCntrs[3].goodPktsRcv.l[0]  == numExpectRx[3]) &&
         (portCntrs[0].goodPktsSent.l[0] == numExpectTx[0]) &&
         (portCntrs[1].goodPktsSent.l[0] == numExpectTx[1]) &&
         (portCntrs[2].goodPktsSent.l[0] == numExpectTx[2]) &&
         (portCntrs[3].goodPktsSent.l[0] == numExpectTx[3]) ? GT_OK : GT_FAIL;

    /* print number of received packets */
    PRV_UTF_LOG4_MAC("Eth counters on ports [%5d, %5d, %5d, %5d]\n",
                     prvTgfPortsArray[0], prvTgfPortsArray[1],
                     prvTgfPortsArray[2], prvTgfPortsArray[3]);
    PRV_UTF_LOG4_MAC("     respectively: Rx [%5d, %5d, %5d, %5d]\n",
                     portCntrs[0].goodPktsRcv.l[0],
                     portCntrs[1].goodPktsRcv.l[0],
                     portCntrs[2].goodPktsRcv.l[0],
                     portCntrs[3].goodPktsRcv.l[0]);
    PRV_UTF_LOG4_MAC("                   Tx [%5d, %5d, %5d, %5d]\n\n",
                     portCntrs[0].goodPktsSent.l[0],
                     portCntrs[1].goodPktsSent.l[0],
                     portCntrs[2].goodPktsSent.l[0],
                     portCntrs[3].goodPktsSent.l[0]);

    /* print expected values if error */
    if (GT_OK != rc)
    {
        PRV_UTF_LOG4_MAC(" expected packets: Rx [%5d, %5d, %5d, %5d]\n",
                         numExpectRx[0], numExpectRx[1], numExpectRx[2], numExpectRx[3]);
        PRV_UTF_LOG4_MAC("                   Tx [%5d, %5d, %5d, %5d]\n\n",
                         numExpectTx[0], numExpectTx[1], numExpectTx[2], numExpectTx[3]);
    }

    /* check Eth counters */
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "got other number of Eth packets than expected");

    /* get and print ip counters values for TX port */
    PRV_UTF_LOG1_MAC("IP counters on port [%d]:\n", prvTgfPortsArray[PRV_TGF_TX_0_IDX_CNS]);
    rc = prvTgfCountersIpGet(prvTgfDevNum, PRV_TGF_TX_0_IDX_CNS,
                             GT_TRUE, &ipCounters);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCountersIpGet");

    /* check ip counters */
    UTF_VERIFY_EQUAL0_STRING_MAC(numExpNonRoutedPackets, ipCounters.inUcTrappedMirrorPkts,
                                 "got other number of inUcTrappedMirrorPkts than expected");
}

/******************************************************************************\
 *                            Public test functions                           *
\******************************************************************************/

/**
* @internal prvTgfTablesFillingLttConfigurationSet function
* @endinternal
*
* @brief   Set common configuration
*/
GT_VOID prvTgfTablesFillingLttConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS                     rc = GT_OK;
    PRV_TGF_BRG_MAC_ENTRY_STC     macEntry;
    PRV_TGF_IP_UC_ROUTE_ENTRY_STC routeEntriesArray[1];
    GT_ETHERADDR                  arpMacAddr;
    GT_IPADDR                     ipv4Addr;
    GT_IPV6ADDR                   ipv6Addr;
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT nextHopInfo;
    GT_U32                        portIter;

    /* create SEND VLAN 5 on ports (0) */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_SEND_VLANID_CNS,
                                           prvTgfPortsArray, NULL, NULL, 1);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWithPortsSet: %d",
                                 prvTgfDevNum);

    /* create NEXTHOPE VLAN 6 on ports (8,18,23) */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_NEXTHOPE_VLANID_CNS,
                                           prvTgfPortsArray + 1, NULL, NULL, 3);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWithPortsSet: %d",
                                 prvTgfDevNum);

    /* create a static macEntry in SEND VLAN with .daRoute = GT_TRUE */
    cpssOsMemSet(&macEntry, 0, sizeof(macEntry));
    cpssOsMemCpy(macEntry.key.key.macVlan.macAddr.arEther,
                 prvTgfPacketL2Part.daMac, sizeof(TGF_MAC_ADDR));

    macEntry.key.entryType                = PRV_TGF_FDB_ENTRY_TYPE_MAC_ADDR_E;
    macEntry.key.key.macVlan.vlanId       = PRV_TGF_SEND_VLANID_CNS;
    macEntry.dstInterface.type            = CPSS_INTERFACE_PORT_E;
    macEntry.dstInterface.devPort.hwDevNum  = prvTgfDevNum;
    macEntry.dstInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_TX_0_IDX_CNS];
    macEntry.isStatic                     = GT_TRUE;
    macEntry.daRoute                      = GT_TRUE;

    rc = prvTgfBrgFdbMacEntrySet(&macEntry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbMacEntrySet: %d", prvTgfDevNum);

    /* save default vlanId for restore */
    rc = prvTgfBrgVlanPortVidGet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_TX_0_IDX_CNS], &prvTgfRestoreCfg.vid);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidGet: %d, %d, %d", prvTgfDevNum,
                                 prvTgfPortsArray[PRV_TGF_TX_0_IDX_CNS], &prvTgfRestoreCfg.vid);

    /* set default vlanId */
    rc = prvTgfBrgVlanPortVidSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_TX_0_IDX_CNS],
                                 PRV_TGF_SEND_VLANID_CNS);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidSet: %d, %d, %d", prvTgfDevNum,
                                 prvTgfPortsArray[PRV_TGF_TX_0_IDX_CNS], PRV_TGF_SEND_VLANID_CNS);

    /* enable IPv4 and IPv6 Unicast Routing on a Port */
    rc = prvTgfIpPortRoutingEnable(PRV_TGF_TX_0_IDX_CNS,
                                   CPSS_IP_UNICAST_E, CPSS_IP_PROTOCOL_IPV4V6_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpPortRoutingEnable: %d %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_TX_0_IDX_CNS]);

    /* enable IPv4 and IPv6 Unicast Routing on Vlan */
    rc = prvTgfIpVlanRoutingEnable(PRV_TGF_SEND_VLANID_CNS, CPSS_IP_UNICAST_E,
                                   CPSS_IP_PROTOCOL_IPV4V6_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpVlanRoutingEnable: %d %d",
                                 prvTgfDevNum, PRV_TGF_SEND_VLANID_CNS);

    /* create a ARP MAC address in the Router ARP Table */
    cpssOsMemCpy(arpMacAddr.arEther, prvTgfArpMac, sizeof(TGF_MAC_ADDR));

    rc = prvTgfIpRouterArpAddrWrite(prvTgfArpEntryBaseIndex, &arpMacAddr);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpRouterArpAddrWrite: %d", prvTgfDevNum);

    /* create UC Route entry in the Route Table */
    cpssOsMemSet(routeEntriesArray, 0, sizeof(routeEntriesArray));
    routeEntriesArray[0].nextHopARPPointer     = prvTgfArpEntryBaseIndex;
    routeEntriesArray[0].cmd                   = CPSS_PACKET_CMD_ROUTE_E;
    routeEntriesArray[0].nextHopVlanId         = PRV_TGF_NEXTHOPE_VLANID_CNS;
    routeEntriesArray[0].nextHopInterface.type = CPSS_INTERFACE_PORT_E;
    routeEntriesArray[0].nextHopInterface.devPort.hwDevNum  = prvTgfDevNum;
    routeEntriesArray[0].nextHopInterface.devPort.portNum =
                        prvTgfPortsArray[PRV_TGF_RX_1_IDX_CNS];

    rc = prvTgfIpUcRouteEntriesWrite(prvTgfRouteEntryBaseIndex, routeEntriesArray, 1);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpUcRouteEntriesWrite: %d", prvTgfDevNum);

    /* save all default prefixes to restore */
    cpssOsMemSet(ipv4Addr.arIP, 0, sizeof(ipv4Addr.arIP));
    cpssOsMemSet(ipv6Addr.arIP, 0, sizeof(ipv6Addr.arIP));

    /* save default IPv4 UC prefix */
    rc = prvTgfIpLpmIpv4UcPrefixSearch(prvTgfLpmDBId, 0, ipv4Addr, 0, &nextHopInfo,
                                       &prvTgfRestoreCfg.rowUcIpv4,
                                       &prvTgfRestoreCfg.colUcIpv4);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4UcPrefixSearch");

    prvTgfRestoreCfg.routeDefUcIdx = nextHopInfo.ipLttEntry.routeEntryBaseIndex;

    /* save default IPv4 MC prefix */
    rc = prvTgfIpLpmIpv4McEntrySearch(prvTgfLpmDBId, 0, ipv4Addr, 0, ipv4Addr, 0,
                                      &nextHopInfo.ipLttEntry,
                                      &prvTgfRestoreCfg.rowMcGrpIpv4,
                                      &prvTgfRestoreCfg.colMcGrpIpv4,
                                      &prvTgfRestoreCfg.rowMcSrcIpv4,
                                      &prvTgfRestoreCfg.colMcSrcIpv4);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4McEntrySearch");

    prvTgfRestoreCfg.routeDefMcIdx = nextHopInfo.ipLttEntry.routeEntryBaseIndex;

    /* save default IPv6 UC prefix */
    rc = prvTgfIpLpmIpv6UcPrefixSearch(prvTgfLpmDBId, 0, ipv6Addr, 0, &nextHopInfo,
                                       &prvTgfRestoreCfg.rowUcIpv6,
                                       &prvTgfRestoreCfg.colUcIpv6);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv6UcPrefixSearch");

    /* save default IPv6 MC prefix */
    rc = prvTgfIpLpmIpv6McEntrySearch(prvTgfLpmDBId, 0, ipv6Addr, 0, ipv6Addr, 0,
                                      &nextHopInfo.ipLttEntry,
                                      &prvTgfRestoreCfg.rowMcGrpIpv6,
                                      &prvTgfRestoreCfg.colMcSrcIpv6);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4McEntrySearch");

    /* disable Flow Control to avoid pause frames in xCat, xCat3  */
    if(GT_FALSE != prvUtfDeviceTestNotSupport(prvTgfDevNum, UTF_LION_E | UTF_LION2_E | UTF_CPSS_PP_E_ARCH_CNS| UTF_PUMA_E))
    {
        for (portIter = 0; portIter < prvTgfPortsNum ; portIter++)
        {
            /* reset ethernet counters */
            rc = prvTgfPortFlowControlEnableSet(prvTgfDevNum,prvTgfPortsArray[portIter],CPSS_PORT_FLOW_CONTROL_DISABLE_E);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortFlowControlEnableSet");
        }
    }

    /* show where the default prefixes are located */
    PRV_UTF_LOG0_MAC("===> Default prefixes are located at [row, col]:\n");
    PRV_UTF_LOG3_MAC("     Uc IPv4:    [%d, %d] routeDefUcIdx = %d\n",
                     prvTgfRestoreCfg.rowUcIpv4,
                     prvTgfRestoreCfg.colUcIpv4,
                     prvTgfRestoreCfg.routeDefUcIdx);
    PRV_UTF_LOG5_MAC("     Mc IPv4 grp:[%d, %d] src:[%d, %d] routeDefMcIdx = %d\n",
                     prvTgfRestoreCfg.rowMcGrpIpv4,
                     prvTgfRestoreCfg.colMcGrpIpv4,
                     prvTgfRestoreCfg.rowMcSrcIpv4,
                     prvTgfRestoreCfg.colMcSrcIpv4,
                     prvTgfRestoreCfg.routeDefMcIdx);
    PRV_UTF_LOG2_MAC("     Uc IPv6:    [%d, %d]\n",
                     prvTgfRestoreCfg.rowUcIpv6,
                     prvTgfRestoreCfg.colUcIpv6);
    PRV_UTF_LOG2_MAC("     Mc IPv6 grp:[%d] src:[%d]\n\n",
                     prvTgfRestoreCfg.rowMcGrpIpv6,
                     prvTgfRestoreCfg.colMcSrcIpv6);
}

/**
* @internal prvTgfTablesFillingLttTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*
* @param[in] protocol                 - type of IP stack used
* @param[in] numLoops                 - number of loops of the test to discover
*                                      a memory leak and so on
*                                       None
*/
GT_VOID prvTgfTablesFillingLttTrafficGenerate
(
    IN CPSS_IP_PROTOCOL_STACK_ENT protocol,
    IN GT_U32                     numLoops
)
{
    GT_STATUS rc             = GT_OK;
    GT_U32    numEntries     = 0;
    GT_U32    numPrefixes    = 0;
    GT_U32    numVarPrefixes = 0;
    GT_U32    numEntriesLtt  = 0;
    GT_U32    numErrors      = 0;
    GT_U32    firstError     = 0;
    GT_U32    colLast        = 0;
    GT_U32    burstCount     = 100;

#ifndef ASIC_SIMULATION
#ifdef CHX_FAMILY
    /* decrease burst count to be 25 for MII/RGMII systems.
       25 is number of buffers in the BSP/LSP for MII/RMII port.
       test captures traffic to CPU. So some packets in the burst with more
       then 25 packets may be lost  */
    GT_BOOL useMii;

    useMii =((PRV_CPSS_PP_MAC(prvTgfCpuDevNum)->cpuPortMode == CPSS_NET_CPU_PORT_MODE_MII_E) ? GT_TRUE:
             (PRV_CPSS_PP_MAC(prvTgfCpuDevNum)->cpuPortMode == CPSS_NET_CPU_PORT_MODE_NONE_E) ? GT_FALSE:
             PRV_CPSS_DXCH_CHECK_SDMA_PACKETS_FROM_CPU_CORRUPT_WA_MAC(prvTgfCpuDevNum) ? GT_TRUE : GT_FALSE);

    if (useMii == GT_TRUE)
    {
        burstCount = 25;
    }
#endif /* CHX_FAMILY */
#endif /* ASIC_SIMULATION */

    /* get number of entries of Lookup Translation Table */
    rc = prvUtfCfgTableNumEntriesGet(prvTgfDevNum, PRV_TGF_CFG_TABLE_ROUTER_LTT_E, &numEntriesLtt);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvUtfCfgTableNumEntriesGet");

    /* get number of entries of Router TCAM Table */
    rc = prvUtfCfgTableNumEntriesGet(prvTgfDevNum, PRV_TGF_CFG_TABLE_ROUTER_TCAM_E, &numEntries);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvUtfCfgTableNumEntriesGet");

    /* how many IPv4 and IPv6 prefixes is allowed to add to LPM DB */
    PRV_UTF_LOG1_MAC("===> Router TCAM Table (ROUTER_TCAM)"
                     " has %d rows and 4 columns\n", numEntries);

    /* in case if LPM and LTT tables have different limits
     * we impose constraint on the test
     * the test use only min value
     */
    numEntries = (numEntriesLtt < numEntries) ? numEntriesLtt : numEntries;

    /* how much IPv4 and IPv6 prefixes is enabled to add to TCAM Table */
    PRV_UTF_LOG3_MAC("===> Lookup Translation Table (ROUTER_LTT) has %d entries\n"
                     "     It can hold %d IPv4 or %d IPv6 prefixes\n",
                     numEntries, (numEntries * 4), numEntries);

    /* set parameters for the test */
    switch (protocol)
    {
        case CPSS_IP_PROTOCOL_IPV4_E:
            /* set parameters for the IPv4 test */
            numVarPrefixes = 30;
            numPrefixes = numEntries * 4;
            colLast = 3;

            break;
        case CPSS_IP_PROTOCOL_IPV6_E:
            /* set parameters for the IPv6 test */
            numVarPrefixes = 126;
            numPrefixes = numEntries;
            colLast = 0;

            break;
        default:
            numLoops = 0;
            rc = GT_BAD_PARAM;
    }

    /* checking that the tables have enough entries to run whole algorithm */
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_TRUE, numPrefixes > numVarPrefixes,
            "\n\n\n!!! TEST ERROR !!!\n    This is not enough entries"
            " in the tables to apply the test's algorithm.\n");
    if (numPrefixes <= numVarPrefixes)
    {
        return;
    }

    /* numPrefixes - 1 because the default prefix must exist also */
    numPrefixes--;

    PRV_UTF_LOG1_MAC("\n===> Loop %d:\n", numLoops);

    /* check LPM prefixes */
    while (numLoops--)
    {
        /*----------------------------------------------------------------------
         * 1. create prefixes
         */

        /* Fill Router LTT by entries pointed to the same route entry */
        rc = prvTgfTablesFillingLttWrite(0, numEntries - 1, 0, colLast,
                                         prvTgfRouteEntryBaseIndex, &numErrors, &firstError);
        UTF_VERIFY_EQUAL4_STRING_MAC(0, numErrors,
                "prvTgfTablesFillingLttWrite"
                "\n       could successfully write only %d entries !!!"
                "\n       It has not handled %d entries."
                "\n       First error was happened on writing of %d entry"
                "\n       rc = %d %s",
                (numPrefixes - numErrors), numErrors, firstError, rc);

        /* Fill Router TCAM Table by prefixes with full length */
        rc = prvTgfTablesFillingLttFullLenSet(protocol, 0, numEntries - 1,
                                              numVarPrefixes, &numErrors, &firstError);
        UTF_VERIFY_EQUAL4_STRING_MAC(0, numErrors,
                "prvTgfTablesFillingLttFullLenSet"
                "\n       could successfully set only %d prefixes !!!"
                "\n       It has not handled %d prefixes."
                "\n       First error was happened on setting of %d prefix"
                "\n       rc = %d %s",
                (numPrefixes - numVarPrefixes - numErrors), numErrors, firstError, rc);

        /* Fill Router TCAM Table by prefixes with various length */
        rc = prvTgfTablesFillingLttVarLenSet(protocol, numVarPrefixes, &numErrors, &firstError);
        UTF_VERIFY_EQUAL4_STRING_MAC(0, numErrors,
                "prvTgfTablesFillingLttVarLenSet"
                "\n       could successfully set only %d prefixes !!!"
                "\n       It has not handled %d prefixes."
                "\n       First error was happened on setting of %d prefix"
                "\n       rc = %d %s",
                (numVarPrefixes - numErrors), numErrors, firstError, rc);

        /* Set default UC prefix */
        rc = prvTgfTablesFillingLttDefaultUcPrefixSet(protocol, numEntries - 1, colLast,
                                                      prvTgfRestoreCfg.routeDefUcIdx);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTablesFillingLttDefaultUcPrefixSet");

        /*----------------------------------------------------------------------
         * 2. check prefixes after creating
         */

        /* reset ETH counters */
        rc = prvTgfEthCountersReset(prvTgfDevNum);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfEthCountersReset");

        /* reset IP counters */
        rc = prvTgfCountersIpSet(prvTgfPortsArray[PRV_TGF_TX_0_IDX_CNS], 0);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCountersIpSet");

        /* check prefixes with full length */
        PRV_UTF_LOG3_MAC("To check %d prefixes with full length after creating,\n"
                         "  transmit %d matched and %d unmatched packets\n",
                         numPrefixes - numVarPrefixes, numPrefixes - numVarPrefixes,
                         prvTgfWrongPackets);

        /* set default ip addresses to check prefixes with full length */
        rc = prvTgfTablesFillingLttIpToPtr(protocol, prvTgfDefDipv4, prvTgfDefDipv6, prvTgfIpCur);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTablesFillingLttIpToPtr");

        /* set current next ip mode */
        prvTgfNextIpMode = PRV_TGF_NEXT_IP_MODE_INC_E;

        /* Generate Traffic to check existed prefixes */
        prvTgfTablesFillingLttBaseTrafficGenerate(protocol, numPrefixes, numVarPrefixes,
                                                  burstCount, numPrefixes - numVarPrefixes);

        /* check prefixes with various length */
        PRV_UTF_LOG2_MAC("To check %d prefixes with various length after"
                         " creating,\n  transmit %d matched packets\n",
                         numVarPrefixes, numVarPrefixes);

        /* set default ip addresses to check prefixes with various length */
        rc = prvTgfTablesFillingLttIpToPtr(protocol, prvTgfDefDipv4Various,
                                           prvTgfDefDipv6Various, prvTgfIpCur);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTablesFillingLttIpToPtr");

        /* set current next ip mode */
        prvTgfNextIpMode = PRV_TGF_NEXT_IP_MODE_SHL_E;

        /* Generate Traffic to check existed prefixes */
        prvTgfTablesFillingLttBaseTrafficGenerate(protocol, numPrefixes, numVarPrefixes,
                                                  1, numVarPrefixes);

        /* Check Counters */
        prvTgfTablesFillingLttCheckCounters(numPrefixes, numPrefixes, prvTgfWrongPackets);

        /*----------------------------------------------------------------------
         * 3. delete prefixes - Clear Router TCAM Table
         */
        rc = prvTgfTablesFillingLttInvalidate(protocol, 0, numEntries - 1, &numErrors, &firstError);
        UTF_VERIFY_EQUAL4_STRING_MAC(0, numErrors,
                "prvTgfTablesFillingLttInvalidate"
                "\n       could successfully invalidate only %d prefixes !!!"
                "\n       It has not handled %d prefixes."
                "\n       First error was happened on setting of %d prefix"
                "\n       rc = %d %s",
                (numPrefixes + numVarPrefixes - numErrors), numErrors, firstError, rc);

        /*----------------------------------------------------------------------
         * 4. check prefixes after deleting
         */

        /* reset ETH counters */
        rc = prvTgfEthCountersReset(prvTgfDevNum);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfEthCountersReset");

        /* reset IP counters */
        rc = prvTgfCountersIpSet(prvTgfPortsArray[PRV_TGF_TX_0_IDX_CNS], 0);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCountersIpSet");

        /* check prefixes with full length */
        PRV_UTF_LOG3_MAC("To check %d prefixes with full length after deleting,\n"
                         "  transmit %d matched and %d unmatched packets\n",
                         numPrefixes - numVarPrefixes, numPrefixes - numVarPrefixes,
                         prvTgfWrongPackets);

        /* set default ip addresses to check prefixes with full length */
        rc = prvTgfTablesFillingLttIpToPtr(protocol, prvTgfDefDipv4, prvTgfDefDipv6, prvTgfIpCur);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTablesFillingLttIpToPtr");

        /* set current next ip mode */
        prvTgfNextIpMode = PRV_TGF_NEXT_IP_MODE_INC_E;

        /* Generate Traffic to check deleted prefixes*/
        prvTgfTablesFillingLttBaseTrafficGenerate(protocol, numPrefixes, numVarPrefixes,
                                                  burstCount, 0);

        /* check prefixes with various length */
        PRV_UTF_LOG2_MAC("To check %d prefixes with various length after"
                         " deleting,\n  transmit %d matched packets\n\n",
                         numVarPrefixes, numVarPrefixes);

        /* set default ip addresses to check prefixes with various length */
        rc = prvTgfTablesFillingLttIpToPtr(protocol, prvTgfDefDipv4Various,
                                           prvTgfDefDipv6Various, prvTgfIpCur);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTablesFillingLttIpToPtr");

        /* set current next ip mode */
        prvTgfNextIpMode = PRV_TGF_NEXT_IP_MODE_SHL_E;

        /* Generate Traffic to check existed prefixes */
        prvTgfTablesFillingLttBaseTrafficGenerate(protocol, numPrefixes,
                                                  numVarPrefixes, 1, 0);

        /* Check Counters */
        prvTgfTablesFillingLttCheckCounters(numPrefixes, 0, numPrefixes + prvTgfWrongPackets);
    }
}

/**
* @internal prvTgfTablesFillingLttConfigurationRestore function
* @endinternal
*
* @brief   Restore configuration
*/
GT_VOID prvTgfTablesFillingLttConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS rc;
    GT_U32 row;
    GT_U32 col;
    GT_U32 routeIdx;

    /* restore default UC IPv4 prefix */
    row      = prvTgfRestoreCfg.rowUcIpv4;
    col      = prvTgfRestoreCfg.colUcIpv4;
    routeIdx = prvTgfRestoreCfg.routeDefUcIdx;

    rc = prvTgfTablesFillingLttDefaultUcPrefixSet(CPSS_IP_PROTOCOL_IPV4_E, row, col, routeIdx);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTablesFillingLttDefaultUcPrefixSet: %d",
                                 prvTgfDevNum);

    /* restore default UC IPv6 prefix */
    row      = prvTgfRestoreCfg.rowUcIpv6;
    col      = prvTgfRestoreCfg.colUcIpv6;
    routeIdx = prvTgfRestoreCfg.routeDefUcIdx;

    rc = prvTgfTablesFillingLttDefaultUcPrefixSet(CPSS_IP_PROTOCOL_IPV6_E, row, col, routeIdx);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTablesFillingLttDefaultUcPrefixSet: %d",
                                 prvTgfDevNum);

    /* restore default MC IPv4 prefix */
    row      = prvTgfRestoreCfg.rowMcGrpIpv4;
    col      = prvTgfRestoreCfg.colMcGrpIpv4;
    routeIdx = prvTgfRestoreCfg.routeDefMcIdx;

    rc = prvTgfTablesFillingLttDefaultMcPrefixSet(CPSS_IP_PROTOCOL_IPV4_E, row, col, routeIdx);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTablesFillingLttDefaultMcPrefixSet: %d",
                                 prvTgfDevNum);

    /* restore default MC IPv6 prefix */
    row      = prvTgfRestoreCfg.rowMcGrpIpv6;
    col      = 0;
    routeIdx = prvTgfRestoreCfg.routeDefMcIdx;

    rc = prvTgfTablesFillingLttDefaultMcPrefixSet(CPSS_IP_PROTOCOL_IPV6_E, row, col, routeIdx);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTablesFillingLttDefaultMcPrefixSet: %d",
                                 prvTgfDevNum);

    /* disable IPv4 and IPv6 Unicast Routing on a Port */
    rc = prvTgfIpPortRoutingEnable(PRV_TGF_TX_0_IDX_CNS,
                                   CPSS_IP_UNICAST_E, CPSS_IP_PROTOCOL_IPV4V6_E, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpPortRoutingEnable: %d %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_TX_0_IDX_CNS]);

    /* disable IPv4 and IPv6 Unicast Routing on Vlan */
    rc = prvTgfIpVlanRoutingEnable(PRV_TGF_SEND_VLANID_CNS, CPSS_IP_UNICAST_E,
                                   CPSS_IP_PROTOCOL_IPV4V6_E, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpVlanRoutingEnable: %d %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_VLANID_CNS]);

    /* restore default vlanId to all ports */
    rc = prvTgfBrgVlanPvidSet(prvTgfRestoreCfg.vid);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPvidSet: %d, %d",
                                 prvTgfDevNum, prvTgfRestoreCfg.vid);

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush");

    /* invalidate vlan entry (and reset vlan entry) */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_SEND_VLANID_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate");

    /* invalidate vlan entry (and reset vlan entry) */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_NEXTHOPE_VLANID_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate");
}


