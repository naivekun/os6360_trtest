/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* lpmTypes.h
*
* DESCRIPTION:
*       LPM definitions
*
* FILE REVISION NUMBER:
*       $Revision: 3 $
*******************************************************************************/

#ifndef __lpmTypesh
#define __lpmTypesh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/generic/ip/cpssIpTypes.h>
#include <cpss/dxCh/dxChxGen/config/cpssDxChCfgInit.h>


/* Number of RAM memory blocks */
#define CPSS_DXCH_LPM_RAM_NUM_OF_MEMORIES_CNS   20
#define CPSS_DXCH_SIP6_LPM_RAM_NUM_OF_MEMORIES_CNS   30

/**
 * @enum CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT
 * @brief
 *      Defines the different types of LPM structures that may be pointed by a
 *      next pointer in an lpm node.
 * @brief
 *  Enumerations:
 *      CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E                            - Regular node.
 *      CPSS_DXCH_LPM_COMPRESSED_1_NODE_PTR_TYPE_E                       - one-line compressed node
 *      CPSS_DXCH_LPM_COMPRESSED_2_NODE_PTR_TYPE_E                       - two-lines compressed node
 *      CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E                        - Next Hop entry.
 *      CPSS_DXCH_LPM_ECMP_ENTRY_PTR_TYPE_E                              - ECMP entry
 *      CPSS_DXCH_LPM_QOS_ENTRY_PTR_TYPE_E                               - QoS entry
 *      CPSS_DXCH_LPM_COMPRESSED_NODE_PTR_TYPE_E                         - node serves till 10 ranges. (Applicable for SIP7)
 *      CPSS_DXCH_LPM_COMPRESSED_UP_TO_7_RANGES_1_LEAF_NODE_PTR_TYPE_E   - this type includes all compressed nodes till 7 ranges with 1 leaf.
 *      CPSS_DXCH_LPM_COMPRESSED_UP_TO_5_RANGES_2_LEAVES_NODE_PTR_TYPE_E - this type includes all compressed nodes till 5 ranges with 2 leaves.
 *      CPSS_DXCH_LPM_COMPRESSED_3_RANGES_3_LEAVES_NODE_PTR_TYPE_E       - this type includes all compressed nodes with 3 ranges and 3 leaves.
 *      CPSS_DXCH_LPM_MULTIPATH_ENTRY_PTR_TYPE_E                         - multipath entry (ECMP or QOS based on global configuration).
 *                                                                         (Applicable for SIP6)
 *
 */
typedef enum
{
    CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E      = 0,
    CPSS_DXCH_LPM_COMPRESSED_1_NODE_PTR_TYPE_E = 1,
    CPSS_DXCH_LPM_COMPRESSED_2_NODE_PTR_TYPE_E = 2,
    CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E  = 3,
    CPSS_DXCH_LPM_ECMP_ENTRY_PTR_TYPE_E        = 4,
    CPSS_DXCH_LPM_QOS_ENTRY_PTR_TYPE_E         = 5,
    CPSS_DXCH_LPM_COMPRESSED_NODE_PTR_TYPE_E   = 6,
    CPSS_DXCH_LPM_COMPRESSED_UP_TO_7_RANGES_1_LEAF_NODE_PTR_TYPE_E   = 7,
    CPSS_DXCH_LPM_COMPRESSED_UP_TO_5_RANGES_2_LEAVES_NODE_PTR_TYPE_E   = 8,
    CPSS_DXCH_LPM_COMPRESSED_3_RANGES_3_LEAVES_NODE_PTR_TYPE_E   = 9,
    CPSS_DXCH_LPM_MULTIPATH_ENTRY_PTR_TYPE_E
} CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT;

/**
* @enum CPSS_DXCH_LPM_LEAF_ENTRY_PRIORITY_TYPE_ENT
 *
*  @brief  Define priority between LPM/PBR/ExactMatch results
*          and FDB result.
*/
typedef enum
{
    /** @brief FDB has higher priority over LPM/PBR/ExactMatch result. */
    CPSS_DXCH_LPM_LEAF_ENTRY_PRIORITY_FDB_E,

    /** @brief LPM/PBR/ExactMatch result has higher priority over FDB result. */
    CPSS_DXCH_LPM_LEAF_ENTRY_PRIORITY_LPM_E,
} CPSS_DXCH_LPM_LEAF_ENTRY_PRIORITY_TYPE_ENT;

