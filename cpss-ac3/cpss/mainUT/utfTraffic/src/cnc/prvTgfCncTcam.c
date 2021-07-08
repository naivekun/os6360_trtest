/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvTgfCncGen.c
*
* DESCRIPTION:
*     CPSS Centralized counters (CNC) TCAM related Clients.
*
* FILE REVISION NUMBER:
*       $Revision: 4 $
*
*******************************************************************************/

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
#include <common/tgfTcamGen.h>
#include <common/tgfTunnelGen.h>
#include <cnc/prvTgfCncGen.h>
#include <cnc/prvTgfCncTcam.h>


/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

/* client group used in test */
#define PRV_TGF_CNS_CLIENT_GROUP_CNS 2

/* size of TCAM floor size in standard (30 bytes) rules */
#define PRV_TGF_CNS_TCAM_FLOOR_STD_RULES_CNS 1024

/* PCL rule relative index for tests - 1024 standard rules on floor.
   Devices with 3 floors use only two first floors
   */
#define PCL_RULE_INDEX_HIT_0_CNS          (1)
#define PCL_RULE_INDEX_HIT_1_CNS          (515)
#define PCL_RULE_INDEX_HIT_2_CNS          (1025)
#define PCL_RULE_INDEX_HIT_3_CNS          (1539)

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS         1

/* port number to receive traffic from */
#define PRV_TGF_RECEIVE_PORT_IDX_CNS      2

/* VLAN Id */
#define PRV_TGF_VLAN_ID_CNS               5

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part =
{
    {0x00, 0x12, 0x34, 0x56, 0x78, 0x9A},                /* daMac */
    {0x00, 0x11, 0x22, 0x33, 0x44, 0x55}                 /* saMac */
};

/* VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTagPart =
{
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,    /* etherType */
    1, 0, PRV_TGF_VLAN_ID_CNS      /* pri, cfi, VlanId */
};

/* ethertype part of IPV4 packet */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketIpv4EtherTypePart =
{
    TGF_ETHERTYPE_0800_IPV4_TAG_CNS
};

/* Payload of packet */
static GT_U8 prvTgfPacketPayloadDataArr[] =
{
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
    0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
    0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F
};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart = {
    sizeof(prvTgfPacketPayloadDataArr),            /* dataLength */
    prvTgfPacketPayloadDataArr                     /* dataPtr */
};

/* packet's IPv4 */
static TGF_PACKET_IPV4_STC prvTgfPacketIpv4OtherHeaderPart =
{
    4,                                                               /* version */
    (TGF_IPV4_HEADER_SIZE_CNS / 4),                                  /* headerLen */
    (GT_U8)(33 << 2),                                                /* typeOfService */
    (sizeof(prvTgfPacketPayloadDataArr)),                            /* totalLen */
    0,                                                               /* id */
    4,                                                               /* flags */
    0,                                                               /* offset */
    0x40,                                                            /* timeToLive */
    4,                                                               /* protocol */
    TGF_PACKET_AUTO_CALC_CHECKSUM_CNS,                               /* csum */
    {22, 22, 22, 22},                                                /* srcAddr */
    { 1,  1,  1,  3}                                                 /* dstAddr */
};

