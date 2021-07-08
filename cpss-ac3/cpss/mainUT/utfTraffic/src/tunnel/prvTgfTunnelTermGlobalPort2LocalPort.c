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
* @file prvTgfTunnelTermGlobalPort2LocalPort.c
*
* @brief Tunnel Term: Global Port 2 Local Port - Basic
*
* @version   20
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
#include <common/tgfIpGen.h>
#include <common/tgfTunnelGen.h>
#include <tunnel/prvTgfTunnelTermGlobalPort2LocalPort.h>
#include <tunnel/prvTgfTunnelTermPortGroupIpv4overIpv4.h>

static GT_U32    nextHopPortNum = PRV_TGF_NEXTHOPE_PORT_NUM_CNS;
/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

#define PRV_TGF_SEND_PORT1_NUM_CNS      PRV_TGF_SEND_PORT_GROUP_X_PORT_NUM_MAC(0,2)

/* default VLAN Id */
#define PRV_TGF_SEND_VLANID_CNS           5

/* nextHop VLAN Id */
#define PRV_TGF_NEXTHOPE_VLANID_CNS       6

/* number of ports */
#define PRV_TGF_PORT_COUNT_CNS            CPSS_MAX_PORT_GROUPS_CNS

/* port number to send traffic to */
#define PRV_TGF_FDB_PORT_IDX_CNS          1

/* non existing port */
#define PRV_TGF_NON_EXISTING_PORT         59

/* the counter set for a route entry is linked to */
#define PRV_TGF_COUNT_SET_CNS     CPSS_IP_CNT_SET0_E

/* default number of packets to send */
static GT_U32        prvTgfBurstCount   = 1;

/* VLANs array */
static GT_U8         prvTgfVlanArray[] = {PRV_TGF_SEND_VLANID_CNS, PRV_TGF_NEXTHOPE_VLANID_CNS};

/* VLANs array */
static GT_U8         prvTgfVlanPerPortArray[PRV_TGF_PORT_COUNT_CNS] = {
    PRV_TGF_SEND_VLANID_CNS, PRV_TGF_SEND_VLANID_CNS,
    PRV_TGF_SEND_VLANID_CNS,PRV_TGF_NEXTHOPE_VLANID_CNS,
    PRV_TGF_SEND_VLANID_CNS, PRV_TGF_SEND_VLANID_CNS,
    PRV_TGF_SEND_VLANID_CNS, PRV_TGF_SEND_VLANID_CNS
};

/* the Arp Address index of the Router ARP Table
   to write to the UC Route entry Arp nextHopARPPointer field */
static GT_U32        prvTgfRouterArpIndex      = 1;

/* the Route entry index for UC Route entry Table */
static GT_U32        prvTgfRouteEntryBaseIndex = 3;

/* stored default Vlan ID */
static GT_U16   prvTgfDefVlanId = 0;

/* the TTI Rule index */
static GT_U32        prvTgfTtiRuleIndex        = 1;

/* the LPM DB id for LPM Table */
static GT_U32        prvTgfLpmDBId             = 0;

/* the ARP MAC address to write to the Router ARP Table */
static TGF_MAC_ADDR  prvTgfArpMac = {0x00, 0x00, 0x00, 0x00, 0x00, 0x22};

/* the Mask for MAC address */
static TGF_MAC_ADDR  prvTgfFullMaskMac = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

/* the Mask for IP address */
static TGF_IPV4_ADDR prvTgfFullMaskIp = {255, 255, 255, 255};

/**************************** Test Configuration ******************************/
typedef struct
{
    GT_BOOL    srcIsTrunk;      /* Whether source is port or trunk (0..1)
                                   GT_FALSE: Source is not a trunk (i.e. it is port)
                                   GT_TRUE: Source is a trunk */
    GT_U32     srcPortTrunk;    /* Source port or Source trunk-ID
                                   if port, range (0..63)
                                   if trunk, range (0..127)*/
    GT_BOOL    dsaSrcIsTrunk;   /* Whether source is Trunk or not */
    GT_U8      dsaSrcPortTrunk; /* DSA tag source port or trunk; relevant only for DSA tagged packets */
    GT_U8      dsaSrcDevice;    /* DSA tag source device; relevant only for DSA tagged packets */
}prvTgfPortTest;

typedef struct
{
    prvTgfPortTest  pattern;    /* Rule's pattern */
    prvTgfPortTest  mask;       /* Rules mask */

}prvTgfPortTestConfig;

typedef struct
{
    prvTgfPortTestConfig conf[CPSS_MAX_PORT_GROUPS_CNS]; /* each test has 8 configurations */
}prvTgfPortTestConfigStruct;

/********************** Test 1: pattern mask configuration ********************/
static prvTgfPortTestConfigStruct confTest1 =
{
    /* Test 1: Sending Basic IPv4 Tunnel Termination - to All Port Groups, local port 2
               Bits 4,5 in mask <srcPortTrunk> = 'don't care': When port group id bits
               in global port are 'don't care',the same global port should be written to
               all group ports. Write to all port groups, local port 2: ports - 3, 19, 35, 51 */
    {

    {{GT_FALSE,0x02,GT_FALSE,0,0},{GT_FALSE,0xF,GT_FALSE,0,0}}, /* local port 2, port group 0 */
    {{GT_FALSE,0x12,GT_FALSE,0,0},{GT_FALSE,0xF,GT_FALSE,0,0}}, /* local port 2, port group 1 */
    {{GT_FALSE,0x22,GT_FALSE,0,0},{GT_FALSE,0xF,GT_FALSE,0,0}}, /* local port 2, port group 2 */
    {{GT_FALSE,0x32,GT_FALSE,0,0},{GT_FALSE,0xF,GT_FALSE,0,0}}, /* local port 2, port group 3 */
    {{GT_FALSE,0x42,GT_FALSE,0,0},{GT_FALSE,0xF,GT_FALSE,0,0}}, /* local port 2, port group 4 */
    {{GT_FALSE,0x52,GT_FALSE,0,0},{GT_FALSE,0xF,GT_FALSE,0,0}}, /* local port 2, port group 5 */
    {{GT_FALSE,0x62,GT_FALSE,0,0},{GT_FALSE,0xF,GT_FALSE,0,0}}, /* local port 2, port group 6 */
    {{GT_FALSE,0x72,GT_FALSE,0,0},{GT_FALSE,0xF,GT_FALSE,0,0}}  /* local port 2, port group 7 */

    }
};

/********************** Test 2: pattern mask configuration ********************/
static prvTgfPortTestConfigStruct confTest2 =
{
    /* Test 2: Sending Basic IPv4 Tunnel Termination - to Specific Port Group and local port
               Bits 4,5 in mask <srcPortTrunk> = 'exact match': When port group id bits
               in global port are 'exact match',write to a single core */
    {
    {{GT_FALSE,0x02,GT_FALSE,0,0},{GT_TRUE,0x3F,GT_FALSE,0,0}}, /* local port 2, port group 0,4 */
    {{GT_FALSE,0x12,GT_FALSE,0,0},{GT_TRUE,0x3F,GT_FALSE,0,0}}, /* local port 2, port group 1,5 */
    {{GT_FALSE,0x22,GT_FALSE,0,0},{GT_TRUE,0x3F,GT_FALSE,0,0}}, /* local port 2, port group 2,6 */
    {{GT_FALSE,0x32,GT_FALSE,0,0},{GT_TRUE,0x3F,GT_FALSE,0,0}}, /* local port 2, port group 3,7 */

    {{GT_FALSE,0x42,GT_FALSE,0,0},{GT_TRUE,0x7F,GT_FALSE,0,0}}, /* local port 2, port group 4 */
    {{GT_FALSE,0x52,GT_FALSE,0,0},{GT_TRUE,0x7F,GT_FALSE,0,0}}, /* local port 2, port group 5 */
    {{GT_FALSE,0x62,GT_FALSE,0,0},{GT_TRUE,0x7F,GT_FALSE,0,0}}, /* local port 2, port group 6 */
    {{GT_FALSE,0x72,GT_FALSE,0,0},{GT_TRUE,0x7F,GT_FALSE,0,0}}  /* local port 2, port group 7 */
    }
};

/********************** Test 3: pattern mask configuration ********************/
static prvTgfPortTestConfigStruct confTest3 =
{
    /* Test 3: Sending Basic IPv4 Tunnel Termination - to 2 Port Groups, local port 2
               Bit 4 in mask <srcPortTrunk> = 'exact match', Bit 5 = 'don't care':
               When bit 4 in pattern is 0: Relevant to port groups 0, 2.
               When bit 4 in pattern is 1: Relevant to port groups 1, 3.
               Write to port groups 0,2, and then 1,3 local port 2.
               Bit 5 in mask <srcPortTrunk> = 'exact match', Bit 4 = 'don't care':
               When bit 5 in pattern is 0: Relevant to port groups 0, 1.
               When bit 5 in pattern is 1: Relevant to port groups 2, 3.
               Write to port groups 0,1, and then 2,3 local port 2 */
   {
    {{GT_FALSE,0x02,GT_FALSE,0,0},{GT_TRUE,0x1F,GT_FALSE,0,0}}, /* Write to port groups 0,2,4,6 local port 2 */
    {{GT_FALSE,0x12,GT_FALSE,0,0},{GT_TRUE,0x1F,GT_FALSE,0,0}}, /* Write to port groups 1,3,5,7 local port 2 */
    {{GT_FALSE,0x02,GT_FALSE,0,0},{GT_TRUE,0x2F,GT_FALSE,0,0}}, /* Write to port groups 0,1,4,5 local port 2 */
    {{GT_FALSE,0x22,GT_FALSE,0,0},{GT_TRUE,0x2F,GT_FALSE,0,0}}, /* Write to port groups 2,3,6,7 local port 2 */

    {{GT_FALSE,0x42,GT_FALSE,0,0},{GT_TRUE,0x4F,GT_FALSE,0,0}}, /* Write to port groups 4,5,6,7 local port 2 */
    {{GT_FALSE,0x12,GT_FALSE,0,0},{GT_TRUE,0x5F,GT_FALSE,0,0}}, /* Write to port groups 1,3  local port 2 */
    {{GT_FALSE,0x62,GT_FALSE,0,0},{GT_TRUE,0x6F,GT_FALSE,0,0}}, /* Write to port groups 6,7  local port 2 */
    {{GT_FALSE,0x22,GT_FALSE,0,0},{GT_TRUE,0x6F,GT_FALSE,0,0}}, /* Write to port groups 2,3  local port 2 */
   }

};


/********************** Test 1: pattern mask configuration ********************/
static prvTgfPortTestConfigStruct confTest1_XLG =
{
    /* Test 1: Sending Basic IPv4 Tunnel Termination - to All Port Groups, local port 8
               Bits 4,5 in mask <srcPortTrunk> = 'don't care': When port group id bits
               in global port are 'don't care',the same global port should be written to
               all group ports. Write to all port groups, local port 8: ports - 8, 24, 40, 56 */
    {

    {{GT_FALSE,0x08,GT_FALSE,0,0},{GT_FALSE,0xF,GT_FALSE,0,0}}, /* local port 8, port group 0 */
    {{GT_FALSE,0x18,GT_FALSE,0,0},{GT_FALSE,0xF,GT_FALSE,0,0}}, /* local port 8, port group 1 */
    {{GT_FALSE,0x28,GT_FALSE,0,0},{GT_FALSE,0xF,GT_FALSE,0,0}}, /* local port 8, port group 2 */
    {{GT_FALSE,0x38,GT_FALSE,0,0},{GT_FALSE,0xF,GT_FALSE,0,0}}, /* local port 8, port group 3 */
    {{GT_FALSE,0x48,GT_FALSE,0,0},{GT_FALSE,0xF,GT_FALSE,0,0}}, /* local port 8, port group 4 */
    {{GT_FALSE,0x58,GT_FALSE,0,0},{GT_FALSE,0xF,GT_FALSE,0,0}}, /* local port 8, port group 5 */
    {{GT_FALSE,0x68,GT_FALSE,0,0},{GT_FALSE,0xF,GT_FALSE,0,0}}, /* local port 8, port group 6 */
    {{GT_FALSE,0x78,GT_FALSE,0,0},{GT_FALSE,0xF,GT_FALSE,0,0}}  /* local port 8, port group 7 */

    }
};

