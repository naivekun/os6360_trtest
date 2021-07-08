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
* @file prvTgfTablesFillingLpm.c
*
* @brief High Level IP table filling test
*
* @version   17
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

#include <ip/prvTgfTablesFillingLpm.h>


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

/* number of wrong (not matched) packets to send default value */
static GT_U32        prvTgfWrongPacketsDefault = 10;

/* number of wrong (not matched) packets to send */
static GT_U32        prvTgfWrongPackets = 0;

/* the LPM DB id for LPM Table */
static GT_U32        prvTgfLpmDBId = 0;

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

} prvTgfRestoreCfg;

/******************************************************************************\
 *                           Private declarations                             *
\******************************************************************************/

/* enumeration that allow select a mode for next IP address */
typedef enum
{
    PRV_TGF_NEXT_MODE_STATIC_E,
    PRV_TGF_NEXT_MODE_SHL_E,
    PRV_TGF_NEXT_MODE_INC_E,
    PRV_TGF_NEXT_MODE_INC_VAL_E,
    PRV_TGF_NEXT_MODE_RANDOM_E,
    PRV_TGF_NEXT_MODE_LAST_E
} PRV_TGF_NEXT_MODE_ENT;

/* current mode to calculate IP address in the test */
static PRV_TGF_NEXT_MODE_ENT prvTgfNextIpMode     = PRV_TGF_NEXT_MODE_INC_E;

/* current mode to calculate lengths in the test */
static PRV_TGF_NEXT_MODE_ENT prvTgfNextLengthMode = PRV_TGF_NEXT_MODE_INC_E;

/* current increment value for PRV_TGF_NEXT_MODE_INC_VAL_E mode */
static GT_U32 prvTgfIncValue = 1;

/* current index of IP address in array */
static GT_U32 prvTgfIpIndex = 0;

/* current seed value to initialize randomize engine */
static GT_U32  prvTgfSeed = 10;

/* arrays of IPv4 addresses to create prefixes and transmit traffic */
static GT_IPADDR   *prvTgfArIpv4 = NULL;

/* arrays of IPv4 masks to create prefixes and transmit traffic */
static GT_IPADDR   *prvTgfArMsk4 = NULL;

/* arrays of IPv4 lengths to create prefixes and transmit traffic */
static GT_U8       *prvTgfArLen4 = NULL;

/* arrays of IPv6 addresses to create prefixes and transmit traffic */
static GT_IPV6ADDR *prvTgfArIpv6 = NULL;

/* arrays of IPv6 masks to create prefixes and transmit traffic */
static GT_IPV6ADDR *prvTgfArMsk6 = NULL;

/* arrays of IPv6 lengths to create prefixes and transmit traffic */
static GT_U8       *prvTgfArLen6 = NULL;

/* number of IPv4 prefixes allowed to add to LPM DB */
static GT_U32      prvTgfNumIpv4 = 0;

/* number of IPv6 prefixes allowed to add to LPM DB */
static GT_U32      prvTgfNumIpv6 = 0;

/* prefix counters */
static struct
{
    GT_U32 numMatched;
    GT_U32 numCovered;
    GT_U32 numDipErrors;
    GT_U32 numLpmErrors;
    GT_U32 numCoverErrors;
} prvTgfPrefixCounters;


/******************************************************************************\
 *                            Private test functions                          *
\******************************************************************************/

/**
* @internal prvTgfTablesFillingLpmIpToPtr function
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
static GT_STATUS prvTgfTablesFillingLpmIpToPtr
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
        cpssOsMemCpy(ipPtr, ipv4Addr, 4*sizeof(GT_U8));
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
* @internal prvTgfTablesFillingLpmIpInc function
* @endinternal
*
* @brief   Increment IPv4 or IPv6 address
*
* @param[in] protocol                 - type of IP stack used
* @param[in,out] ipPtr                    - (pointer to) IP address
* @param[in] deltaIndex               - [index + delta] for the next IP address
* @param[in,out] ipPtr                    - (pointer to) IP address + 1
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfTablesFillingLpmIpInc
(
    IN    CPSS_IP_PROTOCOL_STACK_ENT protocol,
    INOUT GT_U8                      *ipPtr,
    IN    GT_U32                     deltaIndex
)
{
    GT_U8  offset    = 0;
    GT_U32 deltaIter = 0;
    GT_U32 incValue  = 0;
    GT_U32 incIter   = 0;

    /* check input parameters */
    CPSS_NULL_PTR_CHECK_MAC(ipPtr);

    /* set increment value */
    incValue = (PRV_TGF_NEXT_MODE_INC_VAL_E == prvTgfNextIpMode) ? prvTgfIncValue : 1;

    for (incIter = 0; incIter < incValue; incIter++)
    {
        for (deltaIter = 0; deltaIter < deltaIndex; deltaIter++)
        {
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
        }
    }

    return GT_OK;
}

/**
* @internal prvTgfTablesFillingLpmIpShl function
* @endinternal
*
* @brief   Shift Left IPv4 or IPv6 address
*
* @param[in] protocol                 - type of IP stack used
* @param[in,out] ipPtr                    - (pointer to) IP address
* @param[in] deltaIndex               - [index + delta] for the next IP address
* @param[in,out] ipPtr                    - (pointer to) IP address << 1
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfTablesFillingLpmIpShl
(
    IN    CPSS_IP_PROTOCOL_STACK_ENT protocol,
    INOUT GT_U8                      *ipPtr,
    IN    GT_U32                     deltaIndex
)
{
    GT_U8  offset    = 0;
    GT_U8  carrySave = 0;
    GT_U8  carry     = 0;
    GT_U32 deltaIter = 0;

    /* check input parameters */
    CPSS_NULL_PTR_CHECK_MAC(ipPtr);

    for (deltaIter = 0; deltaIter < deltaIndex; deltaIter++)
    {
        /* set offset for IPv4\IPv6 addr */
        offset = (GT_U8) ((CPSS_IP_PROTOCOL_IPV4_E == protocol) ? 4 : 16);

        /* shift each byte in IP addr */
        while (offset--)
        {
            carry = carrySave;
            carrySave = (GT_U8) ((ipPtr[offset] & 0x80) ? 1 : 0);
            ipPtr[offset] <<= 1;
            ipPtr[offset] = (GT_U8) (ipPtr[offset] + carry);
        }
    }

    return GT_OK;
}

/**
* @internal prvTgfTablesFillingLpmIpGet function
* @endinternal
*
* @brief   Get IP from IPv4 or IPv6 array,
*         and move cursor to the next IP
* @param[in] protocol                 - type of IP stack used
* @param[in] deltaIndex               - [index + delta] for the next IP address
*
* @param[out] ipPtr                    - (pointer to) next IP address
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfTablesFillingLpmIpGet
(
    IN  CPSS_IP_PROTOCOL_STACK_ENT protocol,
    OUT GT_U8                      *ipPtr,
    IN  GT_U32                     deltaIndex
)
{
    GT_U32 numPrefixes = 0;
    GT_U8  numBytes    = 0;
    GT_U8  *arIp       = NULL;

    /* get number of bytes in IP address */
    numBytes = (GT_U8) ((CPSS_IP_PROTOCOL_IPV4_E == protocol) ? 4 : 16);

    /* get pointer to current IP address */
    arIp = (CPSS_IP_PROTOCOL_IPV4_E == protocol) ?
           prvTgfArIpv4[prvTgfIpIndex].arIP : prvTgfArIpv6[prvTgfIpIndex].arIP;

    /* copy IP address */
    cpssOsMemCpy(ipPtr, arIp, numBytes);

    /* get number available prefixes */
    numPrefixes = (CPSS_IP_PROTOCOL_IPV4_E == protocol) ? prvTgfNumIpv4 : prvTgfNumIpv6;

    /* move cursor to next IP address */
    if ((prvTgfIpIndex + deltaIndex) < numPrefixes)
    {
       prvTgfIpIndex += deltaIndex;
    }
    else
    {
       prvTgfIpIndex = 0;
    }

    return GT_OK;
}

/**
* @internal prvTgfTablesFillingLpmLenGet function
* @endinternal
*
* @brief   Get length from array
*
* @param[in] protocol                 - type of IP stack used
*
* @param[out] lenPtr                   - (pointer to) next IP address
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfTablesFillingLpmLenGet
(
    IN  CPSS_IP_PROTOCOL_STACK_ENT protocol,
    OUT GT_U32                     *lenPtr
)
{
    GT_STATUS rc = GT_OK;

    /* get length depending on lenght mode */
    switch (prvTgfNextLengthMode)
    {
        case PRV_TGF_NEXT_MODE_RANDOM_E:
            *lenPtr = (GT_U32) (CPSS_IP_PROTOCOL_IPV4_E == protocol) ?
                       prvTgfArLen4[prvTgfIpIndex] : prvTgfArLen6[prvTgfIpIndex];
            break;

        case PRV_TGF_NEXT_MODE_STATIC_E:
            *lenPtr = (GT_U32) (CPSS_IP_PROTOCOL_IPV4_E == protocol) ? 32 : 128;
            break;

        default:
            return GT_BAD_PARAM;
    }

    return rc;
}