/**
 * @struct CPSS_DXCH_LPM_NODE_NEXT_POINTER_STCT
 *
 * @brief Representation of LPM next node pointer in HW
 *
 * @brief Fields:
 *
 *      pointerType       - the type of the next level pointer (node/NH/ECMP)
 *      nextPointer       - the address of the next node. Relevant only when
 *                          pointerType is regular bucket, compressed-1 or
 *                          compressed-2.
 *      range5Index       - the location (index in the node) of the fifth
 *                          address range. Relevant only when pointerType
 *                          is 2-lines compressed.
 *      pointToSipTree    - indicates if the next node is a root of SIP lookup.
 *                          Relevant for MC entries, when pointerType is regular
 *                          bucket, compressed-1 or compressed-2.
 *      ucRPFCheckEnable  - Enable Unicast RPF check for this Entry.
 *      sipSaCheckMismatchEnable -
 *                          Enable Unicast SIP MAC SA match check.
 *      ipv6MCGroupScopeLevel -
 *                          The IPv6 Multicast group scope level.
 *      activityState     - LPM Activity State.
 *                          GT_TRUE: leaf entry is active: packets with given
 *                          Ip address is running.
 *                          GT_FALSE: leaf entry is not active, packets with given
 *                          Ip address is not running.
 *      entryIndex        - the index of the nexthop entry in the bexthop table
 *                          if pointerType is CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E
 *                          or the index of the ECMP entry in the ECMP table.
 *                          if pointerType is CPSS_DXCH_LPM_ECMP_QOS_ENTRY_PTR_TYPE_E.
 */
typedef struct CPSS_DXCH_LPM_NODE_NEXT_POINTER_STCT
{
    CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT pointerType;
    union
    {
        struct
        {
            GT_U32                           nextPointer;
            GT_U32                           range5Index;
            GT_BOOL                          pointToSipTree;
        } nextNodePointer;
        struct
        {
            GT_BOOL                          ucRpfCheckEnable;
            GT_BOOL                          srcAddrCheckMismatchEnable;
            CPSS_IPV6_PREFIX_SCOPE_ENT       ipv6McGroupScopeLevel;
            GT_BOOL                          activityState;
            GT_U32                           entryIndex;
        } nextHopOrEcmpPointer;
    } pointerData;
} CPSS_DXCH_LPM_NODE_NEXT_POINTER_STC;

/**
 * @struct CPSS_DXCH_LPM_REGULAR_NODE_BIT_VECTOR_ENTRY_STCT
 *
 * @brief Representation of regular node bit vector in HW
 *
 * @brief Fields:
 *      bitMap - 24 bits bitmap which goes by this rule:
 *               0 - continue : the current bit location belongs to the previous
 *                              address range
 *               1 - next : the current bit location belongs to a new address
 *                          range
 *      rangeCounter - the total number of address ranges in the previous words.
 *                     On word 0 this field has to be 0.
 */
typedef struct CPSS_DXCH_LPM_REGULAR_NODE_BIT_VECTOR_ENTRY_STCT
{
    GT_U32 bitMap;
    GT_U8  rangeCounter;
} CPSS_DXCH_LPM_REGULAR_NODE_BIT_VECTOR_ENTRY_STC;

/**
 * @union CPSS_DXCH_LPM_NODE_RANGE_SELECTION_UNT
 *
 * @brief
 *      Representation of the lpm node range selection section.
 *      The section can be from one of the 3 following types:
 *      compressed-1 - the range selection section contains 4 ranges (1 word)
 *      compressed-2 - the range selection section contains 8 ranges (2 words)
 *      regular - the range selection section contains 11 bit vector entries
 *               (11 words)
 *
 * @brief Fields:
 *      ranges1_4 - the range index array (table) of ranges 1 to 4
 *                  (for compressed 1 or compressed 2)
 *      ranges6_9 - the range index array (table) of ranges 6 to 9 (for
 *                  compressed 2)
 *      bitVectorEntry - the 11 bit vector entries for a regular node
 */
typedef union
{
    struct
    {
        GT_U8   ranges1_4[4];
    }compressed1BitVector;

    struct
    {
        GT_U8   ranges1_4[4];
        GT_U8   ranges6_9[4];
    }compressed2BitVector;

    struct
    {
        CPSS_DXCH_LPM_REGULAR_NODE_BIT_VECTOR_ENTRY_STC bitVectorEntry[11];
    }regularBitVector;
} CPSS_DXCH_LPM_NODE_RANGE_SELECTION_UNT;