/********************** Test 2: pattern mask configuration ********************/
static prvTgfPortTestConfigStruct confTest2_XLG =
{
    /* Test 2: Sending Basic IPv4 Tunnel Termination - to Specific Port Group and local port
               Bits 4,5 in mask <srcPortTrunk> = 'exact match': When port group id bits
               in global port are 'exact match',write to a single core */
    {
    {{GT_FALSE,0x08,GT_FALSE,0,0},{GT_TRUE,0x3F,GT_FALSE,0,0}}, /* local port 8, port group 0,4 */
    {{GT_FALSE,0x18,GT_FALSE,0,0},{GT_TRUE,0x3F,GT_FALSE,0,0}}, /* local port 8, port group 1,5 */
    {{GT_FALSE,0x28,GT_FALSE,0,0},{GT_TRUE,0x3F,GT_FALSE,0,0}}, /* local port 8, port group 2,6 */
    {{GT_FALSE,0x38,GT_FALSE,0,0},{GT_TRUE,0x3F,GT_FALSE,0,0}}, /* local port 8, port group 3,7 */

    {{GT_FALSE,0x48,GT_FALSE,0,0},{GT_TRUE,0x7F,GT_FALSE,0,0}}, /* local port 8, port group 4 */
    {{GT_FALSE,0x58,GT_FALSE,0,0},{GT_TRUE,0x7F,GT_FALSE,0,0}}, /* local port 8, port group 5 */
    {{GT_FALSE,0x68,GT_FALSE,0,0},{GT_TRUE,0x7F,GT_FALSE,0,0}}, /* local port 8, port group 6 */
    {{GT_FALSE,0x78,GT_FALSE,0,0},{GT_TRUE,0x7F,GT_FALSE,0,0}}  /* local port 8, port group 7 */
    }
};

/********************** Test 3: pattern mask configuration ********************/
static prvTgfPortTestConfigStruct confTest3_XLG =
{
    /* Test 3: Sending Basic IPv4 Tunnel Termination - to 2 Port Groups, local port 8
               Bit 4 in mask <srcPortTrunk> = 'exact match', Bit 5 = 'don't care':
               When bit 4 in pattern is 0: Relevant to port groups 0, 2.
               When bit 4 in pattern is 1: Relevant to port groups 1, 3.
               Write to port groups 0,2, and then 1,3 local port 8.
               Bit 5 in mask <srcPortTrunk> = 'exact match', Bit 4 = 'don't care':
               When bit 5 in pattern is 0: Relevant to port groups 0, 1.
               When bit 5 in pattern is 1: Relevant to port groups 2, 3.
               Write to port groups 0,1, and then 2,3 local port 8 */
   {
    {{GT_FALSE,0x08,GT_FALSE,0,0},{GT_TRUE,0x1F,GT_FALSE,0,0}}, /* Write to port groups 0,2,4,6 local port 8 */
    {{GT_FALSE,0x18,GT_FALSE,0,0},{GT_TRUE,0x1F,GT_FALSE,0,0}}, /* Write to port groups 1,3,5,7 local port 8 */
    {{GT_FALSE,0x08,GT_FALSE,0,0},{GT_TRUE,0x2F,GT_FALSE,0,0}}, /* Write to port groups 0,1,4,5 local port 8 */
    {{GT_FALSE,0x28,GT_FALSE,0,0},{GT_TRUE,0x2F,GT_FALSE,0,0}}, /* Write to port groups 2,3,6,7 local port 8 */

    {{GT_FALSE,0x48,GT_FALSE,0,0},{GT_TRUE,0x4F,GT_FALSE,0,0}}, /* Write to port groups 4,5,6,7 local port 8 */
    {{GT_FALSE,0x38,GT_FALSE,0,0},{GT_TRUE,0x5F,GT_FALSE,0,0}}, /* Write to port groups 1,3  local port 8 */
    {{GT_FALSE,0x78,GT_FALSE,0,0},{GT_TRUE,0x6F,GT_FALSE,0,0}}, /* Write to port groups 6,7  local port 8 */
    {{GT_FALSE,0x38,GT_FALSE,0,0},{GT_TRUE,0x6F,GT_FALSE,0,0}}, /* Write to port groups 2,3  local port 8 */
   }

};


typedef struct
{
    GT_U32   Tx; /* expected results: Tx port */
    GT_U32   Rx; /* expected results: Rx port */
} prvTgfPortRxTx;

typedef struct
{
    prvTgfPortRxTx transmission[CPSS_MAX_PORT_GROUPS_CNS]; /* each iteration has 8 transmission */
}prvTgfPortRxTxCounters;

typedef struct
{
    prvTgfPortRxTxCounters iteration[CPSS_MAX_PORT_GROUPS_CNS]; /* each test has 8 iteration */
}prvTgfPortRxTxCountersConfig;

#define FILL_IN_RUNTIME 0xFF

/************************* Test 1: Port Rx Tx Counters ***********************/
static prvTgfPortRxTxCountersConfig TxRxTest1 =
{   /* iteration 0 */
  {
   {{
     {FILL_IN_RUNTIME,PRV_TGF_NEXTHOPE_PORT_NUM_CNS},  /* transmission 0 */
     {FILL_IN_RUNTIME,PRV_TGF_NEXTHOPE_PORT_NUM_CNS},  /* transmission 1 */
     {FILL_IN_RUNTIME,PRV_TGF_NEXTHOPE_PORT_NUM_CNS},  /* transmission 2 */
     {FILL_IN_RUNTIME,PRV_TGF_NEXTHOPE_PORT_NUM_CNS},  /* transmission 3 */
     {FILL_IN_RUNTIME,PRV_TGF_NEXTHOPE_PORT_NUM_CNS},  /* transmission 4 */
     {FILL_IN_RUNTIME,PRV_TGF_NEXTHOPE_PORT_NUM_CNS},  /* transmission 5 */
     {FILL_IN_RUNTIME,PRV_TGF_NEXTHOPE_PORT_NUM_CNS},  /* transmission 6 */
     {FILL_IN_RUNTIME,PRV_TGF_NEXTHOPE_PORT_NUM_CNS}   /* transmission 7 */
   }},
    /* iteration 1 */
   {{
     {FILL_IN_RUNTIME,PRV_TGF_NEXTHOPE_PORT_NUM_CNS},  /* transmission 0 */
     {FILL_IN_RUNTIME,PRV_TGF_NEXTHOPE_PORT_NUM_CNS},  /* transmission 1 */
     {FILL_IN_RUNTIME,PRV_TGF_NEXTHOPE_PORT_NUM_CNS},  /* transmission 2 */
     {FILL_IN_RUNTIME,PRV_TGF_NEXTHOPE_PORT_NUM_CNS},  /* transmission 3 */
     {FILL_IN_RUNTIME,PRV_TGF_NEXTHOPE_PORT_NUM_CNS},  /* transmission 4 */
     {FILL_IN_RUNTIME,PRV_TGF_NEXTHOPE_PORT_NUM_CNS},  /* transmission 5 */
     {FILL_IN_RUNTIME,PRV_TGF_NEXTHOPE_PORT_NUM_CNS},  /* transmission 6 */
     {FILL_IN_RUNTIME,PRV_TGF_NEXTHOPE_PORT_NUM_CNS}   /* transmission 7 */
   }},
    /* iteration 2 */
   {{
     {FILL_IN_RUNTIME,PRV_TGF_NEXTHOPE_PORT_NUM_CNS},  /* transmission 0 */
     {FILL_IN_RUNTIME,PRV_TGF_NEXTHOPE_PORT_NUM_CNS},  /* transmission 1 */
     {FILL_IN_RUNTIME,PRV_TGF_NEXTHOPE_PORT_NUM_CNS},  /* transmission 2 */
     {FILL_IN_RUNTIME,PRV_TGF_NEXTHOPE_PORT_NUM_CNS},  /* transmission 3 */
     {FILL_IN_RUNTIME,PRV_TGF_NEXTHOPE_PORT_NUM_CNS},  /* transmission 4 */
     {FILL_IN_RUNTIME,PRV_TGF_NEXTHOPE_PORT_NUM_CNS},  /* transmission 5 */
     {FILL_IN_RUNTIME,PRV_TGF_NEXTHOPE_PORT_NUM_CNS},  /* transmission 6 */
     {FILL_IN_RUNTIME,PRV_TGF_NEXTHOPE_PORT_NUM_CNS}   /* transmission 7 */
   }},
    /* iteration 3 */
   {{
     {FILL_IN_RUNTIME,PRV_TGF_NEXTHOPE_PORT_NUM_CNS},  /* transmission 0 */
     {FILL_IN_RUNTIME,PRV_TGF_NEXTHOPE_PORT_NUM_CNS},  /* transmission 1 */
     {FILL_IN_RUNTIME,PRV_TGF_NEXTHOPE_PORT_NUM_CNS},  /* transmission 2 */
     {FILL_IN_RUNTIME,PRV_TGF_NEXTHOPE_PORT_NUM_CNS},  /* transmission 3 */
     {FILL_IN_RUNTIME,PRV_TGF_NEXTHOPE_PORT_NUM_CNS},  /* transmission 4 */
     {FILL_IN_RUNTIME,PRV_TGF_NEXTHOPE_PORT_NUM_CNS},  /* transmission 5 */
     {FILL_IN_RUNTIME,PRV_TGF_NEXTHOPE_PORT_NUM_CNS},  /* transmission 6 */
     {FILL_IN_RUNTIME,PRV_TGF_NEXTHOPE_PORT_NUM_CNS}   /* transmission 7 */
   }},

    /* iteration 4 */
   {{
     {FILL_IN_RUNTIME,PRV_TGF_NEXTHOPE_PORT_NUM_CNS},  /* transmission 0 */
     {FILL_IN_RUNTIME,PRV_TGF_NEXTHOPE_PORT_NUM_CNS},  /* transmission 1 */
     {FILL_IN_RUNTIME,PRV_TGF_NEXTHOPE_PORT_NUM_CNS},  /* transmission 2 */
     {FILL_IN_RUNTIME,PRV_TGF_NEXTHOPE_PORT_NUM_CNS},  /* transmission 3 */
     {FILL_IN_RUNTIME,PRV_TGF_NEXTHOPE_PORT_NUM_CNS},  /* transmission 4 */
     {FILL_IN_RUNTIME,PRV_TGF_NEXTHOPE_PORT_NUM_CNS},  /* transmission 5 */
     {FILL_IN_RUNTIME,PRV_TGF_NEXTHOPE_PORT_NUM_CNS},  /* transmission 6 */
     {FILL_IN_RUNTIME,PRV_TGF_NEXTHOPE_PORT_NUM_CNS}   /* transmission 7 */
   }},
    /* iteration 5 */
   {{
     {FILL_IN_RUNTIME,PRV_TGF_NEXTHOPE_PORT_NUM_CNS},  /* transmission 0 */
     {FILL_IN_RUNTIME,PRV_TGF_NEXTHOPE_PORT_NUM_CNS},  /* transmission 1 */
     {FILL_IN_RUNTIME,PRV_TGF_NEXTHOPE_PORT_NUM_CNS},  /* transmission 2 */
     {FILL_IN_RUNTIME,PRV_TGF_NEXTHOPE_PORT_NUM_CNS},  /* transmission 3 */
     {FILL_IN_RUNTIME,PRV_TGF_NEXTHOPE_PORT_NUM_CNS},  /* transmission 4 */
     {FILL_IN_RUNTIME,PRV_TGF_NEXTHOPE_PORT_NUM_CNS},  /* transmission 5 */
     {FILL_IN_RUNTIME,PRV_TGF_NEXTHOPE_PORT_NUM_CNS},  /* transmission 6 */
     {FILL_IN_RUNTIME,PRV_TGF_NEXTHOPE_PORT_NUM_CNS}   /* transmission 7 */
   }},
    /* iteration 6 */
   {{
     {FILL_IN_RUNTIME,PRV_TGF_NEXTHOPE_PORT_NUM_CNS},  /* transmission 0 */
     {FILL_IN_RUNTIME,PRV_TGF_NEXTHOPE_PORT_NUM_CNS},  /* transmission 1 */
     {FILL_IN_RUNTIME,PRV_TGF_NEXTHOPE_PORT_NUM_CNS},  /* transmission 2 */
     {FILL_IN_RUNTIME,PRV_TGF_NEXTHOPE_PORT_NUM_CNS},  /* transmission 3 */
     {FILL_IN_RUNTIME,PRV_TGF_NEXTHOPE_PORT_NUM_CNS},  /* transmission 4 */
     {FILL_IN_RUNTIME,PRV_TGF_NEXTHOPE_PORT_NUM_CNS},  /* transmission 5 */
     {FILL_IN_RUNTIME,PRV_TGF_NEXTHOPE_PORT_NUM_CNS},  /* transmission 6 */
     {FILL_IN_RUNTIME,PRV_TGF_NEXTHOPE_PORT_NUM_CNS}   /* transmission 7 */
   }},
    /* iteration 7 */
   {{
     {FILL_IN_RUNTIME,PRV_TGF_NEXTHOPE_PORT_NUM_CNS},  /* transmission 0 */
     {FILL_IN_RUNTIME,PRV_TGF_NEXTHOPE_PORT_NUM_CNS},  /* transmission 1 */
     {FILL_IN_RUNTIME,PRV_TGF_NEXTHOPE_PORT_NUM_CNS},  /* transmission 2 */
     {FILL_IN_RUNTIME,PRV_TGF_NEXTHOPE_PORT_NUM_CNS},  /* transmission 3 */
     {FILL_IN_RUNTIME,PRV_TGF_NEXTHOPE_PORT_NUM_CNS},  /* transmission 4 */
     {FILL_IN_RUNTIME,PRV_TGF_NEXTHOPE_PORT_NUM_CNS},  /* transmission 5 */
     {FILL_IN_RUNTIME,PRV_TGF_NEXTHOPE_PORT_NUM_CNS},  /* transmission 6 */
     {FILL_IN_RUNTIME,PRV_TGF_NEXTHOPE_PORT_NUM_CNS}   /* transmission 7 */
   }}
  }
};

