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
* @file prvTgfBrgVplsBasicTest_1.c
*
* @brief similar to file prvTgfBrgVplsBasicTest.c but with next changes:
* 1. supports 3 modes :
* a. 'pop tag' - the mode that is tested in prvTgfBrgVplsBasicTest.c
* b. 'raw mode' + 'QinQ terminal'(delete double Vlan)
* c. 'tag mode' + 'add double vlan tag'
*
* 2. in RAW mode ethernet packets come with 2 tags and the passenger on MPLS
* tunnels is without vlan tags.
* 3. in TAG mode the ethernet packets come with one tag but considered untagged
* when become passenger on MPLS tunnels and so added additional 2 vlan tags.
*
* @version   3.
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/generic/cpssCommonDefs.h>
#include <cpss/generic/bridge/cpssGenBrgVlanTypes.h>
#include <cpss/generic/bridge/cpssGenBrgGen.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <utf/private/prvUtfHelpers.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfTunnelGen.h>
#include <bridge/prvTgfBrgVplsBasicTest.h>

#ifdef COW_INTERNAL_COMPILE
    /* those 3 are defined , so remove them */
    #undef EXMXPM_FAMILY
    #undef EX_FAMILY
    #undef CHX_FAMILY

    /* define only the tested one */
    #define CHX_FAMILY

#endif /*COW_INTERNAL_COMPILE*/

#ifndef CHX_FAMILY
GT_VOID prvTgfBrgVplsBasicTest1
(
    GT_VOID
)
{
    /* not implemented for non CHX_FAMILY */
    return ;
}
#else /*CHX_FAMILY*/

/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/
/* VPLS DOMAIN : VLAN Id */
#define PRV_TGF_VLANID_CNS  100

/* default number of packets to send */
static GT_U32  prvTgfBurstCount   = 1;

static GT_U32   errorLine = 0;
/******************************* Test packet **********************************/

/* L2 part of packet - port 0 */
static TGF_PACKET_L2_STC prvTgfPacketL2Part_port0 = {
    {0x00, 0x00, 0x99, 0x99, 0x99, 0x99},               /* daMac */
    {0x00, 0x00, 0x11, 0x11, 0x11, 0x11}                /* saMac */
};
/* L2 part of packet - port 1 */
static TGF_PACKET_L2_STC prvTgfPacketL2Part_port1 = {
    {0x00, 0x00, 0x99, 0x99, 0x99, 0x99},               /* daMac */
    {0x00, 0x00, 0x22, 0x22, 0x22, 0x22}                /* saMac */
};
/* L2 part of packet - port 2,3 */
static TGF_PACKET_L2_STC prvTgfPacketL2Part_dummyTs = { /*--> filled in runtime */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00},               /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}                /* saMac */
};

/* DATA of packet */
static GT_U8 prvTgfPayloadDataArr[] = {
    0x12,0x12,0x12,0x12,0x12,0x12,0x12,0x12,0x12,0x12,0x12,0x12,0x12,0x12,0x12,0x12,
    0x34,0x34,0x34,0x34,0x34,0x34,0x34,0x34,0x34,0x34,0x34,0x34,0x34,0x34,0x34,0x34,
    0x56,0x56,0x56,0x56,0x56,0x56,0x56,0x56,0x56,0x56,0x56,0x56,0x56,0x56,0x56,0x56,
    0x78,0x78,0x78,0x78,0x78,0x78,0x78,0x78,0x78,0x78,0x78,0x78,0x78,0x78,0x78,0x78,
    0x9a,0x9a,0x9a,0x9a,0x9a,0x9a,0x9a,0x9a,0x9a,0x9a,0x9a,0x9a,0x9a,0x9a,0x9a,0x9a,
    0xbc,0xbc,0xbc,0xbc,0xbc,0xbc,0xbc,0xbc,0xbc,0xbc,0xbc,0xbc,0xbc,0xbc,0xbc,0xbc
};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart = {
    sizeof(prvTgfPayloadDataArr),                       /* dataLength */
    prvTgfPayloadDataArr                                /* dataPtr */
};

/* VLAN_TAG part - serviceProvider_port0 */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTagPart_serviceProvider_port0 =
{
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS, /* etherType */
    0,  0,  0       /* pri, cfi, VlanId --> filled in runtime */
};
/* VLAN_TAG part - serviceProvider_port1 */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTagPart_serviceProvider_port1 =
{
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS, /* etherType */
    0,  0,  0       /* pri, cfi, VlanId --> filled in runtime */
};
/* VLAN_TAG part - serviceProvider_port0 */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTagPart_costomer =
{
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS, /* etherType */
    0,  0,  0       /* pri, cfi, VlanId --> filled in runtime */
};
/* VLAN_TAG part - mplsTagMode_tag0_port2 */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTagPart_mplsTagMode_tag0_port2 =
{
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS, /* etherType */
    0,  0,  0       /* pri, cfi, VlanId --> filled in runtime */
};
/* VLAN_TAG part - mplsTagMode_tag1_port2 */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTagPart_mplsTagMode_tag1_port2 =
{
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS, /* etherType */
    0,  0,  0       /* pri, cfi, VlanId --> filled in runtime */
};
/* VLAN_TAG part - mplsTagMode_tag0_port3 */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTagPart_mplsTagMode_tag0_port3 =
{
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS, /* etherType */
    0,  0,  0       /* pri, cfi, VlanId --> filled in runtime */
};
/* VLAN_TAG part - mplsTagMode_tag1_port3 */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTagPart_mplsTagMode_tag1_port3 =
{
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS, /* etherType */
    0,  0,  0       /* pri, cfi, VlanId --> filled in runtime */
};

/* PARTS of packet from port 0 */
static TGF_PACKET_PART_STC prvTgfPacketPartArray_port0[] =
{
    {TGF_PACKET_PART_L2_E,       &prvTgfPacketL2Part_port0},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E, &prvTgfPacketVlanTagPart_serviceProvider_port0},
    {TGF_PACKET_PART_VLAN_TAG_E, &prvTgfPacketVlanTagPart_costomer},
    {TGF_PACKET_PART_PAYLOAD_E,  &prvTgfPacketPayloadPart}
};

/* PARTS of packet from port 1 */
static TGF_PACKET_PART_STC prvTgfPacketPartArray_port1[] =
{
    {TGF_PACKET_PART_L2_E,       &prvTgfPacketL2Part_port1},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E, &prvTgfPacketVlanTagPart_serviceProvider_port1},
    {TGF_PACKET_PART_VLAN_TAG_E, &prvTgfPacketVlanTagPart_costomer},
    {TGF_PACKET_PART_PAYLOAD_E,  &prvTgfPacketPayloadPart}
};

/* packet's MPLS ethertype */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketMplsEtherTypePart = {TGF_ETHERTYPE_8847_MPLS_TAG_CNS};
static TGF_PACKET_MPLS_STC prvTgfPacketMplsLabelPart_mplsDomain =
{
    0,/*filled in runtime*/ /* label */
    1,                  /* exp */
    0,                  /* stack */
    0xff                /* timeToLive */
};
static TGF_PACKET_MPLS_STC prvTgfPacketMplsLabelPart_pwPort2 =
{
    0,/*filled in runtime*/ /* label */
    1,                  /* exp */
    0,                  /* stack */
    0xff                /* timeToLive */
};
static TGF_PACKET_MPLS_STC prvTgfPacketMplsLabelPart_pwPort3 =
{
    0,/*filled in runtime*/ /* label */
    1,                  /* exp */
    0,                  /* stack */
    0xff                /* timeToLive */
};

/* PARTS of packet from port 2 */
static TGF_PACKET_PART_STC prvTgfPacketPartArray_port2[] =
{
    /* the tunnel (runtime will set MAC SA,DA) */
/*0*/    {TGF_PACKET_PART_L2_E,       &prvTgfPacketL2Part_dummyTs},  /* type, partPtr */
/*1*/    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketMplsEtherTypePart},
/*2*/    {TGF_PACKET_PART_MPLS_E,      &prvTgfPacketMplsLabelPart_mplsDomain},
/*3*/    {TGF_PACKET_PART_MPLS_E,      &prvTgfPacketMplsLabelPart_pwPort2},

       /* the passenger (runtime will set full passenger -- depend on current system to check) */
/*4-->PASSENGER_INDEX_CNS*/
       /*L2*/          {TGF_PACKET_PART_L2_E,       NULL/*filled in runtime*/},  /* type, partPtr */
/*5*/  /*tag #1*/      {TGF_PACKET_PART_SKIP_E,     NULL},/*place holder*/  /*filled in runtime*/
/*6*/  /*tag #2*/      {TGF_PACKET_PART_SKIP_E,     NULL},/*place holder*/  /*filled in runtime*/
/*7*/  /*tag #3*/      {TGF_PACKET_PART_SKIP_E,     NULL},/*place holder*/  /*filled in runtime*/
/*8*/  /*tag #4*/      {TGF_PACKET_PART_SKIP_E,     NULL},/*place holder*/  /*filled in runtime*/
/*9*/  /*payload*/     {TGF_PACKET_PART_SKIP_E,     NULL},/*place holder*/  /*filled in runtime*/
/*10*/ /*dummy*/       {TGF_PACKET_PART_SKIP_E,     NULL}
};

/* PARTS of packet from port 3 */
static TGF_PACKET_PART_STC prvTgfPacketPartArray_port3[] =
{
    /* the tunnel (runtime will set MAC SA,DA) */
/*0*/    {TGF_PACKET_PART_L2_E,       &prvTgfPacketL2Part_dummyTs},  /* type, partPtr */
/*1*/    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketMplsEtherTypePart},
/*2*/    {TGF_PACKET_PART_MPLS_E,      &prvTgfPacketMplsLabelPart_mplsDomain},
/*3*/    {TGF_PACKET_PART_MPLS_E,      &prvTgfPacketMplsLabelPart_pwPort3},

       /* the passenger (runtime will set full passenger -- depend on current system to check) */
/*4-->PASSENGER_INDEX_CNS*/
       /*L2*/          {TGF_PACKET_PART_L2_E,       NULL/*filled in runtime*/},  /* type, partPtr */
/*5*/  /*tag #1*/      {TGF_PACKET_PART_SKIP_E,     NULL},/*place holder*/  /*filled in runtime*/
/*6*/  /*tag #2*/      {TGF_PACKET_PART_SKIP_E,     NULL},/*place holder*/  /*filled in runtime*/
/*7*/  /*tag #3*/      {TGF_PACKET_PART_SKIP_E,     NULL},/*place holder*/  /*filled in runtime*/
/*8*/  /*tag #4*/      {TGF_PACKET_PART_SKIP_E,     NULL},/*place holder*/  /*filled in runtime*/
/*9*/  /*payload*/     {TGF_PACKET_PART_SKIP_E,     NULL},/*place holder*/  /*filled in runtime*/
/*10*/ /*dummy*/       {TGF_PACKET_PART_SKIP_E,     NULL} /*place holder*/  /*filled in runtime*/
};

/* LENGTH of untagged packet */
#define PRV_TGF_PACKET_UNTAG_LEN_CNS TGF_L2_HEADER_SIZE_CNS + sizeof(prvTgfPayloadDataArr)

/* LENGTH of untagged packet with CRC */
#define PRV_TGF_PACKET_UNTAG_CRC_LEN_CNS  PRV_TGF_PACKET_UNTAG_LEN_CNS + TGF_CRC_LEN_CNS

