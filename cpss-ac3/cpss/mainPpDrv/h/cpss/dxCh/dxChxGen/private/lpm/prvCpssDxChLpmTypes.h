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
* @file prvCpssDxChLpmTypes.h
*
* @brief This file includes internal LPM definitions.
*
* @version   7
********************************************************************************
*/
#ifndef __prvCpssDxChLpmTypesh
#define __prvCpssDxChLpmTypesh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/generic/ip/cpssIpTypes.h>
#include <cpss/dxCh/dxChxGen/pcl/cpssDxChPcl.h>

/* Defines */
/* number of memories blocks used for IP LPM Management */
#define PRV_CPSS_DXCH_LPM_RAM_MAX_NUM_OF_MEMORY_BLOCKS_CNS  20

/* number of octets in IPv4 protocol*/
#define PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS  4

/* number of octets where GON can reside in IPv4 protocol: 5 octets due to group of nodes arc*/
#define PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_FOR_GON_IN_IPV4_PROTOCOL_CNS  5

/* number of octets in IPv6 protocol */
#define PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS  16

/* number of octets where GON can reside in IPv6 protocol: 17 octets due to group of nodes arc*/
#define PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_FOR_GON_IN_IPV6_PROTOCOL_CNS  17

/* number of octets where GON can reside in IPv6 mc protocol: 17 octets of group abd 17 octets source
   due to group of nodes arc */
#define PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_FOR_GON_IN_IPV6_MC_PROTOCOL_CNS  34

/* number of octets in FCOE protocol */
#define PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_FCOE_PROTOCOL_CNS  3

/* number of octets where GON can reside in FCOE protocol: 4 octets due to group of nodes arc*/
#define PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_FOR_GON_IN_FCOE_PROTOCOL_CNS  4

/* maximal number of octets in protocol */
#define PRV_CPSS_DXCH_LPM_MAX_NUM_OF_OCTETS_FOR_ADDRESS_CNS   16

/* Get the IPv4 bit value within a protocol bitmap */
#define PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_STATUS_GET_MAC(protocolBitmap)   \
        ((protocolBitmap & PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_BIT_E) ? GT_TRUE : GT_FALSE)

/* Get the IPv6 bit value within a protocol bitmap */
#define PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_STATUS_GET_MAC(protocolBitmap)   \
        ((protocolBitmap & PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_BIT_E) ? GT_TRUE : GT_FALSE)

/* Get the FCoE bit value within a protocol bitmap */
#define PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_STATUS_GET_MAC(protocolBitmap)   \
        ((protocolBitmap & PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_BIT_E) ? GT_TRUE : GT_FALSE)

/* Set the FCoE bit value within a protocol bitmap */
#define PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_STATUS_SET_MAC(protocolBitmap)   \
        (protocolBitmap |= (1<<2))

/* clear the FCoE bit value within a protocol bitmap */
#define PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_STATUS_CLEAR_MAC(protocolBitmap)   \
        (protocolBitmap &= ~(1<<2))


/* Convert PRV_CPSS_DXCH_LPM_ENTRY_TYPE_ENT to CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT */
#define PRV_CPSS_DXCH_LPM_NEXT_PTR_TYPE_CONVERT_MAC(inLpmEntryType,outLpmEntryType) \
    switch (inLpmEntryType)                                              \
    {                                                                    \
        case PRV_CPSS_DXCH_LPM_ENTRY_TYPE_ECMP_E:                        \
            outLpmEntryType = CPSS_DXCH_LPM_ECMP_ENTRY_PTR_TYPE_E;       \
            break;                                                       \
        case PRV_CPSS_DXCH_LPM_ENTRY_TYPE_QOS_E:                         \
            outLpmEntryType = CPSS_DXCH_LPM_QOS_ENTRY_PTR_TYPE_E;        \
            break;                                                       \
        default: /* PRV_CPSS_DXCH_LPM_ENTRY_TYPE_REGULAR_E */            \
            outLpmEntryType = CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E; \
            break;                                                       \
    }

/* Convert PRV_CPSS_DXCH_SIP6_LPM_NEXT_PTR_TYPE_CONVERT_MAC to CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT */
#define PRV_CPSS_DXCH_SIP6_LPM_NEXT_PTR_TYPE_CONVERT_MAC(inLpmEntryType,outLpmEntryType) \
    switch (inLpmEntryType)                                              \
    {                                                                    \
        case PRV_CPSS_DXCH_LPM_ENTRY_TYPE_MULTIPATH_E:                   \
            outLpmEntryType = CPSS_DXCH_LPM_MULTIPATH_ENTRY_PTR_TYPE_E;  \
            break;                                                       \
        default: /* PRV_CPSS_DXCH_LPM_ENTRY_TYPE_REGULAR_E */            \
            outLpmEntryType = CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E; \
            break;                                                       \
    }
/* Typedefs */
/* representation of the LPM supported protocols as bitmap.
   See PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_BIT_ENT below */
typedef GT_U32 PRV_CPSS_DXCH_LPM_PROTOCOL_BMP;