/************************* Test 2: Port Rx Tx Counters ***********************/
static prvTgfPortRxTxCountersConfig TxRxTest2 =
{   /* iteration 0 */
  {
    {{
     {FILL_IN_RUNTIME,PRV_TGF_NEXTHOPE_PORT_NUM_CNS},   /* transmission 0 */
     {FILL_IN_RUNTIME,PRV_TGF_NON_EXISTING_PORT},       /* transmission 1 */
     {FILL_IN_RUNTIME,PRV_TGF_NON_EXISTING_PORT},       /* transmission 2 */
     {FILL_IN_RUNTIME,PRV_TGF_NON_EXISTING_PORT},       /* transmission 3 */
     {FILL_IN_RUNTIME,PRV_TGF_NEXTHOPE_PORT_NUM_CNS},   /* transmission 4 */
     {FILL_IN_RUNTIME,PRV_TGF_NON_EXISTING_PORT},       /* transmission 5 */
     {FILL_IN_RUNTIME,PRV_TGF_NON_EXISTING_PORT},       /* transmission 6 */
     {FILL_IN_RUNTIME,PRV_TGF_NON_EXISTING_PORT}        /* transmission 7 */
    }},
    /* iteration 1 */
   {{
     {FILL_IN_RUNTIME,PRV_TGF_NON_EXISTING_PORT},       /* transmission 0 */
     {FILL_IN_RUNTIME,PRV_TGF_NEXTHOPE_PORT_NUM_CNS},   /* transmission 1 */
     {FILL_IN_RUNTIME,PRV_TGF_NON_EXISTING_PORT},       /* transmission 2 */
     {FILL_IN_RUNTIME,PRV_TGF_NON_EXISTING_PORT},       /* transmission 3 */
     {FILL_IN_RUNTIME,PRV_TGF_NON_EXISTING_PORT},       /* transmission 4 */
     {FILL_IN_RUNTIME,PRV_TGF_NEXTHOPE_PORT_NUM_CNS},   /* transmission 5 */
     {FILL_IN_RUNTIME,PRV_TGF_NON_EXISTING_PORT},       /* transmission 6 */
     {FILL_IN_RUNTIME,PRV_TGF_NON_EXISTING_PORT}        /* transmission 7 */
    }},
    /* iteration 2 */
   {{
     {FILL_IN_RUNTIME,PRV_TGF_NON_EXISTING_PORT},       /* transmission 0 */
     {FILL_IN_RUNTIME,PRV_TGF_NON_EXISTING_PORT},       /* transmission 1 */
     {FILL_IN_RUNTIME,PRV_TGF_NEXTHOPE_PORT_NUM_CNS},   /* transmission 2 */
     {FILL_IN_RUNTIME,PRV_TGF_NON_EXISTING_PORT},       /* transmission 3 */
     {FILL_IN_RUNTIME,PRV_TGF_NON_EXISTING_PORT},       /* transmission 4 */
     {FILL_IN_RUNTIME,PRV_TGF_NON_EXISTING_PORT},       /* transmission 5 */
     {FILL_IN_RUNTIME,PRV_TGF_NEXTHOPE_PORT_NUM_CNS},   /* transmission 6 */
     {FILL_IN_RUNTIME,PRV_TGF_NON_EXISTING_PORT}        /* transmission 7 */
    }},

    /* iteration 3 */
   {{
     {FILL_IN_RUNTIME,PRV_TGF_NON_EXISTING_PORT},       /* transmission 0 */
     {FILL_IN_RUNTIME,PRV_TGF_NON_EXISTING_PORT},       /* transmission 1 */
     {FILL_IN_RUNTIME,PRV_TGF_NON_EXISTING_PORT},       /* transmission 2 */
     {FILL_IN_RUNTIME,PRV_TGF_NEXTHOPE_PORT_NUM_CNS},   /* transmission 3 */
     {FILL_IN_RUNTIME,PRV_TGF_NON_EXISTING_PORT},       /* transmission 4 */
     {FILL_IN_RUNTIME,PRV_TGF_NON_EXISTING_PORT},       /* transmission 5 */
     {FILL_IN_RUNTIME,PRV_TGF_NON_EXISTING_PORT},       /* transmission 6 */
     {FILL_IN_RUNTIME,PRV_TGF_NEXTHOPE_PORT_NUM_CNS}    /* transmission 7 */
    }},

    /* iteration 4 */
   {{
     {FILL_IN_RUNTIME,PRV_TGF_NON_EXISTING_PORT},       /* transmission 0 */
     {FILL_IN_RUNTIME,PRV_TGF_NON_EXISTING_PORT},       /* transmission 1 */
     {FILL_IN_RUNTIME,PRV_TGF_NON_EXISTING_PORT},       /* transmission 2 */
     {FILL_IN_RUNTIME,PRV_TGF_NON_EXISTING_PORT},       /* transmission 3 */
     {FILL_IN_RUNTIME,PRV_TGF_NEXTHOPE_PORT_NUM_CNS},   /* transmission 4 */
     {FILL_IN_RUNTIME,PRV_TGF_NON_EXISTING_PORT},       /* transmission 5 */
     {FILL_IN_RUNTIME,PRV_TGF_NON_EXISTING_PORT},       /* transmission 6 */
     {FILL_IN_RUNTIME,PRV_TGF_NON_EXISTING_PORT}        /* transmission 7 */
    }},
    /* iteration 5 */
   {{
     {FILL_IN_RUNTIME,PRV_TGF_NON_EXISTING_PORT},       /* transmission 0 */
     {FILL_IN_RUNTIME,PRV_TGF_NON_EXISTING_PORT},       /* transmission 1 */
     {FILL_IN_RUNTIME,PRV_TGF_NON_EXISTING_PORT},       /* transmission 2 */
     {FILL_IN_RUNTIME,PRV_TGF_NON_EXISTING_PORT},       /* transmission 3 */
     {FILL_IN_RUNTIME,PRV_TGF_NON_EXISTING_PORT},       /* transmission 4 */
     {FILL_IN_RUNTIME,PRV_TGF_NEXTHOPE_PORT_NUM_CNS},   /* transmission 5 */
     {FILL_IN_RUNTIME,PRV_TGF_NON_EXISTING_PORT},       /* transmission 6 */
     {FILL_IN_RUNTIME,PRV_TGF_NON_EXISTING_PORT}        /* transmission 7 */
    }},
    /* iteration 6 */
   {{
     {FILL_IN_RUNTIME,PRV_TGF_NON_EXISTING_PORT},       /* transmission 0 */
     {FILL_IN_RUNTIME,PRV_TGF_NON_EXISTING_PORT},       /* transmission 1 */
     {FILL_IN_RUNTIME,PRV_TGF_NON_EXISTING_PORT},       /* transmission 2 */
     {FILL_IN_RUNTIME,PRV_TGF_NON_EXISTING_PORT},       /* transmission 3 */
     {FILL_IN_RUNTIME,PRV_TGF_NON_EXISTING_PORT},       /* transmission 4 */
     {FILL_IN_RUNTIME,PRV_TGF_NON_EXISTING_PORT},       /* transmission 5 */
     {FILL_IN_RUNTIME,PRV_TGF_NEXTHOPE_PORT_NUM_CNS},   /* transmission 6 */
     {FILL_IN_RUNTIME,PRV_TGF_NON_EXISTING_PORT}        /* transmission 7 */
    }},
    /* iteration 7 */
   {{
     {FILL_IN_RUNTIME,PRV_TGF_NON_EXISTING_PORT},       /* transmission 0 */
     {FILL_IN_RUNTIME,PRV_TGF_NON_EXISTING_PORT},       /* transmission 1 */
     {FILL_IN_RUNTIME,PRV_TGF_NON_EXISTING_PORT},       /* transmission 2 */
     {FILL_IN_RUNTIME,PRV_TGF_NON_EXISTING_PORT},       /* transmission 3 */
     {FILL_IN_RUNTIME,PRV_TGF_NON_EXISTING_PORT},       /* transmission 4 */
     {FILL_IN_RUNTIME,PRV_TGF_NON_EXISTING_PORT},       /* transmission 5 */
     {FILL_IN_RUNTIME,PRV_TGF_NON_EXISTING_PORT},       /* transmission 6 */
     {FILL_IN_RUNTIME,PRV_TGF_NEXTHOPE_PORT_NUM_CNS}    /* transmission 7 */
    }}
  }
};