/* Tagged IPV4 Other Packet Parts Array */
static TGF_PACKET_PART_STC prvTgfPacketIpv4OthPartsArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketIpv4EtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacketIpv4OtherHeaderPart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* Tagged IPV4 Other Packet Info */
static TGF_PACKET_STC prvTgfPacketIpv4OthInfo = {
    (TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS
     + TGF_IPV4_HEADER_SIZE_CNS + sizeof(prvTgfPacketPayloadDataArr)),   /* totalLen */
    sizeof(prvTgfPacketIpv4OthPartsArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketIpv4OthPartsArray                                        /* partsArray */
};

/******************************************************************************\
 *                           Private declarations                             *
\******************************************************************************/

/******************************************************************************\
 *                            Private test functions                          *
\******************************************************************************/

/**
* @struct PRV_TGF_CNC_TCAM_FLOORS_CFG_STC
 *
 * @brief The configuration of 4 TCAM Floors access.
*/
typedef struct
{
    GT_BOOL    clientEnableArr[5];
    GT_U32     clientGroupArr[5];
    struct
    {
        PRV_TGF_TCAM_BLOCK_INFO_STC floorInfoArr[PRV_TGF_TCAM_MAX_TCAM_BLOCKS_CNS];
    } floor[CPSS_DXCH_TCAM_MAX_NUM_FLOORS_CNS];
} PRV_TGF_CNC_TCAM_FLOORS_CFG_STC;

/* used to restore configuration */
/* read once, restore as many times as needed */
static GT_BOOL saveFloorsCfgValid = GT_FALSE;
static PRV_TGF_CNC_TCAM_FLOORS_CFG_STC saveFloorsCfg;
static GT_U32 saveTtiPclId;

typedef struct
{
    PRV_TGF_CNC_CLIENT_ENT client;
    GT_U32                 cncBlock;
    GT_U32                 counterIndex; /* both in block and in action */
    GT_U32                 ruleIndex;
    GT_U16                 pclId;
} PRV_TGF_CNC_TCAM_RULE_INFO_STC;

static PRV_TGF_CNC_TCAM_RULE_INFO_STC * tcamRuleInfoTab;

static PRV_TGF_CNC_TCAM_RULE_INFO_STC tcamRuleInfoTab_4_floors[] =
{
    /* TTI Rule index when relativeIndex==GT_FALSE */
    /* not multiplied by 3 in TGF API and just     */
    /* measured in 10-bytes rules as in CPSS API   */
    {PRV_TGF_CNC_CLIENT_TTI_PARALLEL_0_E,
        0,  0, (       0), 1},
    {PRV_TGF_CNC_CLIENT_TTI_PARALLEL_1_E,
        1,  1, (    (PRV_TGF_CNS_TCAM_FLOOR_STD_RULES_CNS * 3)), 1},
    {PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_PARALLEL_0_E,
        2,  2, (       1), 2},
    {PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_PARALLEL_1_E,
        3,  3, (PRV_TGF_CNS_TCAM_FLOOR_STD_RULES_CNS + 1), 2},
    {PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_PARALLEL_2_E,
        4,  4, ((2 * PRV_TGF_CNS_TCAM_FLOOR_STD_RULES_CNS) + 1), 2},
    {PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_PARALLEL_3_E,
        5,  5, ((3 * PRV_TGF_CNS_TCAM_FLOOR_STD_RULES_CNS) + 1), 2},
    {PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_1_PARALLEL_0_E,
        6,  6, (       2), 3},
    {PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_1_PARALLEL_1_E,
        7,  7, (PRV_TGF_CNS_TCAM_FLOOR_STD_RULES_CNS + 2), 3},
    {PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_1_PARALLEL_2_E,
        8,  8, ((2 * PRV_TGF_CNS_TCAM_FLOOR_STD_RULES_CNS) + 2), 3},
    {PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_1_PARALLEL_3_E,
        9,  9, ((3 * PRV_TGF_CNS_TCAM_FLOOR_STD_RULES_CNS) + 2), 3},
    {PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_2_PARALLEL_0_E,
        10, 10, (       3), 4},
    {PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_2_PARALLEL_1_E,
        11, 11, (PRV_TGF_CNS_TCAM_FLOOR_STD_RULES_CNS + 3), 4},
    {PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_2_PARALLEL_2_E,
        12, 12, ((2 * PRV_TGF_CNS_TCAM_FLOOR_STD_RULES_CNS) + 3), 4},
    {PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_2_PARALLEL_3_E,
        13, 13, ((3 * PRV_TGF_CNS_TCAM_FLOOR_STD_RULES_CNS) + 3), 4},
    {PRV_TGF_CNC_CLIENT_EGRESS_PCL_PARALLEL_0_E,
        14, 14, (       4), 5},
    {PRV_TGF_CNC_CLIENT_EGRESS_PCL_PARALLEL_1_E,
        15, 15, (PRV_TGF_CNS_TCAM_FLOOR_STD_RULES_CNS + 4), 5},
    {PRV_TGF_CNC_CLIENT_EGRESS_PCL_PARALLEL_2_E,
        16, 16, ((2 * PRV_TGF_CNS_TCAM_FLOOR_STD_RULES_CNS) + 4), 5},
    {PRV_TGF_CNC_CLIENT_EGRESS_PCL_PARALLEL_3_E,
        17, 17, ((3 * PRV_TGF_CNS_TCAM_FLOOR_STD_RULES_CNS) + 4), 5}
};

static GT_U32 tcamRuleInfoTabSize = sizeof(tcamRuleInfoTab_4_floors) / sizeof(tcamRuleInfoTab_4_floors[0]);

static PRV_TGF_CNC_TCAM_RULE_INFO_STC tcamRuleInfoTab_2_floors[] =
{
    {PRV_TGF_CNC_CLIENT_TTI_PARALLEL_0_E,
        0,  0, (       0), 1},
    {PRV_TGF_CNC_CLIENT_TTI_PARALLEL_1_E,
        1,  1, (       6), 1},

    {PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_PARALLEL_0_E,
        2,  2, (PCL_RULE_INDEX_HIT_0_CNS), 2},
    {PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_PARALLEL_1_E,
        3,  3, (PCL_RULE_INDEX_HIT_1_CNS), 2},
    {PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_PARALLEL_2_E,
        4,  4, (PCL_RULE_INDEX_HIT_2_CNS), 2},
    {PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_PARALLEL_3_E,
        5,  5, (PCL_RULE_INDEX_HIT_3_CNS), 2},

    {PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_1_PARALLEL_0_E,
        6,  6, (PCL_RULE_INDEX_HIT_0_CNS + 4), 3},
    {PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_1_PARALLEL_1_E,
        7,  7, (PCL_RULE_INDEX_HIT_1_CNS + 4), 3},
    {PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_1_PARALLEL_2_E,
        8,  8, (PCL_RULE_INDEX_HIT_2_CNS + 4), 3},
    {PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_1_PARALLEL_3_E,
        9,  9, (PCL_RULE_INDEX_HIT_3_CNS + 4), 3},

    {PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_2_PARALLEL_0_E,
        10, 10, (PCL_RULE_INDEX_HIT_0_CNS + 8), 4},
    {PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_2_PARALLEL_1_E,
        11, 11, (PCL_RULE_INDEX_HIT_1_CNS + 8), 4},
    {PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_2_PARALLEL_2_E,
        12, 12, (PCL_RULE_INDEX_HIT_2_CNS + 8), 4},
    {PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_2_PARALLEL_3_E,
        13, 13, (PCL_RULE_INDEX_HIT_3_CNS + 8), 4},

    {PRV_TGF_CNC_CLIENT_EGRESS_PCL_PARALLEL_0_E,
        14, 14, (PCL_RULE_INDEX_HIT_0_CNS + 12), 5},
    {PRV_TGF_CNC_CLIENT_EGRESS_PCL_PARALLEL_1_E,
        15, 15, (PCL_RULE_INDEX_HIT_1_CNS + 12), 5},
    {PRV_TGF_CNC_CLIENT_EGRESS_PCL_PARALLEL_2_E,
        16, 16, (PCL_RULE_INDEX_HIT_2_CNS + 12), 5},
    {PRV_TGF_CNC_CLIENT_EGRESS_PCL_PARALLEL_3_E,
        17, 17, (PCL_RULE_INDEX_HIT_3_CNS + 12), 5}
};

/**
* @internal prvTgfCnsTcamFloorsCfgSet function
* @endinternal
*
* @brief   Sets 4 TCAM Floor access:
*         cliets membersip in client-groups
*         and client-groups and hits for all 2 halfs of every floor.
* @param[in] stopOnError              - stop On Error
*                                      GT_TRUE for configure, GT_FALSE for restore
*                                      tcaqm4FloorsCfgPtr - pointer to configuration
*
* @retval GT_OK                    - on success
*/
static GT_STATUS prvTgfCnsTcamFloorsCfgSet
(
    IN  GT_BOOL                           stopOnError,
    IN  PRV_TGF_CNC_TCAM_FLOORS_CFG_STC * tcaqmFloorsCfgPtr
)
{
    GT_STATUS   rc, rc1 = GT_OK;
    GT_U32      floorIndex;
    GT_U32      floorNumber;
    GT_U32      i;

    if(!HARD_WIRE_TCAM_MAC(prvTgfDevNum))
    {
        /* AUTODOC: Clients to TCAM groups binding */
        for (i = 0 ; (i < 5); i++)
        {
            if ((i == PRV_TGF_TCAM_IPCL_2_E) &&
                (PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->hwInfo.pcl.iPcl0Bypass))
            {
                /* PCL stage is bypassed */
                continue;
            }

            rc = prvTgfTcamClientGroupSet(
                i, tcaqmFloorsCfgPtr->clientGroupArr[i],
                tcaqmFloorsCfgPtr->clientEnableArr[i]);
            PRV_UTF_VERIFY_LOG_COND_STOP_MAC(
                stopOnError, rc1, rc, "prvTgfTcamClientGroupSet");
        }
    }

    floorNumber = (prvTgfTcamFloorsNumGet() > 3) ? 4 : 2;

    /* AUTODOC: Set TCAM floor info */
    for (floorIndex = 0 ; (floorIndex < floorNumber); floorIndex++)
    {
        rc = prvTgfTcamIndexRangeHitNumAndGroupSet(
            floorIndex, &(tcaqmFloorsCfgPtr->floor[floorIndex].floorInfoArr[0]));
        PRV_UTF_VERIFY_LOG_COND_STOP_MAC(
            stopOnError, rc1, rc, "prvTgfTcamIndexRangeHitNumAndGroupSet");
    }

    return rc1;
}

/**
* @internal prvTgfCnsTcamFloorsCfgGet function
* @endinternal
*
* @brief   Sets TCAM Floor access:
*         clients membersip in client-groups
*         and client-groups and hits for all 2 halfs of every floor.
*
* @retval GT_OK                    - on success
*/
static GT_STATUS prvTgfCnsTcamFloorsCfgGet
(
    OUT PRV_TGF_CNC_TCAM_FLOORS_CFG_STC *tcaqmFloorsCfgPtr
)
{
    GT_STATUS   rc;
    GT_U32      floorIndex;
    GT_U32      i;

    cpssOsMemSet(
        tcaqmFloorsCfgPtr, 0, sizeof(PRV_TGF_CNC_TCAM_FLOORS_CFG_STC));

    if(!HARD_WIRE_TCAM_MAC(prvTgfDevNum))
    {
        /* AUTODOC: Clients to TCAM groups binding */
        for (i = 0 ; (i < 5); i++)
        {
            rc = prvTgfTcamClientGroupGet(
                i, &(tcaqmFloorsCfgPtr->clientGroupArr[i]),
                &(tcaqmFloorsCfgPtr->clientEnableArr[i]));
            PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfTcamClientGroupGet");
        }
    }

    /* AUTODOC: Get TCAM floor info */
    for (floorIndex = 0 ; (floorIndex < prvTgfTcamFloorsNumGet()); floorIndex++)
    {
        rc = prvTgfTcamIndexRangeHitNumAndGroupGet(
            floorIndex, &(tcaqmFloorsCfgPtr->floor[floorIndex].floorInfoArr[0]));
        PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfTcamIndexRangeHitNumAndGroupGet");
    }

    return GT_OK;
}

/**
* @internal prvTgfCnsTcamFloorsConfigure function
* @endinternal
*
* @brief   Sets TCAM Floor access:
*         clients membersip in client-groups
*         and client-groups and hits for all 2 halfs of 4 floors.
*
* @retval GT_OK                    - on success
*/
static GT_STATUS prvTgfCnsTcamFloorsConfigure
(
    GT_VOID
)
{
    PRV_TGF_CNC_TCAM_FLOORS_CFG_STC tcaqmFloorsCfg;
    GT_STATUS                         rc;
    GT_U32                            i, j;
    GT_U32                          floorNumber;
    GT_U32                          hitNumber;

    /* save regrular TCAM configuration */
    if (saveFloorsCfgValid == GT_FALSE)
    {
        rc = prvTgfCnsTcamFloorsCfgGet(&saveFloorsCfg);
        if (rc != GT_OK)
        {
            return rc;
        }
        saveFloorsCfgValid = GT_TRUE;
    }

    /*
    *            All 5 clients enabled in client-group
    *            Floor0 - client-group, hit0
    *            Floor1 - client-group, hit1
    *            Floor2 - client-group, hit2
    *            Floor3 - client-group, hit3
    */
    cpssOsMemSet(&tcaqmFloorsCfg, 0, sizeof(tcaqmFloorsCfg));
    for (i = 0; (i < 5); i++)
    {
        tcaqmFloorsCfg.clientEnableArr[i] = GT_TRUE;
        if(!HARD_WIRE_TCAM_MAC(prvTgfDevNum))
        {
            tcaqmFloorsCfg.clientGroupArr[i]  = PRV_TGF_CNS_CLIENT_GROUP_CNS;
        }
        else
        {
            tcaqmFloorsCfg.clientGroupArr[i]  = i;
        }
    }

    floorNumber = (prvTgfTcamFloorsNumGet() > 3) ? 4 : 2;

    for (i = 0; (i < floorNumber); i++)
    {
        for (j = 0; (j < PRV_TGF_TCAM_MAX_TCAM_BLOCKS_CNS); j++)
        {
            if(!HARD_WIRE_TCAM_MAC(prvTgfDevNum))
            {
                tcaqmFloorsCfg.floor[i].floorInfoArr[j].group  = PRV_TGF_CNS_CLIENT_GROUP_CNS;
            }
            else
            {
                tcaqmFloorsCfg.floor[i].floorInfoArr[j].group  =  i;/* floor per group */
            }

            if (floorNumber == 2)
            {
                hitNumber = i*2 + ((j < (PRV_TGF_TCAM_MAX_TCAM_BLOCKS_CNS / 2))? 0 : 1);
            }
            else
            {
                hitNumber = i;
            }
            tcaqmFloorsCfg.floor[i].floorInfoArr[j].hitNum = hitNumber;
        }
    }

    return prvTgfCnsTcamFloorsCfgSet(
        GT_TRUE /*stopOnError*/, &tcaqmFloorsCfg);
}

/**
* @internal prvTgfCnsTcam4FloorsRestore function
* @endinternal
*
* @brief   Restores 4 TCAM Floor access saved by prvTgfCnsTcam4FloorsConfigure
*
* @retval GT_OK                    - on success
*/
static GT_STATUS prvTgfCnsTcam4FloorsRestore
(
    GT_VOID
)
{
    /* restore regrular TCAM configuration */
    if (saveFloorsCfgValid == GT_FALSE)
    {
        return GT_OK;
    }
    return prvTgfCnsTcamFloorsCfgSet(
        GT_FALSE /*stopOnError*/, &saveFloorsCfg);
}


/**
* @internal prvTgfCnsTcamParallelHitConfigure function
* @endinternal
*
* @brief   Sets Configuration for All TCAM CNC Clients parallel hits.
*
* @retval GT_OK                    - on success
*/
static GT_STATUS prvTgfCnsTcamParallelHitConfigure
(
    GT_VOID
)
{
    GT_STATUS rc;
    GT_U32    i;
    static GT_U32    rangeBitMap1[4] = {1,0,0,0};
    PRV_TGF_CNC_COUNTER_STC counterValue0;
    PRV_TGF_TTI_ACTION_2_STC ttiAction;
    PRV_TGF_TTI_RULE_UNT     ttiPattern;
    PRV_TGF_TTI_RULE_UNT     ttiMask;
    PRV_TGF_PCL_RULE_FORMAT_UNT      ipclMask;
    PRV_TGF_PCL_RULE_FORMAT_UNT      ipclPattern;
    PRV_TGF_PCL_ACTION_STC           ipclAction;
    PRV_TGF_PCL_LOOKUP_CFG_STC       ipclLookupCfg;
    PRV_TGF_PCL_RULE_FORMAT_UNT      epclMask;
    PRV_TGF_PCL_RULE_FORMAT_UNT      epclPattern;
    PRV_TGF_PCL_ACTION_STC           epclAction;
    PRV_TGF_PCL_LOOKUP_CFG_STC       epclLookupCfg;

    cpssOsMemSet(&counterValue0, 0, sizeof(counterValue0));

    rc = prvTgfCnsTcamFloorsConfigure();
    if (rc != GT_OK)
    {
        return rc;
    }

    /* store TTI PCL id */
    rc = prvTgfTtiPclIdGet(
        prvTgfDevNum, PRV_TGF_TTI_KEY_ETH_E, &saveTtiPclId);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfTtiPclIdGet");

    /* TTI Common */
    /* AUTODOC: enable the TTI lookup for given packet type at the port */
    rc = prvTgfTtiPortLookupEnableSet(
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        PRV_TGF_TTI_KEY_ETH_E, GT_TRUE);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfTtiPortLookupEnableSet");

    /* TTI Rule Common */
    /*relative==GT_FALSE is default */
    prvTgfTtiTcamIndexIsRelativeSet(GT_FALSE);
    /* AUTODOC: set the TTI Rule Pattern, Mask and Action for TCAM location */
    cpssOsMemSet(&ttiAction, 0, sizeof(ttiAction));
    cpssOsMemSet(&ttiMask, 0, sizeof(ttiMask));
    cpssOsMemSet(&ttiPattern, 0, sizeof(ttiPattern));
    ttiMask.eth.common.pclId    = 0x3FF;
    ttiAction.command                           = CPSS_PACKET_CMD_FORWARD_E;
    ttiAction.tag0VlanCmd                       = PRV_TGF_TTI_VLAN_DO_NOT_MODIFY_E;
    ttiAction.tag0VlanPrecedence                = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E;
    ttiAction.bindToCentralCounter              = GT_TRUE;
    ttiAction.continueToNextTtiLookup           = GT_TRUE;

    /* IPCL common */
    cpssOsMemSet(&ipclMask, 0, sizeof(ipclMask));
    cpssOsMemSet(&ipclPattern, 0, sizeof(ipclPattern));
    cpssOsMemSet(&ipclAction, 0, sizeof(ipclAction));
    cpssOsMemSet(&ipclLookupCfg, 0, sizeof(ipclLookupCfg));

    ipclMask.ruleStdIpL2Qos.common.pclId = 0x3F;
    ipclAction.pktCmd = CPSS_PACKET_CMD_FORWARD_E;
    ipclAction.matchCounter.enableMatchCount = GT_TRUE;

    ipclLookupCfg.enableLookup = GT_TRUE;
    ipclLookupCfg.groupKeyTypes.nonIpKey =
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E;
    ipclLookupCfg.groupKeyTypes.ipv4Key =
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E;
    ipclLookupCfg.groupKeyTypes.ipv6Key =
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E;
    ipclLookupCfg.tcamSegmentMode =
        PRV_TGF_PCL_TCAM_SEGMENT_MODE_4_TCAMS_E;

    /* EPCL common */
    cpssOsMemSet(&epclMask, 0, sizeof(epclMask));
    cpssOsMemSet(&epclPattern, 0, sizeof(epclPattern));
    cpssOsMemSet(&epclAction, 0, sizeof(epclAction));
    cpssOsMemSet(&epclLookupCfg, 0, sizeof(epclLookupCfg));

    epclMask.ruleEgrStdIpL2Qos.common.pclId    = 0x3F;
    epclAction.pktCmd = CPSS_PACKET_CMD_FORWARD_E;
    epclAction.egressPolicy                  = GT_TRUE;
    epclAction.matchCounter.enableMatchCount = GT_TRUE;

    epclLookupCfg.enableLookup = GT_TRUE;
    epclLookupCfg.groupKeyTypes.nonIpKey =
        PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E;
    epclLookupCfg.groupKeyTypes.ipv4Key =
        PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E;
    epclLookupCfg.groupKeyTypes.ipv6Key =
        PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E;
    epclLookupCfg.tcamSegmentMode =
        PRV_TGF_PCL_TCAM_SEGMENT_MODE_4_TCAMS_E;

    if (prvTgfTcamFloorsNumGet() > 3)
    {
        tcamRuleInfoTab = tcamRuleInfoTab_4_floors;
        tcamRuleInfoTabSize = sizeof(tcamRuleInfoTab_4_floors) / sizeof(tcamRuleInfoTab_4_floors[0]);
    }
    else
    {
        tcamRuleInfoTab = tcamRuleInfoTab_2_floors;
        tcamRuleInfoTabSize = sizeof(tcamRuleInfoTab_2_floors) / sizeof(tcamRuleInfoTab_2_floors[0]);
    }
    if (tcamRuleInfoTabSize > PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->fineTuning.tableSize.cncBlocks)
    {
        tcamRuleInfoTabSize = PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->fineTuning.tableSize.cncBlocks;
    }

    for (i = 0; (i < tcamRuleInfoTabSize); i++)
    {
        if (PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->hwInfo.pcl.iPcl0Bypass)
        {
            switch (tcamRuleInfoTab[i].client)
            {
            case PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_1_E:
            case PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_2_PARALLEL_0_E:
            case PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_2_PARALLEL_1_E:
            case PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_2_PARALLEL_2_E:
            case PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_2_PARALLEL_3_E:
                /* PCL stage is bypassed */
                continue;
            default:
                /* Do nothing */
                break;
            }
        }

        rc = prvTgfCncBlockClientEnableSet(
            tcamRuleInfoTab[i].cncBlock,
            tcamRuleInfoTab[i].client, GT_TRUE);
        PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfCncBlockClientEnableSet");

        rc = prvTgfCncBlockClientRangesSet(
            tcamRuleInfoTab[i].cncBlock,
            tcamRuleInfoTab[i].client, rangeBitMap1);
        PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfCncBlockClientRangesSet");

        rc = prvTgfCncCounterFormatSet(
            tcamRuleInfoTab[i].cncBlock,
            PRV_TGF_CNC_COUNTER_FORMAT_MODE_0_E);
        PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfCncCounterFormatSet");

        rc = prvTgfCncCounterSet(
            tcamRuleInfoTab[i].cncBlock,
            tcamRuleInfoTab[i].counterIndex,
            PRV_TGF_CNC_COUNTER_FORMAT_MODE_0_E,
            &counterValue0);
        PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfCncCounterSet");

        switch (tcamRuleInfoTab[i].client)
        {
            case PRV_TGF_CNC_CLIENT_TTI_PARALLEL_0_E:

                /* the same PCL Id for all TTI rules */
                ttiPattern.eth.common.pclId   = tcamRuleInfoTab[i].pclId;
                /* no break needed */

            case PRV_TGF_CNC_CLIENT_TTI_PARALLEL_1_E:
                rc = prvTgfTtiPclIdSet(
                    prvTgfDevNum, PRV_TGF_TTI_KEY_ETH_E, tcamRuleInfoTab[i].pclId);
                PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfTtiPclIdSet");

                ttiAction.centralCounterIndex = tcamRuleInfoTab[i].counterIndex;

                rc = prvTgfTtiRule2Set(
                    tcamRuleInfoTab[i].ruleIndex, PRV_TGF_TTI_KEY_ETH_E,
                    &ttiPattern, &ttiMask, &ttiAction);
                PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfTtiRule2Set");
                break;

            case PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_PARALLEL_0_E:

                /* the same PCL Id for all Lookup0 PCL rules */
                ipclPattern.ruleStdIpL2Qos.common.pclId = tcamRuleInfoTab[i].pclId;
                ipclLookupCfg.pclId                     = tcamRuleInfoTab[i].pclId;

                rc = prvTgfPclDefPortInitExt2(
                    prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                    CPSS_PCL_DIRECTION_INGRESS_E,
                    CPSS_PCL_LOOKUP_NUMBER_0_E,
                    &ipclLookupCfg);
                PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfPclDefPortInitExt2");
                /* no break needed */

            case PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_PARALLEL_1_E:
            case PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_PARALLEL_2_E:
            case PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_PARALLEL_3_E:

                ipclAction.matchCounter.matchCounterIndex = tcamRuleInfoTab[i].counterIndex;

                /* AUTODOC: PCL rule: Write Rule */
                rc = prvTgfPclRuleSet(
                    PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E,
                    tcamRuleInfoTab[i].ruleIndex,
                    &ipclMask, &ipclPattern, &ipclAction);
                PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfPclRuleSet");
                break;

            case PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_1_PARALLEL_0_E:

                /* the same PCL Id for all Lookup1 PCL rules */
                ipclPattern.ruleStdIpL2Qos.common.pclId = tcamRuleInfoTab[i].pclId;
                ipclLookupCfg.pclId                     = tcamRuleInfoTab[i].pclId;

                rc = prvTgfPclDefPortInitExt2(
                    prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                    CPSS_PCL_DIRECTION_INGRESS_E,
                    CPSS_PCL_LOOKUP_NUMBER_1_E,
                    &ipclLookupCfg);
                PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfPclDefPortInitExt2");
                /* no break needed */

            case PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_1_PARALLEL_1_E:
            case PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_1_PARALLEL_2_E:
            case PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_1_PARALLEL_3_E:

                ipclAction.matchCounter.matchCounterIndex = tcamRuleInfoTab[i].counterIndex;

                /* AUTODOC: PCL rule: Write Rule */
                rc = prvTgfPclRuleSet(
                    PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E,
                    tcamRuleInfoTab[i].ruleIndex,
                    &ipclMask, &ipclPattern, &ipclAction);
                PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfPclRuleSet");
                break;

            case PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_2_PARALLEL_0_E:

                /* the same PCL Id for all Lookup2 PCL rules */
                ipclPattern.ruleStdIpL2Qos.common.pclId = tcamRuleInfoTab[i].pclId;
                ipclLookupCfg.pclId                     = tcamRuleInfoTab[i].pclId;

                rc = prvTgfPclDefPortInitExt2(
                    prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                    CPSS_PCL_DIRECTION_INGRESS_E,
                    CPSS_PCL_LOOKUP_NUMBER_2_E,
                    &ipclLookupCfg);
                PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfPclDefPortInitExt2");
                /* no break needed */

            case PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_2_PARALLEL_1_E:
            case PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_2_PARALLEL_2_E:
            case PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_2_PARALLEL_3_E:

                ipclAction.matchCounter.matchCounterIndex = tcamRuleInfoTab[i].counterIndex;

                /* AUTODOC: PCL rule: Write Rule */
                rc = prvTgfPclRuleSet(
                    PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E,
                    tcamRuleInfoTab[i].ruleIndex,
                    &ipclMask, &ipclPattern, &ipclAction);
                PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfPclRuleSet");
                break;

            case PRV_TGF_CNC_CLIENT_EGRESS_PCL_PARALLEL_0_E:

                /* the same PCL Id for all Lookup2 PCL rules */
                ipclPattern.ruleEgrStdIpL2Qos.common.pclId = tcamRuleInfoTab[i].pclId;
                ipclLookupCfg.pclId                      = tcamRuleInfoTab[i].pclId;

                rc = prvTgfPclDefPortInitExt2(
                    prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
                    CPSS_PCL_DIRECTION_EGRESS_E,
                    CPSS_PCL_LOOKUP_NUMBER_0_E,
                    &epclLookupCfg);
                PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfPclDefPortInitExt2");
                /* no break needed */

            case PRV_TGF_CNC_CLIENT_EGRESS_PCL_PARALLEL_1_E:
            case PRV_TGF_CNC_CLIENT_EGRESS_PCL_PARALLEL_2_E:
            case PRV_TGF_CNC_CLIENT_EGRESS_PCL_PARALLEL_3_E:

                epclAction.matchCounter.matchCounterIndex = tcamRuleInfoTab[i].counterIndex;

                /* AUTODOC: PCL rule: Write Rule */
                rc = prvTgfPclRuleSet(
                    PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E,
                    tcamRuleInfoTab[i].ruleIndex,
                    &epclMask, &epclPattern, &epclAction);
                PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfPclRuleSet");
                break;

           default: break;
        }
    }

    return GT_OK;
}


/**
* @internal prvTgfCnsTcamParallelHitRestore function
* @endinternal
*
* @brief   Restores Configuration for All TCAM CNC Cilents parralel hits.
*/
static GT_STATUS prvTgfCnsTcamParallelHitRestore
(
    GT_VOID
)
{
    GT_STATUS rc, rc1 = GT_OK;
    GT_U32    i;
    static GT_U32    rangeBitMap0[4] = {0,0,0,0};
    PRV_TGF_CNC_COUNTER_STC counterValue0;
    PRV_TGF_PCL_LOOKUP_CFG_STC       ipclLookupCfg;
    PRV_TGF_PCL_LOOKUP_CFG_STC       epclLookupCfg;

    cpssOsMemSet(&counterValue0, 0, sizeof(counterValue0));

    cpssOsMemSet(&ipclLookupCfg, 0, sizeof(ipclLookupCfg));
    ipclLookupCfg.groupKeyTypes.nonIpKey =
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E;
    ipclLookupCfg.groupKeyTypes.ipv4Key =
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E;
    ipclLookupCfg.groupKeyTypes.ipv6Key =
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E;

    cpssOsMemSet(&epclLookupCfg, 0, sizeof(epclLookupCfg));
    epclLookupCfg.groupKeyTypes.nonIpKey =
        PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E;
    epclLookupCfg.groupKeyTypes.ipv4Key =
        PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E;
    epclLookupCfg.groupKeyTypes.ipv6Key =
        PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E;

    /* TTI Common */
    /* AUTODOC: enable the TTI lookup for given packet type at the port */
    rc = prvTgfTtiPortLookupEnableSet(
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        PRV_TGF_TTI_KEY_ETH_E, GT_FALSE);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfTtiPortLookupEnableSet");

    rc = prvTgfTtiPclIdSet(
        prvTgfDevNum, PRV_TGF_TTI_KEY_ETH_E, saveTtiPclId);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfTtiPclIdSet");

    /* TTI Rule Common */
    /*relative==GT_FALSE is default */
    prvTgfTtiTcamIndexIsRelativeSet(GT_FALSE);

    for (i = 0; (i < tcamRuleInfoTabSize); i++)
    {
        if(tcamRuleInfoTab == NULL)
        {
            break;
        }

        if (PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->hwInfo.pcl.iPcl0Bypass)
        {
            switch (tcamRuleInfoTab[i].client)
            {
            case PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_1_E:
            case PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_2_PARALLEL_0_E:
            case PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_2_PARALLEL_1_E:
            case PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_2_PARALLEL_2_E:
            case PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_2_PARALLEL_3_E:
                /* PCL stage is bypassed */
                continue;
            default:
                /* Do nothing */
                break;
            }
        }

        rc = prvTgfCncBlockClientEnableSet(
            tcamRuleInfoTab[i].cncBlock,
            tcamRuleInfoTab[i].client, GT_FALSE);
        PRV_UTF_VERIFY_RC1(rc, "prvTgfCncBlockClientEnableSet");

        rc = prvTgfCncBlockClientRangesSet(
            tcamRuleInfoTab[i].cncBlock,
            tcamRuleInfoTab[i].client, rangeBitMap0);
        PRV_UTF_VERIFY_RC1(rc, "prvTgfCncBlockClientRangesSet");

        rc = prvTgfCncCounterFormatSet(
            tcamRuleInfoTab[i].cncBlock,
            PRV_TGF_CNC_COUNTER_FORMAT_MODE_0_E);
        PRV_UTF_VERIFY_RC1(rc, "prvTgfCncCounterFormatSet");

        rc = prvTgfCncCounterSet(
            tcamRuleInfoTab[i].cncBlock,
            tcamRuleInfoTab[i].counterIndex,
            PRV_TGF_CNC_COUNTER_FORMAT_MODE_0_E,
            &counterValue0);
        PRV_UTF_VERIFY_RC1(rc, "prvTgfCncCounterSet");

        switch (tcamRuleInfoTab[i].client)
        {
            case PRV_TGF_CNC_CLIENT_TTI_PARALLEL_0_E:
            case PRV_TGF_CNC_CLIENT_TTI_PARALLEL_1_E:

                rc = prvTgfTtiRuleValidStatusSet(
                    tcamRuleInfoTab[i].ruleIndex, GT_FALSE);
                PRV_UTF_VERIFY_RC1(rc, "prvTgfTtiRuleValidStatusSet");
                break;

            case PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_PARALLEL_0_E:

                /* AUTODOC: Disable Configuration table entry */
                rc = prvTgfPclDefPortInitExt2(
                    prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                    CPSS_PCL_DIRECTION_INGRESS_E,
                    CPSS_PCL_LOOKUP_NUMBER_0_E,
                    &ipclLookupCfg);
                PRV_UTF_VERIFY_RC1(rc, "prvTgfPclDefPortInitExt2");
                /* no break needed */

            case PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_PARALLEL_1_E:
            case PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_PARALLEL_2_E:
            case PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_PARALLEL_3_E:

                /* AUTODOC: PCL rule: Invalidate Rule */
                rc = prvTgfPclRuleValidStatusSet(
                    CPSS_PCL_RULE_SIZE_STD_E,
                    tcamRuleInfoTab[i].ruleIndex, GT_FALSE);
                PRV_UTF_VERIFY_RC1(rc, "prvTgfPclRuleValidStatusSet");
                break;

            case PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_1_PARALLEL_0_E:

                /* AUTODOC: Disable Configuration table entry */
                rc = prvTgfPclDefPortInitExt2(
                    prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                    CPSS_PCL_DIRECTION_INGRESS_E,
                    CPSS_PCL_LOOKUP_NUMBER_1_E,
                    &ipclLookupCfg);
                PRV_UTF_VERIFY_RC1(rc, "prvTgfPclDefPortInitExt2");
                /* no break needed */

            case PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_1_PARALLEL_1_E:
            case PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_1_PARALLEL_2_E:
            case PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_1_PARALLEL_3_E:

                /* AUTODOC: PCL rule: Invalidate Rule */
                rc = prvTgfPclRuleValidStatusSet(
                    CPSS_PCL_RULE_SIZE_STD_E,
                    tcamRuleInfoTab[i].ruleIndex, GT_FALSE);
                PRV_UTF_VERIFY_RC1(rc, "prvTgfPclRuleValidStatusSet");
                break;

            case PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_2_PARALLEL_0_E:

                /* AUTODOC: Disable Configuration table entry */
                rc = prvTgfPclDefPortInitExt2(
                    prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                    CPSS_PCL_DIRECTION_INGRESS_E,
                    CPSS_PCL_LOOKUP_NUMBER_2_E,
                    &ipclLookupCfg);
                PRV_UTF_VERIFY_RC1(rc, "prvTgfPclDefPortInitExt2");
                /* no break needed */

            case PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_2_PARALLEL_1_E:
            case PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_2_PARALLEL_2_E:
            case PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_2_PARALLEL_3_E:

                /* AUTODOC: PCL rule: Invalidate Rule */
                rc = prvTgfPclRuleValidStatusSet(
                    CPSS_PCL_RULE_SIZE_STD_E,
                    tcamRuleInfoTab[i].ruleIndex, GT_FALSE);
                PRV_UTF_VERIFY_RC1(rc, "prvTgfPclRuleValidStatusSet");
                break;

            case PRV_TGF_CNC_CLIENT_EGRESS_PCL_PARALLEL_0_E:

                /* AUTODOC: Disable Configuration table entry */
                rc = prvTgfPclDefPortInitExt2(
                    prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                    CPSS_PCL_DIRECTION_EGRESS_E,
                    CPSS_PCL_LOOKUP_NUMBER_0_E,
                    &epclLookupCfg);
                PRV_UTF_VERIFY_RC1(rc, "prvTgfPclDefPortInitExt2");
                /* no break needed */

            case PRV_TGF_CNC_CLIENT_EGRESS_PCL_PARALLEL_1_E:
            case PRV_TGF_CNC_CLIENT_EGRESS_PCL_PARALLEL_2_E:
            case PRV_TGF_CNC_CLIENT_EGRESS_PCL_PARALLEL_3_E:

                /* AUTODOC: PCL rule: Invalidate Rule */
                rc = prvTgfPclRuleValidStatusSet(
                    CPSS_PCL_RULE_SIZE_STD_E,
                    tcamRuleInfoTab[i].ruleIndex, GT_FALSE);
                PRV_UTF_VERIFY_RC1(rc, "prvTgfPclRuleValidStatusSet");
                break;

           default: break;
        }
    }

    /* AUTODOC : Disables PCL ingress Policy */
    rc = prvTgfPclIngressPolicyEnable(GT_FALSE);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfPclIngressPolicyEnable");

    /* AUTODOC: Disables PCL egress Policy */
    rc = prvTgfPclEgressPolicyEnable(GT_FALSE);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfPclEgressPolicyEnable");

    /* AUTODOC : disable all PCL configurations on the ports */
    prvTgfPclPortsRestoreAll();

    prvTgfCnsTcam4FloorsRestore();

    return rc1;
}

/**
* @internal prvTgfCnsTcamTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*
* @param[in] prvTgfPacketInfoPtr      - (pointer to) packet info
* @param[in] burstCount               - burst count
*                                       None
*/
static GT_VOID prvTgfCnsTcamTrafficGenerate
(
    IN TGF_PACKET_STC *prvTgfPacketInfoPtr,
    IN GT_U32         burstCount
)
{
    GT_STATUS            rc         = GT_OK;

    /* AUTODOC: Transmit Packets With Capture*/

    rc = prvTgfTransmitPacketsWithCapture(
        prvTgfDevsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        prvTgfPacketInfoPtr,
        burstCount,
        0      /*numVfd*/,
        NULL   /*vfdArray*/,
        prvTgfDevsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
        prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
        TGF_CAPTURE_MODE_MIRRORING_E,
        200                                /*captureOnTime*/);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfTransmitPacketsWithCapture");
}

/**
* @internal prvTgfCnsTcamTrafficEgressCheck function
* @endinternal
*
* @brief   Checks traffic.
*
* @param[in] offset                   - checked field offset
* @param[in] length                   - checked field length
* @param[in] pattern                  - checked field pattern
*                                       None
*/
static GT_VOID prvTgfCnsTcamTrafficEgressCheck
(
    IN GT_U32         offset,
    IN GT_U32         length,
    IN GT_U8          *pattern
)
{
    GT_STATUS                       rc         = GT_OK;
    CPSS_INTERFACE_INFO_STC         portInterface;
    TGF_VFD_INFO_STC                vfdArray[1];
    GT_U32                          numTriggersBmp;

    /* AUTODOC: Check VID from Tag of captured packet */

    cpssOsMemSet(&portInterface, 0, sizeof(portInterface));
    cpssOsMemSet(vfdArray, 0, sizeof(vfdArray));

    portInterface.type = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = prvTgfDevsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS];
    portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS];

    vfdArray[0].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[0].offset = offset;
    vfdArray[0].cycleCount = length;
    cpssOsMemCpy(&(vfdArray[0].patternPtr[0]), pattern, length);

    rc =  tgfTrafficGeneratorPortTxEthTriggerCountersGet(
            &portInterface,
            1 /*numVfd*/,
            vfdArray,
            &numTriggersBmp);
    if (rc != GT_NO_MORE)
    {
        UTF_VERIFY_EQUAL1_STRING_MAC(
            GT_OK, rc, "tgfTrafficGeneratorPortTxEthTriggerCountersGet: %d",
            prvTgfDevNum);
    }
    UTF_VERIFY_NOT_EQUAL0_STRING_MAC(
        0, numTriggersBmp, "received patterns bitmap");
}

/**
* @internal prvTgfCnsTcamPortVlanFdbSet function
* @endinternal
*
* @brief   Set test configuration
*/
static GT_STATUS prvTgfCnsTcamPortVlanFdbSet
(
    GT_VOID
)
{
    GT_STATUS                        rc = GT_OK;

    /* AUTODOC: SETUP CONFIGURATION: */

    /* AUTODOC: reset counters, force Link UP */
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfEthCountersReset");

    /* AUTODOC: clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "tgfTrafficTableRxPcktTblClear");

    /* AUTODOC: create VLAN as TAG in with all TAG0 tagged ports */
    rc = prvTgfBrgDefVlanEntryWriteWithTaggingCmd(
        PRV_TGF_VLAN_ID_CNS, PRV_TGF_BRG_VLAN_PORT_TAG0_CMD_E);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfBrgDefVlanEntryWriteWithTaggingCmd");

    /* AUTODOC: Learn MAC_DA of with source VID to receive port (to be Known UC) */
    rc = prvTgfBrgDefFdbMacEntryOnPortSet(
        prvTgfPacketL2Part.daMac,
        PRV_TGF_VLAN_ID_CNS /*vlanId*/,
        prvTgfDevsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
        prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
        GT_TRUE /*isStatic*/);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfBrgDefFdbMacEntryOnPortSet");

    return GT_OK;
}