#define prvTgfPacket_PortX_MAC(portX)                    \
    /* packet sent from port x */                       \
    TGF_PACKET_STC prvTgfPacket_Port##portX =        \
    {                                                   \
        TGF_PACKET_AUTO_CALC_LENGTH_CNS,                                /* totalLen */         \
        sizeof(prvTgfPacketPartArray_port##portX) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */  \
        prvTgfPacketPartArray_port##portX                                        /* partsArray */  \
    }
/* define packets for port 0,1,2,3 */
static prvTgfPacket_PortX_MAC(0);
static prvTgfPacket_PortX_MAC(1);
static prvTgfPacket_PortX_MAC(2);
static prvTgfPacket_PortX_MAC(3);

/* index 1 (out of 0..3) is sender of service provider */
#define SERVICE_PROVIDER_PORT_SENDER_INDEX_CNS   1
/* index 2 (out of 0..3) is sender of vpls domain */
#define VPLS_DOMAIN_PORT_SENDER_INDEX_2_CNS        2
/* index 2 (out of 0..3) is sender of vpls domain */
#define VPLS_DOMAIN_PORT_SENDER_INDEX_3_CNS        3

#define NO_PASSENGER_INDEX_CNS    0
#define PASSENGER_INDEX_CNS       4
/* value is : index in the array of 'parts' of packets .

ii that used for sendPacketsWithPassengerIndexArr[ii] is 'port index' (0..3)
*/
static GT_U32   sendPacketsWithPassengerIndexArr[4] =
    {NO_PASSENGER_INDEX_CNS,
     NO_PASSENGER_INDEX_CNS,
     PASSENGER_INDEX_CNS,
     PASSENGER_INDEX_CNS};

static TGF_PACKET_STC* sendPacketsArr[4] = {
    &prvTgfPacket_Port0,
    &prvTgfPacket_Port1,
    &prvTgfPacket_Port2,
    &prvTgfPacket_Port3};

static TGF_PACKET_L2_STC newPartL2PassengerArr[4];


/* debug tools */
static GT_U32   debug_on = 0;
static GT_U32   debug_numIterationsOnFlood;    /* for part 1,2 - number of iterations of captures */
static GT_U32   debug_doFloodFromServicePort;  /* part 1 */
static GT_U32   debug_doFloodFromVplsDomain;   /* part 2 */
static GT_U32   debug_doUcFromServicePort;     /* part 3 */
static GT_U32   debug_doUcFromVplsDomain;      /* part 4 */
static GT_U32   debug_vplsSystemIndex_Enabled = GT_FALSE;
static GT_U32   debug_vplsSystemIndex;

/******************************************************************************\
 *                            Private variables                               *
\******************************************************************************/

/******************************************************************************\
 *                            Private test functions                          *
\******************************************************************************/

/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

#include <cpss/dxCh/dxChxGen/l2mll/cpssDxChL2Mll.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgE2Phy.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgVlan.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgFdb.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/cpssDxChHwInit.h>
#include <cpss/dxCh/dxChxGen/ip/cpssDxChIpCtrl.h>
#include <cpss/dxCh/dxChxGen/tti/cpssDxChTti.h>

#define CHECK_RC_MAC(rc)                                                \
    {                                                                   \
        /*print the command(before execution) */                        \
            /*cpssOsPrintf("[%s]\n", */                                 \
            /*    #rc);   */                                            \
                                                                        \
        {                                                               \
            /* execute the command */                                   \
            GT_STATUS   __rc = (rc);                                    \
            if(__rc != GT_OK)                                           \
            {                                                           \
                cpssOsPrintf(" --> Failed in file(%s) LINE(%d) error code(%d) \n", \
                    __FILE__,errorLine = __LINE__,__rc);                           \
                return __rc;                                            \
            }                                                           \
        }                                                               \
    }

static GT_U8    swDevNum = 0;
static GT_HW_DEV_NUM hwDevNum = 0;
static GT_U32   numOfPorts = 4;
static GT_U32   tunnelStartPointerArr[4] = {0,0,0x55/*for eport C*/,0x66/*for ePort D*/};
static GT_ETHERADDR tsMacDaArr[4] =
{
    {{0}},
    {{0}},
    {{0,0,0x55,0x55,0x55,0x55}},
    {{0,0,0x66,0x66,0x66,0x66}}
};
static GT_ETHERADDR tsMacSaArr[4] =
{
    {{0}},
    {{0}},
    {{0,0,0xAA,0xf2,0xdd,0x02}},
    {{0,0,0xBB,0xf3,0xee,0x03}}
};
static GT_U32   pwLabels[4] = {0,0,0x60/* eport C*/,0x70 /*ePort D*/   };
static GT_U32   pwExp[4] = {0,0,5,6};
static GT_U32   pwTtl[4] = {0,0,0x22,0x33};
static GT_U32   mplsLabelArr[4]={0,0,0x50,0x50};

/* those indexes must be divided by 3 without leftovers */
/* TTI rules should be inside first TCAM block to support devices with part of floor per TTI hit like Aldrin.
   TTI lookup 0 is in first part of TCAM floor.
   The index is relative in the floor of TTI client.
   Absolute index is created in run time by adding TTI floor base address. */
#define TTI_INDEX_MAC(_row_in_floor) (12 * (_row_in_floor))

static GT_U32   tunnelTerminationRelativeIndex[4] = {TTI_INDEX_MAC(10),TTI_INDEX_MAC(70),TTI_INDEX_MAC(130)/*for eport C*/,TTI_INDEX_MAC(190)/*for ePort D*/};
static GT_U32   tunnelTerminationIndex[4];

static GT_U16   globalEvlan = 0;
static GT_U16   globalFloodEvidx = 0;

static GT_U16   outerTag0Arr[4] = {0x10,0x20,0x444,0x555};/* the vid 0 of providers ports for egress tagging*/
static GT_U16   outerTag1Arr[4] = {0x30,0x30,0x30,0x30};/* the vid 1 of customers on providers ports for  tti rule*/

static GT_U32   globalStartMllIndex = 0x234;/*index on first entry -- take even index (not odd) */

static GT_U32   meshIdArr[4] = {0,0,15,15};
static GT_U8    timeToLive = 0x40;
static GT_U8    expTs = 7;
/*
    VPLS types of modes :
    VPLS_DOMAIN_MODE_ETH_POP1_E - the ethernet ports need to 'pop' the providers
                                tags and only 'customer' tag will go to the MPLS ports.
                                The passenger on the MPLS is with C tag (tag 1)
    VPLS_DOMAIN_RAW_MODE_E - the passenger on the MPLS is with OUT vlan tags (no tag 0 no tag 1)
    VPLS_DOMAIN_TAG_MODE_E - the passenger on the MPLS is with 2 vlan tags (tag 0 and tag 1)
                                note that in this case the ethernet packet that
                                will ingress with vlan tags will be considered
                                as untagged , so it will be added 2 tags on egress

*/
typedef enum{
    VPLS_DOMAIN_MODE_ETH_POP1_E,
    VPLS_DOMAIN_RAW_MODE_E,
    VPLS_DOMAIN_TAG_MODE_E,

    VPLS_DOMAIN_LAST
}VPLS_DOMAIN_MODE_ENT;

#define STR(strname)    \
    #strname

static char* vplsDomainModeNames[VPLS_DOMAIN_LAST]=
{
    STR(VPLS_DOMAIN_MODE_ETH_POP1_E),
    STR(VPLS_DOMAIN_RAW_MODE_E),
    STR(VPLS_DOMAIN_TAG_MODE_E)
};

static char* vplsDomainModeDescriptions[VPLS_DOMAIN_LAST]=
{
    /*VPLS_DOMAIN_MODE_ETH_POP1_E*/
    "the ethernet ports need to 'pop' the providers \n"
    "tags and only 'customer' tag will go to the MPLS ports.\n"
    "The passenger on the MPLS is with C tag (tag 1))"
    ,
    /*VPLS_DOMAIN_RAW_MODE_E*/
    "the passenger on the MPLS is with OUT vlan tags (no tag 0 no tag 1)"
    ,
    /*VPLS_DOMAIN_TAG_MODE_E*/
    "the passenger on the MPLS is with 2 vlan tags (tag 0 and tag 1) \n"
    "note that in this case the ethernet packet that \n"
    "will ingress with vlan tags will be considered  \n"
    "as untagged , so it will be added 2 tags on egress"
};

static VPLS_DOMAIN_MODE_ENT   vplsSystemIndex;
static GT_U32                 currentNumOfTags=2;

static GT_PHYSICAL_PORT_NUM  test_phyPortArr[4];/* physical port associated with ePort_A..D */
/* eports*/
static GT_PORT_NUM  test_ePortArr[4];/* ePorts corresponds to prvTgfPortsArray */

/* get eport that represents the port for number of ingress tags */
static GT_PORT_NUM  getEportForPhyPort
(
    IN GT_U32   portIndex
)
{
    return test_ePortArr[portIndex] + vplsSystemIndex + ((2 - currentNumOfTags)*VPLS_DOMAIN_LAST);
}

/* get tti index that represents the port for number of ingress tags */
static GT_U32  getTtiIndexForPhyPort
(
    IN GT_U32   portIndex
)
{
    return tunnelTerminationIndex[portIndex] + TTI_INDEX_MAC(((2 - currentNumOfTags)*VPLS_DOMAIN_LAST)  + vplsSystemIndex);
}

/* get eVlan that represents the VPLS domain */
static GT_U16  getEvlanForVplsDomain(void)
{
    return (GT_U16)(globalEvlan + vplsSystemIndex);
}

/* get eVidx that represents the VPLS domain */
static GT_U16  getEvidxForVplsDomain(void)
{
    return (GT_U16)(globalFloodEvidx + vplsSystemIndex);
}

static GT_U32   getTsMplsLabel
(
    IN GT_U32   portIndex
)
{
    return mplsLabelArr[portIndex] + vplsSystemIndex;
}

static GT_U32   setStartMllIndex(void)
{
    return globalStartMllIndex + 8 * vplsSystemIndex;
}

static GT_U16 getTag0ForPhyPort
(
    IN GT_U32   portIndex
)
{
    return (GT_U16)(outerTag0Arr[portIndex] + 0x10 * vplsSystemIndex);
}

static GT_U16 getTag1ForPhyPort
(
    IN GT_U32   portIndex
)
{
    return (GT_U16)(outerTag1Arr[portIndex] + 0x10 * vplsSystemIndex);
}


static GT_U32 getTsIndexForPhyPort
(
    IN GT_U32   portIndex
)
{
    return tunnelStartPointerArr[portIndex] + vplsSystemIndex;
}


/* set e2phy */
static GT_STATUS setE2Phy(void)
{
    GT_U32  ii;
    CPSS_INTERFACE_INFO_STC phyPortInfo;

    phyPortInfo.type = CPSS_INTERFACE_PORT_E;
    phyPortInfo.devPort.hwDevNum = hwDevNum;

    for(ii = 0 ; ii < numOfPorts ; ii++)
    {
        /* AUTODOC: bind the eport to the needed physical port */
        phyPortInfo.devPort.portNum = test_phyPortArr[ii];

        CHECK_RC_MAC(
        cpssDxChBrgEportToPhysicalPortTargetMappingTableSet(swDevNum,
            getEportForPhyPort(ii),&phyPortInfo));
    }

    return GT_OK;
}

#define SAVE_TPID_PROFILE_SIZE_CNS 6*2
static GT_U32 saveTpidProfileArr[SAVE_TPID_PROFILE_SIZE_CNS];
static GT_U32 prvVplsIsTpidSaveDone = 0;

/* set tpid */
/* we not change the 0x8100 on entries */
static GT_STATUS setTpid(void)
{
    GT_U32  tpidBmp,profileAll,profileTag1Only,profileNoTags;
    GT_U32  ii;
    GT_U32  currentEportProfile;
    GT_U32  saveTpidProfileIndex = 0;

    /* AUTODOC: we not change the 0x8100 on entries */
    profileAll = 2;
    /* AUTODOC: make profile 2 recognize tag0 on all TPIDs */
    tpidBmp = 0xFF;
    CHECK_RC_MAC(
        cpssDxChBrgVlanIngressTpidProfileSet(swDevNum,profileAll,CPSS_VLAN_ETHERTYPE0_E,tpidBmp));
    /* AUTODOC: make profile 2 recognize tag1 on all TPIDs */
    tpidBmp = 0xFF;
    CHECK_RC_MAC(
    cpssDxChBrgVlanIngressTpidProfileSet(swDevNum,profileAll,CPSS_VLAN_ETHERTYPE1_E,tpidBmp));

    /* AUTODOC: bind physical port A,B,C,D profile 2 ... to recognize tag 0(provider),1(customer) */
    for(ii = 0 ; ii < numOfPorts ; ii++)
    {
        if(prvVplsIsTpidSaveDone == 0)
        {
            CHECK_RC_MAC(((SAVE_TPID_PROFILE_SIZE_CNS - saveTpidProfileIndex) == 0));
            CHECK_RC_MAC(
            cpssDxChBrgVlanPortIngressTpidProfileGet(swDevNum,
                test_phyPortArr[ii],
                CPSS_VLAN_ETHERTYPE0_E,
                GT_TRUE/*isDefaultProfile*/,
                &saveTpidProfileArr[saveTpidProfileIndex++]));
        }

        CHECK_RC_MAC(
        cpssDxChBrgVlanPortIngressTpidProfileSet(swDevNum,
            test_phyPortArr[ii],
            CPSS_VLAN_ETHERTYPE0_E,
            GT_TRUE/*isDefaultProfile*/,
            profileAll));

        if(prvVplsIsTpidSaveDone == 0)
        {
            CHECK_RC_MAC(((SAVE_TPID_PROFILE_SIZE_CNS - saveTpidProfileIndex) == 0));
            CHECK_RC_MAC(
            cpssDxChBrgVlanPortIngressTpidProfileGet(swDevNum,
                test_phyPortArr[ii],
                CPSS_VLAN_ETHERTYPE1_E,
                GT_TRUE/*isDefaultProfile*/,
                &saveTpidProfileArr[saveTpidProfileIndex++]));
        }

        CHECK_RC_MAC(
        cpssDxChBrgVlanPortIngressTpidProfileSet(swDevNum,
            test_phyPortArr[ii],
            CPSS_VLAN_ETHERTYPE1_E,
            GT_TRUE/*isDefaultProfile*/,
            profileAll));
    }

    profileTag1Only = 3;
    /* AUTODOC: make profile 3 not recognize tag 0 , we need then to recognize tag 1 only */
    tpidBmp = 0;
    CHECK_RC_MAC(
    cpssDxChBrgVlanIngressTpidProfileSet(swDevNum,profileTag1Only,CPSS_VLAN_ETHERTYPE0_E,tpidBmp));
    /* AUTODOC: make profile 3 recognize tag1 on all TPIDs */
    tpidBmp = 0xFF;
    CHECK_RC_MAC(
    cpssDxChBrgVlanIngressTpidProfileSet(swDevNum,profileTag1Only,CPSS_VLAN_ETHERTYPE1_E,tpidBmp));

    profileNoTags = 4;
    /* AUTODOC: make profile 4 not recognize tag 0 */
    tpidBmp = 0;
    CHECK_RC_MAC(
    cpssDxChBrgVlanIngressTpidProfileSet(swDevNum,profileNoTags,CPSS_VLAN_ETHERTYPE0_E,tpidBmp));
    /* AUTODOC: make profile 4 not recognize tag 1 */
    tpidBmp = 0;
    CHECK_RC_MAC(
    cpssDxChBrgVlanIngressTpidProfileSet(swDevNum,profileNoTags,CPSS_VLAN_ETHERTYPE1_E,tpidBmp));

    if(vplsSystemIndex == VPLS_DOMAIN_MODE_ETH_POP1_E)
    {
        /* AUTODOC: bind eports C,D profile 3 ... to not recognize tag 0 , we need then to recognize tag 1 only */
        currentEportProfile = profileTag1Only;
    }
    else
    if(vplsSystemIndex == VPLS_DOMAIN_RAW_MODE_E)
    {
        /* AUTODOC: bind eports C,D profile 4 ... to not recognize tag 0 , and not recognize tag 1 */
        currentEportProfile = profileNoTags;
    }
    else
    {
        /* AUTODOC: bind eports C,D profile 2 ... to recognize tag 0 and tag 1 */
        currentEportProfile = profileAll;
    }

    for(ii = 2/*eport C*/ ; ii < numOfPorts ; ii++)
    {
        if(prvVplsIsTpidSaveDone == 0)
        {
            CHECK_RC_MAC(((SAVE_TPID_PROFILE_SIZE_CNS - saveTpidProfileIndex) == 0));
            CHECK_RC_MAC(
            cpssDxChBrgVlanPortIngressTpidProfileGet(swDevNum,
                getEportForPhyPort(ii),
                CPSS_VLAN_ETHERTYPE0_E,
                GT_FALSE/*isDefaultProfile*/,
                &saveTpidProfileArr[saveTpidProfileIndex++]));
        }

        CHECK_RC_MAC(
        cpssDxChBrgVlanPortIngressTpidProfileSet(swDevNum,
            getEportForPhyPort(ii),
            CPSS_VLAN_ETHERTYPE0_E,
            GT_FALSE/*isDefaultProfile*/,
            currentEportProfile));

        if(prvVplsIsTpidSaveDone == 0)
        {
            CHECK_RC_MAC(((SAVE_TPID_PROFILE_SIZE_CNS - saveTpidProfileIndex) == 0));
            CHECK_RC_MAC(
            cpssDxChBrgVlanPortIngressTpidProfileGet(swDevNum,
                getEportForPhyPort(ii),
                CPSS_VLAN_ETHERTYPE1_E,
                GT_FALSE/*isDefaultProfile*/,
                &saveTpidProfileArr[saveTpidProfileIndex++]));
        }

        CHECK_RC_MAC(
        cpssDxChBrgVlanPortIngressTpidProfileSet(swDevNum,
            getEportForPhyPort(ii),
            CPSS_VLAN_ETHERTYPE1_E,
            GT_FALSE/*isDefaultProfile*/,
            currentEportProfile));
    }

    prvVplsIsTpidSaveDone++;

    return GT_OK;
}

/* set egress tagging */
static GT_STATUS setEgressTagging(void)
{
    GT_U32  ii;
    CPSS_DXCH_BRG_VLAN_EPORT_TAG_STATE_MODE_ENT currentTagMode;
    GT_PORT_NUM eport;

    /* AUTODOC: eport A,B - tag0,tag1 */
    for(ii = 0/*eport A*/ ; ii < 2/*eportB inclusive*/ ; ii++)
    {
        eport = getEportForPhyPort(ii);
        if(vplsSystemIndex == VPLS_DOMAIN_MODE_ETH_POP1_E)
        {
            currentTagMode = CPSS_DXCH_BRG_VLAN_PORT_OUTER_TAG0_INNER_TAG1_CMD_E;
        }
        else /* raw mode - egress with 2 tags on ethernet ports */
        if(vplsSystemIndex == VPLS_DOMAIN_RAW_MODE_E)
        {
            currentTagMode = CPSS_DXCH_BRG_VLAN_PORT_OUTER_TAG0_INNER_TAG1_CMD_E;
        }
        else /* tag mode - egress with 'no tags' on ethernet ports */
        {
            currentTagMode = CPSS_DXCH_BRG_VLAN_PORT_UNTAGGED_CMD_E;
        }

        /* AUTODOC: use tagging from eport not from vlan */
        CHECK_RC_MAC(
        cpssDxChBrgVlanEgressPortTagStateModeSet(swDevNum,
            eport,
            CPSS_DXCH_BRG_VLAN_EPORT_TAG_STATE_EPORT_MODE_E));
        /* AUTODOC: egress tag0,tag1*/
        CHECK_RC_MAC(
        cpssDxChBrgVlanEgressPortTagStateSet(swDevNum,
            eport,
            currentTagMode));

        /* AUTODOC: force to use tag0 from eport not from ingress pipe */
        CHECK_RC_MAC(
        cpssDxChBrgVlanPortVid0CommandEnableSet(swDevNum,
            eport,
            GT_TRUE));

        /* AUTODOC: set value for vid 0 for egress*/
        CHECK_RC_MAC(
        cpssDxChBrgVlanPortVidSet(swDevNum,
            eport,
            CPSS_DIRECTION_EGRESS_E,
            getTag0ForPhyPort(ii)));

        if(vplsSystemIndex == VPLS_DOMAIN_RAW_MODE_E)
        {
            /* in RAW mode need to set the VID1 too per egress eport */

            /* AUTODOC: force to use tag1 from eport not from ingress pipe */
            CHECK_RC_MAC(
            cpssDxChBrgVlanPortVid1CommandEnableSet(swDevNum,
                eport,
                GT_TRUE));

            /* AUTODOC: set value for vid 1 for egress*/
            CHECK_RC_MAC(
            cpssDxChBrgVlanPortVid1Set(swDevNum,
                eport,
                CPSS_DIRECTION_EGRESS_E,
                getTag1ForPhyPort(ii)));
        }
    }

    /* AUTODOC: eport C,D - tag1 only*/
    for(/*continue*/; ii < numOfPorts ; ii++)
    {
        eport = getEportForPhyPort(ii);
        if(vplsSystemIndex == VPLS_DOMAIN_MODE_ETH_POP1_E)
        {
            currentTagMode = CPSS_DXCH_BRG_VLAN_PORT_TAG1_CMD_E;
        }
        else /* raw mode - egress with NO tags on MPLS ports */
        if(vplsSystemIndex == VPLS_DOMAIN_RAW_MODE_E)
        {
            currentTagMode = CPSS_DXCH_BRG_VLAN_PORT_UNTAGGED_CMD_E;
        }
        else /* tag mode - egress with 2 tags on ethernet ports */
        {
            currentTagMode = CPSS_DXCH_BRG_VLAN_PORT_OUTER_TAG0_INNER_TAG1_CMD_E;
        }

        /* AUTODOC: use tagging from eport not from vlan */
        CHECK_RC_MAC(
        cpssDxChBrgVlanEgressPortTagStateModeSet(swDevNum,
            eport,
            CPSS_DXCH_BRG_VLAN_EPORT_TAG_STATE_EPORT_MODE_E));
        /* AUTODOC: egress tag1*/
        CHECK_RC_MAC(
        cpssDxChBrgVlanEgressPortTagStateSet(swDevNum,
            eport,
            currentTagMode));

        if(vplsSystemIndex == VPLS_DOMAIN_TAG_MODE_E)
        {
            /* in TAG mode need to set the VID0,VID1 per egress eport */

            /* AUTODOC: force to use tag0 from eport not from ingress pipe */
            CHECK_RC_MAC(
            cpssDxChBrgVlanPortVid0CommandEnableSet(swDevNum,
                eport,
                GT_TRUE));

            /* AUTODOC: set value for vid 0 for egress*/
            CHECK_RC_MAC(
            cpssDxChBrgVlanPortVidSet(swDevNum,
                eport,
                CPSS_DIRECTION_EGRESS_E,
                getTag0ForPhyPort(ii)));

            /* AUTODOC: force to use tag1 from eport not from ingress pipe */
            CHECK_RC_MAC(
            cpssDxChBrgVlanPortVid1CommandEnableSet(swDevNum,
                eport,
                GT_TRUE));

            /* AUTODOC: set value for vid 1 for egress*/
            CHECK_RC_MAC(
            cpssDxChBrgVlanPortVid1Set(swDevNum,
                eport,
                CPSS_DIRECTION_EGRESS_E,
                getTag1ForPhyPort(ii)));
        }
    }

    return GT_OK;
}

/* set egress VPLS tunnel */
static GT_STATUS setEgressVplsTunnel(void)
{
    GT_U32  ii;
    CPSS_DXCH_BRG_EGRESS_PORT_INFO_STC  egressEportInfo;
    CPSS_DXCH_TUNNEL_START_CONFIG_UNT   tsEntry;
    CPSS_DXCH_TUNNEL_START_MPLS_CONFIG_STC *mplsTsEntryPtr;
    GT_U32  eport;

    cpssOsMemSet(&egressEportInfo,0,sizeof(egressEportInfo));
    egressEportInfo.tunnelStart = GT_TRUE;
    egressEportInfo.tsPassengerPacketType = CPSS_DXCH_TUNNEL_PASSENGER_ETHERNET_E;

    cpssOsMemSet(&tsEntry,0,sizeof(tsEntry));
    mplsTsEntryPtr = &tsEntry.mplsCfg;
    /* AUTODOC: define the TS entries for eports C,D */
    mplsTsEntryPtr->tagEnable = GT_FALSE;/* AUTODOC: don't need vlan tag at the TS header */
    mplsTsEntryPtr->vlanId = 0;
    mplsTsEntryPtr->upMarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;
    mplsTsEntryPtr->up = 0;
    mplsTsEntryPtr->numLabels = 1;/* AUTODOC: 1 label by the TS ... the eport will add another PW label */
    mplsTsEntryPtr->ttl = timeToLive;
    mplsTsEntryPtr->ttlMode = CPSS_DXCH_TUNNEL_START_TTL_TO_INCOMING_TTL_E;
    mplsTsEntryPtr->exp1MarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;
    mplsTsEntryPtr->exp1 = expTs;
    mplsTsEntryPtr->retainCRC = GT_FALSE;/*AUTODOC: the passenger is without CRC (CRC only for full packet) */
    mplsTsEntryPtr->cfi = 0;
    mplsTsEntryPtr->mplsEthertypeSelect = CPSS_DXCH_TUNNEL_START_MPLS_ETHER_TYPE_UC_E;

    /* AUTODOC: eport C,D - need tunnel start and PW label*/
    for(ii = 2/*eport C*/ ; ii < numOfPorts ; ii++)
    {
        eport = getEportForPhyPort(ii);
        /* AUTODOC: enable the PW label */
        CHECK_RC_MAC(
        cpssDxChTunnelStartMplsPwLabelPushEnableSet(swDevNum,eport,GT_TRUE));

        /* AUTODOC: set the PW label */
        CHECK_RC_MAC(
        cpssDxChTunnelStartMplsPwLabelSet(swDevNum,eport,pwLabels[ii]));
        /* AUTODOC: set the PW label EXP */
        CHECK_RC_MAC(
        cpssDxChTunnelStartMplsPwLabelExpSet(swDevNum,eport,pwExp[ii]));

        /* AUTODOC: set the PW label TTL */
        CHECK_RC_MAC(
        cpssDxChTunnelStartMplsPwLabelTtlSet(swDevNum,eport,pwTtl[ii]));

        /* AUTODOC: bind the eport to tunnel start */
        egressEportInfo.tunnelStartPtr  = getTsIndexForPhyPort(ii);

        CHECK_RC_MAC(
        cpssDxChBrgEportToPhysicalPortEgressPortInfoSet(swDevNum,eport,&egressEportInfo));

        /* AUTODOC: set unique TS mac DA */
        mplsTsEntryPtr->macDa = tsMacDaArr[ii];
        /* AUTODOC: set TS MPLS label (most outer) */
        mplsTsEntryPtr->label1 = getTsMplsLabel(ii);
        /* AUTODOC: add the tunnels */
        CHECK_RC_MAC(
        cpssDxChTunnelStartEntrySet(swDevNum,
            egressEportInfo.tunnelStartPtr/*routerArpTunnelStartLineIndex*/,
            CPSS_TUNNEL_X_OVER_MPLS_E,
            &tsEntry));

        /* AUTODOC: define the 'mac from me' (Global Mac Sa table) --> used as mac SA for TS */
        /* AUTODOC: bind the eport to index (index  = physical port) in the
           global 'mac from me' table */
        CHECK_RC_MAC(
        cpssDxChIpRouterPortGlobalMacSaIndexSet(
            swDevNum, eport, test_phyPortArr[ii] & 0xFF));

        /* AUTODOC: set unique mac SA at index (index  = physical port) in the
           global 'mac from me' table */
        CHECK_RC_MAC(
        cpssDxChIpRouterGlobalMacSaSet(
            swDevNum, test_phyPortArr[ii] & 0xFF,&tsMacSaArr[ii]));

    }

    return GT_OK;
}

/* set ingress Service providers recognition (using TTI rule (no termination))*/
static GT_STATUS setIngressServiceProvidersRecognition(void)
{
    CPSS_DXCH_TTI_RULE_UNT    ttiPattern;
    CPSS_DXCH_TTI_ETH_RULE_STC   *ttiEthPatternPtr;
    CPSS_DXCH_TTI_RULE_UNT    ttiMask;
    CPSS_DXCH_TTI_ETH_RULE_STC   *ttiEthMaskPtr;
    CPSS_DXCH_TTI_ACTION_UNT    ttiAction;
    CPSS_DXCH_TTI_ACTION_2_STC  *ttiAction2Ptr;
    GT_U32  ii;

    cpssOsMemSet(&ttiPattern,0,sizeof(ttiPattern));
    cpssOsMemSet(&ttiMask   ,0,sizeof(ttiMask));
    cpssOsMemSet(&ttiAction ,0,sizeof(ttiAction));

    ttiEthPatternPtr = &ttiPattern.eth;
    ttiEthMaskPtr = &ttiMask.eth;
    ttiAction2Ptr = &ttiAction.type2;

    /* AUTODOC: set ingress Service providers recognition (using TTI rule (no termination)) */
    /* AUTODOC: use ETH rule on 'ingress port' + eVid + vid1 */

    /* pattern : port (not trunk) , 2 tags */
    ttiEthPatternPtr->common.srcIsTrunk   = 0;
    /*tag 0*/
    ttiEthPatternPtr->common.isTagged   = (currentNumOfTags != 0) ? GT_TRUE : GT_FALSE;
    /*tag 1*/
    ttiEthPatternPtr->isVlan1Exists = (currentNumOfTags == 2) ? GT_TRUE : GT_FALSE;

    /* mask : port (not trunk) , 2 tags */
    ttiEthMaskPtr->common.srcIsTrunk   = BIT_1-1;/*1bit*/
    ttiEthMaskPtr->common.srcPortTrunk = BIT_8-1;/*8 bits*/
    /*tag 0*/
    if(currentNumOfTags != 0)
    {
        ttiEthMaskPtr->common.vid   = BIT_13-1;/*13bits*/
    }
    ttiEthMaskPtr->common.isTagged   = BIT_1-1;/*1bit*/
    /*tag1*/
    ttiEthMaskPtr->isVlan1Exists = BIT_1-1;/*1bit*/
    if(currentNumOfTags == 2)
    {
        ttiEthMaskPtr->vid1 = BIT_12-1;/*12bits*/
    }

    /* AUTODOC: define the action : no termination , assign new vid (the global evlan) ,
        not modify vid1 (take from packet) , reassign srcEport */
    /*action*/
    ttiAction2Ptr->tunnelTerminate = GT_FALSE;/* no termination */
    ttiAction2Ptr->command = CPSS_PACKET_CMD_FORWARD_E;
    ttiAction2Ptr->tag0VlanCmd = CPSS_DXCH_TTI_VLAN_MODIFY_ALL_E;
    ttiAction2Ptr->tag0VlanId = getEvlanForVplsDomain();
    ttiAction2Ptr->tag1VlanCmd = CPSS_DXCH_TTI_VLAN_MODIFY_UNTAGGED_E;
    ttiAction2Ptr->sourceEPortAssignmentEnable = GT_TRUE;

    if(vplsSystemIndex == VPLS_DOMAIN_TAG_MODE_E)
    {
        /* in 'TAG mode' the ingress packet go AS is as passenger with additional TAGs */
        /* NOTE: on ingress it should be defined as 'untagged' (the payload may hold tags) */
        ttiAction2Ptr->nestedVlanEnable = GT_TRUE;
    }

    /* AUTODOC: eport A,B - need TTI rules to set the eVlan */
    for(ii = 0 ; ii < 2; ii++)
    {
        /* AUTODOC: enable physical port to do TTI ETH lookup */
        CHECK_RC_MAC(
        cpssDxChTtiPortLookupEnableSet(swDevNum,test_phyPortArr[ii],
            CPSS_DXCH_TTI_KEY_ETH_E,
            GT_TRUE));

        /* pattern : specific port */
        ttiEthPatternPtr->common.srcPortTrunk = test_phyPortArr[ii];
        /* tag 0 */
        if(currentNumOfTags != 0)
        {
            ttiEthPatternPtr->common.vid   = getTag0ForPhyPort(ii);
        }

        if(currentNumOfTags == 2)
        {
            /* tag 1 */
            ttiEthPatternPtr->vid1 = getTag1ForPhyPort(ii);
        }

        /* AUTODOC: assign 'default' vid1 for untagged packets */
        ttiAction2Ptr->tag1VlanId = getTag1ForPhyPort(ii);

        /* AUTODOC: reassign unique src eport */
        ttiAction2Ptr->sourceEPort = getEportForPhyPort(ii);
        /* AUTODOC: set the TTI ETH rule+action */
        CHECK_RC_MAC(
        cpssDxChTtiRuleSet(swDevNum,
            getTtiIndexForPhyPort(ii),/*index*/
            CPSS_DXCH_TTI_RULE_ETH_E,
            &ttiPattern,&ttiMask,
            CPSS_DXCH_TTI_ACTION_TYPE2_ENT,&ttiAction
            ));
    }

    return GT_OK;
}


/* set ingress VPLS tunnel termination*/
static GT_STATUS setIngressVplsTunnelTermination(void)
{
    CPSS_DXCH_TTI_RULE_UNT    ttiPattern;
    CPSS_DXCH_TTI_MPLS_RULE_STC   *ttiMplsPatternPtr;
    CPSS_DXCH_TTI_RULE_UNT    ttiMask;
    CPSS_DXCH_TTI_MPLS_RULE_STC   *ttiMplsMaskPtr;
    CPSS_DXCH_TTI_ACTION_UNT    ttiAction;
    CPSS_DXCH_TTI_ACTION_2_STC  *ttiAction2Ptr;
    GT_U32  ii;

    cpssOsMemSet(&ttiPattern,0,sizeof(ttiPattern));
    cpssOsMemSet(&ttiMask   ,0,sizeof(ttiMask));
    cpssOsMemSet(&ttiAction ,0,sizeof(ttiAction));

    /* AUTODOC: set ingress VPLS tunnel termination */

    ttiMplsPatternPtr = &ttiPattern.mpls;
    ttiMplsMaskPtr = &ttiMask.mpls;
    ttiAction2Ptr = &ttiAction.type2;

    /* pattern : port (not trunk) , 2 labels */
    ttiMplsPatternPtr->common.srcIsTrunk   = 0;
    ttiMplsPatternPtr->numOfLabels     = 1;/* 1 means 2 labels !!! */

    /* mask : port (not trunk) , 2 labels */
    ttiMplsMaskPtr->common.srcIsTrunk   = BIT_1-1;/*1bit*/
    ttiMplsMaskPtr->numOfLabels  = BIT_2-1;/*2bits*/
    /* mask : specific port , label1,2*/
    ttiMplsMaskPtr->common.srcPortTrunk = BIT_8-1;/*8 bits*/
    ttiMplsMaskPtr->label0       = BIT_20-1;/*20 bits*/
    ttiMplsMaskPtr->label1       = BIT_20-1;/*20 bits*/

    /* AUTODOC: define the action : tunnel termination (passenger ETH no CRC) ,
        assign new vid (the global evlan) , not modify vid1 (take from passenger) ,
        reassign srcEport , set meshId (for split horizon) */

    /*action*/
    ttiAction2Ptr->tunnelTerminate = GT_TRUE;
    /* AUTODOC: expect passenger with no CRC ... as we also did TS without it */
    ttiAction2Ptr->ttPassengerPacketType = CPSS_DXCH_TTI_PASSENGER_ETHERNET_NO_CRC_E;
    ttiAction2Ptr->command = CPSS_PACKET_CMD_FORWARD_E;
    ttiAction2Ptr->sourceIdSetEnable = GT_TRUE;
    ttiAction2Ptr->tag0VlanCmd = CPSS_DXCH_TTI_VLAN_MODIFY_ALL_E;
    ttiAction2Ptr->tag0VlanId = getEvlanForVplsDomain();
    ttiAction2Ptr->tag1VlanCmd = CPSS_DXCH_TTI_VLAN_MODIFY_UNTAGGED_E;
    ttiAction2Ptr->sourceEPortAssignmentEnable = GT_TRUE;


    /* eport C,D - need tunnel termination of MPLS tunnels */
    for(ii = 2/*eport C*/ ; ii < numOfPorts; ii++)
    {
        /* AUTODOC: enable physical port to do TTI MPLS lookup */
        CHECK_RC_MAC(
        cpssDxChTtiPortLookupEnableSet(swDevNum,test_phyPortArr[ii],
            CPSS_DXCH_TTI_KEY_MPLS_E,
            GT_TRUE));

        /* pattern : specific port , label1,2*/
        ttiMplsPatternPtr->common.srcPortTrunk = test_phyPortArr[ii];
        ttiMplsPatternPtr->label0       = getTsMplsLabel(ii);
        ttiMplsPatternPtr->label1       = pwLabels[ii];

        /* AUTODOC: assign 'default' vid1 for untagged packets */
        ttiAction2Ptr->tag1VlanId = (currentNumOfTags == 2) ?
                                    0x500: /* not used as packet should be recognized with tag1 */
                                    outerTag1Arr[ii];
        /* AUTODOC : the mesh-id (by sourceId)*/
        ttiAction2Ptr->sourceId = meshIdArr[ii];
        /* AUTODOC: reassign unique src eport */
        ttiAction2Ptr->sourceEPort = getEportForPhyPort(ii);

        /* AUTODOC: set the TTI MPLS rule+action */
        CHECK_RC_MAC(
        cpssDxChTtiRuleSet(swDevNum,
            getTtiIndexForPhyPort(ii),/*index*/
            CPSS_DXCH_TTI_RULE_MPLS_E/* CPSS_TUNNEL_X_OVER_MPLS_E*/,
            &ttiPattern,&ttiMask,
            CPSS_DXCH_TTI_ACTION_TYPE2_ENT,&ttiAction
            ));
    }

    return GT_OK;
}

/* set TT rule for 'enh-UT' captured packets ... those not need to match the other TTI rules */
static GT_STATUS setTtiRuleForEnUtCapturedPackets(void)
{
    CPSS_DXCH_TTI_RULE_UNT    ttiPattern;
    CPSS_DXCH_TTI_RULE_COMMON_STC   *ttiCommonPatternPtr;
    CPSS_DXCH_TTI_RULE_UNT    ttiMask;
    CPSS_DXCH_TTI_RULE_COMMON_STC   *ttiCommonMaskPtr;
    CPSS_DXCH_TTI_ACTION_UNT    ttiAction;
    CPSS_DXCH_TTI_ACTION_2_STC  *ttiAction2Ptr;

    cpssOsMemSet(&ttiPattern,0,sizeof(ttiPattern));
    cpssOsMemSet(&ttiMask   ,0,sizeof(ttiMask));
    cpssOsMemSet(&ttiAction ,0,sizeof(ttiAction));

    /* AUTODOC : set TT rule for 'enh-UT' captured packets ... those not need to match the other TTI rules
                those rules must be before any of the other TTI rules */
    /* AUTODOC: use ETH rule and MPLS rule to match PVID = 0 (the indication of 'enh-UT' captured packets) */

    ttiCommonPatternPtr = &ttiPattern.eth.common;
    ttiCommonMaskPtr = &ttiMask.eth.common;
    ttiAction2Ptr = &ttiAction.type2;

    /* pattern : port (not trunk) , 2 tags */
    ttiCommonPatternPtr->vid   = 0;/* pvid == 0 !!!!*/

    /* tag 0*/
    ttiCommonMaskPtr->vid   = BIT_13-1;/*13bits*/

    /* AUTODOC: define the action : 'transparent' (no modifications) ..
        just so no other rule will catch it ! */
    /*action*/
    ttiAction2Ptr->tunnelTerminate = GT_FALSE;/* no termination */
    ttiAction2Ptr->command = CPSS_PACKET_CMD_FORWARD_E;
    ttiAction2Ptr->tag0VlanCmd = CPSS_DXCH_TTI_VLAN_DO_NOT_MODIFY_E;
    ttiAction2Ptr->tag1VlanCmd = CPSS_DXCH_TTI_VLAN_MODIFY_UNTAGGED_E;
    ttiAction2Ptr->tag1VlanId = 0;/* not used as packet should be recognized with tag1 */

    /* AUTODOC: set the TTI ETH rule+action */
    CHECK_RC_MAC(
    cpssDxChTtiRuleSet(swDevNum,
        tunnelTerminationIndex[0] - TTI_INDEX_MAC(1),/*index*/
        CPSS_DXCH_TTI_RULE_ETH_E,
        &ttiPattern,&ttiMask,
        CPSS_DXCH_TTI_ACTION_TYPE2_ENT,&ttiAction
        ));

    /* AUTODOC: set the TTI MPLS rule+action */
    CHECK_RC_MAC(
    cpssDxChTtiRuleSet(swDevNum,
        tunnelTerminationIndex[0] - TTI_INDEX_MAC(2),/*index*/
        CPSS_DXCH_TTI_RULE_MPLS_E,
        &ttiPattern,&ttiMask,
        CPSS_DXCH_TTI_ACTION_TYPE2_ENT,&ttiAction
        ));

    return GT_OK;
}

/* set bridge */
static GT_STATUS setBridge(void)
{
    CPSS_PORTS_BMP_STC                   portsMembers;
    CPSS_PORTS_BMP_STC                   portsTagging;
    CPSS_DXCH_BRG_VLAN_INFO_STC          vlanInfo;
    CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC portsTaggingCmd;
    GT_U32                               ii;
    GT_BOOL                              addPort = GT_TRUE;
    GT_U32                               eport;

    cpssOsMemSet(&portsMembers,0,sizeof(portsMembers));
    cpssOsMemSet(&portsTagging,0,sizeof(portsTagging));
    cpssOsMemSet(&portsTaggingCmd,0,sizeof(portsTaggingCmd));
    cpssOsMemSet(&vlanInfo,0,sizeof(vlanInfo));

    /* AUTODOC: define global eVlan : physical ports members , controlled learning ,
        specific evidx for flooding , fid = vid , */

    /* AUTODOC: add the physical ports to global eVlan */
    for(ii = 0 ; ii < numOfPorts ; ii++)
    {
        CPSS_PORTS_BMP_PORT_ENABLE_MAC(&portsMembers,test_phyPortArr[ii],addPort);
    }

    /* AUTODOC: set global eVlan controlled learning*/
    vlanInfo.autoLearnDisable = GT_TRUE;
    vlanInfo.naMsgToCpuEn = GT_TRUE;

    /* specific evidx for flooding */
    vlanInfo.floodVidx = getEvidxForVplsDomain();
    vlanInfo.floodVidxMode = CPSS_DXCH_BRG_VLAN_FLOOD_VIDX_MODE_ALL_FLOODED_TRAFFIC_E;
    /* fid like global eVlan */
    vlanInfo.fidValue       = getEvlanForVplsDomain();
    vlanInfo.unregIpmEVidx  = getEvidxForVplsDomain();

    /* AUTODOC: write the vlan entry */
    CHECK_RC_MAC(
    cpssDxChBrgVlanEntryWrite(swDevNum,getEvlanForVplsDomain(),
        &portsMembers,&portsTagging,&vlanInfo,&portsTaggingCmd));

    /* AUTODOC: set eports to allow controlled learning (na to cpu , no auto learn) */
    for(ii = 0 ; ii < numOfPorts ; ii++)
    {
        eport = getEportForPhyPort(ii);

        CHECK_RC_MAC(
        cpssDxChBrgFdbNaToCpuPerPortSet(swDevNum,eport,GT_TRUE));

        CHECK_RC_MAC(
        cpssDxChBrgFdbPortLearnStatusSet(swDevNum,eport,
            GT_FALSE/*disable auto learn*/,
            CPSS_LOCK_FRWRD_E));
    }



    return GT_OK;
}

/* set L2Mll */
static GT_STATUS setL2Mll(void)
{
    CPSS_DXCH_L2_MLL_LTT_ENTRY_STC        lttEntry;
    CPSS_DXCH_L2_MLL_PAIR_STC             mllPairEntry;
    CPSS_DXCH_L2_MLL_ENTRY_STC            *mllEntryPtr;
    GT_U32                                mllPairIndex;
    GT_U32                                ii;

    cpssOsMemSet(&lttEntry,0,sizeof(lttEntry));
    cpssOsMemSet(&mllPairEntry,0,sizeof(mllPairEntry));

    /* the index is split by the 'lsb' */
    mllPairIndex = setStartMllIndex() >> 1;
    lttEntry.mllPointer    = mllPairIndex;
    lttEntry.entrySelector = 0;/* start of the entry */
    lttEntry.mllMaskProfileEnable = GT_FALSE;/* we have no vidx subsets of this 'evlan flooding domain'*/

    /* AUTODOC: set the L2MLL LTT entry (index = eVidx of global eVlan) */
    /* AUTODOC: the LTT point to index in the MLL table */
    CHECK_RC_MAC(
    cpssDxChL2MllLttEntrySet(swDevNum,
        (getEvidxForVplsDomain() - _4K),/* working in mode of relative to _4K */
        &lttEntry));

    /* AUTODOC: define 'link list' of 'MLL nodes' one per eport */
    /* AUTODOC: use 2 MLL entries (each entry hold 2 nodes) */

    /* AUTODOC: set the 4 eports that are members in the evlan into the MLL*/
    mllEntryPtr = &mllPairEntry.firstMllNode;
    mllEntryPtr->last = GT_FALSE;
    mllEntryPtr->egressInterface.type = CPSS_INTERFACE_PORT_E;
    mllEntryPtr->egressInterface.devPort.hwDevNum = hwDevNum;

    ii = 0;
    mllEntryPtr->egressInterface.devPort.portNum = getEportForPhyPort(ii);

    mllEntryPtr->meshId = meshIdArr[ii];

    mllEntryPtr = &mllPairEntry.secondMllNode;
    mllEntryPtr->last = GT_FALSE;
    mllEntryPtr->egressInterface.type = CPSS_INTERFACE_PORT_E;
    mllEntryPtr->egressInterface.devPort.hwDevNum = hwDevNum;

    ii = 1;
    mllEntryPtr->egressInterface.devPort.portNum = getEportForPhyPort(ii);
    mllEntryPtr->meshId = meshIdArr[ii];

    mllPairEntry.nextPointer = mllPairIndex + 1;
    mllPairEntry.entrySelector = 0;/* start of the entry */

    /* AUTODOC: write first MLL entry (2 nodes)*/
    CHECK_RC_MAC(
    cpssDxChL2MllPairWrite(swDevNum, mllPairIndex ,
        CPSS_DXCH_PAIR_READ_WRITE_WHOLE_E,
        &mllPairEntry));

    mllEntryPtr = &mllPairEntry.firstMllNode;
    ii = 2;
    mllEntryPtr->egressInterface.devPort.portNum = getEportForPhyPort(ii);
    mllEntryPtr->meshId = meshIdArr[ii];

    mllEntryPtr = &mllPairEntry.secondMllNode;
    mllEntryPtr->last = GT_TRUE;/* AUTODOC: last eport --> the end of the list */
    ii = 3;
    mllEntryPtr->egressInterface.devPort.portNum = getEportForPhyPort(ii);
    mllEntryPtr->meshId = meshIdArr[ii];

    /* AUTODOC: write second MLL entry (2 nodes)*/
    CHECK_RC_MAC(
    cpssDxChL2MllPairWrite(swDevNum, mllPairIndex + 1 ,
        CPSS_DXCH_PAIR_READ_WRITE_WHOLE_E,
        &mllPairEntry));


    /* AUTODOC: set the L2MLL to consider 4 bits from the sst-id as the meshId */
    CHECK_RC_MAC(
    cpssDxChL2MllSourceBasedFilteringConfigurationSet(swDevNum,GT_TRUE,0,4));


    return GT_OK;
}

typedef struct{
    VPLS_DOMAIN_MODE_ENT   vplsMode;
    GT_U32                 numVlanTags;
}SYSTEM_INFO_STC;


static SYSTEM_INFO_STC systemsToTestArr[] =
{
        /*vplsMode*/            /*numVlanTags*/
    {VPLS_DOMAIN_MODE_ETH_POP1_E    , 2         },
    {VPLS_DOMAIN_RAW_MODE_E         , 2         },
    {VPLS_DOMAIN_TAG_MODE_E         , 1         }
};


static GT_U32  numOfSystemsToTest = sizeof(systemsToTestArr)/sizeof(systemsToTestArr[0]);

static GT_U32   allowPrint = 0;

/* presentation of system is in Documentum :
    Cabinets/SWITCHING/CPSS and PSS/R&D Internal/CPSS/sip5 device (Lion3,BC2)/Test design/VPLS/Bobcat2_VPLS_Demo.pptx
    http://docil.marvell.com/webtop/drl/objectId/0900dd88801954ba

    ePort A is assigned for EFP1 - port to Ethernet domain - service provider 1 with outer vlan tag 10
    ePort B is assigned for EFP2 - port to Ethernet domain - service provider 2 with outer vlan tag 20
    ePort C is assigned for PW1  - port to MPLS domain , LABEL 50 - PW port 1 - PW label 60
    ePort D is assigned for PW2  - port to MPLS domain , LABEL 50 - PW port 2 - PW label 70

    eVlan - VPLS domain - the eVlan that represents the 'VPLS domain'
*/

/*NOT static .. because may be called from terminal , as configurations for 'vpls demo' */
/*NOT static*/GT_STATUS vplsDemoConfigSet1(
    /* physical ports */
    IN  GT_PHYSICAL_PORT_NUM  phyPort_A,/* physical port associated with ePort_A */
    IN  GT_PHYSICAL_PORT_NUM  phyPort_B,/* physical port associated with ePort_B */
    IN  GT_PHYSICAL_PORT_NUM  phyPort_C,/* physical port associated with ePort_C */
    IN  GT_PHYSICAL_PORT_NUM  phyPort_D,/* physical port associated with ePort_D */
    /* eports - service providers */
    IN  GT_PORT_NUM  ePort_A,/* ePort_A - service provider A (outer tag 10)*/
    IN  GT_PORT_NUM  ePort_B,/* ePort_B - service provider B (outer tag 20)*/
    /* eports - MPLS tunnels VPLS. both belong to the same VPLS tree. (no flood between them) */
    IN  GT_PORT_NUM  ePort_C,/* ePort_C - MPLS tunnel 50 PW 60 (MPLS tunnel with Ethernet passenger) , passenger only with tag 1 (no tag 0) */
    IN  GT_PORT_NUM  ePort_D,/* ePort_D - MPLS tunnel 50 PW 70 (MPLS tunnel with Ethernet passenger) , passenger only with tag 1 (no tag 0) */
    /* eVlan info */
    IN GT_U32                 eVlan/* the vlan that the packets use inside the device for all bridging decisions */
)
{
    GT_U32  eport;
    GT_U32  ii = 0;
    GT_U32      ttiBaseIndex = appDemoDxChTcamTtiBaseIndexGet(swDevNum,0);

    test_phyPortArr[0] = phyPort_A;
    test_phyPortArr[1] = phyPort_B;
    test_phyPortArr[2] = phyPort_C;
    test_phyPortArr[3] = phyPort_D;

    test_ePortArr[0] = ePort_A;
    test_ePortArr[1] = ePort_B;
    test_ePortArr[2] = ePort_C;
    test_ePortArr[3] = ePort_D;

    globalEvlan = (GT_U16)eVlan;
    globalFloodEvidx = (GT_U16)((globalEvlan & (_4K-1)) + _4K);/*  _8K > value >= _4K*/

    if(allowPrint)
    {
        cpssOsPrintf("\n\n");
        cpssOsPrintf("vplsMode[%s] \n",
            vplsDomainModeNames[vplsSystemIndex]);
        cpssOsPrintf("Description : %s \n",
            vplsDomainModeDescriptions[vplsSystemIndex]);
        cpssOsPrintf("numVlanTags[%d] (expect [%d] vlan tags on ETH port) \n",
            currentNumOfTags,currentNumOfTags);
        cpssOsPrintf("vpls domain (evlan) [%d]\n",
            getEvlanForVplsDomain());
        cpssOsPrintf("evidx [%d]\n",
            getEvidxForVplsDomain());
        cpssOsPrintf("MLL index [%d]\n",
            setStartMllIndex() >> 1);

        for(ii = 0 ; ii < 4 ; ii ++)
        {
            cpssOsPrintf("\t\t");

            eport = getEportForPhyPort(ii);
            cpssOsPrintf("eport [%d] [0x%x] ",
                eport,
                eport);

            cpssOsPrintf("tti index [0x%x] ",
                getTtiIndexForPhyPort(ii));

            if(ii < 2)
            {
                /* TS not relevant for portIndex 0,1 */
                cpssOsPrintf("TS index [-----] ");
                cpssOsPrintf("TS label [----] ");
                cpssOsPrintf("PW label [----] ");
            }
            else
            {
                cpssOsPrintf("TS index [0x%x] ",
                    getTsIndexForPhyPort(ii));

                cpssOsPrintf("TS label [0x%x] ",
                    getTsMplsLabel(ii));

                cpssOsPrintf("PW label [0x%x] ",
                    pwLabels[ii]);
            }

            if((vplsSystemIndex == VPLS_DOMAIN_MODE_ETH_POP1_E && ii >= 2) || /* those MPLS ports not get 'tag0' */
               (vplsSystemIndex == VPLS_DOMAIN_RAW_MODE_E  && ii >= 2))/* those MPLS ports not get 'tag0' */
            {

                cpssOsPrintf("Tag 0 [----] ");
            }
            else
            {
                cpssOsPrintf("Tag 0 [0x%x] ",
                    getTag0ForPhyPort(ii));
            }

            if((vplsSystemIndex == VPLS_DOMAIN_RAW_MODE_E && ii >= 2) || /* those MPLS ports not get 'tag1' */
               (vplsSystemIndex == VPLS_DOMAIN_TAG_MODE_E && ii < 2))    /* those ETH ports not get 'tag1' */
            {
                cpssOsPrintf("Tag 1 [----] ");
            }
            else
            {
                cpssOsPrintf("Tag 1 [0x%x] ",
                    getTag1ForPhyPort(ii));
            }

            if(vplsSystemIndex == VPLS_DOMAIN_TAG_MODE_E &&
                ii >= 2 && currentNumOfTags > 0)
            {
                /* the TAG mode consider the ingress packet to come without tags
                   (after tti tcam match on the vlan tag) so packet egress with the additional ingress tag(s) */
                cpssOsPrintf("(Tag 2 [0x%x or 0x%x]) ",
                    getTag0ForPhyPort(0) , getTag0ForPhyPort(1));
            }

            if(vplsSystemIndex == VPLS_DOMAIN_TAG_MODE_E &&
                ii >= 2 && currentNumOfTags > 1)
            {
                /* the TAG mode consider the ingress packet to come without tags
                   (after tti tcam match on the vlan tag) so packet egress with the additional ingress tag(s) */
                cpssOsPrintf("(Tag 3 [0x%x or 0x%x]) ",
                    getTag1ForPhyPort(0) , getTag1ForPhyPort(1));
            }

            cpssOsPrintf("\n");
        }
    }

    tunnelTerminationIndex[0] = tunnelTerminationRelativeIndex[0] + ttiBaseIndex;
    tunnelTerminationIndex[1] = tunnelTerminationRelativeIndex[1] + ttiBaseIndex;
    tunnelTerminationIndex[2] = tunnelTerminationRelativeIndex[2] + ttiBaseIndex;
    tunnelTerminationIndex[3] = tunnelTerminationRelativeIndex[3] + ttiBaseIndex;

    /* AUTODOC: get hw dev num */
    CHECK_RC_MAC(
    cpssDxChCfgHwDevNumGet(swDevNum,&hwDevNum));

    /* AUTODOC: set e2phy */
    CHECK_RC_MAC(
    setE2Phy());

    /* AUTODOC: set tpid */
    CHECK_RC_MAC(
    setTpid());

    /* AUTODOC: set egress tagging */
    CHECK_RC_MAC(
    setEgressTagging());

    /* AUTODOC: set egress VPLS tunnel */
    CHECK_RC_MAC(
    setEgressVplsTunnel());

    /* AUTODOC: set ingress Service providers recognition (using TTI rule (no termination))*/
    CHECK_RC_MAC(
    setIngressServiceProvidersRecognition());

    /* AUTODOC: set ingress VPLS tunnel termination*/
    CHECK_RC_MAC(
    setIngressVplsTunnelTermination());

    /* AUTODOC: set bridge */
    CHECK_RC_MAC(
    setBridge());

    /* AUTODOC: set L2Mll */
    CHECK_RC_MAC(
    setL2Mll());


    return GT_OK;
}

/*set from terminal the vpls system */
GT_STATUS vplsDemoConfigSet1_vplsSystemIndex(
    IN GT_U32 index
)
{
    if(index >= VPLS_DOMAIN_LAST)
    {
        return GT_BAD_PARAM;
    }

    vplsSystemIndex = index;

    return GT_OK;
}

/*set from terminal the num of vlan tags to egress the ethernet ports */
GT_STATUS vplsDemoConfigSet1_numOfTags
(
    IN GT_U32 numOfTags
)
{
    if(numOfTags > 2)
    {
        return GT_BAD_PARAM;
    }

    currentNumOfTags = numOfTags;
    return GT_OK;
}

/*NOT static .. because may be called from terminal , as configurations for 'vpls demo' */
/*NOT static*/GT_STATUS vplsDemoConfigSet1_PhyPorts(
    /* physical ports */
    IN  GT_PHYSICAL_PORT_NUM  phyPort_A,/* physical port associated with ePort_A */
    IN  GT_PHYSICAL_PORT_NUM  phyPort_B,/* physical port associated with ePort_B */
    IN  GT_PHYSICAL_PORT_NUM  phyPort_C,/* physical port associated with ePort_C */
    IN  GT_PHYSICAL_PORT_NUM  phyPort_D /* physical port associated with ePort_D */
)
{
    GT_STATUS   rc;
    GT_U32       eVlan = 100;
    GT_PORT_NUM  ePortArr[4];
    GT_U32      ii;

    /* make sure eport not overlap - regardless to phy port */
    ePortArr[0] = (phyPort_A * 10) + 0x1000;
    ePortArr[1] = (phyPort_B * 10) + 0x1000;
    ePortArr[2] = (phyPort_C * 10) + 0x1000;
    ePortArr[3] = (phyPort_D * 10) + 0x1000;

    allowPrint = 1;

    for(ii = 0 ; ii < numOfSystemsToTest ; ii++ )
    {
        vplsSystemIndex  = systemsToTestArr[ii].vplsMode;
        currentNumOfTags = systemsToTestArr[ii].numVlanTags;

        /* AUTODOC: set configurations */
        rc = vplsDemoConfigSet1(
            /* physical ports */
            phyPort_A,    phyPort_B,    phyPort_C,    phyPort_D,
            /* eports */
            ePortArr[0],    ePortArr[1],    ePortArr[2],    ePortArr[3],
            /*evlan*/
            eVlan
            );
        if(rc != GT_OK)
        {
            PRV_UTF_LOG2_MAC("vplsDemoConfigSet1 : failed vplsMode[%d] , numTags[%d] \n",
                vplsSystemIndex, currentNumOfTags);
            return rc;
        }
    }

    allowPrint = 0;


    return GT_OK;
}
/*NOT static .. because may be called from terminal , as configurations for 'vpls demo' */
/*NOT static*/GT_STATUS vplsDemoConfigSet1_AutoParam(void)
{
#ifdef ASIC_SIMULATION
    GT_PHYSICAL_PORT_NUM  phyPortArr[4] = {0,18,36,54};/* physical port associated with ePort_A..D */
#else /*!ASIC_SIMULATION*/
    GT_PHYSICAL_PORT_NUM  phyPortArr[4] = {0,18,30,47};/* physical port associated with ePort_A..D */
#endif/*!ASIC_SIMULATION*/

    return vplsDemoConfigSet1_PhyPorts(phyPortArr[0],
                                phyPortArr[1],
                                phyPortArr[2],
                                phyPortArr[3]);

}

/* set e2phy */
static GT_STATUS setE2Phy_restore(void)
{
    /* nothing to restore , because the high ePort is above the physical ports */

    return GT_OK;
}

/* set tpid */
/* we not change the 0x8100 on entries */
static GT_STATUS setTpid_restore(void)
{
    GT_U32  tpidBmp,profileTag1Only,profileNoTags;
    GT_U32  ii;
    GT_U32  saveTpidProfileIndex = 0;


    /* bind physical port A,B,C,D profile 0 ... to recognize tag 0(provider),1(passenger) */
    for(ii = 0 ; ii < numOfPorts ; ii++)
    {
        CHECK_RC_MAC(((SAVE_TPID_PROFILE_SIZE_CNS - saveTpidProfileIndex) == 0));

        CHECK_RC_MAC(
        cpssDxChBrgVlanPortIngressTpidProfileSet(swDevNum,
            test_phyPortArr[ii],
            CPSS_VLAN_ETHERTYPE0_E,
            GT_TRUE/*isDefaultProfile*/,
            saveTpidProfileArr[saveTpidProfileIndex++]));

        CHECK_RC_MAC(((SAVE_TPID_PROFILE_SIZE_CNS - saveTpidProfileIndex) == 0));

        CHECK_RC_MAC(
        cpssDxChBrgVlanPortIngressTpidProfileSet(swDevNum,
            test_phyPortArr[ii],
            CPSS_VLAN_ETHERTYPE1_E,
            GT_TRUE/*isDefaultProfile*/,
            saveTpidProfileArr[saveTpidProfileIndex++]));
    }

    profileTag1Only = 3;
    /* restore profile 3 */
    tpidBmp = 0xFF;
    CHECK_RC_MAC(
    cpssDxChBrgVlanIngressTpidProfileSet(swDevNum,profileTag1Only,CPSS_VLAN_ETHERTYPE0_E,tpidBmp));

    /* restore eports C,D profile 0 */
    for(ii = 2/*eport C*/ ; ii < numOfPorts ; ii++)
    {
        CHECK_RC_MAC(((SAVE_TPID_PROFILE_SIZE_CNS - saveTpidProfileIndex) == 0));
        CHECK_RC_MAC(
        cpssDxChBrgVlanPortIngressTpidProfileSet(swDevNum,
            getEportForPhyPort(ii),
            CPSS_VLAN_ETHERTYPE0_E,
            GT_FALSE/*isDefaultProfile*/,
            saveTpidProfileArr[saveTpidProfileIndex++]));

        CHECK_RC_MAC(((SAVE_TPID_PROFILE_SIZE_CNS - saveTpidProfileIndex) == 0));
        CHECK_RC_MAC(
        cpssDxChBrgVlanPortIngressTpidProfileSet(swDevNum,
            getEportForPhyPort(ii),
            CPSS_VLAN_ETHERTYPE1_E,
            GT_FALSE/*isDefaultProfile*/,
            saveTpidProfileArr[saveTpidProfileIndex++]));
    }

    profileNoTags = 4;
    /* AUTODOC: make profile 4 not recognize tag 0 */
    tpidBmp = 0xFF;
    CHECK_RC_MAC(
    cpssDxChBrgVlanIngressTpidProfileSet(swDevNum,profileNoTags,CPSS_VLAN_ETHERTYPE0_E,tpidBmp));
    /* AUTODOC: make profile 4 not recognize tag 1 */
    tpidBmp = 0xFF;
    CHECK_RC_MAC(
    cpssDxChBrgVlanIngressTpidProfileSet(swDevNum,profileNoTags,CPSS_VLAN_ETHERTYPE1_E,tpidBmp));

    prvVplsIsTpidSaveDone--;
    return GT_OK;
}

/* set egress tagging */
static GT_STATUS setEgressTagging_restore(void)
{
    GT_U32  ii;
    GT_U32  eport;

    /* eport A,B - tag0,tag1 */
    for(ii = 0/*eport A*/ ; ii < 2/*eportB inclusive*/ ; ii++)
    {
        eport = getEportForPhyPort(ii);
        /*restore tagging from vlan */
        CHECK_RC_MAC(
        cpssDxChBrgVlanEgressPortTagStateModeSet(swDevNum,eport,
            CPSS_DXCH_BRG_VLAN_EPORT_TAG_STATE_EVLAN_MODE_E));
        /*  untagged*/
        CHECK_RC_MAC(
        cpssDxChBrgVlanEgressPortTagStateSet(swDevNum,eport,
            CPSS_DXCH_BRG_VLAN_PORT_UNTAGGED_CMD_E));

        /* do not force to use tag0 from eport not from ingress pipe */
        CHECK_RC_MAC(
        cpssDxChBrgVlanPortVid0CommandEnableSet(swDevNum,eport,GT_FALSE));

        /*set tag 0 for egress - default vlan */
        CHECK_RC_MAC(
        cpssDxChBrgVlanPortVidSet(swDevNum,eport,
            CPSS_DIRECTION_EGRESS_E,
            1));

        CHECK_RC_MAC(
        cpssDxChBrgVlanPortVid1CommandEnableSet(swDevNum,
            eport,
            GT_FALSE));

        /* unset value for vid 1 for egress*/
        CHECK_RC_MAC(
        cpssDxChBrgVlanPortVid1Set(swDevNum,
            eport,
            CPSS_DIRECTION_EGRESS_E,
            0));

    }

    /* eport C,D - tag1 only */
    for(/*continue*/; ii < numOfPorts ; ii++)
    {
        eport = getEportForPhyPort(ii);
        /*restore tagging from vlan */
        CHECK_RC_MAC(
        cpssDxChBrgVlanEgressPortTagStateModeSet(swDevNum,eport,
            CPSS_DXCH_BRG_VLAN_EPORT_TAG_STATE_EVLAN_MODE_E));
        /*  untagged*/
        CHECK_RC_MAC(
        cpssDxChBrgVlanEgressPortTagStateSet(swDevNum,eport,
            CPSS_DXCH_BRG_VLAN_PORT_UNTAGGED_CMD_E));

        /*  remove force of use tag0 from eport not from ingress pipe */
        CHECK_RC_MAC(
        cpssDxChBrgVlanPortVid0CommandEnableSet(swDevNum,
            eport,
            GT_FALSE));

        /*set tag 0 for egress - default vlan */
        CHECK_RC_MAC(
        cpssDxChBrgVlanPortVidSet(swDevNum,
            eport,
            CPSS_DIRECTION_EGRESS_E,
            1));

        /*  force to use tag1 from eport not from ingress pipe */
        CHECK_RC_MAC(
        cpssDxChBrgVlanPortVid1CommandEnableSet(swDevNum,
            eport,
            GT_FALSE));

        /*  unset value for vid 1 for egress*/
        CHECK_RC_MAC(
        cpssDxChBrgVlanPortVid1Set(swDevNum,
            eport,
            CPSS_DIRECTION_EGRESS_E,
            0));

    }

    return GT_OK;
}

/* set egress VPLS tunnel */
static GT_STATUS setEgressVplsTunnel_restore(void)
{
    GT_U32  ii;
    CPSS_DXCH_BRG_EGRESS_PORT_INFO_STC  egressEportInfo;
    GT_U32  eport;

    cpssOsMemSet(&egressEportInfo,0,sizeof(egressEportInfo));
    egressEportInfo.tunnelStart = GT_FALSE;


    /* eport C,D - need tunnel start and PW label*/
    for(ii = 2/*eport C*/ ; ii < numOfPorts ; ii++)
    {
        eport = getEportForPhyPort(ii);
        /* disable the PW label */
        CHECK_RC_MAC(
        cpssDxChTunnelStartMplsPwLabelPushEnableSet(swDevNum,eport,GT_FALSE));

        /* reset the PW label */
        CHECK_RC_MAC(
        cpssDxChTunnelStartMplsPwLabelSet(swDevNum,eport,0));

        CHECK_RC_MAC(
        cpssDxChTunnelStartMplsPwLabelExpSet(swDevNum,eport,0));

        CHECK_RC_MAC(
        cpssDxChTunnelStartMplsPwLabelTtlSet(swDevNum,eport,0));

        /* unbind eport from TS */
        CHECK_RC_MAC(
        cpssDxChBrgEportToPhysicalPortEgressPortInfoSet(swDevNum,eport,&egressEportInfo));

        /* set index of the egress physical port to have 'per port' mac SA of the device
           this is for 'mac from me' */
        CHECK_RC_MAC(
        cpssDxChIpRouterPortGlobalMacSaIndexSet(
            swDevNum, eport, 0));

        CHECK_RC_MAC(
        cpssDxChIpRouterGlobalMacSaSet(
            swDevNum, test_phyPortArr[ii] & 0xFF,&tsMacSaArr[0]/*all zero*/));
    }

    return GT_OK;
}

/* set ingress Service providers recognition (using TTI rule (no termination))*/
static GT_STATUS setIngressServiceProvidersRecognition_restore(void)
{
    GT_U32  ii;

    /* eport A,B - need TTI rules to set the eVlan */
    for(ii = 0 ; ii < 2; ii++)
    {
        /* remove ETH lookup on the port */
        CHECK_RC_MAC(
        cpssDxChTtiPortLookupEnableSet(swDevNum,
            test_phyPortArr[ii],
            CPSS_DXCH_TTI_KEY_ETH_E,
            GT_FALSE));

        /* invalidate the rule */
        CHECK_RC_MAC(
        cpssDxChTtiRuleValidStatusSet(swDevNum,
            getTtiIndexForPhyPort(ii),/*index*/
            GT_FALSE));
    }

    return GT_OK;
}

/* set ingress VPLS tunnel termination*/
static GT_STATUS setIngressVplsTunnelTermination_restore(void)
{
    GT_U32  ii;

    /* eport C,D - need tunnel termination of MPLS tunnels */
    for(ii = 2/*eport C*/ ; ii < numOfPorts; ii++)
    {
        /* remove ETH lookup on the port */
        CHECK_RC_MAC(
        cpssDxChTtiPortLookupEnableSet(swDevNum,
            test_phyPortArr[ii],
            CPSS_DXCH_TTI_KEY_MPLS_E,
            GT_FALSE));

        /* invalidate the rule */
        CHECK_RC_MAC(
        cpssDxChTtiRuleValidStatusSet(swDevNum,
            getTtiIndexForPhyPort(ii),/*index*/
            GT_FALSE));
    }

    return GT_OK;
}

/* set TT rule for 'enh-UT' captured packets ... those not need to match the other TTI rules */
static GT_STATUS setTtiRuleForEnUtCapturedPackets_restore(void)
{
    CHECK_RC_MAC(
    cpssDxChTtiRuleValidStatusSet(swDevNum,
        tunnelTerminationIndex[0] - TTI_INDEX_MAC(1),/*index*/
        GT_FALSE));

    CHECK_RC_MAC(
    cpssDxChTtiRuleValidStatusSet(swDevNum,
        tunnelTerminationIndex[0] - TTI_INDEX_MAC(2),/*index*/
        GT_FALSE));

    return GT_OK;
}

/* set bridge */
static GT_STATUS setBridge_restore(void)
{
    GT_U32                               ii;
    GT_U32  eport;

    CHECK_RC_MAC(
    cpssDxChBrgVlanEntryInvalidate(swDevNum,getEvlanForVplsDomain()));

    /*add the eports to allow controlled learning (na to cpu , no auto learn) */
    for(ii = 0 ; ii < numOfPorts ; ii++)
    {
        eport = getEportForPhyPort(ii);

        CHECK_RC_MAC(
        cpssDxChBrgFdbNaToCpuPerPortSet(swDevNum,eport,GT_FALSE));

        CHECK_RC_MAC(
        cpssDxChBrgFdbPortLearnStatusSet(swDevNum,eport,
            GT_TRUE/*restore auto learn*/,
            CPSS_LOCK_FRWRD_E));
    }

    return GT_OK;
}

/* set L2Mll */
static GT_STATUS setL2Mll_restore(void)
{
    CPSS_DXCH_L2_MLL_LTT_ENTRY_STC        lttEntry;
    CPSS_DXCH_L2_MLL_PAIR_STC             mllPairEntry;
    GT_U32                                mllPairIndex;

    cpssOsMemSet(&lttEntry,0,sizeof(lttEntry));
    cpssOsMemSet(&mllPairEntry,0,sizeof(mllPairEntry));

    /* the index is split by the 'lsb' */
    mllPairIndex = setStartMllIndex() >> 1;
    lttEntry.mllPointer    = 0;
    lttEntry.entrySelector = 0;/* start of the entry */
    lttEntry.mllMaskProfileEnable = GT_FALSE;/* we have no vidx subsets of this 'evlan flooding domain'*/

    /* set the L2MLL LTT entry */
    CHECK_RC_MAC(
    cpssDxChL2MllLttEntrySet(swDevNum,
        (getEvidxForVplsDomain() - _4K),/* working in mode of relative to _4K */
        &lttEntry));

    CHECK_RC_MAC(
    cpssDxChL2MllPairWrite(swDevNum, mllPairIndex ,
        CPSS_DXCH_PAIR_READ_WRITE_WHOLE_E,
        &mllPairEntry));

    CHECK_RC_MAC(
    cpssDxChL2MllPairWrite(swDevNum, mllPairIndex + 1 ,
        CPSS_DXCH_PAIR_READ_WRITE_WHOLE_E,
        &mllPairEntry));


    /* set the L2MLL to consider 0 bits from the sst-id as the meshId */
    CHECK_RC_MAC(
    cpssDxChL2MllSourceBasedFilteringConfigurationSet(swDevNum,GT_TRUE,0,0));


    return GT_OK;
}

/**
* @internal prvTgfBrgVplsBasicTest1_config function
* @endinternal
*
* @brief   config all the system.
*/
static GT_STATUS prvTgfBrgVplsBasicTest1_config
(
    GT_VOID
)
{
    GT_STATUS   rc;
    GT_U32       eVlan = PRV_TGF_VLANID_CNS;
    GT_U32  ii;

    test_phyPortArr[0] = prvTgfPortsArray[0];
    test_phyPortArr[1] = prvTgfPortsArray[1];
    test_phyPortArr[2] = prvTgfPortsArray[2];
    test_phyPortArr[3] = prvTgfPortsArray[3];

    /* used fixed eports that will allow the arithmetic we do in getEportForPhyPort() */

    if(IS_AC3X_DEV_MAC(prvTgfDevNum))
    {
        test_ePortArr[0] = /*test_phyPortArr[0]*/ 58  + 0x1000;
        test_ePortArr[1] = /*test_phyPortArr[1]*/ 36  + 0x1000;
        test_ePortArr[2] = /*test_phyPortArr[2]*/ 18  + 0x1000;
        test_ePortArr[3] = /*test_phyPortArr[3]*/ 71  + 0x1000;
    }
    else
    {
        test_ePortArr[0] = /*test_phyPortArr[0]*/ 58  + 0x200;
        test_ePortArr[1] = /*test_phyPortArr[1]*/ 36  + 0x200;
        test_ePortArr[2] = /*test_phyPortArr[2]*/ 18  + 0x200;
        test_ePortArr[3] = /*test_phyPortArr[3]*/ 71  + 0x200;
    }

    cpssOsPrintf("Start Configure System : \n ");

    for(ii = 0 ; ii < numOfSystemsToTest ; ii++ )
    {
        vplsSystemIndex  = systemsToTestArr[ii].vplsMode;
        currentNumOfTags = systemsToTestArr[ii].numVlanTags;

        /* AUTODOC: set configurations */
        rc = vplsDemoConfigSet1(
            /* physical ports */
            test_phyPortArr[0],    test_phyPortArr[1],    test_phyPortArr[2],    test_phyPortArr[3],
            /* eports */
            test_ePortArr[0],    test_ePortArr[1],    test_ePortArr[2],    test_ePortArr[3],
            /*evlan*/
            eVlan
            );
        if(rc != GT_OK)
        {
            PRV_UTF_LOG2_MAC("vplsDemoConfigSet1 : failed vplsMode[%d] , numTags[%d] \n",
                vplsSystemIndex, currentNumOfTags);
            return rc;
        }
    }

    /* AUTODOC: set TT rule for 'enh-UT' captured packets ... those not need to match the other TTI rules */
    rc = setTtiRuleForEnUtCapturedPackets();
    if(rc != GT_OK)
    {
        PRV_UTF_LOG0_MAC("setTtiRuleForEnUtCapturedPackets : failed \n");
        return rc;
    }

    return rc;
}

/**
* @internal prvTgfBrgVplsBasicTest1_BuildPacketsPerMode function
* @endinternal
*
* @brief   build packet for the current tested mode.
*/
static GT_STATUS prvTgfBrgVplsBasicTest1_BuildPacketsPerMode
(
    GT_VOID
)
{
    GT_U32  ii;
    GT_U32  partIndex;
    GT_U32  passengerPartIndex;
    /* build packet for the current tested mode */

    partIndex = 1;/* first vlan tag */
    if(currentNumOfTags > 0)
    {
        prvTgfPacketPartArray_port0[partIndex].type    = TGF_PACKET_PART_VLAN_TAG_E;
        prvTgfPacketPartArray_port0[partIndex].partPtr = &prvTgfPacketVlanTagPart_serviceProvider_port0;

        prvTgfPacketPartArray_port1[partIndex].type    = TGF_PACKET_PART_VLAN_TAG_E;
        prvTgfPacketPartArray_port1[partIndex].partPtr = &prvTgfPacketVlanTagPart_serviceProvider_port1;

        prvTgfPacketVlanTagPart_serviceProvider_port0.vid = getTag0ForPhyPort(0);
        prvTgfPacketVlanTagPart_serviceProvider_port1.vid = getTag0ForPhyPort(1);

        /* in 'TAG mode' the tag on the egress packet from the ETH port depends on the passenger/packet.

            if ingress from MPLS packet , the passenger hold 2 or more tags , and first 2 will be removed.
            so we left with the other tags.

            if ingress from ethernet port , the packet hold 0/1/2 tags , and those 'unmodified' will egress.

            so at this stage we not know who is the sender.
         */

    }
    else
    {
        prvTgfPacketPartArray_port0[partIndex].type    = TGF_PACKET_PART_SKIP_E;
        prvTgfPacketPartArray_port1[partIndex].type    = TGF_PACKET_PART_SKIP_E;
    }

    partIndex = 2;/* second vlan tag */
    if(currentNumOfTags > 1)
    {
        prvTgfPacketPartArray_port0[partIndex].type    = TGF_PACKET_PART_VLAN_TAG_E;
        prvTgfPacketPartArray_port0[partIndex].partPtr = &prvTgfPacketVlanTagPart_costomer;

        prvTgfPacketPartArray_port1[partIndex].type    = TGF_PACKET_PART_VLAN_TAG_E;
        prvTgfPacketPartArray_port1[partIndex].partPtr = &prvTgfPacketVlanTagPart_costomer;

        prvTgfPacketVlanTagPart_costomer.vid = getTag1ForPhyPort(0);
    }
    else
    {
        prvTgfPacketPartArray_port0[partIndex].type    = TGF_PACKET_PART_SKIP_E;
        prvTgfPacketPartArray_port1[partIndex].type    = TGF_PACKET_PART_SKIP_E;
    }


    /* build the expected passenger on the MPLS ports */
    passengerPartIndex = 1 + PASSENGER_INDEX_CNS;
    /* passenger first vlan tag */
    if(vplsSystemIndex == VPLS_DOMAIN_MODE_ETH_POP1_E ||
       vplsSystemIndex == VPLS_DOMAIN_RAW_MODE_E )
    {
        /* tag 0 not exists */
        prvTgfPacketPartArray_port2[passengerPartIndex].type    = TGF_PACKET_PART_SKIP_E;
        prvTgfPacketPartArray_port2[passengerPartIndex].partPtr = NULL;

        prvTgfPacketPartArray_port3[passengerPartIndex].type    = TGF_PACKET_PART_SKIP_E;
        prvTgfPacketPartArray_port3[passengerPartIndex].partPtr = NULL;
    }
    else /* tag mode VPLS_DOMAIN_TAG_MODE_E */
    {
        /* tag 0 exists */
        prvTgfPacketPartArray_port2[passengerPartIndex].type    = TGF_PACKET_PART_VLAN_TAG_E;
        prvTgfPacketPartArray_port2[passengerPartIndex].partPtr = &prvTgfPacketVlanTagPart_mplsTagMode_tag0_port2;

        prvTgfPacketPartArray_port3[passengerPartIndex].type    = TGF_PACKET_PART_VLAN_TAG_E;
        prvTgfPacketPartArray_port3[passengerPartIndex].partPtr = &prvTgfPacketVlanTagPart_mplsTagMode_tag0_port3;

        prvTgfPacketVlanTagPart_mplsTagMode_tag0_port2.vid = getTag0ForPhyPort(2);
        prvTgfPacketVlanTagPart_mplsTagMode_tag0_port3.vid = getTag0ForPhyPort(3);
    }
    /* passenger second vlan tag */
    passengerPartIndex++;

    if(vplsSystemIndex == VPLS_DOMAIN_RAW_MODE_E)
    {
        /* tag 1 not exists */
        prvTgfPacketPartArray_port2[passengerPartIndex].type    = TGF_PACKET_PART_SKIP_E;
        prvTgfPacketPartArray_port2[passengerPartIndex].partPtr = NULL;

        prvTgfPacketPartArray_port3[passengerPartIndex].type    = TGF_PACKET_PART_SKIP_E;
        prvTgfPacketPartArray_port3[passengerPartIndex].partPtr = NULL;
    }
    else /* pop1 mode / tag mode */
    {
        /* tag 1 exists */
        prvTgfPacketPartArray_port2[passengerPartIndex].type    = TGF_PACKET_PART_VLAN_TAG_E;
        prvTgfPacketPartArray_port2[passengerPartIndex].partPtr = &prvTgfPacketVlanTagPart_mplsTagMode_tag1_port2;

        prvTgfPacketPartArray_port3[passengerPartIndex].type    = TGF_PACKET_PART_VLAN_TAG_E;
        prvTgfPacketPartArray_port3[passengerPartIndex].partPtr = &prvTgfPacketVlanTagPart_mplsTagMode_tag1_port3;

        prvTgfPacketVlanTagPart_mplsTagMode_tag1_port2.vid = getTag1ForPhyPort(2);
        prvTgfPacketVlanTagPart_mplsTagMode_tag1_port3.vid = getTag1ForPhyPort(3);
    }

    /* passenger third,fourth vlan tag */
    passengerPartIndex++;
    if(vplsSystemIndex == VPLS_DOMAIN_TAG_MODE_E)
    {
        partIndex = 1;/* first vlan tag */
        /* tag 2 */
        prvTgfPacketPartArray_port2[passengerPartIndex] =
            prvTgfPacketPartArray_port0[partIndex];  /* taken port 0 .. but it should depends on the 'sender' (port 0 or port 1) */

        prvTgfPacketPartArray_port3[passengerPartIndex] =
            prvTgfPacketPartArray_port0[partIndex];  /* taken port 0 .. but it should depends on the 'sender' (port 0 or port 1) */

        passengerPartIndex++;
        partIndex++;
        /* tag 3 */
        prvTgfPacketPartArray_port2[passengerPartIndex] =
            prvTgfPacketPartArray_port0[partIndex];

        prvTgfPacketPartArray_port3[passengerPartIndex] =
            prvTgfPacketPartArray_port0[partIndex];

        passengerPartIndex++;
    }
    else
    {
        /* tag 2 not exists */
        prvTgfPacketPartArray_port2[passengerPartIndex].type    = TGF_PACKET_PART_SKIP_E;
        prvTgfPacketPartArray_port2[passengerPartIndex].partPtr = NULL;

        prvTgfPacketPartArray_port3[passengerPartIndex].type    = TGF_PACKET_PART_SKIP_E;
        prvTgfPacketPartArray_port3[passengerPartIndex].partPtr = NULL;

        passengerPartIndex++;

        /* tag 3 not exists */
        prvTgfPacketPartArray_port2[passengerPartIndex].type    = TGF_PACKET_PART_SKIP_E;
        prvTgfPacketPartArray_port2[passengerPartIndex].partPtr = NULL;

        prvTgfPacketPartArray_port3[passengerPartIndex].type    = TGF_PACKET_PART_SKIP_E;
        prvTgfPacketPartArray_port3[passengerPartIndex].partPtr = NULL;

        passengerPartIndex++;
    }

    /* now take the 'payload' of the passenger */
    prvTgfPacketPartArray_port2[passengerPartIndex].type    = TGF_PACKET_PART_PAYLOAD_E;
    prvTgfPacketPartArray_port2[passengerPartIndex].partPtr = &prvTgfPacketPayloadPart;

    prvTgfPacketPartArray_port3[passengerPartIndex].type    = TGF_PACKET_PART_PAYLOAD_E;
    prvTgfPacketPartArray_port3[passengerPartIndex].partPtr = &prvTgfPacketPayloadPart;



    prvTgfPacketMplsLabelPart_mplsDomain.label = getTsMplsLabel(2);

    prvTgfPacketMplsLabelPart_mplsDomain.timeToLive = timeToLive;
    prvTgfPacketMplsLabelPart_mplsDomain.stack = 0;
    prvTgfPacketMplsLabelPart_mplsDomain.exp = expTs;

    ii = 2;
    prvTgfPacketMplsLabelPart_pwPort2.label = pwLabels[ii];
    prvTgfPacketMplsLabelPart_pwPort2.timeToLive = (GT_U8)pwTtl[ii];
    prvTgfPacketMplsLabelPart_pwPort2.stack = 1;
    prvTgfPacketMplsLabelPart_pwPort2.exp = (GT_U8)pwExp[ii];

    ii = 3;
    prvTgfPacketMplsLabelPart_pwPort3.label = pwLabels[ii];
    prvTgfPacketMplsLabelPart_pwPort3.timeToLive = (GT_U8)pwTtl[ii];
    prvTgfPacketMplsLabelPart_pwPort3.stack = 1;
    prvTgfPacketMplsLabelPart_pwPort3.exp = (GT_U8)pwExp[ii];

    return GT_OK;
}


/*******************************************************************************
* vplsDemoConfigSet1_restore
*
* DESCRIPTION:
*       restore configurations.
********************************************************************************/
/*NOT static*/GT_STATUS vplsDemoConfigSet1_restore(void)
{
    /* set e2phy */
    CHECK_RC_MAC(
    setE2Phy_restore());

    /* set tpid */
    CHECK_RC_MAC(
    setTpid_restore());

    /* set egress tagging */
    CHECK_RC_MAC(
    setEgressTagging_restore());

    /* set egress VPLS tunnel */
    CHECK_RC_MAC(
    setEgressVplsTunnel_restore());

    /* set ingress Service providers recognition (using TTI rule (no termination))*/
    CHECK_RC_MAC(
    setIngressServiceProvidersRecognition_restore());

    /* set ingress VPLS tunnel termination*/
    CHECK_RC_MAC(
    setIngressVplsTunnelTermination_restore());

    /*set bridge */
    CHECK_RC_MAC(
    setBridge_restore());

    /* set L2Mll */
    CHECK_RC_MAC(
    setL2Mll_restore());

    return GT_OK;
}

/**
* @internal prvTgfBrgVplsBasicTest1_configRestore function
* @endinternal
*
*/
static GT_VOID prvTgfBrgVplsBasicTest1_configRestore(GT_VOID)
{
    GT_STATUS   rc;

    GT_U32  ii;

    for(ii = 0 ; ii < numOfSystemsToTest ; ii++ )
    {
        vplsSystemIndex  = systemsToTestArr[ii].vplsMode;
        currentNumOfTags = systemsToTestArr[ii].numVlanTags;
        /* set configurations */
        rc = vplsDemoConfigSet1_restore();
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vplsDemoConfigSet1_restore");
    }

    rc = setTtiRuleForEnUtCapturedPackets_restore();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "setTtiRuleForEnUtCapturedPackets_restore");

    PRV_UTF_LOG0_MAC("reset ETH counters \n");
    prvTgfEthCountersReset(prvTgfDevNum);

    PRV_UTF_LOG0_MAC("flush FDB include static entries \n");
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d", prvTgfDevNum, GT_TRUE);

}