/************************* Test 3: Port Rx Tx Counters ***********************/
static prvTgfPortRxTxCountersConfig TxRxTest3 =
{   /* iteration 0 */
  {
    {{
     {FILL_IN_RUNTIME,PRV_TGF_NEXTHOPE_PORT_NUM_CNS},   /* transmission 0 */
     {FILL_IN_RUNTIME,PRV_TGF_NON_EXISTING_PORT},       /* transmission 1 */
     {FILL_IN_RUNTIME,PRV_TGF_NEXTHOPE_PORT_NUM_CNS},   /* transmission 2 */
     {FILL_IN_RUNTIME,PRV_TGF_NON_EXISTING_PORT}    ,   /* transmission 3 */
     {FILL_IN_RUNTIME,PRV_TGF_NEXTHOPE_PORT_NUM_CNS},   /* transmission 4 */
     {FILL_IN_RUNTIME,PRV_TGF_NON_EXISTING_PORT},       /* transmission 5 */
     {FILL_IN_RUNTIME,PRV_TGF_NEXTHOPE_PORT_NUM_CNS},   /* transmission 6 */
     {FILL_IN_RUNTIME,PRV_TGF_NON_EXISTING_PORT}        /* transmission 7 */
    }},
    /* iteration 1 */
   {{
     {FILL_IN_RUNTIME,PRV_TGF_NON_EXISTING_PORT},       /* transmission 0 */
     {FILL_IN_RUNTIME,PRV_TGF_NEXTHOPE_PORT_NUM_CNS},   /* transmission 1 */
     {FILL_IN_RUNTIME,PRV_TGF_NON_EXISTING_PORT},       /* transmission 2 */
     {FILL_IN_RUNTIME,PRV_TGF_NEXTHOPE_PORT_NUM_CNS},   /* transmission 3 */
     {FILL_IN_RUNTIME,PRV_TGF_NON_EXISTING_PORT},       /* transmission 4 */
     {FILL_IN_RUNTIME,PRV_TGF_NEXTHOPE_PORT_NUM_CNS},   /* transmission 5 */
     {FILL_IN_RUNTIME,PRV_TGF_NON_EXISTING_PORT},       /* transmission 6 */
     {FILL_IN_RUNTIME,PRV_TGF_NEXTHOPE_PORT_NUM_CNS}    /* transmission 7 */
     }},
    /* iteration 2 */
   {{
     {FILL_IN_RUNTIME,PRV_TGF_NEXTHOPE_PORT_NUM_CNS},   /* transmission 0 */
     {FILL_IN_RUNTIME,PRV_TGF_NEXTHOPE_PORT_NUM_CNS},   /* transmission 1 */
     {FILL_IN_RUNTIME,PRV_TGF_NON_EXISTING_PORT},       /* transmission 2 */
     {FILL_IN_RUNTIME,PRV_TGF_NON_EXISTING_PORT},       /* transmission 3 */
     {FILL_IN_RUNTIME,PRV_TGF_NEXTHOPE_PORT_NUM_CNS},   /* transmission 4 */
     {FILL_IN_RUNTIME,PRV_TGF_NEXTHOPE_PORT_NUM_CNS},   /* transmission 5 */
     {FILL_IN_RUNTIME,PRV_TGF_NON_EXISTING_PORT},       /* transmission 6 */
     {FILL_IN_RUNTIME,PRV_TGF_NON_EXISTING_PORT}        /* transmission 7 */
   }},
    /* iteration 3 */
   {{
     {FILL_IN_RUNTIME,PRV_TGF_NON_EXISTING_PORT},       /* transmission 0 */
     {FILL_IN_RUNTIME,PRV_TGF_NON_EXISTING_PORT},       /* transmission 1 */
     {FILL_IN_RUNTIME,PRV_TGF_NEXTHOPE_PORT_NUM_CNS},   /* transmission 2 */
     {FILL_IN_RUNTIME,PRV_TGF_NEXTHOPE_PORT_NUM_CNS},   /* transmission 3 */
     {FILL_IN_RUNTIME,PRV_TGF_NON_EXISTING_PORT},       /* transmission 4 */
     {FILL_IN_RUNTIME,PRV_TGF_NON_EXISTING_PORT},       /* transmission 5 */
     {FILL_IN_RUNTIME,PRV_TGF_NEXTHOPE_PORT_NUM_CNS},   /* transmission 6 */
     {FILL_IN_RUNTIME,PRV_TGF_NEXTHOPE_PORT_NUM_CNS}    /* transmission 7 */
   }},


    /* iteration 4 */
    {{
     {FILL_IN_RUNTIME,PRV_TGF_NON_EXISTING_PORT},       /* transmission 0 */
     {FILL_IN_RUNTIME,PRV_TGF_NON_EXISTING_PORT},       /* transmission 1 */
     {FILL_IN_RUNTIME,PRV_TGF_NON_EXISTING_PORT},       /* transmission 2 */
     {FILL_IN_RUNTIME,PRV_TGF_NON_EXISTING_PORT}    ,   /* transmission 3 */
     {FILL_IN_RUNTIME,PRV_TGF_NEXTHOPE_PORT_NUM_CNS},   /* transmission 4 */
     {FILL_IN_RUNTIME,PRV_TGF_NEXTHOPE_PORT_NUM_CNS},   /* transmission 5 */
     {FILL_IN_RUNTIME,PRV_TGF_NEXTHOPE_PORT_NUM_CNS},   /* transmission 6 */
     {FILL_IN_RUNTIME,PRV_TGF_NEXTHOPE_PORT_NUM_CNS}    /* transmission 7 */
    }},
    /* iteration 5 */
   {{
     {FILL_IN_RUNTIME,PRV_TGF_NON_EXISTING_PORT},       /* transmission 0 */
     {FILL_IN_RUNTIME,PRV_TGF_NEXTHOPE_PORT_NUM_CNS},   /* transmission 1 */
     {FILL_IN_RUNTIME,PRV_TGF_NON_EXISTING_PORT},       /* transmission 2 */
     {FILL_IN_RUNTIME,PRV_TGF_NEXTHOPE_PORT_NUM_CNS},   /* transmission 3 */
     {FILL_IN_RUNTIME,PRV_TGF_NON_EXISTING_PORT},       /* transmission 4 */
     {FILL_IN_RUNTIME,PRV_TGF_NON_EXISTING_PORT},       /* transmission 5 */
     {FILL_IN_RUNTIME,PRV_TGF_NON_EXISTING_PORT},       /* transmission 6 */
     {FILL_IN_RUNTIME,PRV_TGF_NON_EXISTING_PORT}        /* transmission 7 */
     }},
    /* iteration 6 */
   {{
     {FILL_IN_RUNTIME,PRV_TGF_NON_EXISTING_PORT},       /* transmission 0 */
     {FILL_IN_RUNTIME,PRV_TGF_NON_EXISTING_PORT},       /* transmission 1 */
     {FILL_IN_RUNTIME,PRV_TGF_NON_EXISTING_PORT},       /* transmission 2 */
     {FILL_IN_RUNTIME,PRV_TGF_NON_EXISTING_PORT},       /* transmission 3 */
     {FILL_IN_RUNTIME,PRV_TGF_NON_EXISTING_PORT},       /* transmission 4 */
     {FILL_IN_RUNTIME,PRV_TGF_NON_EXISTING_PORT},       /* transmission 5 */
     {FILL_IN_RUNTIME,PRV_TGF_NEXTHOPE_PORT_NUM_CNS},   /* transmission 6 */
     {FILL_IN_RUNTIME,PRV_TGF_NEXTHOPE_PORT_NUM_CNS}    /* transmission 7 */
   }},
    /* iteration 7 */
   {{
     {FILL_IN_RUNTIME,PRV_TGF_NON_EXISTING_PORT},       /* transmission 0 */
     {FILL_IN_RUNTIME,PRV_TGF_NON_EXISTING_PORT},       /* transmission 1 */
     {FILL_IN_RUNTIME,PRV_TGF_NEXTHOPE_PORT_NUM_CNS},   /* transmission 2 */
     {FILL_IN_RUNTIME,PRV_TGF_NEXTHOPE_PORT_NUM_CNS},   /* transmission 3 */
     {FILL_IN_RUNTIME,PRV_TGF_NON_EXISTING_PORT},       /* transmission 4 */
     {FILL_IN_RUNTIME,PRV_TGF_NON_EXISTING_PORT},       /* transmission 5 */
     {FILL_IN_RUNTIME,PRV_TGF_NON_EXISTING_PORT},       /* transmission 6 */
     {FILL_IN_RUNTIME,PRV_TGF_NON_EXISTING_PORT}        /* transmission 7 */
   }}

   }
};

/******************************* Test packet **********************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x02},               /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x11}                /* saMac */
};
/* VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTagPart = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_SEND_VLANID_CNS                       /* pri, cfi, VlanId */
};
/* packet's ethertype */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketEtherTypePart = {TGF_ETHERTYPE_0800_IPV4_TAG_CNS};
/* packet's IPv4 */
static TGF_PACKET_IPV4_STC prvTgfPacketIpv4Part = {
    4,                  /* version */
    5,                  /* headerLen */
    0,                  /* typeOfService */
    0x2A,               /* totalLen */
    0,                  /* id */
    0,                  /* flags */
    0,                  /* offset */
    0x40,               /* timeToLive */
    0x04,               /* protocol */
    0x5EB5,             /* csum */
    {10, 10, 10, 10},   /* srcAddr */
    { 4,  4,  4,  4}    /* dstAddr */
};
/* packet's IPv4 over IPv4 */
static TGF_PACKET_IPV4_STC prvTgfPacketIpv4OverIpv4Part = {
    4,                  /* version */
    5,                  /* headerLen */
    0,                  /* typeOfService */
    0x16,               /* totalLen */
    0,                  /* id */
    0,                  /* flags */
    0,                  /* offset */
    0x40,               /* timeToLive */
    0xFF,               /* protocol */
    0x73E2,             /* csum */
    { 2,  2,  2,  2},   /* srcAddr */
    { 1,  1,  1,  3}    /* dstAddr */
};
/* DATA of packet */
static GT_U8 prvTgfPayloadDataArr[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19
};
/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart = {
    sizeof(prvTgfPayloadDataArr),                       /* dataLength */
    prvTgfPayloadDataArr                                /* dataPtr */
};

/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfPacketPartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacketIpv4Part},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacketIpv4OverIpv4Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/*************************** Restore config ***********************************/

/* parameters that is needed to be restored */
static struct
{
    PRV_TGF_TTI_MAC_MODE_ENT      macMode;
} prvTgfRestoreCfg;

/******************************************************************************/


