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
* @file cpssPxTgfIngressIpFilteringUT.c
*
* @brief Ingress IP filtering test.
*
* @version   1
********************************************************************************
*/
/* includes */
#include <cpss/px/cpssPxTypes.h>
#include <cpss/px/ingress/cpssPxIngress.h>
#include <cpss/px/networkIf/cpssPxNetIf.h>
#include <utfTraffic/basic/cpssPxTgfBasicUT.h>

#define IP_CLASS_PORT1 (7)
#define IP_CLASS_PORT2 (8)
#define IP_CLASS_PORT3 (9)
#define IP_CLASS_PORT4 (10)
#define PORT_MAP_ENTRY_MAX_INDEX (_4K - 1)

static GT_U8 packetPayload[] =
     {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, /*payload  */
      0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
      0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23,
      0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
      0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B,
      0x3C, 0x3D, 0x3E, 0x3F};

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part[] = {
    {{0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   /* daMac = macGrp */
     {0x00, 0x00, 0x00, 0x00, 0x00, 0x22}},  /* saMac */
    {{0x01, 0x01, 0x01, 0x01, 0x01, 0x01},   /* daMac = macGrp */
     {0x00, 0x00, 0x00, 0x00, 0x00, 0x22}},  /* saMac */
    {{0x02, 0x02, 0x02, 0x02, 0x02, 0x02},   /* daMac = macGrp */
     {0x00, 0x00, 0x00, 0x00, 0x00, 0x22}},  /* saMac */
    {{0x03, 0x03, 0x03, 0x03, 0x03, 0x03},   /* daMac = macGrp */
     {0x00, 0x00, 0x00, 0x00, 0x00, 0x22}}   /* saMac */
};

/* packet's ethertype */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketIpv4EtherTypePart = {
    TGF_ETHERTYPE_0800_IPV4_TAG_CNS
};
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketIpv6EtherTypePart = {
    TGF_ETHERTYPE_86DD_IPV6_TAG_CNS
};


/* packet 1 (IPv4) */
static TGF_PACKET_IPV4_STC prvTgfPacket1Ipv4Part = {
    4,                                 /* version */
    5,                                 /* headerLen */
    0x3F,                              /* typeOfService */
    0x2A,                              /* totalLen = 42 */
    0,                                 /* id */
    0,                                 /* flags */
    0,                                 /* offset */
    0x40,                              /* timeToLive = 64 */
    0x04,                              /* protocol */
    TGF_PACKET_AUTO_CALC_CHECKSUM_CNS, /* csum */
    { 1,  1,  1,  1},                  /* srcAddr = ipSrc */
    { 1,  2,  3,  4}                   /* dstAddr = ipGrp */
};

/* packet 2 (IPv4) */
static TGF_PACKET_IPV4_STC prvTgfPacket2Ipv4Part = {
    4,                                 /* version */
    5,                                 /* headerLen */
    0x00,                              /* typeOfService */
    0x2A,                              /* totalLen = 42 */
    0,                                 /* id */
    0,                                 /* flags */
    0,                                 /* offset */
    0x40,                              /* timeToLive = 64 */
    0x04,                              /* protocol */
    TGF_PACKET_AUTO_CALC_CHECKSUM_CNS, /* csum */
    { 1,  1,  1,  1},                  /* srcAddr = ipSrc */
    { 224,  0,  0,  1}                 /* dstAddr = ipGrp */
};

static TGF_PACKET_IPV6_STC prvTgfPacket3Ipv6Part = {
    6,                  /* version */
    (0x15 << 2),        /* DSCP */
    0,                  /* flowLabel */
    0x40,               /* payloadLen */
    0x3b,               /* nextHeader */
    0x40,               /* hopLimit */
    {0xFFFF, 0, 0, 0, 0, 0, 0, 0x3333}, /* TGF_IPV6_ADDR srcAddr */
    {1, 2, 3, 4, 5, 6, 7, 8}            /* TGF_IPV6_ADDR dstAddr */
};

static TGF_PACKET_IPV6_STC prvTgfPacket4Ipv6Part = {
    6,                  /* version */
    (0x2A << 2),        /* DSCP */
    0,                  /* flowLabel */
    0x40,               /* payloadLen */
    0x3b,               /* nextHeader */
    0x40,               /* hopLimit */
    {0xFFFF, 0, 0, 0, 0, 0, 0, 0x3333}, /* TGF_IPV6_ADDR srcAddr */
    {0xFF02, 0, 0, 0, 0, 0, 0, 1}       /* TGF_IPV6_ADDR dstAddr */
};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart = {
    sizeof(packetPayload),                       /* dataLength */
    packetPayload                                /* dataPtr */
};

/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfPacket1PartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part[0]},  /* type, partPtr */
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketIpv4EtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacket1Ipv4Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

static TGF_PACKET_PART_STC prvTgfPacket2PartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part[1]},  /* type, partPtr */
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketIpv4EtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacket2Ipv4Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

static TGF_PACKET_PART_STC prvTgfPacket3PartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part[2]},  /* type, partPtr */
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketIpv6EtherTypePart},
    {TGF_PACKET_PART_IPV6_E,      &prvTgfPacket3Ipv6Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

static TGF_PACKET_PART_STC prvTgfPacket4PartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part[3]},  /* type, partPtr */
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketIpv6EtherTypePart},
    {TGF_PACKET_PART_IPV6_E,      &prvTgfPacket4Ipv6Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* LENGTH of packet */
#define PRV_TGF_PACKET_IPV4_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS \
    + TGF_IPV4_HEADER_SIZE_CNS + sizeof(packetPayload)

#define PRV_TGF_PACKET_IPV6_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS \
    + TGF_IPV6_HEADER_SIZE_CNS + sizeof(packetPayload)

/* PACKET1 to send */
static TGF_PACKET_STC prvTgfPacket1Info = {
    PRV_TGF_PACKET_IPV4_LEN_CNS,                                  /* totalLen */
    sizeof(prvTgfPacket1PartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacket1PartArray                                        /* partsArray */
};

/* PACKET2 to send */
static TGF_PACKET_STC prvTgfPacket2Info = {
    PRV_TGF_PACKET_IPV4_LEN_CNS,                                  /* totalLen */
    sizeof(prvTgfPacket2PartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacket2PartArray                                        /* partsArray */
};

/* PACKET3 to send */
static TGF_PACKET_STC prvTgfPacket3Info = {
    PRV_TGF_PACKET_IPV6_LEN_CNS,                                  /* totalLen */
    sizeof(prvTgfPacket3PartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacket3PartArray                                        /* partsArray */
};

/* PACKET4 to send */
static TGF_PACKET_STC prvTgfPacket4Info = {
    PRV_TGF_PACKET_IPV6_LEN_CNS,                                  /* totalLen */
    sizeof(prvTgfPacket4PartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacket4PartArray                                        /* partsArray */
};


/**
* @internal prvTgfPxIpClassificationCountersCheck function
* @endinternal
*
* @brief   Counters check
*
* @param[in] dev                      - device number
* @param[in] portNum                  - port number
* @param[in] counterType              - counter type
* @param[in] value                    - expected counter value
*                                       None
*/
static void prvTgfPxIpClassificationCountersCheck
(
    GT_SW_DEV_NUM                                     dev,
    GT_PORT_NUM                                       portNum,
    CPSS_PX_PORT_MAC_COUNTER_ENT                      counterType,
    GT_U32                                            value
)
{
    CPSS_PX_PORT_MAC_COUNTERS_STC   portCounters;
    GT_STATUS st;

    if(cpssDeviceRunCheck_onEmulator())
    {
        cpssOsTimerWkAfter(1000);
    }

    st = cpssPxPortMacCountersOnPortGet(dev, portNum, &portCounters);
    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, portNum);
    UTF_VERIFY_EQUAL1_PARAM_MAC(value, portCounters.mibCounter[counterType].l[0],
        portNum);
}

#define TGF_WAIT_PACKET \
while(1)\
{\
    if(tgfTrafficGeneratorCheckProcessignDone(GT_FALSE, 0))\
    {\
        break;\
    }\
}

/**
* @internal prvTgfPxIpClassificationExecute function
* @endinternal
*
* @brief   Execute IP address classification test.
*
* @param[in] dev                      - device number
*                                       None
*/
void prvTgfPxIpClassificationExecute
(
    GT_U8 dev
)
{
    GT_STATUS st;
    GT_U8                           *buffList[1];
    GT_U32                          buffLenList[1];
    TGF_AUTO_CHECKSUM_FULL_INFO_STC checksumInfo[TGF_AUTO_CHECKSUM_NUM_OF_HEADERS_CNS];
    CPSS_PX_NET_TX_PARAMS_STC       pcktParams;

     if(cpssDeviceRunCheck_onEmulator())
     {
        cpssOsTimerWkAfter(50);
     }

    buffList[0] = cpssOsCacheDmaMalloc(TGF_RX_BUFFER_MAX_SIZE_CNS);
    pcktParams.txQueue = 0;
    pcktParams.recalcCrc = GT_TRUE;

    /* AUTODOC: Packet #1
       DA=0:0:0:0:0:0
       EtherType = 0x0800
       DIP = 1.2.3.4
       DSCP = 0x3F
       Expected: Egress on port 1
    */
    
    cpssOsMemSet(&checksumInfo[0], 0, sizeof(checksumInfo)); 
    
    buffLenList[0] = TGF_RX_BUFFER_MAX_SIZE_CNS - 4; 
    st = tgfTrafficEnginePacketBuild(&prvTgfPacket1Info, buffList[0], 
                                     &buffLenList[0], checksumInfo, NULL); 
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st); 
    buffLenList[0] += TGF_CRC_LEN_CNS; 
    st = cpssPxNetIfSdmaSyncTxPacketSend(dev, &pcktParams, buffList, 
                                         buffLenList, 1); 
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st); 
    TGF_WAIT_PACKET
    prvTgfPxIpClassificationCountersCheck(dev, IP_CLASS_PORT1, 
                                          CPSS_PX_PORT_MAC_COUNTER_GOOD_OCTETS_SENT_E, 102); 
    prvTgfPxIpClassificationCountersCheck(dev, IP_CLASS_PORT2, 
                                          CPSS_PX_PORT_MAC_COUNTER_GOOD_OCTETS_SENT_E, 0); 
    prvTgfPxIpClassificationCountersCheck(dev, IP_CLASS_PORT3, 
                                          CPSS_PX_PORT_MAC_COUNTER_GOOD_OCTETS_SENT_E, 0); 
    prvTgfPxIpClassificationCountersCheck(dev, IP_CLASS_PORT4, 
                                          CPSS_PX_PORT_MAC_COUNTER_GOOD_OCTETS_SENT_E, 0); 

    /* AUTODOC: Packet #2
       DA=1:1:1:1:1:1
       EtherType = 0x0800
       DIP = 224.0.0.1
       DSCP = 0x00
       Expected: Egress on port 2
    */
    buffLenList[0] = TGF_RX_BUFFER_MAX_SIZE_CNS - 4; 
    st = tgfTrafficEnginePacketBuild(&prvTgfPacket2Info, buffList[0], 
                                     &buffLenList[0], checksumInfo, NULL); 
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st); 
    buffLenList[0] += TGF_CRC_LEN_CNS; 
    st = cpssPxNetIfSdmaSyncTxPacketSend(dev, &pcktParams, buffList, 
                                         buffLenList, 1); 
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st); 
    TGF_WAIT_PACKET
    prvTgfPxIpClassificationCountersCheck(dev, IP_CLASS_PORT1, 
                                          CPSS_PX_PORT_MAC_COUNTER_GOOD_OCTETS_SENT_E, 0); 
    prvTgfPxIpClassificationCountersCheck(dev, IP_CLASS_PORT2, 
                                          CPSS_PX_PORT_MAC_COUNTER_GOOD_OCTETS_SENT_E, 102); 
    prvTgfPxIpClassificationCountersCheck(dev, IP_CLASS_PORT3, 
                                          CPSS_PX_PORT_MAC_COUNTER_GOOD_OCTETS_SENT_E, 0); 
    prvTgfPxIpClassificationCountersCheck(dev, IP_CLASS_PORT4, 
                                          CPSS_PX_PORT_MAC_COUNTER_GOOD_OCTETS_SENT_E, 0); 
    
    /* AUTODOC: Packet #3
       DA=2:2:2:2:2:2
       EtherType = 0x86DD
       DIP = 0001:0002:0003:0004:0005:0006:0007:0008
       DSCP = 0x15
       Expected: Egress on port 3
    */
    buffLenList[0] = TGF_RX_BUFFER_MAX_SIZE_CNS - 4; 
    st = tgfTrafficEnginePacketBuild(&prvTgfPacket3Info, buffList[0],
                                     &buffLenList[0], checksumInfo, NULL); 
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st); 
    buffLenList[0]+= TGF_CRC_LEN_CNS; 
    st = cpssPxNetIfSdmaSyncTxPacketSend(dev, &pcktParams, buffList,
                                         buffLenList, 1); 
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st); 
    TGF_WAIT_PACKET
    prvTgfPxIpClassificationCountersCheck(dev, IP_CLASS_PORT1, 
                                          CPSS_PX_PORT_MAC_COUNTER_GOOD_OCTETS_SENT_E, 0); 
    prvTgfPxIpClassificationCountersCheck(dev, IP_CLASS_PORT2, 
                                          CPSS_PX_PORT_MAC_COUNTER_GOOD_OCTETS_SENT_E, 0); 
    prvTgfPxIpClassificationCountersCheck(dev, IP_CLASS_PORT3, 
                                          CPSS_PX_PORT_MAC_COUNTER_GOOD_OCTETS_SENT_E, 122); 
    prvTgfPxIpClassificationCountersCheck(dev, IP_CLASS_PORT4, 
                                          CPSS_PX_PORT_MAC_COUNTER_GOOD_OCTETS_SENT_E, 0); 
    
    /* AUTODOC: Packet #4
       DA=3:3:3:3:3:3
       EtherType = 0x86DD
       DIP = FF02::1
       DSCP = 0x2A
       Expected: Egress on port 4
    */
    buffLenList[0] = TGF_RX_BUFFER_MAX_SIZE_CNS - 4; 
    st = tgfTrafficEnginePacketBuild(&prvTgfPacket4Info, buffList[0],
                                     &buffLenList[0], checksumInfo, NULL); 
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st); 
    buffLenList[0]+= TGF_CRC_LEN_CNS; 
    st = cpssPxNetIfSdmaSyncTxPacketSend(dev, &pcktParams, buffList,
                                         buffLenList, 1); 
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st); 
    TGF_WAIT_PACKET
    prvTgfPxIpClassificationCountersCheck(dev, IP_CLASS_PORT1, 
                                          CPSS_PX_PORT_MAC_COUNTER_GOOD_OCTETS_SENT_E, 0); 
    prvTgfPxIpClassificationCountersCheck(dev, IP_CLASS_PORT2, 
                                          CPSS_PX_PORT_MAC_COUNTER_GOOD_OCTETS_SENT_E, 0); 
    prvTgfPxIpClassificationCountersCheck(dev, IP_CLASS_PORT3, 
                                          CPSS_PX_PORT_MAC_COUNTER_GOOD_OCTETS_SENT_E, 0); 
    prvTgfPxIpClassificationCountersCheck(dev, IP_CLASS_PORT4, 
                                          CPSS_PX_PORT_MAC_COUNTER_GOOD_OCTETS_SENT_E, 122); 

    cpssOsCacheDmaFree(buffList[0]);
}