/**
* @internal setCapture function
* @endinternal
*
*/
static GT_VOID setCapture
(
    IN GT_U32    portIndex,
    IN GT_BOOL enable
)
{
    GT_STATUS   rc;
    CPSS_INTERFACE_INFO_STC portInterface;

    portInterface.type = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum = prvTgfDevNum;
    portInterface.devPort.portNum = prvTgfPortsArray[portIndex];

    if(enable == GT_TRUE)
    {
        /* clear table */
        rc = tgfTrafficTableRxPcktTblClear();
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");
    }

    /* Start capturing Tx packets */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, enable);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorPortTxEthCaptureSet: %d, %d, %d",
                                 portInterface.devPort.hwDevNum, portInterface.devPort.portNum, enable);
}

/**
* @internal checkCapturedPacket function
* @endinternal
*
*/
static GT_VOID checkCapturedPacket
(
    IN GT_U32    senderPortIndex,
    IN GT_U32    egressPortIndex
)
{
    GT_STATUS   rc;
    CPSS_INTERFACE_INFO_STC portInterface;
    GT_U32  actualCapturedNumOfPackets = 0;
    GT_U32  numOfExpectedPackets = prvTgfBurstCount;
    TGF_PACKET_STC      *packetInfoPtr_egress;
    TGF_PACKET_STC      *packetInfoPtr_sender;
    GT_U32  passengerIndex_sender;
    GT_U32  passengerIndex_egress;
    TGF_PACKET_L2_STC *partL2Ptr_sender;
    TGF_PACKET_L2_STC *partL2Ptr_egressOrig;

    packetInfoPtr_sender  = sendPacketsArr[senderPortIndex];
    packetInfoPtr_egress  = sendPacketsArr[egressPortIndex];

    passengerIndex_sender = sendPacketsWithPassengerIndexArr[senderPortIndex];
    passengerIndex_egress = sendPacketsWithPassengerIndexArr[egressPortIndex];

    /* AUTODOC: the expected captured packet :
        if received on TS port :
            if sent from non-TS.
                need to set passenger MAC SA,DA according to sender
            else // TS sender
                need to set passenger MAC SA,DA according to passenger of sender
        else // received on NON TS port
            if sent from non-TS.
                need to set MAC SA,DA according to sender
            else // TS sender
                need to set MAC SA,DA according to passenger of sender
    */
    partL2Ptr_sender = (TGF_PACKET_L2_STC*)packetInfoPtr_sender->partsArray[passengerIndex_sender].partPtr;
    if(packetInfoPtr_sender->partsArray[passengerIndex_sender].type != TGF_PACKET_PART_L2_E)
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, GT_BAD_STATE,
                                     "sender wrong passengerIndex \n");
        return;
    }

    partL2Ptr_egressOrig = (TGF_PACKET_L2_STC*)packetInfoPtr_egress->partsArray[passengerIndex_egress].partPtr;
    if(packetInfoPtr_egress->partsArray[passengerIndex_egress].type != TGF_PACKET_PART_L2_E)
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, GT_BAD_STATE,
                                     "egress wrong passengerIndex \n");
        return;
    }

    /* use sender info into egress info */
    packetInfoPtr_egress->partsArray[passengerIndex_egress].partPtr = partL2Ptr_sender;
    if(passengerIndex_egress)
    {
        /* expect mac DA of TS */
        cpssOsMemCpy(prvTgfPacketL2Part_dummyTs.daMac,
                     tsMacDaArr[egressPortIndex].arEther,
                     6);

        /* expect unique mac SA according to egress port */
        cpssOsMemCpy(prvTgfPacketL2Part_dummyTs.saMac,
                    tsMacSaArr[egressPortIndex].arEther,
                     6);
    }

    if(vplsSystemIndex == VPLS_DOMAIN_TAG_MODE_E &&
       senderPortIndex < 2)
    {
        /* passenger third,fourth vlan tag */
        GT_U32 passengerPartIndex = 3/*after L2 + 2 tags */ + PASSENGER_INDEX_CNS;
        GT_U32 partIndex = 1 + passengerIndex_sender;/* first vlan tag */

        if(egressPortIndex >= 2)/*MPLS ports*/
        {
            /* tag 2 */
            prvTgfPacketPartArray_port2[passengerPartIndex] =
                senderPortIndex == 0 ?
                prvTgfPacketPartArray_port0[partIndex] :  /*port 0*/
                prvTgfPacketPartArray_port1[partIndex] ;  /*port 1*/

            prvTgfPacketPartArray_port3[passengerPartIndex] =
                senderPortIndex == 0 ?
                prvTgfPacketPartArray_port0[partIndex] :  /*port 0*/
                prvTgfPacketPartArray_port1[partIndex] ;  /*port 1*/

            passengerPartIndex++;
            partIndex++;
            /* tag 3 */
            prvTgfPacketPartArray_port2[passengerPartIndex] =
                senderPortIndex == 0 ?
                prvTgfPacketPartArray_port0[partIndex] :  /*port 0*/
                prvTgfPacketPartArray_port1[partIndex] ;  /*port 1*/

            prvTgfPacketPartArray_port3[passengerPartIndex] =
                senderPortIndex == 0 ?
                prvTgfPacketPartArray_port0[partIndex] :  /*port 0*/
                prvTgfPacketPartArray_port1[partIndex] ;  /*port 1*/
        }
        else  /* egress is ethernet port */
        {
            /* in 'TAG mode' the tag on the egress packet from the ETH port depends on the passenger/packet.

                if ingress from MPLS packet , the passenger hold 2 or more tags , and first 2 will be removed.
                so we left with the other tags.

                if ingress from ethernet port , the packet hold 0/1/2 tags , and those 'unmodified' will egress.
             */
            partIndex = 1 + passengerIndex_sender;/* first vlan tag */

            if(senderPortIndex == 1)
            {
                /* copy from port 1 to port 0 */
                prvTgfPacketPartArray_port0[partIndex] =
                    prvTgfPacketPartArray_port1[partIndex] ;  /*port 1*/
                /*second vlan tag */
                partIndex++;
                prvTgfPacketPartArray_port0[partIndex] =
                    prvTgfPacketPartArray_port1[partIndex] ;  /*port 1*/
            }
            else /*senderPortIndex == 0*/
            {
                /* copy from port 0 to port 1 */
                prvTgfPacketPartArray_port1[partIndex] =
                    prvTgfPacketPartArray_port0[partIndex] ;  /*port 0*/
                /*second vlan tag */
                partIndex++;
                prvTgfPacketPartArray_port1[partIndex] =
                    prvTgfPacketPartArray_port0[partIndex] ;  /*port 0*/
            }
        }
    }

    portInterface.type = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum = prvTgfDevNum;
    portInterface.devPort.portNum = prvTgfPortsArray[egressPortIndex];

    /* reset the previous value */
    actualCapturedNumOfPackets = 0;

    PRV_UTF_LOG1_MAC("Port [%d] capturing on physical port:\n", prvTgfPortsArray[egressPortIndex]);

    /* print captured packets and check TriggerCounters */
    rc = tgfTrafficGeneratorPortTxEthCaptureCompare(
            &portInterface,
            packetInfoPtr_egress,
            numOfExpectedPackets,/*numOfPackets*/
            0/*vfdNum*/,
            NULL /*vfdArray*/,
            NULL, /* bytesNum's skip list */
            0,    /* length of skip list */
            &actualCapturedNumOfPackets,
            NULL/*onFirstPacketNumTriggersBmpPtr*/);
    rc = rc == GT_NO_MORE ? GT_OK : rc;
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                 "tgfTrafficGeneratorPortTxEthTriggerCountersGet:"
                                 " port = %d, rc = 0x%02X\n", portInterface.devPort.portNum, rc);

    /* restore egress packet L2 part */
    packetInfoPtr_egress->partsArray[passengerIndex_egress].partPtr = partL2Ptr_egressOrig;
}