/**
* @internal prvTgfTablesFillingLpmNextAddrGet function
* @endinternal
*
* @brief   Get Next IP Address
*
* @param[in] protocol                 - type of IP stack used
* @param[in,out] ipPtr                    - (pointer to) current IP address
* @param[in] deltaIndex               - [index + delta] for the next IP address
* @param[in,out] ipPtr                    - (pointer to) next IP address
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfTablesFillingLpmNextAddrGet
(
    IN    CPSS_IP_PROTOCOL_STACK_ENT protocol,
    INOUT GT_U8                      *ipPtr,
    IN    GT_U32                     deltaIndex
)
{
    GT_STATUS   rc = GT_OK;

    /* check input parameters */
    CPSS_NULL_PTR_CHECK_MAC(ipPtr);

    /* choose IP update mode */
    switch (prvTgfNextIpMode)
    {
        case PRV_TGF_NEXT_MODE_INC_E:
        case PRV_TGF_NEXT_MODE_INC_VAL_E:
            rc = prvTgfTablesFillingLpmIpInc(protocol, ipPtr, deltaIndex);
            PRV_UTF_VERIFY_GT_OK(rc, "prvTgfTablesFillingLpmIpInc");

            break;

        case PRV_TGF_NEXT_MODE_SHL_E:
            rc = prvTgfTablesFillingLpmIpShl(protocol, ipPtr, deltaIndex);
            PRV_UTF_VERIFY_GT_OK(rc, "prvTgfTablesFillingLpmIpShl");

            break;

        case PRV_TGF_NEXT_MODE_RANDOM_E:
            rc = prvTgfTablesFillingLpmIpGet(protocol, ipPtr, deltaIndex);
            PRV_UTF_VERIFY_GT_OK(rc, "prvTgfTablesFillingLpmIpGet");

            break;

        default:
            return GT_BAD_PARAM;
    }

    return rc;
}

/**
* @internal prvTgfTablesFillingLpmSeedGenerate function
* @endinternal
*
* @brief   Generate new seed value and initialize random generator
*
* @param[in,out] seedPtr                  - (pointer to) initial seed value
* @param[in,out] seedPtr                  - (pointer to) seed value
*                                       None
*/
static GT_VOID prvTgfTablesFillingLpmSeedGenerate
(
    INOUT GT_U32 *seedPtr
)
{
    /* generate new random seed value */
    *seedPtr = prvUtfSeedFromStreamNameGet();
    /* set specific seed for random generator */
    cpssOsSrand(*seedPtr);
}

/**
* @internal prvTgfTableFillingLpmGetNumPrefixesToAdd function
* @endinternal
*
* @brief   Get number of prefixes to add from each protocol.
*
* @param[out] numIpv4Ptr               - (points to) number of IPv4 prefixes to add
* @param[out] numIpv6Ptr               - (points to) number of IPv6 prefixes to add
*                                       None
*
* @note 1. If protocol is IPv4 only then numIpv6 will be 0 and via versa.
*       2. Is case of dynamic partition, the shared space will be randomly
*       splitted between IPv4 and IPv6
*       3. Dynamic partition is not supported by CPSS in Policy Based Routing.
*
*/
static GT_STATUS prvTgfTableFillingLpmGetNumPrefixesToAdd
(
    OUT GT_U32                      *numIpv4Ptr,
    OUT GT_U32                      *numIpv6Ptr
)
{
    GT_STATUS                                   rc = GT_OK;
    GT_U32                                      tcamEntriesLeft;
    GT_U32                                      ipv6PrefixesFromShared;
    PRV_TGF_IP_TCAM_SHADOW_TYPE_ENT             shadowType;
    CPSS_IP_PROTOCOL_STACK_ENT                  lpmProtocolStack;
    PRV_TGF_IP_TCAM_LPM_MANGER_INDEX_RANGE_STC  indexesRange;
    GT_BOOL                                     partitionEnable;
    PRV_TGF_IP_TCAM_LPM_MANGER_CAPCITY_CFG_STC  tcamLpmManagerCapcityCfg;
    GT_BOOL                                     isTcamManagerInternal;
    GT_PTR                                      tcamManagerHandlerPtr;

    if ((numIpv4Ptr == NULL) || (numIpv6Ptr == NULL))
        return GT_BAD_PTR;

    /* get LPM DB config */
    rc = prvTgfIpLpmDBConfigGet(prvTgfLpmDBId,&shadowType,&lpmProtocolStack,
                                &indexesRange,&partitionEnable,
                                &tcamLpmManagerCapcityCfg,
                                &isTcamManagerInternal,&tcamManagerHandlerPtr);
    if (rc != GT_OK)
        return rc;

    if(shadowType == PRV_TGF_IP_RAM_SHADOW_E)
    {
        /* claculate numIpv4Ptr and numIpv6Ptr according to the RAM size supported */
        CPSS_TBD_BOOKMARK_EARCH

        return GT_NOT_SUPPORTED;

    }
    else
    {
        /* dynamic partition is not supported in policy based routing */
        if ((partitionEnable == GT_FALSE) && (shadowType == PRV_TGF_IP_TCAM_POLICY_BASED_ROUTING_SHADOW_E))
            return GT_NOT_SUPPORTED;

        /* initially all entries in TCAM section allocated to the LPM are free */
        tcamEntriesLeft = (indexesRange.lastIndex - indexesRange.firstIndex + 1) * 4;

        /* in case of static partition unused entries are allocated to IPv4 */
        if (partitionEnable == GT_TRUE)
        {
            *numIpv6Ptr       = tcamLpmManagerCapcityCfg.numOfIpv6Prefixes;
            tcamEntriesLeft  -= (4 * (*numIpv6Ptr));
            tcamEntriesLeft  -= tcamLpmManagerCapcityCfg.numOfIpv4McSourcePrefixes;
            *numIpv4Ptr       = tcamEntriesLeft;

            /* 2 entries for each protocol stack are used by VR 0 deaults */
            if (((*numIpv4Ptr) < 2) || ((*numIpv6Ptr) < 2))
                return GT_BAD_STATE;
            /* remove entries already used by VR 0 defaults */
            *numIpv4Ptr -= 2;
            *numIpv6Ptr -= 2;
        }
        /* in case of static partition, number of IPv6 entries in the shared TCAM
           lines will be set randomly */
        else
        {
            *numIpv4Ptr       = tcamLpmManagerCapcityCfg.numOfIpv4Prefixes;
            *numIpv6Ptr       = tcamLpmManagerCapcityCfg.numOfIpv6Prefixes;

            /* 2 entries for each protocol stack are used by VR 0 default */
            if ((*numIpv4Ptr) < 2)
                *numIpv4Ptr = 2;
            if ((*numIpv6Ptr) < 2)
                *numIpv6Ptr = 2;

            /* calculate how much shared entries there are */
            tcamEntriesLeft  -= (4 * (*numIpv6Ptr));
            tcamEntriesLeft  -= tcamLpmManagerCapcityCfg.numOfIpv4McSourcePrefixes;
            tcamEntriesLeft  -= (*numIpv4Ptr);

            /* generate new random seed value */
            prvTgfTablesFillingLpmSeedGenerate(&prvTgfSeed);

            /* randomly select number of IPv6 entries from shared area */
            ipv6PrefixesFromShared = cpssOsRand() % ((tcamEntriesLeft/4) + 1);
            *numIpv6Ptr += ipv6PrefixesFromShared;

            /* rest goes to IPv4 */
            tcamEntriesLeft  -= (4 * ipv6PrefixesFromShared);
            *numIpv4Ptr      += tcamEntriesLeft;

            /* remove entries already used by VR 0 defaults */
            *numIpv4Ptr -= 2;
            *numIpv6Ptr -= 2;
        }
    }

    return rc;
};