/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/**
* @internal prvTgfTunnelTermGlobalPort2LocalPortBaseConfigurationSet function
* @endinternal
*
* @brief   Set Base Configuration
*/
GT_VOID prvTgfTunnelTermGlobalPort2LocalPortBaseConfigurationSet
(
    GT_VOID
)
{
    GT_BOOL                     isTagged  = GT_FALSE;
    GT_U32                      vlanIter  = 0;
    GT_U32                      portIter  = 0;
    GT_U32                      vlanCount = sizeof(prvTgfVlanArray);
    GT_STATUS                   rc        = GT_OK;
    CPSS_PORTS_BMP_STC          portsMembers = {{0, 0}};
    CPSS_PORTS_BMP_STC          portsTagging = {{0, 0}};
    PRV_TGF_BRG_VLAN_INFO_STC   vlanInfo;
    PRV_TGF_BRG_MAC_ENTRY_STC   macEntry;
    PRV_TGF_BRG_VLAN_PORTS_TAG_CMD_STC portsTaggingCmd;
    CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_ENT  precedence;

    PRV_UTF_LOG0_MAC("======= Setting Base Configuration =======\n");

    /* set vlan entry */
    cpssOsMemSet(&vlanInfo, 0, sizeof(vlanInfo));
    cpssOsMemSet(&portsTaggingCmd, 0, sizeof(portsTaggingCmd));

    vlanInfo.unkSrcAddrSecBreach  = GT_FALSE;
    vlanInfo.unregNonIpMcastCmd   = CPSS_PACKET_CMD_FORWARD_E;
    vlanInfo.unregIpv4McastCmd    = CPSS_PACKET_CMD_FORWARD_E;
    vlanInfo.unregIpv6McastCmd    = CPSS_PACKET_CMD_FORWARD_E;
    vlanInfo.unkUcastCmd          = CPSS_PACKET_CMD_FORWARD_E;
    vlanInfo.unregIpv4BcastCmd    = CPSS_PACKET_CMD_FORWARD_E;
    vlanInfo.unregNonIpv4BcastCmd = CPSS_PACKET_CMD_FORWARD_E;
    vlanInfo.ipv4IgmpToCpuEn      = GT_FALSE;
    vlanInfo.mirrToRxAnalyzerEn   = GT_FALSE;
    vlanInfo.ipv6IcmpToCpuEn      = GT_FALSE;
    vlanInfo.ipCtrlToCpuEn        = PRV_TGF_BRG_IP_CTRL_NONE_E;
    vlanInfo.ipv4IpmBrgMode       = CPSS_BRG_IPM_SGV_E;
    vlanInfo.ipv6IpmBrgMode       = CPSS_BRG_IPM_SGV_E;
    vlanInfo.ipv4IpmBrgEn         = GT_FALSE;
    vlanInfo.ipv6IpmBrgEn         = GT_FALSE;
    vlanInfo.ipv6SiteIdMode       = CPSS_IP_SITE_ID_INTERNAL_E;
    vlanInfo.ipv4UcastRouteEn     = GT_FALSE;
    vlanInfo.ipv4McastRouteEn     = GT_FALSE;
    vlanInfo.ipv6UcastRouteEn     = GT_FALSE;
    vlanInfo.ipv6McastRouteEn     = GT_FALSE;
    vlanInfo.stgId                = 0;
    vlanInfo.autoLearnDisable     = GT_TRUE;
    vlanInfo.naMsgToCpuEn         = GT_TRUE;
    vlanInfo.mruIdx               = 0;
    vlanInfo.bcastUdpTrapMirrEn   = GT_FALSE;
    vlanInfo.floodVidx            = 0xFFF;
    vlanInfo.floodVidxMode        = PRV_TGF_BRG_VLAN_FLOOD_VIDX_MODE_ALL_FLOODED_TRAFFIC_E;
    vlanInfo.portIsolationMode    = PRV_TGF_BRG_VLAN_PORT_ISOLATION_DISABLE_CMD_E;
    vlanInfo.ucastLocalSwitchingEn = GT_FALSE;
    vlanInfo.mcastLocalSwitchingEn = GT_FALSE;

    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        portsTaggingCmd.portsCmd[prvTgfPortsArray[portIter]] = PRV_TGF_BRG_VLAN_PORT_UNTAGGED_CMD_E;
    }

    /* create vlans */
    for (vlanIter = 0; vlanIter < vlanCount; vlanIter++)
    {
        rc = prvTgfBrgVlanEntryWrite(prvTgfDevNum, prvTgfVlanArray[vlanIter],
                                     &portsMembers, &portsTagging,
                                     &vlanInfo, &portsTaggingCmd);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEntryWrite: %d, %d",
                                     prvTgfDevNum, prvTgfVlanArray[vlanIter]);
    }

    /* add ports to vlan member */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfBrgVlanMemberAdd(prvTgfDevNum, prvTgfVlanPerPortArray[portIter],
                                    prvTgfPortsArray[portIter], isTagged);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanMemberAdd: %d, %d, %d, %d",
                                     prvTgfDevNum, prvTgfVlanPerPortArray[portIter],
                                     prvTgfPortsArray[portIter], isTagged);
    }

    precedence = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E;

    if(prvTgfPortsNum == 1)
    {
        /* otherwise the loop will not initialize it */
        prvTgfDefVlanId = 1;
    }

    for (portIter = 0; portIter < (GT_U32)(prvTgfPortsNum-1); portIter++)
    {
        /* set Port VID Precedence */
        rc = prvTgfBrgVlanPortVidPrecedenceSet(prvTgfDevNum, prvTgfPortsArray[portIter], precedence);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidPrecedenceSet: %d, %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter], precedence);

        /* get default vlanId */
        rc = prvTgfBrgVlanPortVidGet(prvTgfDevNum, prvTgfPortsArray[portIter], &prvTgfDefVlanId);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidGet: %d %d", prvTgfDevNum, prvTgfDefVlanId);

        /* set default vlanId */
        rc = prvTgfBrgVlanPortVidSet(prvTgfDevNum, prvTgfPortsArray[portIter], PRV_TGF_SEND_VLANID_CNS);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidSet: %d, %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter], PRV_TGF_SEND_VLANID_CNS);
    }

    /* enable VLAN based MAC learning */
    rc = prvTgfBrgVlanLearningStateSet(PRV_TGF_SEND_VLANID_CNS, GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanLearningStateSet: %d", prvTgfDevNum);

    /* create a macEntry with .daRoute = GT_TRUE */
    cpssOsMemSet(&macEntry, 0, sizeof(macEntry));
    cpssOsMemCpy(macEntry.key.key.macVlan.macAddr.arEther, prvTgfPacketL2Part.daMac, sizeof(TGF_MAC_ADDR));

    macEntry.key.entryType                  = PRV_TGF_FDB_ENTRY_TYPE_MAC_ADDR_E;
    macEntry.key.key.macVlan.vlanId         = PRV_TGF_SEND_VLANID_CNS;
    macEntry.dstInterface.type              = CPSS_INTERFACE_PORT_E;
    macEntry.dstInterface.devPort.hwDevNum    = prvTgfDevNum;
    macEntry.dstInterface.devPort.portNum   = prvTgfPortsArray[PRV_TGF_FDB_PORT_IDX_CNS];
    macEntry.isStatic                       = GT_TRUE;
    macEntry.daCommand                      = PRV_TGF_PACKET_CMD_FORWARD_E;
    macEntry.saCommand                      = PRV_TGF_PACKET_CMD_FORWARD_E;
    macEntry.daRoute                        = GT_TRUE;
    macEntry.sourceId                       = 0;
    macEntry.userDefined                    = 0;
    macEntry.daQosIndex                     = 0;
    macEntry.saQosIndex                     = 0;
    macEntry.daSecurityLevel                = 0;
    macEntry.saSecurityLevel                = 0;
    macEntry.appSpecificCpuCode             = GT_FALSE;
    macEntry.mirrorToRxAnalyzerPortEn       = GT_FALSE;

    rc = prvTgfBrgFdbMacEntrySet(&macEntry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbMacEntryWrite: %d", prvTgfDevNum);
}

/**
* @internal prvTgfTunnelTermGlobalPort2LocalPortLttRouteConfigurationSet function
* @endinternal
*
* @brief   Set LTT Route Configuration
*/
static GT_VOID prvTgfTunnelTermGlobalPort2LocalPortLttRouteConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS                               rc = GT_OK;
    PRV_TGF_INLIF_ENTRY_STC                 inlifEntry;
    GT_ETHERADDR                            arpMacAddr;
    PRV_TGF_IP_UC_ROUTE_ENTRY_STC           routeEntriesArray[1];
    PRV_TGF_IP_UC_ROUTE_ENTRY_STC           *regularEntryPtr;
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT    nextHopInfo;
    GT_IPADDR                               ipAddr;
    GT_U8                                   portIter  = 0;


    PRV_UTF_LOG0_MAC("======= Setting LTT Route Configuration =======\n");

    /* -------------------------------------------------------------------------
     * 0. Set Inlif Entry - that needs for Puma devices
     */

    /* Bridge Section */
    inlifEntry.bridgeEnable              = GT_TRUE;
    inlifEntry.autoLearnEnable           = GT_TRUE;
    inlifEntry.naMessageToCpuEnable      = GT_TRUE;
    inlifEntry.naStormPreventionEnable   = GT_FALSE;
    inlifEntry.unkSaUcCommand            = CPSS_PACKET_CMD_BRIDGE_E;
    inlifEntry.unkDaUcCommand            = CPSS_PACKET_CMD_FORWARD_E;
    inlifEntry.unkSaNotSecurBreachEnable = GT_TRUE;
    inlifEntry.untaggedMruIndex          = 0;
    inlifEntry.unregNonIpMcCommand       = CPSS_PACKET_CMD_FORWARD_E;
    inlifEntry.unregIpMcCommand          = CPSS_PACKET_CMD_FORWARD_E;
    inlifEntry.unregIpv4BcCommand        = CPSS_PACKET_CMD_FORWARD_E;
    inlifEntry.unregNonIpv4BcCommand     = CPSS_PACKET_CMD_FORWARD_E;

    /* Router Section */
    inlifEntry.ipv4UcRouteEnable = GT_TRUE;
    inlifEntry.ipv4McRouteEnable = GT_FALSE;
    inlifEntry.ipv6UcRouteEnable = GT_FALSE;
    inlifEntry.ipv6McRouteEnable = GT_FALSE;
    inlifEntry.mplsRouteEnable   = GT_FALSE;
    inlifEntry.vrfId             = 0;

    /* SCT Section */
    inlifEntry.ipv4IcmpRedirectEnable         = GT_FALSE;
    inlifEntry.ipv6IcmpRedirectEnable         = GT_FALSE;
    inlifEntry.bridgeRouterInterfaceEnable    = GT_FALSE;
    inlifEntry.ipSecurityProfile              = 0;
    inlifEntry.ipv4IgmpToCpuEnable            = GT_FALSE;
    inlifEntry.ipv6IcmpToCpuEnable            = GT_FALSE;
    inlifEntry.udpBcRelayEnable               = GT_FALSE;
    inlifEntry.arpBcToCpuEnable               = GT_FALSE;
    inlifEntry.arpBcToMeEnable                = GT_FALSE;
    inlifEntry.ripv1MirrorEnable              = GT_FALSE;
    inlifEntry.ipv4LinkLocalMcCommandEnable   = GT_FALSE;
    inlifEntry.ipv6LinkLocalMcCommandEnable   = GT_FALSE;
    inlifEntry.ipv6NeighborSolicitationEnable = GT_FALSE;

    /* Misc Section */
    inlifEntry.mirrorToAnalyzerEnable = GT_FALSE;
    inlifEntry.mirrorToCpuEnable      = GT_FALSE;

    /* call api function */
    rc = prvTgfInlifEntrySet(PRV_TGF_INLIF_TYPE_EXTERNAL_E, 4096, &inlifEntry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfInlifEntrySet: %d", prvTgfDevNum);

    /* -------------------------------------------------------------------------
     * 1. Enable Routing
     */

    for (portIter = 0; portIter < (prvTgfPortsNum -1); portIter++)
    {
        /* enable Unicast IPv4 Routing on a Port */
        rc = prvTgfIpPortRoutingEnable(portIter, CPSS_IP_UNICAST_E,
                                       CPSS_IP_PROTOCOL_IPV4_E, GT_TRUE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpPortRoutingEnable: %d %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* enable IPv4 Unicast Routing on Vlan */
    rc = prvTgfIpVlanRoutingEnable(PRV_TGF_SEND_VLANID_CNS, CPSS_IP_UNICAST_E, CPSS_IP_PROTOCOL_IPV4_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpVlanRoutingEnable: %d %d",
                                 prvTgfDevNum, PRV_TGF_SEND_VLANID_CNS);

    /* -------------------------------------------------------------------------
     * 2. Create the Route entry (Next hop) in Route table and Router ARP Table
     */

    /* write a ARP MAC address to the Router ARP Table */
    cpssOsMemCpy(arpMacAddr.arEther, prvTgfArpMac, sizeof(TGF_MAC_ADDR));

    rc = prvTgfIpRouterArpAddrWrite(prvTgfRouterArpIndex, &arpMacAddr);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpRouterArpAddrWrite: %d", prvTgfDevNum);

    /* write a UC Route entry to the Route Table */
    cpssOsMemSet(routeEntriesArray, 0, sizeof(routeEntriesArray));

    regularEntryPtr = &routeEntriesArray[0];
    regularEntryPtr->cmd                        = CPSS_PACKET_CMD_ROUTE_E;
    regularEntryPtr->cpuCodeIndex               = 0;
    regularEntryPtr->appSpecificCpuCodeEnable   = GT_FALSE;
    regularEntryPtr->unicastPacketSipFilterEnable = GT_FALSE;
    regularEntryPtr->ttlHopLimitDecEnable       = GT_FALSE;
    regularEntryPtr->ttlHopLimDecOptionsExtChkByPass = GT_FALSE;
    regularEntryPtr->ingressMirror              = GT_FALSE;
    regularEntryPtr->qosProfileMarkingEnable    = GT_FALSE;
    regularEntryPtr->qosProfileIndex            = 0;
    regularEntryPtr->qosPrecedence              = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E;
    regularEntryPtr->modifyUp                   = CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E;
    regularEntryPtr->modifyDscp                 = CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E;
    regularEntryPtr->countSet                   = PRV_TGF_COUNT_SET_CNS;
    regularEntryPtr->trapMirrorArpBcEnable      = GT_FALSE;
    regularEntryPtr->sipAccessLevel             = 0;
    regularEntryPtr->dipAccessLevel             = 0;
    regularEntryPtr->ICMPRedirectEnable         = GT_FALSE;
    regularEntryPtr->scopeCheckingEnable        = GT_FALSE;
    regularEntryPtr->siteId                     = CPSS_IP_SITE_ID_INTERNAL_E;
    regularEntryPtr->mtuProfileIndex            = 0;
    regularEntryPtr->isTunnelStart              = GT_FALSE;
    regularEntryPtr->nextHopVlanId              = PRV_TGF_NEXTHOPE_VLANID_CNS;
    regularEntryPtr->nextHopInterface.type      = CPSS_INTERFACE_PORT_E;
    regularEntryPtr->nextHopInterface.devPort.hwDevNum = prvTgfDevNum;
    regularEntryPtr->nextHopInterface.devPort.portNum = nextHopPortNum;
    regularEntryPtr->nextHopARPPointer          = prvTgfRouterArpIndex;
    regularEntryPtr->nextHopTunnelPointer       = 0;

    rc = prvTgfIpUcRouteEntriesWrite(prvTgfRouteEntryBaseIndex, routeEntriesArray, 1);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpUcRouteEntriesWrite: %d", prvTgfDevNum);


    /* reading and checkiug the UC Route entry from the Route Table */
    cpssOsMemSet(routeEntriesArray, 0, sizeof(routeEntriesArray));

    rc = prvTgfIpUcRouteEntriesRead(prvTgfDevNum, prvTgfRouteEntryBaseIndex, routeEntriesArray, 1);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpUcRouteEntriesRead: %d", prvTgfDevNum);

    PRV_UTF_LOG2_MAC("nextHopVlanId = %d, portNum = %d\n",
                     routeEntriesArray[0].nextHopVlanId,
                     routeEntriesArray[0].nextHopInterface.devPort.portNum);

    /* -------------------------------------------------------------------------
     * 3. Create a new Ipv4 prefix in a Virtual Router for the specified LPM DB
     */

    /* fill a nexthope info for the prefix */
    cpssOsMemSet(&nextHopInfo, 0, sizeof(nextHopInfo));
    nextHopInfo.ipLttEntry.routeEntryBaseIndex = prvTgfRouteEntryBaseIndex;
    nextHopInfo.ipLttEntry.routeType = PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_MAC;

    /* fill a destination IP address for the prefix */
    cpssOsMemCpy(ipAddr.arIP, prvTgfPacketIpv4OverIpv4Part.dstAddr, sizeof(ipAddr.arIP));

    /* call CPSS function */
    rc = prvTgfIpLpmIpv4UcPrefixAdd(prvTgfLpmDBId, 0, ipAddr, 32, &nextHopInfo, GT_FALSE, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4UcPrefixAdd: %d", prvTgfDevNum);
}

/**
* @internal prvTgfTunnelTermGlobalPort2LocalPortPbrConfigurationSet function
* @endinternal
*
* @brief   Set PBR Configuration
*/
static GT_VOID prvTgfTunnelTermGlobalPort2LocalPortPbrConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS                               rc = GT_OK;
    GT_ETHERADDR                            arpMacAddr;
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT    nextHopInfo;
    GT_IPADDR                               ipAddr;
    GT_U32                                  portIter = 0;

    PRV_UTF_LOG0_MAC("==== Setting PBR Configuration ====\n");


    /* -------------------------------------------------------------------------
     * 1. PCL Config
     */

    /* init PCL Engine for send port */
    for (portIter = 0; portIter < (GT_U32)(prvTgfPortsNum-1); portIter++)
    {
        rc = prvTgfPclDefPortInitExt1(
            prvTgfPortsArray[portIter],
            CPSS_PCL_DIRECTION_INGRESS_E,
            CPSS_PCL_LOOKUP_1_E,/* PBR must be in lookup 1 ! */
            PRV_TGF_PCL_PBR_ID_FOR_VIRTUAL_ROUTER_CNS, /* pclId */
            PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E /*nonIpKey*/,
            PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E /*ipv4Key*/,
            PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV6_DIP_E /*ipv6Key*/);
        UTF_VERIFY_EQUAL1_STRING_MAC(
            GT_OK, rc, "prvTgfPclDefPortInitExt1: %d", prvTgfDevNum);
    }

    /* -------------------------------------------------------------------------
     * 2. IP Config
     */

    /* enable IPv4 Unicast Routing on Vlan */
    rc = prvTgfIpVlanRoutingEnable(PRV_TGF_SEND_VLANID_CNS, CPSS_IP_UNICAST_E, CPSS_IP_PROTOCOL_IPV4_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpVlanRoutingEnable: %d %d",
                                 prvTgfDevNum, PRV_TGF_SEND_VLANID_CNS);

    /* write ARP MAC address to the Router ARP Table */
    cpssOsMemCpy(arpMacAddr.arEther, prvTgfArpMac, sizeof(TGF_MAC_ADDR));

    rc = prvTgfIpRouterArpAddrWrite(prvTgfRouterArpIndex, &arpMacAddr);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpRouterArpAddrWrite: %d", prvTgfDevNum);

    /* -------------------------------------------------------------------------
     * 3. Create a new Ipv4 prefix in a Virtual Router for the specified LPM DB
     */

    /* fill a nexthope info for the prefix */
    cpssOsMemSet(&nextHopInfo, 0, sizeof(nextHopInfo));

    nextHopInfo.pclIpUcAction.pktCmd                        = CPSS_PACKET_CMD_FORWARD_E;
    nextHopInfo.pclIpUcAction.mirror.cpuCode                = CPSS_NET_CONTROL_E;
    nextHopInfo.pclIpUcAction.matchCounter.enableMatchCount = GT_TRUE;
    nextHopInfo.pclIpUcAction.vlan.vlanId                   = PRV_TGF_NEXTHOPE_VLANID_CNS;
    nextHopInfo.pclIpUcAction.ipUcRoute.doIpUcRoute         = GT_TRUE;
    nextHopInfo.pclIpUcAction.ipUcRoute.arpDaIndex          = prvTgfRouterArpIndex;

    nextHopInfo.pclIpUcAction.redirect.redirectCmd          = PRV_TGF_PCL_ACTION_REDIRECT_CMD_OUT_IF_E;
    nextHopInfo.pclIpUcAction.redirect.data.outIf.outInterface.type            = CPSS_INTERFACE_PORT_E;
    nextHopInfo.pclIpUcAction.redirect.data.outIf.outInterface.devPort.hwDevNum  = prvTgfDevNum;
    nextHopInfo.pclIpUcAction.redirect.data.outIf.outInterface.devPort.portNum = nextHopPortNum;

    /* fill a destination IP address for the prefix */
    cpssOsMemCpy(ipAddr.arIP, prvTgfPacketIpv4OverIpv4Part.dstAddr, sizeof(ipAddr.arIP));

    /* call CPSS function */
    rc = prvTgfIpLpmIpv4UcPrefixAdd(prvTgfLpmDBId, 0, ipAddr, 32, &nextHopInfo, GT_TRUE, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4UcPrefixAdd: %d", prvTgfDevNum);
}

/**
* @internal prvTgfTunnelTermGlobalPort2LocalPortRouteConfigurationSet function
* @endinternal
*
* @brief   Set Route Configuration
*/
GT_VOID prvTgfTunnelTermGlobalPort2LocalPortRouteConfigurationSet
(
    GT_VOID
)
{
    PRV_TGF_IP_ROUTING_MODE_ENT routingMode;
    GT_STATUS                   rc = GT_OK;

    /* get routing mode */
    rc = prvTgfIpRoutingModeGet(&routingMode);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpRoutingModeGet: %d", prvTgfDevNum);

    switch(routingMode)
    {
        case PRV_TGF_IP_ROUTING_MODE_PCL_ACTION_E:

            prvTgfTunnelTermGlobalPort2LocalPortPbrConfigurationSet();
            break;

        case PRV_TGF_IP_ROUTING_MODE_IP_LTT_ENTRY_E:

            prvTgfTunnelTermGlobalPort2LocalPortLttRouteConfigurationSet();
            break;

        case PRV_TGF_IP_ROUTING_MODE_UNDEFINED_E:

            break;
    }
}

/**
* @internal prvTgfTunnelTermGlobalPort2LocalPortTtiConfigurationSet function
* @endinternal
*
* @brief   Set TTI Configuration
*/
GT_VOID prvTgfTunnelTermGlobalPort2LocalPortTtiConfigurationSet
(
    GT_U32      testNum,
    GT_U32      configuration
)
{
    GT_STATUS                rc = GT_OK;
    PRV_TGF_TTI_ACTION_STC   ttiAction = {0};
    PRV_TGF_TTI_RULE_UNT     ttiPattern;
    PRV_TGF_TTI_RULE_UNT     ttiMask;
    GT_U32                   portIter = 0;
    prvTgfPortTestConfigStruct *confTest;

    prvTgfTtiTcamIndexIsRelativeSet(GT_TRUE);

    cpssOsMemSet(&ttiPattern, 0, sizeof(ttiPattern));
    cpssOsMemSet(&ttiMask, 0, sizeof(ttiMask));
    cpssOsMemSet(&ttiAction, 0, sizeof(ttiAction));

    switch (testNum)
    {
    case 1:
        confTest = (prvTgfDevicePortMode == PRV_TGF_DEVICE_PORT_MODE_XLG_E) ? &confTest1_XLG : &confTest1;
        break;
    case 2:
        confTest = (prvTgfDevicePortMode == PRV_TGF_DEVICE_PORT_MODE_XLG_E) ? &confTest2_XLG : &confTest2;
        break;
    case 3:
        confTest = (prvTgfDevicePortMode == PRV_TGF_DEVICE_PORT_MODE_XLG_E) ? &confTest3_XLG : &confTest3;
        break;
    default:
        UTF_VERIFY_EQUAL0_STRING_MAC(0, 1, "test number is incorrect");
        return;
    }

    PRV_UTF_LOG0_MAC("======= Setting TTI Configuration =======\n");

    /* enable the TTI lookup for IPv4 at the port */
    for (portIter = 0; portIter < prvTgfPortsNum ; portIter++)
    {
        rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[portIter], PRV_TGF_TTI_KEY_IPV4_E, GT_TRUE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet: %d", prvTgfDevNum);
    }

    /* save the current lookup Mac mode for IPv4 and DA */
    rc = prvTgfTtiMacModeGet(prvTgfDevNum, PRV_TGF_TTI_KEY_IPV4_E, &prvTgfRestoreCfg.macMode);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiMacModeGet: %d", prvTgfDevNum);

    /* set the lookup Mac mode for IPv4 and DA */
    rc = prvTgfTtiMacModeSet(PRV_TGF_TTI_KEY_IPV4_E, PRV_TGF_TTI_MAC_MODE_DA_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiMacModeSet: %d", prvTgfDevNum);

    /* enable the IPv4 TTI lookup for only tunneled packets received on port */
    for (portIter = 0; portIter < (GT_U32)(prvTgfPortsNum-1); portIter++)
    {
        rc = prvTgfTtiPortIpv4OnlyTunneledEnableSet(prvTgfPortsArray[portIter], GT_TRUE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPortIpv4OnlyTunneledEnableSet: %d", prvTgfDevNum);
    }


    /* set the TTI Rule Pattern, Mask and Action for IPV4 TCAM location */

    /* set TTI Action */
    ttiAction.command                       = CPSS_PACKET_CMD_FORWARD_E;
    ttiAction.redirectCommand               = PRV_TGF_TTI_NO_REDIRECT_E;
    ttiAction.userDefinedCpuCode            = CPSS_NET_FIRST_USER_DEFINED_E;
    ttiAction.tunnelTerminate               = GT_TRUE;
    ttiAction.passengerPacketType           = PRV_TGF_TTI_PASSENGER_IPV4_E;
    ttiAction.copyTtlFromTunnelHeader       = GT_FALSE;
    ttiAction.mirrorToIngressAnalyzerEnable = GT_FALSE;
    ttiAction.policerIndex                  = 0;
    ttiAction.interfaceInfo.type            = CPSS_INTERFACE_PORT_E;
    ttiAction.interfaceInfo.devPort.hwDevNum  = prvTgfDevNum;
    ttiAction.interfaceInfo.devPort.portNum = PRV_TGF_SEND_PORT1_NUM_CNS;
    ttiAction.vlanPrecedence                = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_HARD_E;
    ttiAction.vlanCmd                       = PRV_TGF_TTI_VLAN_DO_NOT_MODIFY_E;
    ttiAction.vlanId                        = 0;
    ttiAction.nestedVlanEnable              = GT_FALSE;
    ttiAction.tunnelStart                   = GT_FALSE;
    ttiAction.tunnelStartPtr                = 0;
    ttiAction.routerLookupPtr               = 0;
    ttiAction.vrfId                         = 0;
    ttiAction.sourceIdSetEnable             = GT_FALSE;
    ttiAction.sourceId                      = 0;
    ttiAction.bindToPolicer                 = GT_FALSE;
    ttiAction.qosPrecedence                 = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E;
    ttiAction.qosTrustMode                  = PRV_TGF_TTI_QOS_KEEP_PREVIOUS_E;
    ttiAction.qosProfile                    = 0;
    ttiAction.modifyUpEnable                = PRV_TGF_TTI_DO_NOT_MODIFY_PREV_UP_E;
    ttiAction.modifyDscpEnable              = PRV_TGF_TTI_DO_NOT_MODIFY_PREV_DSCP_E;
    ttiAction.up                            = 0;
    ttiAction.remapDSCP                     = GT_FALSE;
    ttiAction.vntl2Echo                     = GT_FALSE;
    ttiAction.bridgeBypass                  = GT_FALSE;
    ttiAction.actionStop                    = GT_FALSE;
    ttiAction.activateCounter               = GT_FALSE;
    ttiAction.counterIndex                  = 0;

    /* set TTI Pattern */
    cpssOsMemCpy(ttiPattern.ipv4.common.mac.arEther, prvTgfPacketL2Part.daMac, sizeof(TGF_MAC_ADDR));
    cpssOsMemCpy(ttiPattern.ipv4.srcIp.arIP, prvTgfPacketIpv4Part.srcAddr, sizeof(TGF_IPV4_ADDR));
    cpssOsMemCpy(ttiPattern.ipv4.destIp.arIP, prvTgfPacketIpv4Part.dstAddr, sizeof(TGF_IPV4_ADDR));

    ttiPattern.ipv4.common.pclId            = 1;
    ttiPattern.ipv4.common.srcIsTrunk       = confTest->conf[configuration].pattern.srcIsTrunk;
    ttiPattern.ipv4.common.srcPortTrunk     = confTest->conf[configuration].pattern.srcPortTrunk;
    ttiPattern.ipv4.common.vid              = PRV_TGF_SEND_VLANID_CNS;
    ttiPattern.ipv4.common.isTagged         = GT_TRUE;
    ttiPattern.ipv4.common.dsaSrcIsTrunk    = confTest->conf[configuration].pattern.dsaSrcIsTrunk;
    ttiPattern.ipv4.common.dsaSrcPortTrunk  = confTest->conf[configuration].pattern.dsaSrcPortTrunk;
    ttiPattern.ipv4.common.dsaSrcDevice     = confTest->conf[configuration].pattern.dsaSrcDevice;
    ttiPattern.ipv4.tunneltype              = 0;
    ttiPattern.ipv4.isArp                   = GT_FALSE;

    /* set TTI Mask */
    cpssOsMemCpy(&ttiMask, &ttiPattern, sizeof(ttiPattern));
    cpssOsMemCpy(ttiMask.ipv4.common.mac.arEther, prvTgfFullMaskMac, sizeof(TGF_MAC_ADDR));
    cpssOsMemCpy(ttiMask.ipv4.srcIp.arIP, prvTgfFullMaskIp, sizeof(TGF_IPV4_ADDR));
    cpssOsMemCpy(ttiMask.ipv4.destIp.arIP, prvTgfFullMaskIp, sizeof(TGF_IPV4_ADDR));

    ttiMask.ipv4.common.srcIsTrunk       = confTest->conf[configuration].mask.srcIsTrunk;
    ttiMask.ipv4.common.srcPortTrunk     = confTest->conf[configuration].mask.srcPortTrunk;
    ttiMask.ipv4.common.dsaSrcIsTrunk    = confTest->conf[configuration].mask.dsaSrcIsTrunk;
    ttiMask.ipv4.common.dsaSrcPortTrunk  = confTest->conf[configuration].mask.dsaSrcPortTrunk;
    ttiMask.ipv4.common.dsaSrcDevice     = confTest->conf[configuration].mask.dsaSrcDevice;

    rc = prvTgfTtiRuleSet(prvTgfTtiRuleIndex, PRV_TGF_TTI_KEY_IPV4_E,
                          &ttiPattern, &ttiMask, &ttiAction);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleSet: %d", prvTgfDevNum);

    /* check TTI Rule */
    cpssOsMemSet(&ttiPattern, 0, sizeof(ttiPattern));
    cpssOsMemSet(&ttiMask, 0, sizeof(ttiMask));
    cpssOsMemSet(&ttiAction, 0, sizeof(ttiAction));

    rc = prvTgfTtiRuleGet(prvTgfDevNum, prvTgfTtiRuleIndex, PRV_TGF_TTI_KEY_IPV4_E,
                          &ttiPattern, &ttiMask, &ttiAction);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleGet: %d", prvTgfDevNum);

    PRV_UTF_LOG3_MAC(" Pattern&Mask .ipv4.common.vid = %d, %d\n" \
                     " Action .vlanPrecedence = %d\n",
                     ttiPattern.ipv4.common.vid,
                     ttiMask.ipv4.common.vid,
                     ttiAction.vlanPrecedence);
}

/**
* @internal prvTgfTunnelTermGlobalPort2LocalPortRuleValidStatusSet function
* @endinternal
*
* @brief   Set TTI Rule Valid Status
*/
GT_VOID prvTgfTunnelTermGlobalPort2LocalPortRuleValidStatusSet
(
    GT_BOOL   validStatus
)
{
    GT_STATUS       rc = GT_OK;

    PRV_UTF_LOG0_MAC("======= Setting TTI Rule Valid Status =======\n");

    rc = prvTgfTtiRuleValidStatusSet(prvTgfTtiRuleIndex, validStatus);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleValidStatusSet: %d", prvTgfDevNum);
}


/**
* @internal prvTgfTunnelTermGlobalPort2LocalPortTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfTunnelTermGlobalPort2LocalPortTrafficGenerate
(
    GT_U32 testNum,
    GT_U32 iteration,
    GT_U32 transmission
)
{
    GT_STATUS                       rc          = GT_OK;
    GT_U32                          partsCount  = 0;
    GT_U32                          packetSize  = 0;
    TGF_PACKET_STC                  packetInfo;
    GT_U32                          portIter    = 0;
    GT_U32                          numTriggers = 0;
    TGF_VFD_INFO_STC                vfdArray[1];
    CPSS_INTERFACE_INFO_STC         portInterface;
    CPSS_PORT_MAC_COUNTER_SET_STC   portCntrs;
    CPSS_PORT_MAC_COUNTER_SET_STC   expectedCntrs;
    PRV_TGF_IP_COUNTER_SET_STC      ipCounters;
    prvTgfPortRxTxCountersConfig   *test;
    static GT_U32   initRuntime = 0;

    utfGeneralStateMessageSave(1,"testNum = %d , iteration = %d , transmission = %d ",testNum,
        iteration, transmission);

    /* clear VFD array */
    cpssOsMemSet((GT_VOID*) vfdArray, 0, sizeof(vfdArray));

    if(initRuntime == 0)
    {
        GT_U32  ii,jj;
        GT_U32  iiMax,jjMax;
        initRuntime = 1;

        iiMax =  sizeof(TxRxTest1.iteration) / sizeof(TxRxTest1.iteration[0]);
        jjMax =  sizeof(TxRxTest1.iteration[0].transmission) / sizeof(TxRxTest1.iteration[0].transmission[0]);

        for(ii = 0 ; ii <  iiMax; ii++)
        {
            for(jj = 0 ; jj <  jjMax; jj++)
            {
                TxRxTest1.iteration[ii].transmission[jj].Tx =
                TxRxTest2.iteration[ii].transmission[jj].Tx =
                TxRxTest3.iteration[ii].transmission[jj].Tx = (GT_U8)(
                                         (prvTgfDevicePortMode == PRV_TGF_DEVICE_PORT_MODE_XLG_E) ?
                                         prvTgfTtiMultiPortGroupLocalPort2_XLG_Array[jj]:
                                         prvTgfTtiMultiPortGroupLocalPort2Array[jj]);

                if(MUST_USE_ORIG_PORTS_MAC(prvTgfDevNum))
                {
                    GT_U32  nextHopeIndex = 3;
                    /* do not modify the 12 ports in prvTgfPortsArray[] !!! */
                    if(PRV_TGF_NEXTHOPE_PORT_NUM_CNS == TxRxTest1.iteration[ii].transmission[jj].Rx)
                    {
                        TxRxTest1.iteration[ii].transmission[jj].Rx = prvTgfPortsArray[nextHopeIndex];
                    }
                    if(PRV_TGF_NEXTHOPE_PORT_NUM_CNS == TxRxTest2.iteration[ii].transmission[jj].Rx)
                    {
                        TxRxTest2.iteration[ii].transmission[jj].Rx = prvTgfPortsArray[nextHopeIndex];
                    }
                    if(PRV_TGF_NEXTHOPE_PORT_NUM_CNS == TxRxTest3.iteration[ii].transmission[jj].Rx)
                    {
                        TxRxTest3.iteration[ii].transmission[jj].Rx = prvTgfPortsArray[nextHopeIndex];
                    }
                }

            }
        }
    }

    if(MUST_USE_ORIG_PORTS_MAC(prvTgfDevNum))
    {
        GT_U32  nextHopeIndex = 3;
        nextHopPortNum = prvTgfPortsArray[nextHopeIndex];
    }
    else
    {
        nextHopPortNum = PRV_TGF_NEXTHOPE_PORT_NUM_CNS;
    }



    switch (testNum)
    {
    case 1:
        test = &TxRxTest1;
        break;
    case 2:
        test = &TxRxTest2;
        break;
    case 3:
        test = &TxRxTest3;
        break;
    default:
        UTF_VERIFY_EQUAL0_STRING_MAC(0, 1, "test number is incorrect.");
        return;
    }

    PRV_UTF_LOG0_MAC("======= Generating Traffic =======\n");


    /* -------------------------------------------------------------------------
     * 1. Setup counters and enable capturing
     */

    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* reset ethernet counters */
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        /* reset IP couters and set ROUTE_ENTRY mode */
        rc = prvTgfCountersIpSet(prvTgfPortsArray[portIter], portIter % 4);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfCountersIpSet: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* setup nexthope portInterface for capturing */
    portInterface.type            = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = prvTgfDevNum;
    portInterface.devPort.portNum = nextHopPortNum;

    /* enable capture on nexthope port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                 prvTgfDevNum, nextHopPortNum);


    /* -------------------------------------------------------------------------
     * 2. Generating Traffic
     */

    /* number of parts in packet */
    partsCount = sizeof(prvTgfPacketPartArray) / sizeof(prvTgfPacketPartArray[0]);

    /* calculate packet size */
    rc = prvTgfPacketSizeGet(prvTgfPacketPartArray, partsCount, &packetSize);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketSizeGet:");

    /* build packet */
    packetInfo.totalLen   = packetSize;
    packetInfo.numOfParts = partsCount;
    packetInfo.partsArray = prvTgfPacketPartArray;

    /* setup packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &packetInfo, prvTgfBurstCount, 0, NULL);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d", prvTgfDevNum);

    /* send packet */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, test->iteration[iteration].transmission[transmission].Tx);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, &d\n",
                                 prvTgfDevNum, test->iteration[iteration].transmission[transmission].Tx);

    /* disable capture on nexthope port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                 prvTgfDevNum, nextHopPortNum);


    /* -------------------------------------------------------------------------
     * 3. Get Ethernet Counters
     */
    /* read and check ethernet counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        GT_BOOL isOk;
        GT_U32  expectedPacketSize;

        utfGeneralStateMessageSave(2,"portIter = %d ,prvTgfPortsArray[portIter] = 0x%4.4x ",portIter,
            prvTgfPortsArray[portIter]);

        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d\n",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        /* check Tx and Rx counters */
        if (prvTgfPortsArray[portIter] == test->iteration[iteration].transmission[transmission].Tx)
        {
            /* packetSize is not changed */
            expectedCntrs.goodOctetsSent.l[0] = (packetSize + TGF_CRC_LEN_CNS) * prvTgfBurstCount;
            expectedCntrs.goodPktsSent.l[0]   = prvTgfBurstCount;
            expectedCntrs.ucPktsSent.l[0]     = prvTgfBurstCount;
            expectedCntrs.brdcPktsSent.l[0]   = 0;
            expectedCntrs.mcPktsSent.l[0]     = 0;
            expectedCntrs.goodOctetsRcv.l[0]  = (packetSize + TGF_CRC_LEN_CNS) * prvTgfBurstCount;
            expectedCntrs.goodPktsRcv.l[0]    = prvTgfBurstCount;
            expectedCntrs.ucPktsRcv.l[0]      = prvTgfBurstCount;
            expectedCntrs.brdcPktsRcv.l[0]    = 0;
            expectedCntrs.mcPktsRcv.l[0]      = 0;
        }
        else if (prvTgfPortsArray[portIter] == test->iteration[iteration].transmission[transmission].Rx)
        {
            /* packetSize is changed. The new packet will be without VLAN_TAG and IPV4_HEADER */
            expectedPacketSize = (packetSize - TGF_VLAN_TAG_SIZE_CNS - TGF_IPV4_HEADER_SIZE_CNS + TGF_CRC_LEN_CNS);

            expectedCntrs.goodOctetsSent.l[0] = expectedPacketSize * prvTgfBurstCount;
            expectedCntrs.goodPktsSent.l[0]   = prvTgfBurstCount;
            expectedCntrs.ucPktsSent.l[0]     = prvTgfBurstCount;
            expectedCntrs.brdcPktsSent.l[0]   = 0;
            expectedCntrs.mcPktsSent.l[0]     = 0;
            expectedCntrs.goodOctetsRcv.l[0]  = expectedPacketSize * prvTgfBurstCount;
            expectedCntrs.goodPktsRcv.l[0]    = prvTgfBurstCount;
            expectedCntrs.ucPktsRcv.l[0]      = prvTgfBurstCount;
            expectedCntrs.brdcPktsRcv.l[0]    = 0;
            expectedCntrs.mcPktsRcv.l[0]      = 0;
        }
        else
        {
            /* for other ports */
            expectedCntrs.goodOctetsSent.l[0] = 0;
            expectedCntrs.goodPktsSent.l[0]   = 0;
            expectedCntrs.ucPktsSent.l[0]     = 0;
            expectedCntrs.brdcPktsSent.l[0]   = 0;
            expectedCntrs.mcPktsSent.l[0]     = 0;
            expectedCntrs.goodOctetsRcv.l[0]  = 0;
            expectedCntrs.goodPktsRcv.l[0]    = 0;
            expectedCntrs.ucPktsRcv.l[0]      = 0;
            expectedCntrs.brdcPktsRcv.l[0]    = 0;
            expectedCntrs.mcPktsRcv.l[0]      = 0;
        }

        PRV_TGF_VERIFY_COUNTERS_MAC(isOk, expectedCntrs, portCntrs);

        UTF_VERIFY_EQUAL0_STRING_MAC(isOk, GT_TRUE, "get another counters values.");

        /* print expected values if not equal */
        PRV_TGF_PRINT_DIFF_COUNTERS_MAC(isOk, expectedCntrs, portCntrs);
    }

    /* -------------------------------------------------------------------------
     * 4. Get Trigger Counters
     */

    PRV_UTF_LOG1_MAC("Port [%d] capturing:\n", portInterface.devPort.portNum);

    /* get trigger counters where packet has MAC DA as prvTgfArpMac */
    vfdArray[0].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[0].modeExtraInfo = 0;
    vfdArray[0].offset = 0;
    vfdArray[0].cycleCount = sizeof(TGF_MAC_ADDR);
    cpssOsMemCpy(vfdArray[0].patternPtr, prvTgfArpMac, sizeof(TGF_MAC_ADDR));

    rc = tgfTrafficGeneratorPortTxEthTriggerCountersGet(&portInterface, 1, vfdArray, &numTriggers);
    PRV_UTF_LOG2_MAC("    numTriggers = %d, rc = 0x%02X\n\n", numTriggers, rc);

    /* check TriggerCounters */
    rc = rc == GT_NO_MORE ? GT_OK : rc;
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorPortTxEthTriggerCountersGet: %d, %d\n",
                                 portInterface.devPort.hwDevNum, portInterface.devPort.portNum);

    if (test->iteration[iteration].transmission[transmission].Rx != PRV_TGF_NON_EXISTING_PORT)
    {
        /* check if captured packet has the same MAC DA as prvTgfArpMac */
        UTF_VERIFY_EQUAL6_STRING_MAC(1, numTriggers,
            "\n   MAC DA of captured packet must be: %02X:%02X:%02X:%02X:%02X:%02X",
            prvTgfArpMac[0], prvTgfArpMac[1], prvTgfArpMac[2],
            prvTgfArpMac[3], prvTgfArpMac[4], prvTgfArpMac[5]);
    }

    /* -------------------------------------------------------------------------
     * 5. Get IP Counters
     */

    /* get and print ip counters values */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++) {
        PRV_UTF_LOG1_MAC("IP counters for Port [%d]:\n", prvTgfPortsArray[portIter]);
        prvTgfCountersIpGet(prvTgfDevNum, portIter % 4, GT_TRUE, &ipCounters);
    }
    PRV_UTF_LOG0_MAC("\n");
}