/**
* @internal buildTsReply function
* @endinternal
*
*/
static GT_VOID buildTsReply
(
    IN GT_U32    tsPortIndex,/* the TS port that need to replay */
    IN GT_U32    senderPortIndex /* the port that need to get reply (from the TS port) */
)
{
    TGF_PACKET_STC      *packetInfoPtr_ts;
    TGF_PACKET_STC      *packetInfoPtr_sender;
    GT_U32  passengerIndex_sender;
    GT_U32  passengerIndex_ts;
    TGF_PACKET_L2_STC *partL2Ptr_sender;

    packetInfoPtr_sender  = sendPacketsArr[senderPortIndex];
    packetInfoPtr_ts  = sendPacketsArr[tsPortIndex];

    passengerIndex_sender = sendPacketsWithPassengerIndexArr[senderPortIndex];
    passengerIndex_ts = sendPacketsWithPassengerIndexArr[tsPortIndex];

    /* AUTODOC : the expected 'reply' from TS ports should be similar to the packets that we
       would have send 'to them from 'sender port' , with difference of 'swapped'
       mac SA,DA (in tunnel and in passenger) */

    /* the mac DA,SA of sender */
    partL2Ptr_sender = (TGF_PACKET_L2_STC*)packetInfoPtr_sender->partsArray[passengerIndex_sender].partPtr;
    if(packetInfoPtr_sender->partsArray[passengerIndex_sender].type != TGF_PACKET_PART_L2_E)
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, GT_BAD_STATE,
                                     "sender wrong passengerIndex \n");
        return;
    }

    if(packetInfoPtr_ts->partsArray[passengerIndex_ts].type != TGF_PACKET_PART_L2_E)
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, GT_BAD_STATE,
                                     "egress wrong passengerIndex \n");
        return;
    }

    /* build swapped mac SA DA of passenger packet */
    cpssOsMemCpy(newPartL2PassengerArr[tsPortIndex].saMac,partL2Ptr_sender->daMac,sizeof(TGF_MAC_ADDR));
    cpssOsMemCpy(newPartL2PassengerArr[tsPortIndex].daMac,partL2Ptr_sender->saMac,sizeof(TGF_MAC_ADDR));

    /* change the L2 info for passenger */
    packetInfoPtr_ts->partsArray[passengerIndex_ts].partPtr = &newPartL2PassengerArr[tsPortIndex];

    /* swap mac SA,DA of the sending TS */
    /* expect mac DA of TS */
    cpssOsMemCpy(prvTgfPacketL2Part_dummyTs.daMac,
                 tsMacSaArr[tsPortIndex].arEther,
                 6);
    /* expect mac DA of TS */
    cpssOsMemCpy(prvTgfPacketL2Part_dummyTs.saMac,
                 tsMacDaArr[tsPortIndex].arEther,
                 6);

}


