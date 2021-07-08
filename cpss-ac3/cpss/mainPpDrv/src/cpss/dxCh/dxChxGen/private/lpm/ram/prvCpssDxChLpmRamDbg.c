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
* @file prvCpssDxChLpmRamDbg.c
*
* @brief the CPSS LPM Debug Engine support.
*
* @version   23
********************************************************************************
*/

#include <cpss/dxCh/dxChxGen/private/lpm/ram/prvCpssDxChLpmRamTypes.h>
#include <cpss/dxCh/dxChxGen/private/lpm/ram/prvCpssDxChLpmRam.h>
#include <cpss/dxCh/dxChxGen/private/lpm/ram/prvCpssDxChLpmRamTrie.h>
#include <cpssCommon/private/prvCpssMath.h>
#include <cpssCommon/private/prvCpssDevMemManager.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/dxCh/dxChxGen/private/lpm/ram/prvCpssDxChLpmRamUc.h>
#include <cpss/dxCh/dxChxGen/private/lpm/ram/prvCpssDxChLpmRamMc.h>
#include <cpssCommon/private/prvCpssSkipList.h>
#include <cpss/dxCh/dxChxGen/private/lpm/ram/prvCpssDxChLpmRamDbg.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/private/lpm/hw/prvCpssDxChLpmHw.h>
#include <cpss/dxCh/dxChxGen/private/lpm/prvCpssDxChLpmUtils.h>


#define LPM_SHADOW_VALIDATION_ERROR_MAC(returnOnFailure,retVal,entity,addr,message) \
{                                                                              \
    cpssOsPrintf("Error on %s address 0x%p: %s\n", entity, addr, message);     \
    retVal = GT_FAIL;                                                          \
    if (returnOnFailure == GT_TRUE)                                            \
    {                                                                          \
        return retVal;                                                         \
    }                                                                          \
}

#define LPM_SHADOW_HW_SYNC_VALIDATION_ERROR_MAC(returnOnFailure,retVal,entity,addr,hwAddr,message)           \
{                                                                                                            \
    cpssOsPrintf("Error on %s shadow address 0x%p, HW node address 0x%p: %s\n", entity, addr,hwAddr,message);\
    retVal = GT_FAIL;                                                                                        \
    if (returnOnFailure == GT_TRUE)                                                                          \
    {                                                                                                        \
        return retVal;                                                                                       \
    }                                                                                                        \
}

/*
When lpm memory mode is set to half memory mode ,SW required to duplicate entries.
This is the offset of the duplication.
For example if line number written is X ,then this line will also be written to index X+PRV_CPSS_DXCH_LPM_RAM_BC3_HALF_MEM_MODE_OFFSET_MAC
*/
#define PRV_CPSS_DXCH_LPM_RAM_DBG_BC3_HALF_MEM_MODE_OFFSET_MAC(_devNum) (PRV_CPSS_DXCH_PP_MAC(_devNum)->hwInfo.lpm.numOfLinesInBlock>>1)

static GT_U8 baseAddr[PRV_CPSS_DXCH_LPM_MAX_NUM_OF_OCTETS_FOR_ADDRESS_CNS] = {0};
static GT_U8 baseGroupAddr[PRV_CPSS_DXCH_LPM_MAX_NUM_OF_OCTETS_FOR_ADDRESS_CNS] = {0};
static GT_U32 grpPrefix = 0;
static GT_BOOL InSrcTree = GT_FALSE;
static GT_U8 *baseGroupAddrPtr = NULL;
static GT_U32 numOfErrors = 0;

typedef struct
{
     GT_U32 lpmLinesPerOctet[2*PRV_CPSS_DXCH_LPM_MAX_NUM_OF_OCTETS_FOR_ADDRESS_CNS];   /* count the total number of lpm lines per octet */
     GT_U32 bucketPerOctet[2*PRV_CPSS_DXCH_LPM_MAX_NUM_OF_OCTETS_FOR_ADDRESS_CNS];     /* count total number of buckets per octet */
     GT_U32 bucketPerType[PRV_CPSS_DXCH_LPM_PROTOCOL_LAST_E];       /* count total number of buckets per bucket type */
     GT_U32 bucketSizePerType[4];   /* count total buckets size per bucket type */
     GT_U32 bucketTypePerOctet[2*PRV_CPSS_DXCH_LPM_MAX_NUM_OF_OCTETS_FOR_ADDRESS_CNS][PRV_CPSS_DXCH_LPM_PROTOCOL_LAST_E]; /* count total bucket per octect per type */
     GT_U32 lpmLinesUsed[PRV_CPSS_DXCH_LPM_RAM_NUM_OF_MEMORIES_FALCON_CNS];  /* LPM lines used in each LPM block by specific virtual router */
     GT_U32 numOfNonDefaultNextHopPointers;  /* number of pointers to route entry with base >= 3 (route entries 0-2 typicaly used for defaults) */
     GT_BOOL printRanges;

}prvLpmDumpPrintInfo_STC;

typedef struct
{
     GT_U32  nodeCounterPerTypeSip6[4];   /* count total number of nodes per node type ( types can be: leaf=1,regular=2,comp=3. 0=empty is not used )*/
     GT_U32  nodeCounterPerTypePerOctetSip6[2*PRV_CPSS_DXCH_LPM_MAX_NUM_OF_OCTETS_FOR_ADDRESS_CNS][6]; /* count total number of nodes per octect per
                                                                                               node type (leaf,regular,comp,embedded_1,embedded_2,embedded_3) */
     GT_U32  lpmLinesUsedSip6[PRV_CPSS_DXCH_LPM_RAM_NUM_OF_MEMORIES_FALCON_CNS];  /* LPM lines used in each LPM block by specific virtual router */
     GT_U32  numOfNonDefaultNextHopPointersSip6;  /* number of pointers to route entry with base >= 3 (route entries 0-2 typicaly used for defaults) */
     GT_BOOL printRangesSip6;
}prvLpmDumpPrintInfoSip6_STC;

/* lpm db skip list ptr */
extern GT_VOID *prvCpssDxChIplpmDbSL;

extern GT_VOID * prvCpssSlSearch
(
    IN GT_VOID        *ctrlPtr,
    IN GT_VOID        *dataPtr
);

/* index of RAM (0 to 3) to use with a IPv4 UC/MC LPM engine */
CPSS_TBD_BOOKMARK /* the memory mapping will be decided by the application */
static GT_U8 addrLookupMemIpv4Array[PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS] = {0, 1, 2, 3};

/* index of RAM (0 to 15) to use with a IPv6 UC/MC LPM engine */
CPSS_TBD_BOOKMARK /* the memory mapping will be decided by the application */
static GT_U8 addrLookupMemIpv6Array[PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS] = {4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19};

/* index of RAM (0 to 2) to use with a FCoE LPM engine */
CPSS_TBD_BOOKMARK /* the memory mapping will be decided by the application */
static GT_U8 addrLookupMemFcoeArray[PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_FCOE_PROTOCOL_CNS] = {0, 0, 0};

/* max depth according to type: 4 for IPv4, 16 for IPv6, 3 for FCoE */
#define PRV_DXCH_LPM_RAM_DBG_MAX_DEPTH_CHECK_MAC(protocol)   \
    ((GT_U32)(((protocol) == PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E) ? PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS:  \
             (((protocol) == PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E) ? PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS : \
                                                                  PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_FCOE_PROTOCOL_CNS)))


#define PRV_DXCH_LPM_RAM_DBG_BC3_AGING_BIT_LOCATION_CNS 6


/* forward declaration */
static GT_STATUS prvCpssDxChLpmRamDbgBucketShadowValidityCheck
(
    IN  PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC          *shadowPtr,
    IN  GT_U32                                    vrId,
    IN  PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC   *bucketPtr,
    IN  GT_U8                                     level,
    IN  CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT       expectedNextPointerType,
    IN  GT_U8                                     numOfMaxAllowedLevels,
    IN  CPSS_UNICAST_MULTICAST_ENT                prefixType,
    IN  PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT      protocol,
    IN  GT_U32                                    expectedPointingRangeMemAddr,
    IN  GT_BOOL                                   returnOnFailure
);
/* forward declaration */
static GT_STATUS prvCpssDxChLpmRamDbgBucketHwShadowSyncValidityCheck
(
    IN  GT_U8                                     devNum,
    IN  PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC          *shadowPtr,
    IN  GT_U32                                    vrId,
    IN  PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC   *bucketPtr,
    IN  GT_U8                                     level,
    IN  CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT       expectedNextPointerType,
    IN  GT_U8                                     numOfMaxAllowedLevels,
    IN  CPSS_UNICAST_MULTICAST_ENT                prefixType,
    IN  PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT      protocol,
    IN  GT_U32                                    expectedPointingRangeMemAddr,
    IN  GT_BOOL                                   returnOnFailure,
    IN  GT_BOOL                                   isRootBucket
);
/* forward declaration */
static GT_STATUS prvCpssDxChLpmRamDbgBucketShadowValidityCheckSip6
(
    IN  PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC          *shadowPtr,
    IN  GT_U32                                    vrId,
    IN  PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC   *bucketPtr,
    IN  GT_U8                                     level,
    IN  CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT       expectedNextPointerType,
    IN  GT_U8                                     numOfMaxAllowedLevels,
    IN  CPSS_UNICAST_MULTICAST_ENT                prefixType,
    IN  PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT      protocol,
    IN  GT_U32                                    expectedPointingRangeMemAddr,
    IN  GT_BOOL                                   returnOnFailure
);
/* forward declaration */
static GT_STATUS prvCpssDxChLpmRamDbgBucketHwShadowSyncValidityCheckSip6
(
    IN  GT_U8                                     devNum,
    IN  PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC          *shadowPtr,
    IN  GT_U32                                    vrId,
    IN  PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC   *bucketPtr,
    IN  GT_U32                                    hwBucketDataArr[],
    IN  GT_U8                                     level,
    IN  CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT       expectedNextPointerType,
    IN  GT_U8                                     numOfMaxAllowedLevels,
    IN  CPSS_UNICAST_MULTICAST_ENT                prefixType,
    IN  PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT      protocol,
    IN  GT_BOOL                                   returnOnFailure,
    IN  GT_BOOL                                   isRootBucket
);

static GT_STATUS compareBucket(GT_U8 devNum,GT_U32 hwAddr,GT_U32 *firstBucketDataPtr ,GT_U32 * secondBucketDataPtr,
                                   GT_U32 numOfRanges,CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT     nodeType ,GT_BOOL ignoreAgingBit);


static GT_STATUS outputAddress(GT_U8 *address, GT_U32 bytes2Print, PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT protocol)
{
    while(bytes2Print > 1)
    {
        if ((protocol == PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E) ||
            (protocol == PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E))
        {
            cpssOsPrintf("%03d.%03d.", *address, *(address+1));
        }
        else /* PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E */
        {
            cpssOsPrintf("%02X%02X:", *address, *(address+1));
        }

        address+=2;
        bytes2Print-=2;
    }

    if (bytes2Print > 0)
    {
        if ((protocol == PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E) ||
            (protocol == PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E))
        {
            cpssOsPrintf("%03d", *address);
        }
        else /* PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E */
        {
            cpssOsPrintf("%02X", *address);
        }
    }

    return GT_OK;
}

void prefixBase2Address(GT_U8 *baseAddress, GT_U32 prefixBase, GT_U32 notZeroMask,
                     GT_U8 *address)
{
    GT_U8 i;
    if(notZeroMask == 0)  /* The mask is zero */
        prefixBase += 8;

    for (i = 0; i < PRV_CPSS_DXCH_LPM_MAX_NUM_OF_OCTETS_FOR_ADDRESS_CNS; i++)
    {
        if ((GT_U32)((i+1)*8) <= prefixBase)
        {
            address[i] = baseAddress[i];
        }
        else
        {
            address[i] = 0;
        }
    }
}

GT_U32 getBaseAddrFromHW(CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT type,
                         GT_U32 index,
                         GT_U32 *hwData,GT_U32 bucketPtr)
{
    GT_U32 baseAddr = 0;
    GT_U32 i,hwIdx,tmp;

    switch(type)
    {
    case CPSS_DXCH_LPM_COMPRESSED_1_NODE_PTR_TYPE_E:
        if(index == 0)
            return 0;
        baseAddr = (hwData[0] >> (8 * (index - 1))) & 0xFF;
        break;

    case CPSS_DXCH_LPM_COMPRESSED_2_NODE_PTR_TYPE_E:
        if(index == 0)
            return 0;
        if(index == 5)
            return ((bucketPtr >> 5) & 0xFF);
        hwIdx = (index < 5)? index : (index -5);
        if(index > 5)
            baseAddr = (hwData[1] >> (8 * (hwIdx - 1))) & 0xFF;
        else
            baseAddr = (hwData[0] >> (8 * (hwIdx - 1))) & 0xFF;
        break;

    case CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E:
        i = 0;
        hwIdx = 0;

        while(i <= index)
        {
            baseAddr = 0;
            tmp = hwData[hwIdx] & 0xFFFFFF;
            while((tmp != 0) && (i <= index))
            {
                if((tmp & 0x1) != 0)
                {
                    i++;
                }
                baseAddr++;
                tmp = (tmp >> 1) & 0xFFFFFF;
            }
            hwIdx++;
        }

        hwIdx--;
        baseAddr += (hwIdx * 24) - 1;
        break;

    default:
        break;
    }
    return baseAddr;
}

GT_U32 getEntryByPrefixFromHW(GT_U8 octet,CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT type,
                              GT_U32 *hwData, GT_U32 bucketPtr)
{
    GT_U32 baseAddr = 0;
    GT_U32 i,counter,prevCount,hwIdx,tmp,fromWhere;
    GT_U32 lastAddr = 0;
    hwIdx = 0;

    switch(type)
    {
    case CPSS_DXCH_LPM_COMPRESSED_1_NODE_PTR_TYPE_E:
        for (counter = 0; counter < 4; counter++)
        {
            baseAddr = (hwData[0] >> (8 * counter)) & 0xFF;
            if ((baseAddr > octet) || (lastAddr >= baseAddr))
            {
                break;
            }
            else
            {
                lastAddr = baseAddr;
            }
        }

        hwIdx = counter + 1;
        break;

    case CPSS_DXCH_LPM_COMPRESSED_2_NODE_PTR_TYPE_E:
        baseAddr = ((bucketPtr >> 0x5) & 0xFF);
        lastAddr = baseAddr;
        if (baseAddr > octet)
        {
            prevCount = 1;
            lastAddr = 0;
            tmp = hwData[0];
        }
        else
        {
            prevCount = 6;
            tmp = hwData[1];
        }

        for (counter = 0; counter < 4; counter++)
        {
            baseAddr = (tmp >> (8 * counter)) & 0xFF;

            if ((baseAddr > octet) || (lastAddr >= baseAddr))
            {
                break;
            }
            else
            {
                lastAddr = baseAddr;
            }
        }

        hwIdx = counter + prevCount;
        break;

    case CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E:
        counter = 0;
        prevCount = hwData[octet / 24] >> 24;
        tmp = hwData[octet / 24] ;
        fromWhere = octet%24;
        for (i = 0; i <= fromWhere; i++)
        {
            counter += (tmp & 0x01);
            tmp >>= 1;
        }

        hwIdx = prevCount + counter;

        /* note that the bit vector counter (8 msb) contains the offset of the
           range (in lines) from current bit vector word; refer to the function
           getMirrorBucketDataAndUpdateRangesAddress for details */
        hwIdx += (octet / 24);
        hwIdx -= 10;

        break;

    default:
        break;
    }

    return hwIdx;
}

GT_U32 getNumOfRangesFromHW(CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT type,GT_U32 *hwData, GT_U32 bucketPtr)
{
    return getEntryByPrefixFromHW(255, type, hwData, bucketPtr);
}

/* the function build and arrays of all data related to the ranges.
   it is build according to the HW data and the nodeType */
GT_STATUS getNumOfRangesFromHWSip6(PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT protocol,
                                   GT_U32 depth,
                                   PRV_CPSS_DXCH_LPM_CHILD_TYPE_ENT nodeType,
                                   GT_U32 *hwData,
                                   GT_U32 hwAddr,
                                   GT_U32 nodeChildAddressesArr[6],
                                   GT_U32 nodeTotalChildTypesArr[6][3],
                                   GT_U32 nodeTotalLinesPerTypeArr[6][3],
                                   GT_U32 nodeTotalBucketPerTypesArr[6][3],
                                   GT_U32 *numOfRangesPtr,
                                   GT_U32 *rangesPtr,
                                   GT_U32 *rangesTypePtr,
                                   GT_U32 *rangesTypeIndexPtr,
                                   GT_U32 *rangesBitLinePtr)
{
    GT_U32 baseAddr = 0;
    GT_U32 lastAddr = 0;
    GT_U32 childType = 0;

    GT_U32 i,j;
    GT_U32 numOfRangesCounter = 0;  /* number of bits in "on" state in the bit vector=number of ranges */
    GT_U32 numOfRangesCounterInSingleLine = 0;  /* number of bits in "on" state in a single bit vector line */
    GT_U32 numOfLeafCounter = 0;    /* number of bits in "on" state of type leaf */
    GT_U32 numOfRegularCounter = 0; /* number of bits in "on" state of type regular */
    GT_U32 numOfCompCounter = 0;    /* number of bits in "on" state of type compress */

    GT_U32 value;
    GT_U32 numOfLines=0;
    GT_U32 numberOfEmbeddedLeafs;
    GT_U32 numberOfEmbeddedLeafsTreated=0;
    GT_U32 maxNumOfRangesInSingleLine = 0; /* can be 44 for regular, 3,5,7,10 for compressed */
    GT_U32 numOfOffsetBitsPerRange = 8; /* 8 bits to represent child_offset 0-255 */
    GT_U32 numOfTypetBitsPerRange = 2;  /* 2 bits to represent child_type 0,1,2,3 */
    GT_U32 tempHwDataSingleLineArr[PRV_CPSS_DXCH_LPM_RAM_FALCON_SIZE_OF_LPM_ENTRY_IN_WORDS_CNS];
                                             /* each regular line is a wide one - 114 bits
                                                bit 0-1    : #numOf_leafs fixed 0
                                                bit 2-5    : reserved
                                                bit 5-93   : bit vector for 44 ranges, each range is 2 bits
                                                bit 94-114 : 20 bits for Child_pointer

                                                each compressed line is also 114 bits - see bellow */

    switch(nodeType)
    {
    case PRV_CPSS_DXCH_LPM_CHILD_COMPRESSED_TYPE_E:

        /* compress occupy only one line in HW */
        cpssOsMemCpy(tempHwDataSingleLineArr, hwData, sizeof(tempHwDataSingleLineArr));

        U32_GET_FIELD_IN_ENTRY_MAC(tempHwDataSingleLineArr,0,20,value);/* bits 0-19 child pointer */
        nodeChildAddressesArr[0]= value;

        /* check if we have embedded leafs */
        /* bit 113-112 Embedded_leaf */
        U32_GET_FIELD_IN_ENTRY_MAC(tempHwDataSingleLineArr,112,2,value);
        numberOfEmbeddedLeafs = value;

        numOfLines = PRV_CPSS_DXCH_LPM_RAM_FALCON_BUCKET_SIZE_COMPRESSED_CNS;

        switch (numberOfEmbeddedLeafs)
        {
        case 0:
            /* no embedded leafs */

            /* each compress line is a wide one - 114 bits
            bit 112-113    : #numOf_leafs in case of embedded leafs, in this case it will be 0
            bit 20-111     : bit vector for 10 ranges, each range is 12 bits,
                             2 bits for type and 10 bits for offset value
            bit 0-19       : 20 bits for Child_pointer */
            maxNumOfRangesInSingleLine = PRV_CPSS_DXCH_LPM_MAX_RANGES_IN_COMPRESSED_NODE_E;
            break;
        case 1:
            /* 1 embedded leaf */

            /* each compress line is a wide one - 114 bits
            bit 112-113    : #numOf_leafs in case of embedded leafs, in this case it will be 1
            bit 89-11      : Leaf_0, The first leaf in the Structure
            bit 20-81      : bit vector for 7 ranges, each range is 12 bits,
                             2 bits for type and 10 bits for offset value
            bit 0-19       : 20 bits for Child_pointer */

            maxNumOfRangesInSingleLine = PRV_CPSS_DXCH_LPM_MAX_RANGES_IN_EMBEDDED_1_NODE_E;
            break;
        case 2:
            /* 2 embedded leaf */

            /* each compress line is a wide one - 114 bits
            bit 112-113    : #numOf_leafs in case of embedded leafs, in this case it will be 2
            bit 89-11      : Leaf_0, The first leaf in the Structure
            bit 66-88      : Leaf_1, The second leaf in the Structure
            bit 20-61      : bit vector for 5 ranges, each range is 12 bits,
                             2 bits for type and 10 bits for offset value
            bit 0-19       : 20 bits for Child_pointer */

            maxNumOfRangesInSingleLine = PRV_CPSS_DXCH_LPM_MAX_RANGES_IN_EMBEDDED_2_NODE_E;
            break;
        case 3:
            /* 3 embedded leaf */

            /* each compress line is a wide one - 114 bits
            bit 112-113    : #numOf_leafs in case of embedded leafs, in this case it will be 2
            bit 89-11      : Leaf_0, The first leaf in the Structure
            bit 66-88      : Leaf_1, The second leaf in the Structure
            bit 43-65      : Leaf_2, The third leaf in the Structure
            bit 20-41      : bit vector for 3 ranges, each range is 12 bits,
                             2 bits for type and 10 bits for offset value
            bit 0-19       : 20 bits for Child_pointer */

            maxNumOfRangesInSingleLine = PRV_CPSS_DXCH_LPM_MAX_RANGES_IN_EMBEDDED_3_NODE_E;
            break;
        default:
            break;
        }
        break;

    case PRV_CPSS_DXCH_LPM_CHILD_REGULAR_TYPE_E:

        /* each regular line is a wide one - 114 bits
        bit 112-113    : #numOf_leafs fixed 0
        bit 108-111    : reserved
        bit 20-107     : bit vector for 44 ranges, each range is 2 bits
        bit 0-19       : 20 bits for Child_pointer */

        numOfLines = PRV_CPSS_DXCH_LPM_RAM_FALCON_BUCKET_SIZE_REGULAR_BV_CNS;
        maxNumOfRangesInSingleLine = NUMBER_OF_RANGES_IN_SUBNODE;
        numOfOffsetBitsPerRange = 0;
        break;

    case PRV_CPSS_DXCH_LPM_CHILD_LEAF_TYPE_E: /* no ranges */
    default:
        numOfLines = 0;
        maxNumOfRangesInSingleLine = 0;
        numOfOffsetBitsPerRange=0;
        numOfTypetBitsPerRange = 0;
        break;
    }

    /* go over all line of the node and sum its bits that are set to 1,
       check legally of each range  */
    if (nodeType==PRV_CPSS_DXCH_LPM_CHILD_COMPRESSED_TYPE_E)
    {
        /* ranges indexes are ordered in HW from range9 to range0*/
        for (j=0; j<maxNumOfRangesInSingleLine; j++)
        {
            /* each range holds 10 bits
               range0 hold only 2 bits for the type since the offset is known to be 0
               20 is the number of bits for the child_pointer */
            U32_GET_FIELD_IN_ENTRY_MAC(tempHwDataSingleLineArr,
                                       ((maxNumOfRangesInSingleLine-1-j)*10+20),
                                       numOfTypetBitsPerRange,
                                       value);
            childType = value;

            if (j!=0)
            {
                if (childType!=PRV_CPSS_DXCH_LPM_CHILD_EMPTY_TYPE_E)/* bits are not empty --> there is a valid range */
                {
                    U32_GET_FIELD_IN_ENTRY_MAC(tempHwDataSingleLineArr,
                                       ((maxNumOfRangesInSingleLine-1-j)*10+20+numOfTypetBitsPerRange),
                                       numOfOffsetBitsPerRange,
                                       value);
                    baseAddr = value;
                    if ((baseAddr > 255) || (lastAddr >= baseAddr))
                    {
                        cpssOsPrintf("\n The offset values in the compressed node should be in ascending order,"
                                     "illegal range number=%d, offset value=%d\n",(j-1),baseAddr);
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
                    }
                    else
                    {
                        lastAddr = baseAddr;
                    }

                    rangesPtr[numOfRangesCounter]=baseAddr;
                    rangesTypePtr[numOfRangesCounter]=childType;
                    if((childType==PRV_CPSS_DXCH_LPM_CHILD_LEAF_TYPE_E)&&(numberOfEmbeddedLeafsTreated<numberOfEmbeddedLeafs))
                    {
                        /* embedded leafs should not be counted as regular leafs */
                        numberOfEmbeddedLeafsTreated++;
                    }
                    else
                    {
                        nodeTotalChildTypesArr[0][(childType-1)]++; /* update the array holding the child_type of the bit vector line0
                                                                     childType = empty range is not kept in nodeTotalChildTypesArr
                                                                     childType = leaf is held in nodeTotalChildTypesArr[0][0]
                                                                     childType = regular is held in nodeTotalChildTypesArr[0][1]
                                                                     childType = comp is held in nodeTotalChildTypesArr[0][2] */
                    }
                    rangesBitLinePtr[numOfRangesCounter]=0;          /* 0 means the first line */


                    switch (childType)
                    {
                    case PRV_CPSS_DXCH_LPM_CHILD_LEAF_TYPE_E:/* leaf */
                        rangesTypeIndexPtr[numOfRangesCounter]=numOfLeafCounter;
                        numOfLeafCounter++;
                        break;
                    case PRV_CPSS_DXCH_LPM_CHILD_REGULAR_TYPE_E:/* regular */
                        rangesTypeIndexPtr[numOfRangesCounter]=numOfRegularCounter;
                        numOfRegularCounter++;
                        break;
                    case PRV_CPSS_DXCH_LPM_CHILD_COMPRESSED_TYPE_E:/* compress */
                        rangesTypeIndexPtr[numOfRangesCounter]=numOfCompCounter;
                        numOfCompCounter++;
                        break;
                    case PRV_CPSS_DXCH_LPM_CHILD_EMPTY_TYPE_E:
                    default:
                         cpssOsPrintf("\n illegal childType=%d\n",childType);
                         CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG); /* continue to other branches in the tree */
                    }

                    numOfRangesCounter++; /* add a range to the counter */
                }
            }
            else
            {
                if ((childType==PRV_CPSS_DXCH_LPM_CHILD_EMPTY_TYPE_E)||(numOfRangesCounter!=0))

                {
                    cpssOsPrintf("\n childType cannot be empty in the first range, 0 is always the first range in the bucket\n");
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
                }
                /* last range in HW representation ,child_type_0 do not have an offset - the offset value is always 0 */
                rangesPtr[numOfRangesCounter]=0;
                rangesTypePtr[numOfRangesCounter]=childType;
                if((childType==PRV_CPSS_DXCH_LPM_CHILD_LEAF_TYPE_E)&&(numberOfEmbeddedLeafsTreated<numberOfEmbeddedLeafs))
                {
                    /* embedded leafs should not be counted as regular leafs */
                    numberOfEmbeddedLeafsTreated++;
                }
                else
                {
                    nodeTotalChildTypesArr[0][(childType-1)]++; /* update the array holding the child_type of the bit vector line0 */
                }
                rangesBitLinePtr[numOfRangesCounter]=0;          /* 0 means the first line */


                switch (childType)
                {
                case PRV_CPSS_DXCH_LPM_CHILD_LEAF_TYPE_E:/* leaf */
                    rangesTypeIndexPtr[numOfRangesCounter]=numOfLeafCounter;
                    numOfLeafCounter++;
                    break;
                case PRV_CPSS_DXCH_LPM_CHILD_REGULAR_TYPE_E:/* regular */
                    rangesTypeIndexPtr[numOfRangesCounter]=numOfRegularCounter;
                    numOfRegularCounter++;
                    break;
                case PRV_CPSS_DXCH_LPM_CHILD_COMPRESSED_TYPE_E:/* compress */
                    rangesTypeIndexPtr[numOfRangesCounter]=numOfCompCounter;
                    numOfCompCounter++;
                    break;
                case PRV_CPSS_DXCH_LPM_CHILD_EMPTY_TYPE_E:
                default:
                     cpssOsPrintf("\n illegal childType=%d\n",childType);
                     CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG); /* continue to other branches in the tree */
                }

                numOfRangesCounter++; /* add a range to the counter */
            }
        }
    }
    else/* CPSS_DXCH_LPM_CHILD_REGULAR_TYPE_E */
    {
        /* go over all lines of the node (6 lines for regular)
            and sum its bits that are set to 1 */
        for (i=0; i<numOfLines; i++)
        {
            numOfRangesCounterInSingleLine=0;
            numOfLeafCounter=0;
            numOfRegularCounter=0;
            numOfCompCounter=0;

            cpssOsMemCpy(tempHwDataSingleLineArr, (hwData+(i*4)), sizeof(tempHwDataSingleLineArr));

            U32_GET_FIELD_IN_ENTRY_MAC(tempHwDataSingleLineArr,0,20,value);/* bits 0-19 child pointer */
            nodeChildAddressesArr[i]= value;

           /*Formula: 106 + -(2*j): where j (0-43) represents index */
            for (j=0; j<maxNumOfRangesInSingleLine; j++)
            {
                 /* each range holds 2 bits for child_type
                   20 is the number of bits for the child_pointer */
                U32_GET_FIELD_IN_ENTRY_MAC(tempHwDataSingleLineArr,
                                           (106-(j*2)),
                                           numOfTypetBitsPerRange,
                                           value);
                childType = value;
                if (j!=0)
                {
                    if (childType!=PRV_CPSS_DXCH_LPM_CHILD_EMPTY_TYPE_E)/* bits are not empty - there is a valid range  */
                    {
                        rangesPtr[numOfRangesCounter]=  j +  i*maxNumOfRangesInSingleLine;

                        rangesTypePtr[numOfRangesCounter]=childType;
                        nodeTotalChildTypesArr[i][(childType-1)]++; /* update the array holding the child_type of the bit vector line0
                                                                         childType = empty range is not kepps in nodeTotalChildTypesArr
                                                                         childType = leaf is held in nodeTotalChildTypesArr[0][0]
                                                                         childType = regular is held in nodeTotalChildTypesArr[0][1]
                                                                         childType = comp is held in nodeTotalChildTypesArr[0][2] */
                        rangesBitLinePtr[numOfRangesCounter]=i ;    /* 0 means the first line , 5 means the last line*/


                        switch (childType)
                        {
                        case PRV_CPSS_DXCH_LPM_CHILD_LEAF_TYPE_E:/* leaf */
                            rangesTypeIndexPtr[numOfRangesCounter]=numOfLeafCounter;
                            numOfLeafCounter++;
                            break;
                        case PRV_CPSS_DXCH_LPM_CHILD_REGULAR_TYPE_E:/* regular */
                            rangesTypeIndexPtr[numOfRangesCounter]=numOfRegularCounter;
                            numOfRegularCounter++;
                            break;
                        case PRV_CPSS_DXCH_LPM_CHILD_COMPRESSED_TYPE_E:/* compress */
                            rangesTypeIndexPtr[numOfRangesCounter]=numOfCompCounter;
                            numOfCompCounter++;
                            break;
                        case PRV_CPSS_DXCH_LPM_CHILD_EMPTY_TYPE_E:
                        default:
                             cpssOsPrintf("\n illegal childType=%d\n",childType);
                             numOfErrors++;
                             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
                        }
                        numOfRangesCounterInSingleLine++;
                        numOfRangesCounter++; /* add a range to the counter */
                    }
                }
                else
                {
                    if ((childType==PRV_CPSS_DXCH_LPM_CHILD_EMPTY_TYPE_E)||(numOfRangesCounterInSingleLine!=0))
                    {
                        cpssOsPrintf("\n childType cannot be empty in the first range of a regular line,"
                                     " 0 is always the first range in the bit vector line\n");
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
                    }
                    /* last range in HW representation ,child_type_0, child_type_44, child_type_88... do not have an offset
                       - the offset value known 0,44,88 .... */
                    rangesPtr[numOfRangesCounter]= i*maxNumOfRangesInSingleLine;

                    rangesTypePtr[numOfRangesCounter]=childType;
                    nodeTotalChildTypesArr[i][(childType-1)]++; /* update the array holding the child_type of the bit vector line0
                                                                     childType = empty range is not kepps in nodeTotalChildTypesArr
                                                                     childType = leaf is held in nodeTotalChildTypesArr[0][0]
                                                                     childType = regular is held in nodeTotalChildTypesArr[0][1]
                                                                     childType = comp is held in nodeTotalChildTypesArr[0][2] */
                    rangesBitLinePtr[numOfRangesCounter]=i ;    /* 0 means the first line , 5 means the last line*/


                    switch (childType)
                    {
                    case PRV_CPSS_DXCH_LPM_CHILD_LEAF_TYPE_E:/* leaf */
                        rangesTypeIndexPtr[numOfRangesCounter]=numOfLeafCounter;
                        numOfLeafCounter++;
                        break;
                    case PRV_CPSS_DXCH_LPM_CHILD_REGULAR_TYPE_E:/* regular */
                        rangesTypeIndexPtr[numOfRangesCounter]=numOfRegularCounter;
                        numOfRegularCounter++;
                        break;
                    case PRV_CPSS_DXCH_LPM_CHILD_COMPRESSED_TYPE_E:/* compress */
                        rangesTypeIndexPtr[numOfRangesCounter]=numOfCompCounter;
                        numOfCompCounter++;
                        break;
                    case PRV_CPSS_DXCH_LPM_CHILD_EMPTY_TYPE_E:
                    default:
                         cpssOsPrintf("\n illegal childType=%d\n",childType);
                         CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
                    }
                    numOfRangesCounterInSingleLine++;
                    numOfRangesCounter++; /* add a range to the counter */
                }
            }
        }
    }

    if ((protocol != PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E) || ((protocol == PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E) && (depth > 0)))
    {
        if (rangesPtr[1] == 0)
        {
            cpssOsPrintf("\nAt least 2 ranges should be defined!!! Bucket address 0x%x\n",hwAddr);
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }
    }

    *numOfRangesPtr = numOfRangesCounter;

    /* sum number of line from each GON that have Type=nodeChildType */
    for (i=0;i<numOfLines;i++)
    {
        /* calculate number of lines occupied by all the leafs */
        if(nodeTotalChildTypesArr[i][0]%MAX_NUMBER_OF_LEAVES_IN_LPM_LINE_CNS!=0)
        {
            /* one extra non full line */
            nodeTotalLinesPerTypeArr[i][0]+=(1+nodeTotalChildTypesArr[i][0]/MAX_NUMBER_OF_LEAVES_IN_LPM_LINE_CNS);
            nodeTotalBucketPerTypesArr[i][0]+=(nodeTotalChildTypesArr[i][0]);
        }
        else
        {
            nodeTotalLinesPerTypeArr[i][0]+=(nodeTotalChildTypesArr[i][0]/MAX_NUMBER_OF_LEAVES_IN_LPM_LINE_CNS);
            nodeTotalBucketPerTypesArr[i][0]+=(nodeTotalChildTypesArr[i][0]);
        }

        /* calculate number of lines occupied by all the regular */
        /* each regular bucket holds 6 lines */
        nodeTotalLinesPerTypeArr[i][1]+=(nodeTotalChildTypesArr[i][1]*PRV_CPSS_DXCH_LPM_RAM_FALCON_BUCKET_SIZE_REGULAR_BV_CNS);
        nodeTotalBucketPerTypesArr[i][1]+=(nodeTotalChildTypesArr[i][1]);

        /* calculate number of lines occupied by all the compress */
        nodeTotalLinesPerTypeArr[i][2]+=(nodeTotalChildTypesArr[i][2]);
        nodeTotalBucketPerTypesArr[i][2]+=(nodeTotalChildTypesArr[i][2]);
    }

    return GT_OK;
}

GT_VOID printLpmMcGroup(PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT protocol,
                        GT_U8 prefixLength,
                        GT_U8 startAddr,
                        GT_U8 endAddr)
{
    GT_U8  startAddress[PRV_CPSS_DXCH_LPM_MAX_NUM_OF_OCTETS_FOR_ADDRESS_CNS];
    GT_U8  endAddress[PRV_CPSS_DXCH_LPM_MAX_NUM_OF_OCTETS_FOR_ADDRESS_CNS];
    GT_U8  address[PRV_CPSS_DXCH_LPM_MAX_NUM_OF_OCTETS_FOR_ADDRESS_CNS];
    GT_U8  j;
    GT_U32 bytesToPrint;

    cpssOsMemSet(endAddress,0, sizeof(GT_U8) * PRV_CPSS_DXCH_LPM_MAX_NUM_OF_OCTETS_FOR_ADDRESS_CNS);
    cpssOsMemSet(startAddress,0, sizeof(GT_U8) * PRV_CPSS_DXCH_LPM_MAX_NUM_OF_OCTETS_FOR_ADDRESS_CNS);
    cpssOsMemSet(address,0, sizeof(GT_U8) * PRV_CPSS_DXCH_LPM_MAX_NUM_OF_OCTETS_FOR_ADDRESS_CNS);

    /* print group */
    cpssOsPrintf(" Group :  ");
    prefixBase2Address(baseAddr ,prefixLength,1,address);
    for (j = 8; j < prefixLength; j += 8)
    {
        startAddress[(j/8)-1] = address[(j/8)-1];
        endAddress[(j/8)-1] = address[(j/8)-1];
    }
    startAddress[(j/8)-1] = startAddr;
    endAddress[(j/8)-1] = (GT_U8)endAddr;
    if (protocol == PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E)
    {
        bytesToPrint = PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS;
    }
    else    /* PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E */
    {
        bytesToPrint = PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS;
    }
    outputAddress(startAddress, bytesToPrint, protocol);
    cpssOsPrintf(" - ");
    outputAddress(endAddress, bytesToPrint, protocol);
    cpssOsPrintf(" / %02d\n ", prefixLength);
    cpssOsPrintf(" Src tree for this group :\n");

}

GT_VOID printLpmMcGroupSip6(PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT protocol,
                        GT_U8 prefixLength,
                        GT_U8 startAddr,
                        GT_U8 endAddr)
{
    GT_U8  startAddress[PRV_CPSS_DXCH_LPM_MAX_NUM_OF_OCTETS_FOR_ADDRESS_CNS];
    GT_U8  endAddress[PRV_CPSS_DXCH_LPM_MAX_NUM_OF_OCTETS_FOR_ADDRESS_CNS];
    GT_U8  address[PRV_CPSS_DXCH_LPM_MAX_NUM_OF_OCTETS_FOR_ADDRESS_CNS];
    GT_U8  j;
    GT_U32 bytesToPrint;

    cpssOsMemSet(endAddress,0, sizeof(GT_U8) * PRV_CPSS_DXCH_LPM_MAX_NUM_OF_OCTETS_FOR_ADDRESS_CNS);
    cpssOsMemSet(startAddress,0, sizeof(GT_U8) * PRV_CPSS_DXCH_LPM_MAX_NUM_OF_OCTETS_FOR_ADDRESS_CNS);
    cpssOsMemSet(address,0, sizeof(GT_U8) * PRV_CPSS_DXCH_LPM_MAX_NUM_OF_OCTETS_FOR_ADDRESS_CNS);

    /* print group */
    cpssOsPrintf(" Group :  ");
    prefixBase2Address(baseAddr ,prefixLength,1,address);
    for (j = 8; j < prefixLength; j += 8)
    {
        startAddress[(j/8)-1] = address[(j/8)-1];
        endAddress[(j/8)-1] = address[(j/8)-1];
    }
    startAddress[(j/8)-1] = startAddr;
    endAddress[(j/8)-1] = (GT_U8)endAddr;
    if (protocol == PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E)
    {
        bytesToPrint = PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS;
    }
    else    /* PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E */
    {
        bytesToPrint = PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS;
    }
    outputAddress(startAddress, bytesToPrint, protocol);
    cpssOsPrintf(" - ");
    outputAddress(endAddress, bytesToPrint, protocol);
    cpssOsPrintf(" / %02d\n ", prefixLength);
    cpssOsPrintf(" Src tree for this group :\n");

}

GT_STATUS validateBitVectorOfRegularBucket(GT_U32 *bucketDataPtr, GT_U32 hwAddr, GT_U32 basePrefix)
{
    GT_U32 hwBucketAddr;
    GT_U32 bitSum;
    GT_U32 bitsArray;
    GT_U32 sum;
    GT_U32 i;

    hwBucketAddr = bucketDataPtr[0]; /* bitmap pointer */
    bitSum = (hwBucketAddr >> 24) & 0xFF; /* summry of bits in privious array */
    /* For first word, bitsum must be 0 */
    if (bitSum != 0xA)
    {
        cpssOsPrintf("\nFor first word, sum must be 0!!! Bucket address 0x%x\n",hwAddr);
        cpssOsFree(bucketDataPtr);
        numOfErrors++;
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG); /* continue to other branches in the tree */
    }
    /* Bit 0 in word 0 must be set because first range always starts in 0 */
    if ((hwBucketAddr & 0x1) == 0)
    {
        cpssOsPrintf("\nFirst range always starts in 0!!! Bucket address 0x%x\n",hwAddr);
        cpssOsFree(bucketDataPtr);
        numOfErrors++;
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG); /* continue to other branches in the tree */
    }

    for (i = 0; i < 10; i++)
    {
        hwBucketAddr = bucketDataPtr[i*PRV_CPSS_DXCH_LPM_RAM_SIZE_OF_LPM_ENTRY_IN_WORDS_CNS];
        bitsArray = hwBucketAddr & 0xFFFFFF;
        bitSum = (bucketDataPtr[(i+1)*PRV_CPSS_DXCH_LPM_RAM_SIZE_OF_LPM_ENTRY_IN_WORDS_CNS] >> 24) & 0xFF;
        sum = 0; /* number of set bits from 24 bits array */
        while (bitsArray != 0)
        {
            if (bitsArray & 0x1)
            {
                sum++; /* count number of set bits in array */
            }
            bitsArray = bitsArray >> 1;
        }
        sum += (((bucketDataPtr[i*PRV_CPSS_DXCH_LPM_RAM_SIZE_OF_LPM_ENTRY_IN_WORDS_CNS] >> 24) & 0xFF) - 1);

        /* For each word in the bitmap other than first
           Sum = previous_sum + number of bits in previous word */
        if (sum != bitSum)
        {
            cpssOsPrintf("\nTotal sum is different then total number of set bits!!! Bucket address 0x%x\n",hwAddr);
            cpssOsFree(bucketDataPtr);
            numOfErrors++;
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG); /* continue to other branches in the tree */
        }
    }
    hwBucketAddr = bucketDataPtr[10*PRV_CPSS_DXCH_LPM_RAM_SIZE_OF_LPM_ENTRY_IN_WORDS_CNS]; /* 11'th range of bitmap */
    /* On last word, last 8 bits must be 0 (those do not count as 11*24) */
    if (((hwBucketAddr >> 16) & 0xFF) != 0)
    {
        cpssOsPrintf("\nOn last word, last 8 bits must be 0 !!! Bucket address 0x%x\n",hwAddr);
        cpssOsFree(bucketDataPtr);
        numOfErrors++;
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG); /* continue to other branches in the tree */
    }
    bitsArray = hwBucketAddr & 0xFFFFFF;
    sum = 0;
    while (bitsArray != 0)
    {
        sum++; /* count number of set bits in array */
        bitsArray = bitsArray >> 1;
    }
    bitSum += sum;
    /* Regular bucket is for 11-255 ranges so there must be at least 11 bits != 0
       exception here for root bucket, which can be regular or compressed 1.
       If it regular then it will have ranges 6-255 -> less then 11 bits */
    if (bitSum < 11)
    {
        if (!((InSrcTree == GT_FALSE) && (basePrefix == 8)))
        {
            cpssOsPrintf("\nAt least 6 ranges should be defined!!! Bucket address 0x%x\n",hwAddr);
            cpssOsFree(bucketDataPtr);
            numOfErrors++;
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG); /* continue to other branches in the tree */
        }
    }
    return GT_OK;
}


/* function that vallidate the child pointer taken from the bit vector or from the leaf */
GT_STATUS validateChildPtrSip6(GT_U8 devNum, GT_U32 hwAddr, GT_U32 childPointerOffset)
{
    /* check that the childPointerAddr points to a leggal address
       in HW according to the LPM banks memory configuration
       meanning can not be more than numOfLinesInBlock*/

    if ((childPointerOffset%PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.totalNumOfLinesInBlockIncludingGap)>=
        PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.numOfLinesInBlock)
    {
        cpssOsPrintf("\n illegal childPointerOffset!!! Bucket address 0x%x\n", hwAddr);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }
    return GT_OK;
}

/* validate the child type values in the bit vector ranges */
GT_STATUS validateChildTypeSip6(GT_U32 hwAddr, GT_U32 childType)
{
    if (childType>3)
    {
        cpssOsPrintf("\n illegal childType!!! Bucket address 0x%x\n", hwAddr);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }
    return GT_OK;
}

/* validate the child offset values in the bit vector ranges */
GT_STATUS validateChildOffsetSip6(GT_U32 hwAddr, GT_U32 childOffset)
{
    if (childOffset>255)
    {
        cpssOsPrintf("\n illegal childOffset!!! Bucket address 0x%x\n", hwAddr);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }
    return GT_OK;
}

/* function to validate a Leaf */
GT_STATUS validateLeafSip6(GT_U8 devNum,  GT_U32 hwAddr, GT_U32 embeddedLeafVal,
                          GT_U32 *entryTypePtr, GT_U32 *leafTypePtr, GT_U32 *nhPointerPtr,
                          GT_U32 *nextBucketPointerPtr, GT_U32 *nextNodeTypePtr,GT_BOOL isUc)
{
    GT_STATUS ret = GT_OK;
    PRV_CPSS_DXCH_LPM_ECMP_ENTRY_STC ecmpEntry;
    GT_U32 nextNodeType; /* bit 22
                            Defines the next node entry type
                            0x0 = Regular
                            0x1 = Compressed
                            valid if EntryType="Trigger" */

    GT_U32 nhPointer;       /* bits 21-7
                            Pointer to the Next Hop Table or the ECMP Table,
                            based on the Leaf Type.
                            valid if EntryType="Leaf" */
    GT_U32 nhAdditionalData;/* bits 6-3
                            Contains a set of UC security check enablers
                            and IPv6 MC scope level:
                            1. [6..6] UC SIP SA Check Mismatch Enable
                            2. [5..5] UC RPF Check Enable
                            3. [4..3] IPv6 MC Group Scope Level[1..0]
                            valid if EntryType="Leaf" */
    GT_U32 nextBucketPointer;/* bits 21-2
                                Pointer to the next bucket on the (G,S) lookup
                                valid if EntryType="Trigger" */
    GT_U32 leafType; /* bit 2
                        The leaf entry type
                        0x0 = Regular Leaf
                        0x1 = Multipath Leaf
                        valid if EntryType="Leaf" */
    GT_U32 entryType;/* bit 1
                        In the process of (*,G) lookup.
                        When an entry has this bit set, the (*, G) lookup
                        terminates with a match at the current entry, and (S,
                        G) SIP based lookup is triggered.
                        Note that in such case, head of trie start address for
                        the (S,G) lookup is obtained from the the (*, G)
                        lookup stage.
                         0x0 = Leaf
                         0x1 = Trigger; Trigger IP MC S+G Lookup */
    GT_U32 lpmOverEmPriority;/* bit 0
                                Define the resolution priority between LPM and
                                Exact Match results
                                 0x0 = Low; Exact Match has priority over LPM result
                                 0x1 = High;LPM result has priority over Exact Match */
    GT_U32 routeEntriesNum;

    U32_GET_FIELD_IN_ENTRY_MAC(&embeddedLeafVal,0,1,lpmOverEmPriority);
    U32_GET_FIELD_IN_ENTRY_MAC(&embeddedLeafVal,1,1,entryType);
    U32_GET_FIELD_IN_ENTRY_MAC(&embeddedLeafVal,2,1,leafType);
    U32_GET_FIELD_IN_ENTRY_MAC(&embeddedLeafVal,2,20,nextBucketPointer);
    U32_GET_FIELD_IN_ENTRY_MAC(&embeddedLeafVal,3,4,nhAdditionalData);
    U32_GET_FIELD_IN_ENTRY_MAC(&embeddedLeafVal,7,15,nhPointer);
    U32_GET_FIELD_IN_ENTRY_MAC(&embeddedLeafVal,22,1,nextNodeType);

    if(entryType==PRV_CPSS_DXCH_LPM_LEAF_ENTRY_TYPE_LEAF_E)/* Leaf */
    {
        if (leafType == PRV_CPSS_DXCH_LPM_LEAF_REGULAR_TYPE_ENT) /*Regular Leaf*/
        {
            /* Number of entries in NH table */
            routeEntriesNum = PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.routerNextHop;
        }
        else/* CPSS_DXCH_LPM_LEAF_MULTIPATH_TYPE_E */
        {
            /* Number of entries in ECMP/QoS table */
            routeEntriesNum = PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.ecmpQos;
        }

        /* NH base address must be within the NH table */
        if (nhPointer >= routeEntriesNum)
        {
            cpssOsPrintf("\nNH pointer not in range of NH table!!! NH address %d\n",nhPointer);
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }
        if (leafType == PRV_CPSS_DXCH_LPM_LEAF_MULTIPATH_TYPE_E)/*Multipath Leaf*/
        {
            ret = prvCpssDxChLpmHwEcmpEntryRead(devNum, nhPointer, &ecmpEntry);
            if (ret != GT_OK)
            {
                cpssOsPrintf("\nFailed to read ECMP entry %d\n",nhPointer);
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
            }

            routeEntriesNum = PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.routerNextHop;
            if (ecmpEntry.nexthopBaseIndex + ecmpEntry.numOfPaths >= routeEntriesNum)
            {
                cpssOsPrintf("\nECMP entry points to illegal nexthop index %d\n",nhPointer);
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
            }
        }

        *leafTypePtr = leafType;
        *nhPointerPtr = nhPointer;
        *nextBucketPointerPtr = 0;
        *nextNodeTypePtr=0;
    }
    else /* CPSS_DXCH_LPM_LEAF_ENTRY_TYPE_TRIGGER_E */
    {
        if (isUc==GT_TRUE)
        {
            cpssOsPrintf("\nUnicast tree points to a source tree!!! Bucket address 0x%x\n",hwAddr);
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }
        /* check nextBucketPointer is in a legal range of the banks memory */
        ret = validateChildPtrSip6(devNum, hwAddr, nextBucketPointer);
        if (ret != GT_OK)
        {
            cpssOsPrintf("\nillegal nextBucketPointer in case of entryType=Trigger %d\n",nextBucketPointer);
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }

        *leafTypePtr = 0;
        *nhPointerPtr = 0;
        *nextBucketPointerPtr = nextBucketPointer;
        *nextNodeTypePtr=nextNodeType;
    }

    *entryTypePtr = entryType;

    return GT_OK;
}

/* function that validate the regular bit vector */
GT_STATUS validateBitVectorOfRegularBucketSip6(GT_U8 devNum, GT_U32 *bucketDataPtr,
                                               GT_U32 hwAddr, GT_U32 basePrefix)
{
    GT_STATUS ret=GT_OK;
    GT_U32 hwBucketAddr[PRV_CPSS_DXCH_LPM_RAM_FALCON_SIZE_OF_LPM_ENTRY_IN_WORDS_CNS];/* wide line - 4 words */
    GT_U32 bitSum=0;
    GT_U32 bitsArray;
    GT_U32 sum;
    GT_U32 i,j;

    for (i = 0; i < PRV_CPSS_DXCH_LPM_RAM_FALCON_BUCKET_SIZE_REGULAR_BV_CNS; i++)/* regular bit vector have 6 lines */
    {
        cpssOsMemCpy(hwBucketAddr, (bucketDataPtr+(i*PRV_CPSS_DXCH_LPM_RAM_FALCON_SIZE_OF_LPM_ENTRY_IN_WORDS_CNS)), sizeof(hwBucketAddr)); /* bitmap pointer first line */
        /* bit 113-112 Embedded_leaf MUST be "0" for Non Embedded Leaf Structure */
        if ((hwBucketAddr[3]>>17 & 0x3) != 0)
        {
            cpssOsPrintf("\n Embedded_leaf MUST be 0 for Non Embedded Leaf Structure!!! Bucket address 0x%x\n",hwAddr);
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }
        /* on the last bit vector line, 16 bits must be 0 - last line hold 220-255 ranges */
        if (i==5)
        {
            if((hwBucketAddr[0]>>20 & 0xFFFF) != 0)
            {
                cpssOsPrintf("\nOn last word, last 16 bits must be 0 !!! Bucket address 0x%x\n",hwAddr);
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
            }
        }

        /* check child_pointer is in a legal  range of the banks memory */
        ret = validateChildPtrSip6(devNum, hwAddr, (hwBucketAddr[0]&0xFFFFF));
        if (ret != GT_OK)
        {
            cpssOsPrintf("\nillegal child_pointer in validateBitVectorOfRegularBucketSip6 %d\n",hwAddr);
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }

        sum = 0;
        for (j=0;j<PRV_CPSS_DXCH_LPM_RAM_FALCON_SIZE_OF_LPM_ENTRY_IN_WORDS_CNS;j++)/* go over 4 words of the wide line */
        {
            if (j==0)
            {
                bitsArray = hwBucketAddr[0]>>20;
            }
            else
            {
                bitsArray = hwBucketAddr[j];
            }
            while (bitsArray != 0)
            {
                if ((bitsArray & 0x3) != 0)
                {
                    sum++; /* count number of set bits in array */
                }
                bitsArray = bitsArray >> 2;/* child node is represented by 2 bits */
            }
        }
        bitSum += sum;
    }

    /* Regular bucket is for 11-255 ranges so there must be at least 11 bits != 0
       exception here for root bucket, which can be regular
       If it regular then it will have ranges 6-255 -> less than 11 bits */
    if (bitSum < 11)
    {
        if (!((InSrcTree == GT_FALSE) && (basePrefix == 8)))
        {
            cpssOsPrintf("\nAt least 6 ranges should be defined!!! Bucket address 0x%x\n",hwAddr);
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }
    }
    return GT_OK;
}

GT_STATUS validateBitVectorOfCompressBucketSip6(GT_U8 devNum, GT_U32 *bucketDataPtr,
                                                GT_U32 hwAddr,GT_BOOL isUc)
{
    GT_STATUS status=GT_OK;
    GT_U32 hwDataArr[PRV_CPSS_DXCH_LPM_RAM_FALCON_SIZE_OF_LPM_ENTRY_IN_WORDS_CNS];/* wide line - 4 words */
    GT_U32 i,j;
    GT_U32 numberOfEmbeddedLeafs=0;
    GT_U32 maxRangesVal=10;/* no embedded leafs */
    GT_U32 numberOfRangesUsedVal=0;
    GT_U32 embeddedLeafVal,beforeTheLastOffsetValue;
    GT_U32 value;
    GT_U32 nhPointer,leafType;
    GT_U32 entryType, nextBucketPointer, nextNodeType;

    GT_BOOL foundFirstEmptyRange = GT_FALSE;

    cpssOsMemCpy(hwDataArr, bucketDataPtr, sizeof(hwDataArr)); /* bitmap pointer first line */
    /* check if we have embedded leafs,
       If we do then check their legality  */

    /* bit 113-112 Embedded_leaf */
    U32_GET_FIELD_IN_ENTRY_MAC(hwDataArr,112,2,value);
    numberOfEmbeddedLeafs = value;

    for (i = 0; i < numberOfEmbeddedLeafs; i++)
    {
        switch(i)
        {
        case 0:/*first embedded leaf located at bit 89-111*/
            U32_GET_FIELD_IN_ENTRY_MAC(hwDataArr, 89, 23, value);
            maxRangesVal = 7;
            break;
        case 1:/*second embedded leaf located at bit 66-88*/
            U32_GET_FIELD_IN_ENTRY_MAC(hwDataArr,66,23,value);
            maxRangesVal = 5;
            break;
        case 2:/*third embedded leaf located at bit 43-65*/
            U32_GET_FIELD_IN_ENTRY_MAC(hwDataArr,43,23,value);
            maxRangesVal = 3;
            break;
        default:
            cpssOsPrintf("\n illegal number of embedded leafs!!! Bucket address 0x%x\n", hwAddr);
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG); /* continue to other branches in the tree */
            break;
        }

        embeddedLeafVal = value;
        status = validateLeafSip6(devNum, hwAddr, embeddedLeafVal,
                                  &entryType, &leafType, &nhPointer,
                                  &nextBucketPointer,&nextNodeType,isUc);
        if (status != GT_OK)
        {
            cpssOsPrintf("\n Embedded_leaf illegal Leaf structure!!! Bucket address 0x%x\n", hwAddr);
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG); /* continue to other branches in the tree */
        }
    }

    /* check that the child_pointer has a legal value according to the LPM size */
    U32_GET_FIELD_IN_ENTRY_MAC(hwDataArr,0,20,value);
    status = validateChildPtrSip6(devNum, hwAddr, value);
    if (status != GT_OK)
    {
        cpssOsPrintf("\n Compress bit vecotor - illegal child_pointer!!! Bucket address 0x%x\n", hwAddr);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG); /* continue to other branches in the tree */
    }

    for (j=0;j<maxRangesVal;j++)/* go over all ranges in wide line */
    {
        /* child type */
        U32_GET_FIELD_IN_ENTRY_MAC(hwDataArr,(20+(10*(maxRangesVal-1-j))),2,value);
        /* first child type can not be 0.
           0 always open a range in the compress bit vector */
        if (j==0 && value==0)
        {
            cpssOsPrintf("\n childType can not be empty in the first range, 0 is always the first range in the bucket\n");
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }
        if (value == 0)
        {
            if (foundFirstEmptyRange==GT_FALSE)
            {
                /* the current bit in the bit-vector does not open a new range */
                foundFirstEmptyRange = GT_TRUE;
                continue;
            }
            else
            {
                continue;
            }
        }
        else
        {
            if ((foundFirstEmptyRange==GT_TRUE)&&(j!=(maxRangesVal-1))&&(j!=(maxRangesVal-2)))
            {
                cpssOsPrintf("\n Compress bit vector - illegal range types.\n"
                             " after the first empty range all ranges should be empty except the last one, and the one before the last one"
                             "!!! Bucket address 0x%x\n", hwAddr);
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
            }
        }
        numberOfRangesUsedVal++;

        status = validateChildTypeSip6(hwAddr, value);
        if (status != GT_OK)
        {
            cpssOsPrintf("\n Embedded_leaf illegal Structure!!! Bucket address 0x%x\n", hwAddr);
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }

        /* child offset */
         if (j!=0)/* first range offset is always 0 */
         {
            U32_GET_FIELD_IN_ENTRY_MAC(hwDataArr, ((20 + (10 * (maxRangesVal - 1 - j))) + 2), 8, value);
            status = validateChildOffsetSip6(hwAddr,value);
            if (status != GT_OK)
            {
                cpssOsPrintf("\n Compress illegal ChildOffset!!! Bucket address 0x%x\n", hwAddr);
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
            }
         }
    }

    if (numberOfRangesUsedVal!=maxRangesVal)
    {
        /* not all ranges were used */

        /* When less then max number of ranges possible are in use:
           - All low offset are in use
           - Offset9 is used to represent the range ending in 255
           - The unused offset are places before offset9 with ch_type=0
             except offset8 that will have the ch_type of the last
             valid offset before offset9.
             Offset_value off the unused range will be equal to the
             last valid offset_value before offset9
           - if we have only 2 ranges then the
             1. offset_value of the ranges between the first and the
                last will be of the second range
             2. ch_type of the ranges between the first and the
                last will be 0 */

        /* (numberOfRangesUsedVal-1) is the index of the last range treated out of maxRangesVal
           *10 to get the correct child data (each data is 10 bits of type+offset)
           +20 is for the 20 bits of the child_pointer
           +2 is for the 2 bits of the child_type
           after finding the offset we reduce 10 bits in order to get the "offset before the last" */
        if(numberOfRangesUsedVal>2)
        {
            U32_GET_FIELD_IN_ENTRY_MAC(hwDataArr, (((20 + (10 * ((maxRangesVal-1) - (numberOfRangesUsedVal-1)))) + 2) + 10), 8, value);
        }
        else
        {
            /* check that numberOfRangesUsedVal==2 else illegal option */
            if (numberOfRangesUsedVal==1)
            {
                cpssOsPrintf("\n if numberOfRangesUsedVal==1 illegal!!! Bucket address 0x%x\n", hwAddr);
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
            }
            else
            {
                /* need to find the last offset_value and to see all the offset_values after the first offset(that is 0) are the same */
                U32_GET_FIELD_IN_ENTRY_MAC(hwDataArr, 22, 8, value);

                /*  if we have only 2 ranges then the
                    1. offset_value of the ranges between the first and the
                       last will be of the second range
                    2. ch_type of the ranges between the first and the
                       last will be 0 */
            }
        }

        beforeTheLastOffsetValue = value;

        /* no need to loop on the offsetValue before the last one and to
           check that all offset until the last one have the same value */
        for (j=(numberOfRangesUsedVal-1);j<(maxRangesVal-1);j++)
        {
           U32_GET_FIELD_IN_ENTRY_MAC(hwDataArr, ((20 + (10 * (maxRangesVal - 1 - j))) + 2), 8, value);
            if (value!=beforeTheLastOffsetValue)
            {
                cpssOsPrintf("\n illegal offset values in empty ranges!!! Bucket address 0x%x\n", hwAddr);
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
            }
        }
    }

    return GT_OK;
}


/*************************************************************************/
GT_STATUS printLpm(GT_U8 devNum, GT_U32 bucketPtr, GT_U8 *baseAddress,
                   GT_U32 basePrefix, GT_U8 startAddress, GT_U32 endAddress,
                   GT_BOOL isUc, PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT protocol,
                   GT_U32 depth, prvLpmDumpPrintInfo_STC *printInfo)
{
    GT_U32 *bucketDataPtr;

    CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT bucketType = CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E;
    GT_U32 bitMapSize = 0;
    GT_U32 hwAddr = 0;      /* in lines */
    GT_U32 blockIndex = 0;
    GT_STATUS ret = GT_OK;

    GT_U8  addr[2*PRV_CPSS_DXCH_LPM_MAX_NUM_OF_OCTETS_FOR_ADDRESS_CNS] = {0};
    GT_U8  prefix = 0;
    GT_U32 basePrefixLength = 0;
    GT_U32 hwBucketAddr = 0;
    GT_U32 i = 0,j = 0;
    GT_U32 numOfRanges = 0;
    GT_U8 startAddr[2*PRV_CPSS_DXCH_LPM_MAX_NUM_OF_OCTETS_FOR_ADDRESS_CNS];
    GT_U8 endAddr[2*PRV_CPSS_DXCH_LPM_MAX_NUM_OF_OCTETS_FOR_ADDRESS_CNS];
    GT_U8 *baseAddrPtr = NULL;
    GT_U32 routeEntriesNum = 0;
    GT_U32 nhPointer = 0;
    GT_U32 rangeStart[10] = {0}; /* Bitmap has 9 bytes;
                              Word 1: [range_4_start ; range_3_start ; range_2_start ; range_1_start]
                              Word 2: [range_9_start ; range_8_start ; range_7_start ; range_6_start]
                                - range_5_start is carried from the pointer to this bucket
                                - range_0_start is always 0 (so no need to add to bitmap) */
    GT_BOOL rangeStartIsZero = GT_FALSE; /* After first range x start is 0, all ranges after must be 0 */
    GT_U32 nextBucketType = 0;
    GT_U32 pointerType = 0;
    PRV_CPSS_DXCH_LPM_ECMP_ENTRY_STC ecmpEntry;
    GT_U32    duplicateBucketData[PRV_CPSS_DXCH_LPM_RAM_MAX_SIZE_OF_BUCKET_IN_LPM_LINES_CNS];

    cpssOsMemSet(&ecmpEntry,0,sizeof(ecmpEntry));
    numOfRanges = 0;
    nextBucketType = (bucketPtr >> 3) & 0x3;
    pointerType = bucketPtr & 0x3;
    if (pointerType == 0x0)
    {
        if (nextBucketType == 0x3)
        {
            cpssOsPrintf("\nBucket Type - Invalid value exists!!! Bucket Type %d\n",bucketPtr);
            numOfErrors++;
            return GT_OK; /* continue to other branches in the tree */
        }
        bucketType = nextBucketType;
    }
    else if (pointerType == 0x1)
    {
        bucketType = CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E;
    }
    else if (pointerType == 0x2)
    {
        bucketType = CPSS_DXCH_LPM_ECMP_ENTRY_PTR_TYPE_E;
    }
    else /* pointerType == 0x3 */
    {
        bucketType = CPSS_DXCH_LPM_QOS_ENTRY_PTR_TYPE_E;
    }

    /* Allocate memory for bucket data. Maximum size is regular bucket that
       uses 256 lpm lines for ranges + 11 lpm lines for bitmap */
    bucketDataPtr = cpssOsMalloc(PRV_CPSS_DXCH_LPM_RAM_MAX_SIZE_OF_BUCKET_IN_LPM_LINES_CNS *
                                 PRV_CPSS_DXCH_LPM_RAM_SIZE_OF_LPM_ENTRY_IN_BYTES_CNS);
    if (bucketDataPtr == NULL)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);

    cpssOsMemSet(bucketDataPtr, 0,
                    PRV_CPSS_DXCH_LPM_RAM_MAX_SIZE_OF_BUCKET_IN_LPM_LINES_CNS *
                            PRV_CPSS_DXCH_LPM_RAM_SIZE_OF_LPM_ENTRY_IN_BYTES_CNS);

    basePrefixLength = basePrefix + 8;
    baseAddrPtr = baseAddress + 1;
    if(bucketType == CPSS_DXCH_LPM_COMPRESSED_1_NODE_PTR_TYPE_E)
    {
        bitMapSize = PRV_CPSS_DXCH_LPM_RAM_BUCKET_BIT_VEC_SIZE_COMPRESSED_1_CNS;
        /* hw pointer in LPM entry is in LPM lines*/
        hwAddr = ((bucketPtr & 0x1FFFFFE0) >> 5);
    }
    else if(bucketType == CPSS_DXCH_LPM_COMPRESSED_2_NODE_PTR_TYPE_E)
    {
        bitMapSize = PRV_CPSS_DXCH_LPM_RAM_BUCKET_BIT_VEC_SIZE_COMPRESSED_2_CNS;
        hwAddr = (bucketPtr >> 13);
    }
    else if(bucketType == CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E)
    {
        bitMapSize = PRV_CPSS_DXCH_LPM_RAM_BUCKET_BIT_VEC_SIZE_REGULAR_CNS;
        hwAddr = ((bucketPtr & 0x1FFFFFE0) >> 5);
    }
    else /* bucketType = CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E or
                         CPSS_DXCH_LPM_ECMP_ENTRY_PTR_TYPE_E or
                         CPSS_DXCH_LPM_QOS_ENTRY_PTR_TYPE_E */
    {
        bitMapSize = 0;
        hwAddr = 0;
    }

    if (PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        /* the hwAddr is a relative address to the beginning of the LPM */
        blockIndex = hwAddr/PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.totalNumOfLinesInBlockIncludingGap;
    }
    else
    {
        /* find in which LPM bucket located */
        if (protocol == PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E)
        {
            blockIndex = addrLookupMemIpv4Array[basePrefix/8];
        }
        else if (protocol == PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E)
        {
            blockIndex = addrLookupMemIpv6Array[basePrefix/8];
        }
        else if (protocol == PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E)
        {
            blockIndex = addrLookupMemFcoeArray[basePrefix/8];
        }
        else
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }

    if ((bucketType != CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E) &&
        (bucketType != CPSS_DXCH_LPM_ECMP_ENTRY_PTR_TYPE_E) &&
        (bucketType != CPSS_DXCH_LPM_QOS_ENTRY_PTR_TYPE_E))
    {
        /* this is a bucket */
        printInfo->bucketPerType[bucketType]++;
        printInfo->bucketPerOctet[depth]++;
        printInfo->bucketTypePerOctet[depth][bucketType]++;
        if (printInfo->printRanges)
        {
            cpssOsPrintf (" %X \n",hwAddr);
        }

        /* read the bit vector */
        ret = prvCpssDxChReadTableMultiEntry(devNum,
                                             CPSS_DXCH_SIP5_TABLE_LPM_MEM_E,
                                             hwAddr, bitMapSize, bucketDataPtr);
        if (ret != GT_OK)
        {
            cpssOsFree(bucketDataPtr);
            return ret;
        }

        if(PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.lpmMemMode == PRV_CPSS_DXCH_LPM_RAM_MEM_MODE_HALF_MEM_SIZE_E)
        {
            /* read the bit vector */
            ret = prvCpssDxChReadTableMultiEntry(devNum,
                                             CPSS_DXCH_SIP5_20_TABLE_LPM_MEM_PIPE_1_E,
                                             hwAddr, bitMapSize, duplicateBucketData);
            if (ret != GT_OK)
            {
                cpssOsFree(bucketDataPtr);
                return ret;
            }
        }

        numOfRanges = getNumOfRangesFromHW(bucketType,bucketDataPtr,bucketPtr);

        /* read the ranges (the LPM lines after the bit vector) */
        ret = prvCpssDxChReadTableMultiEntry(devNum,
                                             CPSS_DXCH_SIP5_TABLE_LPM_MEM_E,
                                             hwAddr + bitMapSize, numOfRanges,
                                             &bucketDataPtr[bitMapSize*PRV_CPSS_DXCH_LPM_RAM_SIZE_OF_LPM_ENTRY_IN_WORDS_CNS]);
        if (ret != GT_OK)
        {
            cpssOsFree(bucketDataPtr);
            return ret;
        }

        if(PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.lpmMemMode == PRV_CPSS_DXCH_LPM_RAM_MEM_MODE_HALF_MEM_SIZE_E)
        {
            /* read the ranges (the LPM lines after the bit vector) */
            ret = prvCpssDxChReadTableMultiEntry(devNum,
                                             CPSS_DXCH_SIP5_20_TABLE_LPM_MEM_PIPE_1_E,
                                             hwAddr + bitMapSize, numOfRanges, &duplicateBucketData[bitMapSize*PRV_CPSS_DXCH_LPM_RAM_SIZE_OF_LPM_ENTRY_IN_WORDS_CNS]);
            if (ret != GT_OK)
            {
                cpssOsFree(bucketDataPtr);
                return ret;
            }

            ret = compareBucket(devNum,hwAddr,bucketDataPtr,duplicateBucketData,numOfRanges,bucketType,GT_TRUE);

            if (ret != GT_OK)
            {
                cpssOsFree(bucketDataPtr);
                return ret;
            }
        }




        if (isUc)
        {
            /* IPv4 UC / IPv6 UC/ FCoE Max tree depth is 4/16/3 accordingly */
            if (depth > PRV_DXCH_LPM_RAM_DBG_MAX_DEPTH_CHECK_MAC(protocol))
            {
                cpssOsPrintf("\nUC Max tree depth is %d!!! Bucket address 0x%x\n",PRV_DXCH_LPM_RAM_DBG_MAX_DEPTH_CHECK_MAC(protocol),hwAddr);
                cpssOsFree(bucketDataPtr);
                numOfErrors++;
                return GT_OK; /* continue to other branches in the tree */
            }
            /* From depth 4/16/3 (according to type) all LPM lines must be NH/ECMP/QoS bucket type */
            else if (depth == PRV_DXCH_LPM_RAM_DBG_MAX_DEPTH_CHECK_MAC(protocol))
            {
                cpssOsPrintf("\nFrom depth %d all LPM lines must be NH, ECMP or QoS bucket type!!! Bucket address 0x%x\n",PRV_DXCH_LPM_RAM_DBG_MAX_DEPTH_CHECK_MAC(protocol),hwAddr);
                cpssOsFree(bucketDataPtr);
                numOfErrors++;
                return GT_OK; /* continue to other branches in the tree */
            }
        }
        else /* multicast */
        {
            if (protocol == PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E)
            {
                if (InSrcTree == GT_FALSE)
                {
                    /* Multicast group tree */
                    if (depth > 4)
                    {
                        /* IPv4 Max MC group depth is 4 */
                        cpssOsPrintf("\nIPv4 MC Max tree depth is 4!!! Bucket address 0x%x\n",hwAddr);
                        cpssOsFree(bucketDataPtr);
                        numOfErrors++;
                        return GT_OK; /* continue to other branches in the tree */
                    }
                }
                else
                {
                    /* Multicast source tree */
                    if (depth > 8)
                    {
                        /* Each IPv4 MC group points to root of sources tree (with max depth of 4). So total max depth is 8. */
                        cpssOsPrintf("\nIPv4 MC source Max tree depth is 8!!! Bucket address 0x%x\n",hwAddr);
                        cpssOsFree(bucketDataPtr);
                        numOfErrors++;
                        return GT_OK; /* continue to other branches in the tree */
                    }
                    else if (depth == 8)
                    {
                        /* For IPv4 MC in depth 8 all LPM lines must be NH/ECMP/Qos bucket type */
                        cpssOsPrintf("\nFrom depth %d all LPM lines must be NH/ECMP/QoS bucket type!!! Bucket address 0x%x\n",8,hwAddr);
                        cpssOsFree(bucketDataPtr);
                        numOfErrors++;
                        return GT_OK; /* continue to other branches in the tree */
                    }
                }
            }
            else    /* PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E */
            {
                if (InSrcTree == GT_FALSE)
                {
                    /* Multicast group tree */
                    if (depth > PRV_CPSS_DXCH_LPM_MAX_NUM_OF_OCTETS_FOR_ADDRESS_CNS)
                    {
                        /* IPv6 Max MC group depth is 16 */
                        cpssOsPrintf("\nIPv6 MC Max tree depth is 16!!! Bucket address 0x%x\n",hwAddr);
                        cpssOsFree(bucketDataPtr);
                        numOfErrors++;
                        return GT_OK; /* continue to other branches in the tree */
                    }
                }
                else
                {
                    /* Multicast source tree */
                    if (depth > (2 * PRV_CPSS_DXCH_LPM_MAX_NUM_OF_OCTETS_FOR_ADDRESS_CNS))
                    {
                        /* Each IPv6 MC group points to root of sources tree (with max depth of 16). So total max depth is 32. */
                        cpssOsPrintf("\nIPv6 MC source Max tree depth is 32!!! Bucket address 0x%x\n",hwAddr);
                        cpssOsFree(bucketDataPtr);
                        numOfErrors++;
                        return GT_OK; /* continue to other branches in the tree */
                    }
                    else if (depth == (2 * PRV_CPSS_DXCH_LPM_MAX_NUM_OF_OCTETS_FOR_ADDRESS_CNS))
                    {
                        /* For IPv6 MC in depth 32 all LPM lines must be NH/ECMP/Qos bucket type */
                        cpssOsPrintf("\nFrom depth %d all LPM lines must be NH/ECMP/QoS bucket type!!! Bucket address 0x%x\n",
                                     (2 * PRV_CPSS_DXCH_LPM_MAX_NUM_OF_OCTETS_FOR_ADDRESS_CNS), hwAddr);
                        cpssOsFree(bucketDataPtr);
                        numOfErrors++;
                        return GT_OK; /* continue to other branches in the tree */
                    }
                }
            }
        }

        printInfo->lpmLinesUsed[blockIndex] += (bitMapSize + numOfRanges);
        printInfo->lpmLinesPerOctet[depth] += (bitMapSize + numOfRanges);
        printInfo->bucketSizePerType[bucketType] += (bitMapSize + numOfRanges);
    }

    switch (bucketType)
    {
    case (CPSS_DXCH_LPM_COMPRESSED_1_NODE_PTR_TYPE_E):
        hwBucketAddr = bucketDataPtr[0]; /* bitmap pointer */

        /* Bitmap has 4 bytes [range_4_start ; range_3_start ; range_2_start ; range_1_start]
           Range 0 start is always 0 (so no need to add to bitmap) */
        rangeStart[1] = hwBucketAddr & 0xFF;
        rangeStart[2] = (hwBucketAddr >> 8) & 0xFF;
        rangeStart[3] = (hwBucketAddr >> 16) & 0xFF;
        rangeStart[4] = (hwBucketAddr >> 24) & 0xFF;

        /* For FCoE Compressed 1 can contain one range for level 0 but must not contain one range for the other levels.
           For IPv4/IPv6 Compressed 1 must not contain one range for any level.
           Check that range_1_start (second range) must be != 0 */
        if ((protocol != PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E) || ((protocol == PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E) && (depth > 0)))
        {
            if (rangeStart[1] == 0)
            {
                cpssOsPrintf("\nAt least 2 ranges should be defined!!! Bucket address 0x%x\n",hwAddr);
                cpssOsFree(bucketDataPtr);
                numOfErrors++;
                return GT_OK; /* continue to other branches in the tree */
            }
        }

        rangeStartIsZero = 0;
        for (i = 1; i < PRV_CPSS_DXCH_LPM_RAM_NUM_OF_RANGES_IN_LPM_LINE_CNS; i++)
        {
            if (rangeStart[i] == 0)
            {
                /* no new ranges; first range x start is zero */
                rangeStartIsZero = 1;
                continue;
            }
            /* While range x start != 0 Current range start > previous range start */
            if ((rangeStart[i+1] <= rangeStart[i]) && (rangeStart[i+1] != 0))
            {
                cpssOsPrintf("\nNext range should be bigger then current range!!! Bucket address 0x%x\n",hwAddr);
                cpssOsFree(bucketDataPtr);
                numOfErrors++;
                return GT_OK; /* continue to other branches in the tree */
            }
            /* After first range x start is 0, all ranges after must be 0*/
            if (rangeStartIsZero && (rangeStart[i] != 0))
            {
                cpssOsPrintf("\nAll range starts after first 0 must be 0!!! Bucket address 0x%x\n",hwAddr);
                cpssOsFree(bucketDataPtr);
                numOfErrors++;
                return GT_OK; /* continue to other branches in the tree */
            }
        }

        for (i = 0; i < numOfRanges; i++)
        {
            hwBucketAddr = bucketDataPtr[(PRV_CPSS_DXCH_LPM_RAM_BUCKET_BIT_VEC_SIZE_COMPRESSED_1_CNS + i)*PRV_CPSS_DXCH_LPM_RAM_SIZE_OF_LPM_ENTRY_IN_WORDS_CNS];
            startAddress = (GT_U8)getBaseAddrFromHW(CPSS_DXCH_LPM_COMPRESSED_1_NODE_PTR_TYPE_E,i,bucketDataPtr,bucketPtr);
            endAddress = (i == (numOfRanges-1))? 255 : getBaseAddrFromHW(CPSS_DXCH_LPM_COMPRESSED_1_NODE_PTR_TYPE_E,i+1,bucketDataPtr,bucketPtr)- 1;
            if (endAddress > 255)
            {
                cpssOsPrintf("Err!!");
            }
            if ( (isUc == GT_FALSE)    &&
                 (InSrcTree == GT_TRUE)&&
                 (basePrefix == 0)    )
            {
                /* I am in mc src tree in zero bucket */
                baseAddress[0] = startAddress;
                baseAddrPtr = baseAddr;
            }
            else
                baseAddress[1] = startAddress;

            /* check if the next level is a bucket and if it points to a source tree */
            if (((hwBucketAddr & 0x3) == 0x0) && ((hwBucketAddr >> 2) & 0x1))
            {
                if (isUc == GT_TRUE)
                {
                    cpssOsPrintf("\nUnicast tree points to a source tree!!! Bucket address 0x%x\n",hwAddr);
                    cpssOsFree(bucketDataPtr);
                    numOfErrors++;
                    return GT_OK; /* continue to other branches in the tree */
                }
                else if (InSrcTree == GT_TRUE)
                {
                    cpssOsPrintf("\nMulticast source tree points to a source tree!!! Bucket address 0x%x\n",hwAddr);
                    cpssOsFree(bucketDataPtr);
                    numOfErrors++;
                    return GT_OK; /* continue to other branches in the tree */
                }
                else if ((hwBucketAddr & 0x3) == 0)
                {
                    /* It is last group bucket. The next pointer is not next hop
                       pointer type -- the src tree will be started */
                    if (printInfo->printRanges)
                    {
                        printLpmMcGroup(protocol, (GT_U8)basePrefixLength, startAddress, (GT_U8)endAddress);
                    }
                    InSrcTree = GT_TRUE;
                    grpPrefix = basePrefixLength;
                    basePrefixLength = 0;
                    cpssOsMemCpy(baseGroupAddr,baseAddr,sizeof(baseAddr));
                    cpssOsMemSet(baseAddr,0,sizeof(baseAddr));
                    baseGroupAddrPtr = baseAddrPtr;
                    baseAddrPtr = baseAddr;
                }
            }

            ret = printLpm(devNum,hwBucketAddr,baseAddrPtr,basePrefixLength,
                           startAddress,endAddress,isUc,protocol,depth+1,printInfo);
            if (ret != GT_OK)
            {
                cpssOsFree(bucketDataPtr);
                return ret;
            }

            if( (isUc == GT_FALSE)      &&
                (basePrefixLength == 0)       &&
                (InSrcTree == GT_TRUE)  )
            {
                /* The source tree was displayed and the other group regions
                   will be printed*/

                InSrcTree = GT_FALSE;
                basePrefixLength = grpPrefix;
                cpssOsMemCpy(baseAddr,baseGroupAddr,sizeof(baseAddr));
                baseAddrPtr = baseGroupAddrPtr;
                if (printInfo->printRanges)
                {
                    cpssOsPrintf(" End of Src tree for this group. \n");
                }
            }
        }
        break;

    case (CPSS_DXCH_LPM_COMPRESSED_2_NODE_PTR_TYPE_E):
        hwBucketAddr = bucketDataPtr[0]; /* bitmap 1 pointer */

        /* Bitmap has 9 bytes; Word 1: [range_4_start ; range_3_start ; range_2_start ; range_1_start]
                               Word 2: [range_9_start ; range_8_start ; range_7_start ; range_6_start]
                                - range_5_start is carried from the pointer to this bucket
                                - range_0_start is always 0 (so no need to add to bitmap) */
        rangeStart[1] = hwBucketAddr & 0xFF;
        rangeStart[2] = (hwBucketAddr >> 8) & 0xFF;
        rangeStart[3] = (hwBucketAddr >> 16) & 0xFF;
        rangeStart[4] = (hwBucketAddr >> 24) & 0xFF;
        rangeStart[5] = (bucketPtr >> 5) & 0xFF;

        /* for word 1: all ranges start must be != 0 (if one is 0, then it should be compressed 1);
           Range_5_start must be != 0 (if 0, then should use compressed 1) */
        if ((rangeStart[1] == 0) || (rangeStart[2] == 0) || (rangeStart[3] == 0) || (rangeStart[4] == 0) || (rangeStart[5] == 0))
        {
            cpssOsPrintf("\nAt least 6 ranges should be defined!!! Bucket address 0x%x\n",hwAddr);
            cpssOsFree(bucketDataPtr);
            numOfErrors++;
            return GT_OK; /* continue to other branches in the tree */
        }

        hwBucketAddr = bucketDataPtr[1*PRV_CPSS_DXCH_LPM_RAM_SIZE_OF_LPM_ENTRY_IN_WORDS_CNS]; /* bitmap 2 pointer */

        rangeStart[6] = hwBucketAddr & 0xFF;
        rangeStart[7] = (hwBucketAddr >> 8) & 0xFF;
        rangeStart[8] = (hwBucketAddr >> 16) & 0xFF;
        rangeStart[9] = (hwBucketAddr >> 24) & 0xFF;

        rangeStartIsZero = 0;
        for (i = 1; i < (2 * PRV_CPSS_DXCH_LPM_RAM_NUM_OF_RANGES_IN_LPM_LINE_CNS + 1); i++)
        {
            if (rangeStart[i] == 0)
            {
                /* no new ranges; first range x start is zero */
                rangeStartIsZero = 1;
                continue;
            }
            /* While range x start != 0 Current range start > previous range start */
            if ((rangeStart[i+1] <= rangeStart[i]) && (rangeStart[i+1] != 0))
            {
                if (i <= PRV_CPSS_DXCH_LPM_RAM_NUM_OF_RANGES_IN_LPM_LINE_CNS)
                {
                    hwBucketAddr = bucketDataPtr[0]; /* bitmap 1 pointer */
                }
                else
                {
                    hwBucketAddr = bucketDataPtr[1*PRV_CPSS_DXCH_LPM_RAM_SIZE_OF_LPM_ENTRY_IN_WORDS_CNS]; /* bitmap 2 pointer */
                }
                cpssOsPrintf("\nNext range should be bigger then current range!!! Bucket address 0x%x\n",hwAddr);
                cpssOsFree(bucketDataPtr);
                numOfErrors++;
                return GT_OK; /* continue to other branches in the tree */
            }
            /* After first range x start is 0, all ranges after must be 0*/
            if (rangeStartIsZero && (rangeStart[i] != 0))
            {
                if (i <= PRV_CPSS_DXCH_LPM_RAM_NUM_OF_RANGES_IN_LPM_LINE_CNS)
                {
                    hwBucketAddr = bucketDataPtr[0]; /* bitmap 1 pointer */
                }
                else
                {
                    hwBucketAddr = bucketDataPtr[1*PRV_CPSS_DXCH_LPM_RAM_SIZE_OF_LPM_ENTRY_IN_WORDS_CNS]; /* bitmap 2 pointer */
                }
                cpssOsPrintf("\nAll ranges must be 0!!! Bucket address 0x%x\n",hwAddr);
                cpssOsFree(bucketDataPtr);
                numOfErrors++;
                return GT_OK; /* continue to other branches in the tree */
            }
        }

        for (i = 0; i < numOfRanges; i++)
        {
            hwBucketAddr = bucketDataPtr[PRV_CPSS_DXCH_LPM_RAM_BUCKET_BIT_VEC_SIZE_COMPRESSED_2_CNS*PRV_CPSS_DXCH_LPM_RAM_SIZE_OF_LPM_ENTRY_IN_WORDS_CNS + i*PRV_CPSS_DXCH_LPM_RAM_SIZE_OF_LPM_ENTRY_IN_WORDS_CNS];
            startAddress = (GT_U8)getBaseAddrFromHW(CPSS_DXCH_LPM_COMPRESSED_2_NODE_PTR_TYPE_E,i,bucketDataPtr,bucketPtr);
            endAddress = (i == (numOfRanges-1))? 255 : getBaseAddrFromHW(CPSS_DXCH_LPM_COMPRESSED_2_NODE_PTR_TYPE_E,i+1,bucketDataPtr,bucketPtr)- 1;
            if ( (isUc == GT_FALSE)    &&
                 (InSrcTree == GT_TRUE)&&
                 (basePrefix == 0)    )
            {
                /* I am in mc src tree in zero bucket */
                baseAddress[0] = startAddress;
                baseAddrPtr = baseAddr;
            }
            else
                baseAddress[1] = startAddress;

            if (((hwBucketAddr & 0x3) == 0x0) && ((hwBucketAddr >> 2) & 0x1))
            {
                if (isUc == GT_TRUE)
                {
                    cpssOsPrintf("\nUnicast tree points to a source tree!!! Bucket address 0x%x\n",hwAddr);
                    cpssOsFree(bucketDataPtr);
                    numOfErrors++;
                    return GT_OK; /* continue to other branches in the tree */
                }
                else if (InSrcTree == GT_TRUE)
                {
                    cpssOsPrintf("\nMulticast source tree points to a source tree!!! Bucket address 0x%x\n",hwAddr);
                    cpssOsFree(bucketDataPtr);
                    numOfErrors++;
                    return GT_OK; /* continue to other branches in the tree */
                }
                else if ((hwBucketAddr & 0x3) == 0)
                {
                    /* It is last group bucket. The next pointer is not next hop
                       pointer type -- the src tree will be started */
                    if (printInfo->printRanges)
                    {
                        printLpmMcGroup(protocol, (GT_U8)basePrefixLength, startAddress, (GT_U8)endAddress);
                    }
                    InSrcTree = GT_TRUE;
                    grpPrefix = basePrefixLength;
                    basePrefixLength = 0;
                    cpssOsMemCpy(baseGroupAddr,baseAddr,sizeof(baseAddr));
                    cpssOsMemSet(baseAddr,0,sizeof(baseAddr));
                    baseGroupAddrPtr = baseAddrPtr;
                    baseAddrPtr = baseAddr;
                }
            }

            ret = printLpm(devNum,hwBucketAddr,baseAddrPtr,basePrefixLength,
                           startAddress,endAddress,isUc,protocol,depth+1,printInfo);

            if (ret != GT_OK)
            {
                cpssOsFree(bucketDataPtr);
                return ret;
            }
            if( (isUc == GT_FALSE)      &&
                (basePrefixLength == 0)       &&
                (InSrcTree == GT_TRUE)  )
            {
                /* The source tree was displayed and the other group regions
                   will be printed*/

                InSrcTree = GT_FALSE;
                basePrefixLength = grpPrefix;
                cpssOsMemCpy(baseAddr,baseGroupAddr,sizeof(baseAddr));
                baseAddrPtr = baseGroupAddrPtr;
                if (printInfo->printRanges)
                {
                    cpssOsPrintf(" End of Src tree for this group. \n");
                }
            }
        }
        break;

    case (CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E):
        ret = validateBitVectorOfRegularBucket(bucketDataPtr, hwAddr, basePrefix);
        if (ret != GT_OK)
        {
            return GT_OK; /* continue to other branches in the tree */
        }
        for (i = 0; i < numOfRanges; i++)
        {
            hwBucketAddr = bucketDataPtr[11*PRV_CPSS_DXCH_LPM_RAM_SIZE_OF_LPM_ENTRY_IN_WORDS_CNS + i*PRV_CPSS_DXCH_LPM_RAM_SIZE_OF_LPM_ENTRY_IN_WORDS_CNS];
            startAddress = (GT_U8)getBaseAddrFromHW(CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E,i,bucketDataPtr,bucketPtr);
            endAddress = (i == (numOfRanges-1))? 255 : getBaseAddrFromHW(CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E,i+1,bucketDataPtr,bucketPtr)- 1;
            if ( (isUc == GT_FALSE)    &&
                 (InSrcTree == GT_TRUE)&&
                 (basePrefix == 0)    )
            {
                /* I am in mc src tree in zero bucket */
                baseAddress[0] = startAddress;
                baseAddrPtr = baseAddr;
            }
            else
                baseAddress[1] = startAddress;

            if (((hwBucketAddr & 0x3) == 0x0) && ((hwBucketAddr >> 2) & 0x1))
            {
                if (isUc == GT_TRUE)
                {
                    cpssOsPrintf("\nUnicast tree points to a source tree!!! Bucket address 0x%x\n",hwAddr);
                    cpssOsFree(bucketDataPtr);
                    numOfErrors++;
                    return GT_OK; /* continue to other branches in the tree */
                }
                else if (InSrcTree == GT_TRUE)
                {
                    cpssOsPrintf("\nMulticast source tree points to a source tree!!! Bucket address 0x%x\n",hwAddr);
                    cpssOsFree(bucketDataPtr);
                    numOfErrors++;
                    return GT_OK; /* continue to other branches in the tree */
                }
                else if ((hwBucketAddr & 0x3) == 0)
                {
                    /* It is last group bucket. The next pointer is not next hop
                       pointer type -- the src tree will be started */
                    if (printInfo->printRanges)
                    {
                        printLpmMcGroup(protocol, (GT_U8)basePrefixLength, startAddress, (GT_U8)endAddress);
                    }
                    InSrcTree = GT_TRUE;
                    grpPrefix = basePrefixLength;
                    basePrefixLength = 0;
                    cpssOsMemCpy(baseGroupAddr,baseAddr,sizeof(baseAddr));
                    cpssOsMemSet(baseAddr,0,sizeof(baseAddr));
                    baseGroupAddrPtr = baseAddrPtr;
                    baseAddrPtr = baseAddr;
                }
            }

            ret = printLpm(devNum,hwBucketAddr,baseAddrPtr,basePrefixLength,
                           startAddress,endAddress,isUc,protocol,depth+1,printInfo);
            if (ret != GT_OK)
            {
                cpssOsFree(bucketDataPtr);
                return ret;
            }
            if( (isUc == GT_FALSE)      &&
                (basePrefixLength == 0)       &&
                (InSrcTree == GT_TRUE)  )
            {
                /* The source tree was displayed and the other group regions
                   will be printed*/

                InSrcTree = GT_FALSE;
                basePrefixLength = grpPrefix;
                cpssOsMemCpy(baseAddr,baseGroupAddr,sizeof(baseAddr));
                baseAddrPtr = baseGroupAddrPtr;
                if (printInfo->printRanges)
                {
                    cpssOsPrintf(" End of Src tree for this group. \n");
                }
            }
        }
        break;

    case (CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E):
    case (CPSS_DXCH_LPM_ECMP_ENTRY_PTR_TYPE_E):
    case (CPSS_DXCH_LPM_QOS_ENTRY_PTR_TYPE_E):

        if (bucketType == CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E)
        {
            /* Number of entries in NH table */
            routeEntriesNum = PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.routerNextHop;
        }
        else
        {
            /* Number of entries in ECMP/QoS table */
            routeEntriesNum = PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.ecmpQos;
        }
        nhPointer = ((bucketPtr >> 7) & 0x00007FFF);

        /* NH base address must be within the NH table */
        if (nhPointer >= routeEntriesNum)
        {
            cpssOsPrintf("\nNH pointer not in range of NH table!!! NH address %d\n",nhPointer);
            cpssOsFree(bucketDataPtr);
            numOfErrors++;
            return GT_OK; /* continue to other branches in the tree */
        }

        if ((bucketType == CPSS_DXCH_LPM_ECMP_ENTRY_PTR_TYPE_E) ||
            (bucketType == CPSS_DXCH_LPM_QOS_ENTRY_PTR_TYPE_E))
        {
            ret = prvCpssDxChLpmHwEcmpEntryRead(devNum, nhPointer, &ecmpEntry);
            if (ret != GT_OK)
            {
                cpssOsPrintf("\nFailed to read ECMP entry %d\n",nhPointer);
                cpssOsFree(bucketDataPtr);
                numOfErrors++;
                return GT_OK; /* continue to other branches in the tree */
            }
            routeEntriesNum = PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.routerNextHop;
            if (ecmpEntry.nexthopBaseIndex + ecmpEntry.numOfPaths >= routeEntriesNum)
            {
                cpssOsPrintf("\nECMP entry points to illegal nexthop index %d\n",nhPointer);
                cpssOsFree(bucketDataPtr);
                numOfErrors++;
                return GT_OK; /* continue to other branches in the tree */
            }
        }

        cpssOsMemSet(endAddr,0, sizeof(GT_U8) * (2 * PRV_CPSS_DXCH_LPM_MAX_NUM_OF_OCTETS_FOR_ADDRESS_CNS));
        cpssOsMemSet(startAddr,0, sizeof(GT_U8) * (2 * PRV_CPSS_DXCH_LPM_MAX_NUM_OF_OCTETS_FOR_ADDRESS_CNS));

        prefix = (GT_U8)basePrefix;
        prefixBase2Address(baseAddr ,prefix,1,addr);
        for (j = 8; j < prefix; j += 8)
        {
            startAddr[(j/8)-1] = addr[(j/8)-1];
            endAddr[(j/8)-1] = addr[(j/8)-1];
        }
        startAddr[(j/8)-1] = startAddress;
        endAddr[(j/8)-1] = (GT_U8)endAddress;
        if (printInfo->printRanges)
        {
            if (protocol == PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E)
            {
                outputAddress(startAddr, PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS, protocol);
                cpssOsPrintf(" - ");
                outputAddress(endAddr, PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS, protocol);
            }
            else if (protocol == PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E)
            {
                outputAddress(startAddr, PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS, protocol);
                cpssOsPrintf(" - ");
                outputAddress(endAddr, PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS, protocol);
            }
            else /* PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E */
            {
                outputAddress(startAddr, PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_FCOE_PROTOCOL_CNS, protocol);
                cpssOsPrintf(" - ");
                outputAddress(endAddr, PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_FCOE_PROTOCOL_CNS, protocol);
            }
            cpssOsPrintf(" / %02d ", prefix);
            if (bucketType == CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E)
            {
                cpssOsPrintf(" -> NH %X\n",nhPointer);
            }
            else
            {
                cpssOsPrintf(" -> ECMP/QoS %X\n",nhPointer);
            }
        }
        if (nhPointer >= PRV_CPSS_DXCH_LPM_RAM_NUM_OF_DEFAULT_ENTRIES)
            printInfo->numOfNonDefaultNextHopPointers++;
        break;

    default:
        break;
    }

    cpssOsFree(bucketDataPtr);
    return GT_OK;
}

/*************************************************************************/
GT_STATUS checkDepthSip6(GT_BOOL isUc, PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT protocol,GT_U32 depth, GT_U32 hwAddr)
{
    if (isUc)
    {
        /* IPv4 UC / IPv6 UC/ FCoE Max tree depth is 4/16/3 accordingly */
        if (depth > PRV_DXCH_LPM_RAM_DBG_MAX_DEPTH_CHECK_MAC(protocol))
        {
            cpssOsPrintf("\nUC Max tree depth is %d!!! Bucket address 0x%x\n",PRV_DXCH_LPM_RAM_DBG_MAX_DEPTH_CHECK_MAC(protocol),hwAddr);
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }
        /* From depth 4/16/3 (according to type) all LPM lines must be NH/ECMP/QoS bucket type */
        else if (depth == PRV_DXCH_LPM_RAM_DBG_MAX_DEPTH_CHECK_MAC(protocol))
        {
            cpssOsPrintf("\nFrom depth %d all LPM lines must be NH, ECMP or QoS bucket type!!! Bucket address 0x%x\n",PRV_DXCH_LPM_RAM_DBG_MAX_DEPTH_CHECK_MAC(protocol),hwAddr);
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }
    }
    else /* multicast */
    {
        if (protocol == PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E)
        {
            if (InSrcTree == GT_FALSE)
            {
                /* Multicast group tree */
                if (depth > 4)
                {
                    /* IPv4 Max MC group depth is 4 */
                    cpssOsPrintf("\nIPv4 MC Max tree depth is 4!!! Bucket address 0x%x\n",hwAddr);
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
                }
            }
            else
            {
                /* Multicast source tree */
                if (depth > 8)
                {
                    /* Each IPv4 MC group points to root of sources tree (with max depth of 4). So total max depth is 8. */
                    cpssOsPrintf("\nIPv4 MC source Max tree depth is 8!!! Bucket address 0x%x\n",hwAddr);
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
                }
                else if (depth == 8)
                {
                    /* For IPv4 MC in depth 8 all LPM lines must be NH/ECMP/Qos bucket type */
                    cpssOsPrintf("\nFrom depth %d all LPM lines must be NH/ECMP/QoS bucket type!!! Bucket address 0x%x\n",8,hwAddr);
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
                }
            }
        }
        else    /* PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E */
        {
            if (InSrcTree == GT_FALSE)
            {
                /* Multicast group tree */
                if (depth > PRV_CPSS_DXCH_LPM_MAX_NUM_OF_OCTETS_FOR_ADDRESS_CNS)
                {
                    /* IPv6 Max MC group depth is 16 */
                    cpssOsPrintf("\nIPv6 MC Max tree depth is 16!!! Bucket address 0x%x\n",hwAddr);
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
                }
            }
            else
            {
                /* Multicast source tree */
                if (depth > (2 * PRV_CPSS_DXCH_LPM_MAX_NUM_OF_OCTETS_FOR_ADDRESS_CNS))
                {
                    /* Each IPv6 MC group points to root of sources tree (with max depth of 16). So total max depth is 32. */
                    cpssOsPrintf("\nIPv6 MC source Max tree depth is 32!!! Bucket address 0x%x\n",hwAddr);
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
                }
                else if (depth == (2 * PRV_CPSS_DXCH_LPM_MAX_NUM_OF_OCTETS_FOR_ADDRESS_CNS))
                {
                    /* For IPv6 MC in depth 32 all LPM lines must be NH/ECMP/Qos bucket type */
                    cpssOsPrintf("\nFrom depth %d all LPM lines must be NH/ECMP/QoS bucket type!!! Bucket address 0x%x\n",
                                 (2 * PRV_CPSS_DXCH_LPM_MAX_NUM_OF_OCTETS_FOR_ADDRESS_CNS), hwAddr);
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
                }
            }
        }
    }
    return GT_OK;
}

/* the child address calculation differs according to the child type.
   The group of child nodes are organized so that all child nodes
   of the same type are grouped together*/
GT_STATUS getFromTheGonOneNodeAddrAndSize(GT_U32 gonBaseAddr,
                                          GT_U32 *totalChildsTypeArr,
                                          GT_U32 rangeType,
                                          GT_U32 rangeTypeIndex,
                                          GT_U32 *gonNodeAddrPtr,
                                          GT_U32 *gonNodeSizePtr)
{
    GT_U32      lineHwAddress=0;
    GT_U32      baseRegularAddr=0;
    GT_U32      baseCompAddr=0;
    GT_U32      baseLeafAddr=0;

    baseRegularAddr=gonBaseAddr;
    baseCompAddr=gonBaseAddr+(PRV_CPSS_DXCH_LPM_RAM_FALCON_BUCKET_SIZE_REGULAR_BV_CNS * totalChildsTypeArr[1]);
    baseLeafAddr=baseCompAddr + totalChildsTypeArr[2];

    switch (rangeType)
    {
    case PRV_CPSS_DXCH_LPM_CHILD_LEAF_TYPE_E:/* leaf*/
        lineHwAddress = baseLeafAddr + (rangeTypeIndex/MAX_NUMBER_OF_LEAVES_IN_LPM_LINE_CNS);
        *gonNodeSizePtr = PRV_CPSS_DXCH_LPM_RAM_FALCON_BUCKET_SIZE_COMPRESSED_CNS;
        break;
    case PRV_CPSS_DXCH_LPM_CHILD_REGULAR_TYPE_E:/* regular */
        lineHwAddress = baseRegularAddr + (PRV_CPSS_DXCH_LPM_RAM_FALCON_BUCKET_SIZE_REGULAR_BV_CNS*rangeTypeIndex);
        *gonNodeSizePtr = PRV_CPSS_DXCH_LPM_RAM_FALCON_BUCKET_SIZE_REGULAR_BV_CNS;
        break;
    case PRV_CPSS_DXCH_LPM_CHILD_COMPRESSED_TYPE_E:/* compress*/
        lineHwAddress = baseCompAddr + rangeTypeIndex;
        *gonNodeSizePtr = PRV_CPSS_DXCH_LPM_RAM_FALCON_BUCKET_SIZE_COMPRESSED_CNS;
        break;
    default:
        cpssOsPrintf("\ngetFromTheGonOneNodeAddrAndSize illegal rangeType=%d\n",rangeType);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        break;
    }

    *gonNodeAddrPtr = lineHwAddress;
    return GT_OK;
}

GT_STATUS printLpmSip6(GT_U8 devNum, GT_U32 *bucketPtr, GT_U8 *baseAddress,
                       GT_U32 basePrefix, GT_U8 startAddress, GT_U32 endAddress,
                       GT_U32 nodeType,GT_U32 bankIndexOfTheGon,GT_U32 gonNodeAddr,
                       GT_BOOL isUc, PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT protocol,
                       GT_U32 depth, prvLpmDumpPrintInfoSip6_STC *printInfo)
{
    GT_STATUS status=GT_OK;
    GT_U32    numOfRanges;
    GT_U32    numOfRangesUsed=0;     /* parameter used when going over the lines of regular bit vector */
    GT_U32    rangesArr[256];        /* maximum 256 ranges per octet*/
    GT_U32    rangesTypesArr[256];   /* for each range keep its type (regular,compress,leaf) */
    GT_U32    rangesBitLineArr[256]; /* for each range keep its bit vector line */
    GT_U32    rangesTypeIndexArr[256]; /* what is the number of this range type
                                       from total number of lines with the same type -
                                       where it is located in the GON */
    GT_U32    *bucketDataPtr;   /* data read from HW, max lines read will be 6 lines for regular bit vector  */
    GT_U32    *bucketDataOneRangeGonPtr;   /* data of one range GON read from HW */
    GT_U32    leafData=0;       /* one leaf data read from hw (a leaf read from the line of leafs) */
    GT_U32    nodeSize;         /* size of the node
                                   regular have 6 lines, all the rest hold 1 line*/
    GT_U32    gonNodeSize;

    GT_U32    nodeChildAddressesArr[PRV_CPSS_DXCH_LPM_RAM_FALCON_MAX_SIZE_OF_BUCKET_IN_LPM_LINES_CNS];/* the child_pointer located at the beginning of each line
                                                                                            for regular node we have 6 pointers, for compress node one pointer */
    GT_U32    nodeTotalChildTypesArr[PRV_CPSS_DXCH_LPM_RAM_FALCON_MAX_SIZE_OF_BUCKET_IN_LPM_LINES_CNS][3];/* 6 elements in a regular node
                                                                                                3 types of child that can be for each range (leaf,regular,compress) */

    GT_U32    nodeTotalLinesPerTypeArr[6][3]; /* sum the number of lines in the GON according to the Type */
    GT_U32    nodeTotalBucketPerTypesArr[6][3];  /* sum the number of buckets (copmress, regular or leaf) in the GON according to the Type */

    GT_U32    numberOfEmbeddedLeafs=0;
    GT_U32    indexOfEmbeddedLeaf=0;
    GT_U32    value;


    GT_U32 hwAddr = gonNodeAddr;    /* in lines */
    GT_U32 bankIndex = bankIndexOfTheGon;
    GT_STATUS ret = GT_OK;

    GT_U8  addr[2*PRV_CPSS_DXCH_LPM_MAX_NUM_OF_OCTETS_FOR_ADDRESS_CNS] = {0};
    GT_U8  prefix = 0;
    GT_U32 basePrefixLength = 0;
    GT_U32 hwBucketAddr[PRV_CPSS_DXCH_LPM_RAM_FALCON_MAX_SIZE_OF_BUCKET_IN_LPM_LINES_CNS*PRV_CPSS_DXCH_LPM_RAM_FALCON_SIZE_OF_LPM_ENTRY_IN_WORDS_CNS];
    GT_U32 i = 0,j = 0;
    GT_U8 startAddr[2*PRV_CPSS_DXCH_LPM_MAX_NUM_OF_OCTETS_FOR_ADDRESS_CNS];
    GT_U8 endAddr[2*PRV_CPSS_DXCH_LPM_MAX_NUM_OF_OCTETS_FOR_ADDRESS_CNS];
    GT_U8 *baseAddrPtr = NULL;

    PRV_CPSS_DXCH_LPM_ECMP_ENTRY_STC ecmpEntry;

    GT_U32 nhPointer,leafType;
    GT_U32 entryType, nextBucketPointer,nextNodeType;
    GT_U32 nextBucketSize;
    GT_U32 bankIndexsOfTheGonsArray[6];  /* the indexs of the banks were the GON is located */


    cpssOsMemSet(&ecmpEntry,0,sizeof(ecmpEntry));

    /* Allocate memory for bucket data.
    Maximum size is a regular bucket --> 6 lines
    each line is a wide line = 4 words */
    bucketDataPtr = cpssOsMalloc(PRV_CPSS_DXCH_LPM_RAM_FALCON_MAX_SIZE_OF_BUCKET_IN_LPM_LINES_CNS * PRV_CPSS_DXCH_LPM_RAM_FALCON_SIZE_OF_LPM_ENTRY_IN_BYTES_CNS);
    if (bucketDataPtr == NULL)
     CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);

    cpssOsMemSet(bucketDataPtr, 0, PRV_CPSS_DXCH_LPM_RAM_FALCON_MAX_SIZE_OF_BUCKET_IN_LPM_LINES_CNS * PRV_CPSS_DXCH_LPM_RAM_FALCON_SIZE_OF_LPM_ENTRY_IN_BYTES_CNS);

    bucketDataOneRangeGonPtr = cpssOsMalloc(PRV_CPSS_DXCH_LPM_RAM_FALCON_MAX_SIZE_OF_BUCKET_IN_LPM_LINES_CNS * PRV_CPSS_DXCH_LPM_RAM_FALCON_SIZE_OF_LPM_ENTRY_IN_BYTES_CNS);
    if (bucketDataOneRangeGonPtr == NULL)
     CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);

    cpssOsMemSet(bucketDataOneRangeGonPtr, 0, PRV_CPSS_DXCH_LPM_RAM_FALCON_SIZE_OF_LPM_ENTRY_IN_BYTES_CNS);

    cpssOsMemSet(nodeChildAddressesArr,0,sizeof(nodeChildAddressesArr));
    cpssOsMemSet(nodeTotalChildTypesArr,0,sizeof(nodeTotalChildTypesArr));
    cpssOsMemSet(nodeTotalLinesPerTypeArr,0,sizeof(nodeTotalLinesPerTypeArr));
    cpssOsMemSet(nodeTotalBucketPerTypesArr,0,sizeof(nodeTotalBucketPerTypesArr));
    cpssOsMemSet(rangesArr,0,sizeof(rangesArr));
    cpssOsMemSet(rangesTypesArr,0,sizeof(rangesTypesArr));
    cpssOsMemSet(rangesBitLineArr,0,sizeof(rangesBitLineArr));
    cpssOsMemSet(rangesTypeIndexArr,0,sizeof(rangesTypeIndexArr));
    cpssOsMemSet(bankIndexsOfTheGonsArray,0,sizeof(bankIndexsOfTheGonsArray));
    cpssOsMemSet(hwBucketAddr,0,sizeof(hwBucketAddr));

    basePrefixLength = basePrefix + 8;
    baseAddrPtr = baseAddress + 1;
    switch(nodeType)
    {
    case PRV_CPSS_DXCH_LPM_CHILD_LEAF_TYPE_E:
        nodeSize = 1;
        cpssOsMemCpy(bucketDataPtr, bucketPtr, sizeof(GT_U32)); /* bucketDataPtr will hold one leaf data - 23 bits of data */

        ret = validateLeafSip6(devNum,  hwAddr, bucketDataPtr[0],
                               &entryType, &leafType, &nhPointer, &nextBucketPointer,
                                &nextNodeType,isUc);
        if (ret != GT_OK)
        {
            cpssOsFree(bucketDataPtr);
            CPSS_LOG_ERROR_AND_RETURN_MAC(status, LOG_ERROR_NO_MSG);
        }

        if(entryType==PRV_CPSS_DXCH_LPM_LEAF_ENTRY_TYPE_TRIGGER_E)/* "trigger" */
        {
            if (isUc == GT_TRUE)
            {
                cpssOsPrintf("\nUnicast tree points to a source tree!!! Bucket address 0x%x\n",hwAddr);
                cpssOsFree(bucketDataPtr);
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
            }
            else if (InSrcTree == GT_TRUE)
            {
                cpssOsPrintf("\nMulticast source tree points to a source tree!!! Bucket address 0x%x\n",hwAddr);
                cpssOsFree(bucketDataPtr);
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
            }
            else
            {
                /* It is last group bucket. The next pointer is not next hop
                   pointer type -- the src tree will be started */
                if (printInfo->printRangesSip6)
                {
                    printLpmMcGroupSip6(protocol, (GT_U8)basePrefixLength, startAddress, (GT_U8)endAddress);
                }
                InSrcTree = GT_TRUE;
                grpPrefix = basePrefixLength;
                basePrefixLength = 0;
                cpssOsMemCpy(baseGroupAddr,baseAddr,sizeof(baseAddr));
                cpssOsMemSet(baseAddr,0,sizeof(baseAddr));
                baseGroupAddrPtr = baseAddrPtr;
                baseAddrPtr = baseAddr;

                /* get the MC src GON */
                switch(nextNodeType)
                {
                case PRV_CPSS_DXCH_LPM_LEAF_NEXT_NODE_REGULAR_TYPE_E:/* regular */
                    nextBucketSize=PRV_CPSS_DXCH_LPM_RAM_FALCON_BUCKET_SIZE_REGULAR_BV_CNS;
                    break;
                case PRV_CPSS_DXCH_LPM_LEAF_NEXT_NODE_COMPRESSED_TYPE_E: /* compress */
                    nextBucketSize=PRV_CPSS_DXCH_LPM_RAM_FALCON_BUCKET_SIZE_COMPRESSED_CNS;
                    break;
                default:
                    cpssOsPrintf("\nillegal nextNodeType!!! Bucket address 0x%x\n",hwAddr);
                    cpssOsFree(bucketDataPtr);
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
                    break;
                }
                /* read one range (range number i) */
                status = prvCpssDxChReadTableMultiEntry(devNum,
                                                     CPSS_DXCH_SIP5_TABLE_LPM_MEM_E,
                                                     nextBucketPointer,
                                                     nextBucketSize,
                                                     bucketDataPtr);

                /* now we should print the MC SRC */
                ret = printLpmSip6(devNum,
                                   bucketDataPtr,
                                   baseAddrPtr,
                                   basePrefixLength,/* 0 */
                                   startAddress,
                                   endAddress,
                                   nodeType,
                                   bankIndexOfTheGon,
                                   gonNodeAddr,
                                   isUc,            /* GT_FALSE */
                                   protocol,
                                   depth+1,
                                   printInfo);


                if (ret != GT_OK)
                {
                    cpssOsFree(bucketDataPtr);
                    CPSS_LOG_ERROR_AND_RETURN_MAC(ret, LOG_ERROR_NO_MSG);
                }
            }
        }
        else
        {
            cpssOsMemSet(endAddr,0, sizeof(GT_U8) * (2 * PRV_CPSS_DXCH_LPM_MAX_NUM_OF_OCTETS_FOR_ADDRESS_CNS));
            cpssOsMemSet(startAddr,0, sizeof(GT_U8) * (2 * PRV_CPSS_DXCH_LPM_MAX_NUM_OF_OCTETS_FOR_ADDRESS_CNS));

            prefix = (GT_U8)basePrefix;
            prefixBase2Address(baseAddr ,prefix,1,addr);
            for (j = 8; j < prefix; j += 8)
            {
                startAddr[(j/8)-1] = addr[(j/8)-1];
                endAddr[(j/8)-1] = addr[(j/8)-1];
            }
            startAddr[(j/8)-1] = startAddress;
            endAddr[(j/8)-1] = (GT_U8)endAddress;
            if (printInfo->printRangesSip6)
            {
                if (protocol == PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E)
                {
                    outputAddress(startAddr, PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS, protocol);
                    cpssOsPrintf(" - ");
                    outputAddress(endAddr, PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS, protocol);
                }
                else if (protocol == PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E)
                {
                    outputAddress(startAddr, PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS, protocol);
                    cpssOsPrintf(" - ");
                    outputAddress(endAddr, PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS, protocol);
                }
                else /* PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E */
                {
                    outputAddress(startAddr, PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_FCOE_PROTOCOL_CNS, protocol);
                    cpssOsPrintf(" - ");
                    outputAddress(endAddr, PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_FCOE_PROTOCOL_CNS, protocol);
                }
                cpssOsPrintf(" / %02d ", prefix);
                if (leafType == PRV_CPSS_DXCH_LPM_LEAF_REGULAR_TYPE_ENT)/* regular */
                {
                    cpssOsPrintf(" -> NH %X\n",nhPointer);
                }
                else /* CPSS_DXCH_LPM_LEAF_MULTIPATH_TYPE_E */
                {
                    cpssOsPrintf(" -> ECMP/QoS %X\n",nhPointer);
                }
            }
            if (nhPointer >= PRV_CPSS_DXCH_LPM_RAM_NUM_OF_DEFAULT_ENTRIES)
                printInfo->numOfNonDefaultNextHopPointersSip6++;
        }
        break;
    case PRV_CPSS_DXCH_LPM_CHILD_REGULAR_TYPE_E:
        nodeSize = PRV_CPSS_DXCH_LPM_RAM_FALCON_BUCKET_SIZE_REGULAR_BV_CNS;
        cpssOsMemCpy(bucketDataPtr, bucketPtr,
                     PRV_CPSS_DXCH_LPM_RAM_FALCON_BUCKET_SIZE_REGULAR_BV_CNS * PRV_CPSS_DXCH_LPM_RAM_FALCON_SIZE_OF_LPM_ENTRY_IN_BYTES_CNS);/* bucketDataPtr will hold 6 wide lines - 768 bits of data  */

        /* the data we have in bucketDataPtr is already the next Bit vector for the range,
           it was read in previous stage. this is the range we want to work on */
        status = validateBitVectorOfRegularBucketSip6(devNum,bucketDataPtr, gonNodeAddr, basePrefixLength);
        if (status != GT_OK)
        {
            cpssOsFree(bucketDataPtr);
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }

        status = getNumOfRangesFromHWSip6(protocol,
                                          depth,
                                          nodeType,
                                          bucketDataPtr,
                                          gonNodeAddr,
                                          nodeChildAddressesArr,
                                          nodeTotalChildTypesArr,
                                          nodeTotalLinesPerTypeArr,
                                          nodeTotalBucketPerTypesArr,
                                          &numOfRanges,
                                          rangesArr,
                                          rangesTypesArr,
                                          rangesTypeIndexArr,
                                          rangesBitLineArr);
        if (status != GT_OK)
        {
            cpssOsFree(bucketDataPtr);
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }

        status = checkDepthSip6(isUc,protocol,depth,hwAddr);
        if (status != GT_OK)
        {
            cpssOsFree(bucketDataPtr);
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }

        if (printInfo->printRangesSip6)
        {
            cpssOsPrintf (" %X \n",hwAddr);
        }

        /* update the print info for the first octet */
        printInfo->lpmLinesUsedSip6[bankIndex] += (nodeSize);
        printInfo->nodeCounterPerTypePerOctetSip6[depth][(PRV_CPSS_DXCH_LPM_CHILD_REGULAR_TYPE_E-1)]++;
        printInfo->nodeCounterPerTypeSip6[2] += (nodeSize);

        for (j=0;j<nodeSize;j++)
        {
            for (i = numOfRangesUsed; i < numOfRanges; i++)
            {
                if (rangesBitLineArr[i]!=j)
                {
                    break; /* break the loop on ranges and move to the next bit vector line */
                }

                /* calculate the address and the size of a single node from the GON
                   (6 lines for regular all the rest 1 line) according to the
                   parameters we got from getNumOfRangesFromHWSip6 */
                ret = getFromTheGonOneNodeAddrAndSize(nodeChildAddressesArr[j],
                                                       nodeTotalChildTypesArr[j],
                                                       rangesTypesArr[i],
                                                       rangesTypeIndexArr[i],
                                                       &gonNodeAddr,
                                                       &gonNodeSize);
                if (ret != GT_OK)
                {
                    cpssOsPrintf("\nInconsistency exists in call to getFromTheGonOneNodeAddrAndSize!!!\n");
                    cpssOsFree(bucketDataPtr);
                    CPSS_LOG_ERROR_AND_RETURN_MAC(ret, LOG_ERROR_NO_MSG); /* continue to other branches in the tree */
                }

                 /* read one range (range number i) */
                status = prvCpssDxChReadTableMultiEntry(devNum,
                                                     CPSS_DXCH_SIP5_TABLE_LPM_MEM_E,
                                                     gonNodeAddr,
                                                     gonNodeSize,
                                                     bucketDataPtr);
                if (status != GT_OK)
                {
                    cpssOsPrintf("\nInconsistency exists in call to prvCpssDxChReadTableMultiEntry!!!\n");
                    cpssOsFree(bucketDataPtr);
                    CPSS_LOG_ERROR_AND_RETURN_MAC(ret, LOG_ERROR_NO_MSG);
                }


                cpssOsMemCpy(hwBucketAddr, bucketDataPtr, sizeof(hwBucketAddr));
                startAddress = (GT_U8)rangesArr[i];
                endAddress = (i == (numOfRanges-1))? 255 : (rangesArr[i+1]- 1);
                if (endAddress > 255)
                {
                    cpssOsPrintf("Err!!");
                }
                if ( (isUc == GT_FALSE)    &&
                     (InSrcTree == GT_TRUE)&&
                     (basePrefix == 0)    )
                {
                    /* I am in mc src tree in zero bucket */
                    baseAddress[0] = startAddress;
                    baseAddrPtr = baseAddr;
                }
                else
                    baseAddress[1] = startAddress;

                /* the hwAddr is a relative address to the beginning of the LPM *//* should be changed later */
                bankIndexOfTheGon = gonNodeAddr/PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.totalNumOfLinesInBlockIncludingGap;
                bankIndexsOfTheGonsArray[j]=bankIndexOfTheGon;

                if(rangesTypesArr[i]==PRV_CPSS_DXCH_LPM_CHILD_LEAF_TYPE_E)
                {
                     /* need to find the correct leaf out of all the leafs in the line - in HW the first leaf are keept in last bits:
                       0-22: leaf 4
                       23-45: leaf 3
                       46-68: leaf 2
                       69-91: leaf 1
                       92-114: leaf 0 */
                    U32_GET_FIELD_IN_ENTRY_MAC(bucketDataPtr,
                                               (MAX_NUMBER_OF_LEAVES_IN_LPM_LINE_CNS-(rangesTypeIndexArr[i]%MAX_NUMBER_OF_LEAVES_IN_LPM_LINE_CNS)-1)*23,
                                               23,leafData);

                    ret = printLpmSip6(devNum, &leafData,
                                       baseAddrPtr, basePrefixLength, startAddress, endAddress,
                                       rangesTypesArr[i],bankIndexOfTheGon,gonNodeAddr, isUc, protocol,
                                       depth,
                                       printInfo);
                    if (ret != GT_OK)
                    {
                        cpssOsFree(bucketDataPtr);
                        CPSS_LOG_ERROR_AND_RETURN_MAC(ret, LOG_ERROR_NO_MSG);
                    }

                    printInfo->nodeCounterPerTypePerOctetSip6[depth][(PRV_CPSS_DXCH_LPM_CHILD_LEAF_TYPE_E-1)]++;
                }
                else
                {
                    ret = printLpmSip6(devNum, hwBucketAddr,
                                       baseAddrPtr, basePrefixLength, startAddress, endAddress,
                                       rangesTypesArr[i],bankIndexOfTheGon,gonNodeAddr, isUc, protocol,
                                       depth+1,
                                       printInfo);
                    if (ret != GT_OK)
                    {
                        cpssOsFree(bucketDataPtr);
                        CPSS_LOG_ERROR_AND_RETURN_MAC(ret, LOG_ERROR_NO_MSG);
                    }
                }

                if( (isUc == GT_FALSE)      &&
                    (basePrefixLength == 0)       &&
                    (InSrcTree == GT_TRUE)  )
                {
                    /* The source tree was displayed and the other group regions
                       will be printed*/

                    InSrcTree = GT_FALSE;
                    basePrefixLength = grpPrefix;
                    cpssOsMemCpy(baseAddr,baseGroupAddr,sizeof(baseAddr));
                    baseAddrPtr = baseGroupAddrPtr;
                    if (printInfo->printRangesSip6)
                    {
                        cpssOsPrintf(" End of Src tree for this group. \n");
                    }
                }
                numOfRangesUsed++;
            }

           /*  after printing the all ranges we need to udate the number of lines occupied by the leafs
                those are not updated in the print recursive calls.
                all calculation of how many lines is occupied by the leafs is
                calculated in getRootBucketRangesSip6.*/

            /* update the print info for the first octet */
            printInfo->lpmLinesUsedSip6[bankIndexsOfTheGonsArray[j]] += (nodeTotalLinesPerTypeArr[j][(PRV_CPSS_DXCH_LPM_CHILD_LEAF_TYPE_E-1)]);
            printInfo->nodeCounterPerTypeSip6[1] += nodeTotalBucketPerTypesArr[j][0]; /* add the number of leafs calculated for this GON */
        }

        break;
    case PRV_CPSS_DXCH_LPM_CHILD_COMPRESSED_TYPE_E:
        nodeSize = PRV_CPSS_DXCH_LPM_RAM_FALCON_BUCKET_SIZE_COMPRESSED_CNS;
        cpssOsMemCpy(bucketDataPtr, bucketPtr, PRV_CPSS_DXCH_LPM_RAM_FALCON_SIZE_OF_LPM_ENTRY_IN_BYTES_CNS);/* bucketDataPtr will hold 1 wide line - 128 bits of data  */

        /* the data we have in bucketDataPtr is already the next Bit vector for the range, it was read in previous stage  */
        status = validateBitVectorOfCompressBucketSip6(devNum, bucketDataPtr, gonNodeAddr,isUc);
        if (status != GT_OK)
        {
            cpssOsFree(bucketDataPtr);
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }

        status = getNumOfRangesFromHWSip6(protocol,
                                          depth,
                                          nodeType,
                                          bucketDataPtr,
                                          gonNodeAddr,
                                          nodeChildAddressesArr,
                                          nodeTotalChildTypesArr,
                                          nodeTotalLinesPerTypeArr,
                                          nodeTotalBucketPerTypesArr,
                                          &numOfRanges,
                                          rangesArr,
                                          rangesTypesArr,
                                          rangesTypeIndexArr,
                                          rangesBitLineArr);
        if (status != GT_OK)
        {
            cpssOsFree(bucketDataPtr);
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }
        status = checkDepthSip6(isUc,protocol,depth,hwAddr);
        if (status != GT_OK)
        {
            cpssOsFree(bucketDataPtr);
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }

        if (printInfo->printRangesSip6)
        {
            cpssOsPrintf (" %X \n",hwAddr);
        }


        /* get the number of embedded leafs in the compress bit vector */
        /* bit 113-112 Embedded_leaf */
        U32_GET_FIELD_IN_ENTRY_MAC(bucketDataPtr,112,2,value);
        numberOfEmbeddedLeafs = value;

        /* update the print info for the first octet */
        printInfo->lpmLinesUsedSip6[bankIndex] += (nodeSize);

        printInfo->nodeCounterPerTypePerOctetSip6[depth][(PRV_CPSS_DXCH_LPM_CHILD_COMPRESSED_TYPE_E-1+numberOfEmbeddedLeafs)]++;
        printInfo->nodeCounterPerTypeSip6[3] += (nodeSize);

        /* go over the ranges and print them recursively */
        for (i = 0; i < numOfRanges; i++)
        {
            if ((numberOfEmbeddedLeafs!=0)&&(rangesTypesArr[i]==PRV_CPSS_DXCH_LPM_CHILD_LEAF_TYPE_E))/* leaf */
            {
                /* if the range is of an embedded leaf the validity of the embedded
                   leaf was done in the call to validateBitVectorOfCompressBucketSip6 */

                /* the leaf data is already located in bucketDataPtr so no need to read from HW
                   just to print it */

                    switch(indexOfEmbeddedLeaf)
                    {
                    case 0:/*first embedded leaf located at bit 89-111*/
                        U32_GET_FIELD_IN_ENTRY_MAC(bucketDataPtr, 89, 23, value);
                        break;
                    case 1:/*second embedded leaf located at bit 66-88*/
                        U32_GET_FIELD_IN_ENTRY_MAC(bucketDataPtr,66,23,value);
                        break;
                    case 2:/*third embedded leaf located at bit 43-65*/
                        U32_GET_FIELD_IN_ENTRY_MAC(bucketDataPtr,43,23,value);
                        break;
                    default:
                        cpssOsPrintf("\n illegal number of embedded leafs!!! Bucket address 0x%x\n", hwAddr);
                        cpssOsFree(bucketDataPtr);
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
                    }

                    leafData = value;
                    indexOfEmbeddedLeaf++;/* an index for knowing how many embedded leafs we already dealt with */

                    cpssOsMemCpy(hwBucketAddr, bucketDataPtr, sizeof(hwBucketAddr)); /* bitmap pointer first line */
                    startAddress = (GT_U8)rangesArr[i];
                    endAddress = (i == (numOfRanges-1))? 255 : (rangesArr[i+1]- 1);
                    if (endAddress > 255)
                    {
                        cpssOsPrintf("Err!!");
                    }
                    if ( (isUc == GT_FALSE)    &&
                         (InSrcTree == GT_TRUE)&&
                         (basePrefix == 0)    )
                    {
                        /* I am in mc src tree in zero bucket */
                        baseAddress[0] = startAddress;
                        baseAddrPtr = baseAddr;
                    }
                    else
                        baseAddress[1] = startAddress;

                    /* the hwAddr is a relative address to the beginning of the LPM *//* should be changed later */
                    bankIndexOfTheGon = gonNodeAddr/PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.totalNumOfLinesInBlockIncludingGap;

                    ret = printLpmSip6(devNum,
                                       &leafData,
                                       baseAddrPtr,
                                       basePrefixLength,
                                       startAddress,
                                       endAddress,
                                       rangesTypesArr[i],
                                       bankIndexOfTheGon,
                                       gonNodeAddr,
                                       isUc,
                                       protocol,
                                       depth,/* we leave the same depth, since we are still in the same octet, just want to print the leaf */
                                       printInfo);

                     if (ret != GT_OK)
                    {
                        cpssOsPrintf("\nInconsistency exists in call to printLpmSip6 !!!\n");
                        cpssOsFree(bucketDataPtr);
                        CPSS_LOG_ERROR_AND_RETURN_MAC(ret, LOG_ERROR_NO_MSG);
                    }
            }
            else
            {
                /* calculate the address and the size of a single node from the GON
                   (6 lines for regular all the rest 1 line) according to the
                   parameters we got from getNumOfRangesFromHWSip6 */
                ret = getFromTheGonOneNodeAddrAndSize(nodeChildAddressesArr[0],
                                                      nodeTotalChildTypesArr[0],
                                                      rangesTypesArr[i],
                                                      rangesTypeIndexArr[i],
                                                      &gonNodeAddr,
                                                      &gonNodeSize);
                if (ret != GT_OK)
                {
                    cpssOsPrintf("\nInconsistency exists in call to getFromTheGonOneNodeAddrAndSize!!!\n");
                    cpssOsFree(bucketDataPtr);
                    CPSS_LOG_ERROR_AND_RETURN_MAC(ret, LOG_ERROR_NO_MSG);

                }

                /* read one range (range number i) */
                ret = prvCpssDxChReadTableMultiEntry(devNum,
                                                     CPSS_DXCH_SIP5_TABLE_LPM_MEM_E,
                                                     gonNodeAddr,
                                                     gonNodeSize,
                                                     bucketDataOneRangeGonPtr);
                if (ret != GT_OK)
                {
                    cpssOsPrintf("\nInconsistency exists in call to prvCpssDxChReadTableMultiEntry !!!\n");
                    cpssOsFree(bucketDataPtr);
                    cpssOsFree(bucketDataOneRangeGonPtr);
                    CPSS_LOG_ERROR_AND_RETURN_MAC(ret, LOG_ERROR_NO_MSG);
                }
                startAddress = (GT_U8)rangesArr[i];
                endAddress = (i == (numOfRanges-1))? 255 : (rangesArr[i+1]- 1);
                if (endAddress > 255)
                {
                    cpssOsPrintf("Err!!");
                    cpssOsFree(bucketDataPtr);
                    cpssOsFree(bucketDataOneRangeGonPtr);
                    CPSS_LOG_ERROR_AND_RETURN_MAC(ret, LOG_ERROR_NO_MSG);
                }
                if ( (isUc == GT_FALSE)    &&
                     (InSrcTree == GT_TRUE)&&
                     (basePrefix == 0)    )
                {
                    /* I am in mc src tree in zero bucket */
                    baseAddress[0] = startAddress;
                    baseAddrPtr = baseAddr;
                }
                else
                {
                    baseAddress[1] = startAddress;
                }

                 /* the hwAddr is a relative address to the beginning of the LPM *//* should be changed later */
                bankIndexOfTheGon = gonNodeAddr/PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.totalNumOfLinesInBlockIncludingGap;

                if (rangesTypesArr[i]==PRV_CPSS_DXCH_LPM_CHILD_LEAF_TYPE_E)/* leaf */
                {
                    /* not an embedded leaf that was treated above but a regular leaf */

                    /* need to find the correct leaf out of all the leafs in the line - in HW the first leaf are keept in last bits:
                       0-22: leaf 4
                       23-45: leaf 3
                       46-68: leaf 2
                       69-91: leaf 1
                       92-114: leaf 0 */
                    U32_GET_FIELD_IN_ENTRY_MAC(bucketDataOneRangeGonPtr,
                                               (MAX_NUMBER_OF_LEAVES_IN_LPM_LINE_CNS-(rangesTypeIndexArr[i]%MAX_NUMBER_OF_LEAVES_IN_LPM_LINE_CNS)-1)*23,
                                               23,leafData);

                    ret = printLpmSip6(devNum,
                                       &leafData,
                                       baseAddrPtr,
                                       basePrefixLength,
                                       startAddress,
                                       endAddress,
                                       rangesTypesArr[i],
                                       bankIndexOfTheGon,
                                       gonNodeAddr,
                                       isUc,
                                       protocol,
                                       depth,/* we leave the same depth, since we are still in the same octet, just want to print the leaf */
                                       printInfo);

                    if (ret != GT_OK)
                    {
                        cpssOsPrintf("\nInconsistency exists in call to printLpmSip6 !!!\n");
                        cpssOsFree(bucketDataPtr);
                        cpssOsFree(bucketDataOneRangeGonPtr);
                        CPSS_LOG_ERROR_AND_RETURN_MAC(ret, LOG_ERROR_NO_MSG);
                    }
                    printInfo->nodeCounterPerTypePerOctetSip6[depth][(PRV_CPSS_DXCH_LPM_CHILD_LEAF_TYPE_E-1)]++;
                }
                else
                {
                    /* the range is regular or compress */
                    ret = printLpmSip6(devNum,
                                   bucketDataOneRangeGonPtr,
                                   baseAddrPtr,
                                   basePrefixLength,
                                   startAddress,
                                   endAddress,
                                   rangesTypesArr[i],
                                   bankIndexOfTheGon,
                                   gonNodeAddr,
                                   isUc,
                                   protocol,
                                   depth+1,
                                   printInfo);

                    if (ret != GT_OK)
                    {
                        cpssOsPrintf("\nInconsistency exists in call to printLpmSip6 !!!\n");
                        cpssOsFree(bucketDataPtr);
                        cpssOsFree(bucketDataOneRangeGonPtr);
                        CPSS_LOG_ERROR_AND_RETURN_MAC(ret, LOG_ERROR_NO_MSG);
                    }
                }
            }

            if( (isUc == GT_FALSE)      &&
                (basePrefixLength == 0)       &&
                (InSrcTree == GT_TRUE)  )
            {
                /* The source tree was displayed and the other group regions
                   will be printed*/

                InSrcTree = GT_FALSE;
                basePrefixLength = grpPrefix;
                cpssOsMemCpy(baseAddr,baseGroupAddr,sizeof(baseAddr));
                baseAddrPtr = baseGroupAddrPtr;
                if (printInfo->printRangesSip6)
                {
                    cpssOsPrintf(" End of Src tree for this group. \n");
                }
            }
        }
        /* update the print info for the first octet */
        printInfo->lpmLinesUsedSip6[bankIndexOfTheGon] += (nodeTotalLinesPerTypeArr[j][(PRV_CPSS_DXCH_LPM_CHILD_LEAF_TYPE_E-1)]);
        printInfo->nodeCounterPerTypeSip6[2] += nodeTotalBucketPerTypesArr[0][1];/* compress have only one line --> one element in nodeTotalBucketPerTypesArr */
        printInfo->nodeCounterPerTypeSip6[1] += nodeTotalBucketPerTypesArr[0][0];/* add the number of leafs calculated for this GON */
        break;
    default:
        cpssOsPrintf("\n illegal nodeType %d!!! Bucket address 0x%x\n",nodeType,hwAddr);
        cpssOsFree(bucketDataPtr);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        break;
    }

    cpssOsFree(bucketDataPtr);
    cpssOsFree(bucketDataOneRangeGonPtr);
    return GT_OK;
}


static GT_STATUS compareBucket(GT_U8 devNum,GT_U32 hwAddr,GT_U32 *firstBucketDataPtr ,GT_U32 * secondBucketDataPtr,
                                   GT_U32 numOfRanges,CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT     nodeType ,GT_BOOL ignoreAgingBit)
{
    GT_U8 agingBit;
    GT_U8 pointerType;
    GT_U32 i;
    GT_U32 firstHwBucketAddr = 0;
    GT_U32 secondHwBucketAddr = 0;
    GT_U32 bitVectorSize = 0;

    switch(nodeType)
    {
        case CPSS_DXCH_LPM_COMPRESSED_1_NODE_PTR_TYPE_E:
        case CPSS_DXCH_LPM_COMPRESSED_2_NODE_PTR_TYPE_E:
        case CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E:

            if(nodeType == CPSS_DXCH_LPM_COMPRESSED_1_NODE_PTR_TYPE_E)
            {
                bitVectorSize = PRV_CPSS_DXCH_LPM_RAM_BUCKET_BIT_VEC_SIZE_COMPRESSED_1_CNS;
            }
            else if(nodeType == CPSS_DXCH_LPM_COMPRESSED_2_NODE_PTR_TYPE_E)
            {
                bitVectorSize = PRV_CPSS_DXCH_LPM_RAM_BUCKET_BIT_VEC_SIZE_COMPRESSED_2_CNS;
            }
            else
            {
                bitVectorSize = PRV_CPSS_DXCH_LPM_RAM_BUCKET_BIT_VEC_SIZE_REGULAR_CNS;
            }

            /*compare bitmap*/
            if(0!=cpssOsMemCmp(firstBucketDataPtr,secondBucketDataPtr,bitVectorSize))
            {
                cpssOsPrintf("Half mode validation failed .Error on comparing bit vector of bucket at line %d with bucket at line %d.\n", hwAddr,hwAddr+PRV_CPSS_DXCH_LPM_RAM_DBG_BC3_HALF_MEM_MODE_OFFSET_MAC(devNum));
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
            }

            /*compare ranges*/
            for(i=0;i<numOfRanges;i++)
            {
                firstHwBucketAddr = firstBucketDataPtr[(bitVectorSize + i)*PRV_CPSS_DXCH_LPM_RAM_SIZE_OF_LPM_ENTRY_IN_WORDS_CNS];
                secondHwBucketAddr = secondBucketDataPtr[(bitVectorSize + i)*PRV_CPSS_DXCH_LPM_RAM_SIZE_OF_LPM_ENTRY_IN_WORDS_CNS];

                pointerType = (firstHwBucketAddr & 0x3);

                if(pointerType!= (secondHwBucketAddr& 0x3))
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
                }
                /*Aging bit is not handled by HW at the second half of RAM bank,so need to sync first and second half before comparison*/
                if(ignoreAgingBit == GT_TRUE)
                {
                    if(pointerType!= 0)
                    {   /*copy aging bit from  first bucket to second bucket*/
                        agingBit = (firstHwBucketAddr>>PRV_DXCH_LPM_RAM_DBG_BC3_AGING_BIT_LOCATION_CNS)&0x1;

                        if(agingBit == 0x1)
                        {
                            secondHwBucketAddr|= 0x1<<PRV_DXCH_LPM_RAM_DBG_BC3_AGING_BIT_LOCATION_CNS;
                        }
                        else
                        {
                           secondHwBucketAddr&= (~(0x1<<PRV_DXCH_LPM_RAM_DBG_BC3_AGING_BIT_LOCATION_CNS));
                        }
                    }
                }

                if(firstHwBucketAddr!=secondHwBucketAddr)
                {
                    cpssOsPrintf("Half mode validation failed .Error on comparing bucket at line %d with bucket at line %d.\n", hwAddr,hwAddr+PRV_CPSS_DXCH_LPM_RAM_DBG_BC3_HALF_MEM_MODE_OFFSET_MAC(devNum));
                    cpssOsPrintf("Word  0x%x is different from word 0x%x \n", firstHwBucketAddr,secondHwBucketAddr);
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
                }
            }
            break;

          default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
            break;

    }



    return GT_OK;
}

GT_STATUS getRootBucketRanges(GT_U8 devNum, GT_U32 vrId,
                              PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT protocol,
                              GT_U32 *bitMapSize, GT_U32 *numOfRanges,
                              GT_U32 *ranges, GT_U32 *bucketDataPtr,
                              prvLpmDumpPrintInfo_STC *printInfo)
{
    GT_U32                              rootNodeAddr = 0;
    CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT nodeType = 0;
    GT_STATUS                           status = 0;
    GT_U32                              i = 0;
    GT_U32                              blockIndex = 0;
    GT_U32                              duplicateBucketData[PRV_CPSS_DXCH_LPM_RAM_MAX_SIZE_OF_BUCKET_IN_LPM_LINES_CNS];

    if (PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        /* the root bucket for all protocols are located by default in
           the init phase in LPM block 0*/
        blockIndex=0;
    }
    else
    {
        switch (protocol)
        {
        case PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E:
            blockIndex = addrLookupMemIpv4Array[0];
            break;

        case PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E:
            blockIndex = addrLookupMemIpv6Array[0];
            break;

        case PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E:
            blockIndex = addrLookupMemFcoeArray[0];
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }

    status = prvCpssDxChLpmHwVrfEntryRead(devNum, vrId, protocol,
                                          &nodeType, &rootNodeAddr);
    if (status != GT_OK)
    {
        return status;
    }

    if (nodeType == CPSS_DXCH_LPM_COMPRESSED_1_NODE_PTR_TYPE_E)
    {
        *bitMapSize = PRV_CPSS_DXCH_LPM_RAM_BUCKET_BIT_VEC_SIZE_COMPRESSED_1_CNS;
        /* hw pointer in LPM entry is in LPM lines*/
    }
    else /* CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E */
    {
        *bitMapSize = PRV_CPSS_DXCH_LPM_RAM_BUCKET_BIT_VEC_SIZE_REGULAR_CNS;
    }

    /* read the bit vector */
    status = prvCpssDxChReadTableMultiEntry(devNum,
                                         CPSS_DXCH_SIP5_TABLE_LPM_MEM_E,
                                         rootNodeAddr, *bitMapSize, bucketDataPtr);
    if (status != GT_OK)
    {
        return status;
    }

    if(PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.lpmMemMode == PRV_CPSS_DXCH_LPM_RAM_MEM_MODE_HALF_MEM_SIZE_E)
    {
        /* read the bit vector */
        status = prvCpssDxChReadTableMultiEntry(devNum,
                                         CPSS_DXCH_SIP5_20_TABLE_LPM_MEM_PIPE_1_E,
                                         rootNodeAddr, *bitMapSize, duplicateBucketData);
        if (status != GT_OK)
        {
            return status;
        }


    }

    *numOfRanges = getNumOfRangesFromHW(nodeType, bucketDataPtr, 0);

    /* read the ranges (the LPM lines after the bit vector) */
    status = prvCpssDxChReadTableMultiEntry(devNum,
                                         CPSS_DXCH_SIP5_TABLE_LPM_MEM_E,
                                         rootNodeAddr + (*bitMapSize), *numOfRanges,
                                         &bucketDataPtr[(*bitMapSize)*PRV_CPSS_DXCH_LPM_RAM_SIZE_OF_LPM_ENTRY_IN_WORDS_CNS]);
    if (status != GT_OK)
    {
        return status;
    }

    if(PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.lpmMemMode == PRV_CPSS_DXCH_LPM_RAM_MEM_MODE_HALF_MEM_SIZE_E)
    {
        /* read the ranges (the LPM lines after the bit vector) */
        status = prvCpssDxChReadTableMultiEntry(devNum,
                                         CPSS_DXCH_SIP5_20_TABLE_LPM_MEM_PIPE_1_E,
                                         rootNodeAddr + (*bitMapSize), *numOfRanges, &duplicateBucketData[(*bitMapSize)*PRV_CPSS_DXCH_LPM_RAM_SIZE_OF_LPM_ENTRY_IN_WORDS_CNS]);
        if (status != GT_OK)
        {
            return status;
        }

        status = compareBucket(devNum,rootNodeAddr,bucketDataPtr,duplicateBucketData,*numOfRanges,nodeType,GT_TRUE);

        if (status != GT_OK)
        {
            return status;
        }
    }


    if (nodeType == CPSS_DXCH_LPM_COMPRESSED_1_NODE_PTR_TYPE_E)
    {
        ranges[0] = 0;
        ranges[1] = bucketDataPtr[0] & 0xFF;
        ranges[2] = (bucketDataPtr[0] >> 8) & 0xFF;
        ranges[3] = (bucketDataPtr[0] >> 16) & 0xFF;
        ranges[4] = (bucketDataPtr[0] >> 24) & 0xFF;
    }
    else /* CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E */
    {
        status = validateBitVectorOfRegularBucket(bucketDataPtr, rootNodeAddr, 8);
        if (status != GT_OK)
        {
            return GT_OK; /* continue to other branches in the tree */
        }
        for (i = 0; i < *numOfRanges; i++)
        {
            ranges[i] = (GT_U8)getBaseAddrFromHW(CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E, i, bucketDataPtr, 0);
        }
    }

    /* update the print info for the first octet */
    printInfo->lpmLinesUsed[blockIndex] += (*bitMapSize + *numOfRanges);
    printInfo->lpmLinesPerOctet[0] = (*bitMapSize + *numOfRanges);
    printInfo->bucketSizePerType[nodeType] = (*bitMapSize + *numOfRanges);
    printInfo->bucketPerType[nodeType] = 1;
    printInfo->bucketPerOctet[0] = 1;
    printInfo->bucketTypePerOctet[0][nodeType] = 1;
    return GT_OK;
}

/* this function get the root bucket (regular bit vector) pointed by VR table */
GT_STATUS getRootBucketRangesSip6(GT_U8 devNum,
                                  GT_U32 vrId,
                                  PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT protocol,
                                  GT_U32 *nodeSize,
                                  GT_U32 nodeChildAddressesArr[6],
                                  GT_U32 nodeTotalChildTypesArr[6][3],
                                  GT_U32 nodeTotalLinesPerTypeArr[6][3],
                                  GT_U32 nodeTotalBucketPerTypesArr[6][3],
                                  GT_U32 *numOfRanges,
                                  GT_U32 *ranges,
                                  GT_U32 *rangesTypes,
                                  GT_U32 *rangesTypeIndex,
                                  GT_U32 *rangesBitLine,
                                  GT_U32 *bucketDataPtr,
                                  GT_U32 depth,
                                  prvLpmDumpPrintInfoSip6_STC *printInfo)
{
    GT_U32                              rootNodeAddr = 0;
    PRV_CPSS_DXCH_LPM_CHILD_TYPE_ENT    nodeType = 0;
    GT_STATUS                           status = 0;
    GT_U32                              bankIndex = 0;

    /* the root bucket for all protocols are located by default in
       the init phase in LPM bank 0*/
    bankIndex=0;

    /* get the nodeType and the address of the regular bit vector pointed by the VR table */
    status = prvCpssDxChLpmHwVrfEntryReadSip6(devNum, vrId, protocol,
                                              &nodeType, &rootNodeAddr);
    if (status != GT_OK)
    {
        return status;
    }

    /* for SIP6 the Root bucket can only be of Regular type */
    if (nodeType == PRV_CPSS_DXCH_LPM_CHILD_REGULAR_TYPE_E)
    {
        /* hw pointer in LPM entry is in LPM lines*/
        *nodeSize=PRV_CPSS_DXCH_LPM_RAM_FALCON_BUCKET_SIZE_REGULAR_BV_CNS;
    }
    else
    {
        cpssOsPrintf("\nfor SIP6 the Root bucket can only be of Regular type!!! Root bucket address 0x%x\n",rootNodeAddr);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);

    }

    /* read the bit vector */
    status = prvCpssDxChReadTableMultiEntry(devNum,
                                            CPSS_DXCH_SIP5_TABLE_LPM_MEM_E,
                                            rootNodeAddr, *nodeSize, bucketDataPtr);
    if (status != GT_OK)
    {
        cpssOsPrintf("\nError in calling prvCpssDxChReadTableMultiEntry!!! Root bucket address 0x%x\n",rootNodeAddr);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    status = validateBitVectorOfRegularBucketSip6(devNum,bucketDataPtr, rootNodeAddr, 8);
    if (status != GT_OK)
    {
        return status;
    }

    status = getNumOfRangesFromHWSip6(protocol,
                                      depth,
                                      nodeType,
                                      bucketDataPtr,
                                      rootNodeAddr,
                                      nodeChildAddressesArr,
                                      nodeTotalChildTypesArr,
                                      nodeTotalLinesPerTypeArr,
                                      nodeTotalBucketPerTypesArr,
                                      numOfRanges,
                                      ranges,
                                      rangesTypes,
                                      rangesTypeIndex,
                                      rangesBitLine);
    if (status != GT_OK)
    {
        return status;
    }

    /* update the print info for the first octet */
    printInfo->lpmLinesUsedSip6[bankIndex] += *nodeSize;
    printInfo->nodeCounterPerTypePerOctetSip6[0][(nodeType-1)] = 1; /* octet=0, nodeType=CPSS_DXCH_LPM_CHILD_REGULAR_TYPE_E */
    printInfo->nodeCounterPerTypeSip6[2] = 1;                /* the root is a regulat bit vector */

    return GT_OK;
}

GT_STATUS printLpmIpv4UcTable(GT_U8 devNum, GT_U32 vrId, prvLpmDumpPrintInfo_STC *printInfo)
{
    GT_U32    numOfRanges;
    GT_U32    ranges[256];
    GT_U32    endAddress;
    GT_U32    *bucketDataPtr;
    GT_U32    bitMapSize;
    GT_U32    i;
    GT_STATUS ret;
    GT_BOOL   needToFreeBucketDataPtr=GT_FALSE;

     /* Allocate memory for bucket data. Maximum size is regular bucket that
       uses 256 lpm lines for ranges + 11 lpm lines for bitmap */
    bucketDataPtr = cpssOsMalloc(PRV_CPSS_DXCH_LPM_RAM_MAX_SIZE_OF_BUCKET_IN_LPM_LINES_CNS * sizeof(GT_U32));
    if (bucketDataPtr == NULL)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);

    cpssOsMemSet(bucketDataPtr, 0, PRV_CPSS_DXCH_LPM_RAM_MAX_SIZE_OF_BUCKET_IN_LPM_LINES_CNS * sizeof(GT_U32));
    cpssOsMemSet(baseAddr, 0, sizeof(GT_U8)* PRV_CPSS_DXCH_LPM_MAX_NUM_OF_OCTETS_FOR_ADDRESS_CNS);
    ret = getRootBucketRanges(devNum, vrId, PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E,
                              &bitMapSize, &numOfRanges, ranges,
                              bucketDataPtr, printInfo);

    if (ret != GT_OK)
    {
        cpssOsFree(bucketDataPtr);
        return ret;
    }

    /* if an error accur in getRootBucketRanges then bucketDataPtr was already free */
    if(numOfErrors == 0)
        needToFreeBucketDataPtr = GT_TRUE;

    for (i = 0; i < numOfRanges; i++)
    {
        if (ranges[i] < PRV_CPSS_DXCH_LPM_RAM_START_OF_IPV4_MC_ADDRESS_SPACE_CNS)
        {
            if ((i == (numOfRanges - 1)) ||
                (ranges[i+1] >= PRV_CPSS_DXCH_LPM_RAM_START_OF_IPV4_MC_ADDRESS_SPACE_CNS))
            {
                endAddress = PRV_CPSS_DXCH_LPM_RAM_START_OF_IPV4_MC_ADDRESS_SPACE_CNS - 1;
            }
            else
            {
                endAddress = ranges[i+1] - 1;
            }
        }
        else
        {
            break;
        }
        baseAddr[0] = (GT_U8)ranges[i];
        ret = printLpm(devNum, bucketDataPtr[bitMapSize+i], baseAddr, 8, baseAddr[0],
                       endAddress, GT_TRUE, PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E,
                       1, printInfo);
        if ((ret != GT_OK) || numOfErrors)
        {
            cpssOsPrintf("\nInconsistency exists!!!\n");
        }
    }

    if(needToFreeBucketDataPtr == GT_TRUE)
        cpssOsFree(bucketDataPtr);

    return ret;
}

/* This function print recursively the IP UC LPM tree */
GT_STATUS printLpmIpUcTableSip6(GT_U8 devNum,
                                  GT_U32 vrId,
                                  PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT protocol,
                                  prvLpmDumpPrintInfoSip6_STC *printInfo)
{
    GT_STATUS ret=GT_OK;
    GT_U32    j;
    GT_U32    numOfRanges;
    GT_U32    numOfRangesUsed=0;     /* parameter used when going over the lines of regular bit vector */
    GT_U32    rangesArr[256];        /* maximum 256 ranges per octet*/
    GT_U32    rangesTypesArr[256];   /* for each range keep its type (regular,compress,leaf) */
    GT_U32    rangesBitLineArr[256]; /* for each range keep its bit vector line */
    GT_U32    rangesTypeIndexArr[256]; /* what is the number of this range type
                                       from total number of lines with the same type -
                                       where it is located in the GON */
    GT_U32    endAddress;           /* end of range value */
    GT_U32    *bucketDataPtr;       /* data read from HW, max lines read will be 6 lines for regular bit vector  */
    GT_U32    leafData=0;           /* one leaf data read from hw (a leaf read from the line of leafs) */
    GT_U32    nodeSize;             /* size of the node in lines
                                       regular node have 6 lines, all the rest hold 1 line*/

    GT_U32    nodeChildAddressesArr[PRV_CPSS_DXCH_LPM_RAM_FALCON_MAX_SIZE_OF_BUCKET_IN_LPM_LINES_CNS];/* the child_pointer located at the beginning of each line
                                                                                            for regular node we have 6 pointers, for compress node one pointer */
    GT_U32    nodeTotalChildTypesArr[PRV_CPSS_DXCH_LPM_RAM_FALCON_MAX_SIZE_OF_BUCKET_IN_LPM_LINES_CNS][PRV_CPSS_DXCH_LPM_MAX_CHILD_TYPE_CNS];/* 6 elements in a regular node
                                                                                                3 types of child that can be for each range (leaf,regular,compress) */

    GT_U32    nodeTotalLinesPerTypeArr[PRV_CPSS_DXCH_LPM_RAM_FALCON_MAX_SIZE_OF_BUCKET_IN_LPM_LINES_CNS][PRV_CPSS_DXCH_LPM_MAX_CHILD_TYPE_CNS];   /* sum the number of lines in the GON according to the Type */
    GT_U32    nodeTotalBucketPerTypesArr[PRV_CPSS_DXCH_LPM_RAM_FALCON_MAX_SIZE_OF_BUCKET_IN_LPM_LINES_CNS][PRV_CPSS_DXCH_LPM_MAX_CHILD_TYPE_CNS]; /* sum the number of buckets (copmress, regular or leaf) in the GON according to the Type */

    GT_U32    gonNodeAddr,gonNodeSize;
    GT_U32    i;
    GT_U32    bankIndexOfTheGon=0;  /* the index of the bank were the node (one of the GON node) is located */
    GT_U32    bankIndexsOfTheGonsArray[PRV_CPSS_DXCH_LPM_RAM_FALCON_MAX_SIZE_OF_BUCKET_IN_LPM_LINES_CNS];  /* the indexs of the banks were the GON is located */


    /* Allocate memory for bucket data.
       Maximum size is a regular bucket --> 6 lines
       each line is a wide line = 4 words */

    bucketDataPtr = cpssOsMalloc(PRV_CPSS_DXCH_LPM_RAM_FALCON_MAX_SIZE_OF_BUCKET_IN_LPM_LINES_CNS * PRV_CPSS_DXCH_LPM_RAM_FALCON_SIZE_OF_LPM_ENTRY_IN_BYTES_CNS);
    if (bucketDataPtr == NULL)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);

    cpssOsMemSet(bucketDataPtr, 0, PRV_CPSS_DXCH_LPM_RAM_FALCON_MAX_SIZE_OF_BUCKET_IN_LPM_LINES_CNS * PRV_CPSS_DXCH_LPM_RAM_FALCON_SIZE_OF_LPM_ENTRY_IN_BYTES_CNS);

    cpssOsMemSet(baseAddr, 0, sizeof(GT_U8)* PRV_CPSS_DXCH_LPM_MAX_NUM_OF_OCTETS_FOR_ADDRESS_CNS);

    cpssOsMemSet(nodeChildAddressesArr,0,sizeof(nodeChildAddressesArr));
    cpssOsMemSet(nodeTotalChildTypesArr,0,sizeof(nodeTotalChildTypesArr));
    cpssOsMemSet(nodeTotalLinesPerTypeArr,0,sizeof(nodeTotalLinesPerTypeArr));
    cpssOsMemSet(nodeTotalBucketPerTypesArr,0,sizeof(nodeTotalBucketPerTypesArr));
    cpssOsMemSet(rangesArr,0,sizeof(rangesArr));
    cpssOsMemSet(rangesTypesArr,0,sizeof(rangesTypesArr));
    cpssOsMemSet(rangesBitLineArr,0,sizeof(rangesBitLineArr));
    cpssOsMemSet(rangesTypeIndexArr,0,sizeof(rangesTypeIndexArr));
    cpssOsMemSet(bankIndexsOfTheGonsArray,0,sizeof(bankIndexsOfTheGonsArray));

    ret = getRootBucketRangesSip6(devNum,
                                  vrId,
                                  protocol,
                                  &nodeSize,
                                  nodeChildAddressesArr,
                                  nodeTotalChildTypesArr,
                                  nodeTotalLinesPerTypeArr,
                                  nodeTotalBucketPerTypesArr,
                                  &numOfRanges,
                                  rangesArr,
                                  rangesTypesArr,
                                  rangesTypeIndexArr,
                                  rangesBitLineArr,
                                  bucketDataPtr,
                                  0,
                                  printInfo);

    if (ret != GT_OK)
    {
        cpssOsPrintf("\nInconsistency exists in the call to getRootBucketRangesSip6 !!!\n");
        cpssOsFree(bucketDataPtr);
        CPSS_LOG_ERROR_AND_RETURN_MAC(ret, LOG_ERROR_NO_MSG);
    }

    for (j=0;j<nodeSize;j++)
    {
        for (i = numOfRangesUsed; i < numOfRanges; i++)
        {
            if (rangesBitLineArr[i]!=j)
            {
                break; /* break the loop on ranges and move to the next bit vector line
                          this range is related to the next bit vector line */
            }

            /* calculate the address and the size of a single node from the GON
               (6 lines for regular all the rest 1 line) according to the
               parameters we got from getNumOfRangesFromHWSip6 */
            ret = getFromTheGonOneNodeAddrAndSize(nodeChildAddressesArr[j],
                                                  nodeTotalChildTypesArr[j],
                                                  rangesTypesArr[i],
                                                  rangesTypeIndexArr[i],
                                                  &gonNodeAddr,
                                                  &gonNodeSize);
            if (ret != GT_OK)
            {
                cpssOsPrintf("\nInconsistency exists in the call to getFromTheGonOneNodeAddrAndSize !!!\n");
                cpssOsFree(bucketDataPtr);
                CPSS_LOG_ERROR_AND_RETURN_MAC(ret, LOG_ERROR_NO_MSG);
            }

             /* read one range (range number i) */
            ret = prvCpssDxChReadTableMultiEntry(devNum,
                                                 CPSS_DXCH_SIP5_TABLE_LPM_MEM_E,
                                                 gonNodeAddr,
                                                 gonNodeSize,
                                                 bucketDataPtr);
            if (ret != GT_OK)
            {
                cpssOsPrintf("\nInconsistency exists - printLpmIpv4UcTableSip6 !!!\n");
                cpssOsFree(bucketDataPtr);
                CPSS_LOG_ERROR_AND_RETURN_MAC(ret, LOG_ERROR_NO_MSG);
            }

            /* UC range only */
            switch(protocol)
            {
                case PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E:

                if ((rangesArr[i] < PRV_CPSS_DXCH_LPM_RAM_START_OF_IPV4_MC_ADDRESS_SPACE_CNS)||
                    (rangesArr[i] > PRV_CPSS_DXCH_LPM_RAM_END_OF_IPV4_MC_ADDRESS_SPACE_CNS))

                {
                    if (i == (numOfRanges - 1)) /* if this is the last range then endAddress=255 */
                    {
                        endAddress = PRV_CPSS_DXCH_LPM_RAM_END_OF_IPV4_RESERVED_SPACE_ADDRESS_SPACE_CNS;
                    }
                    else
                    {
                        endAddress = rangesArr[i+1] - 1;
                    }
                    numOfRangesUsed++;
                }
                else
                {
                    /* multicast range should not be printed */
                    numOfRangesUsed++;
                    continue;

                }
               break;
            case PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E:

                if ((rangesArr[i] < PRV_CPSS_DXCH_LPM_RAM_START_OF_IPV6_MC_ADDRESS_SPACE_CNS)||
                   (rangesArr[i] > PRV_CPSS_DXCH_LPM_RAM_END_OF_IPV6_MC_ADDRESS_SPACE_CNS))

                {
                    if (i == (numOfRanges - 1)) /* if this is the last range then endAddress=255 */
                    {
                        endAddress = PRV_CPSS_DXCH_LPM_RAM_START_OF_IPV6_MC_ADDRESS_SPACE_CNS;
                    }
                    else
                    {
                        endAddress = rangesArr[i+1] - 1;
                    }
                    numOfRangesUsed++;
                }
                else
                {
                    /* multicast range should not be printed */
                    numOfRangesUsed++;
                    continue;

                }
                break;
            case PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E:

                if (i == (numOfRanges - 1)) /* if this is the last range then endAddress=255 */
                {
                    endAddress = PRV_CPSS_DXCH_LPM_RAM_END_OF_IPV4_RESERVED_SPACE_ADDRESS_SPACE_CNS;
                }
                else
                {
                    endAddress = rangesArr[i+1] - 1;
                }
                numOfRangesUsed++;
                break;
            default:
                cpssOsPrintf("\nvalidity not supported !!!\n");
                cpssOsFree(bucketDataPtr);
                CPSS_LOG_ERROR_AND_RETURN_MAC(ret, LOG_ERROR_NO_MSG);
                break;
            }


            /* until here we were dealing with the root of the LPM tree
               Now we are starting to go over all the ranges recursively */

            baseAddr[0] = (GT_U8)rangesArr[i];

            /* the hwAddr is a relative address to the beginning of the LPM */
            bankIndexOfTheGon = gonNodeAddr/PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.totalNumOfLinesInBlockIncludingGap;
            if ((bankIndexsOfTheGonsArray[j]!=0)&&(bankIndexsOfTheGonsArray[j]!=bankIndexOfTheGon))
            {
                cpssOsPrintf("\nInconsistency exists a single GON can not be located in 2 different banks - printLpmIpv4UcTableSip6 !!!\n");
                cpssOsFree(bucketDataPtr);
                CPSS_LOG_ERROR_AND_RETURN_MAC(ret, LOG_ERROR_NO_MSG);
            }
            else
            {
                bankIndexsOfTheGonsArray[j] = bankIndexOfTheGon;
            }

            if (rangesTypesArr[i]==PRV_CPSS_DXCH_LPM_CHILD_LEAF_TYPE_E)/* leaf */
            {
                /* need to find the correct leaf out of all the leafs in the line - in HW the first leaf are kept in last bits:
                   0-22: leaf 4
                   23-45: leaf 3
                   46-68: leaf 2
                   69-91: leaf 1
                   92-114: leaf 0 */
                U32_GET_FIELD_IN_ENTRY_MAC(bucketDataPtr,
                                           (MAX_NUMBER_OF_LEAVES_IN_LPM_LINE_CNS-(rangesTypeIndexArr[i]%MAX_NUMBER_OF_LEAVES_IN_LPM_LINE_CNS)-1)*23,
                                           23,leafData);

                ret = printLpmSip6(devNum,
                                   &leafData,               /* we print the Leaf - we arrived to a NH/ECMP/QOS */
                                   baseAddr,                /* array that keeps the IP address according to its octets */
                                   8,                       /* basePrefix */
                                   baseAddr[0], endAddress, /* start and end of the range */
                                   rangesTypesArr[i],       /* type of the range . in the case a leaf */
                                   bankIndexOfTheGon,      /* the index of the bank were the GON is located */
                                   gonNodeAddr,             /* the address from were we read the HW data */
                                   GT_TRUE,                 /* isUc */
                                   protocol,
                                   0,                       /* we are still in depth 0 - just want to print the leaf */
                                   printInfo);

                if ((ret != GT_OK) || numOfErrors)
                {
                    cpssOsPrintf("\nInconsistency exists in the call to printLpmSip6 !!!\n");
                    cpssOsFree(bucketDataPtr);
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
                }

                printInfo->nodeCounterPerTypePerOctetSip6[0][(PRV_CPSS_DXCH_LPM_CHILD_LEAF_TYPE_E-1)]++;

            }
            else/* regular or compress */
            {
                ret = printLpmSip6(devNum,
                                   bucketDataPtr,           /* the bit vector HW data, 6 lines read in case of regular,
                                                               one line in case of compress */
                                   baseAddr,                /* array that keeps the IP address according to its octets */
                                   8,                       /* basePrefix */
                                   baseAddr[0],endAddress,  /* start and end of the range */
                                   rangesTypesArr[i],       /* type of the range . in the case a leaf */
                                   bankIndexOfTheGon,       /* the index of the bank were the GON is located */
                                   gonNodeAddr,             /* the address from were we read the HW data */
                                   GT_TRUE,                 /* isUc */
                                   protocol,
                                   1,                       /* depth - print the next range for octet 1 */
                                   printInfo);

                if ((ret != GT_OK) || numOfErrors)
                {
                    cpssOsPrintf("\nInconsistency exists in the call to printLpmSip6 !!!\n");
                    cpssOsFree(bucketDataPtr);
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
                }
            }
        }

       /*  after printing the all ranges we need to update the number of lines occupied by the leafs
            those are not updated in the print recursive calls.
            all calculation of how many lines is occupied by the leafs is
            calculated in getRootBucketRangesSip6.*/

        /* update the print info for the first octet */
        printInfo->lpmLinesUsedSip6[bankIndexsOfTheGonsArray[j]] += (nodeTotalLinesPerTypeArr[j][(PRV_CPSS_DXCH_LPM_CHILD_LEAF_TYPE_E-1)]);
        printInfo->nodeCounterPerTypeSip6[1] += nodeTotalChildTypesArr[j][0]; /* add the number of leafs calculated for this GON */
    }

    cpssOsFree(bucketDataPtr);
    return ret;
}

GT_STATUS printLpmIpv4McTable(GT_U8 devNum, GT_U32 vrId, prvLpmDumpPrintInfo_STC *printInfo)
{
    GT_U32    numOfRanges;
    GT_U32    ranges[256];
    GT_U32    endAddress;
    GT_U32    *bucketDataPtr;
    GT_U32    bitMapSize;
    GT_U32    i;
    GT_STATUS ret;
    GT_BOOL   needToFreeBucketDataPtr=GT_FALSE;

     /* Allocate memory for bucket data. Maximum size is regular bucket that
       uses 256 lpm lines for ranges + 11 lpm lines for bitmap */
    bucketDataPtr = cpssOsMalloc(PRV_CPSS_DXCH_LPM_RAM_MAX_SIZE_OF_BUCKET_IN_LPM_LINES_CNS * sizeof(GT_U32));
    if (bucketDataPtr == NULL)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);

    cpssOsMemSet(bucketDataPtr, 0, PRV_CPSS_DXCH_LPM_RAM_MAX_SIZE_OF_BUCKET_IN_LPM_LINES_CNS * sizeof(GT_U32));
    cpssOsMemSet(baseAddr, 0, sizeof(GT_U8)* PRV_CPSS_DXCH_LPM_MAX_NUM_OF_OCTETS_FOR_ADDRESS_CNS);
    ret = getRootBucketRanges(devNum, vrId, PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E,
                              &bitMapSize, &numOfRanges, ranges,
                              bucketDataPtr, printInfo);
    if (ret != GT_OK)
    {
        cpssOsFree(bucketDataPtr);
        return ret;
    }

    /* if an error accur in getRootBucketRanges then bucketDataPtr was already free */
    if(numOfErrors == 0)
        needToFreeBucketDataPtr = GT_TRUE;

    for (i = 0; i < numOfRanges; i++)
    {
        if (ranges[i] >= PRV_CPSS_DXCH_LPM_RAM_START_OF_IPV4_RESERVED_SPACE_ADDRESS_SPACE_CNS)
        {
            break;
        }
        else if (ranges[i] < PRV_CPSS_DXCH_LPM_RAM_START_OF_IPV4_MC_ADDRESS_SPACE_CNS)
        {
            continue;
        }
        else if (i != (numOfRanges - 1))
        {
            endAddress = ranges[i+1] - 1;
        }
        else
        {
            endAddress = PRV_CPSS_DXCH_LPM_RAM_END_OF_IPV4_MC_ADDRESS_SPACE_CNS;
        }
        baseAddr[0] = (GT_U8)ranges[i];
        InSrcTree = GT_FALSE;
        ret = printLpm(devNum, bucketDataPtr[bitMapSize+i], baseAddr, 8, baseAddr[0],
                       endAddress, GT_FALSE, PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E,
                       1, printInfo);
        if ((ret != GT_OK) || numOfErrors)
        {
            cpssOsPrintf("\nInconsistency exists!!!\n");
        }
    }
    if(needToFreeBucketDataPtr == GT_TRUE)
        cpssOsFree(bucketDataPtr);

    return ret;
}

GT_STATUS printLpmIpv6UcTable(GT_U8 devNum, GT_U32 vrId, prvLpmDumpPrintInfo_STC *printInfo)
{
    GT_U32    numOfRanges;
    GT_U32    ranges[256];
    GT_U32    endAddress;
    GT_U32    *bucketDataPtr;
    GT_U32    bitMapSize;
    GT_U32    i;
    GT_STATUS ret;
    GT_BOOL   needToFreeBucketDataPtr=GT_FALSE;

    /* Allocate memory for bucket data. Maximum size is regular bucket that
       uses 256 lpm lines for ranges + 11 lpm lines for bitmap */
    bucketDataPtr = cpssOsMalloc(PRV_CPSS_DXCH_LPM_RAM_MAX_SIZE_OF_BUCKET_IN_LPM_LINES_CNS * sizeof(GT_U32));
    if (bucketDataPtr == NULL)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);

    cpssOsMemSet(bucketDataPtr, 0, PRV_CPSS_DXCH_LPM_RAM_MAX_SIZE_OF_BUCKET_IN_LPM_LINES_CNS * sizeof(GT_U32));
    cpssOsMemSet(baseAddr, 0, sizeof(GT_U8)* PRV_CPSS_DXCH_LPM_MAX_NUM_OF_OCTETS_FOR_ADDRESS_CNS);
    ret = getRootBucketRanges(devNum, vrId, PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E,
                              &bitMapSize, &numOfRanges, ranges,
                              bucketDataPtr, printInfo);
    if (ret != GT_OK)
    {
        cpssOsFree(bucketDataPtr);
        return ret;
    }
    /* if an error accur in getRootBucketRanges then bucketDataPtr was already free */
    if(numOfErrors == 0)
        needToFreeBucketDataPtr = GT_TRUE;

    for (i = 0; i < numOfRanges; i++)
    {
        if (ranges[i] < PRV_CPSS_DXCH_LPM_RAM_START_OF_IPV6_MC_ADDRESS_SPACE_CNS)
        {
            if ((i == (numOfRanges - 1)) ||
                (ranges[i+1] >= PRV_CPSS_DXCH_LPM_RAM_START_OF_IPV6_MC_ADDRESS_SPACE_CNS))
            {
                endAddress = PRV_CPSS_DXCH_LPM_RAM_START_OF_IPV6_MC_ADDRESS_SPACE_CNS - 1;
            }
            else
            {
                endAddress = ranges[i+1] - 1;
            }
        }
        else
        {
            break;
        }
        baseAddr[0] = (GT_U8)ranges[i];
        ret = printLpm(devNum, bucketDataPtr[bitMapSize+i], baseAddr, 8, baseAddr[0],
                       endAddress, GT_TRUE, PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E,
                       1, printInfo);
        if ((ret != GT_OK) || numOfErrors)
        {
            cpssOsPrintf("\nInconsistency exists!!!\n");
        }
    }
    if(needToFreeBucketDataPtr == GT_TRUE)
        cpssOsFree(bucketDataPtr);

    return ret;
}

GT_STATUS printLpmIpv6McTable(GT_U8 devNum, GT_U32 vrId, prvLpmDumpPrintInfo_STC *printInfo)
{
    GT_U32    numOfRanges = 0;
    GT_U32    ranges[256] = {0};
    GT_U32    endAddress = 0;
    GT_U32    *bucketDataPtr = NULL;
    GT_U32    bitMapSize = 0;
    GT_U32    i = 0;
    GT_STATUS ret = GT_FAIL;
    GT_BOOL   needToFreeBucketDataPtr=GT_FALSE;

    /* Allocate memory for bucket data. Maximum size is regular bucket that
       uses 256 lpm lines for ranges + 11 lpm lines for bitmap */
    bucketDataPtr = cpssOsMalloc(PRV_CPSS_DXCH_LPM_RAM_MAX_SIZE_OF_BUCKET_IN_LPM_LINES_CNS * sizeof(GT_U32));
    if (bucketDataPtr == NULL)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);

    cpssOsMemSet(bucketDataPtr, 0, PRV_CPSS_DXCH_LPM_RAM_MAX_SIZE_OF_BUCKET_IN_LPM_LINES_CNS * sizeof(GT_U32));

    cpssOsMemSet(baseAddr, 0, sizeof(GT_U8)* PRV_CPSS_DXCH_LPM_MAX_NUM_OF_OCTETS_FOR_ADDRESS_CNS);

    ret = getRootBucketRanges(devNum, vrId, PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E,
                              &bitMapSize, &numOfRanges, ranges,
                              bucketDataPtr, printInfo);
    if (ret != GT_OK)
    {
        cpssOsFree(bucketDataPtr);
        return ret;
    }

    /* if an error accur in getRootBucketRanges then bucketDataPtr was already free */
    if(numOfErrors == 0)
        needToFreeBucketDataPtr = GT_TRUE;

    for (i = 0; i < numOfRanges; i++)
    {
        if (ranges[i] < PRV_CPSS_DXCH_LPM_RAM_START_OF_IPV6_MC_ADDRESS_SPACE_CNS)
        {
            continue;
        }
        else if (i != (numOfRanges - 1))
        {
            endAddress = ranges[i+1] - 1;
        }
        else
        {
            endAddress = PRV_CPSS_DXCH_LPM_RAM_END_OF_IPV6_MC_ADDRESS_SPACE_CNS;
        }
        baseAddr[0] = (GT_U8)ranges[i];
        InSrcTree = GT_FALSE;
        ret = printLpm(devNum, bucketDataPtr[bitMapSize+i], baseAddr, 8, baseAddr[0],
                       endAddress, GT_FALSE, PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E,
                       1, printInfo);
        if ((ret != GT_OK) || numOfErrors)
        {
            cpssOsPrintf("\nInconsistency exists!!!\n");
        }
    }
    if(needToFreeBucketDataPtr == GT_TRUE)
        cpssOsFree(bucketDataPtr);
    return ret;
}

GT_STATUS printLpmFcoeTable(GT_U8 devNum, GT_U32 vrId, prvLpmDumpPrintInfo_STC *printInfo)
{
    GT_U32    numOfRanges;
    GT_U32    ranges[256];
    GT_U32    endAddress;
    GT_U32    *bucketDataPtr;
    GT_U32    bitMapSize;
    GT_U32    i;
    GT_STATUS ret;
    GT_BOOL   needToFreeBucketDataPtr=GT_FALSE;

    /* Allocate memory for bucket data. Maximum size is regular bucket that
       uses 256 lpm lines for ranges + 11 lpm lines for bitmap */
    bucketDataPtr = cpssOsMalloc(PRV_CPSS_DXCH_LPM_RAM_MAX_SIZE_OF_BUCKET_IN_LPM_LINES_CNS * sizeof(GT_U32));
    if (bucketDataPtr == NULL)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);

    cpssOsMemSet(bucketDataPtr, 0, PRV_CPSS_DXCH_LPM_RAM_MAX_SIZE_OF_BUCKET_IN_LPM_LINES_CNS * sizeof(GT_U32));
    cpssOsMemSet(baseAddr, 0, sizeof(GT_U8)* PRV_CPSS_DXCH_LPM_MAX_NUM_OF_OCTETS_FOR_ADDRESS_CNS);
    ret = getRootBucketRanges(devNum, vrId, PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E,
                              &bitMapSize, &numOfRanges, ranges,
                              bucketDataPtr, printInfo);
    if (ret != GT_OK)
    {
        cpssOsFree(bucketDataPtr);
        return ret;
    }

    /* if an error accur in getRootBucketRanges then bucketDataPtr was already free */
    if(numOfErrors == 0)
        needToFreeBucketDataPtr = GT_TRUE;

    for (i = 0; i < numOfRanges; i++)
    {
        baseAddr[0] = (GT_U8)ranges[i];
        if (i != (numOfRanges - 1))
        {
            endAddress = ranges[i+1] - 1;
        }
        else
        {
            endAddress = 255;
        }
        ret = printLpm(devNum, bucketDataPtr[bitMapSize+i], baseAddr, 8, baseAddr[0],
                       endAddress, GT_TRUE, PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E,
                       1, printInfo);
        if ((ret != GT_OK) || numOfErrors)
        {
            cpssOsPrintf("\nInconsistency exists!!!\n");
        }
    }
    if(needToFreeBucketDataPtr == GT_TRUE)
        cpssOsFree(bucketDataPtr);
    return ret;
}

/**
* @internal prvCpssDxChLpmRamDbgHwDfsScanSip6 function
* @endinternal
 *
* @brief   This function is intended to do hardware LPM validation and dump
 *
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2
*                                  Bobcat2; Caelum; Aldrin;
*                                  AC3X; Bobcat3.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum or vrId.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @param[in] devNum                   - physical device number.
* @param[in] vrId                     - virtual router ID
* @param[in] protocol                 - the
* @param[in] prefixType               - the prefix type (Unicast or Multicast)
* @param[in] print                    -  ranges
*                                      GT_TRUE --- print ranges
*                                      GT_FALSE --- don't print ranges
*/
static GT_STATUS prvCpssDxChLpmRamDbgHwDfsScanSip6
(
    IN  GT_U8                                   devNum,
    IN  GT_U32                                  vrId,
    IN  PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT    protocol,
    IN  CPSS_UNICAST_MULTICAST_ENT              prefixType,
    IN  GT_BOOL                                 print
)
{
    GT_STATUS rc = GT_OK;                   /* return code */
    GT_U32    i;
    GT_U32    maxDepth = 0;

    prvLpmDumpPrintInfoSip6_STC printInfo; /* strcut holds all the printing info */

    cpssOsMemSet(&printInfo, 0, sizeof(prvLpmDumpPrintInfoSip6_STC));

    printInfo.printRangesSip6 = print;

    /* check if device is active */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_CH1_E| CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
          CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E |
          CPSS_BOBCAT2_E  | CPSS_CAELUM_E   | CPSS_ALDRIN_E   | CPSS_BOBCAT3_E  | CPSS_ALDRIN2_E);

    numOfErrors = 0;
    switch (protocol)
    {
        case PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E:
            if (prefixType == CPSS_UNICAST_E)
            {
                rc = printLpmIpUcTableSip6(devNum, vrId, PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E, &printInfo);
            }
            else /* CPSS_MULTICAST_E */
            {
                /*rc = printLpmIpv4McTableSip6(devNum, vrId, &printInfo);*/
            }
            break;

        case PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E:
            if (prefixType == CPSS_UNICAST_E)
            {
                rc = printLpmIpUcTableSip6(devNum, vrId, PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E, &printInfo);
            }
            else /* CPSS_MULTICAST_E */
            {
                /*rc = printLpmIpv6McTableSip6(devNum, vrId, &printInfo);*/
            }
            break;

        case PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E:
            rc = printLpmIpUcTableSip6(devNum, vrId, PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E, &printInfo);
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (printInfo.printRangesSip6)
    {
        cpssOsPrintf("LPM lines used by the virtual router buckets:\n");
        for (i = 0 ; i < PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.numOfBlocks; i++)
        {
            cpssOsPrintf("    Block[%d]: %d\n", i, printInfo.lpmLinesUsedSip6[i]);
        }

        cpssOsPrintf("Buckets per type: (UC/MC ipv4/ipv6/Fcoe)\n");
        cpssOsPrintf("   Regular node = %d\n",     printInfo.nodeCounterPerTypeSip6[2]);
        cpssOsPrintf("   Compressed node  = %d\n", printInfo.nodeCounterPerTypeSip6[3]);
        cpssOsPrintf("   Leafs = %d\n",            printInfo.nodeCounterPerTypeSip6[1]);

        cpssOsPrintf("\nNumber of non default next hop pointers: %d\n", printInfo.numOfNonDefaultNextHopPointersSip6);

        cpssOsPrintf("\n<octet><# Leafs><# regular><# comp><#embedded_1><#embedded_2><#embedded_3> \n");
        cpssOsPrintf("----------------------------------------------------------------------------------------------------------------------------------\n");

        /* maxDepth must be declared as local var (and not macro in loop), prevent gcc -O2 infinite loop bug */
        maxDepth = 2 * PRV_DXCH_LPM_RAM_DBG_MAX_DEPTH_CHECK_MAC(protocol);
        for (i = 0 ; i < maxDepth; i++)
        {
            cpssOsPrintf("    %d\t%d\t%d\t%d\t%d\t\t%d\t\t %d \n",i+1,
                            printInfo.nodeCounterPerTypePerOctetSip6[i][0],
                            printInfo.nodeCounterPerTypePerOctetSip6[i][1],
                            printInfo.nodeCounterPerTypePerOctetSip6[i][2],
                            printInfo.nodeCounterPerTypePerOctetSip6[i][3],
                            printInfo.nodeCounterPerTypePerOctetSip6[i][4],
                            printInfo.nodeCounterPerTypePerOctetSip6[i][5]);
        }
    }
    if (numOfErrors > 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }
    return rc;
}


/**
* @internal prvCpssDxChLpmRamDbgHwDfsScan function
* @endinternal
*
* @brief   This function is intended to do hardware LPM validation and dump
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
*
* @param[in] devNum                   - physical device number.
* @param[in] vrId                     - virtual router ID
* @param[in] protocol                 - the protocol
* @param[in] prefixType               - the prefix type (Unicast or Multicast)
* @param[in] print                    -  ranges
*                                      GT_TRUE --- print ranges
*                                      GT_FALSE --- don't print ranges
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum or vrId.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS prvCpssDxChLpmRamDbgHwDfsScan
(
    IN  GT_U8                                   devNum,
    IN  GT_U32                                  vrId,
    IN  PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT    protocol,
    IN  CPSS_UNICAST_MULTICAST_ENT              prefixType,
    IN  GT_BOOL                                 print
)
{
    GT_STATUS rc = GT_OK;                   /* return code */
    GT_U32    i;
    GT_U32    maxDepth = 0;
    prvLpmDumpPrintInfo_STC printInfo; /* strcut holds all the printing info */

    /* if this is a SIP6 architecture the call specific sip6 function */
    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        rc = prvCpssDxChLpmRamDbgHwDfsScanSip6(devNum, vrId, protocol,
                                               prefixType,print);
        return rc;
    }

    cpssOsMemSet(&printInfo, 0, sizeof(prvLpmDumpPrintInfo_STC));

    printInfo.printRanges = print;

    /* check if device is active */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    numOfErrors = 0;
    switch (protocol)
    {
        case PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E:
            if (prefixType == CPSS_UNICAST_E)
            {
                rc = printLpmIpv4UcTable(devNum, vrId, &printInfo);
            }
            else /* CPSS_MULTICAST_E */
            {
                rc = printLpmIpv4McTable(devNum, vrId, &printInfo);
            }
            break;

        case PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E:
            if (prefixType == CPSS_UNICAST_E)
            {
                rc = printLpmIpv6UcTable(devNum, vrId, &printInfo);
            }
            else /* CPSS_MULTICAST_E */
            {
                rc = printLpmIpv6McTable(devNum, vrId, &printInfo);
            }
            break;

        case PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E:
            rc = printLpmFcoeTable(devNum, vrId, &printInfo);
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if (printInfo.printRanges)
    {
        cpssOsPrintf("LPM lines used by the virtual router buckets:\n");
        for (i = 0 ; i < PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.numOfBlocks; i++)
        {
            cpssOsPrintf("    Block[%d]: %d\n", i, printInfo.lpmLinesUsed[i]);
        }

        cpssOsPrintf("Buckets per type:\n");
        cpssOsPrintf("   Regular node = %d\n", printInfo.bucketPerType[0]);
        cpssOsPrintf("   Compressed 1 node = %d\n", printInfo.bucketPerType[1]);
        cpssOsPrintf("   Compressed 2 node = %d\n", printInfo.bucketPerType[2]);

        cpssOsPrintf("Average bucket size per type:\n");
        cpssOsPrintf("   Regular node = %4.2f\n", (printInfo.bucketPerType[0] == 0)?0:(float)printInfo.bucketSizePerType[0]/printInfo.bucketPerType[0]);
        cpssOsPrintf("   Compressed 1 node = %4.2f\n", (printInfo.bucketPerType[1] == 0)?0:(float)printInfo.bucketSizePerType[1]/printInfo.bucketPerType[1]);
        cpssOsPrintf("   Compressed 2 node = %4.2f\n", (printInfo.bucketPerType[2] == 0)?0:(float)printInfo.bucketSizePerType[2]/printInfo.bucketPerType[2]);

        cpssOsPrintf("\nNumber of non default next hop pointers: %d\n", printInfo.numOfNonDefaultNextHopPointers);

        cpssOsPrintf("\n<octet>  <lpm lines>  <# buckets>  <# comp1>  <# comp2>  <# regular>\n");
        cpssOsPrintf("----------------------------------------------------------------------------------------------------------------\n");

        /* maxDepth must be declared as local var (and not macro in loop), prevent gcc -O2 infinite loop bug */
        maxDepth = 2 * PRV_DXCH_LPM_RAM_DBG_MAX_DEPTH_CHECK_MAC(protocol);
        for (i = 0 ; i < maxDepth; i++)
        {
            cpssOsPrintf("    %d\t %d\t %d\t %d\t %d\t %d\n",i+1,
                            printInfo.lpmLinesPerOctet[i],
                            printInfo.bucketPerOctet[i],
                            printInfo.bucketTypePerOctet[i][1],
                            printInfo.bucketTypePerOctet[i][2],
                            printInfo.bucketTypePerOctet[i][0]);
        }
    }
    if (numOfErrors > 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }
    return rc;
}

/**
* @internal prvCpssDxChLpmRamDbgDump function
* @endinternal
*
* @brief   This function is intended to do hardware LPM dump
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
*
* @param[in] devNum                   - physical device number.
* @param[in] vrId                     - virtual router ID
* @param[in] protocol                 - the protocol
* @param[in] prefixType               - the prefix type (Unicast or Multicast)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum or vrId.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmRamDbgDump
(
    IN  GT_U8                                   devNum,
    IN  GT_U32                                  vrId,
    IN  PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT    protocol,
    IN  CPSS_UNICAST_MULTICAST_ENT              prefixType
)
{
    return prvCpssDxChLpmRamDbgHwDfsScan(devNum, vrId, protocol, prefixType, GT_TRUE);
}

/**
* @internal prvCpssDxChLpmRamDbgHwValidation function
* @endinternal
*
* @brief   This function is intended to do hardware LPM validation
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
*
* @param[in] devNum                   - physical device number.
* @param[in] vrId                     - virtual router ID
* @param[in] protocol                 - the protocol
* @param[in] prefixType               - the prefix type (Unicast or Multicast)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum or vrId.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note All the checks
*       For every LPM LINE
*       Bucket type (bits 3-4) - all values valid
*       If bucket type regular, bits 5-28 are next bucket pointer (index of LPM line, offset from LPM mem start)
*       Can check validity of the regular bitmap of the next bucket (see later)
*       If bucket type compressed-1, bits 5-28 are next bucket pointer
*       Can check validity of compressed-1 bitmap of the next bucket (see later)
*       If bucket type is compressed-2, bits 13-31 are next bucket point
*       Can check validity of compressed-2 bitmap (and pass its 5th range as parameter)
*       Regular bitmap
*       --------------
*       For first word, sum must be 0
*       For each word in the bitmap other than first
*       Sum = previous_sum + number of bits in previous word
*       On last word, last 8 bits must be 0 (those do not count as 1124 = 264 = 256 + 8)
*       Regular bucket is for 11-255 ranges so there must be at least 11 bits != 0 (exception here for first MC source bucket)
*       Bit 0 in word 0 must be set because first range always starts in 0
*       Compressed 1 bitmap (ranges are 0 based)
*       ----------------------------------------
*       Bitmap has 4 bytes [range_4_start ; range_3_start ; range_2_start ; range_1_start]
*       Range 0 start is always 0 (so no need to add to bitmap).
*       While range x start != 0
*       Current range start > previous range start
*       After first 0, all must be 0
*       Compressed 1 must contain 2-5 ranges so range_1_start (second range) must be != 0
*       Compressed 2 bitmap (ranges are 0 based)
*       ----------------------------------------
*       Word 1: [range_4_start ; range_3_start ; range_2_start ; range_1_start]
*       Word 2: [range_9_start ; range_8_start ; range_7_start ; range_6_start]
*       range_5_start is carried from the pointer to this bucket
*       for each word:
*       each range start > previous range start
*       for word 1:
*       all ranges start must be != 0 (if one is 0, then it should be compressed 1)
*       for word 2:
*       all must be bigger than range_5_start or 0!!
*       Range_5_start must be != 0 (if 0, then should use compressed 1)
*       Tree based checks
*       -----------------
*       IPv4 UC
*       Max tree depth is 4 so from depth 4 all LPM lines must be NH or ECMP/QoS bucket type
*       Illegal to point to a source tree.
*       IPv6 UC
*       Same but depth is 16
*       IPv4 MC SRC
*       Max MC group depth is 4 and each group points to root of sources tree (with max depth of 4). So total max depth is 8.
*       Illegal to point to another source tree.
*       If you reach depth 8, then all LPM lines must be pointing to NH or ECMP/QoS.
*       IPv6 MC
*       Same but 8 instead of 4 and 16 instead of 8
*
*/
GT_STATUS prvCpssDxChLpmRamDbgHwValidation
(
    IN  GT_U8                                   devNum,
    IN  GT_U32                                  vrId,
    IN  PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT    protocol,
    IN  CPSS_UNICAST_MULTICAST_ENT              prefixType
)
{
    return prvCpssDxChLpmRamDbgHwDfsScan(devNum, vrId, protocol, prefixType, GT_FALSE);
}

/**
* @internal prvCpssDxChLpmRamDbgDbHwMemPrint function
* @endinternal
*
* @brief   Function Relevant mode : High Level API modes
*         This function print LPM debug information
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
*
* @param[in] lpmDbPtr                 - The LPM DB.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - otherwise
*
* @note none.
*
*/
GT_STATUS prvCpssDxChLpmRamDbgDbHwMemPrint
(
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC      *lpmDbPtr
)
{
    GT_STATUS retVal;
    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC  *shadowPtr;
    GT_UINTPTR  partitionId;
    GT_U32      i;

    shadowPtr = &lpmDbPtr->shadowArray[0];

    for ( i = 0 ; i < shadowPtr->numOfLpmMemories ; i++ )
    {
        partitionId = shadowPtr->lpmRamStructsMemPoolPtr[i];
        if( partitionId == 0)
            continue;

        retVal = prvCpssDmmPartitionPrint(partitionId);
        if(retVal != GT_OK)
        {
            return (retVal);
        }
    }

    return (GT_OK);
}

/**
* @internal prvCpssDxChLpmRamDbgRouteEntryShadowValidityCheck function
* @endinternal
*
* @brief   Shadow validation of a route entry
*
* @param[in] shadowPtr                - pointer to shadow structure
* @param[in] routeEntryPtr            - pointer to route entry
* @param[in] returnOnFailure          - GT_TRUE: the validation check stops on
*                                      first failure
*                                      GT_FALSE: continue with the test on failure
*
* @retval GT_FAIL                  - on failure
* @retval GT_OK                    - on success
*/
static GT_STATUS prvCpssDxChLpmRamDbgRouteEntryShadowValidityCheck
(
    IN  PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC            *shadowPtr,
    IN  PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC   *routeEntryPtr,
    IN  GT_BOOL                                     returnOnFailure
)
{
    GT_U32    shareDevListLen, devIdx, maxMemSize;
    GT_U8     *shareDevsList;  /* List of devices sharing this LPM structure */
    GT_STATUS retVal = GT_OK;

    shareDevsList   = shadowPtr->workDevListPtr->shareDevs;
    shareDevListLen = shadowPtr->workDevListPtr->shareDevNum;

    if (routeEntryPtr == NULL)
    {
        LPM_SHADOW_VALIDATION_ERROR_MAC(returnOnFailure, retVal, "Next hop",
                                        0,
                                        "routeEntryPtr is null");
    }
    else
    {
        if ((routeEntryPtr->routeEntryMethod != PRV_CPSS_DXCH_LPM_ENTRY_TYPE_ECMP_E) &&
            (routeEntryPtr->routeEntryMethod != PRV_CPSS_DXCH_LPM_ENTRY_TYPE_QOS_E) &&
            (routeEntryPtr->routeEntryMethod != PRV_CPSS_DXCH_LPM_ENTRY_TYPE_REGULAR_E))
        {
            LPM_SHADOW_VALIDATION_ERROR_MAC(returnOnFailure, retVal, "Next hop",
                                            routeEntryPtr,
                                            "illegal route entry method");
        }

         /* For each device check validity of route entries */
        for (devIdx = 0; devIdx < shareDevListLen; devIdx++)
        {
            /*Lock the access to per device data base in order to avoid corruption*/
            CPSS_API_LOCK_MAC(devIdx,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

            if (routeEntryPtr->routeEntryMethod == PRV_CPSS_DXCH_LPM_ENTRY_TYPE_REGULAR_E)
            {
                maxMemSize = PRV_CPSS_DXCH_PP_MAC(shareDevsList[devIdx])->fineTuning.tableSize.routerNextHop;
            }
            else
            {
                maxMemSize = PRV_CPSS_DXCH_PP_MAC(shareDevsList[devIdx])->fineTuning.tableSize.ecmpQos;
            }

            /*Unlock the access to per device data base*/
            CPSS_API_UNLOCK_MAC(devIdx,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

            /* Check that the base address is within the table */
            if (routeEntryPtr->routeEntryBaseMemAddr >= maxMemSize)
            {
                LPM_SHADOW_VALIDATION_ERROR_MAC(returnOnFailure, retVal, "Next hop",
                                                routeEntryPtr,
                                                "illegal route entry address");
            }

            if (routeEntryPtr->routeEntryMethod != PRV_CPSS_DXCH_LPM_ENTRY_TYPE_REGULAR_E)
            {
                /*Lock the access to per device data base in order to avoid corruption*/
                CPSS_API_LOCK_MAC(devIdx,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

                /* ECMP/QoS */
                maxMemSize = PRV_CPSS_DXCH_PP_MAC(shareDevsList[devIdx])->fineTuning.tableSize.routerNextHop;

                /*Unlock the access to per device data base*/
                CPSS_API_UNLOCK_MAC(devIdx,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

                if (routeEntryPtr->routeEntryBaseMemAddr + routeEntryPtr->blockSize > maxMemSize)
                {
                    LPM_SHADOW_VALIDATION_ERROR_MAC(returnOnFailure, retVal, "ECMP",
                                                    routeEntryPtr,
                                                    "illegal ECMP entry, out of bound of nexthop table");
                }
            }
        }

        if (routeEntryPtr->ipv6McGroupScopeLevel > CPSS_IPV6_PREFIX_SCOPE_GLOBAL_E)
        {
            LPM_SHADOW_VALIDATION_ERROR_MAC(returnOnFailure, retVal, "Next hop",
                                            routeEntryPtr,
                                            "illegal IPv6 MC Group Scope Level");
        }
    }
    return retVal;
}
/**
* @internal
*           prvCpssDxChLpmRamDbgRouteEntryShadowValidityCheckSip6
*           function
* @endinternal
*
* @brief   Shadow validation of a route entry
*
* @param[in] shadowPtr                - pointer to shadow structure
* @param[in] routeEntryPtr            - pointer to route entry
* @param[in] returnOnFailure          - GT_TRUE: the validation check stops on
*                                      first failure
*                                      GT_FALSE: continue with the test on failure
*
* @retval GT_FAIL                  - on failure
* @retval GT_OK                    - on success
*/
static GT_STATUS prvCpssDxChLpmRamDbgRouteEntryShadowValidityCheckSip6
(
    IN  PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC            *shadowPtr,
    IN  PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC   *routeEntryPtr,
    IN  GT_BOOL                                     returnOnFailure
)
{
    GT_U32    shareDevListLen, devIdx, maxMemSize;
    GT_U8     *shareDevsList;  /* List of devices sharing this LPM structure */
    GT_STATUS retVal = GT_OK;

    shareDevsList   = shadowPtr->workDevListPtr->shareDevs;
    shareDevListLen = shadowPtr->workDevListPtr->shareDevNum;

    if (routeEntryPtr == NULL)
    {
        LPM_SHADOW_VALIDATION_ERROR_MAC(returnOnFailure, retVal, "Next hop",
                                        0,
                                        "routeEntryPtr is null");
    }
    else
    {
        if ((routeEntryPtr->routeEntryMethod != PRV_CPSS_DXCH_LPM_ENTRY_TYPE_MULTIPATH_E) &&
            (routeEntryPtr->routeEntryMethod != PRV_CPSS_DXCH_LPM_ENTRY_TYPE_REGULAR_E))
        {
            LPM_SHADOW_VALIDATION_ERROR_MAC(returnOnFailure, retVal, "Next hop",
                                            routeEntryPtr,
                                            "illegal route entry method");
        }

         /* For each device check validity of route entries */
        for (devIdx = 0; devIdx < shareDevListLen; devIdx++)
        {
            /*Lock the access to per device data base in order to avoid corruption*/
            CPSS_API_LOCK_MAC(devIdx,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

            if (routeEntryPtr->routeEntryMethod == PRV_CPSS_DXCH_LPM_ENTRY_TYPE_REGULAR_E)
            {
                maxMemSize = PRV_CPSS_DXCH_PP_MAC(shareDevsList[devIdx])->fineTuning.tableSize.routerNextHop;
            }
            else
            {
                maxMemSize = PRV_CPSS_DXCH_PP_MAC(shareDevsList[devIdx])->fineTuning.tableSize.ecmpQos;
            }

            /*Unlock the access to per device data base*/
            CPSS_API_UNLOCK_MAC(devIdx,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

            /* Check that the base address is within the table */
            if (routeEntryPtr->routeEntryBaseMemAddr >= maxMemSize)
            {
                LPM_SHADOW_VALIDATION_ERROR_MAC(returnOnFailure, retVal, "Next hop",
                                                routeEntryPtr,
                                                "illegal route entry address");
            }
        }

        if (routeEntryPtr->ipv6McGroupScopeLevel > CPSS_IPV6_PREFIX_SCOPE_GLOBAL_E)
        {
            LPM_SHADOW_VALIDATION_ERROR_MAC(returnOnFailure, retVal, "Next hop",
                                            routeEntryPtr,
                                            "illegal IPv6 MC Group Scope Level");
        }
    }
    return retVal;
}

/**
* @internal prvCpssDxChLpmRamDbgTrieShadowValidityCheck function
* @endinternal
*
* @brief   Validation of bucket's trie
*
* @param[in] rootPtr                  - a root node
* @param[in] level                    - the  in the trie. first  is 0.
* @param[in] nodeStartAddr            - the start address that this node represents
* @param[in] sumOfHigherLevelsMask    - the sum of the masks for this node's higher
* @param[in] level
* @param[in] returnOnFailure          - GT_TRUE: the validation check stops on
*                                      first failure
*                                      GT_FALSE: continue with the test on failure
* @param[in,out] maskForRangeInTrieArray[] - array of the ranges masks as found in the
*                                      trie. The array index represents the range
*                                      start address
* @param[in,out] validRangeInTrieArray[]  - array to indicate if a range was found in
*                                      the trie. The array index represents the
*                                      range start address.
* @param[in,out] maskForRangeInTrieArray[] - array of the ranges masks as found in the
*                                      trie. The array index represents the range
* @param[in,out] validRangeInTrieArray[]  - array to indicate if a range was found in
*                                      the trie. The array index represents the
*                                      range start address.
*
* @retval GT_FAIL                  - on failure
* @retval GT_OK                    - on success
*/
static GT_STATUS prvCpssDxChLpmRamDbgTrieShadowValidityCheck
(
    IN    PRV_CPSS_DXCH_LPM_RAM_TRIE_NODE_STC     *rootPtr,
    IN    GT_U8                                   level,
    IN    GT_U8                                   nodeStartAddr,
    IN    GT_U8                                   sumOfHigherLevelsMask,
    IN    GT_BOOL                                 returnOnFailure,
    INOUT GT_U8                                   maskForRangeInTrieArray[],
    INOUT GT_BOOL                                 validRangeInTrieArray[]
)
{
    GT_U8 power, rangeIdx, thisLevelMask = 0;
    GT_STATUS status, retVal = GT_OK;

    /* Check that there are not too many levels in the trie */
    if (level > PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS)
    {
        LPM_SHADOW_VALIDATION_ERROR_MAC(returnOnFailure, retVal, "Trie",
                                        rootPtr,
                                        "illegal level in the trie");
    }

    /* the trie root must contain pData */
    if ((level == 0) && (rootPtr->pData == NULL))
    {
        LPM_SHADOW_VALIDATION_ERROR_MAC(returnOnFailure, retVal, "Trie",
                                        rootPtr,
                                        "the trie root must contain pData");
    }

    /* the node must hold some info */
    if ((rootPtr->pData == NULL) && (rootPtr->leftSon == NULL) && (rootPtr->rightSon == NULL))
    {
        LPM_SHADOW_VALIDATION_ERROR_MAC(returnOnFailure, retVal, "Trie",
                                        rootPtr,
                                        "the trie node holds no pData and no children");
    }

    /* Father and son should point to each other */
    if (rootPtr->leftSon != NULL)
    {
        if (rootPtr->leftSon->father != rootPtr)
        {
            LPM_SHADOW_VALIDATION_ERROR_MAC(returnOnFailure, retVal, "Trie",
                                            rootPtr,
                                            "father and left son don't point to each other");
        }
    }
    if (rootPtr->rightSon != NULL)
    {
        if (rootPtr->rightSon->father != rootPtr)
        {
            LPM_SHADOW_VALIDATION_ERROR_MAC(returnOnFailure, retVal, "Trie",
                                            rootPtr,
                                            "father and right son don't point to each other");
        }
    }

    /* The node holds a pointer to NH, so startAddr is a start of range */
    if (rootPtr->pData != NULL)
    {
        if (level > 0)
        {
            status = prvCpssMathPowerOf2((GT_U8)(level - 1), &thisLevelMask);
            if (status != GT_OK)
            {
                cpssOsPrintf("prvCpssMathPowerOf2 failed\n");
                return status;
            }
            maskForRangeInTrieArray[nodeStartAddr] =
                (GT_U8)(sumOfHigherLevelsMask + thisLevelMask);
        }
        else
        {
            maskForRangeInTrieArray[nodeStartAddr] = 0;
        }
        validRangeInTrieArray[nodeStartAddr] = GT_TRUE;
    }
    /* For all the levels except the root level:
       In case that the node is a left son and it represents a start address,
       a new range exists and its startAddr is the end address of the node's
       range + 1 */
    if ((level > 0) && (rootPtr->father->leftSon == rootPtr) && (rootPtr->pData != NULL))
    {
        status = prvCpssMathPowerOf2((GT_U8)(8 - level), &power);
        if (status != GT_OK)
        {
            cpssOsPrintf("prvCpssMathPowerOf2 failed\n");
            return status;
        }
        rangeIdx = (GT_U8)(nodeStartAddr + power);
        validRangeInTrieArray[rangeIdx] = GT_TRUE;
        maskForRangeInTrieArray[rangeIdx] = sumOfHigherLevelsMask;
    }

    level++;
    sumOfHigherLevelsMask = (GT_U8)(sumOfHigherLevelsMask + thisLevelMask);
    /* Validate the sub-tries */
    if (rootPtr->leftSon != NULL)
    {
        status = prvCpssDxChLpmRamDbgTrieShadowValidityCheck(rootPtr->leftSon,
                                                             level,
                                                             nodeStartAddr,
                                                             sumOfHigherLevelsMask,
                                                             returnOnFailure,
                                                             maskForRangeInTrieArray,
                                                             validRangeInTrieArray);
        if (status != GT_OK)
        {
            retVal = status;
            if (returnOnFailure == GT_TRUE)
            {
                return retVal;
            }
        }
    }
    if (rootPtr->rightSon != NULL)
    {
        status = prvCpssMathPowerOf2((GT_U8)(8 - level), &power);
        if (status != GT_OK)
        {
            cpssOsPrintf("prvCpssMathPowerOf2 failed\n");
            return status;
        }
        status = prvCpssDxChLpmRamDbgTrieShadowValidityCheck(rootPtr->rightSon,
                                                             level,
                                                             (GT_U8)(nodeStartAddr + power),
                                                             sumOfHigherLevelsMask,
                                                             returnOnFailure,
                                                             maskForRangeInTrieArray,
                                                             validRangeInTrieArray);
        if (status != GT_OK)
        {
            retVal = status;
            if (returnOnFailure == GT_TRUE)
            {
                return retVal;
            }
        }
    }

    return retVal;
}

/**
* @internal prvCpssDxChLpmRamDbgTrieShadowValidityCheckSip6
*           function
* @endinternal
*
* @brief   Validation of bucket's trie
*
* @param[in] rootPtr                  - a root node
* @param[in] level                    - the  in the trie. first  is 0.
* @param[in] nodeStartAddr            - the start address that this node represents
* @param[in] sumOfHigherLevelsMask    - the sum of the masks for this node's higher
* @param[in] level
* @param[in] returnOnFailure          - GT_TRUE: the validation check stops on
*                                      first failure
*                                      GT_FALSE: continue with the test on failure
* @param[in,out] maskForRangeInTrieArray[] - array of the ranges masks as found in the
*                                      trie. The array index represents the range
*                                      start address
* @param[in,out] validRangeInTrieArray[]  - array to indicate if a range was found in
*                                      the trie. The array index represents the
*                                      range start address.
* @param[in,out] maskForRangeInTrieArray[] - array of the ranges masks as found in the
*                                      trie. The array index represents the range
* @param[in,out] validRangeInTrieArray[]  - array to indicate if a range was found in
*                                      the trie. The array index represents the
*                                      range start address.
*
* @retval GT_FAIL                  - on failure
* @retval GT_OK                    - on success
*/
static GT_STATUS prvCpssDxChLpmRamDbgTrieShadowValidityCheckSip6
(
    IN    PRV_CPSS_DXCH_LPM_RAM_TRIE_NODE_STC     *rootPtr,
    IN    GT_U8                                   level,
    IN    GT_U8                                   nodeStartAddr,
    IN    GT_U8                                   sumOfHigherLevelsMask,
    IN    GT_BOOL                                 returnOnFailure,
    INOUT GT_U8                                   maskForRangeInTrieArray[],
    INOUT GT_BOOL                                 validRangeInTrieArray[]
)
{
    GT_U8 power, rangeIdx, thisLevelMask = 0;
    GT_STATUS status, retVal = GT_OK;

    /* Check that there are not too many levels in the trie */
    if (level > PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS)
    {
        LPM_SHADOW_VALIDATION_ERROR_MAC(returnOnFailure, retVal, "Trie",
                                        rootPtr,
                                        "illegal level in the trie");
    }

    /* the trie root must contain pData */
    if ((level == 0) && (rootPtr->pData == NULL))
    {
        LPM_SHADOW_VALIDATION_ERROR_MAC(returnOnFailure, retVal, "Trie",
                                        rootPtr,
                                        "the trie root must contain pData");
    }

    /* the node must hold some info */
    if ((rootPtr->pData == NULL) && (rootPtr->leftSon == NULL) && (rootPtr->rightSon == NULL))
    {
        LPM_SHADOW_VALIDATION_ERROR_MAC(returnOnFailure, retVal, "Trie",
                                        rootPtr,
                                        "the trie node holds no pData and no children");
    }

    /* Father and son should point to each other */
    if (rootPtr->leftSon != NULL)
    {
        if (rootPtr->leftSon->father != rootPtr)
        {
            LPM_SHADOW_VALIDATION_ERROR_MAC(returnOnFailure, retVal, "Trie",
                                            rootPtr,
                                            "father and left son don't point to each other");
        }
    }
    if (rootPtr->rightSon != NULL)
    {
        if (rootPtr->rightSon->father != rootPtr)
        {
            LPM_SHADOW_VALIDATION_ERROR_MAC(returnOnFailure, retVal, "Trie",
                                            rootPtr,
                                            "father and right son don't point to each other");
        }
    }

    /* The node holds a pointer to NH, so startAddr is a start of range */
    if (rootPtr->pData != NULL)
    {
        if (level > 0)
        {
            status = prvCpssMathPowerOf2((GT_U8)(level - 1), &thisLevelMask);
            if (status != GT_OK)
            {
                cpssOsPrintf("prvCpssMathPowerOf2 failed\n");
                return status;
            }
            maskForRangeInTrieArray[nodeStartAddr] =
                (GT_U8)(sumOfHigherLevelsMask + thisLevelMask);
        }
        else
        {
            maskForRangeInTrieArray[nodeStartAddr] = 0;
        }
        validRangeInTrieArray[nodeStartAddr] = GT_TRUE;
    }
    /* For all the levels except the root level:
       In case that the node is a left son and it represents a start address,
       a new range exists and its startAddr is the end address of the node's
       range + 1 */
    if ((level > 0) && (rootPtr->father->leftSon == rootPtr) && (rootPtr->pData != NULL))
    {
        status = prvCpssMathPowerOf2((GT_U8)(8 - level), &power);
        if (status != GT_OK)
        {
            cpssOsPrintf("prvCpssMathPowerOf2 failed\n");
            return status;
        }
        rangeIdx = (GT_U8)(nodeStartAddr + power);
        validRangeInTrieArray[rangeIdx] = GT_TRUE;
        maskForRangeInTrieArray[rangeIdx] = sumOfHigherLevelsMask;
    }

    level++;
    sumOfHigherLevelsMask = (GT_U8)(sumOfHigherLevelsMask + thisLevelMask);
    /* Validate the sub-tries */
    if (rootPtr->leftSon != NULL)
    {
        status = prvCpssDxChLpmRamDbgTrieShadowValidityCheckSip6(rootPtr->leftSon,
                                                             level,
                                                             nodeStartAddr,
                                                             sumOfHigherLevelsMask,
                                                             returnOnFailure,
                                                             maskForRangeInTrieArray,
                                                             validRangeInTrieArray);
        if (status != GT_OK)
        {
            retVal = status;
            if (returnOnFailure == GT_TRUE)
            {
                return retVal;
            }
        }
    }
    if (rootPtr->rightSon != NULL)
    {
        status = prvCpssMathPowerOf2((GT_U8)(8 - level), &power);
        if (status != GT_OK)
        {
            cpssOsPrintf("prvCpssMathPowerOf2 failed\n");
            return status;
        }
        status = prvCpssDxChLpmRamDbgTrieShadowValidityCheckSip6(rootPtr->rightSon,
                                                             level,
                                                             (GT_U8)(nodeStartAddr + power),
                                                             sumOfHigherLevelsMask,
                                                             returnOnFailure,
                                                             maskForRangeInTrieArray,
                                                             validRangeInTrieArray);
        if (status != GT_OK)
        {
            retVal = status;
            if (returnOnFailure == GT_TRUE)
            {
                return retVal;
            }
        }
    }

    return retVal;
}

/**
* @internal prvCpssDxChLpmRamDbgRangesShadowValidityCheck function
* @endinternal
*
* @brief   Shadow validation of bucket's ranges
*
* @param[in] shadowPtr                - pointer to shadow structure
* @param[in] vrId                     - the virtual router id
* @param[in] bucketPtr                - pointer to the bucket
* @param[in] maskForRangeInTrieArray[] - array of the ranges masks as found in the
*                                      trie. The array index represents the range
*                                      start address
* @param[in] validRangeInTrieArray[]  - array to indicate if a range was found in
*                                      the trie. The array index represents the
*                                      range start address.
* @param[in] level                    - the  in the tree (first  is 0)
* @param[in] numOfMaxAllowedLevels    - the maximal number of levels that is allowed
*                                      for the relevant protocol and prefix type
* @param[in] prefixType               - unicast or multicast tree
* @param[in] protocol                 - protocol
* @param[in] returnOnFailure          - GT_TRUE: the validation check stops on
*                                      first failure
*                                      GT_FALSE: continue with the test on failure
*
* @retval GT_FAIL                  - on failure
* @retval GT_OK                    - on success
*/
static GT_STATUS prvCpssDxChLpmRamDbgRangesShadowValidityCheck
(
    IN  PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC          *shadowPtr,
    IN  GT_U32                                    vrId,
    IN  PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC   *bucketPtr,
    IN  GT_U8                                     maskForRangeInTrieArray[],
    IN  GT_BOOL                                   validRangeInTrieArray[],
    IN  GT_U8                                     level,
    IN  GT_U8                                     numOfMaxAllowedLevels,
    IN  CPSS_UNICAST_MULTICAST_ENT                prefixType,
    IN  PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT      protocol,
    IN  GT_BOOL                                   returnOnFailure
)
{
    PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC  *rangePtr, *nextRangePtr;
    GT_U32                                  bucketBaseAddr, rangeOffset,
                                            expectedPointingRangeMemAddr;
    GT_U16                                  numOfRanges, numOfRangesForPrefixType;
    GT_U8                                   prefixTypeFirstRange, prefixTypeLastRange;
    GT_U32                                  tmpPrefixTypeRange;
    GT_U8                                   trieRangeIdx;
    GT_STATUS                               status, retVal = GT_OK;
    GT_U32                                  blockIndex=0;
    PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC      *octetToBlockMapping;
    GT_BOOL                                 foundMapping=GT_FALSE;
    GT_U32                                  octetNum=0;

    rangePtr = bucketPtr->rangeList;
    numOfRanges = 0;
    numOfRangesForPrefixType = 0;

    if (level == 0)
    {
        if (prefixType == CPSS_UNICAST_E)
        {
            prefixTypeFirstRange = 0;
            tmpPrefixTypeRange = (protocol == PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E) ?
                (PRV_CPSS_DXCH_LPM_RAM_START_OF_IPV4_MC_ADDRESS_SPACE_CNS - 1) :
                (PRV_CPSS_DXCH_LPM_RAM_START_OF_IPV6_MC_ADDRESS_SPACE_CNS - 1);
            prefixTypeLastRange = (GT_U8)tmpPrefixTypeRange;
        }
        else /* CPSS_MULTICAST_E */
        {
            tmpPrefixTypeRange = (protocol == PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E) ?
                PRV_CPSS_DXCH_LPM_RAM_START_OF_IPV4_MC_ADDRESS_SPACE_CNS :
                PRV_CPSS_DXCH_LPM_RAM_START_OF_IPV6_MC_ADDRESS_SPACE_CNS;
            prefixTypeFirstRange = (GT_U8)tmpPrefixTypeRange;
            tmpPrefixTypeRange = (protocol == PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E) ?
                PRV_CPSS_DXCH_LPM_RAM_END_OF_IPV4_MC_ADDRESS_SPACE_CNS :
                PRV_CPSS_DXCH_LPM_RAM_END_OF_IPV6_MC_ADDRESS_SPACE_CNS;
            prefixTypeLastRange = (GT_U8)tmpPrefixTypeRange;
        }
    }
    else
    {
        prefixTypeFirstRange = 0;
        prefixTypeLastRange = 255;
    }

    /* check that the bucket bank number exist in the shadow linked_list of the specific level */

    /* find the octetNum according to level/protocol/type */
    if (prefixType == CPSS_UNICAST_E)
    {
        octetNum = level;
    }
    else
    {
        switch(protocol)
        {
        case PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E:
            octetNum = level%PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS;
            break;
        case PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E:
            octetNum = level%PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS;
            break;
        case PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E:
            octetNum = level%PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_FCOE_PROTOCOL_CNS;
            break;
        default:
            LPM_SHADOW_VALIDATION_ERROR_MAC(returnOnFailure, retVal, "Level",
                                        level,
                                        "Wrong protocol level");
            break;
        }
    }
    blockIndex = PRV_CPSS_DXCH_LPM_RAM_GET_LPM_OFFSET_FROM_DMM_MAC(bucketPtr->hwBucketOffsetHandle)/shadowPtr->lpmRamTotalBlocksSizeIncludingGap;
    octetToBlockMapping = &shadowPtr->lpmMemInfoArray[protocol][octetNum];
    while((octetToBlockMapping!=NULL)&&(octetToBlockMapping->structsMemPool!=0))
    {
        /* go over all the block list assosiated to the level*/
        if (octetToBlockMapping->ramIndex==blockIndex)
        {
            /* we found the block in the list */
            foundMapping=GT_TRUE;
            break;
        }
        octetToBlockMapping = octetToBlockMapping->nextMemInfoPtr;
    }
    if(foundMapping == GT_FALSE)
    {
        LPM_SHADOW_VALIDATION_ERROR_MAC(returnOnFailure, retVal, "Level",
                                        level,
                                        "The bucket block do not exist in the octet to block mapping list");
    }

    /* Check that the numbers of actual ranges is bucketPtr->numOfRanges */
    while (rangePtr)
    {
        /* Skip unicast ranges for multicast validation or multicast ranges for
           unicast validation */
        if ((rangePtr->startAddr < prefixTypeFirstRange) ||
            (rangePtr->startAddr > prefixTypeLastRange))
        {
            numOfRanges++;
            rangePtr = rangePtr->next;
            continue;
        }

        /* Check startAddr for the first range of the unicast/multicast prefixes */
        if ((numOfRangesForPrefixType == 0) && (rangePtr->startAddr != prefixTypeFirstRange))
        {
            LPM_SHADOW_VALIDATION_ERROR_MAC(returnOnFailure, retVal, "Range",
                                            rangePtr,
                                            "start address of the first range is not 0");
        }

        /* In compress 2, startAddr of the 5th range (actually 6th because the
           first is always zero) must be equal to bucketPtr->fifthAddress */
        if ((bucketPtr->bucketType == CPSS_DXCH_LPM_COMPRESSED_2_NODE_PTR_TYPE_E) &&
            (numOfRanges == 5) && (rangePtr->startAddr != bucketPtr->fifthAddress))
        {
            LPM_SHADOW_VALIDATION_ERROR_MAC(returnOnFailure, retVal, "Range",
                                            rangePtr,
                                            "start address of the fifth range is different than the bucket fifthAddress");
        }

        nextRangePtr = rangePtr->next;
        /* Ranges must be in increasing order */
        if ((nextRangePtr) && (nextRangePtr->startAddr <= rangePtr->startAddr))
        {
            LPM_SHADOW_VALIDATION_ERROR_MAC(returnOnFailure, retVal, "Range",
                                            rangePtr,
                                            "the ranges are not in increasing order");
        }

        /* A range that points to bucket must be of length 1 (its start address
           must be equal to its end address) */
        if ((rangePtr->startAddr < 255) &&
            (rangePtr->pointerType <= CPSS_DXCH_LPM_COMPRESSED_2_NODE_PTR_TYPE_E))
        {
            if (nextRangePtr == NULL)
            {
                /* no next range */
                LPM_SHADOW_VALIDATION_ERROR_MAC(returnOnFailure, retVal, "Range",
                                                rangePtr,
                                                "the range points to bucket but its length is not 1");
            }
            else
            {
                if (rangePtr->startAddr != (nextRangePtr->startAddr - 1))
                {
                    LPM_SHADOW_VALIDATION_ERROR_MAC(returnOnFailure, retVal, "Range",
                                                    rangePtr,
                                                    "the range points to bucket but its length is not 1");
                }
            }
        }

        if ((rangePtr->pointerType > CPSS_DXCH_LPM_QOS_ENTRY_PTR_TYPE_E) &&
            ((GT_U32)rangePtr->pointerType != (GT_U32)PRV_CPSS_DXCH_LPM_RAM_TRIE_PTR_TYPE_E))
        {
            LPM_SHADOW_VALIDATION_ERROR_MAC(returnOnFailure, retVal, "Level",
                                            rangePtr,
                                            "The range points to an unknown pointer type");
        }

        /* check that tree level is not over the limit */
        if ((((protocol == PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E) && (level == (2*PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS))) ||
             ((protocol == PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E) && (level == (2*PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS)))) &&
            (prefixType == CPSS_MULTICAST_E))
        {
            LPM_SHADOW_VALIDATION_ERROR_MAC(returnOnFailure, retVal, "Level",
                                                rangePtr,
                                                "MC tree reached an illegal level");
        }
        else
        {
            if ((((protocol == PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E) && (level == PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS)) ||
                 ((protocol == PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E) && (level == PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS))) &&
                (prefixType == CPSS_UNICAST_E))
            {
                LPM_SHADOW_VALIDATION_ERROR_MAC(returnOnFailure, retVal, "Level",
                                                    rangePtr,
                                                    "UC tree reached an illegal level");
            }
        }

        /* Check validity for nexthop */
        if ((rangePtr->pointerType == CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E) ||
            (rangePtr->pointerType == CPSS_DXCH_LPM_ECMP_ENTRY_PTR_TYPE_E) ||
            (rangePtr->pointerType == CPSS_DXCH_LPM_QOS_ENTRY_PTR_TYPE_E))
        {
            status = prvCpssDxChLpmRamDbgRouteEntryShadowValidityCheck(shadowPtr,
                                                                       rangePtr->lowerLpmPtr.nextHopEntry,
                                                                       returnOnFailure);
            if (status != GT_OK)
            {
                retVal = status;
                if (returnOnFailure == GT_TRUE)
                {
                    return retVal;
                }
            }
        }

        /* The range was possibly found in the trie. */
        trieRangeIdx = rangePtr->startAddr;
        if (validRangeInTrieArray[trieRangeIdx] == GT_TRUE)
        {
            if (rangePtr->mask != maskForRangeInTrieArray[trieRangeIdx])
            {
                LPM_SHADOW_VALIDATION_ERROR_MAC(returnOnFailure, retVal, "Range",
                                                rangePtr,
                                                "the range has a different mask in the trie");
            }
            validRangeInTrieArray[trieRangeIdx] = GT_FALSE;
        }
        numOfRanges++;
        numOfRangesForPrefixType++;
        rangePtr = nextRangePtr;
    }

    if (numOfRanges != bucketPtr->numOfRanges)
    {
        LPM_SHADOW_VALIDATION_ERROR_MAC(returnOnFailure, retVal, "Bucket",
                                        bucketPtr,
                                        "the number of actual ranges is different than the bucket's numOfRanges");
    }

    /* Check if all the ranges in the trie exist in the bucket ranges list */
    for (trieRangeIdx = 0; trieRangeIdx < 255; trieRangeIdx++)
    {
        if ((trieRangeIdx < prefixTypeFirstRange) || (trieRangeIdx > prefixTypeLastRange))
        {
            continue;
        }
        if (validRangeInTrieArray[trieRangeIdx] == GT_TRUE)
        {
            LPM_SHADOW_VALIDATION_ERROR_MAC(returnOnFailure, retVal, "Bucket",
                                            bucketPtr,
                                            "a range that appears in the trie does not appear in the ranges list");
        }
    }

    /* Check next buckets */
    rangePtr = bucketPtr->rangeList;
    if (prefixType == CPSS_UNICAST_E)
    {
        bucketBaseAddr =
            shadowPtr->ucSearchMemArrayPtr[protocol][level]->structsBase +
            PRV_CPSS_DXCH_LPM_RAM_GET_LPM_OFFSET_FROM_DMM_MAC(bucketPtr->hwBucketOffsetHandle);
    }
    else
    {
        bucketBaseAddr =
            shadowPtr->mcSearchMemArrayPtr[protocol][level]->structsBase +
            PRV_CPSS_DXCH_LPM_RAM_GET_LPM_OFFSET_FROM_DMM_MAC(bucketPtr->hwBucketOffsetHandle);
    }

    /* number of LPM lines for the bitmap */
    if (bucketPtr->bucketType == CPSS_DXCH_LPM_COMPRESSED_1_NODE_PTR_TYPE_E)
    {
        rangeOffset = PRV_CPSS_DXCH_LPM_RAM_BUCKET_BIT_VEC_SIZE_COMPRESSED_1_CNS;
    }
    else if (bucketPtr->bucketType == CPSS_DXCH_LPM_COMPRESSED_2_NODE_PTR_TYPE_E)
    {
        rangeOffset = PRV_CPSS_DXCH_LPM_RAM_BUCKET_BIT_VEC_SIZE_COMPRESSED_2_CNS;
    }
    else    /* CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E */
    {
        rangeOffset = PRV_CPSS_DXCH_LPM_RAM_BUCKET_BIT_VEC_SIZE_REGULAR_CNS;
    }

    while (rangePtr)
    {
        if ((rangePtr->pointerType != CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E) &&
            (rangePtr->pointerType != CPSS_DXCH_LPM_ECMP_ENTRY_PTR_TYPE_E) &&
            (rangePtr->pointerType != CPSS_DXCH_LPM_QOS_ENTRY_PTR_TYPE_E))
        {
            if ((rangePtr->startAddr < prefixTypeFirstRange) ||
                (rangePtr->startAddr > prefixTypeLastRange))
            {
                rangePtr = rangePtr->next;
                rangeOffset++;
                continue;
            }
            expectedPointingRangeMemAddr = bucketBaseAddr + rangeOffset;
            status = prvCpssDxChLpmRamDbgBucketShadowValidityCheck(shadowPtr,
                                                                   vrId,
                                                                   rangePtr->lowerLpmPtr.nextBucket,
                                                                   (GT_U8)(level + 1),
                                                                   rangePtr->pointerType,
                                                                   numOfMaxAllowedLevels,
                                                                   prefixType,
                                                                   protocol,
                                                                   expectedPointingRangeMemAddr,
                                                                   returnOnFailure);
            if (status != GT_OK)
            {
                retVal = status;
                if (returnOnFailure == GT_TRUE)
                {
                    return retVal;
                }
            }
        }
        rangePtr = rangePtr->next;
        rangeOffset++;
    }

    return retVal;
}

/**
* @internal prvCpssDxChLpmRamDbgBucketCheckSizesOfGonsSip6
*           function
* @endinternal
*
* @brief  function that goes over the
*          bucketPtr->hwGroupOffsetHandle and over the range
*          list and check for each pointer size that is the same
*          as the size of all structs pointed by those ranges
*
* @param[in] bucketPtr                - pointer to the bucket
*
*
* @retval GT_FAIL                  - on failure
* @retval GT_OK                    - on success
*/
static GT_STATUS prvCpssDxChLpmRamDbgBucketCheckSizesOfGonsSip6
(
    IN  PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC   *bucketPtr
)
{
    GT_U32 i=0,j=0;
    GT_U32 numberOfGons=0;
    GT_U32 sizeOfTheGonAccordingToRangeTypes=0;
    GT_U32 sizeOfTheGonAccordingToTheHwOffsetHandle=0;
    GT_U32 endOfRange=0,startOfRange=0;
    GT_BOOL hiddenRangeWasCounted=GT_FALSE, getSizeOfGon=GT_TRUE;
    PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC *tempRangePtr;
    GT_U32 numberOfRegular=0;
    GT_U32 numberOfComp=0;
    GT_U32 numberOfLeafs=0;
    GT_U32 numberOfLeafsLines=0;
    GT_U32 maxNumberOfEmbedded=0;


    for(i=0;i<PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS; i++)
    {
        if(bucketPtr->hwGroupOffsetHandle[i]!=0)
            numberOfGons++;
    }

    if ((numberOfGons!=0) && (numberOfGons!=1) && (numberOfGons!=6))
    {
         /* the only 3 leggal options for the array are:
            1. 6 leggal pointers, meaning a regular bucket
            2. single pointer , meaning a compressed bucket
            3. null pointer, meaning last octet bucket is all embedded
           */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    switch(bucketPtr->bucketType)
    {
    case CPSS_DXCH_LPM_COMPRESSED_UP_TO_7_RANGES_1_LEAF_NODE_PTR_TYPE_E:
        maxNumberOfEmbedded = 1;
        break;
    case CPSS_DXCH_LPM_COMPRESSED_UP_TO_5_RANGES_2_LEAVES_NODE_PTR_TYPE_E:
        maxNumberOfEmbedded = 2;
        break;
    case CPSS_DXCH_LPM_COMPRESSED_3_RANGES_3_LEAVES_NODE_PTR_TYPE_E:
        maxNumberOfEmbedded = 3;
        break;
    case CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E:
    case CPSS_DXCH_LPM_MULTIPATH_ENTRY_PTR_TYPE_E:
    case CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E:
    case CPSS_DXCH_LPM_COMPRESSED_NODE_PTR_TYPE_E:
        maxNumberOfEmbedded=0;
        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    if(numberOfGons==0)
    {
        /* last octet treated */
        switch(bucketPtr->bucketType)
        {
        case CPSS_DXCH_LPM_COMPRESSED_UP_TO_7_RANGES_1_LEAF_NODE_PTR_TYPE_E:
        case CPSS_DXCH_LPM_COMPRESSED_UP_TO_5_RANGES_2_LEAVES_NODE_PTR_TYPE_E:
        case CPSS_DXCH_LPM_COMPRESSED_3_RANGES_3_LEAVES_NODE_PTR_TYPE_E:
            sizeOfTheGonAccordingToTheHwOffsetHandle = 1;
            getSizeOfGon=GT_FALSE;
            numberOfGons = 1;/* change value inorder to enter the loop */
            break;
        case CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E:
        case CPSS_DXCH_LPM_COMPRESSED_NODE_PTR_TYPE_E:
        case CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E:
        case CPSS_DXCH_LPM_MULTIPATH_ENTRY_PTR_TYPE_E:
        default:
            /* if all bucketPtr->hwGroupOffsetHandle[i] pointers are 0,
               it means that we have an all embedded bucket, all other options are illegal */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }
    }

    tempRangePtr = bucketPtr->rangeList;
    for(i=0;i<numberOfGons; i++)
    {
        numberOfRegular=0;
        numberOfComp=0;
        numberOfLeafs=0;
        numberOfLeafsLines=0;
        hiddenRangeWasCounted=0;

        if (getSizeOfGon==GT_TRUE)
        {
            sizeOfTheGonAccordingToTheHwOffsetHandle = PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_SIZE_FROM_DMM_MAC(bucketPtr->hwGroupOffsetHandle[i]);
        }
        startOfRange = (numberOfGons==1)? 0 : (i*44);
        endOfRange = (numberOfGons==1)? 255 : (i*44+43);
        /* go over the range list until we get to startOfRange, and check number of lines in the GON.
           regular hold 6 lines
           compressed hold 1 line
           and 5 leafs hold 1 line */
        while (tempRangePtr != NULL)
        {
            if(tempRangePtr->startAddr >= startOfRange &&
               tempRangePtr->startAddr <= endOfRange)
            {
                /* in this case the middle ranges of regular bucket should be counted
                   as the pointer they are*/
                switch(tempRangePtr->pointerType)
                {
                case CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E:
                    numberOfRegular++;
                    break;
                case CPSS_DXCH_LPM_COMPRESSED_NODE_PTR_TYPE_E:
                    numberOfComp++;
                    break;
                case CPSS_DXCH_LPM_COMPRESSED_UP_TO_7_RANGES_1_LEAF_NODE_PTR_TYPE_E:
                    numberOfComp++;
                    break;
                case CPSS_DXCH_LPM_COMPRESSED_UP_TO_5_RANGES_2_LEAVES_NODE_PTR_TYPE_E:
                    numberOfComp++;
                    break;
                case CPSS_DXCH_LPM_COMPRESSED_3_RANGES_3_LEAVES_NODE_PTR_TYPE_E:
                    numberOfComp++;
                    break;
                case CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E:
                case CPSS_DXCH_LPM_MULTIPATH_ENTRY_PTR_TYPE_E:
                    numberOfLeafs++;
                    break;
                default:
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
                }

                if (tempRangePtr->startAddr == startOfRange)
                {
                     if(hiddenRangeWasCounted==GT_FALSE)
                    {
                        hiddenRangeWasCounted=GT_TRUE;
                    }
                }
                else
                {
                    /* if the startAddr is bigger then the startOfRange then
                       it means the middle ranges of the regular bit vector
                       is hidden and we should count it as a leaf*/
                    if(hiddenRangeWasCounted==GT_FALSE)
                    {
                        hiddenRangeWasCounted=GT_TRUE;
                        numberOfLeafs++;
                    }
                }

                tempRangePtr = tempRangePtr->next;
            }
            else
            {   /* if the startAddr is bigger then the startOfRange then
                   it means the middle ranges of the regular bit vector
                   is hidden and we should count it as a leaf*/
                if(hiddenRangeWasCounted==GT_FALSE)
                {
                    hiddenRangeWasCounted=GT_TRUE;
                    numberOfLeafs++;
                }

                /* need to finish the current GON calculation and move to the next one
                in case of regular bucket, no need to go to next range- we still work on the same range  */
                break;
            }
        }

        /* reduce from the number of leafs counted the ones that will
           be embedded in a compressed line*/
        if (numberOfRegular==0&&numberOfComp==0&&numberOfLeafs<=maxNumberOfEmbedded)
        {
            /* in this case we have one line all embedde */
            sizeOfTheGonAccordingToRangeTypes=1;
        }
        else
        {
            for (j = 0; j < maxNumberOfEmbedded; j++)
            {
                if (numberOfLeafs!=0)
                {
                    numberOfLeafs--;
                }
            }
            /* now calculate the number of lines */
            numberOfLeafsLines = numberOfLeafs/MAX_NUMBER_OF_LEAVES_IN_LPM_LINE_CNS;
            if (numberOfLeafs%MAX_NUMBER_OF_LEAVES_IN_LPM_LINE_CNS!=0)
            {
                /* one extra non full line */
                numberOfLeafsLines++;
            }
            sizeOfTheGonAccordingToRangeTypes = (numberOfRegular*PRV_CPSS_DXCH_LPM_RAM_FALCON_BUCKET_SIZE_REGULAR_BV_CNS)+
                                                (numberOfComp*PRV_CPSS_DXCH_LPM_RAM_FALCON_BUCKET_SIZE_COMPRESSED_CNS)+
                                                (numberOfLeafsLines);
        }

        if (sizeOfTheGonAccordingToRangeTypes!=sizeOfTheGonAccordingToTheHwOffsetHandle)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }
    }
    return GT_OK;
}

/**
* @internal prvCpssDxChLpmRamDbgRangesShadowValidityCheckSip6
*           function
* @endinternal
*
* @brief   Shadow validation of bucket's ranges
*
* @param[in] shadowPtr                - pointer to shadow structure
* @param[in] vrId                     - the virtual router id
* @param[in] bucketPtr                - pointer to the bucket
* @param[in] maskForRangeInTrieArray[] - array of the ranges masks as found in the
*                                      trie. The array index represents the range
*                                      start address
* @param[in] validRangeInTrieArray[]  - array to indicate if a range was found in
*                                      the trie. The array index represents the
*                                      range start address.
* @param[in] level                    - the  in the tree (first  is 0)
* @param[in] numOfMaxAllowedLevels    - the maximal number of levels that is allowed
*                                      for the relevant protocol and prefix type
* @param[in] prefixType               - unicast or multicast tree
* @param[in] protocol                 - protocol
* @param[in] returnOnFailure          - GT_TRUE: the validation check stops on
*                                      first failure
*                                      GT_FALSE: continue with the test on failure
*
* @retval GT_FAIL                  - on failure
* @retval GT_OK                    - on success
*/
static GT_STATUS prvCpssDxChLpmRamDbgRangesShadowValidityCheckSip6
(
    IN  PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC          *shadowPtr,
    IN  GT_U32                                    vrId,
    IN  PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC   *bucketPtr,
    IN  GT_U8                                     maskForRangeInTrieArray[],
    IN  GT_BOOL                                   validRangeInTrieArray[],
    IN  GT_U8                                     level,
    IN  GT_U8                                     numOfMaxAllowedLevels,
    IN  CPSS_UNICAST_MULTICAST_ENT                prefixType,
    IN  PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT      protocol,
    IN  GT_BOOL                                   returnOnFailure
)
{
    PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC  *rangePtr, *nextRangePtr;
    GT_U32                                  expectedPointingRangeMemAddr;
    GT_U16                                  numOfRanges, numOfRangesForPrefixType;
    GT_U8                                   prefixTypeFirstRange, prefixTypeLastRange;
    GT_U8                                   prefixTypeSecondRange=0, prefixTypeSecondLastRange=0;
    GT_U32                                  tmpPrefixTypeRange;
    GT_U8                                   trieRangeIdx;
    GT_STATUS                               status, retVal = GT_OK;
    #if 0
    GT_U32                                  i, blockIndex=0;
    PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC      *octetToBlockMapping;
    GT_BOOL                                 foundMapping=GT_FALSE;
    GT_UINTPTR                              tmpHwBucketOffsetHandle;
    GT_U32                                  octetNum;
    #endif
    rangePtr = bucketPtr->rangeList;
    numOfRanges = 0;
    numOfRangesForPrefixType = 0;

    if (level == 0)
    {
        if (prefixType == CPSS_UNICAST_E)
        {
            prefixTypeFirstRange = 0;
            tmpPrefixTypeRange = (protocol == PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E) ?
                (PRV_CPSS_DXCH_LPM_RAM_START_OF_IPV4_MC_ADDRESS_SPACE_CNS - 1) :
                (PRV_CPSS_DXCH_LPM_RAM_START_OF_IPV6_MC_ADDRESS_SPACE_CNS - 1);
            prefixTypeLastRange = (GT_U8)tmpPrefixTypeRange;

            if (protocol == PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E)
            {
                prefixTypeSecondRange = PRV_CPSS_DXCH_LPM_RAM_START_OF_IPV4_RESERVED_SPACE_ADDRESS_SPACE_CNS;
                prefixTypeSecondLastRange = PRV_CPSS_DXCH_LPM_RAM_END_OF_IPV4_RESERVED_SPACE_ADDRESS_SPACE_CNS;
            }
        }
        else /* CPSS_MULTICAST_E */
        {
            tmpPrefixTypeRange = (protocol == PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E) ?
                PRV_CPSS_DXCH_LPM_RAM_START_OF_IPV4_MC_ADDRESS_SPACE_CNS :
                PRV_CPSS_DXCH_LPM_RAM_START_OF_IPV6_MC_ADDRESS_SPACE_CNS;
            prefixTypeFirstRange = (GT_U8)tmpPrefixTypeRange;
            tmpPrefixTypeRange = (protocol == PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E) ?
                PRV_CPSS_DXCH_LPM_RAM_END_OF_IPV4_MC_ADDRESS_SPACE_CNS :
                PRV_CPSS_DXCH_LPM_RAM_END_OF_IPV6_MC_ADDRESS_SPACE_CNS;
            prefixTypeLastRange = (GT_U8)tmpPrefixTypeRange;
        }
    }
    else
    {
        prefixTypeFirstRange = 0;
        prefixTypeLastRange = 255;
        prefixTypeSecondRange = 0;
        prefixTypeSecondLastRange = 255;

    }
#if 0
    /* check that the bucket bank number exist in the shadow linked_list of the specific level */

    /* find the octetNum according to level/protocol/type */
    if (prefixType == CPSS_UNICAST_E)
    {
        octetNum = level;
    }
    else
    {
        switch(protocol)
        {
        case PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E:
            octetNum = level%PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS;
            break;
        case PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E:
            octetNum = level%PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS;
            break;
        case PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E:
            octetNum = level%PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_FCOE_PROTOCOL_CNS;
            break;
        default:
            LPM_SHADOW_VALIDATION_ERROR_MAC(returnOnFailure, retVal, "Level",
                                        level,
                                        "Wrong protocol level");
            break;
        }
    }
    /* go over all gons */
    for(i=0;i<6;i++)
    {
        /* check if we are in a root bucket or in a root SRC*/
        if((i==0)&&(bucketPtr->hwBucketOffsetHandle!=0))
        {
            tmpHwBucketOffsetHandle = bucketPtr->hwBucketOffsetHandle;
            i=6;/* set to 6 to pervent continue in the loop */
        }
        else
        {
            tmpHwBucketOffsetHandle = bucketPtr->hwGroupOffsetHandle[i];
        }

        if (tmpHwBucketOffsetHandle!=0)
        {
            blockIndex = PRV_CPSS_DXCH_LPM_RAM_GET_LPM_OFFSET_FROM_DMM_MAC(tmpHwBucketOffsetHandle) / shadowPtr->lpmRamTotalBlocksSizeIncludingGap;
            blockIndex=blockIndex-shadowPtr->memoryOffsetValue;

            octetToBlockMapping = &shadowPtr->lpmMemInfoArray[protocol][octetNum];
            while((octetToBlockMapping!=NULL)&&(octetToBlockMapping->structsMemPool!=0))
            {
                /* go over all the block list assosiated to the level*/
                if (octetToBlockMapping->ramIndex==blockIndex)
                {
                    /* we found the block in the list */
                    foundMapping=GT_TRUE;
                    break;
                }
                octetToBlockMapping = octetToBlockMapping->nextMemInfoPtr;
            }
        }
    }

    if(foundMapping == GT_FALSE)
    {
        LPM_SHADOW_VALIDATION_ERROR_MAC(returnOnFailure, retVal, "Level",
                                        level,
                                        "The bucket block do not exist in the octet to block mapping list");
    }

#endif
    /* function that goes over the bucketPtr->hwGroupOffsetHandle[j]
       and over the range list and check for each pointer size that
       is the same as the size of all structs pointed by the ranges */
    retVal = prvCpssDxChLpmRamDbgBucketCheckSizesOfGonsSip6(bucketPtr);
    if (retVal != GT_OK)
    {
        LPM_SHADOW_VALIDATION_ERROR_MAC(returnOnFailure, retVal, "Bucket",
                                        bucketPtr,
                                        "hwGroupOffsetHandle wrong GON size");
    }

    /* Check that the numbers of actual ranges is bucketPtr->numOfRanges */
    while (rangePtr)
    {
        /* Skip unicast ranges for multicast validation or multicast ranges for
           unicast validation */
        if (protocol == PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E)
        {
            if ((rangePtr->startAddr < prefixTypeFirstRange) ||
                ((rangePtr->startAddr > prefixTypeLastRange) && (rangePtr->startAddr<prefixTypeSecondRange))||
                 (rangePtr->startAddr > prefixTypeSecondLastRange))
            {
                numOfRanges++;
                rangePtr = rangePtr->next;
                continue;
            }
        }
        else
        {
            if ((rangePtr->startAddr < prefixTypeFirstRange) ||
                (rangePtr->startAddr > prefixTypeLastRange))
            {
                numOfRanges++;
                rangePtr = rangePtr->next;
                continue;
            }
        }
        /* Check startAddr for the first range of the unicast/multicast prefixes */
        if ((numOfRangesForPrefixType == 0) && (rangePtr->startAddr != prefixTypeFirstRange))
        {
            LPM_SHADOW_VALIDATION_ERROR_MAC(returnOnFailure, retVal, "Range",
                                            rangePtr,
                                            "start address of the first range is not 0");
        }

        nextRangePtr = rangePtr->next;
        /* Ranges must be in increasing order */
        if ((nextRangePtr) && (nextRangePtr->startAddr <= rangePtr->startAddr))
        {
            LPM_SHADOW_VALIDATION_ERROR_MAC(returnOnFailure, retVal, "Range",
                                            rangePtr,
                                            "the ranges are not in increasing order");
        }

        /* A range that points to bucket must be of length 1 (its start address
           must be equal to its end address) */
        if ((rangePtr->startAddr < 255) &&
            ((rangePtr->pointerType == CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E)||
             (rangePtr->pointerType == CPSS_DXCH_LPM_COMPRESSED_NODE_PTR_TYPE_E)||
             (rangePtr->pointerType == CPSS_DXCH_LPM_COMPRESSED_UP_TO_7_RANGES_1_LEAF_NODE_PTR_TYPE_E)||
             (rangePtr->pointerType == CPSS_DXCH_LPM_COMPRESSED_UP_TO_5_RANGES_2_LEAVES_NODE_PTR_TYPE_E)||
             (rangePtr->pointerType == CPSS_DXCH_LPM_COMPRESSED_3_RANGES_3_LEAVES_NODE_PTR_TYPE_E)))
        {
            if (nextRangePtr == NULL)
            {
                /* no next range */
                LPM_SHADOW_VALIDATION_ERROR_MAC(returnOnFailure, retVal, "Range",
                                                rangePtr,
                                                "the range points to bucket but its length is not 1");
            }
            else
            {
                if (rangePtr->startAddr != (nextRangePtr->startAddr - 1))
                {
                    LPM_SHADOW_VALIDATION_ERROR_MAC(returnOnFailure, retVal, "Range",
                                                    rangePtr,
                                                    "the range points to bucket but its length is not 1");
                }
            }
        }


        /* check that tree level is not over the limit */
        if ((((protocol == PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E) && (level == (2*PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_FOR_GON_IN_IPV4_PROTOCOL_CNS))) ||
             ((protocol == PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E) && (level == (2*PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_FOR_GON_IN_IPV6_PROTOCOL_CNS)))) &&
             (prefixType == CPSS_MULTICAST_E))
        {
            LPM_SHADOW_VALIDATION_ERROR_MAC(returnOnFailure, retVal, "Level",
                                                rangePtr,
                                                "MC tree reached an illegal level");
        }
        else
        {
            if ((((protocol == PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E) && (level == PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_FOR_GON_IN_IPV4_PROTOCOL_CNS)) ||
                 ((protocol == PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E) && (level == PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_FOR_GON_IN_IPV6_PROTOCOL_CNS))) &&
                (prefixType == CPSS_UNICAST_E))
            {
                LPM_SHADOW_VALIDATION_ERROR_MAC(returnOnFailure, retVal, "Level",
                                                    rangePtr,
                                                    "UC tree reached an illegal level");
            }
        }

        /* Check validity for nexthop */
        if ((rangePtr->pointerType == CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E) ||
            (rangePtr->pointerType == CPSS_DXCH_LPM_MULTIPATH_ENTRY_PTR_TYPE_E))
        {
            status = prvCpssDxChLpmRamDbgRouteEntryShadowValidityCheckSip6(shadowPtr,
                                                                       rangePtr->lowerLpmPtr.nextHopEntry,
                                                                       returnOnFailure);
            if (status != GT_OK)
            {
                retVal = status;
                if (returnOnFailure == GT_TRUE)
                {
                    return retVal;
                }
            }
        }

        /* The range was possibly found in the trie. */
        trieRangeIdx = rangePtr->startAddr;
        if (validRangeInTrieArray[trieRangeIdx] == GT_TRUE)
        {
            if (rangePtr->mask != maskForRangeInTrieArray[trieRangeIdx])
            {
                LPM_SHADOW_VALIDATION_ERROR_MAC(returnOnFailure, retVal, "Range",
                                                rangePtr,
                                                "the range has a different mask in the trie");
            }
            validRangeInTrieArray[trieRangeIdx] = GT_FALSE;
        }
        numOfRanges++;
        numOfRangesForPrefixType++;
        rangePtr = nextRangePtr;
    }

    if (numOfRanges != bucketPtr->numOfRanges)
    {
        LPM_SHADOW_VALIDATION_ERROR_MAC(returnOnFailure, retVal, "Bucket",
                                        bucketPtr,
                                        "the number of actual ranges is different than the bucket's numOfRanges");
    }

    /* Check if all the ranges in the trie exist in the bucket ranges list */
    for (trieRangeIdx = 0; trieRangeIdx < 255; trieRangeIdx++)
    {
        if ((trieRangeIdx < prefixTypeFirstRange) || (trieRangeIdx > prefixTypeLastRange))
        {
            continue;
        }
        if (validRangeInTrieArray[trieRangeIdx] == GT_TRUE)
        {
            LPM_SHADOW_VALIDATION_ERROR_MAC(returnOnFailure, retVal, "Bucket",
                                            bucketPtr,
                                            "a range that appears in the trie does not appear in the ranges list");
        }
    }

    /* Check next buckets */
    rangePtr = bucketPtr->rangeList;

    while (rangePtr)
    {
        if ((rangePtr->pointerType != CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E) &&
            (rangePtr->pointerType != CPSS_DXCH_LPM_MULTIPATH_ENTRY_PTR_TYPE_E))
        {
            if (protocol == PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E)
            {
                if ((rangePtr->startAddr < prefixTypeFirstRange) ||
                    ((rangePtr->startAddr > prefixTypeLastRange) && (rangePtr->startAddr<prefixTypeSecondRange))||
                     (rangePtr->startAddr > prefixTypeSecondLastRange))
                {
                    rangePtr = rangePtr->next;
                    continue;
                }
            }
            else
            {
                if ((rangePtr->startAddr < prefixTypeFirstRange) ||
                    (rangePtr->startAddr > prefixTypeLastRange))
                {
                     rangePtr = rangePtr->next;
                     continue;
                }
            }
            /* SIP6 do not use this parameter, so set it to 0 */
            expectedPointingRangeMemAddr = 0;
            status = prvCpssDxChLpmRamDbgBucketShadowValidityCheckSip6(shadowPtr,
                                                                   vrId,
                                                                   rangePtr->lowerLpmPtr.nextBucket,
                                                                   (GT_U8)(level + 1),
                                                                   rangePtr->pointerType,
                                                                   numOfMaxAllowedLevels,
                                                                   prefixType,
                                                                   protocol,
                                                                   expectedPointingRangeMemAddr,
                                                                   returnOnFailure);
            if (status != GT_OK)
            {
                retVal = status;
                if (returnOnFailure == GT_TRUE)
                {
                    return retVal;
                }
            }
        }
        rangePtr = rangePtr->next;
    }
    return retVal;
}

/**
* @internal prvCpssDxChLpmRamDbgBucketShadowValidityCheck function
* @endinternal
*
* @brief   Shadow validation of a bucket
*
* @param[in] shadowPtr                - pointer to shadow structure
* @param[in] vrId                     - the virtual router id
* @param[in] bucketPtr                - pointer to the bucket
* @param[in] level                    - the  in the tree (first  is 0)
* @param[in] expectedNextPointerType  - the expected type of the bucket, ignored
*                                      for level 0
* @param[in] numOfMaxAllowedLevels    - the maximal allowed number of levels
*                                      for the relevant protocol and prefix type
* @param[in] prefixType               - unicast or multicast tree
* @param[in] protocol                 - protocol
* @param[in] expectedPointingRangeMemAddr - the expected pointingRangeMemAddr field
*                                      of the bucket
* @param[in] returnOnFailure          - GT_TRUE: the validation check stops on
*                                      first failure
*                                      GT_FALSE: continue with the test on
*                                      failure
*
* @retval GT_FAIL                  - on failure
* @retval GT_OK                    - on success
*/
static GT_STATUS prvCpssDxChLpmRamDbgBucketShadowValidityCheck
(
    IN  PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC          *shadowPtr,
    IN  GT_U32                                    vrId,
    IN  PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC   *bucketPtr,
    IN  GT_U8                                     level,
    IN  CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT       expectedNextPointerType,
    IN  GT_U8                                     numOfMaxAllowedLevels,
    IN  CPSS_UNICAST_MULTICAST_ENT                prefixType,
    IN  PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT      protocol,
    IN  GT_U32                                    expectedPointingRangeMemAddr,
    IN  GT_BOOL                                   returnOnFailure
)
{
    GT_U32    swapBaseAddr, secondSwapBaseAddr, memSize, baseAddrOfMemBlock;
    GT_U32    beginningOfBucket, endOfBucket;
    GT_STATUS status, retVal = GT_OK;

    /* the mask as retrieved from the trie. The array index is the range start
       address */
    GT_U8   maskForRangeInTrieArray[256] = {0};
    /* indicates whether the range was found during the trie scan. The array
       index is the range start address.
       For each range that is found during the trie scan, the value in the array
       will be updated to GT_TRUE. After that, we will go over the bucket ranges
       and check that all the ranges with GT_TRUE are found */
    GT_BOOL validRangeInTrieArray[256] = {0};

    /* make compiler silent */
    swapBaseAddr = 0;
    secondSwapBaseAddr = 0;

     /* if this is a SIP6 architecture the call specific sip6 function */
    if(shadowPtr->shadowType==PRV_CPSS_DXCH_LPM_RAM_SIP6_SHADOW_E)
    {
       retVal = prvCpssDxChLpmRamDbgBucketShadowValidityCheckSip6(shadowPtr,
                                                               vrId,
                                                               bucketPtr,
                                                               level,
                                                               expectedNextPointerType,
                                                               numOfMaxAllowedLevels,
                                                               prefixType,
                                                               protocol,
                                                               expectedPointingRangeMemAddr,
                                                               returnOnFailure);
        return retVal;
    }

    /* Check that we don't exceed the number of allowed levels */
    if (level >= numOfMaxAllowedLevels)
    {
        LPM_SHADOW_VALIDATION_ERROR_MAC(returnOnFailure, retVal, "Bucket",
                                        bucketPtr,
                                        "exceeded max number of levels");
    }

    /* Check that bucketType and pointerType of the pointing range are the same.
       Exception: bucketType of a MC source is different than pointerType of the
       pointing range */
    if ((expectedNextPointerType != bucketPtr->bucketType) &&
        ((GT_U32)expectedNextPointerType != (GT_U32)PRV_CPSS_DXCH_LPM_RAM_TRIE_PTR_TYPE_E))
    {
        LPM_SHADOW_VALIDATION_ERROR_MAC(returnOnFailure, retVal, "Bucket",
                                        bucketPtr,
                                        "bucket type is different than the pointing range type");
    }

    /* Check validity of bucketType and compatibility with number of ranges */
    switch (bucketPtr->bucketType)
    {
    case CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E:
        /* The root can't be compressed 2 (HW limitation) */
        if (level == 0)
        {
            if ((bucketPtr->numOfRanges < 6) || (bucketPtr->numOfRanges > 256))
            {
                LPM_SHADOW_VALIDATION_ERROR_MAC(returnOnFailure, retVal, "Bucket",
                                                bucketPtr,
                                                "bucket type is not compatible with the number of ranges");
            }
        }
        else
        {
            if ((bucketPtr->numOfRanges < 11) || (bucketPtr->numOfRanges > 256))
            {
                LPM_SHADOW_VALIDATION_ERROR_MAC(returnOnFailure, retVal, "Bucket",
                                                bucketPtr,
                                                "bucket type is not compatible with the number of ranges");
            }
        }
        break;

    case CPSS_DXCH_LPM_COMPRESSED_1_NODE_PTR_TYPE_E:
        if (level == 0)
        {
            /* The root bucket can contain one range */
            if ((bucketPtr->numOfRanges < 1) || (bucketPtr->numOfRanges > 5))
            {
                LPM_SHADOW_VALIDATION_ERROR_MAC(returnOnFailure, retVal, "Bucket",
                                                bucketPtr,
                                                "bucket type is not compatible with the number of ranges");
            }
        }
        else
        {
            if ((bucketPtr->numOfRanges < 2) || (bucketPtr->numOfRanges > 5))
            {
                LPM_SHADOW_VALIDATION_ERROR_MAC(returnOnFailure, retVal, "Bucket",
                                                bucketPtr,
                                                "bucket type is not compatible with the number of ranges");
            }
        }
        break;

    case CPSS_DXCH_LPM_COMPRESSED_2_NODE_PTR_TYPE_E:
        /* The root can't be compressed 2 (HW limitation) */
        if (level == 0)
        {
            LPM_SHADOW_VALIDATION_ERROR_MAC(returnOnFailure, retVal, "Bucket",
                                            bucketPtr,
                                            "root bucket type is compressed 2");
        }
        if ((bucketPtr->numOfRanges < 6) || (bucketPtr->numOfRanges > 10))
        {
            LPM_SHADOW_VALIDATION_ERROR_MAC(returnOnFailure, retVal, "Bucket",
                                            bucketPtr,
                                            "bucket type is not compatible with the number of ranges");
        }
        break;

    case CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E:
        if ((prefixType != CPSS_MULTICAST_E) ||
            ((GT_U32)expectedNextPointerType != (GT_U32)PRV_CPSS_DXCH_LPM_RAM_TRIE_PTR_TYPE_E))
        {
            LPM_SHADOW_VALIDATION_ERROR_MAC(returnOnFailure, retVal, "Bucket",
                                            bucketPtr,
                                            "invalid bucket type");
        }
        break;

    default:
        LPM_SHADOW_VALIDATION_ERROR_MAC(returnOnFailure, retVal, "Bucket",
                                        bucketPtr,
                                        "invalid bucket type");
    }

    if (bucketPtr->bucketType != CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E)
    {
        /* Check hwBucketOffsetHandle */
        if (prefixType == CPSS_UNICAST_E)
        {
            baseAddrOfMemBlock = shadowPtr->ucSearchMemArrayPtr[protocol][level]->structsBase;
        }
        else
        {
            baseAddrOfMemBlock = shadowPtr->mcSearchMemArrayPtr[protocol][level]->structsBase;
        }
        swapBaseAddr = PRV_CPSS_DXCH_LPM_RAM_GET_LPM_OFFSET_FROM_DMM_MAC(shadowPtr->swapMemoryAddr);
        secondSwapBaseAddr = PRV_CPSS_DXCH_LPM_RAM_GET_LPM_OFFSET_FROM_DMM_MAC(shadowPtr->secondSwapMemoryAddr);

        memSize = shadowPtr->lpmRamTotalBlocksSizeIncludingGap * PRV_CPSS_DXCH_LPM_RAM_NUM_OF_MEMORIES_FALCON_CNS;

        /* the beginning and the end of the bucket in HW */
        beginningOfBucket =
            PRV_CPSS_DXCH_LPM_RAM_GET_LPM_OFFSET_FROM_DMM_MAC(bucketPtr->hwBucketOffsetHandle);
        endOfBucket =
            beginningOfBucket +
            PRV_CPSS_DXCH_LPM_RAM_GET_LPM_SIZE_FROM_DMM_MAC(bucketPtr->hwBucketOffsetHandle);
        /* Check that hwBucketOffsetHandle is in range */
        if (endOfBucket > memSize)
        {
            LPM_SHADOW_VALIDATION_ERROR_MAC(returnOnFailure, retVal, "Bucket",
                                            bucketPtr,
                                            "hwBucketOffsetHandle is out of range");
        }
        /* Check that hwBucketOffsetHandle is not within the swapping areas */
        memSize = PRV_CPSS_DXCH_LPM_RAM_MAX_SIZE_OF_BUCKET_IN_LPM_LINES_CNS;
        beginningOfBucket += baseAddrOfMemBlock;
        endOfBucket += baseAddrOfMemBlock;
        if (((swapBaseAddr < beginningOfBucket) && (beginningOfBucket < swapBaseAddr + memSize)) ||
            ((swapBaseAddr < endOfBucket) && (endOfBucket < swapBaseAddr + memSize)) ||
            ((secondSwapBaseAddr < beginningOfBucket) && (beginningOfBucket < secondSwapBaseAddr + memSize)) ||
            ((secondSwapBaseAddr < endOfBucket) && (endOfBucket < secondSwapBaseAddr + memSize)))
        {
            LPM_SHADOW_VALIDATION_ERROR_MAC(returnOnFailure, retVal, "Bucket",
                                            bucketPtr,
                                            "hwBucketOffsetHandle is within the swapping area");
        }
    }
    /* Check pointingRangeMemAddr */
    if (bucketPtr->pointingRangeMemAddr != expectedPointingRangeMemAddr)
    {
        LPM_SHADOW_VALIDATION_ERROR_MAC(returnOnFailure, retVal, "Bucket",
                                        bucketPtr,
                                        "pointingRangeMemAddr is wrong");
    }
    /* Trie validity check. It also fills maskForRangeInTrieArray and
       validRangeInTrieArray. They will be used later to verify that all the
       ranges that can be retrieved from the trie exist in the bucket */
    status = prvCpssDxChLpmRamDbgTrieShadowValidityCheck(&bucketPtr->trieRoot,
                                                         0,
                                                         0,
                                                         0,
                                                         returnOnFailure,
                                                         maskForRangeInTrieArray,
                                                         validRangeInTrieArray);
    if (status != GT_OK)
    {
        retVal = status;
        if (returnOnFailure == GT_TRUE)
        {
            return retVal;
        }
    }

    if (bucketPtr->bucketType == CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E)
    {
        /* This is a root of MC source tree that points directly to a nexthop or
           ECMP/QoS entry */
        status = prvCpssDxChLpmRamDbgRouteEntryShadowValidityCheck(shadowPtr,
                                                                   bucketPtr->rangeList->lowerLpmPtr.nextHopEntry,
                                                                   returnOnFailure);
    }
    else
    {
        /* Ranges validity check */
        status = prvCpssDxChLpmRamDbgRangesShadowValidityCheck(shadowPtr,
                                                               vrId,
                                                               bucketPtr,
                                                               maskForRangeInTrieArray,
                                                               validRangeInTrieArray,
                                                               level,
                                                               numOfMaxAllowedLevels,
                                                               prefixType,
                                                               protocol,
                                                               returnOnFailure);
    }

    if (status != GT_OK)
    {
        retVal = status;
        if (returnOnFailure == GT_TRUE)
        {
            return retVal;
        }
    }
    return retVal;
}

/**
* @internal prvCpssDxChLpmRamDbgBucketShadowValidityCheckSip6 function
* @endinternal
*
* @brief   Shadow validation of a bucket
*
* @param[in] shadowPtr                - pointer to shadow structure
* @param[in] vrId                     - the virtual router id
* @param[in] bucketPtr                - pointer to the bucket
* @param[in] level                    - the  in the tree (first  is 0)
* @param[in] expectedNextPointerType  - the expected type of the bucket, ignored
*                                      for level 0
* @param[in] numOfMaxAllowedLevels    - the maximal allowed number of levels
*                                      for the relevant protocol and prefix type
* @param[in] prefixType               - unicast or multicast tree
* @param[in] protocol                 - protocol
* @param[in] expectedPointingRangeMemAddr - the expected pointingRangeMemAddr field
*                                      of the bucket
* @param[in] returnOnFailure          - GT_TRUE: the validation check stops on
*                                      first failure
*                                      GT_FALSE: continue with the test on
*                                      failure
*
* @retval GT_FAIL                  - on failure
* @retval GT_OK                    - on success
*/
static GT_STATUS prvCpssDxChLpmRamDbgBucketShadowValidityCheckSip6
(
    IN  PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC          *shadowPtr,
    IN  GT_U32                                    vrId,
    IN  PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC   *bucketPtr,
    IN  GT_U8                                     level,
    IN  CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT       expectedNextPointerType,
    IN  GT_U8                                     numOfMaxAllowedLevels,
    IN  CPSS_UNICAST_MULTICAST_ENT                prefixType,
    IN  PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT      protocol,
    IN  GT_U32                                    expectedPointingRangeMemAddr,
    IN  GT_BOOL                                   returnOnFailure
)
{
    GT_U32    swapBaseAddr, secondSwapBaseAddr, memSize, baseAddrOfMemBlock;
    GT_U32    beginningOfBucket, endOfBucket,j;
    GT_STATUS status, retVal = GT_OK;

    /* the mask as retrieved from the trie. The array index is the range start
       address */
    GT_U8   maskForRangeInTrieArray[256] = {0};
    /* indicates whether the range was found during the trie scan. The array
       index is the range start address.
       For each range that is found during the trie scan, the value in the array
       will be updated to GT_TRUE. After that, we will go over the bucket ranges
       and check that all the ranges with GT_TRUE are found */
    GT_BOOL validRangeInTrieArray[256] = {0};

    /* make compiler silent */
    swapBaseAddr = 0;
    secondSwapBaseAddr = 0;

    if(prefixType!=CPSS_UNICAST_E)
    {
        /* TBD - at the moment only UC prefixes are supported for SIP6 */
        return GT_OK;
    }

    /* Check that we don't exceed the number of allowed levels */
    if (level >= numOfMaxAllowedLevels)
    {
        LPM_SHADOW_VALIDATION_ERROR_MAC(returnOnFailure, retVal, "Bucket",
                                        bucketPtr,
                                        "exceeded max number of levels");
    }

    /* Check that bucketType and pointerType of the pointing range are the same.
       Exception: bucketType of a MC source is different than pointerType of the
       pointing range */
    if ((expectedNextPointerType != bucketPtr->bucketType) &&
        ((GT_U32)expectedNextPointerType != (GT_U32)PRV_CPSS_DXCH_LPM_RAM_TRIE_PTR_TYPE_E))
    {
        LPM_SHADOW_VALIDATION_ERROR_MAC(returnOnFailure, retVal, "Bucket",
                                        bucketPtr,
                                        "bucket type is different than the pointing range type");
    }

    /* Check validity of bucketType and compatibility with number of ranges */
    switch (bucketPtr->bucketType)
    {
    case CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E:
       /* the root must be regular */
        if (level == 0)
        {
            if ((protocol==PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E && bucketPtr->numOfRanges < 3)||
                (protocol==PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E && bucketPtr->numOfRanges < 2)||
                (bucketPtr->numOfRanges > 256))
            {
                LPM_SHADOW_VALIDATION_ERROR_MAC(returnOnFailure, retVal, "Bucket",
                                                bucketPtr,
                                                "bucket type is not compatible with the number of ranges");
            }
        }
        else
        {
            if ((bucketPtr->numOfRanges < 11) || (bucketPtr->numOfRanges > 256))
            {
                LPM_SHADOW_VALIDATION_ERROR_MAC(returnOnFailure, retVal, "Bucket",
                                                bucketPtr,
                                                "bucket type is not compatible with the number of ranges");
            }
        }
        break;

    case CPSS_DXCH_LPM_COMPRESSED_NODE_PTR_TYPE_E:
        if (level == 0)
        {
            LPM_SHADOW_VALIDATION_ERROR_MAC(returnOnFailure, retVal, "Bucket",
                                            bucketPtr,
                                            "root bucket type is compressed");
        }
        else
        {
            if ((bucketPtr->numOfRanges < 2) || (bucketPtr->numOfRanges > 10))
            {
                LPM_SHADOW_VALIDATION_ERROR_MAC(returnOnFailure, retVal, "Bucket",
                                                bucketPtr,
                                                "bucket type is not compatible with the number of ranges");
            }
        }
        break;

     case CPSS_DXCH_LPM_COMPRESSED_UP_TO_7_RANGES_1_LEAF_NODE_PTR_TYPE_E:
        /* The root must be regular */
        if (level == 0)
        {
            LPM_SHADOW_VALIDATION_ERROR_MAC(returnOnFailure, retVal, "Bucket",
                                            bucketPtr,
                                            "root bucket type is embedded_1");
        }
        if ((bucketPtr->numOfRanges < 2) || (bucketPtr->numOfRanges > 7))
        {
            LPM_SHADOW_VALIDATION_ERROR_MAC(returnOnFailure, retVal, "Bucket",
                                            bucketPtr,
                                            "bucket type is not compatible with the number of ranges");
        }
        break;

     case CPSS_DXCH_LPM_COMPRESSED_UP_TO_5_RANGES_2_LEAVES_NODE_PTR_TYPE_E:
        /* The root must be regular */
        if (level == 0)
        {
            LPM_SHADOW_VALIDATION_ERROR_MAC(returnOnFailure, retVal, "Bucket",
                                            bucketPtr,
                                            "root bucket type is embedded_2");
        }
        if ((bucketPtr->numOfRanges < 2) || (bucketPtr->numOfRanges > 5))
        {
            LPM_SHADOW_VALIDATION_ERROR_MAC(returnOnFailure, retVal, "Bucket",
                                            bucketPtr,
                                            "bucket type is not compatible with the number of ranges");
        }
        break;
     case CPSS_DXCH_LPM_COMPRESSED_3_RANGES_3_LEAVES_NODE_PTR_TYPE_E:
        /* The root must be regular */
        if (level == 0)
        {
            LPM_SHADOW_VALIDATION_ERROR_MAC(returnOnFailure, retVal, "Bucket",
                                            bucketPtr,
                                            "root bucket type is embedded_3");
        }
        if (bucketPtr->numOfRanges != 3)
        {
            LPM_SHADOW_VALIDATION_ERROR_MAC(returnOnFailure, retVal, "Bucket",
                                            bucketPtr,
                                            "bucket type is not compatible with the number of ranges");
        }
        break;

    case CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E:
        if ((prefixType != CPSS_MULTICAST_E) ||
            ((GT_U32)expectedNextPointerType != (GT_U32)PRV_CPSS_DXCH_LPM_RAM_TRIE_PTR_TYPE_E))
        {
            LPM_SHADOW_VALIDATION_ERROR_MAC(returnOnFailure, retVal, "Bucket",
                                            bucketPtr,
                                            "invalid bucket type");
        }
        break;
    default:
        LPM_SHADOW_VALIDATION_ERROR_MAC(returnOnFailure, retVal, "Bucket",
                                        bucketPtr,
                                        "invalid bucket type");
    }

    if (bucketPtr->bucketType != CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E)
    {
        /* Check hwBucketOffsetHandle */
        if (prefixType == CPSS_UNICAST_E)
        {
            baseAddrOfMemBlock = shadowPtr->ucSearchMemArrayPtr[protocol][level]->structsBase;
        }
        else
        {
            baseAddrOfMemBlock = shadowPtr->mcSearchMemArrayPtr[protocol][level]->structsBase;
        }
        swapBaseAddr = PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(shadowPtr->swapMemoryAddr);
        secondSwapBaseAddr = PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(shadowPtr->secondSwapMemoryAddr);

        for (j=0;j<PRV_CPSS_DXCH_LPM_RAM_FALCON_MAX_SIZE_OF_BUCKET_IN_LPM_LINES_CNS;j++)
        {
            memSize = shadowPtr->lpmRamTotalBlocksSizeIncludingGap * PRV_CPSS_DXCH_LPM_RAM_NUM_OF_MEMORIES_FALCON_CNS;

            /* in case of regular bit vector we will have 6 pointers (hwBucketOffsetHandle[6])*/
            if (bucketPtr->hwGroupOffsetHandle[j]!=0)
            {
                /* the beginning and the end of the bucket in HW */
                beginningOfBucket =
                    PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(bucketPtr->hwGroupOffsetHandle[j]);
                endOfBucket =
                    beginningOfBucket +
                    PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_SIZE_FROM_DMM_MAC(bucketPtr->hwGroupOffsetHandle[j]);
                /* Check that hwBucketOffsetHandle is in range */
                if (endOfBucket > memSize)
                {
                    LPM_SHADOW_VALIDATION_ERROR_MAC(returnOnFailure, retVal, "Bucket",
                                                    bucketPtr,
                                                    "hwBucketOffsetHandle is out of range");
                }
                /* Check that hwBucketOffsetHandle is not within the swapping areas */
                memSize = PRV_CPSS_DXCH_LPM_RAM_FALCON_MAX_GROUP_OF_NODES_SIZE_IN_LPM_LINES_CNS;
                beginningOfBucket += baseAddrOfMemBlock;
                endOfBucket += baseAddrOfMemBlock;
                if (((swapBaseAddr < beginningOfBucket) && (beginningOfBucket < swapBaseAddr + memSize)) ||
                    ((swapBaseAddr < endOfBucket) && (endOfBucket < swapBaseAddr + memSize)) ||
                    ((secondSwapBaseAddr < beginningOfBucket) && (beginningOfBucket < secondSwapBaseAddr + memSize)) ||
                    ((secondSwapBaseAddr < endOfBucket) && (endOfBucket < secondSwapBaseAddr + memSize)))
                {
                    LPM_SHADOW_VALIDATION_ERROR_MAC(returnOnFailure, retVal, "Bucket",
                                                    bucketPtr,
                                                    "hwBucketOffsetHandle is within the swapping area");
                }
            }
        }
    }

    /* to avoid warnings */
    expectedPointingRangeMemAddr=expectedPointingRangeMemAddr;
#if 0
/* currently pointingRangeMemAddr is always 0 for SIP6 - nothing to check
   maybe in the future when using bulk */
    /* Check pointingRangeMemAddr */
    if (bucketPtr->pointingRangeMemAddr != expectedPointingRangeMemAddr)
    {
        LPM_SHADOW_VALIDATION_ERROR_MAC(returnOnFailure, retVal, "Bucket",
                                        bucketPtr,
                                        "pointingRangeMemAddr is wrong");
    }
#endif
    /* Trie validity check. It also fills maskForRangeInTrieArray and
       validRangeInTrieArray. They will be used later to verify that all the
       ranges that can be retrieved from the trie exist in the bucket */
    status = prvCpssDxChLpmRamDbgTrieShadowValidityCheckSip6(&bucketPtr->trieRoot,
                                                         0,
                                                         0,
                                                         0,
                                                         returnOnFailure,
                                                         maskForRangeInTrieArray,
                                                         validRangeInTrieArray);
    if (status != GT_OK)
    {
        retVal = status;
        if (returnOnFailure == GT_TRUE)
        {
            return retVal;
        }
    }

    if (bucketPtr->bucketType == CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E)
    {
        /* This is a root of MC source tree that points directly to a nexthop or
           ECMP/QoS entry */
        status = prvCpssDxChLpmRamDbgRouteEntryShadowValidityCheckSip6(shadowPtr,
                                                                   bucketPtr->rangeList->lowerLpmPtr.nextHopEntry,
                                                                   returnOnFailure);
    }
    else
    {
        /* Ranges validity check */
        status = prvCpssDxChLpmRamDbgRangesShadowValidityCheckSip6(shadowPtr,
                                                               vrId,
                                                               bucketPtr,
                                                               maskForRangeInTrieArray,
                                                               validRangeInTrieArray,
                                                               level,
                                                               numOfMaxAllowedLevels,
                                                               prefixType,
                                                               protocol,
                                                               returnOnFailure);
    }

    if (status != GT_OK)
    {
        retVal = status;
        if (returnOnFailure == GT_TRUE)
        {
            return retVal;
        }
    }
    return retVal;
}

/**
* @internal prvCpssDxChLpmRamDbgShadowValidityCheck function
* @endinternal
*
* @brief   Validation function for the LPM shadow
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
*
* @param[in] lpmDbPtr                 - LPM DB
* @param[in] vrId                     - virtual router id, 4096 means "all vrIds"
* @param[in] protocolBitmap           - protocols bitmap
* @param[in] prefixType               - UC/MC/both prefix type
* @param[in] returnOnFailure          - GT_TRUE: the validation check stops at first failure
*                                      GT_FALSE: continue with the test on failure
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on illegal input parameter/s
* @retval GT_NOT_FOUND             - LPM DB was not found
* @retval GT_NOT_INITIALIZED       - LPM DB is not initialized
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmRamDbgShadowValidityCheck
(
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC     *lpmDbPtr,
    IN GT_U32                                   vrId,
    IN PRV_CPSS_DXCH_LPM_PROTOCOL_BMP           protocolBitmap,
    IN CPSS_UNICAST_MULTICAST_ENT               prefixType,
    IN GT_BOOL                                  returnOnFailure
)
{
    PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC  *bucketPtr;
    CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT      bucketType;
    GT_U32                                   shadowIdx, vrIdStartIdx, vrIdEndIdx;
    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC         *shadowPtr;
    GT_U8                                    numOfMaxAllowedLevels;
    GT_STATUS                                status, rc = GT_OK;
    GT_U32                                   blockIndex=0,numOfOctetsInProtocol=0,octetIndex=0;
    PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT     protocolChecked;
    GT_BOOL                                  blockIsUsedByAnOctetOfThisProtocol;

    switch(prefixType)
    {
        case CPSS_UNICAST_E:
        case CPSS_MULTICAST_E:
        case CPSS_UNICAST_MULTICAST_E:
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if ((PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_STATUS_GET_MAC(protocolBitmap) == GT_FALSE) &&
        (PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_STATUS_GET_MAC(protocolBitmap) == GT_FALSE) &&
        (PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_STATUS_GET_MAC(protocolBitmap) == GT_FALSE))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* FCoE is unicast only */
    if ((PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_STATUS_GET_MAC(protocolBitmap)) &&
         (prefixType != CPSS_UNICAST_E))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* if initialization has not been done for the requested protocol stack -
       return error */
    if ((PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_STATUS_GET_MAC(protocolBitmap)) &&
        (PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_STATUS_GET_MAC(lpmDbPtr->protocolBitmap) == GT_FALSE))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }
    else if ((PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_STATUS_GET_MAC(protocolBitmap)) &&
             (PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_STATUS_GET_MAC(lpmDbPtr->protocolBitmap) == GT_FALSE))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }
    else if ((PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_STATUS_GET_MAC(protocolBitmap)) &&
             (PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_STATUS_GET_MAC(lpmDbPtr->protocolBitmap) == GT_FALSE))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    for (shadowIdx = 0; shadowIdx < lpmDbPtr->numOfShadowCfg; shadowIdx++)
    {
        shadowPtr = &lpmDbPtr->shadowArray[shadowIdx];
        if ((vrId >= shadowPtr->vrfTblSize) && (vrId != 4096))
        {
            /* Illegal virtual router id */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        if (PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_STATUS_GET_MAC(protocolBitmap))
        {
            if (shadowPtr->isProtocolInitialized[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E] == GT_FALSE)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
            }
        }
        if (PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_STATUS_GET_MAC(protocolBitmap))
        {
            if (shadowPtr->isProtocolInitialized[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E] == GT_FALSE)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
            }
        }
        if (PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_STATUS_GET_MAC(protocolBitmap))
        {
            if (shadowPtr->isProtocolInitialized[PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E] == GT_FALSE)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
            }
        }

        /* check if the struct holding block to octet mapping fit the sharing mode */
        if(shadowPtr->lpmRamBlocksAllocationMethod==PRV_CPSS_DXCH_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITHOUT_BLOCK_SHARING_E)
        {
            for (blockIndex=0;blockIndex<shadowPtr->numOfLpmMemories;blockIndex++)
            {
                /* the block is not used at all, so no need to check sharing violation */
                if (shadowPtr->lpmRamOctetsToBlockMappingPtr[blockIndex].isBlockUsed==GT_FALSE)
                {
                    continue;
                }

                for (protocolChecked=PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E;
                     protocolChecked<PRV_CPSS_DXCH_LPM_PROTOCOL_LAST_E;
                     protocolChecked++)
                {
                    switch (protocolChecked)
                    {
                        case PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E:
                            if (PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_STATUS_GET_MAC(protocolBitmap)==GT_FALSE)
                                continue;
                            numOfOctetsInProtocol = PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS;
                            break;
                        case PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E:
                            if (PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_STATUS_GET_MAC(protocolBitmap)==GT_FALSE)
                                continue;
                            numOfOctetsInProtocol = PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS;
                            break;
                        case PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E:
                            if (PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_STATUS_GET_MAC(protocolBitmap)==GT_FALSE)
                                continue;
                            numOfOctetsInProtocol = PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_FCOE_PROTOCOL_CNS;
                            break;
                        default:
                            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_VALUE, LOG_ERROR_NO_MSG);
                    }

                    /* the block is not used by the protocol,so no need to check sharing violation */
                    if (PRV_CPSS_DXCH_LPM_RAM_IS_BLOCK_USED_BY_PROTOCOL_GET_MAC(shadowPtr,protocolChecked,blockIndex)==GT_FALSE)
                    {
                        continue;
                    }
                    blockIsUsedByAnOctetOfThisProtocol=GT_FALSE;
                    for (octetIndex = 0; octetIndex < numOfOctetsInProtocol; octetIndex++)
                    {
                        if (PRV_CPSS_DXCH_LPM_RAM_IS_BLOCK_USED_BY_OCTET_IN_PROTOCOL_GET_MAC(shadowPtr,protocolChecked,octetIndex,blockIndex)==GT_TRUE)
                        {
                            if (blockIsUsedByAnOctetOfThisProtocol==GT_FALSE)
                            {
                                blockIsUsedByAnOctetOfThisProtocol=GT_TRUE;
                            }
                            else
                            {
                                /* if blockIsUsedByAnOctetOfThisProtocol is already true
                                   it means we have more than one bit up in the bitmap,
                                   meaning more than one octet sharing this block */
                                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
                            }
                        }
                    }
                }
            }
        }

        if (vrId == 4096)
        {
            vrIdStartIdx = 0;
            vrIdEndIdx = shadowPtr->vrfTblSize - 1;
        }
        else
        {
            vrIdStartIdx = vrIdEndIdx = vrId;
        }

        for (vrId = vrIdStartIdx; vrId <= vrIdEndIdx; vrId++)
        {
            if (shadowPtr->vrRootBucketArray[vrId].valid == 0)
            {
                continue;
            }

            if ((prefixType == CPSS_UNICAST_E) || (prefixType == CPSS_UNICAST_MULTICAST_E))
            {
                if (PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_STATUS_GET_MAC(protocolBitmap))
                {
                    bucketPtr = shadowPtr->vrRootBucketArray[vrId].rootBucket[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E];
                    bucketType = shadowPtr->vrRootBucketArray[vrId].rootBucketType[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E];
                    numOfMaxAllowedLevels = PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS;
                    status = prvCpssDxChLpmRamDbgBucketShadowValidityCheck(shadowPtr,
                                                                           vrId,
                                                                           bucketPtr,
                                                                           0,
                                                                           bucketType,
                                                                           numOfMaxAllowedLevels,
                                                                           CPSS_UNICAST_E,
                                                                           PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E,
                                                                           0,
                                                                           returnOnFailure);
                    if (status != GT_OK)
                    {
                        rc = status;
                        if (returnOnFailure == GT_TRUE)
                        {
                            return rc;
                        }
                    }
                }
                if (PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_STATUS_GET_MAC(protocolBitmap))
                {
                    bucketPtr = shadowPtr->vrRootBucketArray[vrId].rootBucket[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E];
                    bucketType = shadowPtr->vrRootBucketArray[vrId].rootBucketType[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E];
                    numOfMaxAllowedLevels = PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS;
                    status = prvCpssDxChLpmRamDbgBucketShadowValidityCheck(shadowPtr,
                                                                           vrId,
                                                                           bucketPtr,
                                                                           0,
                                                                           bucketType,
                                                                           numOfMaxAllowedLevels,
                                                                           CPSS_UNICAST_E,
                                                                           PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E,
                                                                           0,
                                                                           returnOnFailure);
                    if (status != GT_OK)
                    {
                        rc = status;
                        if (returnOnFailure == GT_TRUE)
                        {
                            return rc;
                        }
                    }
                }
                if (PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_STATUS_GET_MAC(protocolBitmap)&&
                   (shadowPtr->vrRootBucketArray[vrId].rootBucket[PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E]!=NULL))
                {
                    bucketPtr = shadowPtr->vrRootBucketArray[vrId].rootBucket[PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E];
                    bucketType = shadowPtr->vrRootBucketArray[vrId].rootBucketType[PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E];
                    numOfMaxAllowedLevels = PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_FCOE_PROTOCOL_CNS;
                    status = prvCpssDxChLpmRamDbgBucketShadowValidityCheck(shadowPtr,
                                                                           vrId,
                                                                           bucketPtr,
                                                                           0,
                                                                           bucketType,
                                                                           numOfMaxAllowedLevels,
                                                                           CPSS_UNICAST_E,
                                                                           PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E,
                                                                           0,
                                                                           returnOnFailure);
                    if (status != GT_OK)
                    {
                        rc = status;
                        if (returnOnFailure == GT_TRUE)
                        {
                            return rc;
                        }
                    }
                }
            }
            if ((prefixType == CPSS_MULTICAST_E) || (prefixType == CPSS_UNICAST_MULTICAST_E))
            {
                if (PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_STATUS_GET_MAC(protocolBitmap))
                {
                    bucketPtr = shadowPtr->vrRootBucketArray[vrId].rootBucket[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E];
                    bucketType = shadowPtr->vrRootBucketArray[vrId].rootBucketType[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E];
                    numOfMaxAllowedLevels = 8;
                    status = prvCpssDxChLpmRamDbgBucketShadowValidityCheck(shadowPtr,
                                                                           vrId,
                                                                           bucketPtr,
                                                                           0,
                                                                           bucketType,
                                                                           numOfMaxAllowedLevels,
                                                                           CPSS_MULTICAST_E,
                                                                           PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E,
                                                                           0,
                                                                           returnOnFailure);
                    if (status != GT_OK)
                    {
                        rc = status;
                        if (returnOnFailure == GT_TRUE)
                        {
                            return rc;
                        }
                    }
                }
                if (PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_STATUS_GET_MAC(protocolBitmap))
                {
                    bucketPtr = shadowPtr->vrRootBucketArray[vrId].rootBucket[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E];
                    bucketType = shadowPtr->vrRootBucketArray[vrId].rootBucketType[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E];
                    numOfMaxAllowedLevels =  (2 * PRV_CPSS_DXCH_LPM_MAX_NUM_OF_OCTETS_FOR_ADDRESS_CNS);
                    status = prvCpssDxChLpmRamDbgBucketShadowValidityCheck(shadowPtr,
                                                                           vrId,
                                                                           bucketPtr,
                                                                           0,
                                                                           bucketType,
                                                                           numOfMaxAllowedLevels,
                                                                           CPSS_MULTICAST_E,
                                                                           PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E,
                                                                           0,
                                                                           returnOnFailure);
                    if (status != GT_OK)
                    {
                        rc = status;
                        if (returnOnFailure == GT_TRUE)
                        {
                            return rc;
                        }
                    }
                }
            }
        }
    }
    return rc;
}

/**
* @internal prvCpssDxChLpmRamDbgRangesHwShadowSyncValidityCheck function
* @endinternal
*
* @brief   Shadow and HW synchronization validation of bucket's ranges
*
* @param[in] devNum                   - The device number
* @param[in] shadowPtr                - pointer to shadow structure
* @param[in] vrId                     - the virtual router id
* @param[in] bucketPtr                - pointer to the bucket
*                                      maskForRangeInTrieArray   - array of the ranges masks as found in the
*                                      trie. The array index represents the range
*                                      start address
*                                      validRangeInTrieArray     - array to indicate if a range was found in
*                                      the trie. The array index represents the
*                                      range start address.
* @param[in] level                    - the  in the tree (first  is 0)
* @param[in] numOfMaxAllowedLevels    - the maximal number of levels that is allowed
*                                      for the relevant protocol and prefix type
* @param[in] prefixType               - unicast or multicast tree
* @param[in] protocol                 - protocol
* @param[in] returnOnFailure          - GT_TRUE: the validation check stops on
*                                      first failure
*                                      GT_FALSE: continue with the test on failure
*
* @retval GT_FAIL                  - on failure
* @retval GT_OK                    - on success
*/
static GT_STATUS prvCpssDxChLpmRamDbgRangesHwShadowSyncValidityCheck
(
    IN  GT_U8                                     devNum,
    IN  PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC          *shadowPtr,
    IN  GT_U32                                    vrId,
    IN  PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC   *bucketPtr,
    IN  GT_U8                                     level,
    IN  GT_U8                                     numOfMaxAllowedLevels,
    IN  CPSS_UNICAST_MULTICAST_ENT                prefixType,
    IN  PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT      protocol,
    IN  GT_BOOL                                   returnOnFailure
)
{
    PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC  *rangePtr;
    GT_U32                                  bucketBaseAddr, rangeOffset,
                                            expectedPointingRangeMemAddr;
    GT_U8                                   prefixTypeFirstRange, prefixTypeLastRange;
    GT_U32                                  tmpPrefixTypeRange;
    GT_STATUS                               status, retVal = GT_OK;

    rangePtr = bucketPtr->rangeList;
    if (level == 0)
    {
        if (prefixType == CPSS_UNICAST_E)
        {
            prefixTypeFirstRange = 0;
            tmpPrefixTypeRange = (protocol == PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E) ?
                (PRV_CPSS_DXCH_LPM_RAM_START_OF_IPV4_MC_ADDRESS_SPACE_CNS - 1) :
                (PRV_CPSS_DXCH_LPM_RAM_START_OF_IPV6_MC_ADDRESS_SPACE_CNS - 1);
            prefixTypeLastRange = (GT_U8)tmpPrefixTypeRange;
        }
        else /* CPSS_MULTICAST_E */
        {
            tmpPrefixTypeRange = (protocol == PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E) ?
                PRV_CPSS_DXCH_LPM_RAM_START_OF_IPV4_MC_ADDRESS_SPACE_CNS :
                PRV_CPSS_DXCH_LPM_RAM_START_OF_IPV6_MC_ADDRESS_SPACE_CNS;
            prefixTypeFirstRange = (GT_U8)tmpPrefixTypeRange;
            tmpPrefixTypeRange = (protocol == PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E) ?
                PRV_CPSS_DXCH_LPM_RAM_END_OF_IPV4_MC_ADDRESS_SPACE_CNS :
                PRV_CPSS_DXCH_LPM_RAM_END_OF_IPV6_MC_ADDRESS_SPACE_CNS;
            prefixTypeLastRange = (GT_U8)tmpPrefixTypeRange;
        }
    }
    else
    {
        prefixTypeFirstRange = 0;
        prefixTypeLastRange = 255;
    }

    /* Check next buckets */
    rangePtr = bucketPtr->rangeList;
    if (prefixType == CPSS_UNICAST_E)
    {
        bucketBaseAddr =
            shadowPtr->ucSearchMemArrayPtr[protocol][level]->structsBase +
            PRV_CPSS_DXCH_LPM_RAM_GET_LPM_OFFSET_FROM_DMM_MAC(bucketPtr->hwBucketOffsetHandle);
    }
    else
    {
        bucketBaseAddr =
            shadowPtr->mcSearchMemArrayPtr[protocol][level]->structsBase +
            PRV_CPSS_DXCH_LPM_RAM_GET_LPM_OFFSET_FROM_DMM_MAC(bucketPtr->hwBucketOffsetHandle);
    }

    /* number of LPM lines for the bitmap */
    if (bucketPtr->bucketType == CPSS_DXCH_LPM_COMPRESSED_1_NODE_PTR_TYPE_E)
    {
        rangeOffset = PRV_CPSS_DXCH_LPM_RAM_BUCKET_BIT_VEC_SIZE_COMPRESSED_1_CNS;
    }
    else if (bucketPtr->bucketType == CPSS_DXCH_LPM_COMPRESSED_2_NODE_PTR_TYPE_E)
    {
        rangeOffset = PRV_CPSS_DXCH_LPM_RAM_BUCKET_BIT_VEC_SIZE_COMPRESSED_2_CNS;
    }
    else    /* CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E */
    {
        rangeOffset = PRV_CPSS_DXCH_LPM_RAM_BUCKET_BIT_VEC_SIZE_REGULAR_CNS;
    }

    while (rangePtr)
    {
        if ((rangePtr->pointerType != CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E) &&
            (rangePtr->pointerType != CPSS_DXCH_LPM_ECMP_ENTRY_PTR_TYPE_E) &&
            (rangePtr->pointerType != CPSS_DXCH_LPM_QOS_ENTRY_PTR_TYPE_E) &&
            (!(((GT_U32)rangePtr->pointerType==(GT_U32)PRV_CPSS_DXCH_LPM_RAM_TRIE_PTR_TYPE_E) &&
             (rangePtr->lowerLpmPtr.nextBucket->bucketType==CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E))))
        {
            if ((rangePtr->startAddr < prefixTypeFirstRange) ||
                (rangePtr->startAddr > prefixTypeLastRange))
            {
                rangePtr = rangePtr->next;
                rangeOffset++;
                continue;
            }
            expectedPointingRangeMemAddr = bucketBaseAddr + rangeOffset;
            status = prvCpssDxChLpmRamDbgBucketHwShadowSyncValidityCheck(devNum,
                                                                 shadowPtr,
                                                                 vrId,
                                                                 rangePtr->lowerLpmPtr.nextBucket,
                                                                 (GT_U8)(level + 1),
                                                                 rangePtr->pointerType,
                                                                 numOfMaxAllowedLevels,
                                                                 prefixType,
                                                                 protocol,
                                                                 expectedPointingRangeMemAddr,
                                                                 returnOnFailure,
                                                                 GT_FALSE/*not the root bucket*/);
            if (status != GT_OK)
            {
                retVal = status;
                if (returnOnFailure == GT_TRUE)
                {
                    return retVal;
                }
            }
        }
        else
        {
            /* in case of NextHop pointers no need to do anything since
               the SW and HW NextHop synchronization was checked in
               prvCpssDxChLpmRamDbgBucketHwShadowSyncValidityCheck */
        }
        rangePtr = rangePtr->next;
        rangeOffset++;
    }

    return retVal;
}

/**
* @internal
*           prvCpssDxChLpmRamDbgBucketShadowGetNumOfTotalRangesHiddenIncluddedSip6
*           function
* @endinternal
*
* @brief  function that goes over the range list and count
*         hidden ranges. relevant for regular buckets
*
* @param[in] bucketPtr            - pointer to the bucket
*
* @param[out] totalNumOfRangesPtr  - pointer to all
*                             ranges includded the hidden ones
*            totalRangesPtr       - pointer to the ranges values
*            totalRangesTypesPtr  - pointer to the ranges types
*            totalRangesIsHiddenPtr - pointer to flags for
*                                     hidden ranges
*
*
* @retval GT_FAIL                  - on failure
* @retval GT_OK                    - on success
*/
static GT_STATUS prvCpssDxChLpmRamDbgBucketShadowGetNumOfTotalRangesHiddenIncluddedSip6
(
    IN  PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC   *bucketPtr,
    OUT GT_U32                                    *totalNumOfRangesPtr,
    OUT GT_U32                                    *totalRangesPtr,
    OUT GT_U32                                    *totalRangesTypesPtr,
    OUT GT_BOOL                                   *totalRangesIsHiddenPtr
)
{
    GT_U32 i=0;
    GT_U32 numberOfGons=0;
    GT_U32 endOfRange=0,startOfRange=0;
    GT_BOOL hiddenRangeWasCounted=GT_FALSE;
    PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC *tempRangePtr;
    GT_U32 numOfRanges=0;

    for(i=0;i<PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS; i++)
    {
        if(bucketPtr->hwGroupOffsetHandle[i]!=0)
            numberOfGons++;
    }

    if ((numberOfGons!=0) && (numberOfGons!=1) && (numberOfGons!=6))
    {
         /* the only 3 leggal options for the array are:
            1. 6 leggal pointers, meaning a regular bucket
            2. single pointer , meaning a compressed bucket
            3. null pointer, meaning last octet bucket is all embedded
           */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    if(numberOfGons==0)
    {
        /* last octet treated */
        switch(bucketPtr->bucketType)
        {
        case CPSS_DXCH_LPM_COMPRESSED_UP_TO_7_RANGES_1_LEAF_NODE_PTR_TYPE_E:
        case CPSS_DXCH_LPM_COMPRESSED_UP_TO_5_RANGES_2_LEAVES_NODE_PTR_TYPE_E:
        case CPSS_DXCH_LPM_COMPRESSED_3_RANGES_3_LEAVES_NODE_PTR_TYPE_E:
            numberOfGons = 1;/* change value inorder to enter the loop */
            break;
        case CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E:
        case CPSS_DXCH_LPM_COMPRESSED_NODE_PTR_TYPE_E:
        case CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E:
        case CPSS_DXCH_LPM_MULTIPATH_ENTRY_PTR_TYPE_E:
        default:
            /* if all bucketPtr->hwGroupOffsetHandle[i] pointers are 0,
               it means that we have an all embedded bucket, all other options are illegal */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }
    }

    tempRangePtr = bucketPtr->rangeList;
    numOfRanges=0;

    for(i=0;i<numberOfGons; i++)
    {
        /* in case of regular node, if we reach here before we went over all the GONs
           then we need to count all the hidden ranges that were not counted */
        if(tempRangePtr == NULL)
        {
            totalRangesPtr[numOfRanges]=(i*44);/* the hidden range */
            totalRangesTypesPtr[numOfRanges]=CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E;
            totalRangesIsHiddenPtr[numOfRanges]=GT_TRUE;
            numOfRanges++;
            continue;
        }

        hiddenRangeWasCounted=0;

        startOfRange = (numberOfGons==1)? 0 : (i*44);
        endOfRange = (numberOfGons==1)? 255 : (i*44+43);
        /* go over the range list until we get to startOfRange, and check number of lines in the GON.
           regular hold 6 lines
           compressed hold 1 line
           and 5 leafs hold 1 line */
        while (tempRangePtr != NULL)
        {
            if(tempRangePtr->startAddr >= startOfRange &&
               tempRangePtr->startAddr <= endOfRange)
            {
                if (tempRangePtr->startAddr == startOfRange)
                {
                    if(hiddenRangeWasCounted==GT_FALSE)
                    {
                        hiddenRangeWasCounted=GT_TRUE;
                    }
                }
                else
                {
                    /* if the startAddr is bigger then the startOfRange then
                       it means the middle ranges of the regular bit vector
                       is hidden and we should count it */
                    if(hiddenRangeWasCounted==GT_FALSE)
                    {
                        hiddenRangeWasCounted=GT_TRUE;
                        totalRangesPtr[numOfRanges]=startOfRange;
                        totalRangesTypesPtr[numOfRanges]=CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E;
                        totalRangesIsHiddenPtr[numOfRanges]=GT_TRUE;
                        numOfRanges++;
                    }
                }

                totalRangesPtr[numOfRanges]=tempRangePtr->startAddr;
                totalRangesTypesPtr[numOfRanges]=tempRangePtr->pointerType;
                tempRangePtr = tempRangePtr->next;
                totalRangesIsHiddenPtr[numOfRanges]=GT_FALSE;
                numOfRanges++;
            }
            else
            {   /* if the startAddr is bigger then the startOfRange then
                   it means the middle ranges of the regular bit vector
                   is hidden and we should count it as a leaf*/
                if(hiddenRangeWasCounted==GT_FALSE)
                {
                    hiddenRangeWasCounted=GT_TRUE;
                    totalRangesPtr[numOfRanges]=startOfRange;
                    totalRangesTypesPtr[numOfRanges]=CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E;
                    totalRangesIsHiddenPtr[numOfRanges]=GT_TRUE;
                    numOfRanges++;
                }

                /* need to finish the current GON calculation and move to the next one
                in case of regular bucket, no need to go to next range- we still work on the same range  */
                break;
            }
        }
    }

    *totalNumOfRangesPtr = numOfRanges;
    return GT_OK;
}

/**
* @internal
*           prvCpssDxChLpmRamDbgRangesHwShadowSyncValidityCheckSip6
*           function
* @endinternal
*
* @brief   Shadow and HW synchronization validation of bucket's ranges
*
* @param[in] devNum                   - The device number
* @param[in] shadowPtr                - pointer to shadow structure
* @param[in] vrId                     - the virtual router id
* @param[in] bucketPtr                - pointer to the bucket
*                                      maskForRangeInTrieArray   - array of the ranges masks as found in the
*                                      trie. The array index represents the range
*                                      start address
*                                      validRangeInTrieArray     - array to indicate if a range was found in
*                                      the trie. The array index represents the
*                                      range start address.
* @param[in]nodeTotalChildTypesArr[PRV_CPSS_DXCH_LPM_RAM_FALCON_MAX_SIZE_OF_BUCKET_IN_LPM_LINES_CNS][CPSS_DXCH_LPM_MAX_CHILD_TYPE_CNS];
*                                   6 elements in a regular node
*                                   3 types of child that can be
*                                   for each range
*                                   (leaf,regular,compress)
* @param[in] level                    - the  in the tree (first  is 0)
* @param[in] numOfMaxAllowedLevels    - the maximal number of levels that is allowed
*                                      for the relevant protocol and prefix type
* @param[in] prefixType               - unicast or multicast tree
* @param[in] protocol                 - protocol
* @param[in] returnOnFailure          - GT_TRUE: the validation check stops on
*                                      first failure
*                                      GT_FALSE: continue with the test on failure
*
* @retval GT_FAIL                  - on failure
* @retval GT_OK                    - on success
*/
static GT_STATUS prvCpssDxChLpmRamDbgRangesHwShadowSyncValidityCheckSip6
(
    IN  GT_U8                                     devNum,
    IN  PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC          *shadowPtr,
    IN  GT_U32                                    vrId,
    IN  PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC   *bucketPtr,
    IN  GT_U32                                    nodeTotalChildTypesArr[6][3],
    IN  GT_U8                                     level,
    IN  GT_U8                                     numOfMaxAllowedLevels,
    IN  CPSS_UNICAST_MULTICAST_ENT                prefixType,
    IN  PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT      protocol,
    IN  GT_BOOL                                   returnOnFailure
)
{
    PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC  *rangePtr;
    GT_U8                                   prefixTypeFirstRange, prefixTypeLastRange;
    GT_U8                                   prefixTypeSecondRange=0, prefixTypeSecondLastRange=0;
    GT_U32                                  tmpPrefixTypeRange;
    GT_STATUS                               status, retVal = GT_OK;
    GT_U32    hwBucketDataArr[PRV_CPSS_DXCH_LPM_RAM_FALCON_MAX_SIZE_OF_BUCKET_IN_LPM_LINES_CNS*PRV_CPSS_DXCH_LPM_RAM_FALCON_SIZE_OF_LPM_ENTRY_IN_WORDS_CNS];
    GT_U32    gonNodeSize=0;/* the node size can be 6 for regular or 1 for compress */
    GT_U32    gonPointerIndex=0;/* can be 0 for compress or 0-5 for regular, this is the pointer to the GON */
    GT_U32    tempGonPointerIndex=0;/* can be 0 for compress or 0-5 for regular, this is the pointer to the GON */
    GT_U32    gonNodeAddr=0; /* Node address to read the HW data from */
    GT_U32    hwNodeAddr = 0;/* base node address to read the HW data from */
    GT_U32    rangeType;/* CPSS_DXCH_LPM_CHILD_TYPE_ENT */
    GT_U32    totalChildTypesUntilNow=0;
    GT_U32    totalCompressedChilds=0;
    GT_U32    totalRegularChilds=0;

    cpssOsMemSet(&hwBucketDataArr[0], 0, sizeof(hwBucketDataArr));

    if (level == 0)
    {
        if (prefixType == CPSS_UNICAST_E)
        {
            prefixTypeFirstRange = 0;
            tmpPrefixTypeRange = (protocol == PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E) ?
                (PRV_CPSS_DXCH_LPM_RAM_START_OF_IPV4_MC_ADDRESS_SPACE_CNS - 1) :
                (PRV_CPSS_DXCH_LPM_RAM_START_OF_IPV6_MC_ADDRESS_SPACE_CNS - 1);
            prefixTypeLastRange = (GT_U8)tmpPrefixTypeRange;

            if (protocol == PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E)
            {
                prefixTypeSecondRange = PRV_CPSS_DXCH_LPM_RAM_START_OF_IPV4_RESERVED_SPACE_ADDRESS_SPACE_CNS;
                prefixTypeSecondLastRange = PRV_CPSS_DXCH_LPM_RAM_END_OF_IPV4_RESERVED_SPACE_ADDRESS_SPACE_CNS;
            }
        }
        else /* CPSS_MULTICAST_E */
        {
            tmpPrefixTypeRange = (protocol == PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E) ?
                PRV_CPSS_DXCH_LPM_RAM_START_OF_IPV4_MC_ADDRESS_SPACE_CNS :
                PRV_CPSS_DXCH_LPM_RAM_START_OF_IPV6_MC_ADDRESS_SPACE_CNS;
            prefixTypeFirstRange = (GT_U8)tmpPrefixTypeRange;
            tmpPrefixTypeRange = (protocol == PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E) ?
                PRV_CPSS_DXCH_LPM_RAM_END_OF_IPV4_MC_ADDRESS_SPACE_CNS :
                PRV_CPSS_DXCH_LPM_RAM_END_OF_IPV6_MC_ADDRESS_SPACE_CNS;
            prefixTypeLastRange = (GT_U8)tmpPrefixTypeRange;
        }
    }
    else
    {
        prefixTypeFirstRange = 0;
        prefixTypeLastRange = 255;
        prefixTypeSecondRange = 0;
        prefixTypeSecondLastRange = 255;
    }

    /* Check next buckets */
    rangePtr = bucketPtr->rangeList;
    while (rangePtr)
    {
        /* Skip unicast ranges for multicast validation or multicast ranges for
           unicast validation */
        if (protocol == PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E)
        {
            if ((rangePtr->startAddr < prefixTypeFirstRange) ||
                ((rangePtr->startAddr > prefixTypeLastRange) && (rangePtr->startAddr<prefixTypeSecondRange))||
                 (rangePtr->startAddr > prefixTypeSecondLastRange))
            {
                rangePtr = rangePtr->next;
                continue;
            }
        }
        else
        {
            if ((rangePtr->startAddr < prefixTypeFirstRange) ||
                (rangePtr->startAddr > prefixTypeLastRange))
            {
                rangePtr = rangePtr->next;
                continue;
            }
        }

        /* TBD
           When MC implementation will be done need to change code
           rangePtr->lowerLpmPtr.nextBucket->bucketType==CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E
           my no longer be legal */
        if ((rangePtr->pointerType != CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E) &&
            (rangePtr->pointerType != CPSS_DXCH_LPM_MULTIPATH_ENTRY_PTR_TYPE_E) &&
            (!(((GT_U32)rangePtr->pointerType==(GT_U32)PRV_CPSS_DXCH_LPM_RAM_TRIE_PTR_TYPE_E) &&
             (rangePtr->lowerLpmPtr.nextBucket->bucketType==CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E))))
        {
            if (bucketPtr->bucketType==CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E)
            {
                /* find the correct GON pointer out of 6 possible pointers */
                gonPointerIndex = rangePtr->startAddr/44;
                /* in case we move from one goneindex to the next one,
                   we need to reset the counter of the childs */
                if (gonPointerIndex!=tempGonPointerIndex)
                {
                    totalCompressedChilds = 0;
                    totalRegularChilds    = 0;
                    tempGonPointerIndex = gonPointerIndex;

                }
            }
            else
            {
                gonPointerIndex = 0;
            }

            if (rangePtr->pointerType==CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E)
            {
                rangeType = PRV_CPSS_DXCH_LPM_CHILD_REGULAR_TYPE_E;
                totalChildTypesUntilNow = totalRegularChilds;
                totalRegularChilds++;
            }
            else
            {
                rangeType = PRV_CPSS_DXCH_LPM_CHILD_COMPRESSED_TYPE_E;
                totalChildTypesUntilNow = totalCompressedChilds;
                totalCompressedChilds++;
            }

             /* calculate the address and the size of a single node from the GON
               (6 lines for regular all the rest 1 line) according to the
               parameters we got from getNumOfRangesFromHWSip6 */

            hwNodeAddr = PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(bucketPtr->hwGroupOffsetHandle[gonPointerIndex]);

            status = getFromTheGonOneNodeAddrAndSize(hwNodeAddr,
                                                     nodeTotalChildTypesArr[gonPointerIndex],
                                                     rangeType,
                                                     totalChildTypesUntilNow,
                                                     &gonNodeAddr,
                                                     &gonNodeSize);
            if (status != GT_OK)
            {
                retVal = status;
                if (returnOnFailure == GT_TRUE)
                {
                    return retVal;
                }
            }

            /* read the HW data for the specific range */
            status = prvCpssDxChReadTableMultiEntry(devNum,
                                                 CPSS_DXCH_SIP5_TABLE_LPM_MEM_E,
                                                 gonNodeAddr,
                                                 gonNodeSize,
                                                 &hwBucketDataArr[0]);

            if (status != GT_OK)
            {
                retVal = status;
                if (returnOnFailure == GT_TRUE)
                {
                    return retVal;
                }
            }

            status = prvCpssDxChLpmRamDbgBucketHwShadowSyncValidityCheckSip6(devNum,
                                                                 shadowPtr,
                                                                 vrId,
                                                                 rangePtr->lowerLpmPtr.nextBucket,
                                                                 &hwBucketDataArr[0],
                                                                 (GT_U8)(level + 1),
                                                                 rangePtr->pointerType,
                                                                 numOfMaxAllowedLevels,
                                                                 prefixType,
                                                                 protocol,
                                                                 returnOnFailure,
                                                                 GT_FALSE/*not the root bucket*/);
            if (status != GT_OK)
            {
                retVal = status;
                if (returnOnFailure == GT_TRUE)
                {
                    return retVal;
                }
            }
        }
        else
        {
            /* in case of NextHop pointers no need to do anything since
               the SW and HW NextHop synchronization was checked in
               prvCpssDxChLpmRamDbgBucketHwShadowSyncValidityCheckSip6 */
        }
        rangePtr = rangePtr->next;
    }

    return retVal;
}

/**
* @internal
*           prvCpssDxChLpmRamDbgGetLeafDataSip6 function
* @endinternal
*
* @brief   get the leaf data from HW
*
* @param[in] devNum                   - The device number
* @param[in] bucketPtr                - pointer to the bucket
* @param[in] hwBucketDataArr          - array holding hw data.
*                                       in case of root this is
*                                       a null pointer
* @param[in] returnOnFailure          - GT_TRUE: the validation check stops on
*                                      first failure
*                                      GT_FALSE: continue with the test on
*                                      failure
* @param[out] nextNodeTypePtr         - pointer to the next node
*                                       entry type
* @param[out] nhAdditionalDataPtr     - pointer to a set of UC
*                                       security check enablers
*                                       and IPv6 MC scope level
* @param[out] nextBucketPointerPtr    - pointer to the next
*                                       bucket on the (G,S)
*                                       lookup
* @param[out] nhPointerPtr            - Pointer to the Next Hop
*                                       Table or the ECMP Table,
*                                       based on the Leaf Type
* @param[out] leafTypePtr             - pointer to The leaf
*                                       entry type
* @param[out] entryTypePtr            - pointer to entry type:
*                                       Leaf ot Trigger
* @param[out] lpmOverEmPriorityPtr    - pointer to the resolution
*                                       priority between LPM and
*                                       Exact Match results
*
* @retval GT_FAIL                  - on failure
* @retval GT_OK                    - on success
*/
static GT_STATUS prvCpssDxChLpmRamDbgGetLeafDataSip6
(
    IN  GT_U8                                       devNum,
    IN  PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC     *bucketPtr,
    IN  GT_U32                                      hwBucketDataArr[],
    IN  GT_U32                                      hwRangeType,
    IN  GT_U32                                      hwRangeTypeIndex,
    IN  GT_U32                                      nodeChildAddress,
    IN  GT_U32                                      nodeTotalChildTypesArr[],
    IN  GT_BOOL                                     returnOnFailure,
    OUT GT_U32                                      *nextNodeTypePtr,
    OUT GT_U32                                      *nhAdditionalDataPtr,
    OUT GT_U32                                      *nextBucketPointerPtr,
    OUT GT_U32                                      *nhPointerPtr,
    OUT GT_U32                                      *leafTypePtr,
    OUT GT_U32                                      *entryTypePtr,
    OUT GT_U32                                      *lpmOverEmPriorityPtr
)
{
    GT_STATUS status = GT_OK;
    GT_U32    gonNodeAddr, gonNodeSize;
    GT_U32    leafData,numberOfEmbeddedLeafs;
    GT_U32    value;

    GT_U32 nextNodeType; /* bit 22
                            Defines the next node entry type
                            0x0 = Regular
                            0x1 = Compressed
                            valid if EntryType="Trigger" */

    GT_U32 nhPointer;       /* bits 21-7
                            Pointer to the Next Hop Table or the ECMP Table,
                            based on the Leaf Type.
                            valid if EntryType="Leaf" */
    GT_U32 nhAdditionalData;/* bits 6-3
                            Contains a set of UC security check enablers
                            and IPv6 MC scope level:
                            1. [6..6] UC SIP SA Check Mismatch Enable
                            2. [5..5] UC RPF Check Enable
                            3. [4..3] IPv6 MC Group Scope Level[1..0]
                            valid if EntryType="Leaf" */
    GT_U32 nextBucketPointer;/* bits 21-2
                                Pointer to the next bucket on the (G,S) lookup
                                valid if EntryType="Trigger" */
    GT_U32 leafType; /* bit 2
                        The leaf entry type
                        0x0 = Regular Leaf
                        0x1 = Multipath Leaf
                        valid if EntryType="Leaf" */
    GT_U32 entryType;/* bit 1
                        In the process of (*,G) lookup.
                        When an entry has this bit set, the (*, G) lookup
                        terminates with a match at the current entry, and (S,
                        G) SIP based lookup is triggered.
                        Note that in such case, head of trie start address for
                        the (S,G) lookup is obtained from the the (*, G)
                        lookup stage.
                         0x0 = Leaf
                         0x1 = Trigger; Trigger IP MC S+G Lookup */
    GT_U32 lpmOverEmPriority;/* bit 0
                                Define the resolution priority between LPM and
                                Exact Match results
                                 0x0 = Low; Exact Match has priority over LPM result
                                 0x1 = High;LPM result has priority over Exact Match */

    /* if the bucket is not embedded we read the leaf from HW */
    if(!((bucketPtr->bucketType>=CPSS_DXCH_LPM_COMPRESSED_UP_TO_7_RANGES_1_LEAF_NODE_PTR_TYPE_E)&&
         (bucketPtr->bucketType<=CPSS_DXCH_LPM_COMPRESSED_3_RANGES_3_LEAVES_NODE_PTR_TYPE_E)))
    {
        /* calculate the address and the size of a single node from the GON
          (6 lines for regular all the rest 1 line) according to the
           parameters we got from getNumOfRangesFromHWSip6 */
        status = getFromTheGonOneNodeAddrAndSize(nodeChildAddress,
                                                 nodeTotalChildTypesArr,
                                                 hwRangeType,
                                                 hwRangeTypeIndex,
                                                 &gonNodeAddr,
                                                 &gonNodeSize);
        if (status != GT_OK)
        {
            LPM_SHADOW_HW_SYNC_VALIDATION_ERROR_MAC(returnOnFailure, status, "Bucket",
                                        bucketPtr,nodeChildAddress,
                                        "Error: call to getFromTheGonOneNodeAddrAndSize in prvCpssDxChLpmRamDbgGetLeafDataSip6 \n");
        }

         /* read one range (range number i) */
        status = prvCpssDxChReadTableMultiEntry(devNum,
                                     CPSS_DXCH_SIP5_TABLE_LPM_MEM_E,
                                     gonNodeAddr,
                                     gonNodeSize,
                                     hwBucketDataArr);
        if (status != GT_OK)
        {
            LPM_SHADOW_HW_SYNC_VALIDATION_ERROR_MAC(returnOnFailure, status, "Bucket",
                                        bucketPtr,nodeChildAddress,
                                        "Error: call to prvCpssDxChReadTableMultiEntry in prvCpssDxChLpmRamDbgGetLeafDataSip6\n");
        }
        /* need to find the correct leaf out of all the leafs in the line - in HW the first leaf are kept in last bits:
           0-22: leaf 4
           23-45: leaf 3
           46-68: leaf 2
           69-91: leaf 1
           92-114: leaf 0 */
        U32_GET_FIELD_IN_ENTRY_MAC(hwBucketDataArr,
                                   (MAX_NUMBER_OF_LEAVES_IN_LPM_LINE_CNS-(hwRangeTypeIndex%MAX_NUMBER_OF_LEAVES_IN_LPM_LINE_CNS)-1)*23,
                                   23,leafData);

        U32_GET_FIELD_IN_ENTRY_MAC(&leafData,0,1,lpmOverEmPriority);
        U32_GET_FIELD_IN_ENTRY_MAC(&leafData,1,1,entryType);
        U32_GET_FIELD_IN_ENTRY_MAC(&leafData,2,1,leafType);
        U32_GET_FIELD_IN_ENTRY_MAC(&leafData,2,20,nextBucketPointer);
        U32_GET_FIELD_IN_ENTRY_MAC(&leafData,3,4,nhAdditionalData);
        U32_GET_FIELD_IN_ENTRY_MAC(&leafData,7,15,nhPointer);
        U32_GET_FIELD_IN_ENTRY_MAC(&leafData,22,1,nextNodeType);
    }
    else
    {   /* the HW data is already given to us as a parameter to the function */

        /* bit 113-112 Embedded_leaf */
        U32_GET_FIELD_IN_ENTRY_MAC(hwBucketDataArr,112,2,value);
        numberOfEmbeddedLeafs = value;

        if (numberOfEmbeddedLeafs!=0)
        {
            /* check numberOfEmbeddedLeafs fit SW type */
            switch (numberOfEmbeddedLeafs)
            {
            case 1:
                if (bucketPtr->bucketType!=CPSS_DXCH_LPM_COMPRESSED_UP_TO_7_RANGES_1_LEAF_NODE_PTR_TYPE_E)
                {
                    LPM_SHADOW_HW_SYNC_VALIDATION_ERROR_MAC(returnOnFailure, status, "Bucket",
                                            bucketPtr,nodeChildAddress,
                                            "Error: no synchronization between HW and SW - Hw numberOfEmbeddedLeafs=1 in SW "
                                            "embedded type - in call to prvCpssDxChLpmRamDbgGetLeafDataSip6\n");
                }
                break;
            case 2:
                if (bucketPtr->bucketType!=CPSS_DXCH_LPM_COMPRESSED_UP_TO_5_RANGES_2_LEAVES_NODE_PTR_TYPE_E)
                {
                    LPM_SHADOW_HW_SYNC_VALIDATION_ERROR_MAC(returnOnFailure, status, "Bucket",
                                            bucketPtr,nodeChildAddress,
                                            "Error: no synchronization between HW and SW - Hw numberOfEmbeddedLeafs=2 in SW "
                                            "embedded type - in call to prvCpssDxChLpmRamDbgGetLeafDataSip6\n");
                }
                break;
            case 3:
                if (bucketPtr->bucketType!=CPSS_DXCH_LPM_COMPRESSED_3_RANGES_3_LEAVES_NODE_PTR_TYPE_E)
                {
                    LPM_SHADOW_HW_SYNC_VALIDATION_ERROR_MAC(returnOnFailure, status, "Bucket",
                                            bucketPtr,nodeChildAddress,
                                            "Error: no synchronization between HW and SW - Hw numberOfEmbeddedLeafs=3 in SW"
                                            "embedded type - in call to prvCpssDxChLpmRamDbgGetLeafDataSip6\n");
                }
                break;
            default:
                LPM_SHADOW_HW_SYNC_VALIDATION_ERROR_MAC(returnOnFailure, status, "Bucket",
                                        bucketPtr,nodeChildAddress,
                                        "Error: no synchronization between HW and SW - Hw numberOfEmbeddedLeafs=0 in SW "
                                        "embedded type - in call to prvCpssDxChLpmRamDbgGetLeafDataSip6\n");
            }
            switch (hwRangeTypeIndex)
            {
            case 0:/*first embedded leaf located at bit 89-111*/
                U32_GET_FIELD_IN_ENTRY_MAC(hwBucketDataArr, 89, 23, value);
                break;
            case 1:/*second embedded leaf located at bit 66-88*/
                U32_GET_FIELD_IN_ENTRY_MAC(hwBucketDataArr,66,23,value);
                break;
            case 2:/*third embedded leaf located at bit 43-65*/
                U32_GET_FIELD_IN_ENTRY_MAC(hwBucketDataArr,43,23,value);
                break;
            default:
                LPM_SHADOW_HW_SYNC_VALIDATION_ERROR_MAC(returnOnFailure, status, "Bucket",
                                        bucketPtr,nodeChildAddress,
                                        "Error: no synchronization between HW and SW - illegal value in hwRangesTypeIndexArr"
                                        " -  in call to prvCpssDxChLpmRamDbgGetLeafDataSip6\n");
                break;
            }
        }
        else
        {
            LPM_SHADOW_HW_SYNC_VALIDATION_ERROR_MAC(returnOnFailure, status, "Bucket",
                                    bucketPtr,nodeChildAddress,
                                    "Error: no synchronization between HW and SW - Hw numberOfEmbeddedLeafs=0 in SW "
                                    "embedded type - in call to prvCpssDxChLpmRamDbgGetLeafDataSip6\n");
        }

        leafData = value;

        U32_GET_FIELD_IN_ENTRY_MAC(&leafData,0,1,lpmOverEmPriority);
        U32_GET_FIELD_IN_ENTRY_MAC(&leafData,1,1,entryType);
        U32_GET_FIELD_IN_ENTRY_MAC(&leafData,2,1,leafType);
        U32_GET_FIELD_IN_ENTRY_MAC(&leafData,2,20,nextBucketPointer);
        U32_GET_FIELD_IN_ENTRY_MAC(&leafData,3,4,nhAdditionalData);
        U32_GET_FIELD_IN_ENTRY_MAC(&leafData,7,15,nhPointer);
        U32_GET_FIELD_IN_ENTRY_MAC(&leafData,22,1,nextNodeType);
    }

    *nextNodeTypePtr = nextNodeType;
    *nhAdditionalDataPtr = nhAdditionalData;
    *nextBucketPointerPtr = nextBucketPointer;
    *nhPointerPtr = nhPointer;
    *leafTypePtr = leafType;
    *entryTypePtr = entryType;
    *lpmOverEmPriorityPtr = lpmOverEmPriority;

    return GT_OK;
}
/**
* @internal
*           prvCpssDxChLpmRamDbgBucketHwShadowSyncValidityCheckSip6
*           function
* @endinternal
*
* @brief   Shadow and HW synchronization validation of a bucket
*
* @param[in] devNum                   - The device number
* @param[in] shadowPtr                - pointer to shadow structure
* @param[in] vrId                     - the virtual router id
* @param[in] bucketPtr                - pointer to the bucket
* @param[in] hwBucketDataArr          - array holding hw data.
*                                       in case of root this is
*                                       a null pointer
* @param[in] level                    - the  in the tree (first  is 0)
* @param[in] expectedNextPointerType  - the expected type of the bucket, ignored
*                                      for level 0
* @param[in] numOfMaxAllowedLevels    - the maximal allowed number of levels
*                                      for the relevant protocol and prefix type
* @param[in] prefixType               - unicast or multicast tree
* @param[in] protocol                 - protocol
* @param[in] returnOnFailure          - GT_TRUE: the validation check stops on
*                                      first failure
*                                      GT_FALSE: continue with the test on
*                                      failure
* @param[in] isRootBucket             - GT_TRUE:the bucketPtr is the root bucket
*                                      GT_FALSE:the bucketPtr is not the root bucket
*
* @retval GT_FAIL                  - on failure
* @retval GT_OK                    - on success
*/
static GT_STATUS prvCpssDxChLpmRamDbgBucketHwShadowSyncValidityCheckSip6
(
    IN  GT_U8                                     devNum,
    IN  PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC          *shadowPtr,
    IN  GT_U32                                    vrId,
    IN  PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC   *bucketPtr,
    IN  GT_U32                                    hwBucketDataArr[],
    IN  GT_U8                                     level,
    IN  CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT       expectedNextPointerType,
    IN  GT_U8                                     numOfMaxAllowedLevels,
    IN  CPSS_UNICAST_MULTICAST_ENT                prefixType,
    IN  PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT      protocol,
    IN  GT_BOOL                                   returnOnFailure,
    IN  GT_BOOL                                   isRootBucket
)
{
    GT_STATUS status, retVal = GT_OK;
    PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC *tempRangePtr=NULL;/* Current range pointer.   */
    GT_U32    hwNumOfRanges;   /* the number of ranges in the HW */
    GT_U32    hwRangesArr[256];   /* the ranges values retrieved from the HW */
    GT_U32    hwRangesTypesArr[256];   /* for each range keep its type (regular,compress,leaf) */
    GT_U32    hwRangesBitLineArr[256]; /* for each range keep its bit vector line */
    GT_U32    hwRangesTypeIndexArr[256]; /* what is the number of this range type
                                            from total number of lines with the same type -
                                            where it is located in the GON */
    GT_U32    nodeChildAddressesArr[PRV_CPSS_DXCH_LPM_RAM_FALCON_MAX_SIZE_OF_BUCKET_IN_LPM_LINES_CNS];/* the child_pointer located at the beginning of each line
                                                                                            for regular node we have 6 pointers, for compress node one pointer */
    GT_U32    nodeTotalChildTypesArr[PRV_CPSS_DXCH_LPM_RAM_FALCON_MAX_SIZE_OF_BUCKET_IN_LPM_LINES_CNS][PRV_CPSS_DXCH_LPM_MAX_CHILD_TYPE_CNS];/* 6 elements in a regular node
                                                                                                3 types of child that can be for each range (leaf,regular,compress) */

    GT_U32    nodeTotalLinesPerTypeArr[PRV_CPSS_DXCH_LPM_RAM_FALCON_MAX_SIZE_OF_BUCKET_IN_LPM_LINES_CNS][PRV_CPSS_DXCH_LPM_MAX_CHILD_TYPE_CNS];   /* sum the number of lines in the GON according to the Type */
    GT_U32    nodeTotalBucketPerTypesArr[PRV_CPSS_DXCH_LPM_RAM_FALCON_MAX_SIZE_OF_BUCKET_IN_LPM_LINES_CNS][PRV_CPSS_DXCH_LPM_MAX_CHILD_TYPE_CNS]; /* sum the number of buckets (copmress, regular or leaf) in the GON according to the Type */

    GT_U32    bankIndexsOfTheGonsArray[PRV_CPSS_DXCH_LPM_RAM_FALCON_MAX_SIZE_OF_BUCKET_IN_LPM_LINES_CNS];  /* the indexs of the banks were the GON is located */

    GT_U32    hwPointerType;   /* The type of the next entry this entry points to. can be Leaf/Trigger */
    GT_U32    hwNhPointer;     /* This is the next hop pointer for accessing next hop table or
                                  the pointer for the ECMP table for fetching the ECMP attributes from ECMP table */
    GT_U32    i;
    GT_U32                              swNodeAddr = 0;/* SW node address kept in shadow(hwBucketOffsetHandle) in line representation from the base */
    CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT swNodeType = 0;/* the bucketType retrieved from SW */
    GT_U32                              hwNodeAddr[6];/* HW node address */
    GT_U32                              hwNodeSize=0; /* size of the node in lines
                                                         regular node have 6 lines, all the rest hold 1 line*/
    PRV_CPSS_DXCH_LPM_CHILD_TYPE_ENT    hwNodeType = 0;/* the bucketType retrieved from HW */
    GT_U32                              hwRootNodeAddr=0;/* HW Root node address */
    PRV_CPSS_DXCH_LPM_CHILD_TYPE_ENT    hwRootNodeType =0;/* the root bucketType retrieved from HW */

    GT_U32                              swTotalNumOfRanges=0;/* number of ranges from sw including the ranges that are hidden inside a regular node */
    GT_U32                              swTotalRangesArr[256];/* the ranges values retrieved from the SW */
    GT_U32                              swTotalRangesTypesArr[256];/* the ranges types values retrieved from the SW */
    GT_BOOL                             swTotalRangesIsHiddenArr[256];/* Is the range a hidden one or not */

    GT_U32 nextNodeType; /* bit 22
                            Defines the next node entry type
                            0x0 = Regular
                            0x1 = Compressed
                            valid if EntryType="Trigger" */

    GT_U32 nhPointer;       /* bits 21-7
                            Pointer to the Next Hop Table or the ECMP Table,
                            based on the Leaf Type.
                            valid if EntryType="Leaf" */
    GT_U32 nhAdditionalData;/* bits 6-3
                            Contains a set of UC security check enablers
                            and IPv6 MC scope level:
                            1. [6..6] UC SIP SA Check Mismatch Enable
                            2. [5..5] UC RPF Check Enable
                            3. [4..3] IPv6 MC Group Scope Level[1..0]
                            valid if EntryType="Leaf" */
    GT_U32 nextBucketPointer;/* bits 21-2
                                Pointer to the next bucket on the (G,S) lookup
                                valid if EntryType="Trigger" */
    GT_U32 leafType; /* bit 2
                        The leaf entry type
                        0x0 = Regular Leaf
                        0x1 = Multipath Leaf
                        valid if EntryType="Leaf" */
    GT_U32 entryType;/* bit 1
                        In the process of (*,G) lookup.
                        When an entry has this bit set, the (*, G) lookup
                        terminates with a match at the current entry, and (S,
                        G) SIP based lookup is triggered.
                        Note that in such case, head of trie start address for
                        the (S,G) lookup is obtained from the the (*, G)
                        lookup stage.
                         0x0 = Leaf
                         0x1 = Trigger; Trigger IP MC S+G Lookup */
    GT_U32 lpmOverEmPriority;/* bit 0
                                Define the resolution priority between LPM and
                                Exact Match results
                                 0x0 = Low; Exact Match has priority over LPM result
                                 0x1 = High;LPM result has priority over Exact Match */

    cpssOsMemSet(&hwNodeAddr[0], 0, sizeof(hwNodeAddr));
    if (isRootBucket==GT_TRUE)
    {
        /* 1. check that the Root SW bucketType is the same as HW bucketType */

        /* read the HW data directly from the VRF table and compare it to the SW data in the shadow */
        status = prvCpssDxChLpmHwVrfEntryReadSip6(devNum, vrId, protocol,
                                                  &hwRootNodeType, &hwRootNodeAddr);
        if (status != GT_OK)
        {
            cpssOsPrintf("Error on reading Vrf table for vrfId=%d, and protocol=%d\n", vrId, protocol);
            return status;
        }
    }
    else
    {
        /* if this is not a call to the root bucket phase, but a call done from the ranges loop in
           prvCpssDxChLpmRamDbgRangesHwShadowSyncCheckSip6, then we need to take the data address of
           the bucket from the shadow and to retrieved all the HW data */
        switch (bucketPtr->bucketType)
        {
        case CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E:
            for (i=0;i<PRV_CPSS_DXCH_LPM_RAM_FALCON_BUCKET_SIZE_REGULAR_BV_CNS;i++)
            {
                hwNodeAddr[i] = PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(bucketPtr->hwGroupOffsetHandle[i]);
                hwNodeSize=6;
            }
            hwNodeType = PRV_CPSS_DXCH_LPM_CHILD_REGULAR_TYPE_E;
            swNodeType = bucketPtr->bucketType;
            break;
        case CPSS_DXCH_LPM_COMPRESSED_NODE_PTR_TYPE_E:
        case CPSS_DXCH_LPM_COMPRESSED_UP_TO_7_RANGES_1_LEAF_NODE_PTR_TYPE_E:
        case CPSS_DXCH_LPM_COMPRESSED_UP_TO_5_RANGES_2_LEAVES_NODE_PTR_TYPE_E:
        case CPSS_DXCH_LPM_COMPRESSED_3_RANGES_3_LEAVES_NODE_PTR_TYPE_E:
            /* in cases of last GON level the pointer will be null */
            if (bucketPtr->hwGroupOffsetHandle[0]!=0)
            {
                hwNodeAddr[0] = PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(bucketPtr->hwGroupOffsetHandle[0]);
            }
            else
            {
                hwNodeAddr[0] = 0;
            }

            hwNodeType = PRV_CPSS_DXCH_LPM_CHILD_COMPRESSED_TYPE_E;
            swNodeType = bucketPtr->bucketType;
            hwNodeSize=1;
            break;
        case CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E:
        case CPSS_DXCH_LPM_MULTIPATH_ENTRY_PTR_TYPE_E:
        default:
            /* error - we should not get here in case of a leaf */
            cpssOsPrintf("Error - we should not get here in case of a leaf. vrfId=%d, and protocol=%d\n", vrId, protocol);
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
        }
    }

    /* check that the HW next bucket type is the same as SW bucketType
       Exception: bucketType of a MC source is different than pointerType of the
       pointing range */
    if(((GT_U32)expectedNextPointerType != (GT_U32)PRV_CPSS_DXCH_LPM_RAM_TRIE_PTR_TYPE_E))
    {
        switch (expectedNextPointerType)
        {
            case CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E:

                if (isRootBucket==GT_TRUE)
                {
                    if(hwRootNodeType!=PRV_CPSS_DXCH_LPM_CHILD_REGULAR_TYPE_E)
                    {
                        LPM_SHADOW_HW_SYNC_VALIDATION_ERROR_MAC(returnOnFailure, retVal, "Bucket",
                                                    bucketPtr,hwRootNodeAddr,
                                                    "Error: no synchronization between HW and SW - NextPointerType\n");
                    }
                }
                else
                {
                    if(hwNodeType!=PRV_CPSS_DXCH_LPM_CHILD_REGULAR_TYPE_E)
                    {
                        LPM_SHADOW_HW_SYNC_VALIDATION_ERROR_MAC(returnOnFailure, retVal, "Bucket",
                                                    bucketPtr,hwNodeAddr[0],
                                                    "Error: no synchronization between HW and SW - NextPointerType\n");
                    }
                }
                break;
            case CPSS_DXCH_LPM_COMPRESSED_NODE_PTR_TYPE_E:
            case CPSS_DXCH_LPM_COMPRESSED_UP_TO_7_RANGES_1_LEAF_NODE_PTR_TYPE_E:
            case CPSS_DXCH_LPM_COMPRESSED_UP_TO_5_RANGES_2_LEAVES_NODE_PTR_TYPE_E:
            case CPSS_DXCH_LPM_COMPRESSED_3_RANGES_3_LEAVES_NODE_PTR_TYPE_E:
                /* the VR table can only point to regular */
                if (isRootBucket==GT_TRUE)
                {
                    LPM_SHADOW_HW_SYNC_VALIDATION_ERROR_MAC(returnOnFailure, retVal, "Bucket",
                                                bucketPtr,hwRootNodeAddr,
                                                "invalid VR bucket type");
                }
                else
                {
                    if(swNodeType!=expectedNextPointerType)
                        LPM_SHADOW_HW_SYNC_VALIDATION_ERROR_MAC(returnOnFailure, retVal, "Bucket",
                                                        bucketPtr,hwNodeAddr[0],
                                                        "Error: no synchronization between HW and SW - NextPointerType\n");
                }
                break;
            case CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E:
            case CPSS_DXCH_LPM_MULTIPATH_ENTRY_PTR_TYPE_E:
            default:
                LPM_SHADOW_HW_SYNC_VALIDATION_ERROR_MAC(returnOnFailure, retVal, "Bucket",
                                                bucketPtr,hwNodeAddr[0],
                                                "invalid NextPointerType");
        }
    }

    if (isRootBucket==GT_TRUE)
    {
        /* 2. check that the Root SW Head Of Trie is the same as HW Head Of Trie */

        /* only the root uses hwBucketOffsetHandle pointer and not hwGroupOffsetHandle */
        swNodeAddr = PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(bucketPtr->hwBucketOffsetHandle);
        if(swNodeAddr != hwRootNodeAddr)
        {
            LPM_SHADOW_HW_SYNC_VALIDATION_ERROR_MAC(returnOnFailure, retVal, "Bucket",
                                            bucketPtr,hwRootNodeAddr,
                                            "Error: no synchronization between HW and SW - Root Head Of Trie\n");
        }
    }
    else
    {
        /* no need to make the check since in case of non-root bucket we are assigning hwNodeAddr to be the value retrieved from the shadow
           meaning hwNodeAddr is swNodeAddr, we are doing so since we are interested to check the data of this address and not the address itself
           the checking of the address was done in previous recursive call */
    }

    cpssOsMemSet(swTotalRangesArr, 0, sizeof(swTotalRangesArr));
    cpssOsMemSet(swTotalRangesTypesArr, 0, sizeof(swTotalRangesTypesArr));
    cpssOsMemSet(swTotalRangesIsHiddenArr, 0, sizeof(swTotalRangesIsHiddenArr));

    cpssOsMemSet(nodeChildAddressesArr,0,sizeof(nodeChildAddressesArr));
    cpssOsMemSet(nodeTotalChildTypesArr,0,sizeof(nodeTotalChildTypesArr));
    cpssOsMemSet(nodeTotalLinesPerTypeArr,0,sizeof(nodeTotalLinesPerTypeArr));
    cpssOsMemSet(nodeTotalBucketPerTypesArr,0,sizeof(nodeTotalBucketPerTypesArr));
    cpssOsMemSet(hwRangesArr,0,sizeof(hwRangesArr));
    cpssOsMemSet(hwRangesTypesArr,0,sizeof(hwRangesTypesArr));
    cpssOsMemSet(hwRangesBitLineArr,0,sizeof(hwRangesBitLineArr));
    cpssOsMemSet(hwRangesTypeIndexArr,0,sizeof(hwRangesTypeIndexArr));
    cpssOsMemSet(bankIndexsOfTheGonsArray,0,sizeof(bankIndexsOfTheGonsArray));

    if (isRootBucket==GT_TRUE)
    {
        /* hw pointer in LPM entry is in LPM lines*/
        hwNodeSize=PRV_CPSS_DXCH_LPM_RAM_FALCON_BUCKET_SIZE_REGULAR_BV_CNS;

        /* read the bit vector */
        status = prvCpssDxChReadTableMultiEntry(devNum,
                                            CPSS_DXCH_SIP5_TABLE_LPM_MEM_E,
                                            hwRootNodeAddr, hwNodeSize, &hwBucketDataArr[0]);
        if (status != GT_OK)
        {
            return status;
        }

       status = getNumOfRangesFromHWSip6(protocol,
                                          level,
                                          hwRootNodeType, /* the Root node type */
                                          &hwBucketDataArr[0],/* read from HW according to Root address */
                                          hwRootNodeAddr,/* hwAddr is used in the fuction only for print incase of an error */
                                          nodeChildAddressesArr,
                                          nodeTotalChildTypesArr,
                                          nodeTotalLinesPerTypeArr,
                                          nodeTotalBucketPerTypesArr,
                                          &hwNumOfRanges,
                                          hwRangesArr,
                                          hwRangesTypesArr,
                                          hwRangesTypeIndexArr,
                                          hwRangesBitLineArr);
        if (status != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }
    }
    else
    {
        /*  we are dealing with a non-root bucket */

        /* hwBucketDataArr contains the HW data of the node */
        status = getNumOfRangesFromHWSip6(protocol,
                                          level,
                                          hwNodeType,      /* the node type */
                                          &hwBucketDataArr[0], /* given as a parameter to the function from the call to
                                                                  prvCpssDxChLpmRamDbgRangesHwShadowSyncValidityCheckSip6*/
                                          hwNodeAddr[0],/* hwAddr is used in the fuction only for print incase of an error */
                                          nodeChildAddressesArr,
                                          nodeTotalChildTypesArr,
                                          nodeTotalLinesPerTypeArr,
                                          nodeTotalBucketPerTypesArr,
                                          &hwNumOfRanges,
                                          hwRangesArr,
                                          hwRangesTypesArr,
                                          hwRangesTypeIndexArr,
                                          hwRangesBitLineArr);
        if (status != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }

    }

    /* 3. check that the SW ranges is the same as HW bitVector values
          get the SW number of ranges including the ones hidden incase of regular node */
    status = prvCpssDxChLpmRamDbgBucketShadowGetNumOfTotalRangesHiddenIncluddedSip6(bucketPtr,
                                                                                    &swTotalNumOfRanges,
                                                                                    swTotalRangesArr,
                                                                                    swTotalRangesTypesArr,
                                                                                    swTotalRangesIsHiddenArr);
    if (status!=GT_OK)
    {
        LPM_SHADOW_HW_SYNC_VALIDATION_ERROR_MAC(returnOnFailure, retVal, "Bucket",
                                        bucketPtr,hwNodeAddr[0],
                                        "Error: no synchronization between HW and SW - "
                                        "call to prvCpssDxChLpmRamDbgBucketShadowGetNumOfTotalRangesHiddenIncluddedSip6\n");

    }
    if (swTotalNumOfRanges != hwNumOfRanges)
    {
        LPM_SHADOW_HW_SYNC_VALIDATION_ERROR_MAC(returnOnFailure, retVal, "Bucket",
                                        bucketPtr,hwNodeAddr[0],
                                        "Error: no synchronization between HW and SW - Root num of ranges\n");
    }

    /* 4. check that the SW ranges values is the same as HW ranges values */
    tempRangePtr = bucketPtr->rangeList;
    for (i=0; i<hwNumOfRanges; i++)
    {
        if(swTotalRangesArr[i] != hwRangesArr[i])
        {
            LPM_SHADOW_HW_SYNC_VALIDATION_ERROR_MAC(returnOnFailure, retVal, "Bucket",
                                        bucketPtr,hwNodeAddr[hwRangesBitLineArr[i]],
                                        "Error: no synchronization between HW and SW - ranges values\n");
        }
        else
        {
            /* range value is synchronized between HW and SW so now check that the SW range type
               is the same as HW range type, if it is the same then compare the range pointer data */
            hwPointerType = hwRangesTypesArr[i];/* HW type can be empty=0/leaf=1/regular=2/compressed=3
                                                   SW type can be regular=0/route=3/compressed=6/
                                                   embedded1=7/embedded2=8/embedded3=9/multipath=9 */

            if(((swTotalRangesTypesArr[i] == CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E)&&(hwPointerType!=PRV_CPSS_DXCH_LPM_CHILD_REGULAR_TYPE_E))||
                ((swTotalRangesTypesArr[i]>=CPSS_DXCH_LPM_COMPRESSED_NODE_PTR_TYPE_E)&&
                 (swTotalRangesTypesArr[i]<=CPSS_DXCH_LPM_COMPRESSED_3_RANGES_3_LEAVES_NODE_PTR_TYPE_E)&&
                 (hwPointerType!=PRV_CPSS_DXCH_LPM_CHILD_COMPRESSED_TYPE_E))||
                ((swTotalRangesTypesArr[i]==CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E)&&(hwPointerType!=PRV_CPSS_DXCH_LPM_CHILD_LEAF_TYPE_E))||
                ((swTotalRangesTypesArr[i]==CPSS_DXCH_LPM_MULTIPATH_ENTRY_PTR_TYPE_E)&&(hwPointerType!=PRV_CPSS_DXCH_LPM_CHILD_LEAF_TYPE_E)))
            {
                LPM_SHADOW_HW_SYNC_VALIDATION_ERROR_MAC(returnOnFailure, retVal, "Bucket",
                                        bucketPtr,hwNodeAddr[hwRangesBitLineArr[i]],
                                        "Error: no synchronization between HW and SW - range Pointer Type value\n");
            }
            else
            {
                /* If the pointerType is to a MC source and the MC source bucket is a regular bucket then it means that
                   this is a root of MC source tree that points directly to a nexthop or ECMP/QoS entry */
                if ((swTotalRangesIsHiddenArr[i]==GT_FALSE)&&
                    ((GT_U32)swTotalRangesTypesArr[i]==(GT_U32)PRV_CPSS_DXCH_LPM_RAM_TRIE_PTR_TYPE_E) &&
                    (tempRangePtr->lowerLpmPtr.nextBucket->bucketType==CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E))
                {
                    /* TBD
                       after MC implementation need to check this code again since the if
                       tempRangePtr->lowerLpmPtr.nextBucket->bucketType==CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E
                       will no longer be legal - different implementation then in SIP5 */
                    retVal = prvCpssDxChLpmRamDbgGetLeafDataSip6(devNum,
                                            bucketPtr,
                                            hwBucketDataArr,
                                            hwRangesTypesArr[i],
                                            hwRangesTypeIndexArr[i],
                                            nodeChildAddressesArr[hwRangesBitLineArr[i]],
                                            nodeTotalChildTypesArr[hwRangesBitLineArr[i]],
                                            returnOnFailure,
                                            &nextNodeType,
                                            &nhAdditionalData,
                                            &nextBucketPointer,
                                            &nhPointer,
                                            &leafType,
                                            &entryType,
                                            &lpmOverEmPriority);
                    if (retVal!=GT_OK)
                    {
                        LPM_SHADOW_HW_SYNC_VALIDATION_ERROR_MAC(returnOnFailure, retVal, "Bucket",
                                    bucketPtr,hwNodeAddr[hwRangesBitLineArr[i]],
                                    "Error: no synchronization between HW and SW - prvCpssDxChLpmRamDbgGetLeafDataSip6\n");
                    }

                    if(entryType!=PRV_CPSS_DXCH_LPM_LEAF_ENTRY_TYPE_TRIGGER_E)
                    {
                        LPM_SHADOW_HW_SYNC_VALIDATION_ERROR_MAC(returnOnFailure, retVal, "Bucket",
                                    bucketPtr,hwNodeAddr[hwRangesBitLineArr[i]],
                                    "Error: no synchronization between HW and SW - Pointer value\n");
                    }


                    /* TBD this check will probably be wrong in the new implementation */
                    switch (tempRangePtr->lowerLpmPtr.nextBucket->rangeList->lowerLpmPtr.nextHopEntry->routeEntryMethod)
                    {
                        case PRV_CPSS_DXCH_LPM_ENTRY_TYPE_MULTIPATH_E:
                            if (leafType != PRV_CPSS_DXCH_LPM_LEAF_MULTIPATH_TYPE_E)/*Multipath Leaf*/
                            {
                                LPM_SHADOW_HW_SYNC_VALIDATION_ERROR_MAC(returnOnFailure, retVal, "Bucket",
                                                bucketPtr,hwNodeAddr[hwRangesBitLineArr[i]],
                                                "Error: no synchronization between HW and SW - Pointer value\n");
                            }
                            else
                            {
                                /* 5. check that if a SW ranges is of type ECMP/QOS then HW range should point to the same NH */
                                hwNhPointer = nhPointer;

                                if (tempRangePtr->lowerLpmPtr.nextBucket->rangeList->lowerLpmPtr.nextHopEntry->routeEntryBaseMemAddr != hwNhPointer)
                                {
                                    LPM_SHADOW_HW_SYNC_VALIDATION_ERROR_MAC(returnOnFailure, retVal, "Bucket",
                                                bucketPtr,hwNodeAddr[hwRangesBitLineArr[i]],
                                                "Error: no synchronization between HW and SW - Pointer value\n");
                                }
                            }
                            break;
                        case PRV_CPSS_DXCH_LPM_ENTRY_TYPE_REGULAR_E:
                            if (leafType != PRV_CPSS_DXCH_LPM_LEAF_REGULAR_TYPE_ENT)/*Regular Leaf*/
                            {
                                LPM_SHADOW_HW_SYNC_VALIDATION_ERROR_MAC(returnOnFailure, retVal, "Bucket",
                                                bucketPtr,hwNodeAddr[hwRangesBitLineArr[i]],
                                                "Error: no synchronization between HW and SW - Pointer value\n");
                            }
                            else
                            {
                                /* 5. check that if a SW ranges is of type NH then HW range should point to the same NH */
                                hwNhPointer = nhPointer;
                                if (tempRangePtr->lowerLpmPtr.nextBucket->rangeList->lowerLpmPtr.nextHopEntry->routeEntryBaseMemAddr != hwNhPointer)
                                {
                                    LPM_SHADOW_HW_SYNC_VALIDATION_ERROR_MAC(returnOnFailure, retVal, "Bucket",
                                                bucketPtr,hwNodeAddr[hwRangesBitLineArr[i]],
                                                "Error: no synchronization between HW and SW - Pointer value\n");
                                }
                            }
                            break;
                        default:
                            LPM_SHADOW_HW_SYNC_VALIDATION_ERROR_MAC(returnOnFailure, retVal, "Bucket",
                                            bucketPtr,hwNodeAddr[hwRangesBitLineArr[i]],
                                            "Error: no synchronization between HW and SW - error in routeEntryMethod \n");
                            break;
                    }
                }
                else
                {
                    switch (hwPointerType)
                    {
                    case PRV_CPSS_DXCH_LPM_CHILD_LEAF_TYPE_E:

                         /* only incase we are dealing with a non-hidden range we should continue
                            checking this leaf */
                        if (swTotalRangesIsHiddenArr[i]==GT_FALSE)
                        {
                            retVal = prvCpssDxChLpmRamDbgGetLeafDataSip6(devNum,
                                                                        bucketPtr,
                                                                        hwBucketDataArr,
                                                                        hwRangesTypesArr[i],
                                                                        hwRangesTypeIndexArr[i],
                                                                        nodeChildAddressesArr[hwRangesBitLineArr[i]],
                                                                        nodeTotalChildTypesArr[hwRangesBitLineArr[i]],
                                                                        returnOnFailure,
                                                                        &nextNodeType,
                                                                        &nhAdditionalData,
                                                                        &nextBucketPointer,
                                                                        &nhPointer,
                                                                        &leafType,
                                                                        &entryType,
                                                                        &lpmOverEmPriority);
                            if (retVal!=GT_OK)
                            {
                                LPM_SHADOW_HW_SYNC_VALIDATION_ERROR_MAC(returnOnFailure, retVal, "Bucket",
                                            bucketPtr,hwNodeAddr[hwRangesBitLineArr[i]],
                                            "Error: no synchronization between HW and SW - prvCpssDxChLpmRamDbgGetLeafDataSip6\n");
                            }

                            if((GT_U32)swTotalRangesTypesArr[i]==(GT_U32)PRV_CPSS_DXCH_LPM_RAM_TRIE_PTR_TYPE_E)
                            {
                                /* we are dealing with a range that points to a SRC tree that is not a NH (this was dealt with above)
                                   Need to check that the HW is also triggered to MC */

                                if(entryType!=PRV_CPSS_DXCH_LPM_LEAF_ENTRY_TYPE_TRIGGER_E)
                                {
                                    LPM_SHADOW_HW_SYNC_VALIDATION_ERROR_MAC(returnOnFailure, retVal, "Bucket",
                                                bucketPtr,hwNodeAddr[hwRangesBitLineArr[i]],
                                                "Error: no synchronization between HW and SW - Pointer value is not TRIGGER \n");
                                }

                                /* TBD MC is not implemented yet, once implemented code should be rewriten */

                                /* HW offset handle for MC is a special case that we use hwBucketOffsetHandle and not hwGroupOffsetHandle */
                                if (nextBucketPointer != PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(bucketPtr->hwBucketOffsetHandle))
                                {
                                    LPM_SHADOW_HW_SYNC_VALIDATION_ERROR_MAC(returnOnFailure, retVal, "Bucket",
                                                bucketPtr,hwNodeAddr[hwRangesBitLineArr[i]],
                                                "Error: no synchronization between HW and SW - nextBucketPointer \n");
                                }

                                /* nextNodeType can be regular or compress */
                                switch (nextNodeType)
                                {
                                    case 0x0:/* Regular */
                                        if(tempRangePtr->lowerLpmPtr.nextBucket->bucketType!=CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E)
                                        {
                                            LPM_SHADOW_HW_SYNC_VALIDATION_ERROR_MAC(returnOnFailure, retVal, "Bucket",
                                                        bucketPtr,hwNodeAddr[hwRangesBitLineArr[i]],
                                                        "Error: no synchronization between HW and SW - nextBucketPointer \n");
                                        }
                                        break;
                                    case 0x1: /* Compressed */
                                        if((tempRangePtr->lowerLpmPtr.nextBucket->bucketType!=CPSS_DXCH_LPM_COMPRESSED_NODE_PTR_TYPE_E)||
                                           (tempRangePtr->lowerLpmPtr.nextBucket->bucketType!=CPSS_DXCH_LPM_COMPRESSED_UP_TO_7_RANGES_1_LEAF_NODE_PTR_TYPE_E)||
                                           (tempRangePtr->lowerLpmPtr.nextBucket->bucketType!=CPSS_DXCH_LPM_COMPRESSED_UP_TO_5_RANGES_2_LEAVES_NODE_PTR_TYPE_E)||
                                           (tempRangePtr->lowerLpmPtr.nextBucket->bucketType!=CPSS_DXCH_LPM_COMPRESSED_3_RANGES_3_LEAVES_NODE_PTR_TYPE_E))
                                        {
                                            LPM_SHADOW_HW_SYNC_VALIDATION_ERROR_MAC(returnOnFailure, retVal, "Bucket",
                                                        bucketPtr,hwNodeAddr[hwRangesBitLineArr[i]],
                                                        "Error: no synchronization between HW and SW - nextBucketPointer \n");
                                        }
                                        break;
                                    default:
                                        LPM_SHADOW_HW_SYNC_VALIDATION_ERROR_MAC(returnOnFailure, retVal, "Bucket",
                                                    bucketPtr,hwNodeAddr[hwRangesBitLineArr[i]],
                                                    "Error: no synchronization between HW and SW - nextBucketPointer \n");
                                        break;
                                }
                            }
                            else
                            {
                                switch (tempRangePtr->lowerLpmPtr.nextHopEntry->routeEntryMethod)
                                {
                                    case PRV_CPSS_DXCH_LPM_ENTRY_TYPE_MULTIPATH_E:
                                        if (leafType != PRV_CPSS_DXCH_LPM_LEAF_MULTIPATH_TYPE_E)/*Multipath Leaf*/
                                        {
                                            LPM_SHADOW_HW_SYNC_VALIDATION_ERROR_MAC(returnOnFailure, retVal, "Bucket",
                                                            bucketPtr,hwNodeAddr[hwRangesBitLineArr[i]],
                                                            "Error: no synchronization between HW and SW - Pointer value\n");
                                        }
                                        else
                                        {
                                            /* 5. check that if a SW ranges is of type ECMP/QOS then HW range should point to the same NH */
                                            hwNhPointer = nhPointer;

                                            if (tempRangePtr->lowerLpmPtr.nextHopEntry->routeEntryBaseMemAddr != hwNhPointer)
                                            {
                                                LPM_SHADOW_HW_SYNC_VALIDATION_ERROR_MAC(returnOnFailure, retVal, "Bucket",
                                                            bucketPtr,hwNodeAddr[hwRangesBitLineArr[i]],
                                                            "Error: no synchronization between HW and SW - Pointer value\n");
                                            }
                                        }
                                        break;
                                    case PRV_CPSS_DXCH_LPM_ENTRY_TYPE_REGULAR_E:
                                        if (leafType != PRV_CPSS_DXCH_LPM_LEAF_REGULAR_TYPE_ENT)/*Regular Leaf*/
                                        {
                                            LPM_SHADOW_HW_SYNC_VALIDATION_ERROR_MAC(returnOnFailure, retVal, "Bucket",
                                                            bucketPtr,hwNodeAddr[hwRangesBitLineArr[i]],
                                                            "Error: no synchronization between HW and SW - Pointer value\n");
                                        }
                                        else
                                        {
                                            /* 5. check that if a SW ranges is of type NH then HW range should point to the same NH */
                                            hwNhPointer = nhPointer;
                                            if (tempRangePtr->lowerLpmPtr.nextHopEntry->routeEntryBaseMemAddr != hwNhPointer)
                                            {
                                                LPM_SHADOW_HW_SYNC_VALIDATION_ERROR_MAC(returnOnFailure, retVal, "Bucket",
                                                            bucketPtr,hwNodeAddr[hwRangesBitLineArr[i]],
                                                            "Error: no synchronization between HW and SW - Pointer value\n");
                                            }
                                        }
                                        break;
                                    default:
                                        LPM_SHADOW_HW_SYNC_VALIDATION_ERROR_MAC(returnOnFailure, retVal, "Bucket",
                                                        bucketPtr,hwNodeAddr[hwRangesBitLineArr[i]],
                                                        "Error: no synchronization between HW and SW - error in routeEntryMethod \n");
                                        break;
                                }
                            }
                        }
                        break;
                    case PRV_CPSS_DXCH_LPM_CHILD_REGULAR_TYPE_E:
                    case PRV_CPSS_DXCH_LPM_CHILD_COMPRESSED_TYPE_E:
                        /* nothing to check at this moment
                           the node will be checked in the function
                           prvCpssDxChLpmRamDbgRangesHwShadowSyncValidityCheckSip6 */
                        break;
                    default:
                        LPM_SHADOW_HW_SYNC_VALIDATION_ERROR_MAC(returnOnFailure, retVal, "Bucket",
                                            bucketPtr,hwNodeAddr[hwRangesBitLineArr[i]],
                                            "Error: no synchronization between HW and SW - error in HW Pointer Type \n");

                        break;
                    }
                }

                /* if this is a hidden range it will not apear in the tempRangePtr,
                   so need to continue with the same tempRangePtr */
                if ((swTotalRangesIsHiddenArr[i]==GT_FALSE)&&(tempRangePtr->next != NULL))
                {
                    tempRangePtr = tempRangePtr->next;
                }
            }
        }
    }
    if(i != hwNumOfRanges)
    {
        LPM_SHADOW_HW_SYNC_VALIDATION_ERROR_MAC(returnOnFailure, retVal, "Bucket",
                                        bucketPtr,hwNodeAddr[hwRangesBitLineArr[i]],
                                        "Error: no synchronization between HW and SW - illegal ranges values\n");
    }

    /* 6. if the SW range point to a next bucket then recursively check the new bucket, stages 1-5  */
    if (bucketPtr->bucketType != CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E)
    {
        /* Ranges validity check */
        status = prvCpssDxChLpmRamDbgRangesHwShadowSyncValidityCheckSip6(devNum,
                                                             shadowPtr,
                                                             vrId,
                                                             bucketPtr,
                                                             nodeTotalChildTypesArr,
                                                             level,
                                                             numOfMaxAllowedLevels,
                                                             prefixType,
                                                             protocol,
                                                             returnOnFailure);
    }
    if (status != GT_OK)
    {
        retVal = status;
        if (returnOnFailure == GT_TRUE)
        {
            return retVal;
        }
    }

    return retVal;
}


/**
* @internal prvCpssDxChLpmRamDbgBucketHwShadowSyncValidityCheck function
* @endinternal
*
* @brief   Shadow and HW synchronization validation of a bucket
*
* @param[in] devNum                   - The device number
* @param[in] shadowPtr                - pointer to shadow structure
* @param[in] vrId                     - the virtual router id
* @param[in] bucketPtr                - pointer to the bucket
* @param[in] level                    - the  in the tree (first  is 0)
* @param[in] expectedNextPointerType  - the expected type of the bucket, ignored
*                                      for level 0
* @param[in] numOfMaxAllowedLevels    - the maximal allowed number of levels
*                                      for the relevant protocol and prefix type
* @param[in] prefixType               - unicast or multicast tree
* @param[in] protocol                 - protocol
* @param[in] expectedPointingRangeMemAddr - the expected pointingRangeMemAddr field
*                                      of the bucket
* @param[in] returnOnFailure          - GT_TRUE: the validation check stops on
*                                      first failure
*                                      GT_FALSE: continue with the test on
*                                      failure
* @param[in] isRootBucket             - GT_TRUE:the bucketPtr is the root bucket
*                                      GT_FALSE:the bucketPtr is not the root bucket
*
* @retval GT_FAIL                  - on failure
* @retval GT_OK                    - on success
*/
static GT_STATUS prvCpssDxChLpmRamDbgBucketHwShadowSyncValidityCheck
(
    IN  GT_U8                                     devNum,
    IN  PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC          *shadowPtr,
    IN  GT_U32                                    vrId,
    IN  PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC   *bucketPtr,
    IN  GT_U8                                     level,
    IN  CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT       expectedNextPointerType,
    IN  GT_U8                                     numOfMaxAllowedLevels,
    IN  CPSS_UNICAST_MULTICAST_ENT                prefixType,
    IN  PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT      protocol,
    IN  GT_U32                                    expectedPointingRangeMemAddr,
    IN  GT_BOOL                                   returnOnFailure,
    IN  GT_BOOL                                   isRootBucket
)
{
    GT_STATUS status, retVal = GT_OK;
    PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC *tempRangePtr;/* Current range pointer.   */
    GT_U32    hwNumOfRanges;   /* the number of ranges in the HW will be calculated according to the bit vector*/
    GT_U32    hwRanges[256];   /* the ranges values retrieved from the HW */
    GT_U32    *hwBucketDataPtr;/* pointer to all the data retrieved for HW for a specific bucket */
    GT_U32    hwBucketDataArr[PRV_CPSS_DXCH_LPM_RAM_FALCON_MAX_SIZE_OF_BUCKET_IN_LPM_LINES_CNS*PRV_CPSS_DXCH_LPM_RAM_FALCON_SIZE_OF_LPM_ENTRY_IN_WORDS_CNS];
    GT_U32    hwRangeData=0;   /* one range line retrieved from HW */
    GT_U32    hwBitMapSize=0;  /* the size of the bitmap in HW: 1 for comp_1, 2 for comp_2, 11 for regular*/
    GT_U32    hwBitMapLine;    /* one line data from the bit map vector HW value*/
    GT_U32    hwPointerType;   /* The type of the next entry this entry points to. can be Bucket/Regular_Leaf/ECMP_Leaf/QoS_Leaf */
    GT_U32    hwNextBucketType;/* The next bucket (point to by NextBucketPointer) type. can be Regular/One_Compressed/Two_Compressed */
    GT_U32    hwNhPointer;     /* This is the next hop pointer for accessing next hop table or
                                  the pointer for the ECMP table for fetching the ECMP attributes from ECMP table */
    GT_U32    hwAddr = 0;      /* The pointer to the next bucket entry in lines */
    GT_U32    i;
    GT_BOOL   hwNeedToFreeBucketDataPtr=GT_FALSE;
    prvLpmDumpPrintInfo_STC printInfo; /* strcut holds all the printing info */
    GT_U32                              swNodeAddr = 0;/* SW node address kept in shadow(hwBucketOffsetHandle) in line representation from the base */
    GT_U32                              hwNodeAddr = 0;/* HW node address */
    CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT hwNodeType = 0;/* the bucketType retrieved from HW */

     /* if this is a SIP6 architecture the call specific sip6 function */
    if(shadowPtr->shadowType==PRV_CPSS_DXCH_LPM_RAM_SIP6_SHADOW_E)
    {
       cpssOsMemSet(&hwBucketDataArr[0], 0, sizeof(hwBucketDataArr));
       retVal = prvCpssDxChLpmRamDbgBucketHwShadowSyncValidityCheckSip6(devNum,
                                                                       shadowPtr,
                                                                       vrId,
                                                                       bucketPtr,
                                                                       &hwBucketDataArr[0],
                                                                       level,
                                                                       expectedNextPointerType,
                                                                       numOfMaxAllowedLevels,
                                                                       prefixType,
                                                                       protocol,
                                                                       returnOnFailure,
                                                                       isRootBucket);
        return retVal;
    }

    if (isRootBucket==GT_TRUE)
    {
        /* 1. check that the Root SW bucketType is the same as HW bucketType */

        /* read the HW data directly from the VRF table and compare it to the SW data in the shadow */
        status = prvCpssDxChLpmHwVrfEntryRead(devNum, vrId, protocol,
                                          &hwNodeType, &hwNodeAddr);
        if (status != GT_OK)
        {
            cpssOsPrintf("Error on reading Vrf table for vrfId=%d, and protocol=%d\n", vrId, protocol);
            return status;
        }
    }
    else
    {
        /* if this is not a call to the root bucket phase, but a call done from the ranges loop in
           prvCpssDxChLpmRamDbgRangesHwShadowSyncCheck, then we need to take the data address of
           the bucket from the shadow and to retrieved all the HW data */
        if(bucketPtr->bucketType == CPSS_DXCH_LPM_COMPRESSED_1_NODE_PTR_TYPE_E)
        {
            hwBitMapSize = PRV_CPSS_DXCH_LPM_RAM_BUCKET_BIT_VEC_SIZE_COMPRESSED_1_CNS;
        }
        else if(bucketPtr->bucketType == CPSS_DXCH_LPM_COMPRESSED_2_NODE_PTR_TYPE_E)
        {
            hwBitMapSize = PRV_CPSS_DXCH_LPM_RAM_BUCKET_BIT_VEC_SIZE_COMPRESSED_2_CNS;
        }
        else if(bucketPtr->bucketType == CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E)
        {
            hwBitMapSize = PRV_CPSS_DXCH_LPM_RAM_BUCKET_BIT_VEC_SIZE_REGULAR_CNS;
        }
        else /* bucketType = CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E or
                             CPSS_DXCH_LPM_ECMP_ENTRY_PTR_TYPE_E or
                             CPSS_DXCH_LPM_QOS_ENTRY_PTR_TYPE_E */
        {
            /* error - we should not get here in case of a leaf */
            cpssOsPrintf("Error - we should not get here in case of a leaf. vrfId=%d, and protocol=%d\n", vrId, protocol);
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
        }

        hwNodeAddr = PRV_CPSS_DXCH_LPM_RAM_GET_LPM_OFFSET_FROM_DMM_MAC(bucketPtr->hwBucketOffsetHandle);
        hwNodeType = bucketPtr->bucketType;/* we already checked the father nextPointer type to be syncronazid in SW and HW */
    }


    /* check that the HW next bucket type is the same as SW bucketType
       Exception: bucketType of a MC source is different than pointerType of the
       pointing range */
    if(((GT_U32)expectedNextPointerType != (GT_U32)PRV_CPSS_DXCH_LPM_RAM_TRIE_PTR_TYPE_E))
    {
        switch (expectedNextPointerType)
        {
            case CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E:
                if(hwNodeType!=CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E)
                    LPM_SHADOW_HW_SYNC_VALIDATION_ERROR_MAC(returnOnFailure, retVal, "Bucket",
                                                    bucketPtr,hwNodeAddr,
                                                    "Error: no synchronization between HW and SW - NextPointerType\n");
                break;
            case CPSS_DXCH_LPM_COMPRESSED_1_NODE_PTR_TYPE_E:
                if(hwNodeType!=CPSS_DXCH_LPM_COMPRESSED_1_NODE_PTR_TYPE_E)
                    LPM_SHADOW_HW_SYNC_VALIDATION_ERROR_MAC(returnOnFailure, retVal, "Bucket",
                                                    bucketPtr,hwNodeAddr,
                                                    "Error: no synchronization between HW and SW - NextPointerType\n");
                break;
            case CPSS_DXCH_LPM_COMPRESSED_2_NODE_PTR_TYPE_E:
                /* the VR table can only point to regular or compress_1 bucket*/
                if (isRootBucket==GT_TRUE)
                {
                    LPM_SHADOW_HW_SYNC_VALIDATION_ERROR_MAC(returnOnFailure, retVal, "Bucket",
                                                bucketPtr,hwNodeAddr,
                                                "invalid VR bucket type");
                }
                else
                {
                    if(hwNodeType!=CPSS_DXCH_LPM_COMPRESSED_2_NODE_PTR_TYPE_E)
                        LPM_SHADOW_HW_SYNC_VALIDATION_ERROR_MAC(returnOnFailure, retVal, "Bucket",
                                                    bucketPtr,hwNodeAddr,
                                                    "Error: no synchronization between HW and SW - NextPointerType\n");
                }
                break;
            case CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E:
            default:
                LPM_SHADOW_HW_SYNC_VALIDATION_ERROR_MAC(returnOnFailure, retVal, "Bucket",
                                                bucketPtr,hwNodeAddr,
                                                "invalid bucket type");
        }
    }
    else
    {
        /* in case of PRV_CPSS_DXCH_LPM_RAM_TRIE_PTR_TYPE_E need to check HW bit 2 is set.
           In the process of (*,G) lookup.
           When an entry has this bit set, the (*, G) lookup terminates with a match
           at the current entry, and (S, G) SIP based lookup is triggered. */

        status = prvCpssDxChReadTableMultiEntry(devNum,
                                             CPSS_DXCH_SIP5_TABLE_LPM_MEM_E,
                                             expectedPointingRangeMemAddr,
                                             1,
                                             &hwRangeData);
        if (status != GT_OK)
        {
            return status;
        }

        if(((hwRangeData >> 2) & 0x1)!=1)
        {
            LPM_SHADOW_HW_SYNC_VALIDATION_ERROR_MAC(returnOnFailure, retVal, "Bucket",
                                                bucketPtr,hwNodeAddr,
                                                "invalid bucket type");
        }
    }

    if (isRootBucket==GT_TRUE)
    {
        /* 2. check that the Root SW Head Of Trie is the same as HW Head Of Trie */
        swNodeAddr = PRV_CPSS_DXCH_LPM_RAM_GET_LPM_OFFSET_FROM_DMM_MAC(bucketPtr->hwBucketOffsetHandle);
        if(swNodeAddr != hwNodeAddr)
        {
            LPM_SHADOW_HW_SYNC_VALIDATION_ERROR_MAC(returnOnFailure, retVal, "Bucket",
                                            bucketPtr,hwNodeAddr,
                                            "Error: no synchronization between HW and SW - Root Head Of Trie\n");
        }
    }
    else
    {
        /* no need to make the check since in case of non-root bucket we are assigning hwNodeAddr to be the value retrieved from the shadow
           meaning hwNodeAddr is swNodeAddr, we are doing so since we are interested to check the data of this address and not the address itself
           the checking of the address was done in previous recursive call */
    }

    cpssOsMemSet(&printInfo, 0, sizeof(prvLpmDumpPrintInfo_STC));

    printInfo.printRanges = GT_FALSE;

     /* Allocate memory for bucket data. Maximum size is regular bucket that
       uses 256 lpm lines for ranges + 11 lpm lines for bitmap */
    hwBucketDataPtr = cpssOsMalloc(PRV_CPSS_DXCH_LPM_RAM_MAX_SIZE_OF_BUCKET_IN_LPM_LINES_CNS * sizeof(GT_U32));
    if (hwBucketDataPtr == NULL)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);

    cpssOsMemSet(hwBucketDataPtr, 0, PRV_CPSS_DXCH_LPM_RAM_MAX_SIZE_OF_BUCKET_IN_LPM_LINES_CNS * sizeof(GT_U32));
    cpssOsMemSet(hwRanges, 0, sizeof(hwRanges));
    if (isRootBucket==GT_TRUE)
    {
        /* this function retrieves according to the VRF table and the LPM Memory the number of ranges, ranges values, and ranges data */
        status = getRootBucketRanges(devNum, vrId, protocol,
                                  &hwBitMapSize, &hwNumOfRanges, hwRanges,
                                  hwBucketDataPtr, &printInfo);
        if (status != GT_OK)
        {
            cpssOsFree(hwBucketDataPtr);
            return status;
        }
    }
    else
    {
        /* we are dealing with a non-root bucket */

        /* read the bit vector according to the address retrieved from the Shadow bucket */
        status = prvCpssDxChReadTableMultiEntry(devNum,
                                             CPSS_DXCH_SIP5_TABLE_LPM_MEM_E,
                                             hwNodeAddr,
                                             hwBitMapSize,
                                             hwBucketDataPtr);
        if (status != GT_OK)
        {
            cpssOsFree(hwBucketDataPtr);
            return status;
        }

        if (bucketPtr->bucketType == CPSS_DXCH_LPM_COMPRESSED_2_NODE_PTR_TYPE_E)
        {
            /* in case of compress_2 read the 6 range where we have the "Two Compressed Last Offset" value,
               we need this data for the number of ranges calculation in getNumOfRangesFromHW.
               this value is keep in the father of this bucket */
            status = prvCpssDxChReadTableMultiEntry(devNum,
                                             CPSS_DXCH_SIP5_TABLE_LPM_MEM_E,
                                             expectedPointingRangeMemAddr,
                                             1,
                                             &hwRangeData);
            if (status != GT_OK)
            {
                cpssOsFree(hwBucketDataPtr);
                return status;
            }
        }

        hwNumOfRanges = getNumOfRangesFromHW(bucketPtr->bucketType, hwBucketDataPtr, hwRangeData);

        /* calculate HW ranges values according to bit vector */
        switch (bucketPtr->bucketType)
        {
            case (CPSS_DXCH_LPM_COMPRESSED_1_NODE_PTR_TYPE_E):
                hwBitMapLine = hwBucketDataPtr[0]; /* bitmap pointer */

                /* Bitmap has 4 bytes [range_4_start ; range_3_start ; range_2_start ; range_1_start]
                   Range 0 start is always 0 (so no need to add to bitmap) */
                hwRanges[1] = hwBitMapLine & 0xFF;
                hwRanges[2] = (hwBitMapLine >> 8) & 0xFF;
                hwRanges[3] = (hwBitMapLine >> 16) & 0xFF;
                hwRanges[4] = (hwBitMapLine >> 24) & 0xFF;
                break;

            case (CPSS_DXCH_LPM_COMPRESSED_2_NODE_PTR_TYPE_E):
                hwBitMapLine = hwBucketDataPtr[0]; /* bitmap 1 pointer */

                /* Bitmap has 9 bytes; Word 1: [range_4_start ; range_3_start ; range_2_start ; range_1_start]
                                       Word 2: [range_9_start ; range_8_start ; range_7_start ; range_6_start]
                                        - range_5_start is carried from the pointer to this bucket
                                        - range_0_start is always 0 (so no need to add to bitmap) */
                hwRanges[1] = hwBitMapLine & 0xFF;
                hwRanges[2] = (hwBitMapLine >> 8) & 0xFF;
                hwRanges[3] = (hwBitMapLine >> 16) & 0xFF;
                hwRanges[4] = (hwBitMapLine >> 24) & 0xFF;
                hwRanges[5] = (hwRangeData >> 5) & 0xFF;

                hwBitMapLine = hwBucketDataPtr[1*PRV_CPSS_DXCH_LPM_RAM_SIZE_OF_LPM_ENTRY_IN_WORDS_CNS]; /* bitmap 2 pointer */

                hwRanges[6] = hwBitMapLine & 0xFF;
                hwRanges[7] = (hwBitMapLine >> 8) & 0xFF;
                hwRanges[8] = (hwBitMapLine >> 16) & 0xFF;
                hwRanges[9] = (hwBitMapLine >> 24) & 0xFF;

                break;

            case (CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E):
                for (i = 0; i < hwNumOfRanges; i++)
                {
                    hwRanges[i] = (GT_U8)getBaseAddrFromHW(CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E, i, hwBucketDataPtr, 0);
                }
                break;

            case (CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E):
            case (CPSS_DXCH_LPM_ECMP_ENTRY_PTR_TYPE_E):
            case (CPSS_DXCH_LPM_QOS_ENTRY_PTR_TYPE_E):
            default:
                break;
        }

        /* read all the ranges (the LPM lines after the bit vector) */
        status = prvCpssDxChReadTableMultiEntry(devNum,
                                         CPSS_DXCH_SIP5_TABLE_LPM_MEM_E,
                                         hwNodeAddr + hwBitMapSize, hwNumOfRanges,
                                         &hwBucketDataPtr[hwBitMapSize * PRV_CPSS_DXCH_LPM_RAM_SIZE_OF_LPM_ENTRY_IN_WORDS_CNS]);
        if (status != GT_OK)
        {
            cpssOsFree(hwBucketDataPtr);
            return status;
        }
    }

    /* if an error accur in getRootBucketRanges then bucketDataPtr was already free */
    if(numOfErrors == 0)
        hwNeedToFreeBucketDataPtr = GT_TRUE;

    /* 3. check that the SW ranges is the same as HW bitVector values */
    if(bucketPtr->numOfRanges!=hwNumOfRanges)
    {
        if ((returnOnFailure == GT_TRUE)&&(hwNeedToFreeBucketDataPtr == GT_TRUE))
        {
            cpssOsFree(hwBucketDataPtr);
        }
        LPM_SHADOW_HW_SYNC_VALIDATION_ERROR_MAC(returnOnFailure, retVal, "Bucket",
                                        bucketPtr,hwNodeAddr,
                                        "Error: no synchronization between HW and SW - Root num of ranges\n");
    }

    /* 4. check that the SW ranges values is the same as HW ranges values */
    tempRangePtr = bucketPtr->rangeList;
    for (i=0; (i<hwNumOfRanges && tempRangePtr!=NULL); i++)
    {
        if(tempRangePtr->startAddr != hwRanges[i])
        {
            if ((returnOnFailure == GT_TRUE)&&(hwNeedToFreeBucketDataPtr == GT_TRUE))
            {
                cpssOsFree(hwBucketDataPtr);
            }
            LPM_SHADOW_HW_SYNC_VALIDATION_ERROR_MAC(returnOnFailure, retVal, "Bucket",
                                        bucketPtr,hwNodeAddr,
                                        "Error: no synchronization between HW and SW - ranges values\n");
        }
        else
        {
            /* range value is synchronized between HW and SW so now check that the SW range type
               is the same as HW range type, if it is the same the compare the range pointer data */
            hwPointerType = (hwBucketDataPtr[i+hwBitMapSize]) & 0x3;
            hwNextBucketType = (hwBucketDataPtr[i+hwBitMapSize] >> 3) & 0x3;

            if(((tempRangePtr->pointerType <= CPSS_DXCH_LPM_COMPRESSED_2_NODE_PTR_TYPE_E)&&(hwPointerType!=0))||
               ((tempRangePtr->pointerType==CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E)&&(hwPointerType!=1))||
               ((tempRangePtr->pointerType==CPSS_DXCH_LPM_ECMP_ENTRY_PTR_TYPE_E)&&(hwPointerType!=2))||
               ((tempRangePtr->pointerType==CPSS_DXCH_LPM_QOS_ENTRY_PTR_TYPE_E)&&(hwPointerType!=3)))
            {
                if ((returnOnFailure == GT_TRUE)&&(hwNeedToFreeBucketDataPtr == GT_TRUE))
                {
                    cpssOsFree(hwBucketDataPtr);
                }
                LPM_SHADOW_HW_SYNC_VALIDATION_ERROR_MAC(returnOnFailure, retVal, "Bucket",
                                        bucketPtr,hwNodeAddr,
                                        "Error: no synchronization between HW and SW - range Pointer Type value\n");
            }
            else
            {
                /* If the pointerType is to a MC source and the MC source bucket is a regular bucket then it means that
                   this is a root of MC source tree that points directly to a nexthop or ECMP/QoS entry */
                if (((GT_U32)tempRangePtr->pointerType==(GT_U32)PRV_CPSS_DXCH_LPM_RAM_TRIE_PTR_TYPE_E) &&
                    (tempRangePtr->lowerLpmPtr.nextBucket->bucketType==CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E))
                {
                    switch (tempRangePtr->lowerLpmPtr.nextBucket->rangeList->lowerLpmPtr.nextHopEntry->routeEntryMethod)
                    {
                        case PRV_CPSS_DXCH_LPM_ENTRY_TYPE_QOS_E:
                        case PRV_CPSS_DXCH_LPM_ENTRY_TYPE_ECMP_E:
                            /* 5. check that if a SW ranges is of type ECMP/QOS then HW range should point to the same NH */
                            hwNhPointer = ((hwBucketDataPtr[i+hwBitMapSize] >> 7) & 0x00003FFF);
                            if (tempRangePtr->lowerLpmPtr.nextBucket->rangeList->lowerLpmPtr.nextHopEntry->routeEntryBaseMemAddr != hwNhPointer)
                            {
                                if ((returnOnFailure == GT_TRUE)&&(hwNeedToFreeBucketDataPtr == GT_TRUE))
                                {
                                    cpssOsFree(hwBucketDataPtr);
                                }
                                LPM_SHADOW_HW_SYNC_VALIDATION_ERROR_MAC(returnOnFailure, retVal, "Bucket",
                                            bucketPtr,hwNodeAddr,
                                            "Error: no synchronization between HW and SW - Pointer value\n");
                            }
                            break;
                        case PRV_CPSS_DXCH_LPM_ENTRY_TYPE_REGULAR_E:
                            /* 5. check that if a SW ranges is of type NH then HW range should point to the same NH */
                            hwNhPointer = ((hwBucketDataPtr[i+hwBitMapSize] >> 7) & 0x00007FFF);
                            if (tempRangePtr->lowerLpmPtr.nextBucket->rangeList->lowerLpmPtr.nextHopEntry->routeEntryBaseMemAddr != hwNhPointer)
                            {
                                if ((returnOnFailure == GT_TRUE)&&(hwNeedToFreeBucketDataPtr == GT_TRUE))
                                {
                                    cpssOsFree(hwBucketDataPtr);
                                }
                                LPM_SHADOW_HW_SYNC_VALIDATION_ERROR_MAC(returnOnFailure, retVal, "Bucket",
                                            bucketPtr,hwNodeAddr,
                                            "Error: no synchronization between HW and SW - Pointer value\n");
                            }
                            break;
                        default:
                            if ((returnOnFailure == GT_TRUE)&&(hwNeedToFreeBucketDataPtr == GT_TRUE))
                            {
                                cpssOsFree(hwBucketDataPtr);
                            }
                            LPM_SHADOW_HW_SYNC_VALIDATION_ERROR_MAC(returnOnFailure, retVal, "Bucket",
                                            bucketPtr,hwNodeAddr,
                                            "Error: no synchronization between HW and SW - error in routeEntryMethod \n");
                            break;
                    }
                }
                else
                {
                    switch (hwPointerType)
                    {
                        case 0:/* pointing to bucket */
                            /* check the bucket type: regular/comp_1/comp_2 */
                            switch(hwNextBucketType)
                            {
                                case 0: /* CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E */
                                case 1: /* CPSS_DXCH_LPM_COMPRESSED_1_NODE_PTR_TYPE_E */
                                     /* hw pointer in LPM entry is in LPM lines*/
                                    hwAddr = ((hwBucketDataPtr[i+hwBitMapSize] & 0x1FFFFFE0) >> 5);
                                    break;
                                case 2: /* CPSS_DXCH_LPM_COMPRESSED_2_NODE_PTR_TYPE_E */
                                    hwAddr = (hwBucketDataPtr[i+hwBitMapSize] >> 13);
                                    break;
                                default:
                                    if ((returnOnFailure == GT_TRUE)&&(hwNeedToFreeBucketDataPtr == GT_TRUE))
                                    {
                                        cpssOsFree(hwBucketDataPtr);
                                    }
                                    LPM_SHADOW_HW_SYNC_VALIDATION_ERROR_MAC(returnOnFailure, retVal, "Bucket",
                                                        bucketPtr,hwNodeAddr,
                                                        "Error: no synchronization between HW and SW - error in Next Bucket Pointer Type \n");
                            }
                            /* check the bucket address */
                            if (hwAddr!=PRV_CPSS_DXCH_LPM_RAM_GET_LPM_OFFSET_FROM_DMM_MAC(tempRangePtr->lowerLpmPtr.nextBucket->hwBucketOffsetHandle))
                            {
                                if ((returnOnFailure == GT_TRUE)&&(hwNeedToFreeBucketDataPtr == GT_TRUE))
                                {
                                    cpssOsFree(hwBucketDataPtr);
                                }
                                LPM_SHADOW_HW_SYNC_VALIDATION_ERROR_MAC(returnOnFailure, retVal, "Bucket",
                                                        bucketPtr,hwNodeAddr,
                                                        "Error: no synchronization between HW and SW - error in Next Bucket Pointer Adress\n");
                            }
                            break;
                        case 0x1:/* CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E */
                            /* 5. check that if a SW ranges is of type NH then HW range should point to the same NH */
                            hwNhPointer = ((hwBucketDataPtr[i+hwBitMapSize] >> 7) & 0x00007FFF);
                            if (tempRangePtr->lowerLpmPtr.nextHopEntry->routeEntryBaseMemAddr != hwNhPointer)
                            {
                                if ((returnOnFailure == GT_TRUE)&&(hwNeedToFreeBucketDataPtr == GT_TRUE))
                                {
                                    cpssOsFree(hwBucketDataPtr);
                                }
                                LPM_SHADOW_HW_SYNC_VALIDATION_ERROR_MAC(returnOnFailure, retVal, "Bucket",
                                            bucketPtr,hwNodeAddr,
                                            "Error: no synchronization between HW and SW - Pointer value\n");
                            }
                            break;
                        case 0x2:/* CPSS_DXCH_LPM_ECMP_ENTRY_PTR_TYPE_E */
                        case 0x3:/* CPSS_DXCH_LPM_QOS_ENTRY_PTR_TYPE_E */

                            /* 5. check that if a SW ranges is of type ECMP/QOS then HW range should point to the same NH */
                            hwNhPointer = ((hwBucketDataPtr[i+hwBitMapSize] >> 7) & 0x00003FFF);
                            if (tempRangePtr->lowerLpmPtr.nextHopEntry->routeEntryBaseMemAddr != hwNhPointer)
                            {
                                if ((returnOnFailure == GT_TRUE)&&(hwNeedToFreeBucketDataPtr == GT_TRUE))
                                {
                                    cpssOsFree(hwBucketDataPtr);
                                }
                                LPM_SHADOW_HW_SYNC_VALIDATION_ERROR_MAC(returnOnFailure, retVal, "Bucket",
                                            bucketPtr,hwNodeAddr,
                                            "Error: no synchronization between HW and SW - Pointer value\n");
                            }
                            break;
                        default:
                            if ((returnOnFailure == GT_TRUE)&&(hwNeedToFreeBucketDataPtr == GT_TRUE))
                            {
                                cpssOsFree(hwBucketDataPtr);
                            }
                            LPM_SHADOW_HW_SYNC_VALIDATION_ERROR_MAC(returnOnFailure, retVal, "Bucket",
                                            bucketPtr,hwNodeAddr,
                                            "Error: no synchronization between HW and SW - error in HW Bucket Pointer Type\n");
                            break;

                    }
                }

                if (tempRangePtr->next != NULL)
                    tempRangePtr = tempRangePtr->next;
                else
                    break;
            }
        }
    }
    if(i != (hwNumOfRanges-1))
    {
        if ((returnOnFailure == GT_TRUE)&&(hwNeedToFreeBucketDataPtr == GT_TRUE))
        {
            cpssOsFree(hwBucketDataPtr);
        }
        LPM_SHADOW_HW_SYNC_VALIDATION_ERROR_MAC(returnOnFailure, retVal, "Bucket",
                                        bucketPtr,hwNodeAddr,
                                        "Error: no synchronization between HW and SW - ranges values\n");
    }

    /* 6. if the SW range point to a next bucket then recursively check the new bucket, stages 1-5  */
    if (bucketPtr->bucketType != CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E)
    {
        /* Ranges validity check */
        status = prvCpssDxChLpmRamDbgRangesHwShadowSyncValidityCheck(devNum,
                                                             shadowPtr,
                                                             vrId,
                                                             bucketPtr,
                                                             level,
                                                             numOfMaxAllowedLevels,
                                                             prefixType,
                                                             protocol,
                                                             returnOnFailure);
    }
    if (status != GT_OK)
    {
        retVal = status;
        if (returnOnFailure == GT_TRUE)
        {
            if(hwNeedToFreeBucketDataPtr == GT_TRUE)
            {
                cpssOsFree(hwBucketDataPtr);
            }
            return retVal;
        }
    }

    if(hwNeedToFreeBucketDataPtr == GT_TRUE)
    {
        cpssOsFree(hwBucketDataPtr);
    }

    return retVal;
}

/**
* @internal prvCpssDxChLpmRamDbgHwShadowSyncValidityCheck function
* @endinternal
*
* @brief   This function validates synchronization between the SW and HW of the LPM
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
*
* @param[in] devNum                   - The device number
* @param[in] lpmDbPtr                 - LPM DB
* @param[in] vrId                     - virtual router id, 256 means "all vrIds"
* @param[in] protocolBitmap           - protocols bitmap
* @param[in] prefixType               - UC/MC/both prefix type
* @param[in] returnOnFailure          - GT_TRUE: the validation check stops at first failure
*                                      GT_FALSE: continue with the test on failure
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on illegal input parameter/s
* @retval GT_NOT_FOUND             - LPM DB was not found
* @retval GT_NOT_INITIALIZED       - LPM DB is not initialized
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmRamDbgHwShadowSyncValidityCheck
(
    IN GT_U8                                    devNum,
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC     *lpmDbPtr,
    IN GT_U32                                   vrId,
    IN PRV_CPSS_DXCH_LPM_PROTOCOL_BMP           protocolBitmap,
    IN CPSS_UNICAST_MULTICAST_ENT               prefixType,
    IN GT_BOOL                                  returnOnFailure
)
{
    PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC  *bucketPtr;
    CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT      bucketType;
    GT_U32                                   shadowIdx, vrIdStartIdx, vrIdEndIdx;
    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC         *shadowPtr;
    GT_U8                                    numOfMaxAllowedLevels;
    GT_STATUS                                status, rc = GT_OK;

    switch(prefixType)
    {
        case CPSS_UNICAST_E:
        case CPSS_MULTICAST_E:
        case CPSS_UNICAST_MULTICAST_E:
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if ((PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_STATUS_GET_MAC(protocolBitmap) == GT_FALSE) &&
        (PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_STATUS_GET_MAC(protocolBitmap) == GT_FALSE) &&
        (PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_STATUS_GET_MAC(protocolBitmap) == GT_FALSE))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* FCoE is unicast only */
    if ((PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_STATUS_GET_MAC(protocolBitmap)) &&
         (prefixType != CPSS_UNICAST_E))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* if initialization has not been done for the requested protocol stack -
       return error */
    if ((PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_STATUS_GET_MAC(protocolBitmap)) &&
        (PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_STATUS_GET_MAC(lpmDbPtr->protocolBitmap) == GT_FALSE))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }
    else if ((PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_STATUS_GET_MAC(protocolBitmap)) &&
             (PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_STATUS_GET_MAC(lpmDbPtr->protocolBitmap) == GT_FALSE))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }
    else if ((PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_STATUS_GET_MAC(protocolBitmap)) &&
             (PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_STATUS_GET_MAC(lpmDbPtr->protocolBitmap) == GT_FALSE))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    for (shadowIdx = 0; shadowIdx < lpmDbPtr->numOfShadowCfg; shadowIdx++)
    {
        shadowPtr = &lpmDbPtr->shadowArray[shadowIdx];
        if ((vrId >= shadowPtr->vrfTblSize) && (vrId != 4096))
        {
            /* Illegal virtual router id */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        if (PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_STATUS_GET_MAC(protocolBitmap))
        {
            if (shadowPtr->isProtocolInitialized[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E] == GT_FALSE)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
            }
        }
        if (PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_STATUS_GET_MAC(protocolBitmap))
        {
            if (shadowPtr->isProtocolInitialized[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E] == GT_FALSE)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
            }
        }
        if (PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_STATUS_GET_MAC(protocolBitmap))
        {
            if (shadowPtr->isProtocolInitialized[PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E] == GT_FALSE)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
            }
        }

        if (vrId == 4096)
        {
            vrIdStartIdx = 0;
            vrIdEndIdx = shadowPtr->vrfTblSize - 1;
        }
        else
        {
            vrIdStartIdx = vrIdEndIdx = vrId;
        }

        for (vrId = vrIdStartIdx; vrId <= vrIdEndIdx; vrId++)
        {
            if (shadowPtr->vrRootBucketArray[vrId].valid == 0)
            {
                continue;
            }

            if ((prefixType == CPSS_UNICAST_E) || (prefixType == CPSS_UNICAST_MULTICAST_E))
            {
                if (PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_STATUS_GET_MAC(protocolBitmap))
                {
                    bucketPtr = shadowPtr->vrRootBucketArray[vrId].rootBucket[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E];
                    bucketType = shadowPtr->vrRootBucketArray[vrId].rootBucketType[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E];
                    numOfMaxAllowedLevels = PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS;
                    status = prvCpssDxChLpmRamDbgBucketHwShadowSyncValidityCheck(devNum,
                                                                         shadowPtr,
                                                                         vrId,
                                                                         bucketPtr,
                                                                         0,
                                                                         bucketType,
                                                                         numOfMaxAllowedLevels,
                                                                         CPSS_UNICAST_E,
                                                                         PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E,
                                                                         0,
                                                                         returnOnFailure,
                                                                         GT_TRUE);
                    if (status != GT_OK)
                    {
                        rc = status;
                        if (returnOnFailure == GT_TRUE)
                        {
                            return rc;
                        }
                    }
                }
                if (PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_STATUS_GET_MAC(protocolBitmap))
                {
                    bucketPtr = shadowPtr->vrRootBucketArray[vrId].rootBucket[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E];
                    bucketType = shadowPtr->vrRootBucketArray[vrId].rootBucketType[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E];
                    numOfMaxAllowedLevels = PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS;
                    status = prvCpssDxChLpmRamDbgBucketHwShadowSyncValidityCheck(devNum,
                                                                         shadowPtr,
                                                                         vrId,
                                                                         bucketPtr,
                                                                         0,
                                                                         bucketType,
                                                                         numOfMaxAllowedLevels,
                                                                         CPSS_UNICAST_E,
                                                                         PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E,
                                                                         0,
                                                                         returnOnFailure,
                                                                         GT_TRUE);
                    if (status != GT_OK)
                    {
                        rc = status;
                        if (returnOnFailure == GT_TRUE)
                        {
                            return rc;
                        }
                    }
                }

                if ((PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_STATUS_GET_MAC(protocolBitmap))&&(prefixType == CPSS_UNICAST_E))
                {
                    bucketPtr = shadowPtr->vrRootBucketArray[vrId].rootBucket[PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E];
                    bucketType = shadowPtr->vrRootBucketArray[vrId].rootBucketType[PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E];
                    numOfMaxAllowedLevels = PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_FCOE_PROTOCOL_CNS;
                    status = prvCpssDxChLpmRamDbgBucketHwShadowSyncValidityCheck(devNum,
                                                                         shadowPtr,
                                                                         vrId,
                                                                         bucketPtr,
                                                                         0,
                                                                         bucketType,
                                                                         numOfMaxAllowedLevels,
                                                                         CPSS_UNICAST_E,
                                                                         PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E,
                                                                         0,
                                                                         returnOnFailure,
                                                                         GT_TRUE);
                    if (status != GT_OK)
                    {
                        rc = status;
                        if (returnOnFailure == GT_TRUE)
                        {
                            return rc;
                        }
                    }
                }
            }
            if ((prefixType == CPSS_MULTICAST_E) || (prefixType == CPSS_UNICAST_MULTICAST_E))
            {
                if (PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_STATUS_GET_MAC(protocolBitmap))
                {
                    bucketPtr = shadowPtr->vrRootBucketArray[vrId].rootBucket[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E];
                    bucketType = shadowPtr->vrRootBucketArray[vrId].rootBucketType[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E];
                    numOfMaxAllowedLevels = 8;
                    status = prvCpssDxChLpmRamDbgBucketHwShadowSyncValidityCheck(devNum,
                                                                         shadowPtr,
                                                                         vrId,
                                                                         bucketPtr,
                                                                         0,
                                                                         bucketType,
                                                                         numOfMaxAllowedLevels,
                                                                         CPSS_MULTICAST_E,
                                                                         PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E,
                                                                         0,
                                                                         returnOnFailure,
                                                                         GT_TRUE);
                    if (status != GT_OK)
                    {
                        rc = status;
                        if (returnOnFailure == GT_TRUE)
                        {
                            return rc;
                        }
                    }
                }
                if (PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_STATUS_GET_MAC(protocolBitmap))
                {
                    bucketPtr = shadowPtr->vrRootBucketArray[vrId].rootBucket[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E];
                    bucketType = shadowPtr->vrRootBucketArray[vrId].rootBucketType[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E];
                    numOfMaxAllowedLevels =  (2 * PRV_CPSS_DXCH_LPM_MAX_NUM_OF_OCTETS_FOR_ADDRESS_CNS);
                    status = prvCpssDxChLpmRamDbgBucketHwShadowSyncValidityCheck(devNum,
                                                                         shadowPtr,
                                                                         vrId,
                                                                         bucketPtr,
                                                                         0,
                                                                         bucketType,
                                                                         numOfMaxAllowedLevels,
                                                                         CPSS_MULTICAST_E,
                                                                         PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E,
                                                                         0,
                                                                         returnOnFailure,
                                                                         GT_TRUE);
                    if (status != GT_OK)
                    {
                        rc = status;
                        if (returnOnFailure == GT_TRUE)
                        {
                            return rc;
                        }
                    }
                }
            }
        }
    }
    return rc;
}

/**
* @internal prvCpssDxChLpmRamDbgHwOctetsToBlockMappingInfoPrint function
* @endinternal
*
* @brief   Print Octet to Block mapping debug information for a specific LPM DB
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - otherwise
* @retval GT_NOT_FOUND             - if can't find the lpm DB
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note none.
*
*/
GT_STATUS prvCpssDxChLpmRamDbgHwOctetsToBlockMappingInfoPrint
(
    IN GT_U32                           lpmDbId
)
{
    PRV_CPSS_DXCH_LPM_SHADOW_STC        *lpmDbPtr,tmpLpmDb;
    PRV_CPSS_DXCH_LPM_HW_ENT            lpmHw;
    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC    *shadowPtr;
    PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC *shadowsPtr;
    GT_U32      i,k;

    tmpLpmDb.lpmDbId = lpmDbId;
    lpmDbPtr = prvCpssSlSearch(prvCpssDxChIplpmDbSL,&tmpLpmDb);
    if (lpmDbPtr == NULL)
    {
        /* can't find the lpm DB */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }

    lpmHw = prvCpssDxChLpmGetHwType(lpmDbPtr->shadowType);
    if (lpmHw == PRV_CPSS_DXCH_LPM_HW_TCAM_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }

    shadowsPtr = (PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC*)(lpmDbPtr->shadow);
    for (k = 0; k < shadowsPtr->numOfShadowCfg; k++ )
    {
        shadowPtr =  &(shadowsPtr->shadowArray[k]);

        if ((shadowPtr->isProtocolInitialized[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E] == GT_FALSE) &&
            (shadowPtr->isProtocolInitialized[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E] == GT_FALSE) &&
            (shadowPtr->isProtocolInitialized[PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E] == GT_FALSE))
            continue;

        cpssOsPrintf("\nLPM RAM SHADOW ID [%d] GENERAL HW INFO\n\n",k);
        cpssOsPrintf("NumEntriesBetweenBlocks 0x%x\t BlocksAllocationMethod %s\n",shadowPtr->lpmRamTotalBlocksSizeIncludingGap,
                     (shadowPtr->lpmRamBlocksAllocationMethod == PRV_CPSS_DXCH_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITHOUT_BLOCK_SHARING_E ? " NO_SHARING" : " WITH_SHARING"));
        for (i = 0; i < shadowPtr->numOfLpmMemories; i++)
        {
            cpssOsPrintf("MemBlock[%d]\t BlocksSize 0x%x\t memPoolId 0x%x\t isBlockUsed   %s\t  octetsToBlockMappingBitmap:\t IPv4 0x%x     IPv6 0x%x     FCOE 0x%x\n",
                         i,
                         shadowPtr->lpmRamBlocksSizeArrayPtr[i],
                         shadowPtr->lpmRamStructsMemPoolPtr[i],
                         (shadowPtr->lpmRamOctetsToBlockMappingPtr[i].isBlockUsed ? " YES" : " NO"),
                         shadowPtr->lpmRamOctetsToBlockMappingPtr[i].octetsToBlockMappingBitmap[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E],
                         shadowPtr->lpmRamOctetsToBlockMappingPtr[i].octetsToBlockMappingBitmap[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E],
                         shadowPtr->lpmRamOctetsToBlockMappingPtr[i].octetsToBlockMappingBitmap[PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E]);
        }
        cpssOsPrintf("------------------------------------------------------------------------------------------------\n");
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChLpmDbgHwOctetPerBlockPrint function
* @endinternal
*
* @brief   Print Octet per Block debug information for a specific LPM DB
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*                                       None.
*/
GT_VOID prvCpssDxChLpmDbgHwOctetPerBlockPrint
(
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC      *lpmDbPtr
)
{
    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC  *shadowPtr;
    GT_U32 octetsToBlockBitmap;
    GT_U32 i,j,octetId;
    static GT_CHAR *protocolNames[] = {"IPv4","IPv6","FCOE"};

    shadowPtr = &lpmDbPtr->shadowArray[0];

    for ( i = 0 ; i < shadowPtr->numOfLpmMemories ; i++ )
    {
        if (shadowPtr->lpmRamOctetsToBlockMappingPtr[i].isBlockUsed==GT_TRUE)
        {
            cpssOsPrintf("\nBlock [%d]\n",i);
        }
        for (j = 0; j < PRV_CPSS_DXCH_LPM_PROTOCOL_LAST_E; j++)
        {
            octetsToBlockBitmap = shadowPtr->lpmRamOctetsToBlockMappingPtr[i].octetsToBlockMappingBitmap[j];
            if (octetsToBlockBitmap)
            {
               cpssOsPrintf("\tprotocol   %s :", protocolNames[j]);
            }
            else
            {
                continue;
            }

            octetId = 0;
            while (octetsToBlockBitmap)
            {
                if (octetsToBlockBitmap & 0x1)
                    cpssOsPrintf("\t Octet [%d]     ",octetId);

                octetsToBlockBitmap >>= 1;
                octetId++;
            }
            cpssOsPrintf("\n");
        }
    }

    return;
}


/**
* @internal prvCpssDxChLpmDbgHwOctetPerProtocolPrint function
* @endinternal
*
* @brief   Print Octet per Protocol debug information for a specific LPM DB
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*                                       None.
*/
GT_VOID prvCpssDxChLpmDbgHwOctetPerProtocolPrint
(
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC      *lpmDbPtr
)
{
    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC  *shadowPtr;
    GT_U32 octetsToBlockBitmap;
    GT_U32 i,j,octetId;
    static GT_CHAR *protocolNames[] = {"IPv4","IPv6","FCOE"};

    shadowPtr = &lpmDbPtr->shadowArray[0];

    for (j = 0; j < PRV_CPSS_DXCH_LPM_PROTOCOL_LAST_E; j++)
    {
        cpssOsPrintf("\nprotocol   %s : ",protocolNames[j]);
        for ( i = 0 ; i < shadowPtr->numOfLpmMemories ; i++ )
        {
            octetsToBlockBitmap = shadowPtr->lpmRamOctetsToBlockMappingPtr[i].octetsToBlockMappingBitmap[j];
            if (octetsToBlockBitmap)
                cpssOsPrintf("\nBlock [%d]",i);
            else
                continue;
            octetId = 0;
            while (octetsToBlockBitmap)
            {
                if (octetsToBlockBitmap & 0x1)
                    cpssOsPrintf("\t Octet [%d]",octetId);

                octetsToBlockBitmap >>= 1;
                octetId++;
            }
        }
        cpssOsPrintf("\n");
    }

    return;
}

/**
* @internal prvCpssDxChLpmDbgHwOctetPerProtocolPrintLpmLinesCounters function
* @endinternal
*
* @brief   Print Octet per Protocol LPM lines debug information for a specific LPM DB
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*                                       None.
*/
GT_VOID prvCpssDxChLpmDbgHwOctetPerProtocolPrintLpmLinesCounters
(
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC      *lpmDbPtr
)
{
    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC  *shadowPtr;
    GT_U32 i;
    PRV_CPSS_DXCH_LPM_ADDRESS_COUNTERS_INFO_STC protocolCountersPerBlockElement;

    shadowPtr = &lpmDbPtr->shadowArray[0];

    for ( i = 0 ; i < shadowPtr->numOfLpmMemories ; i++ )
    {
        protocolCountersPerBlockElement = shadowPtr->protocolCountersPerBlockArr[i];
        cpssOsPrintf("\nBlock number = [%d]",i);
        cpssOsPrintf("\n num of Ipv4 LPM lines allocted in the block [%d]",protocolCountersPerBlockElement.sumOfIpv4Counters);
        cpssOsPrintf("\n num of Ipv6 LPM lines allocted in the block  [%d]",protocolCountersPerBlockElement.sumOfIpv6Counters);
        cpssOsPrintf("\n num of FCOE LPM lines allocted in the block  [%d]",protocolCountersPerBlockElement.sumOfFcoeCounters);
    }
    cpssOsPrintf("\n");

    return;
}
/**
* @internal prvCpssDxChLpmDbgHwOctetPerProtocolPrintLpmMemoryAndCounters function
* @endinternal
*
* @brief   Print Octet per Protocol LPM lines and Banks memory debug information for a
*         specific LPM DB
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*                                       None.
*/
GT_VOID prvCpssDxChLpmDbgHwOctetPerProtocolPrintLpmMemoryAndCounters
(
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC      *lpmDbPtr
)
{
    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC  *shadowPtr;
    GT_U32 i;
    PRV_CPSS_DXCH_LPM_ADDRESS_COUNTERS_INFO_STC protocolCountersPerBlockElement;
    GT_UINTPTR  partitionId;

    shadowPtr = &lpmDbPtr->shadowArray[0];

    for ( i = 0 ; i < shadowPtr->numOfLpmMemories ; i++ )
    {
        partitionId = shadowPtr->lpmRamStructsMemPoolPtr[i];
        protocolCountersPerBlockElement = shadowPtr->protocolCountersPerBlockArr[i];
        if( partitionId != 0)
        {
            prvCpssDmmPartitionPrint(partitionId);

            cpssOsPrintf("\n\n Block number = [%d], Block partitionId = [0x%X]",i, partitionId);
            cpssOsPrintf("\n num of Ipv4 LPM lines allocted in the block [%d]",protocolCountersPerBlockElement.sumOfIpv4Counters);
            cpssOsPrintf("\n num of Ipv6 LPM lines allocted in the block  [%d]",protocolCountersPerBlockElement.sumOfIpv6Counters);
            cpssOsPrintf("\n num of FCOE LPM lines allocted in the block  [%d]",protocolCountersPerBlockElement.sumOfFcoeCounters);
        }
        else
        {
            cpssOsPrintf("\n partitionId is 0 ");
        }
    }
    return;
}