/**
* @internal prvTgfTablesFillingLpmIpCreate function
* @endinternal
*
* @brief   Create IP addresses and fill array of IP addresses
*
* @param[in] protocol                 - type of IP stack used
* @param[in] numPrefixes              - general number of IP addresses to generate
*                                       None
*/
static GT_VOID prvTgfTablesFillingLpmIpCreate
(
    IN CPSS_IP_PROTOCOL_STACK_ENT protocol,
    IN GT_U32                     numPrefixes
)
{
    GT_STATUS   rc         = GT_OK;
    GT_U32  ipIter     = 0;
    GT_U8       byteIter   = 0;
    GT_U32  uniqueIter = 0;
    GT_U32  numHits    = 0;
    GT_U32  firstHit   = 0;
    GT_U16  rnd        = 0;
    GT_BOOL isUniquePreffix = GT_FALSE;
    GT_BOOL isUniqueBytes   = GT_FALSE;
    GT_U8   maxPrefixLen    = 0;
    GT_U8   maxPrefixBytes  = 0;
    GT_U8   *arLen;
    GT_U8   *arIP, *ipPtr;
    GT_U8   *arMask, *mskPtr;


    /* generate new random seed value */
    prvTgfTablesFillingLpmSeedGenerate(&prvTgfSeed);

    /* set max IPv4\IPv6 prefix length */
    maxPrefixLen   = (GT_U8) ((CPSS_IP_PROTOCOL_IPV4_E == protocol) ? 32 : 128);
    maxPrefixBytes = (GT_U8) ((CPSS_IP_PROTOCOL_IPV4_E == protocol) ?  4 :  16);

    /* generate new prefixes */
    for (ipIter = 0; ipIter < numPrefixes; ipIter++)
    {
        /* point to the free prefix in IPv4 or IPv6 arrays to fill by values */
        arIP = (CPSS_IP_PROTOCOL_IPV4_E == protocol) ?
                prvTgfArIpv4[ipIter].arIP : prvTgfArIpv6[ipIter].arIP;
        arMask = (CPSS_IP_PROTOCOL_IPV4_E == protocol) ?
                prvTgfArMsk4[ipIter].arIP : prvTgfArMsk6[ipIter].arIP;
        arLen = (CPSS_IP_PROTOCOL_IPV4_E == protocol) ?
                &prvTgfArLen4[ipIter] : &prvTgfArLen6[ipIter];

        /* get random ip address */
        for (byteIter = 0; byteIter < maxPrefixBytes; byteIter += 2)
        {
            rnd = (GT_U16) (cpssOsRand() + cpssOsRand());
            arIP[byteIter] = (GT_U8) (rnd >> 8);
            arIP[byteIter + 1] = (GT_U8) rnd;
        }

        /* get random length */
        *arLen = (GT_U8) ((cpssOsRand() % (maxPrefixLen - 1)) + 1);

        /* convert length to mask */
        cpssOsMemSet(arMask, 0, maxPrefixBytes);
        cpssOsMemSet(arMask, 0xFF, (*arLen / 8));
        if ((*arLen % 8) != 0)
        {
            arMask[(*arLen / 8)] = (GT_U8)((0xFF << (8 - (*arLen % 8))) & 0xFF);
        }

        /* disable forbidden ip addresses */
        if (CPSS_IP_PROTOCOL_IPV4_E == protocol)
        {
            /* disable localhost IP addresses */
            if (arIP[0] == 127)
            {
                numHits++;
                if (firstHit == 0)
                {
                    firstHit = ipIter;
                }

                ipIter--;
                continue;
            }

            /* disable IP multicast and broadcast addresses */
            if (arIP[0] >= 224)
            {
                numHits++;
                if (firstHit == 0)
                {
                    firstHit = ipIter;
                }

                ipIter--;
                continue;
            }
        }
        else
        {
            /* disable IP multicast addresses */
            if (arIP[0] == 255)
            {
                numHits++;
                if (firstHit == 0)
                {
                    firstHit = ipIter;
                }

                ipIter--;
                continue;
            }
        }

        /* check if prefix is unique - scan array to find the same one */
        isUniquePreffix = GT_TRUE;
        for (uniqueIter = 0; uniqueIter < ipIter; uniqueIter++)
        {
            /* point to the existed prefix in IPv4 or IPv6 arrays to check matching */
            ipPtr = (CPSS_IP_PROTOCOL_IPV4_E == protocol) ?
                     prvTgfArIpv4[uniqueIter].arIP : prvTgfArIpv6[uniqueIter].arIP;
            mskPtr = (CPSS_IP_PROTOCOL_IPV4_E == protocol) ?
                     prvTgfArMsk4[uniqueIter].arIP : prvTgfArMsk6[uniqueIter].arIP;

            isUniqueBytes = GT_FALSE;

            /* scan all bytes of ip to find at least one difference */
            for (byteIter = 0; byteIter < maxPrefixBytes; byteIter++)
            {
                if ((arIP[byteIter] & arMask[byteIter]) != (ipPtr[byteIter] & mskPtr[byteIter]))
                {
                    isUniqueBytes = GT_TRUE;
                    break;
                }
            }

            /* if the same prefix was found, stop searching */
            if (GT_FALSE == isUniqueBytes)
            {
                isUniquePreffix = GT_FALSE;
                numHits++;
                if (firstHit == 0)
                {
                    firstHit = ipIter;
                }

                ipIter--;
                break;
            }
        }

        /* generate a new prefix if such prefix had been found in array */
        if (GT_FALSE == isUniquePreffix)
        {
            continue;
        }

        /* print generated prefixes */
        if (GT_TRUE == prvTgfEnablePrefixLog)
        {
            /* print IP address */
            PRV_UTF_LOG1_MAC("iter=%04d:", ipIter);

            rc = prvTgfPrefixPrint((CPSS_IP_PROTOCOL_IPV4_E == protocol) ? protocol :
                                    CPSS_IP_PROTOCOL_IPV6_E, arIP, arMask, *arLen);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPrefixPrint");
        }
    }

    PRV_UTF_LOG2_MAC("numHits = %d; firstHit = %d;\n", numHits, firstHit);
}

/**
* @internal prvTgfTablesFillingLpmIpv4Add function
* @endinternal
*
* @brief   Add numEntries IPv4 UC prefixes to LPM DB
*
* @param[in] numPrefixes              - number of prefixes to add
*
* @param[out] numErrorsPtr             - number of errors caused with prvTgfIpv4PrefixSet call
* @param[out] firstErrorPtr            - rc of prvTgfIpv4PrefixSet call when first error happen
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfTablesFillingLpmIpv4Add
(
    IN  GT_U32 numPrefixes,
    OUT GT_U32 *numErrorsPtr,
    OUT GT_U32 *firstErrorPtr
)
{
    GT_STATUS   rc        = GT_OK;
    GT_STATUS   firstRc   = GT_OK;
    GT_U32      numErrors = 0;
    GT_U32      prefixLen = 0;
    GT_U32      ipIter    = 0;
    GT_IPADDR   ipv4Addr;
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT nextHopInfo;


    PRV_UTF_LOG1_MAC("Add %d IPv4 UC prefixes\n", numPrefixes);

    /* set default IP address for the prefixes */
    rc = prvTgfTablesFillingLpmIpToPtr(CPSS_IP_PROTOCOL_IPV4_E, prvTgfDefDipv4,
                                       prvTgfDefDipv6, ipv4Addr.arIP);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfTablesFillingLpmIpToPtr");

    /* set nextHopInfo */
    cpssOsMemSet(&nextHopInfo, 0, sizeof(nextHopInfo));

    /* reset error counter */
    numErrors      = 0;
    *firstErrorPtr = 0;

    /* start filling from the beginning of array */
    prvTgfIpIndex = 0;

    for (ipIter = 0; ipIter < numPrefixes; ipIter++)
    {
        /* get current prefix length */
        rc = prvTgfTablesFillingLpmLenGet(CPSS_IP_PROTOCOL_IPV4_E, &prefixLen);
        PRV_UTF_VERIFY_GT_OK(rc, "prvTgfTablesFillingLpmLenGet");

        /* get current IP address and move cursor to next address */
        rc = prvTgfTablesFillingLpmNextAddrGet(CPSS_IP_PROTOCOL_IPV4_E, ipv4Addr.arIP, 1);
        PRV_UTF_VERIFY_GT_OK(rc, "prvTgfTablesFillingLpmNextAddrGet");

        /* set routeEntryBaseIndex */
        nextHopInfo.ipLttEntry.routeEntryBaseIndex = prvTgfRouteEntryBaseIndex + prefixLen;
        nextHopInfo.ipLttEntry.routeType = PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_MAC;

        /* add prefix to LPM DB */
        rc = prvTgfIpLpmIpv4UcPrefixAdd(prvTgfLpmDBId, 0, ipv4Addr, prefixLen,
                                        &nextHopInfo, GT_FALSE, GT_FALSE);

        /* accumulate errors in numErrors */
        if (GT_OK != rc)
        {
            /* capture first error */
            if (0 == *firstErrorPtr)
            {
                *firstErrorPtr = ipIter;
                firstRc        = rc;
            }

            numErrors++;
        }
    }

    *numErrorsPtr = numErrors;

    return firstRc;
}