/**
* @internal checkFdb function
* @endinternal
*
*/
static GT_VOID checkFdb
(
    IN GT_U32    senderPortIndex,
    IN GT_BOOL    isSingle
)
{
    GT_STATUS   rc;
    GT_U32  numOfValidMinusSkip;
    GT_U32  numOfValid;
    GT_U32  numOfSkip;
    GT_U32  numOfStormPrevention;
    PRV_TGF_MAC_ENTRY_KEY_STC  searchKey;
    TGF_PACKET_STC      *packetInfoPtr_sender;
    GT_U32  passengerIndex_sender;
    TGF_PACKET_L2_STC *partL2Ptr_sender;
    GT_U32                       index;
    GT_BOOL                      aged;
    GT_HW_DEV_NUM                hwDevNum;
    PRV_TGF_BRG_MAC_ENTRY_STC    macEntry;

    if(prvTgfFdbShadowUsed == GT_TRUE)
    {
        /* do not count the HW */
    }
    else
    if(isSingle == GT_TRUE)
    {
        /* get FDB counters */
        rc = prvTgfBrgFdbPortGroupCount(prvTgfDevNum,0,&numOfValid,&numOfSkip,NULL,&numOfStormPrevention,NULL);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

        /* should be single entry */
        numOfValidMinusSkip = numOfValid - numOfSkip - numOfStormPrevention;
        UTF_VERIFY_EQUAL1_STRING_MAC(1, numOfValidMinusSkip,
            "Expect single FDB entry but got[%d] \n",numOfValidMinusSkip);
    }

    passengerIndex_sender = sendPacketsWithPassengerIndexArr[senderPortIndex];
    packetInfoPtr_sender = sendPacketsArr[senderPortIndex];

    partL2Ptr_sender = (TGF_PACKET_L2_STC*)packetInfoPtr_sender->partsArray[passengerIndex_sender].partPtr;

    /* get the SA of the sender (passenger used for learning) */
    searchKey.entryType = PRV_TGF_FDB_ENTRY_TYPE_MAC_ADDR_E;
    searchKey.key.macVlan.vlanId = getEvlanForVplsDomain();
    cpssOsMemCpy(searchKey.key.macVlan.macAddr.arEther,
                 partL2Ptr_sender->saMac,
                 6);

    /****************************/
    /* next 2 lines are WA to :
       [UTF]: Memory leak detected in Falcon 6.4T device

       this is because : the 'find' will cause 'numOfTiles' registrations of 'prvTgfListAddAfter'
       but after the first response from the device of 'QR' it will detach the
       'CB' that should call to prvTgfListDelItem
    */
    /****************************/
    usePortGroupsBmp  = GT_TRUE;
    currPortGroupsBmp = 1;

    /* query the FDB about the entry */
    rc = prvTgfBrgFdbMacEntryFind(&searchKey,&index,&aged,&hwDevNum,&macEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,"The FDB entry was not found \n");

    /* restore the value */
    usePortGroupsBmp = GT_FALSE;
}

/* expected counters : all counters should be ZERO */
static CPSS_PORT_MAC_COUNTER_SET_STC   allZeroCntrs;
/**
* @internal checkGotNoPacket function
* @endinternal
*
*/
static GT_VOID checkGotNoPacket
(
    IN GT_U32 egressPortIndex
)
{
    GT_STATUS   rc;
    GT_BOOL                         isEqualCntrs;
    CPSS_PORT_MAC_COUNTER_SET_STC   portCntrs;/* current counters of the port */

    rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[egressPortIndex], GT_TRUE, &portCntrs);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[egressPortIndex]);

    /* compare the counters */
    PRV_TGF_VERIFY_COUNTERS_MAC(isEqualCntrs, allZeroCntrs, portCntrs);
    UTF_VERIFY_EQUAL0_STRING_MAC(isEqualCntrs, GT_TRUE, "get another counters values.");

    /* print expected values if not equal */
    PRV_TGF_PRINT_DIFF_COUNTERS_MAC(isEqualCntrs, allZeroCntrs, portCntrs);

}

/**
* @internal sendPacket function
* @endinternal
*
*/
static GT_VOID sendPacket
(
    IN GT_U32 senderPortIndex
)
{
    GT_STATUS   rc;
    TGF_PACKET_STC      *packetInfoPtr_sender = sendPacketsArr[senderPortIndex];

    /* setup transmit params */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, packetInfoPtr_sender,
        prvTgfBurstCount, 0, NULL);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfSetTxSetupEth: %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[senderPortIndex]);

    /* start transmitting */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[senderPortIndex]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfStartTransmitingEth: %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[senderPortIndex]);

}