/**
* @internal prvTgfPxIpClassificationConfig function
* @endinternal
*
* @brief   Save or restore IP classification original configuration
*
* @param[in] dev                      - device number
* @param[in] configure                - GT_TRUE - save configuration
*                                      GT_FALSE - restore configuration
*                                       None
*/
void prvTgfPxIpClassificationConfig
(
    GT_U8   dev,
    GT_BOOL configure
)
{
    GT_STATUS st;
    GT_U32 ii;
    static CPSS_PX_INGRESS_IP2ME_ENTRY_STC ip2meEntriesRestore[5];
    CPSS_PX_INGRESS_IP2ME_ENTRY_STC ip2meEntry;
    static CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC packetTypekeyPatternRestore[4];
    static CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC packetTypekeyMaskRestore[4];
    CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC ptKeyPattern;
    CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC ptKeyMask;
    static CPSS_PX_INGRESS_PORT_PACKET_TYPE_KEY_STC profileConfigRestore;
    CPSS_PX_INGRESS_PORT_PACKET_TYPE_KEY_STC profileConfig;
    static CPSS_PX_INGRESS_PORT_MAP_PACKET_TYPE_FORMAT_STC     packetTypeFormatRestore[4];
    CPSS_PX_INGRESS_PORT_MAP_PACKET_TYPE_FORMAT_STC packetTypeFormat;
    static CPSS_PX_PORTS_BMP dstPortBmpRestore[4];
    static GT_BOOL lagEnRestore[4];
    static GT_BOOL portEnableRestore[4];
    static GT_BOOL egrPortFilteringRerstore[4];

    if(GT_TRUE == configure)
    {
        /* AUTODOC: Save first 4 entries of IP2ME table */
        for(ii = 1; ii < 5; ii++)
        {
            st = cpssPxIngressIp2MeEntryGet(dev, ii, &ip2meEntriesRestore[ii]);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, ii);
        }

        /* AUTODOC: Set first 4 entries of IP2ME table */
        /*
            Index Valid IPver. IP address                              Prefix
            1     1     4      1.2.3.4                                 32
            2     1     4      224.0.0.1                               24
            3     1     6      0001:0002:0003:0004:0005:0006:0007:0008 128
            4     1     6      FF02::0                                 16
        */
        ip2meEntry.valid = GT_TRUE;
        ip2meEntry.prefixLength = 32;
        ip2meEntry.isIpv6 = GT_FALSE;
        ip2meEntry.ipAddr.ipv4Addr.arIP[0] = 1;
        ip2meEntry.ipAddr.ipv4Addr.arIP[1] = 2;
        ip2meEntry.ipAddr.ipv4Addr.arIP[2] = 3;
        ip2meEntry.ipAddr.ipv4Addr.arIP[3] = 4;
        st = cpssPxIngressIp2MeEntrySet(dev, 1, &ip2meEntry);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, ii);

        ip2meEntry.valid = GT_TRUE;
        ip2meEntry.prefixLength = 24;
        ip2meEntry.isIpv6 = GT_FALSE;
        ip2meEntry.ipAddr.ipv4Addr.arIP[0] = 224;
        ip2meEntry.ipAddr.ipv4Addr.arIP[1] = 0;
        ip2meEntry.ipAddr.ipv4Addr.arIP[2] = 0;
        ip2meEntry.ipAddr.ipv4Addr.arIP[3] = 0;
        st = cpssPxIngressIp2MeEntrySet(dev, 2, &ip2meEntry);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, ii);

        ip2meEntry.valid = GT_TRUE;
        ip2meEntry.prefixLength = 128;
        ip2meEntry.isIpv6 = GT_TRUE;
        ip2meEntry.ipAddr.ipv6Addr.u32Ip[0] = CPSS_32BIT_BE(0x00010002);
        ip2meEntry.ipAddr.ipv6Addr.u32Ip[1] = CPSS_32BIT_BE(0x00030004);
        ip2meEntry.ipAddr.ipv6Addr.u32Ip[2] = CPSS_32BIT_BE(0x00050006);
        ip2meEntry.ipAddr.ipv6Addr.u32Ip[3] = CPSS_32BIT_BE(0x00070008);
        st = cpssPxIngressIp2MeEntrySet(dev, 3, &ip2meEntry);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, ii);

        ip2meEntry.valid = GT_TRUE;
        ip2meEntry.prefixLength = 16;
        ip2meEntry.isIpv6 = GT_TRUE;
        ip2meEntry.ipAddr.ipv6Addr.u32Ip[0] = CPSS_32BIT_BE(0xFF020000);
        ip2meEntry.ipAddr.ipv6Addr.u32Ip[1] = CPSS_32BIT_BE(0x00000000);
        ip2meEntry.ipAddr.ipv6Addr.u32Ip[2] = CPSS_32BIT_BE(0x00000000);
        ip2meEntry.ipAddr.ipv6Addr.u32Ip[3] = CPSS_32BIT_BE(0x00000000);
        st = cpssPxIngressIp2MeEntrySet(dev, 4, &ip2meEntry);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, ii);

        /* AUTODOC: Save packet type key entries */
        for(ii = 0; ii < 4; ii++)
        {
            st = cpssPxIngressPacketTypeKeyEntryGet(dev, ii,
                &packetTypekeyPatternRestore[ii], &packetTypekeyMaskRestore[ii]);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, ii);
        }

        /* AUTODOC: Configure packet type key entries */
        /*
            Index DA          isLLC UDBP0(DSCP)     IP2ME idx
            0     0:0:0:0:0:0 0     0x003F M:0x00FC 1
            1     1:1:1:1:1:1 0     0x0000 M:0x00FC 2
            2     2:2:2:2:2:2 0     0x15 M:0x0FC0   3
            3     3:3:3:3:3:3 0     0x2A M:0x0FC0   4
        */
        cpssOsMemSet(&ptKeyPattern, 0, sizeof(ptKeyPattern));
        cpssOsMemSet(&ptKeyMask, 0, sizeof(ptKeyMask));
        ptKeyPattern.macDa.arEther[0] = 0x00;
        ptKeyPattern.macDa.arEther[1] = 0x00;
        ptKeyPattern.macDa.arEther[2] = 0x00;
        ptKeyPattern.macDa.arEther[3] = 0x00;
        ptKeyPattern.macDa.arEther[4] = 0x00;
        ptKeyPattern.macDa.arEther[5] = 0x00;
        ptKeyMask.macDa.arEther[0] = 0xFF;
        ptKeyMask.macDa.arEther[1] = 0xFF;
        ptKeyMask.macDa.arEther[2] = 0xFF;
        ptKeyMask.macDa.arEther[3] = 0xFF;
        ptKeyMask.macDa.arEther[4] = 0xFF;
        ptKeyMask.macDa.arEther[5] = 0xFF;
        ptKeyPattern.isLLCNonSnap = GT_FALSE;
        ptKeyMask.isLLCNonSnap = GT_TRUE;
        ptKeyPattern.ip2meIndex = 1;
        ptKeyMask.ip2meIndex = 0x7;
        ptKeyPattern.udbPairsArr[0].udb[0] = 0x00;
        ptKeyPattern.udbPairsArr[0].udb[1] = 0x3F;
        ptKeyMask.udbPairsArr[0].udb[0] = 0x00;
        ptKeyMask.udbPairsArr[0].udb[1] = 0xFC;
        st = cpssPxIngressPacketTypeKeyEntrySet(dev, 0, &ptKeyPattern, &ptKeyMask);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, 0);

        cpssOsMemSet(&ptKeyPattern, 0, sizeof(ptKeyPattern));
        cpssOsMemSet(&ptKeyMask, 0, sizeof(ptKeyMask));
        ptKeyPattern.macDa.arEther[0] = 0x01;
        ptKeyPattern.macDa.arEther[1] = 0x01;
        ptKeyPattern.macDa.arEther[2] = 0x01;
        ptKeyPattern.macDa.arEther[3] = 0x01;
        ptKeyPattern.macDa.arEther[4] = 0x01;
        ptKeyPattern.macDa.arEther[5] = 0x01;
        ptKeyMask.macDa.arEther[0] = 0xFF;
        ptKeyMask.macDa.arEther[1] = 0xFF;
        ptKeyMask.macDa.arEther[2] = 0xFF;
        ptKeyMask.macDa.arEther[3] = 0xFF;
        ptKeyMask.macDa.arEther[4] = 0xFF;
        ptKeyMask.macDa.arEther[5] = 0xFF;
        ptKeyPattern.isLLCNonSnap = GT_FALSE;
        ptKeyMask.isLLCNonSnap = GT_TRUE;
        ptKeyPattern.ip2meIndex = 2;
        ptKeyMask.ip2meIndex = 0x7;
        ptKeyPattern.udbPairsArr[0].udb[0] = 0x00;
        ptKeyPattern.udbPairsArr[0].udb[1] = 0x00;
        ptKeyMask.udbPairsArr[0].udb[0] = 0x00;
        ptKeyMask.udbPairsArr[0].udb[1] = 0xFC;
        st = cpssPxIngressPacketTypeKeyEntrySet(dev, 1, &ptKeyPattern, &ptKeyMask);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, 1);

        cpssOsMemSet(&ptKeyPattern, 0, sizeof(ptKeyPattern));
        cpssOsMemSet(&ptKeyMask, 0, sizeof(ptKeyMask));
        ptKeyPattern.macDa.arEther[0] = 0x02;
        ptKeyPattern.macDa.arEther[1] = 0x02;
        ptKeyPattern.macDa.arEther[2] = 0x02;
        ptKeyPattern.macDa.arEther[3] = 0x02;
        ptKeyPattern.macDa.arEther[4] = 0x02;
        ptKeyPattern.macDa.arEther[5] = 0x02;
        ptKeyMask.macDa.arEther[0] = 0xFF;
        ptKeyMask.macDa.arEther[1] = 0xFF;
        ptKeyMask.macDa.arEther[2] = 0xFF;
        ptKeyMask.macDa.arEther[3] = 0xFF;
        ptKeyMask.macDa.arEther[4] = 0xFF;
        ptKeyMask.macDa.arEther[5] = 0xFF;
        ptKeyPattern.isLLCNonSnap = GT_FALSE;
        ptKeyMask.isLLCNonSnap = GT_TRUE;
        ptKeyPattern.ip2meIndex = 3;
        ptKeyMask.ip2meIndex = 0x7;
        ptKeyPattern.udbPairsArr[0].udb[0] = 0x05;/*DSCP=0x15*/
        ptKeyPattern.udbPairsArr[0].udb[1] = 0x40;
        ptKeyMask.udbPairsArr[0].udb[0] = 0x0F;
        ptKeyMask.udbPairsArr[0].udb[1] = 0xC0;
        st = cpssPxIngressPacketTypeKeyEntrySet(dev, 2, &ptKeyPattern, &ptKeyMask);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, 2);


        cpssOsMemSet(&ptKeyPattern, 0, sizeof(ptKeyPattern));
        cpssOsMemSet(&ptKeyMask, 0, sizeof(ptKeyMask));
        ptKeyPattern.macDa.arEther[0] = 0x03;
        ptKeyPattern.macDa.arEther[1] = 0x03;
        ptKeyPattern.macDa.arEther[2] = 0x03;
        ptKeyPattern.macDa.arEther[3] = 0x03;
        ptKeyPattern.macDa.arEther[4] = 0x03;
        ptKeyPattern.macDa.arEther[5] = 0x03;
        ptKeyMask.macDa.arEther[0] = 0xFF;
        ptKeyMask.macDa.arEther[1] = 0xFF;
        ptKeyMask.macDa.arEther[2] = 0xFF;
        ptKeyMask.macDa.arEther[3] = 0xFF;
        ptKeyMask.macDa.arEther[4] = 0xFF;
        ptKeyMask.macDa.arEther[5] = 0xFF;
        ptKeyPattern.isLLCNonSnap = GT_FALSE;
        ptKeyMask.isLLCNonSnap = GT_TRUE;
        ptKeyPattern.ip2meIndex = 4;
        ptKeyMask.ip2meIndex = 0x7;
        ptKeyPattern.udbPairsArr[0].udb[0] = 0x0A;/*DSCP=0x2A*/
        ptKeyPattern.udbPairsArr[0].udb[1] = 0x80;
        ptKeyMask.udbPairsArr[0].udb[0] = 0x0F;
        ptKeyMask.udbPairsArr[0].udb[1] = 0xC0;
        st = cpssPxIngressPacketTypeKeyEntrySet(dev, 3, &ptKeyPattern, &ptKeyMask);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, 3);

        /* AUTODOC: Save packet type key for CPU port */
        st = cpssPxIngressPortPacketTypeKeyGet(dev, 16, &profileConfigRestore);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, 16);

        /* AUTODOC: Configure packet type key with UDBP0 has offset 2 on L3  */
        cpssOsMemSet(&profileConfig, 0, sizeof(profileConfig));
        profileConfig.srcPortProfile = 16; /* CPU port */
        profileConfig.portUdbPairArr[0].udbAnchorType =
            CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L3_E;
        profileConfig.portUdbPairArr[0].udbByteOffset = 2;
        st = cpssPxIngressPortPacketTypeKeySet(dev, 16/* CPU port */, &profileConfig);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, 16/* CPU port */);

        /* AUTODOC: Save Packet Type Destination Format table */
        for(ii = 0; ii < 4; ii++)
        {
            st = cpssPxIngressPortMapPacketTypeFormatEntryGet(dev,
                CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E,
                ii, &packetTypeFormatRestore[ii]);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        }

        /* AUTODOC: Configure Packet Type Destination Format table */
        /*
            PType BitField#3 BitField#2 BitField#1 BitField#0 Max_DstIdx Dst_Idx
            0     nbits=0    nbits=0    nbits=0    nbits=0    8K         0
            1     nbits=0    nbits=0    nbits=0    nbits=0    8K         1
            2     nbits=0    nbits=0    nbits=0    nbits=0    8K         2
            3     nbits=0    nbits=0    nbits=0    nbits=0    8K         3
        */
        cpssOsMemSet(&packetTypeFormat, 0, sizeof(packetTypeFormat));
        packetTypeFormat.indexConst = 0;
        packetTypeFormat.indexMax = PORT_MAP_ENTRY_MAX_INDEX;
        st = cpssPxIngressPortMapPacketTypeFormatEntrySet(dev,
            CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E, 0,
            &packetTypeFormat);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        cpssOsMemSet(&packetTypeFormat, 0, sizeof(packetTypeFormat));
        packetTypeFormat.indexConst = 1;
        packetTypeFormat.indexMax = PORT_MAP_ENTRY_MAX_INDEX;
        st = cpssPxIngressPortMapPacketTypeFormatEntrySet(dev,
            CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E, 1,
            &packetTypeFormat);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        cpssOsMemSet(&packetTypeFormat, 0, sizeof(packetTypeFormat));
        packetTypeFormat.indexConst = 2;
        packetTypeFormat.indexMax = PORT_MAP_ENTRY_MAX_INDEX;
        st = cpssPxIngressPortMapPacketTypeFormatEntrySet(dev,
            CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E, 2,
            &packetTypeFormat);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            cpssOsMemSet(&packetTypeFormat, 0, sizeof(packetTypeFormat));
        packetTypeFormat.indexConst = 3;
        packetTypeFormat.indexMax = PORT_MAP_ENTRY_MAX_INDEX;
        st = cpssPxIngressPortMapPacketTypeFormatEntrySet(dev,
            CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E, 3,
            &packetTypeFormat);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* AUTODOC: Save destination port map entry table entries */
        for(ii = 0; ii < 4; ii++)
        {
            st = cpssPxIngressPortMapEntryGet(dev,
                CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E, ii,
                &dstPortBmpRestore[ii], &lagEnRestore[ii]);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev,
                CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E, ii);
        }

        /* AUTODOC: Configure destination port map entry table entries */
        /*
            PType PortBitmap
            0     Port 1
            1     Port 2
            2     Port 3
            3     Port 4
        */
        st = cpssPxIngressPortMapEntrySet(dev,
            CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E, 0,
            (1 << IP_CLASS_PORT1), GT_FALSE);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev,
            CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E, 0);
        st = cpssPxIngressPortMapEntrySet(dev,
            CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E, 1,
            (1 << IP_CLASS_PORT2), GT_FALSE);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev,
            CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E, 1);
        st = cpssPxIngressPortMapEntrySet(dev,
            CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E, 2,
            (1 << IP_CLASS_PORT3), GT_FALSE);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev,
            CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E, 2);
        st = cpssPxIngressPortMapEntrySet(dev,
            CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E, 3,
            (1 << IP_CLASS_PORT4), GT_FALSE);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev,
            CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E, 3);

        /* AUTODOC: Save target port enabled states */
        st = cpssPxIngressPortTargetEnableGet(dev, IP_CLASS_PORT1,
            &portEnableRestore[0]);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, IP_CLASS_PORT1);
        st = cpssPxIngressPortTargetEnableGet(dev, IP_CLASS_PORT2,
            &portEnableRestore[1]);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, IP_CLASS_PORT2);
        st = cpssPxIngressPortTargetEnableGet(dev, IP_CLASS_PORT3,
            &portEnableRestore[2]);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, IP_CLASS_PORT3);
        st = cpssPxIngressPortTargetEnableGet(dev, IP_CLASS_PORT4,
            &portEnableRestore[3]);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, IP_CLASS_PORT4);

        /* AUTODOC: Configure target port enabled states */
        st = cpssPxIngressPortTargetEnableSet(dev, IP_CLASS_PORT1, GT_TRUE);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, IP_CLASS_PORT1, GT_TRUE);
        st = cpssPxIngressPortTargetEnableSet(dev, IP_CLASS_PORT2, GT_TRUE);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, IP_CLASS_PORT2, GT_TRUE);
        st = cpssPxIngressPortTargetEnableSet(dev, IP_CLASS_PORT3, GT_TRUE);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, IP_CLASS_PORT3, GT_TRUE);
        st = cpssPxIngressPortTargetEnableSet(dev, IP_CLASS_PORT4, GT_TRUE);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, IP_CLASS_PORT4, GT_TRUE);

        /* AUTODOC: Link UP */
        st = prvTgfEgressPortForceLinkPassEnable(dev, IP_CLASS_PORT1, GT_TRUE, GT_TRUE);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, IP_CLASS_PORT1, GT_TRUE);
        st = prvTgfEgressPortForceLinkPassEnable(dev, IP_CLASS_PORT2, GT_TRUE, GT_TRUE);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, IP_CLASS_PORT2, GT_TRUE);
        st = prvTgfEgressPortForceLinkPassEnable(dev, IP_CLASS_PORT3, GT_TRUE, GT_TRUE);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, IP_CLASS_PORT3, GT_TRUE);
        st = prvTgfEgressPortForceLinkPassEnable(dev, IP_CLASS_PORT4, GT_TRUE, GT_TRUE);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, IP_CLASS_PORT4, GT_TRUE);


        /* AUTODOC: Save egress port filtering */
        for(ii = 0; ii < 4; ii++)
        {
            st = cpssPxIngressFilteringEnableGet(dev, CPSS_DIRECTION_EGRESS_E,
                ii, &egrPortFilteringRerstore[ii]);
        }

        /* AUTODOC: Disable egress port filtering */
        for(ii = 0; ii < 4; ii++)
        {
            st = cpssPxIngressFilteringEnableSet(dev, CPSS_DIRECTION_EGRESS_E,
                ii, GT_FALSE);
        }
    }
    else
    {
        /* AUTODOC: Restore egress port filtering */
        for(ii = 0; ii < 4; ii++)
        {
            st = cpssPxIngressFilteringEnableSet(dev, CPSS_DIRECTION_EGRESS_E,
                ii, egrPortFilteringRerstore[ii]);
        }

        /* AUTODOC: Link Down */
        st = prvTgfEgressPortForceLinkPassEnable(dev, IP_CLASS_PORT1, GT_FALSE, GT_TRUE);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, IP_CLASS_PORT1, GT_FALSE);
        st = prvTgfEgressPortForceLinkPassEnable(dev, IP_CLASS_PORT2, GT_FALSE, GT_TRUE);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, IP_CLASS_PORT2, GT_FALSE);
        st = prvTgfEgressPortForceLinkPassEnable(dev, IP_CLASS_PORT3, GT_FALSE, GT_TRUE);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, IP_CLASS_PORT3, GT_FALSE);
        st = prvTgfEgressPortForceLinkPassEnable(dev, IP_CLASS_PORT4, GT_FALSE, GT_TRUE);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, IP_CLASS_PORT4, GT_FALSE);

        /* AUTODOC: Restore target port enabled states */
        st = cpssPxIngressPortTargetEnableSet(dev, IP_CLASS_PORT1,
            portEnableRestore[0]);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, IP_CLASS_PORT1);
        st = cpssPxIngressPortTargetEnableSet(dev, IP_CLASS_PORT2,
            portEnableRestore[1]);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, IP_CLASS_PORT2);
        st = cpssPxIngressPortTargetEnableSet(dev, IP_CLASS_PORT3,
            portEnableRestore[2]);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, IP_CLASS_PORT3);
        st = cpssPxIngressPortTargetEnableSet(dev, IP_CLASS_PORT4,
            portEnableRestore[3]);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, IP_CLASS_PORT4);

        /* AUTODOC: Restore destination port map entry table entries */
        for(ii = 0; ii < 4; ii++)
        {
            st = cpssPxIngressPortMapEntrySet(dev,
                CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E, ii,
                dstPortBmpRestore[ii], lagEnRestore[ii]);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev,
                CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E, ii);
        }

        /* AUTODOC: Restore Packet Type Destination Format table */
        for(ii = 0; ii < 4; ii++)
        {
            st = cpssPxIngressPortMapPacketTypeFormatEntrySet(dev,
                CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E,
                ii, &packetTypeFormatRestore[ii]);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        }

        /* AUTODOC: Restore packet type key for CPU port */
        st = cpssPxIngressPortPacketTypeKeySet(dev, 16, &profileConfigRestore);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, 16);

        /* AUTODOC: Restore packet type key entries */
        for(ii = 0; ii < 4; ii++)
        {
            st = cpssPxIngressPacketTypeKeyEntrySet(dev, ii,
                &packetTypekeyPatternRestore[ii], &packetTypekeyMaskRestore[ii]);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, ii);
        }

        /* AUTODOC: Restore first 4 entries of IP2ME table */
        for(ii = 1; ii < 5; ii++)
        {
            st = cpssPxIngressIp2MeEntrySet(dev, ii, &ip2meEntriesRestore[ii]);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, ii);
        }
    }
}