/**
* @internal prvTgfTablesFillingLpmIpv6Add function
* @endinternal
*
* @brief   Add numEntries IPv6 UC prefixes to LPM DB
*
* @param[in] numPrefixes              - number of prefixes to add
*
* @param[out] numErrorsPtr             - number of errors caused with prvTgfIpv6PrefixSet call
* @param[out] firstErrorPtr            - rc of prvTgfIpv6PrefixSet call when first error happen
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfTablesFillingLpmIpv6Add
(
    IN  GT_U32 numPrefixes,
    OUT GT_U32 *numErrorsPtr,
    OUT GT_U32 *firstErrorPtr
)
{
    GT_STATUS       rc        = GT_OK;
    GT_STATUS       firstRc   = GT_OK;
    GT_U32          numErrors = 0;
    GT_U32          prefixLen = 0;
    GT_U32          ipIter    = 0;
    GT_IPV6ADDR     ipv6Addr;
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT nextHopInfo;


    PRV_UTF_LOG1_MAC("Add %d IPv6 UC prefixes\n", numPrefixes);

    /* set default IP address for the prefixes */
    rc = prvTgfTablesFillingLpmIpToPtr(CPSS_IP_PROTOCOL_IPV6_E, prvTgfDefDipv4,
                                       prvTgfDefDipv6, ipv6Addr.arIP);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfTablesFillingLpmIpToPtr");

    /* set nextHopInfo */
    cpssOsMemSet(&nextHopInfo, 0, sizeof(nextHopInfo));

    /* reset error counter */
    numErrors      = 0;
    *firstErrorPtr = 0;

    /* start filling from the beginning of array */
    prvTgfIpIndex = 0;

    for (ipIter = 0; ipIter < numPrefixes; ipIter++)
    {
        /* get current prefix length */
        rc = prvTgfTablesFillingLpmLenGet(CPSS_IP_PROTOCOL_IPV6_E, &prefixLen);
        PRV_UTF_VERIFY_GT_OK(rc, "prvTgfTablesFillingLpmLenGet");

        /* get current IP address and move cursor to next address */
        rc = prvTgfTablesFillingLpmNextAddrGet(CPSS_IP_PROTOCOL_IPV6_E, ipv6Addr.arIP, 1);
        PRV_UTF_VERIFY_GT_OK(rc, "prvTgfTablesFillingLpmNextAddrGet");

        /* set routeEntryBaseIndex */
        nextHopInfo.ipLttEntry.routeEntryBaseIndex = prvTgfRouteEntryBaseIndex + prefixLen;
        nextHopInfo.ipLttEntry.routeType = PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_MAC;

        /* add prefix to LPM DB */
        rc = prvTgfIpLpmIpv6UcPrefixAdd(prvTgfLpmDBId, 0, ipv6Addr, prefixLen,
                                        &nextHopInfo, GT_FALSE, GT_FALSE);

        /* accumulate errors in numErrors */
        if (GT_OK != rc)
        {
            /* capture first error */
            if (0 == *firstErrorPtr)
            {
                *firstErrorPtr = ipIter;
                firstRc        = rc;
            }

            numErrors++;
        }
    }

    *numErrorsPtr = numErrors;

    return firstRc;
}

/**
* @internal prvTgfTablesFillingLpmAdd function
* @endinternal
*
* @brief   Add numEntries prefixes to LPM DB
*
* @param[in] protocol                 - type of IP stack used
* @param[in] numPrefixes              - number of prefixes to add
*
* @param[out] numErrorsPtr             - number of errors
* @param[out] firstErrorPtr            - rc when first error happen
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfTablesFillingLpmAdd
(
    IN  CPSS_IP_PROTOCOL_STACK_ENT  protocol,
    IN  GT_U32                      numPrefixes,
    OUT GT_U32                      *numErrorsPtr,
    OUT GT_U32                      *firstErrorPtr
)
{
    GT_STATUS   rc = GT_OK;

    /* fill LPM DB by IPv4\IPv6 prefixes */
    switch (protocol)
    {
        case CPSS_IP_PROTOCOL_IPV4_E:
            /* fill LPM DB by IPv4 UC prefixes */
            rc = prvTgfTablesFillingLpmIpv4Add(numPrefixes, numErrorsPtr, firstErrorPtr);
            PRV_UTF_VERIFY_GT_OK(rc, "prvTgfTablesFillingLpmIpv4Add");

            break;

        case CPSS_IP_PROTOCOL_IPV6_E:
        case CPSS_IP_PROTOCOL_IPV4V6_E:
            /* fill LPM DB by IPv6 UC prefixes */
            rc = prvTgfTablesFillingLpmIpv6Add(numPrefixes, numErrorsPtr, firstErrorPtr);
            PRV_UTF_VERIFY_GT_OK(rc, "prvTgfTablesFillingLpmIpv6Add");

            break;

        default:
            return GT_BAD_PARAM;
    }

    return rc;
}

/**
* @internal prvTgfTablesFillingLpmUcPrefixesFlush function
* @endinternal
*
* @brief   Flushes the unicast Routing table and stays with the default prefix
*         only for a specific LPM DB.
* @param[in] protocol                 - type of IP stack used
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfTablesFillingLpmUcPrefixesFlush
(
    IN  CPSS_IP_PROTOCOL_STACK_ENT protocol
)
{
    GT_STATUS   rc = GT_OK;

    /* flush IPv4\IPv6 prefixes */
    switch (protocol)
    {
        case CPSS_IP_PROTOCOL_IPV4_E:
            /* flush all IPv4 UC prefixes */
            rc = prvTgfIpLpmIpv4UcPrefixesFlush(prvTgfLpmDBId, 0);
            PRV_UTF_VERIFY_GT_OK(rc, "prvTgfIpLpmIpv4UcPrefixesFlush");

            break;

        case CPSS_IP_PROTOCOL_IPV6_E:
        case CPSS_IP_PROTOCOL_IPV4V6_E:
            /* flush all IPv6 UC prefixes */
            rc = prvTgfIpLpmIpv6UcPrefixesFlush(prvTgfLpmDBId, 0);
            PRV_UTF_VERIFY_GT_OK(rc, "prvTgfIpLpmIpv6UcPrefixesFlush");

            break;

        default:
            return GT_BAD_PARAM;
    }

    return rc;
}

/**
* @internal prvTgfTablesFillingLpmPrefixFindIpv4 function
* @endinternal
*
* @brief   Scan whole captured table to check all burstCount expected packet
*
* @param[in] expIpPtr                 - (pointer to) ip of prefix to find
* @param[in] expLen                   - length of prefix to find
*
* @param[out] indexPtr                 - (pointer to) index of found prefix
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfTablesFillingLpmPrefixFindIpv4
(
    IN  GT_U8  *expIpPtr,
    IN  GT_U32 expLen,
    OUT GT_U32 *indexPtr
)
{
    GT_STATUS   rc                = GT_OK;
    GT_U32      byteIter          = 0;
    GT_U32      prvTgfIpIndexSave = 0;


    /* save current cursor */
    prvTgfIpIndexSave = prvTgfIpIndex;

    /* scan arrays */
    for (prvTgfIpIndex = 0; prvTgfIpIndex < prvTgfNumIpv4; prvTgfIpIndex++)
    {
        /* find only prefixes with expected length */
        if (prvTgfArLen4[prvTgfIpIndex] == expLen)
        {
            /* check ip address - only relevant bits by mask */
            for (byteIter = 0; byteIter < 4; byteIter++)
            {
                /* if at least one byte not matched, find other prefix */
                if ((prvTgfArIpv4[prvTgfIpIndex].arIP[byteIter] &
                    prvTgfArMsk4[prvTgfIpIndex].arIP[byteIter]) !=
                    (expIpPtr[byteIter] & prvTgfArMsk4[prvTgfIpIndex].arIP[byteIter]))
                {
                    rc = GT_NOT_FOUND;
                    break;
                }
            }

            if (GT_OK == rc)
            {
                /* prefix is found */
                *indexPtr = prvTgfIpIndex;
                break;
            }
            else
            {
                /* continue searching */
                rc = GT_OK;
            }
        }
    }

    /* restore current cursor */
    prvTgfIpIndex = prvTgfIpIndexSave;

    return rc;
}

/**
* @internal prvTgfTablesFillingLpmPrefixFindIpv6 function
* @endinternal
*
* @brief   Scan whole captured table to check all burstCount expected packet
*
* @param[in] expIpPtr                 - (pointer to) ip of prefix to find
* @param[in] expLen                   - length of prefix to find
*
* @param[out] indexPtr                 - (pointer to) index of found prefix
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfTablesFillingLpmPrefixFindIpv6
(
    IN  GT_U8  *expIpPtr,
    IN  GT_U32 expLen,
    OUT GT_U32 *indexPtr
)
{
    GT_STATUS   rc                = GT_OK;
    GT_U32      byteIter          = 0;
    GT_U32      prvTgfIpIndexSave = 0;


    /* save current cursor */
    prvTgfIpIndexSave = prvTgfIpIndex;

    /* scan arrays */
    for (prvTgfIpIndex = 0; prvTgfIpIndex < prvTgfNumIpv6; prvTgfIpIndex++)
    {
        /* find only prefixes with expected length */
        if (prvTgfArLen6[prvTgfIpIndex] == expLen)
        {
            /* check ip address - only relevant bits by mask */
            for (byteIter = 0; byteIter < 16; byteIter++)
            {
                /* if at least one byte not matched, find other prefix */
                if ((prvTgfArIpv6[prvTgfIpIndex].arIP[byteIter] &
                    prvTgfArMsk6[prvTgfIpIndex].arIP[byteIter]) !=
                    (expIpPtr[byteIter] &
                    prvTgfArMsk6[prvTgfIpIndex].arIP[byteIter]))
                {
                    rc = GT_NOT_FOUND;
                    break;
                }
            }

            if (GT_OK == rc)
            {
                /* prefix is found */
                *indexPtr = prvTgfIpIndex;
                break;
            }
            else
            {
                /* continue searching */
                rc = GT_OK;
            }
        }
    }

    /* restore current cursor */
    prvTgfIpIndex = prvTgfIpIndexSave;

    return rc;
}

/**
* @internal prvTgfTablesFillingLpmPrefixFind function
* @endinternal
*
* @brief   Scan whole captured table to check all burstCount expected packet
*
* @param[in] protocol                 - type of IP stack used
* @param[in] expIpPtr                 - (pointer to) ip of prefix to find
* @param[in] expLen                   - length of prefix to find
*
* @param[out] indexPtr                 - (pointer to) index of found prefix
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfTablesFillingLpmPrefixFind
(
    IN  CPSS_IP_PROTOCOL_STACK_ENT protocol,
    IN  GT_U8                      *expIpPtr,
    IN  GT_U32                     expLen,
    OUT GT_U32                     *indexPtr
)
{
    GT_STATUS rc = GT_OK;


    /* scan table to get prefix index */
    switch (protocol)
    {
        case CPSS_IP_PROTOCOL_IPV4_E:
            rc = prvTgfTablesFillingLpmPrefixFindIpv4(expIpPtr, expLen, indexPtr);
            PRV_UTF_VERIFY_GT_OK(rc, "prvTgfTablesFillingLpmPrefixFindIpv4");

            break;
        case CPSS_IP_PROTOCOL_IPV6_E:
        case CPSS_IP_PROTOCOL_IPV4V6_E:
            rc = prvTgfTablesFillingLpmPrefixFindIpv6(expIpPtr, expLen, indexPtr);
            PRV_UTF_VERIFY_GT_OK(rc, "prvTgfTablesFillingLpmPrefixFindIpv6");

            break;

        default:
            return GT_BAD_PARAM;
    }

    return rc;
}

/**
* @internal prvTgfTablesFillingLpmCapturedPacketTrace function
* @endinternal
*
* @brief   scan whole captured table until GT_NO_MORE
*         to check all burstCount expected packet
* @param[in] protocol                 - type of IP stack used
* @param[in] packetNumber             - number of checked packet
* @param[in] packetBuff               - (pointer to) buffer for the captured packet
* @param[in] packetLen                - length of buffer to trace
* @param[in] expPrefixLen             - expected prefix length
* @param[in] vfdPtr                   - (pointer to) expected pattern for checking
* @param[in] matchedPrefixIndex       - prefix that was matched
* @param[in] enableTrace              - enable or disable to trace packet
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_VOID prvTgfTablesFillingLpmCapturedPacketTrace
(
    IN CPSS_IP_PROTOCOL_STACK_ENT protocol,
    IN GT_U32                     packetNumber,
    IN GT_U8                      *packetBuff,
    IN GT_U32                     packetLen,
    IN GT_U32                     expPrefixLen,
    IN TGF_VFD_INFO_STC           *vfdPtr,
    IN GT_U32                     matchedPrefixIndex,
    IN GT_BOOL                    enableTrace
)
{
    GT_STATUS   rc         = GT_OK;
    GT_U32      byteIter   = 0;
    GT_U32      matchedLen = 0;
    GT_U8       *matchedIpPtr;


    /* get matched prefix */
    matchedIpPtr = (CPSS_IP_PROTOCOL_IPV4_E == protocol) ?
           prvTgfArIpv4[matchedPrefixIndex].arIP :
           prvTgfArIpv6[matchedPrefixIndex].arIP;

    matchedLen = (CPSS_IP_PROTOCOL_IPV4_E == protocol) ?
           prvTgfArLen4[matchedPrefixIndex] :
           prvTgfArLen6[matchedPrefixIndex];

    /* print captured ip/length */
    PRV_UTF_LOG1_MAC("\npacket# %4d, got: ", packetNumber);
    for (byteIter = 0; byteIter < vfdPtr->cycleCount; byteIter++)
    {
        PRV_UTF_LOG1_MAC(" %2.2x", packetBuff[vfdPtr->offset + byteIter]);
    }
    PRV_UTF_LOG1_MAC(" / 0x%02x ", packetBuff[5]);

    /* print expected ip/length */
    PRV_UTF_LOG0_MAC("\n         expected: ");
    for (byteIter = 0; byteIter < vfdPtr->cycleCount; byteIter++)
    {
        PRV_UTF_LOG1_MAC(" %2.2x", vfdPtr->patternPtr[byteIter]);
    }
    PRV_UTF_LOG1_MAC(" / 0x%02x ", expPrefixLen);

    /* print matched ip/length */
    PRV_UTF_LOG0_MAC("\n          matched: ");
    for (byteIter = 0; byteIter < vfdPtr->cycleCount; byteIter++)
    {
        PRV_UTF_LOG1_MAC(" %2.2x", matchedIpPtr[byteIter]);
    }
    PRV_UTF_LOG1_MAC(" / 0x%02x ", matchedLen);

    if (GT_TRUE == enableTrace)
    {
        PRV_UTF_LOG0_MAC("\n");

        rc = tgfTrafficTracePacket(packetBuff, packetLen, GT_TRUE);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTracePacket");
    }
}

/**
* @internal prvTgfTablesFillingLpmCapturedPacketGet function
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
static GT_STATUS prvTgfTablesFillingLpmCapturedPacketGet
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
* @internal prvTgfTablesFillingLpmCapturedTableCheck function
* @endinternal
*
* @brief   scan whole captured table until GT_NO_MORE
*         to check all burstCount expected packet
* @param[in] protocol                 - type of IP stack used
* @param[in] packetIter               - number already checked packets in the test
* @param[in] portInterfacePtr         - (pointer to) port for capturing
* @param[in] numExpectedPackets       - number of packet to expect in capturing table
* @param[in] vfdPtr                   - (pointer to) pattern for checking
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_VOID prvTgfTablesFillingLpmCapturedTableCheck
(
    IN CPSS_IP_PROTOCOL_STACK_ENT protocol,
    IN GT_U32                     packetIter,
    IN CPSS_INTERFACE_INFO_STC    *portInterfacePtr,
    IN GT_U32                     numExpectedPackets,
    IN TGF_VFD_INFO_STC           *vfdPtr
)
{
    GT_STATUS rc = GT_OK;
    GT_U8     packetBuff[TGF_RX_BUFFER_MAX_SIZE_CNS] = {0};
    GT_U32    packetLen = 0;
    GT_U32    burstIter = 0;
    GT_U32    prefLen   = 0;
    GT_U32    prefIndex = 0;
    GT_BOOL   needToLog = GT_FALSE;
    GT_BOOL   needToTrace = GT_FALSE;


    /* scan whole captured table to check all burstCount expected packet */
    for (burstIter = 0; burstIter <= numExpectedPackets; burstIter++)
    {
        /* get next captured packet in specified interface */
        packetLen = TGF_RX_BUFFER_MAX_SIZE_CNS;

        rc = prvTgfTablesFillingLpmCapturedPacketGet(portInterfacePtr, packetBuff, &packetLen);

        /* finish the loop if there are no more packets in captured table */
        if (GT_NO_MORE == rc)
        {
            /* trace expected DIP */
            if (burstIter != numExpectedPackets)
            {
                GT_U8 *p = vfdPtr->patternPtr;

                PRV_UTF_LOG1_MAC("\nexpected DIP for the packet# %d: ", packetIter + burstIter);
                while (p < vfdPtr->patternPtr + vfdPtr->cycleCount)
                {
                    PRV_UTF_LOG1_MAC(" %2.2x", *p++);
                }
            }

            /* check if we have burstCount packets in the capturing table */
            UTF_VERIFY_EQUAL0_STRING_MAC(numExpectedPackets, burstIter,
                    "\n   Capturing table has less packets than expected.\n");

            break;
        }

        /* check if we've really got a packet in the buffer */
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
                "\n   tgfTrafficGeneratorCapturedPacketGet: %d, %d, %d\n",
                portInterfacePtr->devPort.hwDevNum,
                portInterfacePtr->devPort.portNum,
                packetLen);

        /* the capturing table must have no more than burstCount packets */
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_TRUE, burstIter < numExpectedPackets,
                "\n   Capturing table has more packets than expected.\n");

        /* get expected prefix's length to match with captured packet */
        rc = prvTgfTablesFillingLpmLenGet(protocol, &prefLen);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTablesFillingLpmLenGet\n");

        /* get expected ip address to match with captured packet */
        rc = prvTgfTablesFillingLpmNextAddrGet(protocol, vfdPtr->patternPtr, 1);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTablesFillingLpmNextAddrGet\n");

        /* check the packet in the buffer if we've got it */
        if (GT_OK == rc)
        {
            needToLog   = GT_FALSE;
            needToTrace = GT_FALSE;

            /* check whether captured packet has the proper IP address */
            rc = (0 == cpssOsMemCmp(packetBuff + vfdPtr->offset,
                                    vfdPtr->patternPtr,
                                    vfdPtr->cycleCount)) ? GT_OK : GT_FAIL;

            /* trace a BAD packet if it differs from an expected one */
            if (GT_OK != rc)
            {
                /* accumulate errors instead of logging out for each one */
                prvTgfPrefixCounters.numDipErrors++;
                needToLog   = GT_TRUE;
                needToTrace = GT_TRUE;
            }

            /* check whether captured packet was routed by expected prefix
             * we have already checked DIP address (prefix's ip)
             * now prefix's length must correlate with packet's MAC DA
             */

            /* prefix can be covered only by lager prefix */
            if (packetBuff[5] < prefLen)
            {
                /* BUG: longer prefixes covered by smaller one */
                /* prefix can be covered by LPM - try to find
                 * the same prefix with captured length: packetBuff[5] */
                rc = prvTgfTablesFillingLpmPrefixFind(protocol,
                                                      vfdPtr->patternPtr,
                                                      packetBuff[5],
                                                      &prefIndex);

                /* accumulate errors instead of logging out for each one */
                prvTgfPrefixCounters.numLpmErrors++;
                needToLog = GT_TRUE;
            }
            else if (packetBuff[5] > prefLen)
            {
                /* prefix can be covered by LPM - try to find
                 * the same prefix with captured length: packetBuff[5] */
                rc = prvTgfTablesFillingLpmPrefixFind(protocol,
                                                      vfdPtr->patternPtr,
                                                      packetBuff[5],
                                                      &prefIndex);
                if (GT_OK != rc)
                {
                    /* BUG: it is real bug */
                    /* accumulate errors instead of logging out for each one */
                    prvTgfPrefixCounters.numCoverErrors++;
                    needToLog = GT_TRUE;
                }
                else
                {
                    /* prefix was covered properly */
                    prvTgfPrefixCounters.numCovered++;
                }
            }
            else
            {
                /* prefix was matched properly */
                prvTgfPrefixCounters.numMatched++;
            }

            /* logging out info about expected prefix, matched prefix and
             * captured packet */
            if (GT_TRUE == needToLog)
            {
                prvTgfTablesFillingLpmCapturedPacketTrace(protocol,packetIter + burstIter,
                                                          packetBuff, packetLen, prefLen,
                                                          vfdPtr, prefIndex, needToTrace);
            }
        }
    }
}

/**
* @internal prvTgfTablesFillingLpmTransmit function
* @endinternal
*
* @brief   Generate traffic
*
* @param[in] portNum                  - port number to transmit traffic from
* @param[in] portInterfacePtr         - (pointet to) port interface to capturing
*                                       None
*/
static GT_VOID prvTgfTablesFillingLpmTransmit
(
    IN GT_U32                   portNum,
    IN CPSS_INTERFACE_INFO_STC *portInterfacePtr
)
{
    GT_STATUS rc = GT_OK;

    /* enable capture on nexthope port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(portInterfacePtr,
                                                TGF_CAPTURE_MODE_MIRRORING_E,
                                                GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
            "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, %d\n",
           portInterfacePtr->devPort.hwDevNum, portInterfacePtr->devPort.portNum);

    /* disable packet trace */
    rc = tgfTrafficTracePacketByteSet(GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
            "tgfTrafficTracePacketByteSet");

    /* transmit burstCount packets */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, portNum);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfStartTransmitingEth: %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_TX_0_IDX_CNS]);

    /* enable packet trace */
    rc = tgfTrafficTracePacketByteSet(GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTracePacketByteSet");
}

/**
* @internal prvTgfTablesFillingLpmBaseTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*
* @param[in] protocol                 - type of IP stack used
* @param[in] numPackets               - number of packets to transmit
* @param[in] burstCount               - number of packets to transmit in 1 burst
* @param[in] numRoutedPackets         - expected number of packets in capturing table
*                                       None
*/
static GT_VOID prvTgfTablesFillingLpmBaseTrafficGenerate
(
    IN CPSS_IP_PROTOCOL_STACK_ENT protocol,
    IN GT_U32                     numPackets,
    IN GT_U32                     burstCount,
    IN GT_U32                     numRoutedPackets
)
{
    GT_STATUS rc = GT_OK;
    TGF_VFD_INFO_STC vfdArray[1] = {{0}};
    TGF_PACKET_STC             *packetInfoPtr;
    CPSS_INTERFACE_INFO_STC    portInterface;
    GT_U32                     packetIter = 0;
    GT_U32                     numExpectedPackets = 0;
    GT_U32                  timeActuallyWait;/*time waited for all packets to get to the CPU*/


    PRV_UTF_LOG1_MAC("\nEach dot corresponds to %d successfully checked prefixes.", burstCount);

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* setup nexthope portInterface for capturing */
    portInterface.type            = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = prvTgfDevNum;
    portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_RX_1_IDX_CNS];

    /* set up vfdArray for using TGF_VFD_MODE_ARRAY_PATTERN_E */
    if (CPSS_IP_PROTOCOL_IPV4_E == protocol)
    {
        packetInfoPtr                  = &prvTgfPacketInfo;
        vfdArray[0].mode               = TGF_VFD_MODE_ARRAY_PATTERN_E;
        vfdArray[0].arrayPatternPtr    = prvTgfArIpv4->arIP;
        vfdArray[0].arrayPatternOffset = 0;
        vfdArray[0].arrayPatternSize   = sizeof(prvTgfPacketIpv4Part.dstAddr);
        vfdArray[0].cycleCount         = sizeof(prvTgfPacketIpv4Part.dstAddr);
        vfdArray[0].offset             = TGF_L2_HEADER_SIZE_CNS +
                                         TGF_ETHERTYPE_SIZE_CNS +
                                         TGF_IPV4_HEADER_SIZE_CNS -
                                         sizeof(prvTgfPacketIpv4Part.dstAddr);
    }
    else
    {
        packetInfoPtr                  = &prvTgfPacketIpv6Info;
        vfdArray[0].mode               = TGF_VFD_MODE_ARRAY_PATTERN_E;
        vfdArray[0].arrayPatternPtr    = prvTgfArIpv6->arIP;
        vfdArray[0].arrayPatternOffset = 0;
        vfdArray[0].arrayPatternSize   = sizeof(prvTgfPacketIpv6Part.dstAddr);
        vfdArray[0].cycleCount         = sizeof(prvTgfPacketIpv6Part.dstAddr);
        vfdArray[0].offset             = TGF_L2_HEADER_SIZE_CNS +
                                         TGF_ETHERTYPE_SIZE_CNS +
                                         TGF_IPV6_HEADER_SIZE_CNS -
                                         sizeof(prvTgfPacketIpv6Part.dstAddr);
    }

    /* setup transmit parameters */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, packetInfoPtr, burstCount, 1, vfdArray);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfSetTxSetupEth: %d", prvTgfDevNum);

    for (packetIter = 0; packetIter < numPackets; packetIter += burstCount)
    {
        /* last burst usually less than burstCount
         * fix burstCount for the last burst */
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

        /* set packet's number to transmit */
        prvTgfIpIndex = packetIter;
        vfdArray[0].arrayPatternOffset = packetIter;

        /* transmit packets */
        prvTgfTablesFillingLpmTransmit(prvTgfPortsArray[PRV_TGF_TX_0_IDX_CNS],
                                       &portInterface);

        /* allow the appdemo task to process the last captured packets */
        (void)tgfTrafficGeneratorCaptureLoopTimeSet(200);

        if (packetIter + burstCount <= numRoutedPackets)
        {
            /* scan whole capturing table and check burstCount expected packet */
            numExpectedPackets = burstCount;
        }
        else if (packetIter < numRoutedPackets)
        {
            /* prvTgfWrongPackets last packets are wrong (non routed) packets.
             * There are no IP prefixes for them. They not appear in captured table.
             * fix burstCount for them.
             */
            numExpectedPackets = numRoutedPackets - packetIter;

            PRV_UTF_LOG1_MAC("\nCheck last %d routed packets: ", numExpectedPackets);
        }

        if(numExpectedPackets)
        {
            rc = tgfTrafficGeneratorRxInCpuNumWait(numExpectedPackets,1000,&timeActuallyWait);
            if(rc == GT_OK)
            {
                /*don't let the function tgfTrafficGeneratorPortTxEthCaptureSet(...) do any extra sleep */
                (void)tgfTrafficGeneratorCaptureLoopTimeSet(1);

                if(numExpectedPackets > 1 && prvTgfTrafficPrintPacketTxEnableGet() == GT_TRUE)
                {
                    /*cpssOsPrintf*/
                    PRV_UTF_LOG2_MAC
                        ("[TGF]: prvTgfTablesFillingLpmBaseTrafficGenerate [%d] packets to CPU in [%d] milliseconds \n", numExpectedPackets,timeActuallyWait);
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

        if (numExpectedPackets > 0)
        {
            /* check numExpectedPackets in capturing table */
            prvTgfTablesFillingLpmCapturedTableCheck(protocol, packetIter, &portInterface,
                                                     numExpectedPackets, &vfdArray[0]);
        }

        /* clear capturing RxPcktTable */
        rc = tgfTrafficTableRxPcktTblClear();
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");
    }

    PRV_UTF_LOG0_MAC("\n");
}

/**
* @internal prvTgfTablesFillingLpmCountersReset function
* @endinternal
*
* @brief   Reset all counters used in the test
*
* @param[in] portNum                  - port number to reset IP counters
*                                       None
*/
static GT_VOID prvTgfTablesFillingLpmCountersReset
(
    IN GT_U32 portNum
)
{
    GT_STATUS rc = GT_OK;

    /* reset ETH counters */
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfEthCountersReset");

    /* reset IP counters */
    rc = prvTgfCountersIpSet(portNum, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCountersIpSet");

    /* reset prefix counters */
    prvTgfPrefixCounters.numMatched     = 0;
    prvTgfPrefixCounters.numCovered     = 0;
    prvTgfPrefixCounters.numDipErrors   = 0;
    prvTgfPrefixCounters.numLpmErrors   = 0;
    prvTgfPrefixCounters.numCoverErrors = 0;
}

/**
* @internal prvTgfTablesFillingLpmCountersCheck function
* @endinternal
*
* @brief   Generate traffic
*
* @param[in] numEntries               - general number of prefixes to check
* @param[in] numExpRoutedPackets      - expected number of routed packets
* @param[in] numExpNonRoutedPackets   - expected number of non routed packets
*                                       None
*/
static GT_VOID prvTgfTablesFillingLpmCountersCheck
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
     * 1. check Eth counters
     */

    /* read Eth counters for all receive ports */
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


    /*--------------------------------------------------------------------------
     * 2. check ip counters
     */

    /* get and print ip counters values for TX port */
    PRV_UTF_LOG1_MAC("IP counters on port [%d]:\n", prvTgfPortsArray[PRV_TGF_TX_0_IDX_CNS]);

    rc = prvTgfCountersIpGet(prvTgfDevNum, PRV_TGF_TX_0_IDX_CNS, GT_TRUE, &ipCounters);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCountersIpGet");

    /* check ip counters */
    UTF_VERIFY_EQUAL0_STRING_MAC( numExpNonRoutedPackets, ipCounters.inUcTrappedMirrorPkts,
                                  "got other number of inUcTrappedMirrorPkts than expected");


    /*--------------------------------------------------------------------------
     * 3. check prefix counters
     */

    PRV_UTF_LOG0_MAC("Prefix counters:\n");

    if (prvTgfPrefixCounters.numMatched > 0)
    {
        PRV_UTF_LOG1_MAC("  %d prefixes were matched properly\n",
                         prvTgfPrefixCounters.numMatched);
    }

    if (prvTgfPrefixCounters.numCovered > 0)
    {
        PRV_UTF_LOG1_MAC("  %d prefixes were covered by LPM properly\n",
                         prvTgfPrefixCounters.numCovered);
    }

    if (prvTgfPrefixCounters.numMatched == 0 && prvTgfPrefixCounters.numCovered == 0)
    {
        PRV_UTF_LOG0_MAC("  NO ROUTED PACKETS\n");
    }

    UTF_VERIFY_EQUAL1_STRING_MAC(0, prvTgfPrefixCounters.numLpmErrors,
                                 "  %d longer prefixes were covered by smaller one\n",
                                 prvTgfPrefixCounters.numLpmErrors);

    UTF_VERIFY_EQUAL1_STRING_MAC(0, prvTgfPrefixCounters.numCoverErrors,
                                 "  %d prefixes were covered by LPM NOT properly\n",
                                 prvTgfPrefixCounters.numCoverErrors);

    UTF_VERIFY_EQUAL1_STRING_MAC(0, prvTgfPrefixCounters.numDipErrors,
                                 "  %d packets with wrong DIP were detected\n",
                                 prvTgfPrefixCounters.numDipErrors);

    PRV_UTF_LOG0_MAC("\n");
}


/******************************************************************************\
 *                            Public test functions                           *
\******************************************************************************/

/**
* @internal prvTgfTablesFillingLpmConfigurationSet function
* @endinternal
*
* @brief   Set common configuration
*/
GT_VOID prvTgfTablesFillingLpmConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS                     rc = GT_OK;
    PRV_TGF_BRG_MAC_ENTRY_STC     macEntry;
    PRV_TGF_IP_UC_ROUTE_ENTRY_STC routeEntriesArray[1];
    GT_ETHERADDR                  arpMacAddr;
    GT_U8                         iter;

    /* create SEND VLAN 5 on ports (0) */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_SEND_VLANID_CNS,
                                           prvTgfPortsArray, NULL, NULL, 1);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "prvTgfBrgDefVlanEntryWithPortsSet: %d", prvTgfDevNum);

    /* create NEXTHOPE VLAN 6 on ports (8,18,23) */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_NEXTHOPE_VLANID_CNS,
                                           prvTgfPortsArray + 1, NULL, NULL, 3);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "prvTgfBrgDefVlanEntryWithPortsSet: %d", prvTgfDevNum);

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
    rc = prvTgfBrgVlanPortVidGet(prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_TX_0_IDX_CNS], &prvTgfRestoreCfg.vid);
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

    /* Create 128 ARP MAC addresses in the Router ARP Table
     * and 128 UC Route entries in the Route Table.
     * All packets will have the same destination port,
     * but different prefix lengths will correspond
     * to different route entries and ARP addresses.
     */
    cpssOsMemCpy(arpMacAddr.arEther, prvTgfArpMac, sizeof(TGF_MAC_ADDR));
    cpssOsMemSet(routeEntriesArray, 0, sizeof(routeEntriesArray));

    routeEntriesArray[0].cmd                   = CPSS_PACKET_CMD_ROUTE_E;
    routeEntriesArray[0].nextHopVlanId         = PRV_TGF_NEXTHOPE_VLANID_CNS;
    routeEntriesArray[0].nextHopInterface.type = CPSS_INTERFACE_PORT_E;
    routeEntriesArray[0].nextHopInterface.devPort.hwDevNum  = prvTgfDevNum;
    routeEntriesArray[0].nextHopInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_RX_1_IDX_CNS];

    for (iter = 0; iter < 128; iter++)
    {
        /* set MAC address and ARP index */
        arpMacAddr.arEther[5] = iter;
        routeEntriesArray[0].nextHopARPPointer = prvTgfArpEntryBaseIndex + iter;

        /* write ARP entry */
        rc = prvTgfIpRouterArpAddrWrite(prvTgfArpEntryBaseIndex + iter, &arpMacAddr);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpRouterArpAddrWrite: %d", prvTgfDevNum);

        /* write UC Route entry */
        rc = prvTgfIpUcRouteEntriesWrite(prvTgfRouteEntryBaseIndex + iter,
                                         routeEntriesArray, 1);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpUcRouteEntriesWrite: %d", prvTgfDevNum);
    }
}

/**
* @internal prvTgfTablesFillingLpmTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*
* @param[in] protocol                 - type of IP stack used
*                                      incValue - value for increment ip addresses
*                                      if 0 the test generate random prefixes
* @param[in] numLoops                 - number of loops of the test to discover
*                                      a memory leak and so on
*                                       None
*/
GT_VOID prvTgfTablesFillingLpmTrafficGenerate
(
    IN CPSS_IP_PROTOCOL_STACK_ENT protocol,
    IN GT_U32                     incVulue,
    IN GT_U32                     numLoops
)
{
    GT_STATUS rc = GT_OK;
    GT_U32    numPrefixes = 0;
    GT_U32    numErrors  = 0;
    GT_U32    firstError = 0;
    GT_U32    burstCount = 1000;

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

    PRV_UTF_LOG1_MAC("\n===> Loop %d:\n", numLoops);

    /* check LPM prefixes */
    while (numLoops--)
    {
        /* get number of prefixs to add from each protocol type */
        rc = prvTgfTableFillingLpmGetNumPrefixesToAdd(&prvTgfNumIpv4,&prvTgfNumIpv6);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTableFillingLpmGetMaxPrefixes");
        if (rc != GT_OK)
            return;

        numPrefixes = (CPSS_IP_PROTOCOL_IPV4_E == protocol) ? prvTgfNumIpv4 : prvTgfNumIpv6;

        /* how many IPv4 or IPv6 prefixes is allowed to add to LPM DB */
        PRV_UTF_LOG1_MAC("===> LPM DB allows to allocate %d IPv4 Prefixes\n", prvTgfNumIpv4);
        PRV_UTF_LOG1_MAC("===> LPM DB allows to allocate %d IPv6 Prefixes\n", prvTgfNumIpv6);

        /* allocate arrays of IP addresses and lengths */
        prvTgfArIpv4 = (GT_IPADDR*) cpssOsMalloc((prvTgfNumIpv4 + prvTgfWrongPacketsDefault) * sizeof(GT_IPADDR));
        prvTgfArMsk4 = (GT_IPADDR*) cpssOsMalloc((prvTgfNumIpv4 + prvTgfWrongPacketsDefault) * sizeof(GT_IPADDR));
        prvTgfArLen4 = (GT_U8*)     cpssOsMalloc((prvTgfNumIpv4 + prvTgfWrongPacketsDefault) * sizeof(GT_U8));

        prvTgfArIpv6 = (GT_IPV6ADDR*) cpssOsMalloc((prvTgfNumIpv6 + prvTgfWrongPacketsDefault) * sizeof(GT_IPV6ADDR));
        prvTgfArMsk6 = (GT_IPV6ADDR*) cpssOsMalloc((prvTgfNumIpv6 + prvTgfWrongPacketsDefault) * sizeof(GT_IPV6ADDR));
        prvTgfArLen6 = (GT_U8*)       cpssOsMalloc((prvTgfNumIpv6 + prvTgfWrongPacketsDefault) * sizeof(GT_U8));

        /* set current next ip mode and length mode
         * use different modes only to fill array
         * but we will always use array to create prefixes and transmit packets */
        switch (incVulue)
        {
            case 0:
                prvTgfNextIpMode     = PRV_TGF_NEXT_MODE_RANDOM_E;
                prvTgfNextLengthMode = PRV_TGF_NEXT_MODE_RANDOM_E;
                prvTgfWrongPackets   = 0;

                break;
            case 1:
                prvTgfNextIpMode     = PRV_TGF_NEXT_MODE_INC_E;
                prvTgfNextLengthMode = PRV_TGF_NEXT_MODE_STATIC_E;
                prvTgfIncValue       = 1;
                prvTgfWrongPackets   = prvTgfWrongPacketsDefault;

                break;
            default:
                prvTgfNextIpMode     = PRV_TGF_NEXT_MODE_INC_E;
                prvTgfNextLengthMode = PRV_TGF_NEXT_MODE_STATIC_E;
                prvTgfIncValue       = incVulue;
                prvTgfWrongPackets   = 0;
        }

        /*----------------------------------------------------------------------
         * 1. create prefixes
         */
        prvTgfTablesFillingLpmIpCreate(protocol, numPrefixes + prvTgfWrongPackets);

        /* additionally create IPv4 UC prefixes if we are in mixed mode */
        if (CPSS_IP_PROTOCOL_IPV4V6_E == protocol)
        {
            prvTgfTablesFillingLpmIpCreate(CPSS_IP_PROTOCOL_IPV4_E,
                                           prvTgfNumIpv4 + prvTgfWrongPackets);
        }

        /* always use array for any mode to create prefixes and transmit packets */
        prvTgfNextIpMode     = PRV_TGF_NEXT_MODE_RANDOM_E;
        prvTgfNextLengthMode = PRV_TGF_NEXT_MODE_RANDOM_E;

        /*----------------------------------------------------------------------
         * 2. fill LPM DB by prefixes
         */
        rc = prvTgfTablesFillingLpmAdd(protocol, numPrefixes, &numErrors, &firstError);
        UTF_VERIFY_EQUAL4_STRING_MAC(0, numErrors,
                "prvTgfTablesFillingLpmAdd"
                "\n       could successfully add only %d prefixes !!!"
                "\n       It has not handled %d prefixes."
                "\n       First error was happened after adding of %d prefix"
                "\n       rc = %d",
                (numPrefixes - numErrors), numErrors, firstError, rc);

        /* additionally fill LPM DB by IPv4 UC prefixes if we are in mixed mode */
        if (CPSS_IP_PROTOCOL_IPV4V6_E == protocol)
        {
            rc = prvTgfTablesFillingLpmAdd(CPSS_IP_PROTOCOL_IPV4_E, prvTgfNumIpv4,
                                           &numErrors, &firstError);
            UTF_VERIFY_EQUAL4_STRING_MAC(0, numErrors,
                    "prvTgfTablesFillingLpmAdd"
                    "\n       could successfully add only %d IPv4 UC prefixes !!!"
                    "\n       It has not handled %d prefixes."
                    "\n       First error was happened after adding of %d IPv4 UC prefix"
                    "\n       rc = %d",
                    (numPrefixes - numErrors), numErrors, firstError, rc);
        }


        /*----------------------------------------------------------------------
         * 3. check prefixes after creating
         */

        /* check prefixes */
        PRV_UTF_LOG3_MAC("\nTo check %d prefixes after creating,\n"
                         "  transmit %d matched and %d unmatched packets\n",
                         numPrefixes, numPrefixes, prvTgfWrongPackets);

        /* reset all counters */
        prvTgfTablesFillingLpmCountersReset(prvTgfPortsArray[PRV_TGF_TX_0_IDX_CNS]);

        /* generate traffic to check existed prefixes */
        prvTgfTablesFillingLpmBaseTrafficGenerate(protocol, numPrefixes + prvTgfWrongPackets,
                                                  burstCount, numPrefixes);

        /* check all counters */
        prvTgfTablesFillingLpmCountersCheck(numPrefixes, numPrefixes, prvTgfWrongPackets);

        /* additionally check IPv4 UC prefixes if we are in mixed mode */
        if (CPSS_IP_PROTOCOL_IPV4V6_E == protocol)
        {
            /* reset all counters */
            prvTgfTablesFillingLpmCountersReset(prvTgfPortsArray[PRV_TGF_TX_0_IDX_CNS]);

            /* generate traffic to check existed prefixes */
            prvTgfTablesFillingLpmBaseTrafficGenerate(CPSS_IP_PROTOCOL_IPV4_E,
                                                      prvTgfNumIpv4 + prvTgfWrongPackets,
                                                      burstCount, prvTgfNumIpv4);

            /* check all counters */
            prvTgfTablesFillingLpmCountersCheck(prvTgfNumIpv4, prvTgfNumIpv4,
                                                prvTgfWrongPackets);
        }

        /*----------------------------------------------------------------------
         * 4. flush all UC prefixes in LPM DB
         */
        rc = prvTgfTablesFillingLpmUcPrefixesFlush(protocol);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTablesFillingLpmUcPrefixesFlush");

        /* additionally flush IPv4 UC prefixes if we are in mixed mode */
        if (CPSS_IP_PROTOCOL_IPV4V6_E == protocol)
        {
            /* Flush all IPv4 UC prefixes in LPM DB */
            rc = prvTgfTablesFillingLpmUcPrefixesFlush(CPSS_IP_PROTOCOL_IPV4_E);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTablesFillingLpmUcPrefixesFlush");
        }


        /*----------------------------------------------------------------------
         * 5. check prefixes after deleting
         */

        /* check prefixes */
        PRV_UTF_LOG3_MAC("\nTo check %d prefixes after deleting,\n"
                         "  transmit %d matched and %d unmatched packets\n",
                         numPrefixes, numPrefixes, prvTgfWrongPackets);

        /* reset all counters */
        prvTgfTablesFillingLpmCountersReset(prvTgfPortsArray[PRV_TGF_TX_0_IDX_CNS]);

        /* generate traffic to check deleted prefixes*/
        prvTgfTablesFillingLpmBaseTrafficGenerate(protocol, numPrefixes + prvTgfWrongPackets,
                                                  burstCount, 0);

        /* check counters */
        prvTgfTablesFillingLpmCountersCheck(numPrefixes, 0, numPrefixes + prvTgfWrongPackets);

        /* additionally check IPv4 UC prefixes if we are in mixed mode */
        if (CPSS_IP_PROTOCOL_IPV4V6_E == protocol)
        {
            /* reset all counters */
            prvTgfTablesFillingLpmCountersReset(prvTgfPortsArray[PRV_TGF_TX_0_IDX_CNS]);

            /* generate traffic to check deleted prefixes*/
            prvTgfTablesFillingLpmBaseTrafficGenerate(CPSS_IP_PROTOCOL_IPV4_E,
                                                      prvTgfNumIpv4 + prvTgfWrongPackets,
                                                      burstCount, 0);

            /* check counters */
            prvTgfTablesFillingLpmCountersCheck(prvTgfNumIpv4, 0, prvTgfNumIpv4 + prvTgfWrongPackets);
        }

        /* free arrays of IP addresses and lengths */
        cpssOsFree(prvTgfArIpv4);
        cpssOsFree(prvTgfArMsk4);
        cpssOsFree(prvTgfArLen4);

        cpssOsFree(prvTgfArIpv6);
        cpssOsFree(prvTgfArMsk6);
        cpssOsFree(prvTgfArLen6);
    }
}

/**
* @internal prvTgfTablesFillingLpmConfigurationRestore function
* @endinternal
*
* @brief   Restore configuration
*/
GT_VOID prvTgfTablesFillingLpmConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS rc;

    /* disable IPv4 and IPv6 Unicast Routing on a Port */
    rc = prvTgfIpPortRoutingEnable(PRV_TGF_TX_0_IDX_CNS, CPSS_IP_UNICAST_E,
                                   CPSS_IP_PROTOCOL_IPV4V6_E, GT_FALSE);
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