/**
* @enum PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_BIT_ENT
 *
 * @brief The representation of each protocol in PRV_CPSS_DXCH_LPM_PROTOCOL_BMP
*/
typedef enum{

    /** IPv4 bit */
    PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_BIT_E           = 0x1,

    /** IPv6 bit */
    PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_BIT_E           = 0x2,

    /** FCoE bit */
    PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_BIT_E           = 0x4

} PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_BIT_ENT;

/**
* @enum PRV_CPSS_DXCH_LPM_SHADOW_TYPE_ENT
 *
 * @brief Types of LPM shadows
*/
typedef enum{

    /** @brief xCat type shadow (routing is router TCAM based)
     *  4 columns, row based search order
     *  IPv4 entry use single TCAM entry, IPv6 entry use single TCAM line
     */
    PRV_CPSS_DXCH_LPM_TCAM_XCAT_SHADOW_E                         = 3,

    /** @brief xCat type shadow (routing is policy based routing)
     *  4 columns, row based search order
     *  IPv4 & IPv6 entries use single TCAM entry
     */
    PRV_CPSS_DXCH_LPM_TCAM_XCAT_POLICY_BASED_ROUTING_SHADOW_E    = 4,

    /** Bobcat2; Caelum; Bobcat3; Aldrin type shadow (IP routing and FCoE forwarding is RAM based) */
    PRV_CPSS_DXCH_LPM_RAM_SIP5_SHADOW_E                         = 5,

    /** Falcon type shadow (IP routing and FCoE forwarding is RAM based) */
    PRV_CPSS_DXCH_LPM_RAM_SIP6_SHADOW_E                         = 6,

    /** last value in the enum */
    PRV_CPSS_DXCH_LPM_SHADOW_TYPE_LAST_E

} PRV_CPSS_DXCH_LPM_SHADOW_TYPE_ENT;

/**
* @enum PRV_CPSS_DXCH_LPM_HW_ENT
 *
 * @brief Type of HW that stores the LPM information
*/
typedef enum{

    /** TCAM */
    PRV_CPSS_DXCH_LPM_HW_TCAM_E                 = 0,

    /** RAM */
    PRV_CPSS_DXCH_LPM_HW_RAM_E                  = 1,

    /** last value in the enum */
    PRV_CPSS_DXCH_LPM_HW_LAST_E

} PRV_CPSS_DXCH_LPM_HW_ENT;

/**
* @enum PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT
 *
 * @brief Protocol stack
*/
typedef enum{

    /** IPv4 */
    PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E           = 0,

    /** IPv6 */
    PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E           = 1,

    /** FCoE */
    PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E           = 2,

    /** last value in the enum */
    PRV_CPSS_DXCH_LPM_PROTOCOL_LAST_E           = 3

} PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT;

/**
* @enum PRV_CPSS_DXCH_LPM_ENTRY_TYPE_ENT
 *
 * @brief The entry that is associated to unicast prefix or multicast
 * (G, S) or (G, ) session.
*/
typedef enum{

    /** ECMP routing/forwarding ECMP entry is associated */
    PRV_CPSS_DXCH_LPM_ENTRY_TYPE_ECMP_E      = 0,

    /** @brief For QoS routing/forwarding
     *  ECMP entry is associated (used for QoS)
     */
    PRV_CPSS_DXCH_LPM_ENTRY_TYPE_QOS_E       = 1,

    /** Nexthop is associated */
    PRV_CPSS_DXCH_LPM_ENTRY_TYPE_REGULAR_E   = 2,

    /** multipath routing/forwarding entry is associated */
    PRV_CPSS_DXCH_LPM_ENTRY_TYPE_MULTIPATH_E = 3

} PRV_CPSS_DXCH_LPM_ENTRY_TYPE_ENT;

/*
 * Typedef: struct PRV_CPSS_DXCH_LPM_ECMP_ENTRY_STCT
 *
 * Description: ECMP entry
 *
 * Fields:
 *      randomEnable     - randomize the selection of nexthop
 *      numOfPaths       - number of ECMP or QoS paths
 *      nexthopBaseIndex - the base address of the ECMP block
 *      multiPathMode          - multipath mode. Applicable values:
 *                                PRV_CPSS_DXCH_LPM_ENTRY_TYPE_ECMP_E,
 *                                PRV_CPSS_DXCH_LPM_ENTRY_TYPE_QOS_E.
 *                                (APPLICABLE DEVICES: Falcon)
 
 */
typedef struct PRV_CPSS_DXCH_LPM_ECMP_ENTRY_STCT
{
    GT_BOOL                              randomEnable;
    GT_U32                               numOfPaths;
    GT_U32                               nexthopBaseIndex;
    PRV_CPSS_DXCH_LPM_ENTRY_TYPE_ENT     multiPathMode;

} PRV_CPSS_DXCH_LPM_ECMP_ENTRY_STC;