/**
* @internal prvTgfCnsTcamPortVlanFdbReset function
* @endinternal
*
* @brief   Set test configuration
*/
static GT_STATUS prvTgfCnsTcamPortVlanFdbReset
(
    GT_VOID
)
{
    GT_STATUS rc, rc1 = GT_OK;

    /* AUTODOC: reset counters, force Link UP */
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfEthCountersReset");

    /* AUTODOC: clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    PRV_UTF_VERIFY_RC1(rc, "tgfTrafficTableRxPcktTblClear");

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfBrgFdbFlush");

    /* AUTODOC: invalidate VLAN Table Entry, VID as TAG in */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLAN_ID_CNS);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfBrgDefVlanEntryInvalidate");

    return rc1;
}

/**
* @internal prvTgfCnsTcamCountersCheck1 function
* @endinternal
*
* @brief   Checks Counters.
*
* @param[in] packetCount              - packet count in all counters.
* @param[in] byteCount0               - byte   count in counters given by bitmap below.
* @param[in] byteCount0CheckBmp       - bitmap of indexes in tcamRuleInfoTab to check
*                                      byte count in counters is byteCount0.
* @param[in] byteCount1               - byte   count in counters given by bitmap below.
* @param[in] byteCount1CheckBmp       - bitmap of indexes in tcamRuleInfoTab to check
*                                      byte count in counters is byteCount1.
*                                       None.
*/
static GT_STATUS prvTgfCnsTcamCountersCheck1
(
    IN GT_U32  packetCount,
    IN GT_U32  byteCount0,
    IN GT_U32  byteCount0CheckBmp,
    IN GT_U32  byteCount1,
    IN GT_U32  byteCount1CheckBmp
)
{
    GT_STATUS rc, rc1 = GT_OK;
    GT_U32    i;
    PRV_TGF_CNC_COUNTER_STC counterValue;
    GT_BOOL isTxRemote;
    GT_U32  inc0; /* increment for bytesCount0 */

    isTxRemote = (UTF_CPSS_DXCH_XCAT3X_CHECK_MAC(prvTgfDevNum) &&
                  prvCpssDxChPortRemotePortCheck(prvTgfDevNum,
                                                 prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS]));


    if (UTF_CPSS_DXCH_XCAT3X_CHECK_MAC(prvTgfDevNum) &&
        prvCpssDxChPortRemotePortCheck(prvTgfDevNum,
                                       prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS]))

    for (i = 0; (i < tcamRuleInfoTabSize); i++)
    {
        if (tcamRuleInfoTab[i].client >= PRV_TGF_CNC_CLIENT_EGRESS_PCL_PARALLEL_0_E
            &&
            tcamRuleInfoTab[i].client <= PRV_TGF_CNC_CLIENT_EGRESS_PCL_PARALLEL_3_E)
        {
            /* packets egress via AC3X remote port with 4B FROM_CPU DSA tag */
            inc0 = (isTxRemote == GT_TRUE) ? (packetCount * 4) : 0;
        }
        else
        {
            inc0 = 0;
        }

        rc = prvTgfCncCounterGet(
            tcamRuleInfoTab[i].cncBlock,
            tcamRuleInfoTab[i].counterIndex,
            PRV_TGF_CNC_COUNTER_FORMAT_MODE_0_E,
            &counterValue);
        PRV_UTF_VERIFY_RC1(rc, "prvTgfCncCounterGet");
        utfEqualVerify(GT_OK, rc, __LINE__, __FILE__);

        PRV_UTF_LOG5_MAC(
            "counter %d, block %d index %d packets %d bytes %d\n",
            i, tcamRuleInfoTab[i].cncBlock, tcamRuleInfoTab[i].counterIndex,
            counterValue.packetCount.l[0], counterValue.byteCount.l[0]);

        utfEqualVerify(packetCount, counterValue.packetCount.l[0], __LINE__, __FILE__);

        if (byteCount0CheckBmp & (1 << i))
        {
            utfEqualVerify(byteCount0 + inc0, counterValue.byteCount.l[0], __LINE__, __FILE__);
        }

        if (byteCount1CheckBmp & (1 << i))
        {
            utfEqualVerify(byteCount1, counterValue.byteCount.l[0], __LINE__, __FILE__);
        }
    }

    return rc1;
}