/**
* @internal sendFlood function
* @endinternal
*
*/
static GT_VOID sendFlood
(
    IN GT_U32 senderPortIndex,
    IN GT_U32 bmpFilteredPortIndexBmp
)
{
    GT_U32  ii;
    GT_U32  actualSendNum = 0;


    /* AUTODOC: we check flood , and need to capture the egress packets on all egress ports */
    for(ii = 0 ; ii < numOfPorts ; ii++)
    {
        if(ii == senderPortIndex)
        {
            /* AUTODOC:skip checking the sender port */
            continue;
        }

        /* AUTODOC: start capture + clear captured table */
        PRV_UTF_LOG0_MAC("start capture + clear captured table \n");
        setCapture(ii,GT_TRUE);

        /* AUTODOC: send packet */
        PRV_UTF_LOG0_MAC("send packet \n");
        sendPacket(senderPortIndex);

        /* AUTODOC: stop capture */
        PRV_UTF_LOG0_MAC("stop capture \n");
        setCapture(ii,GT_FALSE);

        actualSendNum++;

        if(0 == (bmpFilteredPortIndexBmp & (BIT_0 << ii)))
        {
            /* AUTODOC: check that egress as expected */
            PRV_UTF_LOG0_MAC("check that egress as expected \n");
            checkCapturedPacket(senderPortIndex, ii);
        }
        else
        {
            PRV_UTF_LOG0_MAC("check the filtered port got no packet \n");
            /* AUTODOC: check the filtered port got no packet */
            checkGotNoPacket(ii);
        }

        /* AUTODOC: reset ETH counters */
        PRV_UTF_LOG0_MAC("reset ETH counters \n");
        prvTgfEthCountersReset(prvTgfDevNum);


        if(debug_on && debug_numIterationsOnFlood <= actualSendNum)
        {
            /* no more iterations */
            break;
        }
    }

    PRV_UTF_LOG0_MAC("check that FDB hold single entry on proper eport , proper evlan \n");
    /* AUTODOC: check that FDB hold single entry on proper eport , proper evlan */
    checkFdb(senderPortIndex,GT_TRUE/*single entry*/);
}