/**
 * @struct CPSS_DXCH_LPM_LEAF_ENTRY_STC
 *
 * @brief LPM leaf entry used for policy based routing
 *
 * @brief Fields:
 *      entryType                - Route entry type
 *                                 APPLICABLE VALUES:
 *                                  CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E
 *                                  CPSS_DXCH_LPM_ECMP_ENTRY_PTR_TYPE_E-(NOT APPLICABLE DEVICES:Falcon)
 *                                  CPSS_DXCH_LPM_QOS_ENTRY_PTR_TYPE_E-(NOT APPLICABLE DEVICES:Falcon)
 *                                  CPSS_DXCH_LPM_MULTIPATH_ENTRY_PTR_TYPE_E-(APPLICABLE DEVICES:Falcon)
 *  
 *      index                    - If entryType is
 *                                 CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E then
 *                                 this is the pointer to the route entry
 *                                 (APPLICABLE RANGES: 0..24575)
 *                                 Otherwise it is the pointer to ECMP/QoS entry
 *                                 (APPLICABLE RANGES: 0..12287)
 *  
 *      ucRPFCheckEnable         - Enable unicast RPF check for this entry
 *  
 *      sipSaCheckMismatchEnable - Enable unicast SIP MAC SA match check
 *  
 *      ipv6MCGroupScopeLevel    - the IPv6 Multicast group scope level
 *  
 *      priority                 - resolution priority between PBR/ExactMatch and FDB match results
 *                                 (APPLICABLE DEVICES: Falcon)
 *      pointToSip               - if points to SIP tree
 *      pointerToSip             - SIP tree offset in LPM memory
 *      nextNodeType             - SIP tree node type
 *
 * @brief Comments:
 *      index can point to either NH or ECMP entry. entryType determines the
 *      type of pointer: if entryType is CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E
 *      then it points to NH, else - to ECMP entry.
 */
typedef struct CPSS_DXCH_LPM_LEAF_ENTRY_STCT
{
    CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT         entryType;
    GT_U32                                      index;
    GT_BOOL                                     ucRPFCheckEnable;
    GT_BOOL                                     sipSaCheckMismatchEnable;
    CPSS_IPV6_PREFIX_SCOPE_ENT                  ipv6MCGroupScopeLevel;
    CPSS_DXCH_LPM_LEAF_ENTRY_PRIORITY_TYPE_ENT  priority;
} CPSS_DXCH_LPM_LEAF_ENTRY_STC;


/**
* @enum CPSS_DXCH_LPM_RAM_BLOCKS_ALLOCATION_METHOD_ENT
 *
 * @brief Defines the method of LPM blocks allocation
*/
typedef enum{

    /** @brief the blocks are allocated dynamically, memory blocks are never shared
     *  among different octets of the same protocol
     */
    CPSS_DXCH_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITHOUT_BLOCK_SHARING_E  = 0,

    /** @brief the blocks are allocated dynamically, memory blocks are shared among
     *  different octets of the same protocol in case of missing free block.
     *  This can cause violation of full wire-speed.
     */
    CPSS_DXCH_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITH_BLOCK_SHARING_E     = 1

} CPSS_DXCH_LPM_RAM_BLOCKS_ALLOCATION_METHOD_ENT;


/**
 * @struct CPSS_DXCH_LPM_RAM_CONFIG_STCT
 *
 * @brief Memory configurations for RAM based LPM shadow
 *
 * @brief Fields:
 *      numOfBlocks             - the number of RAM blocks that LPM uses.
 *                                (APPLICABLE VALUES: 1..20)
 *      blocksSizeArray         - array that holds the sizes of the RAM blocks in bytes
 *      blocksAllocationMethod  - the method of blocks allocation
 *      lpmMemMode              - the mode of LPM MEM.(APPLICABLE DEVICES: Bobcat3; Aldrin2; Falcon)
 *      sharedMemCnfg           - shared memory configuration mode. (APPLICABLE DEVICES: Falcon)
 *      maxNumOfPbrEntries      - max number of LPM leafs that can be allocated in the RAM for policy
 *                                based routing (APPLICABLE DEVICES: Falcon)
 *
 */
typedef struct CPSS_DXCH_LPM_RAM_CONFIG_STCT
{
    GT_U32                                          numOfBlocks;
    GT_U32                                          blocksSizeArray[CPSS_DXCH_SIP6_LPM_RAM_NUM_OF_MEMORIES_CNS];
    CPSS_DXCH_LPM_RAM_BLOCKS_ALLOCATION_METHOD_ENT  blocksAllocationMethod;
    CPSS_DXCH_LPM_RAM_MEM_MODE_ENT                  lpmMemMode;
    CPSS_DXCH_CFG_SHARED_TABLE_MODE_ENT             sharedMemCnfg;
    GT_U32                                          maxNumOfPbrEntries;

} CPSS_DXCH_LPM_RAM_CONFIG_STC;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __lpmTypesh */