/**
* @internal prvTgfTunnelTermGlobalPort2LocalPortConfigurationRestore function
* @endinternal
*
* @brief   Restore configuration
*
* @note 3. Restore TTI Configuration
*       2. Restore Route Configuration
*       1. Restore Base Configuration
*
*/
GT_VOID prvTgfTunnelTermGlobalPort2LocalPortConfigurationRestore
(
    GT_VOID
)
{
    GT_U32      vlanIter  = 0;
    GT_U32      vlanCount = sizeof(prvTgfVlanArray);
    GT_STATUS   rc        = GT_OK;
    GT_IPADDR   ipAddr;
    GT_U8       portIter = 0;

    prvTgfTtiTcamIndexIsRelativeSet(GT_TRUE);

    PRV_UTF_LOG0_MAC("======= Restoring Configuration =======\n");

    /* -------------------------------------------------------------------------
     * 3. Restore TTI Configuration
     */

    /* invalidate TTI rule */
    rc = prvTgfTtiRuleValidStatusSet(prvTgfTtiRuleIndex, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleValidStatusSet: %d", prvTgfDevNum);


    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* disable the IPv4 TTI lookup for only tunneled packets received on port */
        rc = prvTgfTtiPortIpv4OnlyTunneledEnableSet(prvTgfPortsArray[portIter], GT_FALSE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPortIpv4OnlyTunneledEnableSet: %d", prvTgfDevNum);

        /* disable the TTI lookup for IPv4 at the port */
        rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[portIter], PRV_TGF_TTI_KEY_IPV4_E, GT_FALSE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet: %d", prvTgfDevNum);
    }

    /* restore the lookup Mac mode for IPv4 and DA */
    rc = prvTgfTtiMacModeSet(PRV_TGF_TTI_KEY_IPV4_E, prvTgfRestoreCfg.macMode);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiMacModeSet: %d", prvTgfDevNum);


    /* -------------------------------------------------------------------------
     * 2. Restore Route Configuration
     */

    /* fill the destination IP address for Ipv4 prefix in Virtual Router */
    cpssOsMemCpy(ipAddr.arIP, prvTgfPacketIpv4OverIpv4Part.dstAddr, sizeof(ipAddr.arIP));

    /* delete the Ipv4 prefix */
    rc = prvTgfIpLpmIpv4UcPrefixDel(0, 0, ipAddr, 32);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4UcPrefixDel: %d", prvTgfDevNum);

    /* disable Unicast IPv4 Routing on a Port */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfIpPortRoutingEnable(portIter, CPSS_IP_UNICAST_E,
                                       CPSS_IP_PROTOCOL_IPV4_E, GT_FALSE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpPortRoutingEnable: %d %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* disable IPv4 Unicast Routing on Vlan */
    rc = prvTgfIpVlanRoutingEnable(PRV_TGF_SEND_VLANID_CNS, CPSS_IP_UNICAST_E, CPSS_IP_PROTOCOL_IPV4_E, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpVlanRoutingEnable: %d %d",
                                 prvTgfDevNum, PRV_TGF_SEND_VLANID_CNS);

    /* -------------------------------------------------------------------------
     * 1. Restore Base Configuration
     */

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d", prvTgfDevNum, GT_TRUE);

    /* restore default vlanId */
    rc = prvTgfBrgVlanPvidSet(prvTgfDefVlanId);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPvidSet: %d, %d",
                                 prvTgfDevNum, prvTgfDefVlanId);

    /* restore default Precedence to Send port */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfBrgVlanPortVidPrecedenceSet(prvTgfDevNum,
                                               prvTgfPortsArray[portIter],
                                               CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidPrecedenceSet: %d, %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter],
                                     CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E);
    }

    /* delete VLANs */
    for (vlanIter = 0; vlanIter < vlanCount; vlanIter++)
    {
        /* invalidate vlan entry */
        rc = prvTgfBrgVlanEntryInvalidate(prvTgfVlanArray[vlanIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEntryInvalidate: %d, %d",
                                     prvTgfDevNum, prvTgfVlanArray[vlanIter]);
    }

    prvTgfTtiTcamIndexIsRelativeSet(GT_FALSE);
}