/**
* @internal prvTgfBrgVplsBasicTest1_FromServiceProvider_flood function
* @endinternal
*
*/
static GT_VOID prvTgfBrgVplsBasicTest1_FromServiceProvider_flood(GT_VOID)
{
    sendFlood(SERVICE_PROVIDER_PORT_SENDER_INDEX_CNS/*sender*/,
            0/* no filters*/);
}

/**
* @internal prvTgfBrgVplsBasicTest1_FromMplsDomain_flood function
* @endinternal
*
*/
static GT_VOID prvTgfBrgVplsBasicTest1_FromMplsDomain_flood(GT_VOID)
{
    sendFlood(VPLS_DOMAIN_PORT_SENDER_INDEX_2_CNS/*sender*/,
              (BIT_0 << VPLS_DOMAIN_PORT_SENDER_INDEX_3_CNS)/*filtered*/);
}


/**
* @internal sendKnowUc function
* @endinternal
*
*/
static GT_VOID sendKnowUc
(
    IN GT_U32 senderPortIndex,
    IN GT_U32 targetPortIndex
)
{
    GT_U32  ii;

    /* AUTODOC: start capture + clear captured table */
    PRV_UTF_LOG0_MAC("start capture + clear captured table \n");
    setCapture(targetPortIndex,GT_TRUE);

    /* AUTODOC: send packet */
    PRV_UTF_LOG0_MAC("send packet \n");
    sendPacket(senderPortIndex);

    /* AUTODOC: stop capture */
    PRV_UTF_LOG0_MAC("stop capture \n");
    setCapture(targetPortIndex,GT_FALSE);

    /* AUTODOC: check that egress as expected (on target port) */
    PRV_UTF_LOG0_MAC("check that egress as expected (on target port) \n");
    checkCapturedPacket(senderPortIndex, targetPortIndex);

    /* AUTODOC: we check no flood on other ports */
    for(ii = 0 ; ii < numOfPorts ; ii++)
    {
        if(ii == senderPortIndex ||
            ii == targetPortIndex)
        {
            /* AUTODOC: skip checking the sender port */
            /* AUTODOC: skip checking the target port */
            continue;
        }

        /* AUTODOC: check this port got no packet */
        PRV_UTF_LOG0_MAC("check this port got no packet \n");
        checkGotNoPacket(ii);
    }

    /* AUTODOC: reset ETH counters */
    PRV_UTF_LOG0_MAC("reset ETH counters \n");
    prvTgfEthCountersReset(prvTgfDevNum);
}

/**
* @internal prvTgfBrgVplsBasicTest1_FromServiceProvider_knownUc function
* @endinternal
*
*/
static GT_VOID prvTgfBrgVplsBasicTest1_FromServiceProvider_knownUc(GT_VOID)
{
    sendKnowUc(SERVICE_PROVIDER_PORT_SENDER_INDEX_CNS/*sender*/,
        VPLS_DOMAIN_PORT_SENDER_INDEX_2_CNS/*target*/);
}

/**
* @internal prvTgfBrgVplsBasicTest1_FromMplsDomain_knownUc function
* @endinternal
*
*/
static GT_VOID prvTgfBrgVplsBasicTest1_FromMplsDomain_knownUc(IN GT_U32  senderPortIndex)
{
    sendKnowUc(senderPortIndex /*sender*/,
        SERVICE_PROVIDER_PORT_SENDER_INDEX_CNS/*target*/);
}


/**
* @internal prvTgfBrgVplsBasicTest1_currentMode function
* @endinternal
*
* @brief   test current mode.
*/
static GT_VOID prvTgfBrgVplsBasicTest1_currentMode
(
    GT_VOID
)
{
    GT_STATUS   rc;
    GT_U32  errorNum;

    utfGeneralStateMessageSave(0,"vplsSystemIndex = %d",vplsSystemIndex);
    utfGeneralStateMessageSave(1,"currentNumOfTags = %d",currentNumOfTags);

    /* reset the number of errors in DB by calling utfErrorCountGet() */
    errorNum = utfErrorCountGet();
    errorNum = 0;

    /*AUTODOC: test current mode */
    PRV_UTF_LOG1_MAC("test current mode : vplsMode[%s] \n",
        vplsDomainModeNames[vplsSystemIndex]);
    PRV_UTF_LOG1_MAC("Description : %s \n",
        vplsDomainModeDescriptions[vplsSystemIndex]);
    PRV_UTF_LOG2_MAC("numVlanTags[%d] (expect [%d] vlan tags on ETH port) \n",
        currentNumOfTags,currentNumOfTags);

    /*AUTODOC: build packets for current mode */
    prvTgfBrgVplsBasicTest1_BuildPacketsPerMode();

    /*AUTODOC: reset ETH counters (also set link to up on tested ports) */
    PRV_UTF_LOG0_MAC("reset ETH counters (also set link to up on tested ports)\n");
    prvTgfEthCountersReset(prvTgfDevNum);

    if(debug_on == 0 ||(debug_on && debug_doFloodFromServicePort))
    {
        /*AUTODOC: 1. test check 'flooding' from service provider port to the other 3 ports.(TS to the MPLS domain) */
        /*AUTODOC:    a. check SA learning (reassigned src eport) */
        PRV_UTF_LOG0_MAC("1. test check 'flooding' from service provider port to the other 3 ports.(TS to the MPLS domain)\n");
        PRV_UTF_LOG0_MAC("   a. check SA learning (reassigned src eport) \n");

        utfGeneralStateMessageSave(2,"case 1: test check 'flooding' from service provider port");

        prvTgfBrgVplsBasicTest1_FromServiceProvider_flood();

        errorNum += utfErrorCountGet();
        if(debug_on == 0 ||(debug_on && errorNum == 0))
        {
            /*AUTODOC: flush FDB include static entries (to allow flood in next stage) */
            PRV_UTF_LOG0_MAC("flush FDB include static entries  (to allow flood in next stage) \n");
            /* flush FDB include static entries */
            rc = prvTgfBrgFdbFlush(GT_TRUE);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d", prvTgfDevNum, GT_TRUE);
        }
    }

    /* AUTODOC: prepare TS reply if TS ports want to 'reply' to the sender ... */
    buildTsReply(VPLS_DOMAIN_PORT_SENDER_INDEX_2_CNS/*2*/,
                 SERVICE_PROVIDER_PORT_SENDER_INDEX_CNS/*1*/);

    if(debug_on == 0 ||(debug_on && debug_doFloodFromVplsDomain))
    {
        utfGeneralStateMessageSave(2,"case 2: test check 'flooding' from mpls domain port");
        /* AUTODOC: 2. test check 'flooding' from mpls domain port only to service provider ports (filter the other MPLS domain port). (TT of ingress packet - Ethernet passenger) */
        /* AUTODOC:    a. check SA learning (reassigned src eport) */
        PRV_UTF_LOG0_MAC("2. test check 'flooding' from mpls domain port only to service provider ports (filter the other MPLS domain port). (TT of ingress packet - Ethernet passenger) \n");
        PRV_UTF_LOG0_MAC("   a. check SA learning (reassigned src eport) \n");
        prvTgfBrgVplsBasicTest1_FromMplsDomain_flood();
    }

    if(debug_on == 0 ||(debug_on && debug_doUcFromServicePort))
    {
        utfGeneralStateMessageSave(2,"case 3: test 'Known UC' from service provider port to MPLS domain.");
        /* AUTODOC: 3. test 'Known UC' from service provider port to MPLS domain. */
        PRV_UTF_LOG0_MAC("3. test 'Known UC' from service provider port to MPLS domain. \n");
        prvTgfBrgVplsBasicTest1_FromServiceProvider_knownUc();
    }

    if(debug_on == 0 ||(debug_on && debug_doUcFromVplsDomain))
    {
        utfGeneralStateMessageSave(2,"case 4: test 'Known UC' from MPLS domain port to service provider port");
        /* AUTODOC: 4. test 'Known UC' from MPLS domain port to service provider port. */
        PRV_UTF_LOG0_MAC("4. test 'Known UC' from MPLS domain port to service provider port. \n");
        prvTgfBrgVplsBasicTest1_FromMplsDomain_knownUc(VPLS_DOMAIN_PORT_SENDER_INDEX_2_CNS);
    }

    if(debug_on == 0 ||(debug_on && debug_doUcFromVplsDomain))
    {
        /* AUTODOC: prepare TS reply if TS ports want to 'reply' to the sender ... */
        buildTsReply(VPLS_DOMAIN_PORT_SENDER_INDEX_3_CNS/*3*/,
                     SERVICE_PROVIDER_PORT_SENDER_INDEX_CNS/*1*/);

        utfGeneralStateMessageSave(2,"case 5: test 'Known UC' from MPLS other domain port to service provider port.");
        /* AUTODOC: 5. test 'Known UC' from MPLS other domain port to service provider port. */
        PRV_UTF_LOG0_MAC("5. test 'Known UC' from MPLS other domain port to service provider port. \n");
        prvTgfBrgVplsBasicTest1_FromMplsDomain_knownUc(VPLS_DOMAIN_PORT_SENDER_INDEX_3_CNS);
    }

    utfGeneralStateMessageSave(2,"flush fdb,reset counters");

    PRV_UTF_LOG0_MAC("reset ETH counters \n");
    prvTgfEthCountersReset(prvTgfDevNum);

    PRV_UTF_LOG0_MAC("flush FDB include static entries \n");
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d", prvTgfDevNum, GT_TRUE);

    utfGeneralStateMessageSave(2,NULL);

    PRV_UTF_LOG0_MAC("prvTgfBrgVplsBasicTest1 : ended \n");
}

/**
* @internal prvTgfBrgVplsBasicTest1 function
* @endinternal
*
* @brief   similar to file prvTgfBrgVplsBasicTest.c but with next changes:
*         1. supports 3 modes :
*         a. 'pop tag' - the mode that is tested in prvTgfBrgVplsBasicTest.c
*         b. 'raw mode' + 'QinQ terminal'(delete double Vlan)
*         c. 'tag mode' + 'add double vlan tag'
*         2. in RAW mode ethernet packets come with 2 tags and the passenger on MPLS
*         tunnels is without vlan tags.
*         3. in TAG mode the ethernet packets come with one tag but considered untagged
*         when become passenger on MPLS tunnels and so added additional 2 vlan tags.
*/
GT_VOID prvTgfBrgVplsBasicTest1
(
    GT_VOID
)
{
    GT_STATUS   rc;
    GT_U32  ii;
    swDevNum = prvTgfDevNum;

    PRV_UTF_LOG0_MAC("prvTgfBrgVplsBasicTest1 : started \n");
    /*AUTODOC: do ALL configurations before send traffic from any port
      NOTE: configure all systems before sending any packet.
      once packets are sent , no change in configuration is done
      (except 'capture' that uses mirroring)
    */
    rc = prvTgfBrgVplsBasicTest1_config();
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVplsBasicTest1_config in line [%d]" , errorLine);

    /*AUTODOC: loop on all 'systems' to test packets on.
     'each system' is in deferent eVlan --> different VPLS domain
     but all systems coexist.
     */
    for(ii = 0 ; ii < numOfSystemsToTest ; ii++ )
    {
        vplsSystemIndex  = systemsToTestArr[ii].vplsMode;
        currentNumOfTags = systemsToTestArr[ii].numVlanTags;

        if(debug_vplsSystemIndex_Enabled &&
           debug_vplsSystemIndex != (GT_U32)(vplsSystemIndex))
        {
            /* skip non debugged modes */
            continue;
        }

        prvTgfBrgVplsBasicTest1_currentMode();
    }


    /* AUTODOC: restore configuration */
    prvTgfBrgVplsBasicTest1_configRestore();

    PRV_UTF_LOG0_MAC("prvTgfBrgVplsBasicTest1 : ended \n");

}



/* debug indications */
GT_VOID prvTgfBrgVplsBasicTest1_debugIndications
(
    IN GT_U32   numIterationsOnFlood,   /* for part 1,2 - number of iterations of captures */
    IN GT_U32   doFloodFromServicePort, /* part 1 */
    IN GT_U32   doFloodFromVplsDomain,  /* part 2 */
    IN GT_U32   doUcFromServicePort,    /* part 3 */
    IN GT_U32   doUcFromVplsDomain      /* part 4 */
)
{
    debug_on = 1;

    debug_numIterationsOnFlood    =    numIterationsOnFlood    ;
    debug_doFloodFromServicePort  =    doFloodFromServicePort  ;
    debug_doFloodFromVplsDomain   =    doFloodFromVplsDomain   ;
    debug_doUcFromServicePort     =    doUcFromServicePort     ;
    debug_doUcFromVplsDomain      =    doUcFromVplsDomain      ;

    return;
}

/* debug indications */
GT_VOID prvTgfBrgVplsBasicTest1_debugSystem
(
    IN GT_U32   systemIndex
)
{
    debug_vplsSystemIndex_Enabled =  systemIndex >= VPLS_DOMAIN_LAST ? 0 : 1;
    debug_vplsSystemIndex = systemIndex;

    return;
}
#endif /* CHX_FAMILY*/