/**
* @internal prvTgfCnsTcamClientsTest1 function
* @endinternal
*
* @brief   Set test configuration
*/
GT_VOID prvTgfCnsTcamClientsTest1
(
    GT_VOID
)
{
    GT_STATUS rc;
    GT_U32    burstCount = 2;
    GT_U32    l2ByteCount;
    GT_U32    l3ByteCount;

    rc = prvTgfCnsTcamPortVlanFdbSet();
    if (rc != GT_OK) goto label_restore;


    rc = prvTgfCnsTcamParallelHitConfigure();
    if (rc != GT_OK) goto label_restore;

    prvTgfCnsTcamTrafficGenerate(
        &prvTgfPacketIpv4OthInfo, burstCount);

    prvTgfCnsTcamTrafficEgressCheck(
        6/*offset*/,
        6/*length*/,
        prvTgfPacketL2Part.saMac);

    l3ByteCount =
        (TGF_IPV4_HEADER_SIZE_CNS + sizeof(prvTgfPacketPayloadDataArr));
    l2ByteCount =
        (TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS
         + TGF_ETHERTYPE_SIZE_CNS + l3ByteCount + 4/*CRC*/);

    /* all 18 counters assumed in L2 mode */
    prvTgfCnsTcamCountersCheck1(
        burstCount /*packetCount*/,
        (burstCount * l2ByteCount)/*byteCount0*/,
        0xFFFFFFFF/*byteCount0CheckBmp*/,
        (burstCount * l3ByteCount)/*byteCount1*/,
        0/*byteCount1CheckBmp*/);

    label_restore:

    prvTgfCnsTcamParallelHitRestore();

    prvTgfCnsTcamPortVlanFdbReset();
}