/*
 * Typedef: enum PRV_CPSS_DXCH_LPM_LEAF_ENTRY_PRIORITY_TYPE_ENT
 *
 * Description: Define priority between FDB result and LPM result.
 *
 * Enumerations:
 *      PRV_CPSS_DXCH_LPM_LEAF_ENTRY_PRIORITY_FDB_E -
 *          FDB has priority over LPM result.
 *
 *      PRV_CPSS_DXCH_LPM_LEAF_ENTRY_PRIORITY_LPM_E -
 *          LPM result has priority over exact match result
 */
typedef enum
{
    PRV_CPSS_DXCH_LPM_LEAF_ENTRY_PRIORITY_FDB_E,
    PRV_CPSS_DXCH_LPM_LEAF_ENTRY_PRIORITY_LPM_E,
} PRV_CPSS_DXCH_LPM_LEAF_ENTRY_PRIORITY_TYPE_ENT;

/*
 * Typedef: struct PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC
 *
 * Description: the route entry pointer info
 *
 * Fields:
 *      routeEntryBaseMemAddr  -
 *          For DxCh1, DxCh1_Diamond, DxCh2, DxCh3, xCat, xCat3, Lion, xCat2, Lion2:
 *                               the base memory Address (offset) of the
 *                               nexthop entry
 *          For Bobcat2, Caelum, Bobcat3, Aldrin, AC3X:  the base memory Address (offset) of the
 *                               nexthop entry or ECMP entry
 *      blockSize              - the size of the route entries group
 *                               if REGULAR entry, blockSize must be 1
 *                               if ECMP or QoS, the range is from 1 to 16.
 *                               (APPLICABLE DEVICES: DxCh1; DxCh1_Diamond;
 *                                DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2)
 *                               (APPLICABLE RANGES: 1...16)
 *      routeEntryMethod       - the type of the route entries group, see
 *                               description of PRV_CPSS_DXCH_LPM_ENTRY_TYPE_ENT
 *      ucRpfCheckEnable       - enable UC RPF check
 *      srcAddrCheckMismatchEnable - enable mismatch check of SA and SIP (for
 *                               IP) / SA and S_ID (for FCoE)
 *      ipv6McGroupScopeLevel  - the IPv6 MC group scope level
 *      priority               - resolution priority between exact match and LPM results
 */
typedef struct PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STCT
{
    GT_U32                                          routeEntryBaseMemAddr;
    GT_U32                                          blockSize;
    PRV_CPSS_DXCH_LPM_ENTRY_TYPE_ENT                routeEntryMethod;
    GT_BOOL                                         ucRpfCheckEnable;
    GT_BOOL                                         srcAddrCheckMismatchEnable;
    CPSS_IPV6_PREFIX_SCOPE_ENT                      ipv6McGroupScopeLevel;
    PRV_CPSS_DXCH_LPM_LEAF_ENTRY_PRIORITY_TYPE_ENT  priority;
} PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC;

/*
 * Typedef: union PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_INFO_UNTT
 *
 * Description:
 *      This union holds the 2 possible associations.
 *
 * Fields:
 *      pclIpUcActionPtr - a pointer to PCL action representing the UC route
 *                         (APPLICABLE DEVICES: DxCh1; DxCh1_Diamond; xCat; xCat3; xCat2)
 *      routeEntry       - a pointer to a route entry which holds a pointer to
 *                         nexthop or ECMP entry and some more information
 *                         (APPLICABLE DEVICES: DxCh2; DxCh3; xCat; xCat3; Lion;
 *                                              Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
 */
typedef struct PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_INFO_UNTT
{
    CPSS_DXCH_PCL_ACTION_STC                     pclIpUcAction;
    PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC    routeEntry;
} PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_INFO_UNT;

/*
 * Typedef: struct PRV_CPSS_DXCH_LPM_SHADOW_DEVS_LIST_STC
 *
 * Description:
 *      LPM shadow shared device list
 *
 * Fields:
 *
 *      ipShareDevNum   - device numbers of devices sharing this shadow.
 *      ipShareDevs     - Device numbers of PPs sharing the shadow.
 *
 */
typedef struct PRV_CPSS_DXCH_LPM_SHADOW_DEVS_LIST_STCT
{
    GT_U32   ipShareDevNum;
    GT_U8    *ipShareDevs;
} PRV_CPSS_DXCH_LPM_SHADOW_DEVS_LIST_STC;

/*
 * Typedef: struct PRV_CPSS_DXCH_LPM_SHADOW_STC
 *
 * Description:
 *      LPM shadow information for an LPM DB
 *
 * Fields:
 *      lpmDbId      - LPM DB id
 *      shadowType   - the shadow type of this LPM DB
 *      shadow       - the shadow information, can be pointer to either
 *                     PRV_CPSS_DXCH_LPM_TCAM_SHADOW_STC or
 *                     PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC
 *
 */
typedef struct PRV_CPSS_DXCH_LPM_SHADOW_STCT
{
    GT_U32                                       lpmDbId;
    PRV_CPSS_DXCH_LPM_SHADOW_TYPE_ENT            shadowType;
    GT_VOID                                      *shadow;
} PRV_CPSS_DXCH_LPM_SHADOW_STC;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssDxChLpmTypesh */

