/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
*
*/
/**
********************************************************************************
* @file cpssDxChTrunk.c
*
* @brief API definitions for 802.3ad Link Aggregation (Trunk) facility
* CPSS - DxCh
*
* States and Modes :
* A. Application can work in one of 2 modes:
* 1. use "high level" trunk manipulations set of APIs - Mode "HL"
* 2. use "low level" trunk HW tables/registers set of APIs - MODE "LL"
*
* B. Using "high level" trunk manipulations set of APIs - "High level"
* Mode.
* In this mode the CPSS synchronize all the relevant trunk
* tables/registers, implement some WA for trunks Errata.
* 1. Create trunk
* 2. Destroy trunk
* 3. Add/remove member to/from trunk
* 4. Enable/disable member in trunk
* 5. Add/Remove port to/from "non-trunk" entry
* 6. set/unset designated member for Multi-destination traffic and
* traffic sent to Cascade Trunks
* C. Using "low level" trunk HW tables/registers set of APIs- "Low level"
* Mode.
* In this mode the CPSS allow direct access to trunk tables/registers.
* In this mode the Application required to implement the
* synchronization between the trunk tables/registers and to implement
* the WA to some of the trunk Errata.
*
* 1. Set per port the trunkId
* 2. Set per trunk the trunk members , number of members
* 3. Set per trunk the Non-trunk local ports bitmap
* 4. Set per Index the designated local ports bitmap
*
* D. Application responsibility is not to use a mix of using API from
* those 2 modes
* The only time that the Application can shift between the 2 modes,
* is only when there are no trunks in the device.
*
* @version   85
********************************************************************************
*/
#define CPSS_LOG_IN_MODULE_ENABLE
/* get the device info and common info */
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
/* get DxCh Trunk types */
#include <cpss/dxCh/dxChxGen/trunk/cpssDxChTrunk.h>
/* get DxCh Trunk private types */
#include <cpss/dxCh/dxChxGen/trunk/private/prvCpssDxChTrunk.h>
#include <cpss/dxCh/dxChxGen/trunk/private/prvCpssDxChTrunkLog.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwInit.h>

#include <cpssDriver/pp/config/prvCpssDrvPpCfgInit.h>

#include <cpssCommon/cpssFormatConvert.h>

#include <cpss/dxCh/dxChxGen/ip/cpssDxChIp.h>

/* offset between entries in the non trunk table */
#define CHEETAH_TRUNK_NON_TRUNK_TABLE_OFFSET_CNS        0x00000010

/* offset between entries in the designated ports trunk table */
#define CHEETAH_TRUNK_DESIGNATED_PORTS_TABLE_OFFSET_CNS 0x00000010

/* offset between entries in the num of members in trunk table for cheetah */
#define CHEETAH_TRUNK_NUM_MEMBERS_TABLE_OFFSET_CNS      0x00001000

/* offset between entries in the num of members in trunk table for cheetah2 */
#define CHEETAH2_TRUNK_NUM_MEMBERS_TABLE_OFFSET_CNS     0x00000004

/* check the index in the designated ports table  */
#define HW_DESIGNATED_PORTS_ENTRY_INDEX_CHECK_MAC(devNum,index) \
    if((index) >= PRV_CPSS_DEV_TRUNK_INFO_MAC(devNum)->numOfDesignatedTrunkEntriesHw)   \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG)

/* HW check that the trunkId is valid index to trunk members table */
#define HW_TRUNK_ID_AS_INDEX_IN_TRUNK_MEMBERS_CHECK_MAC(devNum,index) \
    if(((index) > PRV_CPSS_DEV_TRUNK_INFO_MAC(devNum)->numTrunksSupportedHw) || ((index) == 0))\
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);                                          \
    if(0 == XCAT_DEVICE_AND_TRUNK_ID_CHECK_WA_TRAFFIC_TO_CPU_MAC(devNum,index))\
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG)/* the device in WA mode but the trunkId not in according to rules */


/* HW check that the trunkId is valid index to non-trunk ports table */
/* no mistake -- check the trunkId against numTrunksSupportedHw and not against
                numNonTrunkEntriesHw
                value 0 is valid !!!
*/
#define HW_TRUNK_ID_AS_INDEX_IN_NON_TRUNK_PORTS_CHECK_MAC(devNum,index)    \
    if((index) > PRV_CPSS_DEV_TRUNK_INFO_MAC(devNum)->numTrunksSupportedHw)\
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);                                               \
    if(0 == XCAT_DEVICE_AND_TRUNK_ID_CHECK_WA_TRAFFIC_TO_CPU_MAC(devNum,index)) \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG) /* the device in WA mode but the trunkId not in according to rules */

/* trunk id that use as index in the "non-trunk" table ,
    HW uses this entry when getting traffic from source trunk that had trunkId
    bigger than what HW support */
#define NULL_TRUNK_ID_CNS   0

#define REGISTER_ADDR_IN_NUM_MEMBERS_TRUNK_TABLE_MAC(devNum,trunkId,regAddrPtr) \
    if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_CHEETAH_E)        \
    {                                                                         \
        /* cheetah */                                                         \
        *regAddrPtr =                                                         \
            PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bridgeRegs.trunkTable +       \
            (((trunkId-1)>>3) * CHEETAH_TRUNK_NUM_MEMBERS_TABLE_OFFSET_CNS);  \
    }                                                                         \
    else                                                                      \
    {                                                                         \
        /* cheetah2 and above */                                              \
        *regAddrPtr =                                                         \
            PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bridgeRegs.trunkTable +       \
            (((trunkId-1)>>3) * CHEETAH2_TRUNK_NUM_MEMBERS_TABLE_OFFSET_CNS); \
    }

/* #define DUMP_TRUNK_UP_TO_MEMBER */

#ifdef ASIC_SIMULATION
    #define DUMP_TRUNK_UP_TO_MEMBER
#endif /*ASIC_SIMULATION*/


/* invalid trunk member */
#define INVALID_TRUNK_MEMBER_VALUE_CNS  0xff

/* indicate that a UP is in use , NOTE: the index in the array of upToUseArr[]
is not the UP , but the value of the element is the UP */
#define UP_IN_USE_CNS  0xff

/* indication to do 'stretch' number of members emulation in eArch */
static GT_U32   eArchDoStrechNumMembersWa = 1;
/* when stretching : this number sets MAX the number of replications */
static GT_U32   eArchDoStrechNumMembersWa_maxDuplications = 4;

/* performance stretch limit - we not want the stretch to impact performance */
/* so limit the stretch to up to 64 members */
/* current use of 4K means --> 'no limit' on the stretch */
static GT_U32   eArchDoStrechNumMembersWa_maxMembersAfterStretch = _4K;

/* bookmark to state that we need last index in the L2 ECMP table to hold the NULL port */
/* see PRV_CPSS_DXCH_PP_MAC(devNum)->bridge.l2EcmpIndex_NULL_port */
#define L2_ECMP_NULL_PORT_BOOKMARK

/* macro to check if the table of 'non trunk members' is ONLY for the cascade trunk ,
   and the trunkId is out of range */
#define CASCADE_TRUNK_ID_FOR_SEPERATION_FROM_REGULARE_TRUNK_TABLE_CHECK(_devNum,_trunkId)    \
    if(PRV_CPSS_DXCH_PP_HW_INFO_TRUNK_MAC(_devNum).useNonTrunkMembers1TableAsCascade == GT_TRUE &&  \
       _trunkId >= PRV_CPSS_DXCH_PP_HW_INFO_TRUNK_MAC(_devNum).nonTrunkTable1NumEntries)     \
    {                                                                                        \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);                                                                 \
    }

/* crc num bytes used for generation of hash value */
#define CRC_NUM_BYTES_USED_FOR_GENERATION_CNS   70
/* number of CRC32Salt[] registers */
#define NUM_SULT_REG_MAC(devNum) \
    ((sizeof(PRV_DXCH_REG1_UNIT_PCL_MAC(devNum).HASH.CRC32Salt) / sizeof(PRV_DXCH_REG1_UNIT_PCL_MAC(devNum).HASH.CRC32Salt[0])))

/*check for eport*/
#define TRUNK_PORT_CHECK_MAC(_devNum, _portNum)                                 \
    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(_devNum))                    \
    {                                                                           \
        PRV_CPSS_DXCH_PORT_CHECK_MAC(_devNum,_portNum);                         \
    }                                                                           \
    else                                                                        \
    {                                                                           \
        PRV_CPSS_DXCH_PHY_PORT_CHECK_MAC(_devNum, _portNum);                    \
    }

/* get 'bit of packet part' value from BMP array of 'masks' */
/* NOTE: each bit in bmpArr represents 8 bits in 'packet part' !!!! */
#define GET_BIT_FROM_MASK_BMP_MAC(bmpArr , bitOfPacketPart) \
    ((bmpArr[((bitOfPacketPart)/8) >> 5] >> (((bitOfPacketPart)/8) & 0x1f)) & 1)

/* get bit value from a byte that is in REVERSE byte order in bytes array of 'packet part' */
#define GET_REVERSE_BYTE_ORDER_BIT____FROM_NETWORK_ORDER_BYTE_MAC(byteArr , bitOfPacketPart) \
    ((byteArr[(sizeof(byteArr) - (1+((bitOfPacketPart)/8)))] >> ((bitOfPacketPart)%8)) & 1)


#define TRUNK_ID_ALL_CNS    0xFFFF

#define NUMBITS_L2_ECMP_START_INDEX_MAC(devNum) \
    (PRV_CPSS_SIP_5_20_CHECK_MAC(devNum) ? 14 : 13)


static GT_BOOL  flexCompatibleTo8Members = GT_FALSE;
void cpssDxChTrunkFlexCompatibleTo8MembersSet(
    IN GT_BOOL isCompatibale
)
{
    flexCompatibleTo8Members = isCompatibale;
}

static GT_BOOL debug_internal_cpssDxChTrunkTableEntrySet_sip5 = GT_FALSE;
void debug_internal_cpssDxChTrunkTableEntrySet(
    IN GT_BOOL doDebug
)
{
    debug_internal_cpssDxChTrunkTableEntrySet_sip5 = doDebug;
}



/**
* @struct EXTRA_TRUNK_ENTRY_STC
 *
 * @brief Structure represent the DB of the DXCH CPSS for single trunk
 * INFO "PER TRUNK"
*/
typedef struct{
    struct{
        struct{
            CPSS_TRUNK_MEMBER_STC   upToMemberMap[PRV_CPSS_TRUNK_8_MAX_NUM_OF_MEMBERS_CNS];
            GT_U8  upToUseArr[PRV_CPSS_TRUNK_8_MAX_NUM_OF_MEMBERS_CNS];
            GT_U32 nextUpIndexToUse;
        }info_WRONG_TRUNK_ID_SOURCE_PORT_INFORMATION_OF_PACKET_TO_CPU;
    }errata;
}EXTRA_TRUNK_ENTRY_STC;

/* max number of trunk in dxch devices */
#define DXCH_MAX_TRUNKS_CNS         128

/* number of trunks needed for the WA for "wrong trunk id source port information of packet to CPU" */
/*
    /2 , because only even trunks are valid for the WA
*/

#define XCAT_NUM_TRUNKS_FOR_WA_WRONG_SOURCE_PORT_TO_CPU  \
    (1 + ((XCAT_LAST_TRUNKID_FOR_WA_WRONG_SOURCE_PORT_TO_CPU - \
     XCAT_FIRST_TRUNKID_FOR_WA_WRONG_SOURCE_PORT_TO_CPU)/2))

/* macro to check the trunkId needed WA for :
   "wrong trunk id source port information of packet to CPU"
   note: trunks 126,127 not need the WA since not traffic to CPU
*/
#define XCAT_TRUNKID_CHECK_FOR_WA_WRONG_SOURCE_PORT_TO_CPU_MAC(_trunkId) \
    (((_trunkId) & 1) ? 0 :/* only even value */                         \
     ((_trunkId) < /*minimum value */                                    \
        XCAT_FIRST_TRUNKID_FOR_WA_WRONG_SOURCE_PORT_TO_CPU) ? 0 :        \
     ((_trunkId) > /*maximum value*/                                     \
        XCAT_LAST_TRUNKID_FOR_WA_WRONG_SOURCE_PORT_TO_CPU) ? 0 :         \
        1)

/* check that device need the WA but the trunkId not according to the rules */
/* this check must be only after checking that trunkId <= numTrunksSupportedHw */
/*
    MACRO returns:
    0 - the index for that device is NOT valid
    1 - the index for that device is valid
*/
#define XCAT_DEVICE_AND_TRUNK_ID_CHECK_WA_TRAFFIC_TO_CPU_MAC(_devNum,index) \
    (((PRV_CPSS_DEV_TRUNK_INFO_MAC(_devNum)->initDone == GT_FALSE) ? 0 : \
      (0 == PRV_CPSS_DXCH_CHECK_TRUNK_WRONG_SOURCE_PORT_TO_CPU_WA_MAC(_devNum))? 0 : \
      (1 == XCAT_TRUNKID_CHECK_FOR_WA_WRONG_SOURCE_PORT_TO_CPU_MAC(index)) ? 0 : \
      (((index) & 0x7e) == 0x7e) ? 0 : /* for devices with "real" 127 trunks support trunks 126,127 for cascading */ \
      ((index) == 0) ? 0 : /* index 0 is valid for all devices */ \
      1/* failed all checks */        \
     ) ?                                \
     0 : /* macro return failure */     \
     1)  /* macro return success */


/**
* @struct DXCH_TRUNK_SPECIFIC_INFO_STC
 *
 * @brief Structure represent the DB of the DXCH CPSS for trunks in that device
 * that needed for specific extra info
 * INFO "PER DEVICE"
*/
typedef struct{

    EXTRA_TRUNK_ENTRY_STC extraTrunkInfoArray[ XCAT_NUM_TRUNKS_FOR_WA_WRONG_SOURCE_PORT_TO_CPU];

} DXCH_TRUNK_SPECIFIC_INFO_STC;


/**
* @struct FIELD_INFO_STC
 *
 * @brief Structure represent the DB of the fields of the CRC hash mask entry
*/
typedef struct{

    /** @brief offset of SW field from start of CPSS_DXCH_TRUNK_LBH_CRC_MASK_ENTRY_STC
     *  value is in bytes.
     */
    GT_U32 offsetOfSwFieldForStcStart;

    /** the start bit in the entry */
    GT_U32 startBit;

    /** number of bits in field */
    GT_U32 length;

    /** offset from bit 0 in the value */
    GT_U32 offset;

    /** is this field value need to be swapped */
    GT_BOOL needToSwap;

} FIELD_INFO_STC;

/*offset of SW field from start of CPSS_DXCH_TRUNK_LBH_CRC_MASK_ENTRY_STC value is in bytes. */
#define HASH_MASK_CRC_FIELD_SW_DISTANCE_MAC(_field)  \
    CPSS_FORMAT_CONVERT_STRUCT_MEMBER_OFFSET_MAC(CPSS_DXCH_TRUNK_LBH_CRC_MASK_ENTRY_STC,_field)

/*  LionB,xcat2,Lion2
    the DB about the fields in the CRC hash mask entry.
    index to this table is according to fields in
    CPSS_DXCH_TRUNK_LBH_CRC_MASK_ENTRY_STC */
static FIELD_INFO_STC hashMaskCrcFieldsInfo[]=
{
     {HASH_MASK_CRC_FIELD_SW_DISTANCE_MAC(l4DstPortMaskBmp   ),2, 2 ,0 ,GT_TRUE } /* l4DstPortMaskBmp;    */
    ,{HASH_MASK_CRC_FIELD_SW_DISTANCE_MAC(l4SrcPortMaskBmp   ),0, 2 ,0 ,GT_TRUE } /* l4SrcPortMaskBmp;    */
    ,{HASH_MASK_CRC_FIELD_SW_DISTANCE_MAC(ipv6FlowMaskBmp    ),4, 3 ,0 ,GT_TRUE } /* ipv6FlowMaskBmp;     */
    ,{HASH_MASK_CRC_FIELD_SW_DISTANCE_MAC(ipDipMaskBmp       ),7, 16,0 ,GT_TRUE } /* ipDipMaskBmp;        */
    ,{HASH_MASK_CRC_FIELD_SW_DISTANCE_MAC(ipSipMaskBmp       ),23,16,0 ,GT_TRUE } /* ipSipMaskBmp;        */
    ,{HASH_MASK_CRC_FIELD_SW_DISTANCE_MAC(macDaMaskBmp       ),39,6 ,0 ,GT_TRUE } /* macDaMaskBmp;        */
    ,{HASH_MASK_CRC_FIELD_SW_DISTANCE_MAC(macSaMaskBmp       ),45,6 ,0 ,GT_TRUE } /* macSaMaskBmp;        */
    ,{HASH_MASK_CRC_FIELD_SW_DISTANCE_MAC(mplsLabel0MaskBmp  ),51,3 ,0 ,GT_TRUE } /* mplsLabel0MaskBmp;   */
    ,{HASH_MASK_CRC_FIELD_SW_DISTANCE_MAC(mplsLabel1MaskBmp  ),54,3 ,0 ,GT_TRUE } /* mplsLabel1MaskBmp;   */
    ,{HASH_MASK_CRC_FIELD_SW_DISTANCE_MAC(mplsLabel2MaskBmp  ),57,3 ,0 ,GT_TRUE } /* mplsLabel2MaskBmp;   */
    ,{HASH_MASK_CRC_FIELD_SW_DISTANCE_MAC(localSrcPortMaskBmp),60,1 ,0 ,GT_FALSE} /* localSrcPortMaskBmp; */
    ,{HASH_MASK_CRC_FIELD_SW_DISTANCE_MAC(udbsMaskBmp        ),61,9 ,14,GT_FALSE} /* udbsMaskBmp;         */
};

/* BC2, Caelum, BC3 :  the DB about the fields in the CRC hash mask entry.
    index to this table is according to fields in
    CPSS_DXCH_TRUNK_LBH_CRC_MASK_ENTRY_STC */
static FIELD_INFO_STC sip5_hashMaskCrcFieldsInfo[]=
{
     {HASH_MASK_CRC_FIELD_SW_DISTANCE_MAC(l4DstPortMaskBmp   ),0, 2 ,0 ,GT_TRUE } /* l4DstPortMaskBmp;    */
    ,{HASH_MASK_CRC_FIELD_SW_DISTANCE_MAC(l4SrcPortMaskBmp   ),2, 2 ,0 ,GT_TRUE } /* l4SrcPortMaskBmp;    */
    ,{HASH_MASK_CRC_FIELD_SW_DISTANCE_MAC(ipv6FlowMaskBmp    ),4, 3 ,0 ,GT_TRUE } /* ipv6FlowMaskBmp;     */
    ,{HASH_MASK_CRC_FIELD_SW_DISTANCE_MAC(ipDipMaskBmp       ),7, 16,0 ,GT_TRUE } /* ipDipMaskBmp;        */
    ,{HASH_MASK_CRC_FIELD_SW_DISTANCE_MAC(ipSipMaskBmp       ),23,16,0 ,GT_TRUE } /* ipSipMaskBmp;        */
    ,{HASH_MASK_CRC_FIELD_SW_DISTANCE_MAC(macDaMaskBmp       ),39,6 ,0 ,GT_TRUE } /* macDaMaskBmp;        */
    ,{HASH_MASK_CRC_FIELD_SW_DISTANCE_MAC(macSaMaskBmp       ),45,6 ,0 ,GT_TRUE } /* macSaMaskBmp;        */
    ,{HASH_MASK_CRC_FIELD_SW_DISTANCE_MAC(mplsLabel0MaskBmp  ),51,3 ,0 ,GT_TRUE } /* mplsLabel0MaskBmp;   */
    ,{HASH_MASK_CRC_FIELD_SW_DISTANCE_MAC(mplsLabel1MaskBmp  ),54,3 ,0 ,GT_TRUE } /* mplsLabel1MaskBmp;   */
    ,{HASH_MASK_CRC_FIELD_SW_DISTANCE_MAC(mplsLabel2MaskBmp  ),57,3 ,0 ,GT_TRUE } /* mplsLabel2MaskBmp;   */
    ,{HASH_MASK_CRC_FIELD_SW_DISTANCE_MAC(localSrcPortMaskBmp),60,1 ,0 ,GT_FALSE} /* localSrcPortMaskBmp; */
    ,{HASH_MASK_CRC_FIELD_SW_DISTANCE_MAC(udbsMaskBmp        ),61,9 ,14,GT_FALSE} /* udbsMaskBmp;         */
};


/* number of words in the entry of : hash mask (for CRC hash) table */
#define HASH_MASK_CRC_ENTRY_NUM_WORDS_CNS       3

/* check index to the hash mask crc table of the device */
#define HASH_MASK_CRC_INDEX_CHECK_MAC(_devNum,_index)   \
    if((_index) >=                                      \
       PRV_CPSS_DXCH_PP_MAC(_devNum)->accessTableInfoPtr[CPSS_DXCH_LION_TABLE_TRUNK_HASH_MASK_CRC_E].maxNumOfEntries)\
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG)


/* set bit in target according to bit from value */
#define SET_BIT_FROM_VALUE_MAC(word,bit,value) \
    if((value))                         \
    {                                   \
        (word) |= ((value) << (bit));   \
    }                                   \
    else                                \
    {                                   \
        (word) &= (( 1  <<  (bit) ) ^ 0xffffffff );\
    }



/*max number of Designated Trunk Entries*/
#define MAX_DESIGNATED_TRUNK_ENTRIES_NUM_CNS    64


/* 12 bits of hash and not 16 (the max value) - to support TXQ to EQ that have only 12 bits in descriptor */
/* the 12 bits issue approved with PD (27/6/13) as defaults for the time that Application not have APIs to set it explicitly */
#define SIP5_LFSR_MAX_12_BITS_CNS   12
#define SIP5_LFSR_LEGACY_6_BITS_CNS 6



/**
* @internal prvCpssDxChTrunkTrafficToCpuWaCpuPortEnableSet function
* @endinternal
*
* @brief   enable/disable the needed for the WA for the CPU/cascade port that may
*         represent the pass to the CPU
*         WA of "wrong trunk id source port information of packet to CPU" erratum
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number of the device that we set member on.
* @param[in] portNum                  - the CPU/cascade port that may represent the pass to the CPU
* @param[in] enable                   - enable/disable the settings
*
* @retval GT_OK                    - on success , or device not need the WA
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
*/
GT_STATUS prvCpssDxChTrunkTrafficToCpuWaCpuPortEnableSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 enable
)
{
    GT_U32  value;/*value to write to register */
    GT_U32  portOffset = portNum * 4;/* offset that related to the port */
    GT_U32  bit; /* bit index in the register */
    GT_U32  additionalInfoBmp; /* bitmap for additional WA info */
    GT_STATUS   rc;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    if (PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_DXCH_XCAT_E)
    {
        /* function called from the CSCD lib ... return GT_OK */
        /* no HW update needed */
        return GT_OK;
    }

    if(0 ==
        PRV_CPSS_DXCH_CHECK_TRUNK_WRONG_SOURCE_PORT_TO_CPU_WA_MAC(devNum))
    {
        return  GT_OK;
    }

    if(enable == GT_TRUE)
    {
        value = 0x00001ff9;
    }
    else
    {
        value = 0;
    }

    additionalInfoBmp = PRV_CPSS_DXCH_PP_MAC(devNum)->errata.
        info_PRV_CPSS_DXCH_XCAT_TRUNK_WRONG_SOURCE_PORT_TO_CPU_WA_E.
        additionalInfoBmp;

    if (((additionalInfoBmp >> 1) & 0x1) == 0)
    {
        rc = prvCpssHwPpWriteRegister(devNum,0x0f80c000 + portOffset,value);
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    else if (((additionalInfoBmp >> 2) & 0x1) == 0)
    {
        rc = prvCpssHwPpWriteRegister(devNum,
                                    0x0f808000 + PRV_CPSS_HW_DEV_NUM_MAC(devNum)*256 + portOffset,value);
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    else
    {
        rc = prvCpssHwPpWriteRegister(devNum,
                                    0x0f80A000 + PRV_CPSS_HW_DEV_NUM_MAC(devNum)*256 + portOffset,value);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    if(portNum == CPSS_CPU_PORT_NUM_CNS)
    {
        bit = 31;
    }
    else
    {
        bit = portNum;
    }

    rc = prvCpssHwPpSetRegField(devNum,0x0f800010,bit,1,BOOL2BIT_MAC(enable));
    if (rc != GT_OK)
    {
        return rc;
    }

    if(enable == GT_TRUE)
    {
        rc = prvCpssHwPpSetRegField(devNum,0x0f800004,bit,1,0);/*set 0 regardless to enable parameter */
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChTrunkTrafficToCpuWaUpInit function
* @endinternal
*
* @brief   initialize the UP (user priority) to port settings,needed WA for :
*         "wrong trunk id source port information of packet to CPU"
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number of the device that we set member on.
* @param[in] additionalInfoBmp        - bitmap for additional WA info
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvCpssDxChTrunkTrafficToCpuWaUpInit
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  additionalInfoBmp
)
{
    GT_STATUS   rc;
    GT_U32      port;           /* port iterator */
    GT_U32      portOffset   ;  /* offset from register address */
    GT_U32      portOffset_1 ;  /* offset #1 from register address */

    if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT_E)

    {
        rc = prvCpssHwPpSetRegField(devNum,0x0e000208,1,4,0);
        if (rc != GT_OK)
        {
            return rc;
        }

        if (((additionalInfoBmp >> 1) & 0x1) == 0)
        {
            rc = prvCpssHwPpSetRegField(devNum,0x0f800000,7,1,1);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
        else if (((additionalInfoBmp >> 2) & 0x1) == 0)
        {
            rc = prvCpssHwPpSetRegField(devNum,0x0f800000,0,19,0x40082);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
        else
        {
            rc = prvCpssHwPpSetRegField(devNum,0x0f800000,0,19,0x50082);
            if (rc != GT_OK)
            {
                return rc;
            }
        }

        rc = prvCpssDxChTrunkTrafficToCpuWaCpuPortEnableSet(devNum,
                    CPSS_CPU_PORT_NUM_CNS,
                    GT_TRUE);/*enable*/
        if(rc != GT_OK)
        {
            return rc;
        }

        for(port = 0 ; port < PRV_CPSS_PP_MAC(devNum)->numOfPorts ; port++)
        {
            portOffset      = 0x40 * port;
            portOffset_1    = 0x10 * port;

            if (additionalInfoBmp & 0x1)
            {
                rc = prvCpssHwPpWriteRegister(devNum,0x0e040020 + portOffset,(63-port) << 11);
                if (rc != GT_OK)
                {
                    return rc;
                }
                rc = prvCpssHwPpWriteRegister(devNum,0x0e040024 + portOffset,0x00080000);
                if (rc != GT_OK)
                {
                    return rc;
                }
                rc = prvCpssHwPpWriteRegister(devNum,0x0e0b0008 + portOffset_1,0x00000000);
                if (rc != GT_OK)
                {
                    return rc;
                }
                rc = prvCpssHwPpWriteRegister(devNum,0x0e0b000c + portOffset_1,0x00000000);
                if (rc != GT_OK)
                {
                    return rc;
                }
                rc = prvCpssHwPpWriteRegister(devNum,0x0e040000 + portOffset,0x00000000);
                if (rc != GT_OK)
                {
                    return rc;
                }
                rc = prvCpssHwPpWriteRegister(devNum,0x0e040004 + portOffset,0x00030000);
                if (rc != GT_OK)
                {
                    return rc;
                }
                rc = prvCpssHwPpWriteRegister(devNum,0x0e040020 + portOffset,0x00000000);
                if (rc != GT_OK)
                {
                    return rc;
                }
                rc = prvCpssHwPpWriteRegister(devNum,0x0e040024 + portOffset,0x00030000);
                if (rc != GT_OK)
                {
                    return rc;
                }
                rc = prvCpssHwPpWriteRegister(devNum,0x0e0b0000 + portOffset_1,0x00000800);
                if (rc != GT_OK)
                {
                    return rc;
                }
                rc = prvCpssHwPpWriteRegister(devNum,0x0e0b0004 + portOffset_1,0x00000000);
                if (rc != GT_OK)
                {
                    return rc;
                }
                rc = prvCpssHwPpWriteRegister(devNum,0x0e040018 + portOffset,0x00000000);
                if (rc != GT_OK)
                {
                    return rc;
                }
                rc = prvCpssHwPpWriteRegister(devNum,0x0e04001c + portOffset,0x00050000);
                if (rc != GT_OK)
                {
                    return rc;
                }
                rc = prvCpssHwPpWriteRegister(devNum,0x0e040010 + portOffset,0x00000000);
                if (rc != GT_OK)
                {
                    return rc;
                }
                rc = prvCpssHwPpWriteRegister(devNum,0x0e040014 + portOffset,0x00050000);
                if (rc != GT_OK)
                {
                    return rc;
                }
                rc = prvCpssHwPpWriteRegister(devNum,0x0e040008 + portOffset,0x00000000);/*devNum << 27*/
                if (rc != GT_OK)
                {
                    return rc;
                }
                rc = prvCpssHwPpWriteRegister(devNum,0x0e04000c + portOffset,0x00050000);
                if (rc != GT_OK)
                {
                    return rc;
                }
                rc = prvCpssHwPpWriteRegister(devNum,0x0e040000 + portOffset,0x000007ff + (port << 11));
                if (rc != GT_OK)
                {
                    return rc;
                }
                rc = prvCpssHwPpWriteRegister(devNum,0x0e040004 + portOffset,0x00050000);
                if (rc != GT_OK)
                {
                    return rc;
                }
                rc = prvCpssHwPpWriteRegister(devNum,0x0e040038 + portOffset,0x00000000);
                if (rc != GT_OK)
                {
                    return rc;
                }
                rc = prvCpssHwPpWriteRegister(devNum,0x0e04003c + portOffset,0x00083000);
                if (rc != GT_OK)
                {
                    return rc;
                }
                rc = prvCpssHwPpWriteRegister(devNum,0x0e040030 + portOffset,0x00000000);
                if (rc != GT_OK)
                {
                    return rc;
                }
                rc = prvCpssHwPpWriteRegister(devNum,0x0e040034 + portOffset,0x00080000);
                if (rc != GT_OK)
                {
                    return rc;
                }
                rc = prvCpssHwPpWriteRegister(devNum,0x0e040028 + portOffset,0x00000000); /*(31-devNum) << 27*/
                if (rc != GT_OK)
                {
                    return rc;
                }
                rc = prvCpssHwPpWriteRegister(devNum,0x0e04002c + portOffset,0x00080000); /*srcTrg<<1*/
                if (rc != GT_OK)
                {
                    return rc;
                }
            }
            else
            {
                rc = prvCpssHwPpWriteRegister(devNum,0x0e06ffc0 - portOffset,0x00000000);
                if (rc != GT_OK)
                {
                    return rc;
                }
                rc = prvCpssHwPpWriteRegister(devNum,0x0e06ffc4 - portOffset,0x00030000);
                if (rc != GT_OK)
                {
                    return rc;
                }
                rc = prvCpssHwPpWriteRegister(devNum,0x0e06ffe0 - portOffset,0x00000000);
                if (rc != GT_OK)
                {
                    return rc;
                }
                rc = prvCpssHwPpWriteRegister(devNum,0x0e06ffe4 - portOffset,0x00030000);
                if (rc != GT_OK)
                {
                    return rc;
                }
                rc = prvCpssHwPpWriteRegister(devNum,0x0e0bbff0 - portOffset_1,0x00000800);
                if (rc != GT_OK)
                {
                    return rc;
                }
                rc = prvCpssHwPpWriteRegister(devNum,0x0e0bbff4 - portOffset_1,0x00000000);
                if (rc != GT_OK)
                {
                    return rc;
                }
                rc = prvCpssHwPpWriteRegister(devNum,0x0e0bbff8 - portOffset_1,0x00000000);
                if (rc != GT_OK)
                {
                    return rc;
                }
                rc = prvCpssHwPpWriteRegister(devNum,0x0e0bbffc - portOffset_1,0x00000000);
                if (rc != GT_OK)
                {
                    return rc;
                }
                rc = prvCpssHwPpWriteRegister(devNum,0x0e06ffd8 - portOffset,0x00000000);
                if (rc != GT_OK)
                {
                    return rc;
                }
                rc = prvCpssHwPpWriteRegister(devNum,0x0e06ffdc - portOffset,0x00050000);
                if (rc != GT_OK)
                {
                    return rc;
                }
                rc = prvCpssHwPpWriteRegister(devNum,0x0e06ffd0 - portOffset,0x00000000);
                if (rc != GT_OK)
                {
                    return rc;
                }
                rc = prvCpssHwPpWriteRegister(devNum,0x0e06ffd4 - portOffset,0x00050000);
                if (rc != GT_OK)
                {
                    return rc;
                }
                rc = prvCpssHwPpWriteRegister(devNum,0x0e06ffc8 - portOffset,0x00000000);/*devNum << 27*/
                if (rc != GT_OK)
                {
                    return rc;
                }
                rc = prvCpssHwPpWriteRegister(devNum,0x0e06ffcc - portOffset,0x00050000);
                if (rc != GT_OK)
                {
                    return rc;
                }
                rc = prvCpssHwPpWriteRegister(devNum,0x0e06ffc0 - portOffset,0x000007ff + (port << 11));
                if (rc != GT_OK)
                {
                    return rc;
                }
                rc = prvCpssHwPpWriteRegister(devNum,0x0e06ffc4 - portOffset,0x00050000);
                if (rc != GT_OK)
                {
                    return rc;
                }
                rc = prvCpssHwPpWriteRegister(devNum,0x0e06fff8 - portOffset,0x00000000);
                if (rc != GT_OK)
                {
                    return rc;
                }
                rc = prvCpssHwPpWriteRegister(devNum,0x0e06fffc - portOffset,0x00083000);
                if (rc != GT_OK)
                {
                    return rc;
                }
                rc = prvCpssHwPpWriteRegister(devNum,0x0e06fff0 - portOffset,0x00000000);
                if (rc != GT_OK)
                {
                    return rc;
                }
                rc = prvCpssHwPpWriteRegister(devNum,0x0e06fff4 - portOffset,0x00080000);
                if (rc != GT_OK)
                {
                    return rc;
                }
                rc = prvCpssHwPpWriteRegister(devNum,0x0e06ffe8 - portOffset,0x00000000);/*(31-devNum) << 27*/
                if (rc != GT_OK)
                {
                    return rc;
                }
                rc = prvCpssHwPpWriteRegister(devNum,0x0e06ffec - portOffset,0x00080000);/*srcTrg<<1*/
                if (rc != GT_OK)
                {
                    return rc;
                }
                rc = prvCpssHwPpWriteRegister(devNum,0x0e06ffe0 - portOffset,(63-port) << 11);
                if (rc != GT_OK)
                {
                    return rc;
                }
                rc = prvCpssHwPpWriteRegister(devNum,0x0e06ffe4 - portOffset,0x00080000);
                if (rc != GT_OK)
                {
                    return rc;
                }
            }
        }
    }

    if(PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.trunksNum <
        XCAT_LAST_TRUNKID_FOR_WA_WRONG_SOURCE_PORT_TO_CPU)
    {
        /* actual number of trunks reduced to 30 !!! even though we 'set' 124 trunks */
        PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.trunksNum =
            XCAT_LAST_TRUNKID_FOR_WA_WRONG_SOURCE_PORT_TO_CPU;
            /* set 124 for the generic engine only */
    }

    return GT_OK;
}

/**
* @internal trunkTrafficToCpuWaUpForPortEnableSet function
* @endinternal
*
* @brief   bind/unbind the UP (user priority) to port
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number of the device that we set member on.
* @param[in] enable                   - GT_TRUE - bind the port to new UP
*                                      GT_FALSE - unbind the port from the UP
* @param[in] up                       - the user priority to bind to the port
*                                      (relevant only when enable == GT_TRUE)
* @param[in] memberPtr                - (pointer to) the trunk member
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
*/
static GT_STATUS trunkTrafficToCpuWaUpForPortEnableSet
(
    IN  GT_U8                   devNum,
    IN  GT_BOOL                 enable,
    IN  GT_U8                   up,
    IN  CPSS_TRUNK_MEMBER_STC   *memberPtr
)
{
    GT_U32  portOffset = 0x10 * memberPtr->port;  /* offset that related to the port */
    GT_U32  value = 0x800;/* value to write to the register */
    GT_U32 additionalInfoBmp; /* bitmap for additional WA info */
    GT_STATUS   rc;             /* return code */

    if (PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_DXCH_XCAT_E)
    {
        /* no HW update needed */
        return GT_OK;
    }


    if(memberPtr->hwDevice != PRV_CPSS_HW_DEV_NUM_MAC(devNum))
    {
        /* not relevant to this device */
        return GT_OK;
    }

    if(0 == PRV_CPSS_PHY_PORT_IS_EXIST_MAC(devNum, memberPtr->port))
    {
        /* not need treatment */
        return GT_OK;
    }

    if(enable)
    {
        value |= 1<<23;
        value |= (up&7) << 18;
    }

    additionalInfoBmp = PRV_CPSS_DXCH_PP_MAC(devNum)->errata.
                        info_PRV_CPSS_DXCH_XCAT_TRUNK_WRONG_SOURCE_PORT_TO_CPU_WA_E.
                        additionalInfoBmp;

    if ((additionalInfoBmp & 0x1) == 0)
    {
        rc = prvCpssHwPpWriteRegister(devNum,0x0e0bbff0 - portOffset,     value);
        if (rc != GT_OK)
        {
            return rc;
    }
        rc = prvCpssHwPpWriteRegister(devNum,0x0e0bbff4 - portOffset,0x00000000);
        if (rc != GT_OK)
        {
            return rc;
        }
        rc = prvCpssHwPpWriteRegister(devNum,0x0e0bbff8 - portOffset,0x00000000);
        if (rc != GT_OK)
        {
            return rc;
        }
        rc = prvCpssHwPpWriteRegister(devNum,0x0e0bbffc - portOffset,0x00000000);
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    else
    {
        rc = prvCpssHwPpWriteRegister(devNum,0x0e0b0000 + portOffset,     value);
        if (rc != GT_OK)
        {
            return rc;
        }
        rc = prvCpssHwPpWriteRegister(devNum,0x0e0b0004 + portOffset,0x00000000);
        if (rc != GT_OK)
        {
            return rc;
        }
        rc = prvCpssHwPpWriteRegister(devNum,0x0e0b0008 + portOffset,0x00000000);
        if (rc != GT_OK)
        {
            return rc;
        }
        rc = prvCpssHwPpWriteRegister(devNum,0x0e0b000c + portOffset,0x00000000);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}


/**
* @internal prvCpssDxChTrunkTrafficToCpuWaUpToMemberConvert function
* @endinternal
*
* @brief   convert the UP (user priority) to trunk member (dev,port)
*         Needed for WA of "wrong trunk id source port information of packet to CPU" erratum
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number of the device that connected to CPU port/SDMA.
* @param[in,out] dsaTagInfoPtr            - (pointer to) the DSA tag info parsed from the DSA tag.
* @param[in,out] dsaTagInfoPtr            - (pointer to) the DSA tag info with fixed info regarding
*                                      "wrong trunk id source port information of packet to CPU"
*
* @retval GT_OK                    - on success , the DSA tag was 'fixed' (updated)
* @retval GT_NO_CHANGE             - there is no need for 'convert'
* @retval GT_DSA_PARSING_ERROR     - No valid mapping between the UP and trunk member.
*/
GT_STATUS prvCpssDxChTrunkTrafficToCpuWaUpToMemberConvert
(
    IN  GT_U8   devNum,
    INOUT CPSS_DXCH_NET_DSA_PARAMS_STC  *dsaTagInfoPtr
)
{
    GT_U32          dev;
    GT_TRUNK_ID     trunkId;/* trunkId to use for the WA */
    DXCH_TRUNK_SPECIFIC_INFO_STC *extraTrunksInfoPtr;/*(pointer to) specific trunk db of the device*/
    EXTRA_TRUNK_ENTRY_STC   *currWaTrunkInfoPtr;/*(pointer to)extra trunk info*/
    CPSS_DXCH_NET_DSA_TO_CPU_STC  *toCpuDsaTagInfoPtr;/*'to cpu' DSA tag format*/
    GT_U32  index;/*index in the specific info trunks */
    CPSS_TRUNK_MEMBER_STC   *memberPtr;/*(pointer to) the trunk member */

    if(dsaTagInfoPtr->dsaType != CPSS_DXCH_NET_DSA_CMD_TO_CPU_E)
    {
        return /*this is not ERROR for log */ GT_NO_CHANGE;
    }

    toCpuDsaTagInfoPtr = &dsaTagInfoPtr->dsaInfo.toCpu;

    /* the trunkId is supposed to be (portNum * 2) */
    trunkId = (GT_TRUNK_ID)(toCpuDsaTagInfoPtr->interface.portNum * 2);

    /* check that the trunkId is in valid range */
    if(0 ==
        XCAT_TRUNKID_CHECK_FOR_WA_WRONG_SOURCE_PORT_TO_CPU_MAC(trunkId))
    {
        /* the WA not needed */
        return /*this is not ERROR for log */ GT_NO_CHANGE;
    }

    if(toCpuDsaTagInfoPtr->srcIsTrunk == GT_TRUE)
    {
        return /*this is not ERROR for log */ GT_NO_CHANGE;
    }

    if(toCpuDsaTagInfoPtr->isEgressPipe == GT_TRUE)
    {
        return /*this is not ERROR for log */ GT_NO_CHANGE;
    }

    if(dsaTagInfoPtr->commonParams.vpt >= PRV_CPSS_TRUNK_8_MAX_NUM_OF_MEMBERS_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(0 == PRV_CPSS_IS_DEV_EXISTS_MAC(toCpuDsaTagInfoPtr->hwDevNum))
    {
        /* the src device is unknown --> assume is same device,revision as this
            <devNum> */
        dev = devNum;
    }
    else
    {
        dev = toCpuDsaTagInfoPtr->hwDevNum;
    }


    if(0 ==
        PRV_CPSS_DXCH_CHECK_TRUNK_WRONG_SOURCE_PORT_TO_CPU_WA_MAC(dev))
    {
        return /*this is not ERROR for log */ GT_NO_CHANGE;
    }

    index = (trunkId - XCAT_FIRST_TRUNKID_FOR_WA_WRONG_SOURCE_PORT_TO_CPU)
            / 2;

    if (index >= XCAT_NUM_TRUNKS_FOR_WA_WRONG_SOURCE_PORT_TO_CPU)
    {
        /* the value of toCpuDsaTagInfoPtr->interface.portNum is too big */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    extraTrunksInfoPtr = PRV_CPSS_DXCH_PP_MAC(devNum)->errata.
            info_PRV_CPSS_DXCH_XCAT_TRUNK_WRONG_SOURCE_PORT_TO_CPU_WA_E.
            extraTrunksInfoPtr;
    currWaTrunkInfoPtr = &extraTrunksInfoPtr->extraTrunkInfoArray[index];
    /* convert VPU to trunk member Index */
    memberPtr = &currWaTrunkInfoPtr->errata.
        info_WRONG_TRUNK_ID_SOURCE_PORT_INFORMATION_OF_PACKET_TO_CPU.upToMemberMap[
            dsaTagInfoPtr->commonParams.vpt];

    if(memberPtr->port == INVALID_TRUNK_MEMBER_VALUE_CNS)
    {
        /* DSA parsing error as result of trunk WA port mapping error */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_DSA_PARSING_ERROR, LOG_ERROR_NO_MSG);
    }

    if(memberPtr->hwDevice != toCpuDsaTagInfoPtr->hwDevNum)
    {
        /* DSA parsing error as result of wrong trunk WA port mapping error */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_DSA_PARSING_ERROR, LOG_ERROR_NO_MSG);
    }

    /* replace the trunk member with the info in the DSA */

    toCpuDsaTagInfoPtr->interface.portNum =  memberPtr->port;

    /* no need to change toCpuDsaTagInfoPtr->hwDevNum because it should be
       unchanged */

    return GT_OK;
}

/**
* @internal trunkTrafficToCpuWaUpToMemberAdd function
* @endinternal
*
* @brief   add new(or existing) member to the trunk --> needed for the DB of
*         map 'UP to trunk member'
*         Needed for WA of "wrong trunk id source port information of packet to CPU" erratum
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number of the device that we set member on.
* @param[in] trunkId                  - the trunk Id to which we add member
* @param[in] memberPtr                - (pointer to) the trunk member
*
* @retval GT_OK                    - on success or the WA not needed
* @retval GT_BAD_PARAM             - bad device number
* @retval GT_BAD_STATE             - unable to add a new member , because no free UP to use
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS trunkTrafficToCpuWaUpToMemberAdd
(
    IN  GT_U8                   devNum,
    IN  GT_TRUNK_ID             trunkId,
    IN  CPSS_TRUNK_MEMBER_STC   *memberPtr
)
{
    DXCH_TRUNK_SPECIFIC_INFO_STC *extraTrunksInfoPtr;/*(pointer to) specific trunk db of the device*/
    EXTRA_TRUNK_ENTRY_STC   *currWaTrunkInfoPtr;/*(pointer to)extra trunk info*/
    GT_U32  index;/*index in the specific info trunks */
    CPSS_TRUNK_MEMBER_STC   *currentMemberPtr;/* (pointer to)current trunk member in DB*/
    GT_U32  nextUpIndexToUse;/* next index to use for new member */
    GT_U8   up;/* UP (use priority) to use */

    index = (trunkId - XCAT_FIRST_TRUNKID_FOR_WA_WRONG_SOURCE_PORT_TO_CPU)
            / 2;
    extraTrunksInfoPtr = PRV_CPSS_DXCH_PP_MAC(devNum)->errata.
            info_PRV_CPSS_DXCH_XCAT_TRUNK_WRONG_SOURCE_PORT_TO_CPU_WA_E.
            extraTrunksInfoPtr;
    currWaTrunkInfoPtr = &extraTrunksInfoPtr->extraTrunkInfoArray[index];

    currentMemberPtr = &currWaTrunkInfoPtr->errata.
        info_WRONG_TRUNK_ID_SOURCE_PORT_INFORMATION_OF_PACKET_TO_CPU.upToMemberMap[
            0];

    /* look for the existence of the member */
    for(index = 0 ; index < PRV_CPSS_TRUNK_8_MAX_NUM_OF_MEMBERS_CNS;
        index++,currentMemberPtr++)
    {
        if(currentMemberPtr->port == memberPtr->port &&
           currentMemberPtr->hwDevice == memberPtr->hwDevice)
        {
            /* member already in DB --> nothing to do more */
            return GT_OK;
        }
    }

    nextUpIndexToUse = currWaTrunkInfoPtr->errata.
        info_WRONG_TRUNK_ID_SOURCE_PORT_INFORMATION_OF_PACKET_TO_CPU.nextUpIndexToUse;

    up = currWaTrunkInfoPtr->errata.
        info_WRONG_TRUNK_ID_SOURCE_PORT_INFORMATION_OF_PACKET_TO_CPU.upToUseArr[
        nextUpIndexToUse];

    if(up == UP_IN_USE_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    currentMemberPtr = &currWaTrunkInfoPtr->errata.
        info_WRONG_TRUNK_ID_SOURCE_PORT_INFORMATION_OF_PACKET_TO_CPU.upToMemberMap[
            up];

    if(currentMemberPtr->port != INVALID_TRUNK_MEMBER_VALUE_CNS)
    {
        /* not empty member ?! */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    *currentMemberPtr = *memberPtr;

    currWaTrunkInfoPtr->errata.
        info_WRONG_TRUNK_ID_SOURCE_PORT_INFORMATION_OF_PACKET_TO_CPU.nextUpIndexToUse =
            (nextUpIndexToUse + 1) % PRV_CPSS_TRUNK_8_MAX_NUM_OF_MEMBERS_CNS;

    currWaTrunkInfoPtr->errata.
        info_WRONG_TRUNK_ID_SOURCE_PORT_INFORMATION_OF_PACKET_TO_CPU.upToUseArr[
        nextUpIndexToUse] = UP_IN_USE_CNS;

    return trunkTrafficToCpuWaUpForPortEnableSet(devNum,GT_TRUE,up,memberPtr);
}

/**
* @internal trunkTrafficToCpuWaUpToMemberRemove function
* @endinternal
*
* @brief   remove existing member from the trunk --> needed for
*         the DB of map 'UP to trunk member'
*         Needed for WA of "wrong trunk id source port information of packet to CPU" erratum
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number of the device that we set member on.
* @param[in] trunkId                  - the trunk Id to which we remove member
* @param[in] memberPtr                - (pointer to) the trunk member
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - bad device number
* @retval GT_BAD_STATE             - member not found
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
*/
static GT_STATUS trunkTrafficToCpuWaUpToMemberRemove
(
    IN  GT_U8                   devNum,
    IN  GT_TRUNK_ID             trunkId,
    IN  CPSS_TRUNK_MEMBER_STC   *memberPtr
)
{
    DXCH_TRUNK_SPECIFIC_INFO_STC *extraTrunksInfoPtr;/*(pointer to) specific trunk db of the device*/
    EXTRA_TRUNK_ENTRY_STC   *currWaTrunkInfoPtr;/*(pointer to)extra trunk info*/
    GT_U32  index;/*index in the specific info trunks */
    CPSS_TRUNK_MEMBER_STC   *currentMemberPtr;/* (pointer to)current trunk member in DB*/
    GT_U32  freeUpIndex;/* the UP index to get the free UP */
    GT_U32  ii;

    index = (trunkId - XCAT_FIRST_TRUNKID_FOR_WA_WRONG_SOURCE_PORT_TO_CPU)
            / 2;
    extraTrunksInfoPtr = PRV_CPSS_DXCH_PP_MAC(devNum)->errata.
            info_PRV_CPSS_DXCH_XCAT_TRUNK_WRONG_SOURCE_PORT_TO_CPU_WA_E.
            extraTrunksInfoPtr;
    currWaTrunkInfoPtr = &extraTrunksInfoPtr->extraTrunkInfoArray[index];

    currentMemberPtr = &currWaTrunkInfoPtr->errata.
        info_WRONG_TRUNK_ID_SOURCE_PORT_INFORMATION_OF_PACKET_TO_CPU.upToMemberMap[
            0];

    /* look for the existence of the member */
    for(index = 0 ; index < PRV_CPSS_TRUNK_8_MAX_NUM_OF_MEMBERS_CNS;
        index++,currentMemberPtr++)
    {
        if(currentMemberPtr->port == memberPtr->port &&
           currentMemberPtr->hwDevice == memberPtr->hwDevice)
        {
            /* member found in DB */
            break;
        }
    }

    if(index == PRV_CPSS_TRUNK_8_MAX_NUM_OF_MEMBERS_CNS)
    {
        /* member not in the trunk  --> should not happen */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    /* invalidate the member for that UP */
    currentMemberPtr->port = INVALID_TRUNK_MEMBER_VALUE_CNS;
    currentMemberPtr->hwDevice = INVALID_TRUNK_MEMBER_VALUE_CNS;

    freeUpIndex = (currWaTrunkInfoPtr->errata.
                    info_WRONG_TRUNK_ID_SOURCE_PORT_INFORMATION_OF_PACKET_TO_CPU.
                    nextUpIndexToUse - 1) %
                    PRV_CPSS_TRUNK_8_MAX_NUM_OF_MEMBERS_CNS;

    /* look for index to free the UP to it */
    for(ii = 0 ; ii < PRV_CPSS_TRUNK_8_MAX_NUM_OF_MEMBERS_CNS ;
        ii++ , freeUpIndex = ((freeUpIndex - 1)%PRV_CPSS_TRUNK_8_MAX_NUM_OF_MEMBERS_CNS))
    {
        if(currWaTrunkInfoPtr->errata.
            info_WRONG_TRUNK_ID_SOURCE_PORT_INFORMATION_OF_PACKET_TO_CPU.upToUseArr[
            freeUpIndex] != UP_IN_USE_CNS)
        {
            freeUpIndex = ((freeUpIndex + 1)%PRV_CPSS_TRUNK_8_MAX_NUM_OF_MEMBERS_CNS);
            break;
        }
    }

    if(ii == PRV_CPSS_TRUNK_8_MAX_NUM_OF_MEMBERS_CNS)
    {
        /* all UPs are used */
        freeUpIndex = currWaTrunkInfoPtr->errata.
            info_WRONG_TRUNK_ID_SOURCE_PORT_INFORMATION_OF_PACKET_TO_CPU.
            nextUpIndexToUse;
    }

    /* restore the 'free UP' to last to use */
    currWaTrunkInfoPtr->errata.
        info_WRONG_TRUNK_ID_SOURCE_PORT_INFORMATION_OF_PACKET_TO_CPU.upToUseArr[
        freeUpIndex] = (GT_U8)index;

    return trunkTrafficToCpuWaUpForPortEnableSet(devNum,GT_FALSE,0,memberPtr);
}

/**
* @internal trunkTrafficToCpuWaTrunkTableEntrySet function
* @endinternal
*
* @brief   Set the trunk table entry --> needed for the DB of map 'UP to trunk member'
*         Needed for WA of "wrong trunk id source port information of packet to CPU" erratum
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] trunkId                  - trunk id
* @param[in] numMembers               - num of enabled members in the trunk
* @param[in] membersArray[]           - array of enabled members of the trunk
*
* @retval GT_OK                    - successes or WA no needed
* @retval GT_FAIL                  - an error occurred.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
*/
static GT_STATUS trunkTrafficToCpuWaTrunkTableEntrySet
(
    IN  GT_U8                   devNum,
    IN  GT_TRUNK_ID             trunkId,
    IN  GT_U32                  numMembers,
    IN  CPSS_TRUNK_MEMBER_STC   membersArray[]
)
{
    GT_STATUS   rc;
    DXCH_TRUNK_SPECIFIC_INFO_STC *extraTrunksInfoPtr;/*(pointer to) specific trunk db of the device*/
    EXTRA_TRUNK_ENTRY_STC   *currWaTrunkInfoPtr;/*(pointer to)extra trunk info*/
    GT_U32  index;/*index in the specific info trunks */
    CPSS_TRUNK_MEMBER_STC   *currentMemberPtr;/* (pointer to)current trunk member in DB*/
    GT_U32  ii;/* index in membersArray[] */
    CPSS_TRUNK_MEMBER_STC   tmpMember;/*tmp trunk member*/
    GT_U32                  numOfDisabledMembers = CPSS_TRUNK_MAX_NUM_OF_MEMBERS_CNS; /* number of members as retrieved from trunk DB*/
    GT_U32                  numTotalMembers = 0;
    CPSS_TRUNK_MEMBER_STC   disabledMembersArray[CPSS_TRUNK_MAX_NUM_OF_MEMBERS_CNS]; /* array of db disabledmembers of the trunk */
    CPSS_TRUNK_MEMBER_STC   totalMembersArray[CPSS_TRUNK_MAX_NUM_OF_MEMBERS_CNS]; /* array of enabled & disabled members of the trunk */


    if(0 == PRV_CPSS_DXCH_CHECK_TRUNK_WRONG_SOURCE_PORT_TO_CPU_WA_MAC(devNum) ||
       0 == XCAT_TRUNKID_CHECK_FOR_WA_WRONG_SOURCE_PORT_TO_CPU_MAC(trunkId))
    {
        /* the WA not needed */
        return GT_OK;
    }

    /* get disabled trunk members */
    rc = cpssDxChTrunkDbDisabledMembersGet(devNum,trunkId,
                                           &numOfDisabledMembers,
                                           &disabledMembersArray[0]);
    if(rc == GT_OK)
    {
       /* if first enable member is NULL port and there are disabled trunk's
       port members, then WA ignore the null port (the array will occupy
       only the disabled members)*/
       if (!((membersArray[0].port == PRV_CPSS_DXCH_NULL_PORT_NUM_CNS) &&
             (numOfDisabledMembers != 0)))
       {
           for(ii = 0; ii < numMembers ;ii++)
           {
               totalMembersArray[ii] = membersArray[ii];
               numTotalMembers++;
           }
       }

       for(ii = 0; ii < numOfDisabledMembers ;ii++)
       {
           totalMembersArray[numTotalMembers] = disabledMembersArray[ii];
           numTotalMembers++;
       }

       if( CPSS_TRUNK_MAX_NUM_OF_MEMBERS_CNS < numTotalMembers)
       {
           CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
       }

    }
    else /* the DB not supported at this stage or at all */
    {
        /* copy the members array to local array */
        cpssOsMemCpy(totalMembersArray, membersArray ,
                     sizeof(CPSS_TRUNK_MEMBER_STC)*numMembers);
        numTotalMembers = numMembers;
    }
    index = (trunkId - XCAT_FIRST_TRUNKID_FOR_WA_WRONG_SOURCE_PORT_TO_CPU)
            / 2;
    extraTrunksInfoPtr = PRV_CPSS_DXCH_PP_MAC(devNum)->errata.
            info_PRV_CPSS_DXCH_XCAT_TRUNK_WRONG_SOURCE_PORT_TO_CPU_WA_E.
            extraTrunksInfoPtr;
    currWaTrunkInfoPtr = &extraTrunksInfoPtr->extraTrunkInfoArray[index];

    currentMemberPtr = &currWaTrunkInfoPtr->errata.
        info_WRONG_TRUNK_ID_SOURCE_PORT_INFORMATION_OF_PACKET_TO_CPU.upToMemberMap[
            0];

    /* remove 'old members' that exists in DB but not in the new settings */
    for(index = 0 ; index < PRV_CPSS_TRUNK_8_MAX_NUM_OF_MEMBERS_CNS;
        index++,currentMemberPtr++)
    {
        if(currentMemberPtr->port == INVALID_TRUNK_MEMBER_VALUE_CNS)
        {
            /* empty space */
            continue;
        }

        for(ii = 0 ; ii < numTotalMembers; ii++)
        {
            if(totalMembersArray[ii].port == currentMemberPtr->port &&
               totalMembersArray[ii].hwDevice == currentMemberPtr->hwDevice )
            {
                break;
            }
        }

        if(ii == numTotalMembers)
        {
            /* copy the member to remove to temporary variable */
            tmpMember = *currentMemberPtr; /* must not supply currentMemberPtr
                        because this will affect behavior of trunkTrafficToCpuWaUpToMemberRemove(...) */

            /* this member should be removed from the WA */
            rc = trunkTrafficToCpuWaUpToMemberRemove(devNum,trunkId,&tmpMember);
            if(rc != GT_OK)
            {
                return rc;
            }
        }
    }

    for(ii = 0 ; ii < numTotalMembers; ii++)
    {
        /* add the members */
        rc = trunkTrafficToCpuWaUpToMemberAdd(devNum,trunkId,
                                              &totalMembersArray[ii]);
        if(rc != GT_OK)
        {
            return rc;
        }
    }


    return GT_OK;
}


#ifdef DUMP_TRUNK_UP_TO_MEMBER
/**
* @internal cpssDxChTrunkTrafficToCpuWaUpToMemberDump function
* @endinternal
*
* @brief   a debug function to dump the DB of 'UP to trunk members' for
*         specific trunk id.
*         Needed for WA of "wrong trunk id source port information of packet to CPU" erratum
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] trunkId                  - the trunk Id to which we dump the info
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - bad device number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTrunkTrafficToCpuWaUpToMemberDump
(
    IN  GT_U8                   devNum,
    IN  GT_TRUNK_ID             trunkId
)
{
    DXCH_TRUNK_SPECIFIC_INFO_STC *extraTrunksInfoPtr;/*(pointer to) specific trunk db of the device*/
    EXTRA_TRUNK_ENTRY_STC   *currWaTrunkInfoPtr;/*(pointer to)extra trunk info*/
    GT_U32  index;/*index in the specific info trunks */
    CPSS_TRUNK_MEMBER_STC   *currentMemberPtr;/* (pointer to)current trunk member in DB*/
    GT_U32  ii;/* iterator */
    GT_U8   *nextUpToUsePtr;/*(pointer to)next Up To Use in array upToUseArr[]*/

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    if(0 ==
        PRV_CPSS_DXCH_CHECK_TRUNK_WRONG_SOURCE_PORT_TO_CPU_WA_MAC(devNum))
    {
        cpssOsPrintf(" this device not implementing the WA \n");

        return GT_OK;
    }

    /* check that the trunkId is in valid range */
    if(0 ==
        XCAT_TRUNKID_CHECK_FOR_WA_WRONG_SOURCE_PORT_TO_CPU_MAC(trunkId))
    {
        cpssOsPrintf(" trunk ID [%d] not in WA range [%d]-[%d] or NOT EVEN \n",
                    trunkId,
                    XCAT_FIRST_TRUNKID_FOR_WA_WRONG_SOURCE_PORT_TO_CPU,
                    XCAT_LAST_TRUNKID_FOR_WA_WRONG_SOURCE_PORT_TO_CPU);
        return GT_OK;
    }

    index = (trunkId - XCAT_FIRST_TRUNKID_FOR_WA_WRONG_SOURCE_PORT_TO_CPU)
            / 2;
    extraTrunksInfoPtr = PRV_CPSS_DXCH_PP_MAC(devNum)->errata.
            info_PRV_CPSS_DXCH_XCAT_TRUNK_WRONG_SOURCE_PORT_TO_CPU_WA_E.
            extraTrunksInfoPtr;
    currWaTrunkInfoPtr = &extraTrunksInfoPtr->extraTrunkInfoArray[index];

    /************************************************/
    currentMemberPtr = &currWaTrunkInfoPtr->errata.
        info_WRONG_TRUNK_ID_SOURCE_PORT_INFORMATION_OF_PACKET_TO_CPU.upToMemberMap[
            0];

    cpssOsPrintf(" upToMemberMap [] \n");
    cpssOsPrintf("\n");

    cpssOsPrintf(" UP       |   {dev}  , {port}   \n");
    cpssOsPrintf("================================\n");

    for(ii = 0 ; ii < PRV_CPSS_TRUNK_8_MAX_NUM_OF_MEMBERS_CNS;
        ii++, currentMemberPtr++)
    {
        cpssOsPrintf(" %d       %d    , %d \n",
                        ii,
                        currentMemberPtr->hwDevice,
                        currentMemberPtr->port);
    }

    cpssOsPrintf("\n");

    /************************************************/

    cpssOsPrintf(" nextUpIndexToUse = [%d] \n",
            currWaTrunkInfoPtr->errata.
                info_WRONG_TRUNK_ID_SOURCE_PORT_INFORMATION_OF_PACKET_TO_CPU.
                    nextUpIndexToUse);
    cpssOsPrintf("\n");

    /************************************************/
    nextUpToUsePtr = &currWaTrunkInfoPtr->errata.
        info_WRONG_TRUNK_ID_SOURCE_PORT_INFORMATION_OF_PACKET_TO_CPU.upToUseArr[
            0];

    cpssOsPrintf(" upToUseArr [] \n");
    cpssOsPrintf("\n");

    cpssOsPrintf(" index     |   next UP to use \n");
    cpssOsPrintf("================================\n");

    for(ii = 0 ; ii < PRV_CPSS_TRUNK_8_MAX_NUM_OF_MEMBERS_CNS;
        ii++, nextUpToUsePtr++)
    {
        cpssOsPrintf(" %d       %d    \n",
                        ii,
                        (*nextUpToUsePtr));
    }

    cpssOsPrintf("\n");
    /************************************************/


    return GT_OK;
}
#endif /*DUMP_TRUNK_UP_TO_MEMBER*/

/**
* @internal trunkDbFlagsInit function
* @endinternal
*
* @brief   get info about the flags of the device about trunk
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
*/
static GT_STATUS trunkDbFlagsInit
(
    IN  GT_U8                       devNum
)
{
    PRV_CPSS_TRUNK_DB_INFO_STC *devTrunkInfoPtr = PRV_CPSS_DEV_TRUNK_INFO_MAC(devNum);
    GT_U32  maxTrunksNeeded;

    devTrunkInfoPtr->nullPort = PRV_CPSS_DXCH_NULL_PORT_NUM_CNS;
    devTrunkInfoPtr->doDesignatedTableWorkAround  =  GT_FALSE;
    devTrunkInfoPtr->designatedTrunkTableSplit = GT_FALSE;

    if(0 == PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(devNum))
    {
        devTrunkInfoPtr->numOfDesignatedTrunkEntriesHw = 8;
    }
    else
    {
        devTrunkInfoPtr->numOfDesignatedTrunkEntriesHw = 64;
    }

    if(CPSS_PP_FAMILY_DXCH_XCAT3_E == PRV_CPSS_PP_MAC(devNum)->devFamily)
    {
        devTrunkInfoPtr->designatedTrunkTableSplit = GT_TRUE;
    }
    else
    if (PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum))
    {
        devTrunkInfoPtr->designatedTrunkTableSplit = GT_TRUE;
    }

    if (devTrunkInfoPtr->designatedTrunkTableSplit == GT_TRUE)
    {
        /* the MC and UC tables , each in the 'size' of regular table */
        devTrunkInfoPtr->numOfDesignatedTrunkEntriesHw *= 2;
    }


    devTrunkInfoPtr->initDone = GT_FALSE;
    /* the trunk members are always 'Physical' ports (and not EPorts) */
    /* NOTEabout 'dual device Id' :
        the generic trunk engine will know to treat 'dual device Id' devices.
        by adding another bit for the check of the mask of 'port'

        we not add it here because the mask depends if the trunk member is
        'dual device Id' or not.
    */
    if (PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum))
    {
        devTrunkInfoPtr->isPhysicalPortByMask = GT_TRUE;
        devTrunkInfoPtr->validityMask.port   = MAX(0xFF , (PRV_CPSS_DXCH_MAX_PHY_PORT_NUMBER_MAC(devNum)-1));
    }
    else
    {
        devTrunkInfoPtr->validityMask.port   = PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_PHY_PORT_MAC(devNum);
    }
    devTrunkInfoPtr->validityMask.hwDevice = PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_HW_DEV_NUM_MAC(devNum);

    /* was already set by generic layer */
    if(devTrunkInfoPtr->loadBalanceType != PRV_CPSS_TRUNK_LOAD_BALANCE_FLEX_E)
    {   /* native mode or soho emulation mode */
        if (PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum))
        {
            /* at this point the fineTuning.tableSize.trunksNum is number of
                max supported trunks (4K) */
            /* but we need to consider (8 members) trunks */
            maxTrunksNeeded = (PRV_CPSS_DXCH_PP_MAC(devNum)->bridge.l2EcmpNumMembers /
                               PRV_CPSS_TRUNK_8_MAX_NUM_OF_MEMBERS_CNS) -1;

            if(maxTrunksNeeded > PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.trunksNum)
            {
                /* the device actually allow less trunks than the '1/2' of the 'L2 ECMP' limitation ! */
                maxTrunksNeeded = PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.trunksNum;
            }

            if(devTrunkInfoPtr->numberOfTrunks < maxTrunksNeeded)
            {
                /* application don't want to use ALL the L2ECMP entries only by trunks ! */
                maxTrunksNeeded = devTrunkInfoPtr->numberOfTrunks;
            }

            /* we need to 'reserve' in the L2ECMP table entries that will be used
               by the trunks */
            PRV_CPSS_DXCH_PP_MAC(devNum)->bridge.l2EcmpNumMembers_usedByTrunk =
                PRV_CPSS_TRUNK_8_MAX_NUM_OF_MEMBERS_CNS * maxTrunksNeeded;

            devTrunkInfoPtr->numNonTrunkEntriesHw = maxTrunksNeeded;
            devTrunkInfoPtr->numTrunksSupportedHw = maxTrunksNeeded;
        }
        else
        {
            devTrunkInfoPtr->numNonTrunkEntriesHw = PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.trunksNum;
            devTrunkInfoPtr->numTrunksSupportedHw = PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.trunksNum;
        }
    }
    else
    {
        /* we are in SIP5 device that need option for :
            1. different number of members in each trunk
            2. all trunks with fixed max number of members (but not limited to 8)
                EXAMPLE : need all trunks to be with 64 members
        */
        devTrunkInfoPtr->numNonTrunkEntriesHw = PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.trunksNum;
        devTrunkInfoPtr->numTrunksSupportedHw = PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.trunksNum;

        /* in this mode there is no explicit partition that the CPSS is doing
           between trunks and ePorts ECMP*/
        PRV_CPSS_DXCH_PP_MAC(devNum)->bridge.l2EcmpNumMembers_usedByTrunk = 0;

        /* state that each trunk may hold different number of 'max' members */
        devTrunkInfoPtr->maxNumMembersInTrunk_isPerTrunk = GT_TRUE;
        /* each trunk may hold up to 4K members , but max limitation actually
           comes from the 'per device' */
        devTrunkInfoPtr->maxNumMembersInTrunk  = _4K;
        /* the device support number of members according to 'L2 ECMP' table */
        devTrunkInfoPtr->maxNumMembersInDevice =
            PRV_CPSS_DXCH_PP_MAC(devNum)->bridge.l2EcmpNumMembers;
    }

    if (PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum))
    {
        #if 0 /* set here the related 'WA' that require 'reserved' index */
              /* it is for 'reference' purposes */
            PRV_CPSS_DXCH_BOBCAT2_RM_TRUNK_LTT_ENTRY_WRONG_DEFAULT_VALUES_WA_E
        #endif

        L2_ECMP_NULL_PORT_BOOKMARK
        /* we need extra index to be untouched (last index) to hold the NULL port */
        PRV_CPSS_DXCH_PP_MAC(devNum)->bridge.l2EcmpIndex_NULL_port =
            PRV_CPSS_DXCH_PP_MAC(devNum)->bridge.l2EcmpNumMembers - 1;

        /* state the ECMP manager that another index is reserved */
        PRV_CPSS_DXCH_PP_MAC(devNum)->bridge.l2EcmpNumMembers_usedByTrunk++;

        if(devTrunkInfoPtr->loadBalanceType == PRV_CPSS_TRUNK_LOAD_BALANCE_FLEX_E)
        {
            /* one less 'member to use' by the trunks */
            devTrunkInfoPtr->maxNumMembersInDevice--;
        }
    }

    return GT_OK;
}

/**
* @internal internal_cpssDxChTrunkDbMembersSortingEnableSet function
* @endinternal
*
* @brief   Function Relevant mode : High Level mode
*         This function enable/disable 'sorting' of trunk members in the
*         'trunk members table' and in the 'designated trunk table'
*         This mode not effect 'cascade trunks' (that not need synchronization between devices)
*         'sorting enabled' : when the application will add/remove/set members in a trunk
*         cpss will make sure to set the trunk members into the
*         device in an order that is not affected by the 'history'
*         of 'add/remove' members to/from the trunk.
*         'sorting disabled' : (legacy mode / default mode) when the application
*         will add/remove/set members in a trunk cpss will set
*         the trunk members into the device in an order that is
*         affected by the 'history' of 'add/remove' members
*         to/from the trunk.
*         function uses the DB (no HW operations)
*         The API start to effect only on trunks that do operations (add/remove/set members)
*         after the API call.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - enable/disable the sorting
*                                      GT_TRUE : enabled , GT_FALSE - disabled
*
* @retval GT_OK                    - on success.
* @retval GT_NOT_INITIALIZED       - the trunk library was not initialized for the device
* @retval GT_BAD_PARAM             - bad device number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTrunkDbMembersSortingEnableSet
(
    IN GT_U8                    devNum,
    IN GT_BOOL                  enable
)
{
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    /* check that DB was initialized */
    if(PRV_CPSS_DEV_TRUNK_INFO_MAC(devNum)->initDone == GT_FALSE)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    /*save value into DB*/
    PRV_CPSS_DEV_TRUNK_INFO_MAC(devNum)->sortTrunkMembers = enable;

    return GT_OK;
}

/**
* @internal cpssDxChTrunkDbMembersSortingEnableSet function
* @endinternal
*
* @brief   Function Relevant mode : High Level mode
*         This function enable/disable 'sorting' of trunk members in the
*         'trunk members table' and in the 'designated trunk table'
*         This mode not effect 'cascade trunks' (that not need synchronization between devices)
*         'sorting enabled' : when the application will add/remove/set members in a trunk
*         cpss will make sure to set the trunk members into the
*         device in an order that is not affected by the 'history'
*         of 'add/remove' members to/from the trunk.
*         'sorting disabled' : (legacy mode / default mode) when the application
*         will add/remove/set members in a trunk cpss will set
*         the trunk members into the device in an order that is
*         affected by the 'history' of 'add/remove' members
*         to/from the trunk.
*         function uses the DB (no HW operations)
*         The API start to effect only on trunks that do operations (add/remove/set members)
*         after the API call.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - enable/disable the sorting
*                                      GT_TRUE : enabled , GT_FALSE - disabled
*
* @retval GT_OK                    - on success.
* @retval GT_NOT_INITIALIZED       - the trunk library was not initialized for the device
* @retval GT_BAD_PARAM             - bad device number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTrunkDbMembersSortingEnableSet
(
    IN GT_U8                    devNum,
    IN GT_BOOL                  enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTrunkDbMembersSortingEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enable));

    rc = internal_cpssDxChTrunkDbMembersSortingEnableSet(devNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTrunkDbMembersSortingEnableGet function
* @endinternal
*
* @brief   Function Relevant mode : High Level mode
*         Gets the enable/disable 'sorting' of trunk members in the 'trunk members table'
*         and in the 'designated trunk table'
*         This mode not effect 'cascade trunks' (that not need synchronization between devices)
*         function uses the DB (no HW operations)
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to) the sorting is enabled/disabled
*
* @retval GT_OK                    - on success.
* @retval GT_NOT_INITIALIZED       - the trunk library was not initialized for the device
* @retval GT_BAD_PARAM             - bad device number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTrunkDbMembersSortingEnableGet
(
    IN GT_U8                    devNum,
    OUT GT_BOOL                 *enablePtr
)
{
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    /* check that DB was initialized */
    if(PRV_CPSS_DEV_TRUNK_INFO_MAC(devNum)->initDone == GT_FALSE)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);

    /*get value from DB*/
    *enablePtr = PRV_CPSS_DEV_TRUNK_INFO_MAC(devNum)->sortTrunkMembers;

    return GT_OK;
}

/**
* @internal cpssDxChTrunkDbMembersSortingEnableGet function
* @endinternal
*
* @brief   Function Relevant mode : High Level mode
*         Gets the enable/disable 'sorting' of trunk members in the 'trunk members table'
*         and in the 'designated trunk table'
*         This mode not effect 'cascade trunks' (that not need synchronization between devices)
*         function uses the DB (no HW operations)
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to) the sorting is enabled/disabled
*
* @retval GT_OK                    - on success.
* @retval GT_NOT_INITIALIZED       - the trunk library was not initialized for the device
* @retval GT_BAD_PARAM             - bad device number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTrunkDbMembersSortingEnableGet
(
    IN GT_U8                    devNum,
    OUT GT_BOOL                 *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTrunkDbMembersSortingEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enablePtr));

    rc = internal_cpssDxChTrunkDbMembersSortingEnableGet(devNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTrunkPortTrunkIdSet function
* @endinternal
*
* @brief   Function Relevant mode : Low level mode
*         Set the trunkId field in the port's control register in the device
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] portNum                  - physical port number.
* @param[in] memberOfTrunk            - is the port associated with the trunk
*                                      GT_FALSE - the port is set as "not member" in the trunk
*                                      GT_TRUE  - the port is set with the trunkId
* @param[in] trunkId                  - the trunk to which the port associate with
*                                      This field indicates the trunk group number (ID) to which the
*                                      port is a member.
*                                      1 through "max number of trunks" = The port is a member of the trunk
*                                      this value relevant only when memberOfTrunk = GT_TRUE
*
* @retval GT_OK                    - successful completion
* @retval GT_FAIL                  - an error occurred.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or
*                                       bad port number , or
*                                       bad trunkId number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The trunkId configuration should be done after the configuration of the
*       ePort to portNum in the translation table.
*       In case of a change in the translation table this API should be called
*       again, since the configuration done here will not be correct any more.
*
*/
static GT_STATUS internal_cpssDxChTrunkPortTrunkIdSet
(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN GT_BOOL                  memberOfTrunk,
    IN GT_TRUNK_ID              trunkId
)
{
    GT_STATUS rc = GT_OK;/* return error code */
    GT_U32 regAddr;     /* register address */
    GT_U32 value;       /* value to write to register */
    GT_U32 bitOffset;   /* offset of nit from start of register */
    GT_U32 length;      /* length of field in the register */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_ENHANCED_PHY_PORT_CHECK_MAC(devNum, portNum);

    if(memberOfTrunk == GT_TRUE)
    {
        HW_TRUNK_ID_AS_INDEX_IN_TRUNK_MEMBERS_CHECK_MAC(devNum,trunkId);

        value = trunkId;
    }
    else
    {
        /* the port is not member in any trunk */
        value = 0;
    }

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        /* set the trunkId in the port physical table*/
         rc = prvCpssDxChWriteTableEntryField(devNum,
                                            CPSS_DXCH_SIP5_TABLE_TTI_PHYSICAL_PORT_ATTRIBUTE_E,
                                            portNum,/*global port*/
                                            PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                            SIP5_TTI_PHYSICAL_PORT_TABLE_FIELDS_TRUNK_ID_E, /* field name */
                                            PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                            value);
    }
    else
    {
        /* trunk id of port control it accessed via interface to :
           Port<n> VLAN and QoS Configuration Entry
           Word 0 , bits 0:6 (7 bits)
        */
        rc = prvCpssDxChWriteTableEntryField(devNum,
                                            CPSS_DXCH_TABLE_PORT_VLAN_QOS_E,
                                            portNum,
                                            0,/* Word 0 */
                                            0,/* start bit 0 */
                                            7,/* 7 bits */
                                            value);
    }

    if(rc != GT_OK)
    {
        return rc;
    }

    /*  NOTE: for multi-port groups devices
        those are 'HA' registers and must be set with 'global port' number and
        the same way to all port groups
    */
    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        rc = prvCpssDxChWriteTableEntryField(devNum,
                                              CPSS_DXCH_SIP5_TABLE_HA_PHYSICAL_PORT_2_E,
                                              portNum, /*global port*/
                                              PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                              SIP5_HA_PHYSICAL_PORT_TABLE_2_FIELDS_TRUNK_ID_E, /* field name */
                                              PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                            value);
    }
    else
    {
        /* base 0x0780002C , 4 ports info in register , for each port info 7 bits*/
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->haRegs.trunkNumConfigReg[portNum >> 2];
        length = 7;/* 7 bits trunk id per port */

        bitOffset = (portNum & 0x3) * length;

        rc = prvCpssHwPpSetRegField(devNum, regAddr, bitOffset, length, value);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    return rc;

}

/**
* @internal cpssDxChTrunkPortTrunkIdSet function
* @endinternal
*
* @brief   Function Relevant mode : Low level mode
*         Set the trunkId field in the port's control register in the device
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] portNum                  - physical port number.
* @param[in] memberOfTrunk            - is the port associated with the trunk
*                                      GT_FALSE - the port is set as "not member" in the trunk
*                                      GT_TRUE  - the port is set with the trunkId
* @param[in] trunkId                  - the trunk to which the port associate with
*                                      This field indicates the trunk group number (ID) to which the
*                                      port is a member.
*                                      1 through "max number of trunks" = The port is a member of the trunk
*                                      this value relevant only when memberOfTrunk = GT_TRUE
*
* @retval GT_OK                    - successful completion
* @retval GT_FAIL                  - an error occurred.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or
*                                       bad port number , or
*                                       bad trunkId number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The trunkId configuration should be done after the configuration of the
*       ePort to portNum in the translation table.
*       In case of a change in the translation table this API should be called
*       again, since the configuration done here will not be correct any more.
*
*/
GT_STATUS cpssDxChTrunkPortTrunkIdSet
(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN GT_BOOL                  memberOfTrunk,
    IN GT_TRUNK_ID              trunkId
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTrunkPortTrunkIdSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, memberOfTrunk, trunkId));

    rc = internal_cpssDxChTrunkPortTrunkIdSet(devNum, portNum, memberOfTrunk, trunkId);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, memberOfTrunk, trunkId));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTrunkPortTrunkIdGet function
* @endinternal
*
* @brief   Function Relevant mode : All modes
*         Set the trunkId field in the port's control register in the device
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] portNum                  - physical port number.
*
* @param[out] memberOfTrunkPtr         - (pointer to) is the port associated with the trunk
*                                      GT_FALSE - the port is set as "not member" in the trunk
*                                      GT_TRUE  - the port is set with the trunkId
* @param[out] trunkIdPtr               - (pointer to)the trunk to which the port associate with
*                                      This field indicates the trunk group number (ID) to which the
*                                      port is a member.
*                                      1 through "max number of trunks" = The port is a member of the trunk
*                                      this value relevant only when (memberOfTrunkPtr) = GT_TRUE
*
* @retval GT_OK                    - successful completion
* @retval GT_FAIL                  - an error occurred.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or
*                                       bad port number , or
*                                       bad trunkId number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_BAD_STATE             - the trunkId value is not synchronized in the 2 registers
*                                       that should hold the same value
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTrunkPortTrunkIdGet
(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    OUT GT_BOOL                 *memberOfTrunkPtr,
    OUT GT_TRUNK_ID             *trunkIdPtr
)
{
    GT_STATUS   rc;
    GT_U32 regAddr;     /* register address */
    GT_U32 value;       /* value read from register */
    GT_U32 bitOffset;   /* offset of nit from start of register */
    GT_U32 length;      /* length of field in the register */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_ENHANCED_PHY_PORT_CHECK_MAC(devNum, portNum);
    CPSS_NULL_PTR_CHECK_MAC(memberOfTrunkPtr);
    CPSS_NULL_PTR_CHECK_MAC(trunkIdPtr);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        /* get the trunkId in the port physical table*/
         rc = prvCpssDxChReadTableEntryField(devNum,
                                            CPSS_DXCH_SIP5_TABLE_TTI_PHYSICAL_PORT_ATTRIBUTE_E,
                                            portNum,/*global port*/
                                            PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                            SIP5_TTI_PHYSICAL_PORT_TABLE_FIELDS_TRUNK_ID_E, /* field name */
                                            PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                            &value);
    }
    else
    {
        /* trunk id of port control it accessed via interface to :
           Port<n> VLAN and QoS Configuration Entry
           Word 0 , bits 0:6 (7 bits)
        */
        rc = prvCpssDxChReadTableEntryField(devNum,
                                        CPSS_DXCH_TABLE_PORT_VLAN_QOS_E,
                                        portNum,
                                        0,/* Word 0 */
                                        0,/* start bit 0 */
                                        7,/* 7 bits */
                                        &value);
    }

    if(rc != GT_OK)
    {
        return rc;
    }

    /* build the trunk-id value */
    *trunkIdPtr = (GT_TRUNK_ID)value;

    /* the port is trunk member if the trunk-id is NOT ZERO */
    *memberOfTrunkPtr = (*trunkIdPtr) ? GT_TRUE : GT_FALSE;


    /************************************************************************/
    /* according to DR decision: 30-05-06                                   */
    /* check that the other register is also configured with the same value */
    /************************************************************************/

    /*  NOTE: for multi-port groups devices
        those are 'HA' registers and must be set with 'global port' number and
        the same way to all port groups
    */

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        rc = prvCpssDxChReadTableEntryField(devNum,
                                              CPSS_DXCH_SIP5_TABLE_HA_PHYSICAL_PORT_2_E,
                                              portNum, /*global port*/
                                              PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                              SIP5_HA_PHYSICAL_PORT_TABLE_2_FIELDS_TRUNK_ID_E, /* field name */
                                              PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                            &value);
        if(rc != GT_OK)
        {
            return rc;
        }
    }
    else
    {
        /* base 0x0780002C , 4 ports info  in register , for each port info 7 bits*/
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->haRegs.trunkNumConfigReg[portNum >> 2];

        length = 7;/* 7 bits per port */
        bitOffset = (portNum & 0x3) * length;

        rc =  prvCpssHwPpGetRegField(devNum, regAddr, bitOffset, length, &value);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    if(value != (GT_U32)(*trunkIdPtr))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal cpssDxChTrunkPortTrunkIdGet function
* @endinternal
*
* @brief   Function Relevant mode : All modes
*         Set the trunkId field in the port's control register in the device
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] portNum                  - physical port number.
*
* @param[out] memberOfTrunkPtr         - (pointer to) is the port associated with the trunk
*                                      GT_FALSE - the port is set as "not member" in the trunk
*                                      GT_TRUE  - the port is set with the trunkId
* @param[out] trunkIdPtr               - (pointer to)the trunk to which the port associate with
*                                      This field indicates the trunk group number (ID) to which the
*                                      port is a member.
*                                      1 through "max number of trunks" = The port is a member of the trunk
*                                      this value relevant only when (memberOfTrunkPtr) = GT_TRUE
*
* @retval GT_OK                    - successful completion
* @retval GT_FAIL                  - an error occurred.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or
*                                       bad port number , or
*                                       bad trunkId number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_BAD_STATE             - the trunkId value is not synchronized in the 2 registers
*                                       that should hold the same value
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTrunkPortTrunkIdGet
(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    OUT GT_BOOL                 *memberOfTrunkPtr,
    OUT GT_TRUNK_ID             *trunkIdPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTrunkPortTrunkIdGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, memberOfTrunkPtr, trunkIdPtr));

    rc = internal_cpssDxChTrunkPortTrunkIdGet(devNum, portNum, memberOfTrunkPtr, trunkIdPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, memberOfTrunkPtr, trunkIdPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal chTrunkMembersTableAccessWrite function
* @endinternal
*
* @brief   function write specific member of a specific trunk :
*         "Trunk table Trunk<n> Member<i> Entry (1<=n<128, 0<=i<8)"
* @param[in] devNum                   - the device number
* @param[in] trunkId                  - trunk id 1<=trunkId<128
* @param[in] trunkMemberIdx           - trunk member index  0<=trunkMemberIdx<8
* @param[in,out] memberPtr                - (pointer to) the member to write to the table
* @param[in,out] memberPtr                - (pointer to) the member to write to the table
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_BAD_PARAM             - bad parameters (or null pointer)
* @retval GT_OUT_OF_RANGE          - parameter not in valid range
*
* @note [1] C.14.2 Trunk table
*       The Trunk table is used to define the trunk members.
*       Up to "max number of trunks" trunks (numbered 1 through "max number of trunks") may be defined,with each trunk
*       having up to eight members.
*       see Table 315 outlines the content of each entry in the table.
*
*/
static GT_STATUS chTrunkMembersTableAccessWrite
(
    IN GT_U8                            devNum,
    IN GT_TRUNK_ID                      trunkId,
    IN GT_U32                           trunkMemberIdx,
    INOUT CPSS_TRUNK_MEMBER_STC         *memberPtr
)
{
    GT_U32      valueArr[1];/* value to write to memory */
    GT_U32      entryIndex; /* entry index */
    GT_U32      devNumBit;/* start bit for devNum field */
    GT_HW_DEV_NUM           hwDevNum;/* hw device Id of the trunk member */
    GT_PHYSICAL_PORT_NUM    portNum; /* port number of the trunk member */

    entryIndex = ((trunkId) << 3) | /* trunk id     */
                  (trunkMemberIdx);  /* member index */

    devNumBit = PRV_CPSS_DXCH_PP_HW_INFO_FLEX_FIELD_NUM_BITS_SUPPORTED_MAC(devNum).phyPort > 6 ? 8 : 6;

    hwDevNum = memberPtr->hwDevice;
    portNum  = memberPtr->port;

    hwDevNum = PRV_CPSS_DXCH_DUAL_HW_DEVICE_CONVERT_DEV_MAC(
        memberPtr->hwDevice, memberPtr->port);

    portNum = PRV_CPSS_DXCH_DUAL_HW_DEVICE_CONVERT_PORT_MAC(
        memberPtr->hwDevice, memberPtr->port);

   /* check phyPort and hwDev*/
    if((portNum > PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_PHY_PORT_MAC(devNum)) ||
       (hwDevNum > PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_HW_DEV_NUM_MAC(devNum)))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    valueArr[0] = (hwDevNum << devNumBit) | portNum;

    return prvCpssDxChWriteTableEntry(devNum,
                                      CPSS_DXCH_TABLE_TRUNK_MEMBERS_E,
                                      entryIndex,
                                      valueArr);
}


/**
* @internal chTrunkMembersTableAccessRead function
* @endinternal
*
* @brief   function read specific member of a specific trunk :
*         "Trunk table Trunk<n> Member<i> Entry (1<=n<128, 0<=i<8)"
* @param[in] devNum                   - the device number
* @param[in] trunkId                  - trunk id 1<=trunkId<128
* @param[in] trunkMemberIdx           - trunk member index  0<=trunkMemberIdx<8
* @param[in,out] memberPtr                - (pointer to) the member read from the table
* @param[in,out] memberPtr                - (pointer to) the member read from the table
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_BAD_PARAM             - bad parameters (or null pointer)
* @retval GT_OUT_OF_RANGE          - parameter not in valid range
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
*
* @note [1] C.14.2 Trunk table
*       The Trunk table is used to define the trunk members.
*       Up to "max number of trunks" trunks (numbered 1 through "max number of trunks") may be defined,with each trunk
*       having up to eight members.
*       see Table 315 outlines the content of each entry in the table.
*
*/
static GT_STATUS chTrunkMembersTableAccessRead
(
    IN GT_U8                            devNum,
    IN GT_TRUNK_ID                      trunkId,
    IN GT_U32                           trunkMemberIdx,
    INOUT CPSS_TRUNK_MEMBER_STC         *memberPtr
)
{

    GT_STATUS rc;    /* return error code */
    GT_U32      valueArr[1];/* value to read from memory */
    GT_U32      entryIndex; /* entry index */
    GT_U32      devNumBit;/* start bit for devNum field */

    entryIndex = trunkId << 3 | /* trunk id     */
                 trunkMemberIdx ;  /* member index */

    rc = prvCpssDxChReadTableEntry(devNum,
                                   CPSS_DXCH_TABLE_TRUNK_MEMBERS_E,
                                   entryIndex,
                                   valueArr);
    if (rc != GT_OK)
    {
        return rc;
    }

    devNumBit = PRV_CPSS_DXCH_PP_HW_INFO_FLEX_FIELD_NUM_BITS_SUPPORTED_MAC(devNum).phyPort > 6 ? 8 : 6;

    memberPtr->hwDevice = (U32_GET_FIELD_MAC(valueArr[0],
                          devNumBit,
                          (PRV_CPSS_DXCH_PP_HW_INFO_FLEX_FIELD_NUM_BITS_SUPPORTED_MAC(devNum).hwDevNum)));
    memberPtr->port   = (U32_GET_FIELD_MAC(valueArr[0],
                          0,
                          (PRV_CPSS_DXCH_PP_HW_INFO_FLEX_FIELD_NUM_BITS_SUPPORTED_MAC(devNum).phyPort)));

    PRV_CPSS_DXCH_DUAL_HW_DEVICE_CONVERT_DEV_PORT_MAC(memberPtr->hwDevice,memberPtr->port);

    return GT_OK;

}

/**
* @internal internal_cpssDxChTrunkTableEntrySet_sip5 function
* @endinternal
*
* @brief   Function Relevant mode : Low level mode
*         Set the trunk table entry , and set the number of members in it.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] trunkId                  - trunk id
* @param[in] numMembers               - num of enabled members in the trunk
*                                      Note : value 0 is not recommended.
* @param[in] membersArray[]           - array of enabled members of the trunk
*
* @retval GT_OK                    - successful completion
* @retval GT_FAIL                  - an error occurred.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - numMembers exceed the number of maximum number
* @retval of members in trunk (total of 0 - 8 members allowed)
* @retval GT_BAD_PARAM             - bad device number , or
*                                       bad trunkId number , or
*                                       bad members parameters :
*                                       (device & 0xE0) != 0 means that the HW can't support
*                                       this value , since HW has 5 bit
*                                       for device number
*                                       (port & 0xC0) != 0 means that the HW can't support
*                                       this value , since HW has 6 bit
*                                       for port number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTrunkTableEntrySet_sip5
(
    IN  GT_U8                   devNum,
    IN  GT_TRUNK_ID             trunkId,
    IN  GT_U32                  numMembers,
    IN  CPSS_TRUNK_MEMBER_STC   membersArray[]
)
{
    GT_STATUS               rc;              /* return error code */
    GT_U32                  offset;          /* offset of field in the register */
    GT_U32                  data;            /* data to write to register */
    GT_U32                  ii;              /* iterator */
    GT_U32                  numBits_L2_ECMP_Start_Index;/* number of bits in <L2_ECMP_Start_Index> */
    PRV_CPSS_TRUNK_DB_INFO_STC *devTrunkInfoPtr = PRV_CPSS_DEV_TRUNK_INFO_MAC(devNum);
    PRV_CPSS_TRUNK_ENTRY_INFO_STC *trunkEntryPtr;
    GT_U32                  maxStretch;/* the actual number of members to write to the trunk LTT entry */
                                        /* it can be more than numMembers to allow 'better' load balance. */
                                        /* this is done by 'repeating' the trunk members and 'stretching' */
                                        /* the number of members */
    GT_U32                  memberIndex;/* index into array of membersArray[]*/

    /* in SIP5 we allow NULL pointer (when numMembers == 0)*/
    if(numMembers)
    {
        CPSS_NULL_PTR_CHECK_MAC(membersArray);
    }
    else/*numMembers==0*/
    {
        /* we are not accessing the membersArray[] ! */
    }

    numBits_L2_ECMP_Start_Index = NUMBITS_L2_ECMP_START_INDEX_MAC(devNum);

    if(devTrunkInfoPtr->shadowValid == GT_FALSE)
    {
        /* in this mode : the application 'should' not use this function !!!
           it should use : cpssDxChTrunkEcmpLttTableSet(...) and
                           cpssDxChBrgL2EcmpTableSet(...)


           because in current API we not know the 'L2 ECMP' index that needed
            for this trunk , and reading it from the HW implies that the
            application needed to set cpssDxChTrunkEcmpLttTableSet with this info
            before calling this API ... it limits the flexibility of 'low level'
        */

        /* but we need to read the trunk LTT and get the current offset into the L2 ECMP */
        rc = prvCpssDxChReadTableEntry(devNum,
            CPSS_DXCH_SIP5_TABLE_EQ_TRUNK_LTT_E,
            trunkId,
            &data);
        if(rc != GT_OK)
        {
            return rc;
        }

        offset = U32_GET_FIELD_MAC(data,2,numBits_L2_ECMP_Start_Index);
    }
    else /* we use DB and we need to get info from it for our calculations */
    {
        if(devTrunkInfoPtr->maxNumMembersInTrunk_isPerTrunk == GT_FALSE)
        {
            if(numMembers > devTrunkInfoPtr->maxNumMembersInTrunk)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
                    "The trunk[%d] supports max of [%d] members and not allow [%d] members \n",
                    trunkId,
                    devTrunkInfoPtr->maxNumMembersInTrunk,
                    numMembers);
            }

            /* use the entries in the higher part of the table */
            /* use (trunkId-1) because we skip the 'unused trunkId=0' */
            offset = (PRV_CPSS_DXCH_PP_MAC(devNum)->bridge.l2EcmpNumMembers -
                      PRV_CPSS_DXCH_PP_MAC(devNum)->bridge.l2EcmpNumMembers_usedByTrunk)
                     + ((trunkId - 1) * devTrunkInfoPtr->maxNumMembersInTrunk);
        }
        else
        if(trunkId > devTrunkInfoPtr->numberOfTrunks)
        {
            /* we allow to initialize the trunks above the 'max trunkId' to 'empty trunks' */
            /* it is mainly done by the 'trunkInit' */
            if(numMembers != 0)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
                    "The trunk[%d] is over max of [%d] trunks and not allow any members (asked for[%d] members) \n",
                    trunkId,
                    devTrunkInfoPtr->numberOfTrunks,
                    numMembers);
            }

            offset = 0;/* dummy ... will not be used , because numMembers == 0 */
        }
        else
        {
            trunkEntryPtr = &devTrunkInfoPtr->trunksArray[trunkId];

            if(numMembers > trunkEntryPtr->flexInfo.myTrunkMaxNumOfMembers)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
                    "The trunk[%d] supports max of [%d] members and not allow [%d] members \n",
                    trunkId,
                    trunkEntryPtr->flexInfo.myTrunkMaxNumOfMembers,
                    numMembers);
            }

            offset = trunkEntryPtr->flexInfo.myTrunkStartIndexInMembersTable;
            if(trunkEntryPtr->flexInfo.myTrunkMaxNumOfMembers == 0)
            {
                /* THE TRUNK NEED TO BE SHUTDOWN */
                numMembers = 0;
            }
        }
    }

    if(eArchDoStrechNumMembersWa && numMembers > 1 &&
        devTrunkInfoPtr->shadowValid == GT_TRUE)
    {
        /* this mode is to support 'force' 8 members in the trunk due to new hash calc.

        The legacy device did :
            member_Index = packet_hash % num_Of_Members
        The 'eArch' do :
            member_Index = (packet_hash * num_Of_Members) >> num_bits_right_shift

        so to get about the same results , we set :
        1. num_Of_Members = 8 members
        2. num_bits_right_shift = 6 (because simple hash generate 6 bits)

        so we will get:

        The 'eArch' do :
            member_Index = (packet_hash * 8) >> 6 = packet_hash >> 3 =
                    packet_hash / 8 = packet_hash[5:3]


        if we compare to 'Legacy devices' behavior of 8 members :
            member_Index = packet_hash % num_Of_Members =
                    packet_hash % 8 = packet_hash[2:0]

            --> so comparing shows that we uses different bits from the hash.
         */
         /* example for 'max' of 8 members in trunk :
         static GT_U32     stretchNumArr[E_ARCH_STRECHED_NUM_MAX_CNS] = {
            0,*stretch number for 0 members  not relevant *
            1,*stretch number for 1 members  not relevant *
            8,*stretch number for 2 members
            6,*stretch number for 3 members
            8 *stretch number for 4 members
            };
         */
        if(devTrunkInfoPtr->maxNumMembersInTrunk_isPerTrunk == GT_FALSE)
        {
            maxStretch = devTrunkInfoPtr->maxNumMembersInTrunk;
        }
        else
        {
            trunkEntryPtr = &devTrunkInfoPtr->trunksArray[trunkId];
            maxStretch = trunkEntryPtr->flexInfo.myTrunkMaxNumOfMembers;
        }

        /* integer arithmetic to remove 'modulo' from the number */
        maxStretch = (maxStretch / numMembers) * numMembers;

        if(maxStretch >
            (eArchDoStrechNumMembersWa_maxDuplications * numMembers))
        {
            /* limit the number of replications to '4' as this number seems enough
               to give better load balance */
            maxStretch =  eArchDoStrechNumMembersWa_maxDuplications * numMembers;
        }

        if(numMembers < eArchDoStrechNumMembersWa_maxMembersAfterStretch &&
           maxStretch > eArchDoStrechNumMembersWa_maxMembersAfterStretch)
        {
            /* we not want to stretch this number of members beyond 64 members */
            maxStretch = (eArchDoStrechNumMembersWa_maxMembersAfterStretch / numMembers) * numMembers;
        }

        if(flexCompatibleTo8Members == GT_TRUE &&
           (numMembers <= 8) && maxStretch >= 8)
        {
            /* allow to behave like the legacy mode (when 'fixed' max of 8 members)
                and the number of members is up to 8 members (inclusive) */
            maxStretch = 8;
            /* integer arithmetic to remove 'modulo' from the number */
            maxStretch = (maxStretch / numMembers) * numMembers;
        }
    }
    else
    {
        /* no stretching needed/wanted */
        maxStretch = numMembers;
    }


    /* set the members , each in different entry */
    for (ii = 0;ii < maxStretch; ii ++)
    {
        memberIndex = ii % numMembers;

       /* check phyPort and hwDev*/
        if((membersArray[memberIndex].port      > devTrunkInfoPtr->validityMask.port) ||
           (membersArray[memberIndex].hwDevice  > devTrunkInfoPtr->validityMask.hwDevice))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        data = membersArray[memberIndex].hwDevice |
               (membersArray[memberIndex].port << 10);

        rc = prvCpssDxChWriteTableEntry(devNum,
            CPSS_DXCH_SIP5_TABLE_EQ_L2_ECMP_E,
            offset + ii,
            &data);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    /* maxStretch : the actual number we need to write to the HW as 'number of members' */

    if(maxStretch)
    {
        writeLttEntry_lbl:

        data = 1 | /*<ECMP Enable> = 1*/
                ((maxStretch-1) << (2+numBits_L2_ECMP_Start_Index)) | /*<Number of L2 ECMP Paths>*/
                (offset << 2);/*<L2 ECMP Start Index>*/

        if(debug_internal_cpssDxChTrunkTableEntrySet_sip5)
        {
            cpssOsPrintf("TRUNK : LTT entry [%d] Enabled , <L2 ECMP Start Index>[%d], <Number of L2 ECMP Paths + 1> = [%d] , numMembers[%d] \n",
                trunkId,offset,
                maxStretch,numMembers);
        }
    }
    else
    {
        if(devTrunkInfoPtr->shadowValid == GT_TRUE)
        {
            /*
                CPSS should not allow a 'used trunk' to hold <ECMP Enable> = disabled. (when application consider 'empty trunk')
                Instead the CPSS must set:
                <ECMP Enable> = true ,
                <Number of L2 ECMP Paths> = 0 --one member-- ,
                < L2 ECMP Start Index > should point to 'NULL port' (62) in the 'L2ECMP table' (reserved index in 'L2ECMP table')
            */
            #if 0 /* set here the related 'WA' that we set 'empty' trunks that
                    trunk LTT will point to 'NULL port' */
                  /* it is for 'reference' purposes */
                PRV_CPSS_DXCH_BOBCAT2_RM_TRUNK_LTT_ENTRY_WRONG_DEFAULT_VALUES_WA_E
            #endif


            L2_ECMP_NULL_PORT_BOOKMARK
            /* we need extra index to be untouched (last index) to hold the NULL port */
            maxStretch = 1;/* one member */
            offset = PRV_CPSS_DXCH_PP_MAC(devNum)->bridge.l2EcmpIndex_NULL_port;

            if(debug_internal_cpssDxChTrunkTableEntrySet_sip5)
            {
                cpssOsPrintf("TRUNK : set 'empty trunk' [%d] to point to NULL port \n",
                    trunkId);
            }
            goto writeLttEntry_lbl;
        }

        /* for low level we allow to set it ! */

        if(debug_internal_cpssDxChTrunkTableEntrySet_sip5)
        {
            cpssOsPrintf("TRUNK : LTT entry [%d] Disabled \n",
                trunkId);
        }
        data = 0;/*<ECMP Enable> = 0*/
    }

    rc = prvCpssDxChWriteTableEntry(devNum,
        CPSS_DXCH_SIP5_TABLE_EQ_TRUNK_LTT_E,
        trunkId,
        &data);
    if(rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal internal_cpssDxChTrunkTableEntrySet function
* @endinternal
*
* @brief   Function Relevant mode : Low level mode
*         Set the trunk table entry , and set the number of members in it.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] trunkId                  - trunk id
* @param[in] numMembers               - num of enabled members in the trunk
*                                      Note : value 0 is not recommended.
* @param[in] membersArray[]           - array of enabled members of the trunk
*
* @retval GT_OK                    - successful completion
* @retval GT_FAIL                  - an error occurred.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - numMembers exceed the number of maximum number
* @retval of members in trunk (total of 0 - 8 members allowed)
* @retval GT_BAD_PARAM             - bad device number , or
*                                       bad trunkId number , or
*                                       bad members parameters :
*                                       (device & 0xE0) != 0 means that the HW can't support
*                                       this value , since HW has 5 bit
*                                       for device number
*                                       (port & 0xC0) != 0 means that the HW can't support
*                                       this value , since HW has 6 bit
*                                       for port number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTrunkTableEntrySet
(
    IN  GT_U8                   devNum,
    IN  GT_TRUNK_ID             trunkId,
    IN  GT_U32                  numMembers,
    IN  CPSS_TRUNK_MEMBER_STC   membersArray[]
)
{
    GT_STATUS               rc;              /* return error code */
    GT_U32                  regAddr;         /* hw memory address to write */
    GT_U32                  offset;          /* offset of field in the register */
    GT_U32                  length;          /* length of field in the register */
    GT_U32                  data;            /* data to write to register */
    GT_U32                  ii;              /* iterator */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    HW_TRUNK_ID_AS_INDEX_IN_TRUNK_MEMBERS_CHECK_MAC(devNum,trunkId);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        return
            internal_cpssDxChTrunkTableEntrySet_sip5(devNum,trunkId,numMembers,membersArray);
    }

    /* in SIP5 we allow NULL pointer (when numMembers == 0)*/
    CPSS_NULL_PTR_CHECK_MAC(membersArray);


    if(numMembers > PRV_CPSS_TRUNK_8_MAX_NUM_OF_MEMBERS_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* NOTE : if (numMembers == 0) the PP will not work "as expected" with
    this configuration */

    for (ii = 0;ii < numMembers; ii ++)
    {
        rc = chTrunkMembersTableAccessWrite(devNum,trunkId,ii,&membersArray[ii]);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    /* do the WA (if needed) for "wrong trunk id source port information of packet to CPU" erratum */
    rc = trunkTrafficToCpuWaTrunkTableEntrySet(devNum,trunkId,numMembers,membersArray);
    if(rc != GT_OK)
    {
        return rc;
    }

    if(GT_TRUE == PRV_CPSS_DXCH_ERRATA_GET_MAC(devNum,PRV_CPSS_DXCH_TRUNK_CONFIG_FOR_TRUNK_NEXT_HOP_WA_E))
    {
        /* Title : Fer#137: Trunk configuration for Trunk Next Hop

        Relevant for:
        =============
        Relevant for: 98DX167/247-A1/A2 and 98DX253/263/273-A1/A298DX107-BCW-A2/B0
        and 98DX107-LKJB098DX163-A1/A2, 98DX163R-A2, and 98DX243-A1/A298DX262-A2

        Description:
        =============
        When the Next Hop is a trunk, the router sets the TrunkHash in the
        descriptor to PktHash[2:0]. This means that the value of the TrunkHash
        may be set to any value: 0 through 7.
        If the Next Hop is a trunk with less than 8 members, the device may
        fetch an invalid trunk member.

        Functional Impact:
        =================
        The application must configure 8 entries in the Trunks Members tables
        with valid members.

        Work Around:
        =============
        All of the entries in the Trunks Members table of trunks that are
        associated with Next Hop must be configured with trunk members.
        For example, for a trunk that has two members:
        -- Trunk<n>Member0 <= Member0
        -- Trunk<n>Member1 <= Member1
        -- Trunk<n>Member2 <= Member0
        -- Trunk<n>Member3 <= Member1
        -- Trunk<n>Member4 <= Member0
        -- Trunk<n>Member5 <= Member1
        -- Trunk<n>Member6 <= Member0
        -- Trunk<n>Member7 <= Member1

        */

        for(ii = numMembers ; ii < PRV_CPSS_TRUNK_8_MAX_NUM_OF_MEMBERS_CNS; ii++)
        {
            rc = chTrunkMembersTableAccessWrite(devNum,trunkId,ii,
                    &membersArray[ii % numMembers]);/* set redundant members */
            if(rc != GT_OK)
            {
                return rc;
            }
        }

        /*
            NOTE : the fix for that Erratum is not changing the value
                "num of members in trunk" -- only set redundant members.
            -- this is in order not to effect all L2 trunk load balancing.
            -- that way we don't care it it is cheetah (not support L3)
        */
    }

    /* update trunk counter register */
    length = 4;/* 4 bits per trunk */
    data = numMembers;

    REGISTER_ADDR_IN_NUM_MEMBERS_TRUNK_TABLE_MAC(devNum,trunkId,(&regAddr));

    offset = length * ((trunkId-1) & 0x7);

    /* write the counter to the HW */
    return prvCpssHwPpSetRegField(devNum,regAddr,offset,length,data);
}

/**
* @internal cpssDxChTrunkTableEntrySet function
* @endinternal
*
* @brief   Function Relevant mode : Low level mode
*         Set the trunk table entry , and set the number of members in it.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] trunkId                  - trunk id
* @param[in] numMembers               - num of enabled members in the trunk
*                                      Note : value 0 is not recommended.
* @param[in] membersArray[]           - array of enabled members of the trunk
*
* @retval GT_OK                    - successful completion
* @retval GT_FAIL                  - an error occurred.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - numMembers exceed the number of maximum number
* @retval of members in trunk (total of 0 - 8 members allowed)
* @retval GT_BAD_PARAM             - bad device number , or
*                                       bad trunkId number , or
*                                       bad members parameters :
*                                       (device & 0xE0) != 0 means that the HW can't support
*                                       this value , since HW has 5 bit
*                                       for device number
*                                       (port & 0xC0) != 0 means that the HW can't support
*                                       this value , since HW has 6 bit
*                                       for port number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTrunkTableEntrySet
(
    IN  GT_U8                   devNum,
    IN  GT_TRUNK_ID             trunkId,
    IN  GT_U32                  numMembers,
    IN  CPSS_TRUNK_MEMBER_STC   membersArray[]
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTrunkTableEntrySet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, trunkId, numMembers, membersArray));

    rc = internal_cpssDxChTrunkTableEntrySet(devNum, trunkId, numMembers, membersArray);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, trunkId, numMembers, membersArray));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTrunkTableEntryGet function
* @endinternal
*
* @brief   Function Relevant mode : All modes
*         Get the trunk table entry , and get the number of members in it.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] trunkId                  - trunk id
*
* @param[out] numMembersPtr            - (pointer to)num of members in the trunk
* @param[out] membersArray[]           - array of enabled members of the trunk
*                                      array is allocated by the caller , it is assumed that
*                                      the array can hold CPSS_TRUNK_MAX_NUM_OF_MEMBERS_CNS
*                                      members
*
* @retval GT_OK                    - successful completion
* @retval GT_FAIL                  - an error occurred.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or
*                                       bad trunkId number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTrunkTableEntryGet
(
    IN  GT_U8                   devNum,
    IN  GT_TRUNK_ID             trunkId,
    OUT GT_U32                  *numMembersPtr,
    OUT CPSS_TRUNK_MEMBER_STC   membersArray[]
)
{
    GT_STATUS       rc;                 /* return error code */
    GT_U32          regAddr;            /* hw memory address to write */
    GT_U32          offset;             /* offset of field in the register */
    GT_U32          length;             /* length of field in the register */
    GT_U32          data;               /* data to write to register */
    GT_U32          ii;                  /* iterator */
    GT_U32          device,port;        /*device and port */
    GT_U32          numBits_L2_ECMP_Start_Index;/* number of bits in <L2_ECMP_Start_Index> */
    GT_U32          numBits_target_eport_phy_port; /*number of bits in <Target_ePort_phy_port> */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    HW_TRUNK_ID_AS_INDEX_IN_TRUNK_MEMBERS_CHECK_MAC(devNum,trunkId);
    CPSS_NULL_PTR_CHECK_MAC(numMembersPtr);
    CPSS_NULL_PTR_CHECK_MAC(membersArray);/* must always be valid and must be
            able to hold CPSS_TRUNK_MAX_NUM_OF_MEMBERS_CNS of members */

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        /* the info read from the trunk LTT and L2ECMP Table */

        rc = prvCpssDxChReadTableEntry(devNum,
            CPSS_DXCH_SIP5_TABLE_EQ_TRUNK_LTT_E,
            trunkId,
            &data);
        if(rc != GT_OK)
        {
            return rc;
        }

        if(0 == U32_GET_FIELD_MAC(data,0,1))/*<ECMP Enable>*/
        {
            *numMembersPtr = 0;
            return GT_OK;
        }

        numBits_L2_ECMP_Start_Index = NUMBITS_L2_ECMP_START_INDEX_MAC(devNum);
        numBits_target_eport_phy_port =
            PRV_CPSS_SIP_6_CHECK_MAC(devNum)    ? 13 :
            PRV_CPSS_SIP_5_20_CHECK_MAC(devNum) ? 14 : 13;

        *numMembersPtr = 1 + (U32_GET_FIELD_MAC(data,2+numBits_L2_ECMP_Start_Index,12));
        offset = U32_GET_FIELD_MAC(data,2,numBits_L2_ECMP_Start_Index);

        /* get the members , each in different entry */
        for (ii = 0;ii < (*numMembersPtr); ii ++)
        {
            rc = prvCpssDxChReadTableEntry(devNum,
                CPSS_DXCH_SIP5_TABLE_EQ_L2_ECMP_E,
                offset + ii,
                &data);
            if(rc != GT_OK)
            {
                return rc;
            }

            device = U32_GET_FIELD_MAC(data, 0,10);
            port = U32_GET_FIELD_MAC(data,10,numBits_target_eport_phy_port);

            if(eArchDoStrechNumMembersWa && (ii > 0) &&
                (PRV_CPSS_DEV_TRUNK_INFO_MAC(devNum)->shadowValid == GT_TRUE))
            {
                if( (device == membersArray[0].hwDevice) &&
                    (port   == membersArray[0].port))
                {
                    /* the first member is duplicated , to current index too */
                    /* so we need to break the loop , and update the number of members */
                    (*numMembersPtr) = ii;
                    break;
                }
            }

            membersArray[ii].hwDevice =  device;
            membersArray[ii].port   =  port;
        }

        return GT_OK;
    }

    /* update trunk counter register */
    length = 4;/* 4 bits per trunk */

    REGISTER_ADDR_IN_NUM_MEMBERS_TRUNK_TABLE_MAC(devNum,trunkId,(&regAddr));

    offset = length * ((trunkId - 1) & 0x7);

    /* write the counter to the HW */
    prvCpssHwPpGetRegField(devNum,regAddr,offset,length,&data);

    *numMembersPtr = data; /* let the caller get the actual number of members */

    for (ii = 0; ii < (*numMembersPtr); ii ++)
    {
        rc = chTrunkMembersTableAccessRead(devNum,trunkId,ii,&membersArray[ii]);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal cpssDxChTrunkTableEntryGet function
* @endinternal
*
* @brief   Function Relevant mode : All modes
*         Get the trunk table entry , and get the number of members in it.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] trunkId                  - trunk id
*
* @param[out] numMembersPtr            - (pointer to)num of members in the trunk
* @param[out] membersArray[]           - array of enabled members of the trunk
*                                      array is allocated by the caller , it is assumed that
*                                      the array can hold CPSS_TRUNK_MAX_NUM_OF_MEMBERS_CNS
*                                      members
*
* @retval GT_OK                    - successful completion
* @retval GT_FAIL                  - an error occurred.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or
*                                       bad trunkId number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTrunkTableEntryGet
(
    IN  GT_U8                   devNum,
    IN  GT_TRUNK_ID             trunkId,
    OUT GT_U32                  *numMembersPtr,
    OUT CPSS_TRUNK_MEMBER_STC   membersArray[]
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTrunkTableEntryGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, trunkId, numMembersPtr, membersArray));

    rc = internal_cpssDxChTrunkTableEntryGet(devNum, trunkId, numMembersPtr, membersArray);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, trunkId, numMembersPtr, membersArray));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal sip5TrunkNonTrunkPortsEntrySet function
* @endinternal
*
* @brief   Function Relevant mode : Low level mode
*         Set the trunk's non-trunk ports specific bitmap entry : in table1,table2
*         for trunkId that is bigger then the number of trunks in table1 , this
*         table is skipped.
*         table2 always accessed.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] trunkId                  - trunk id - in this API  can be ZERO !
* @param[in] nonTrunkPortsPtr         - (pointer to) non trunk port bitmap of the trunk.
*
* @retval GT_OK                    - successful completion
* @retval GT_FAIL                  - an error occurred.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or
*                                       bad trunkId number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS sip5TrunkNonTrunkPortsEntrySet
(
    IN  GT_U8               devNum,
    IN  GT_TRUNK_ID         trunkId,
    IN  CPSS_PORTS_BMP_STC  *nonTrunkPortsPtr
)
{
    GT_STATUS   rc;

    /* check that the device supports all the physical ports that defined in the BMP */
    rc = prvCpssDxChTablePortsBmpCheck(devNum , nonTrunkPortsPtr);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* in E_ARCH table CPSS_DXCH_LION_TABLE_TXQ_NON_TRUNK_MEMBERS_E used for
       cascade trunks , this is smaller table then the table for regular trunks

       but for 'easier' implementation we write to it also entries that we will
       write to 'table 2' (as long that index is valid in 'table 1').
    */
    if(trunkId < PRV_CPSS_DXCH_PP_HW_INFO_TRUNK_MAC(devNum).nonTrunkTable1NumEntries)
    {
        /* call the generic function that handle BMP of ports in the EGF_SHT */
        rc = prvCpssDxChHwEgfShtPortsBmpTableEntryWrite(devNum,
                CPSS_DXCH_LION_TABLE_TXQ_NON_TRUNK_MEMBERS_E,
                trunkId,
                nonTrunkPortsPtr,
                GT_FALSE,
                GT_FALSE);/*don't care*/
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    /* in E_ARCH table CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_SHT_NON_TRUNK_MEMBERS2_E used for
       regular trunks (and not for cascade trunk)


       but for 'easier' implementation we write to it also entries that we wrote
       to 'table 1'.
    */

    /* call the generic function that handle BMP of ports in the EGF_SHT */
    rc = prvCpssDxChHwEgfShtPortsBmpTableEntryWrite(devNum,
            CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_SHT_NON_TRUNK_MEMBERS2_E,
            trunkId,
            nonTrunkPortsPtr,
            GT_FALSE,
            GT_FALSE);/*don't care*/

    return rc;
}

/**
* @internal internal_cpssDxChTrunkNonTrunkPortsEntrySet function
* @endinternal
*
* @brief   Function Relevant mode : Low level mode
*         Set the trunk's non-trunk ports specific bitmap entry.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] trunkId                  - trunk id - in this API  can be ZERO !
* @param[in] nonTrunkPortsPtr         - (pointer to) non trunk port bitmap of the trunk.
*
* @retval GT_OK                    - successful completion
* @retval GT_FAIL                  - an error occurred.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or
*                                       bad trunkId number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTrunkNonTrunkPortsEntrySet
(
    IN  GT_U8               devNum,
    IN  GT_TRUNK_ID         trunkId,
    IN  CPSS_PORTS_BMP_STC  *nonTrunkPortsPtr
)
{
    GT_U32  regAddr;   /* hw memory address to write */
    GT_STATUS   rc;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    HW_TRUNK_ID_AS_INDEX_IN_NON_TRUNK_PORTS_CHECK_MAC(devNum,trunkId);
    CPSS_NULL_PTR_CHECK_MAC(nonTrunkPortsPtr);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        rc = sip5TrunkNonTrunkPortsEntrySet(devNum,trunkId,nonTrunkPortsPtr);
        return rc;
    }
    else
    if(0 == PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(devNum))
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bufferMng.
                trunkTblCfgRegs.baseAddrNonTrunk +
                CHEETAH_TRUNK_NON_TRUNK_TABLE_OFFSET_CNS * trunkId;

        return prvCpssHwPpSetRegField(devNum, regAddr, 0,
                        PRV_CPSS_PP_MAC(devNum)->numOfPorts,
                        nonTrunkPortsPtr->ports[0]);
    }
    else
    {
        /* call the generic function that handle BMP of ports in the EGF_SHT */
        rc = prvCpssDxChHwEgfShtPortsBmpTableEntryWrite(devNum,
                CPSS_DXCH_LION_TABLE_TXQ_NON_TRUNK_MEMBERS_E,
                trunkId,
                nonTrunkPortsPtr,
                GT_FALSE,
                GT_FALSE);/*don't care*/

        return rc;
    }
}

/**
* @internal cpssDxChTrunkNonTrunkPortsEntrySet function
* @endinternal
*
* @brief   Function Relevant mode : Low level mode
*         Set the trunk's non-trunk ports specific bitmap entry.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] trunkId                  - trunk id - in this API  can be ZERO !
* @param[in] nonTrunkPortsPtr         - (pointer to) non trunk port bitmap of the trunk.
*
* @retval GT_OK                    - successful completion
* @retval GT_FAIL                  - an error occurred.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or
*                                       bad trunkId number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTrunkNonTrunkPortsEntrySet
(
    IN  GT_U8               devNum,
    IN  GT_TRUNK_ID         trunkId,
    IN  CPSS_PORTS_BMP_STC  *nonTrunkPortsPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTrunkNonTrunkPortsEntrySet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, trunkId, nonTrunkPortsPtr));

    rc = internal_cpssDxChTrunkNonTrunkPortsEntrySet(devNum, trunkId, nonTrunkPortsPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, trunkId, nonTrunkPortsPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal sip5TrunkNonTrunkPortsEntryGet function
* @endinternal
*
* @brief   Function Relevant mode : All modes
*         Get the trunk's non-trunk ports bitmap specific entry.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] trunkId                  - trunk id - in this API  can be ZERO !
*
* @param[out] nonTrunkPortsPtr         - (pointer to) non trunk port bitmap of the trunk.
*
* @retval GT_OK                    - successful completion
* @retval GT_FAIL                  - an error occurred.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or
*                                       bad trunkId number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS sip5TrunkNonTrunkPortsEntryGet
(
    IN  GT_U8               devNum,
    IN  GT_TRUNK_ID         trunkId,
    OUT  CPSS_PORTS_BMP_STC  *nonTrunkPortsPtr
)
{
    GT_STATUS rc;/* 'table 2' holds all entries */

    /* call the generic function that handle BMP of ports in the EGF_SHT */
    rc = prvCpssDxChHwEgfShtPortsBmpTableEntryRead(devNum,
            CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_SHT_NON_TRUNK_MEMBERS2_E,
            trunkId,
            nonTrunkPortsPtr,
            GT_FALSE,
            NULL);/*don't care*/

    if(rc != GT_OK)
    {
        return rc;
    }
    /* adjusted physical ports of BMP to hold only bmp of existing ports*/
    rc = prvCpssDxChTablePortsBmpAdjustToDevice(devNum , nonTrunkPortsPtr , nonTrunkPortsPtr);

    return rc;
}

/**
* @internal internal_cpssDxChTrunkNonTrunkPortsEntryGet function
* @endinternal
*
* @brief   Function Relevant mode : All modes
*         Get the trunk's non-trunk ports bitmap specific entry.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] trunkId                  - trunk id - in this API  can be ZERO !
*
* @param[out] nonTrunkPortsPtr         - (pointer to) non trunk port bitmap of the trunk.
*
* @retval GT_OK                    - successful completion
* @retval GT_FAIL                  - an error occurred.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or
*                                       bad trunkId number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTrunkNonTrunkPortsEntryGet
(
    IN  GT_U8               devNum,
    IN  GT_TRUNK_ID         trunkId,
    OUT  CPSS_PORTS_BMP_STC  *nonTrunkPortsPtr
)
{
    GT_U32  regAddr;   /* hw memory address to read */
    GT_STATUS   rc;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    HW_TRUNK_ID_AS_INDEX_IN_NON_TRUNK_PORTS_CHECK_MAC(devNum,trunkId);
    CPSS_NULL_PTR_CHECK_MAC(nonTrunkPortsPtr);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        rc = sip5TrunkNonTrunkPortsEntryGet(devNum,trunkId,nonTrunkPortsPtr);
        return rc;
    }
    else
    if(0 == PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(devNum))
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bufferMng.
                trunkTblCfgRegs.baseAddrNonTrunk +
                CHEETAH_TRUNK_NON_TRUNK_TABLE_OFFSET_CNS * trunkId;

        CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(nonTrunkPortsPtr);

        return prvCpssHwPpGetRegField(devNum, regAddr, 0,
                        PRV_CPSS_PP_MAC(devNum)->numOfPorts,
                        &nonTrunkPortsPtr->ports[0]);
    }
    else
    {
        /* call the generic function that handle BMP of ports in the EGF_SHT */
        rc = prvCpssDxChHwEgfShtPortsBmpTableEntryRead(devNum,
                CPSS_DXCH_LION_TABLE_TXQ_NON_TRUNK_MEMBERS_E,
                trunkId,
                nonTrunkPortsPtr,
                GT_FALSE,
                NULL);/*don't care*/

        return rc;
    }
}

/**
* @internal cpssDxChTrunkNonTrunkPortsEntryGet function
* @endinternal
*
* @brief   Function Relevant mode : All modes
*         Get the trunk's non-trunk ports bitmap specific entry.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] trunkId                  - trunk id - in this API  can be ZERO !
*
* @param[out] nonTrunkPortsPtr         - (pointer to) non trunk port bitmap of the trunk.
*
* @retval GT_OK                    - successful completion
* @retval GT_FAIL                  - an error occurred.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or
*                                       bad trunkId number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTrunkNonTrunkPortsEntryGet
(
    IN  GT_U8               devNum,
    IN  GT_TRUNK_ID         trunkId,
    OUT  CPSS_PORTS_BMP_STC  *nonTrunkPortsPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTrunkNonTrunkPortsEntryGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, trunkId, nonTrunkPortsPtr));

    rc = internal_cpssDxChTrunkNonTrunkPortsEntryGet(devNum, trunkId, nonTrunkPortsPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, trunkId, nonTrunkPortsPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChTrunkDesignatedPortsEntrySet function
* @endinternal
*
* @brief   Function Relevant mode : Low level mode
*         Set the designated trunk table specific entry.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] entryIndex               - the index in the designated ports bitmap table
* @param[in] designatedPortsPtr       - (pointer to) designated ports bitmap
*
* @retval GT_OK                    - successful completion
* @retval GT_FAIL                  - an error occurred.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_OUT_OF_RANGE          - entryIndex exceed the number of HW table.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTrunkDesignatedPortsEntrySet
(
    IN  GT_U8               devNum,
    IN  GT_U32              entryIndex,
    IN  CPSS_PORTS_BMP_STC  *designatedPortsPtr
)
{
    GT_U32  regAddr;   /* hw memory address to write */
    GT_STATUS   rc;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(designatedPortsPtr);
    HW_DESIGNATED_PORTS_ENTRY_INDEX_CHECK_MAC(devNum,entryIndex);

    if(0 == PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(devNum))
    {
        /*write the low register*/
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bufferMng.
            trunkTblCfgRegs.baseAddrTrunkDesig +
            CHEETAH_TRUNK_DESIGNATED_PORTS_TABLE_OFFSET_CNS * entryIndex;

        return prvCpssHwPpSetRegField(devNum, regAddr, 0,
                        PRV_CPSS_PP_MAC(devNum)->numOfPorts,
                        designatedPortsPtr->ports[0]);
    }
    else
    {
#ifdef GM_USED
        /* GM have problem with port 255 */
        /*designatedPortsPtr->ports[7] &= 0x7FFFFFFF;*/
        CPSS_PORTS_BMP_PORT_ENABLE_MAC(designatedPortsPtr,255,GT_FALSE);
#endif /*GM_USED*/

        if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
        {
            /* check that the device supports all the physical ports that defined in the BMP */
            rc = prvCpssDxChTablePortsBmpCheck(devNum , designatedPortsPtr);
            if(rc != GT_OK)
            {
                return rc;
            }
        }

        /* call the generic function that handle BMP of ports in the EGF_SHT */
        rc = prvCpssDxChHwEgfShtPortsBmpTableEntryWrite(devNum,
                CPSS_DXCH_LION_TABLE_TXQ_DESIGNATED_PORT_E,
                entryIndex,
                designatedPortsPtr,
                GT_FALSE,
                GT_FALSE);/*don't care*/

        return rc;
    }
}

/**
* @internal cpssDxChTrunkDesignatedPortsEntrySet function
* @endinternal
*
* @brief   Function Relevant mode : Low level mode
*         Set the designated trunk table specific entry.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] entryIndex               - the index in the designated ports bitmap table
* @param[in] designatedPortsPtr       - (pointer to) designated ports bitmap
*
* @retval GT_OK                    - successful completion
* @retval GT_FAIL                  - an error occurred.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_OUT_OF_RANGE          - entryIndex exceed the number of HW table.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTrunkDesignatedPortsEntrySet
(
    IN  GT_U8               devNum,
    IN  GT_U32              entryIndex,
    IN  CPSS_PORTS_BMP_STC  *designatedPortsPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTrunkDesignatedPortsEntrySet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, entryIndex, designatedPortsPtr));

    rc = internal_cpssDxChTrunkDesignatedPortsEntrySet(devNum, entryIndex, designatedPortsPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, entryIndex, designatedPortsPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTrunkDesignatedPortsEntryGet function
* @endinternal
*
* @brief   Function Relevant mode : All modes
*         Get the designated trunk table specific entry.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] entryIndex               - the index in the designated ports bitmap table
*
* @param[out] designatedPortsPtr       - (pointer to) designated ports bitmap
*
* @retval GT_OK                    - successful completion
* @retval GT_FAIL                  - an error occurred.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_OUT_OF_RANGE          - entryIndex exceed the number of HW table.
* @retval the index must be in range (0 - 7)
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTrunkDesignatedPortsEntryGet
(
    IN  GT_U8               devNum,
    IN  GT_U32              entryIndex,
    OUT  CPSS_PORTS_BMP_STC  *designatedPortsPtr
)
{
    GT_U32  regAddr;   /* hw memory address to write */
    GT_STATUS   rc;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(designatedPortsPtr);
    HW_DESIGNATED_PORTS_ENTRY_INDEX_CHECK_MAC(devNum,entryIndex);

    if(0 == PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(devNum))
    {
        /*read the low register*/
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bufferMng.
            trunkTblCfgRegs.baseAddrTrunkDesig +
            CHEETAH_TRUNK_DESIGNATED_PORTS_TABLE_OFFSET_CNS * entryIndex;

        CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(designatedPortsPtr);

        return prvCpssHwPpGetRegField(devNum, regAddr, 0,
                        PRV_CPSS_PP_MAC(devNum)->numOfPorts,
                        &designatedPortsPtr->ports[0]);
    }
    else
    {
        /* call the generic function that handle BMP of ports in the EGF_SHT */
        rc = prvCpssDxChHwEgfShtPortsBmpTableEntryRead(devNum,
                CPSS_DXCH_LION_TABLE_TXQ_DESIGNATED_PORT_E,
                entryIndex,
                designatedPortsPtr,
                GT_FALSE,
                NULL);/*don't care*/

        if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
        {
            if(rc != GT_OK)
            {
                return rc;
            }
            /* adjusted physical ports of BMP to hold only bmp of existing ports*/
            rc = prvCpssDxChTablePortsBmpAdjustToDevice(devNum , designatedPortsPtr , designatedPortsPtr);
        }

        return rc;
    }
}

/**
* @internal cpssDxChTrunkDesignatedPortsEntryGet function
* @endinternal
*
* @brief   Function Relevant mode : All modes
*         Get the designated trunk table specific entry.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] entryIndex               - the index in the designated ports bitmap table
*
* @param[out] designatedPortsPtr       - (pointer to) designated ports bitmap
*
* @retval GT_OK                    - successful completion
* @retval GT_FAIL                  - an error occurred.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_OUT_OF_RANGE          - entryIndex exceed the number of HW table.
* @retval the index must be in range (0 - 7)
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTrunkDesignatedPortsEntryGet
(
    IN  GT_U8               devNum,
    IN  GT_U32              entryIndex,
    OUT  CPSS_PORTS_BMP_STC  *designatedPortsPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTrunkDesignatedPortsEntryGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, entryIndex, designatedPortsPtr));

    rc = internal_cpssDxChTrunkDesignatedPortsEntryGet(devNum, entryIndex, designatedPortsPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, entryIndex, designatedPortsPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTrunkHashIpModeSet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         Enable/Disable the device from considering the IP SIP/DIP information,
*         when calculation the trunk hashing index for a packet.
*         Relevant when the is IPv4 or IPv6 and <TrunkHash Mode> = 0.
*         Note:
*         1. Not relevant to NON Ip packets.
*         2. Not relevant to multi-destination packets (include routed IPM).
*         3. Not relevant when cpssDxChTrunkHashGlobalModeSet(devNum,
*         CPSS_DXCH_TRUNK_LBH_INGRESS_PORT_E)
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number.
* @param[in] enable                   - GT_FALSE - IP data is not added to the trunk load balancing
*                                      hash.
*                                      GT_TRUE - The following function is added to the trunk load
*                                      balancing hash, if the packet is IPv6.
*                                      IPTrunkHash = according to setting of API
*                                      cpssDxChTrunkHashIpv6ModeSet(...)
*                                      else packet is IPv4:
*                                      IPTrunkHash = SIP[5:0]^DIP[5:0]^SIP[21:16]^DIP[21:16].
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTrunkHashIpModeSet
(
    IN GT_U8   devNum,
    IN GT_BOOL enable
)
{
    GT_U32      regAddr;         /* register address                         */
    GT_U32      value;           /* value to write to register               */
    GT_U32      startBit;        /* the word's bit at which the field starts */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr =
            PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).trunkHashSimpleHash.trunkHashConfigReg0;
        startBit  = 5;
    }
    else if (PRV_CPSS_DXCH_XCAT_FAMILY_CHECK_MAC(devNum))
    {
        /* xCat and above */
        regAddr =
            PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->ttiRegs.trunkHashCfg[0];
        startBit  = 5;
    }
    else
    {
        /* DXCH1-3 */
        regAddr =
            PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bridgeRegs.l2IngressControl;
        startBit  = 3;
    }

    value = (enable == GT_TRUE) ? 1 : 0;

    return prvCpssHwPpSetRegField(
        devNum, regAddr, startBit, 1, value);
}

/**
* @internal cpssDxChTrunkHashIpModeSet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         Enable/Disable the device from considering the IP SIP/DIP information,
*         when calculation the trunk hashing index for a packet.
*         Relevant when the is IPv4 or IPv6 and <TrunkHash Mode> = 0.
*         Note:
*         1. Not relevant to NON Ip packets.
*         2. Not relevant to multi-destination packets (include routed IPM).
*         3. Not relevant when cpssDxChTrunkHashGlobalModeSet(devNum,
*         CPSS_DXCH_TRUNK_LBH_INGRESS_PORT_E)
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number.
* @param[in] enable                   - GT_FALSE - IP data is not added to the trunk load balancing
*                                      hash.
*                                      GT_TRUE - The following function is added to the trunk load
*                                      balancing hash, if the packet is IPv6.
*                                      IPTrunkHash = according to setting of API
*                                      cpssDxChTrunkHashIpv6ModeSet(...)
*                                      else packet is IPv4:
*                                      IPTrunkHash = SIP[5:0]^DIP[5:0]^SIP[21:16]^DIP[21:16].
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTrunkHashIpModeSet
(
    IN GT_U8   devNum,
    IN GT_BOOL enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTrunkHashIpModeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enable));

    rc = internal_cpssDxChTrunkHashIpModeSet(devNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTrunkHashIpModeGet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         get the enable/disable of device from considering the IP SIP/DIP
*         information, when calculation the trunk hashing index for a packet.
*         Note:
*         1. Not relevant to NON Ip packets.
*         2. Not relevant to multi-destination packets (include routed IPM).
*         3. Not relevant when cpssDxChTrunkHashGlobalModeSet(devNum,
*         CPSS_DXCH_TRUNK_LBH_INGRESS_PORT_E)
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number.
*
* @param[out] enablePtr                -(pointer to)
*                                      GT_FALSE - IP data is not added to the trunk load balancing
*                                      hash.
*                                      GT_TRUE - The following function is added to the trunk load
*                                      balancing hash, if the packet is IPv6.
*                                      IPTrunkHash = according to setting of API
*                                      cpssDxChTrunkHashIpv6ModeSet(...)
*                                      else packet is IPv4:
*                                      IPTrunkHash = SIP[5:0]^DIP[5:0]^SIP[21:16]^DIP[21:16].
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTrunkHashIpModeGet
(
    IN GT_U8   devNum,
    OUT GT_BOOL *enablePtr
)
{
    GT_STATUS   rc;              /* returned code                            */
    GT_U32      regAddr;         /* register address                         */
    GT_U32      value;           /* value to read from register              */
    GT_U32      startBit;        /* the word's bit at which the field starts */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr =
            PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).trunkHashSimpleHash.trunkHashConfigReg0;
        startBit  = 5;
    }
    else if (PRV_CPSS_DXCH_XCAT_FAMILY_CHECK_MAC(devNum))
    {
        /* xCat and above */
        regAddr =
            PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->ttiRegs.trunkHashCfg[0];
        startBit  = 5;
    }
    else
    {
        /* DXCH1-3 */
        regAddr =
            PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bridgeRegs.l2IngressControl;
        startBit  = 3;
    }


    rc = prvCpssHwPpGetRegField(
        devNum, regAddr, startBit, 1, &value);

    *enablePtr = value ? GT_TRUE : GT_FALSE;

    return rc;
}

/**
* @internal cpssDxChTrunkHashIpModeGet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         get the enable/disable of device from considering the IP SIP/DIP
*         information, when calculation the trunk hashing index for a packet.
*         Note:
*         1. Not relevant to NON Ip packets.
*         2. Not relevant to multi-destination packets (include routed IPM).
*         3. Not relevant when cpssDxChTrunkHashGlobalModeSet(devNum,
*         CPSS_DXCH_TRUNK_LBH_INGRESS_PORT_E)
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number.
*
* @param[out] enablePtr                -(pointer to)
*                                      GT_FALSE - IP data is not added to the trunk load balancing
*                                      hash.
*                                      GT_TRUE - The following function is added to the trunk load
*                                      balancing hash, if the packet is IPv6.
*                                      IPTrunkHash = according to setting of API
*                                      cpssDxChTrunkHashIpv6ModeSet(...)
*                                      else packet is IPv4:
*                                      IPTrunkHash = SIP[5:0]^DIP[5:0]^SIP[21:16]^DIP[21:16].
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTrunkHashIpModeGet
(
    IN GT_U8   devNum,
    OUT GT_BOOL *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTrunkHashIpModeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enablePtr));

    rc = internal_cpssDxChTrunkHashIpModeGet(devNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTrunkHashL4ModeSet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         Enable/Disable the device from considering the L4 TCP/UDP
*         source/destination port information, when calculation the trunk hashing
*         index for a packet.
*         Note:
*         1. Not relevant to NON TCP/UDP packets.
*         2. The Ipv4 hash must also be enabled , otherwise the L4 hash mode
*         setting not considered.
*         3. Not relevant to multi-destination packets (include routed IPM).
*         4. Not relevant when cpssDxChTrunkHashGlobalModeSet(devNum,
*         CPSS_DXCH_CSCD_TRUNK_LINK_HASH_IS_SRC_PORT_E)
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number.
* @param[in] hashMode                 - L4 hash mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTrunkHashL4ModeSet
(
    IN GT_U8   devNum,
    IN CPSS_DXCH_TRUNK_L4_LBH_MODE_ENT hashMode
)
{
    GT_U32      regAddr;         /* register address                         */
    GT_U32      value;           /* HW data value                            */
    GT_U32      mask ;           /* HW data mask                             */
    GT_U32      l4HashBit;       /* l4 Hash Bit                              */
    GT_U32      l4HashLongBit;   /* l4 Hash Long Bit                              */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr =
            PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).trunkHashSimpleHash.trunkHashConfigReg0;
        l4HashBit      = BIT_4;
        l4HashLongBit  = BIT_0;
    }
    else if (PRV_CPSS_DXCH_XCAT_FAMILY_CHECK_MAC(devNum))
    {
        /* xCat and above */
        regAddr =
            PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->ttiRegs.trunkHashCfg[0];
        l4HashBit      = BIT_4;
        l4HashLongBit  = BIT_0;
    }
    else
    {
        /* DXCH1-3 */
        regAddr =
            PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bridgeRegs.l2IngressControl;
        l4HashBit      = BIT_4;
        l4HashLongBit  = BIT_14;
    }

    switch(hashMode)
    {
        case CPSS_DXCH_TRUNK_L4_LBH_DISABLED_E:
            value = 0;
            break;
        case CPSS_DXCH_TRUNK_L4_LBH_LONG_E:
            value = l4HashBit | l4HashLongBit;
            break;
        case CPSS_DXCH_TRUNK_L4_LBH_SHORT_E:
            value = l4HashBit;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    mask = l4HashBit | l4HashLongBit;

    return prvCpssHwPpWriteRegBitMask(devNum, regAddr, mask, value);
}

/**
* @internal cpssDxChTrunkHashL4ModeSet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         Enable/Disable the device from considering the L4 TCP/UDP
*         source/destination port information, when calculation the trunk hashing
*         index for a packet.
*         Note:
*         1. Not relevant to NON TCP/UDP packets.
*         2. The Ipv4 hash must also be enabled , otherwise the L4 hash mode
*         setting not considered.
*         3. Not relevant to multi-destination packets (include routed IPM).
*         4. Not relevant when cpssDxChTrunkHashGlobalModeSet(devNum,
*         CPSS_DXCH_CSCD_TRUNK_LINK_HASH_IS_SRC_PORT_E)
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number.
* @param[in] hashMode                 - L4 hash mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTrunkHashL4ModeSet
(
    IN GT_U8   devNum,
    IN CPSS_DXCH_TRUNK_L4_LBH_MODE_ENT hashMode
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTrunkHashL4ModeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, hashMode));

    rc = internal_cpssDxChTrunkHashL4ModeSet(devNum, hashMode);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, hashMode));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTrunkHashL4ModeGet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         get the Enable/Disable of device from considering the L4 TCP/UDP
*         source/destination port information, when calculation the trunk hashing
*         index for a packet.
*         Note:
*         1. Not relevant to NON TCP/UDP packets.
*         2. The Ipv4 hash must also be enabled , otherwise the L4 hash mode
*         setting not considered.
*         3. Not relevant to multi-destination packets (include routed IPM).
*         4. Not relevant when cpssDxChTrunkHashGlobalModeSet(devNum,
*         CPSS_DXCH_CSCD_TRUNK_LINK_HASH_IS_SRC_PORT_E)
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number.
*
* @param[out] hashModePtr              - (pointer to)L4 hash mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTrunkHashL4ModeGet
(
    IN GT_U8   devNum,
    OUT CPSS_DXCH_TRUNK_L4_LBH_MODE_ENT *hashModePtr
)
{
    GT_STATUS   rc;              /* returned code                            */
    GT_U32      regAddr;         /* register address                         */
    GT_U32      value;           /* HW data value                            */
    GT_U32      l4HashBit;       /* l4 Hash Bit                              */
    GT_U32      l4HashLongBit;   /* l4 Hash Long Bit                              */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(hashModePtr);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr =
            PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).trunkHashSimpleHash.trunkHashConfigReg0;
        l4HashBit      = BIT_4;
        l4HashLongBit  = BIT_0;
    }
    else if (PRV_CPSS_DXCH_XCAT_FAMILY_CHECK_MAC(devNum))
    {
        /* xCat and above */
        regAddr =
            PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->ttiRegs.trunkHashCfg[0];
        l4HashBit      = BIT_4;
        l4HashLongBit  = BIT_0;
    }
    else
    {
        /* DXCH1-3 */
        regAddr =
            PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bridgeRegs.l2IngressControl;
        l4HashBit      = BIT_4;
        l4HashLongBit  = BIT_14;
    }

    rc = prvCpssHwPpReadRegister(devNum, regAddr, &value);

    if(value & l4HashBit)
    {
        if(value & l4HashLongBit)
        {
            *hashModePtr = CPSS_DXCH_TRUNK_L4_LBH_LONG_E;
        }
        else
        {
            *hashModePtr = CPSS_DXCH_TRUNK_L4_LBH_SHORT_E;
        }
    }
    else
    {
        *hashModePtr = CPSS_DXCH_TRUNK_L4_LBH_DISABLED_E;
    }

    return rc;
}

/**
* @internal cpssDxChTrunkHashL4ModeGet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         get the Enable/Disable of device from considering the L4 TCP/UDP
*         source/destination port information, when calculation the trunk hashing
*         index for a packet.
*         Note:
*         1. Not relevant to NON TCP/UDP packets.
*         2. The Ipv4 hash must also be enabled , otherwise the L4 hash mode
*         setting not considered.
*         3. Not relevant to multi-destination packets (include routed IPM).
*         4. Not relevant when cpssDxChTrunkHashGlobalModeSet(devNum,
*         CPSS_DXCH_CSCD_TRUNK_LINK_HASH_IS_SRC_PORT_E)
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number.
*
* @param[out] hashModePtr              - (pointer to)L4 hash mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTrunkHashL4ModeGet
(
    IN GT_U8   devNum,
    OUT CPSS_DXCH_TRUNK_L4_LBH_MODE_ENT *hashModePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTrunkHashL4ModeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, hashModePtr));

    rc = internal_cpssDxChTrunkHashL4ModeGet(devNum, hashModePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, hashModePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChTrunkHashIpv6ModeSet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         Set the hash generation function for Ipv6 packets.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number.
* @param[in] hashMode                 - the Ipv6 hash mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or hash mode
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTrunkHashIpv6ModeSet
(
    IN GT_U8                                devNum,
    IN CPSS_DXCH_TRUNK_IPV6_HASH_MODE_ENT   hashMode
)
{
    GT_U32      regAddr;         /* register address                         */
    GT_U32      value;           /* value to read from register              */
    GT_U32      startBit;        /* the word's bit at which the field starts */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr =
            PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).trunkHashSimpleHash.trunkHashConfigReg0;
        startBit  = 1;
    }
    else if (PRV_CPSS_DXCH_XCAT_FAMILY_CHECK_MAC(devNum))
    {
        /* xCat and above */
        regAddr =
            PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->ttiRegs.trunkHashCfg[0];
        startBit  = 1;
    }
    else
    {
        /* DXCH1-3 */
        regAddr =
            PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bridgeRegs.l2IngressControl;
        startBit  = 10;
    }

    /* setting <Ipv6HashConfig[1:0]> field data */
    switch(hashMode)
    {
        case CPSS_DXCH_TRUNK_IPV6_HASH_LSB_SIP_DIP_FLOW_E:
            value = 0;
            break;
        case CPSS_DXCH_TRUNK_IPV6_HASH_MSB_SIP_DIP_FLOW_E:
            value = 1;
            break;
        case CPSS_DXCH_TRUNK_IPV6_HASH_MSB_LSB_SIP_DIP_FLOW_E:
            value = 2;
            break;
        case CPSS_DXCH_TRUNK_IPV6_HASH_LSB_SIP_DIP_E:
            value = 3;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    return prvCpssHwPpSetRegField(devNum, regAddr, startBit, 2, value);
}

/**
* @internal cpssDxChTrunkHashIpv6ModeSet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         Set the hash generation function for Ipv6 packets.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number.
* @param[in] hashMode                 - the Ipv6 hash mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or hash mode
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTrunkHashIpv6ModeSet
(
    IN GT_U8                                devNum,
    IN CPSS_DXCH_TRUNK_IPV6_HASH_MODE_ENT   hashMode
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTrunkHashIpv6ModeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, hashMode));

    rc = internal_cpssDxChTrunkHashIpv6ModeSet(devNum, hashMode);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, hashMode));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTrunkHashIpv6ModeGet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         Get the hash generation function for Ipv6 packets.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number.
*
* @param[out] hashModePtr              - (pointer to)the Ipv6 hash mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or hash mode
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTrunkHashIpv6ModeGet
(
    IN GT_U8                                devNum,
    OUT CPSS_DXCH_TRUNK_IPV6_HASH_MODE_ENT   *hashModePtr
)
{
    GT_STATUS   rc;              /* returned code                            */
    GT_U32      regAddr;         /* register address                         */
    GT_U32      value;           /* value to read from register              */
    GT_U32      startBit;        /* the word's bit at which the field starts */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(hashModePtr);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr =
            PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).trunkHashSimpleHash.trunkHashConfigReg0;
        startBit  = 1;
    }
    else if (PRV_CPSS_DXCH_XCAT_FAMILY_CHECK_MAC(devNum))
    {
        /* xCat and above */
        regAddr =
            PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->ttiRegs.trunkHashCfg[0];
        startBit  = 1;
    }
    else
    {
        /* DXCH1-3 */
        regAddr =
            PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bridgeRegs.l2IngressControl;
        startBit  = 10;
    }


    rc = prvCpssHwPpGetRegField(
        devNum, regAddr, startBit, 2, &value);

    switch(value)
    {
        case 0:
            *hashModePtr = CPSS_DXCH_TRUNK_IPV6_HASH_LSB_SIP_DIP_FLOW_E;
            break;
        case 1:
            *hashModePtr =  CPSS_DXCH_TRUNK_IPV6_HASH_MSB_SIP_DIP_FLOW_E;
            break;
        case 2:
            *hashModePtr =  CPSS_DXCH_TRUNK_IPV6_HASH_MSB_LSB_SIP_DIP_FLOW_E;
            break;
        case 3:
        default:
            *hashModePtr =  CPSS_DXCH_TRUNK_IPV6_HASH_LSB_SIP_DIP_E;
            break;
    }

    return rc;
}

/**
* @internal cpssDxChTrunkHashIpv6ModeGet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         Get the hash generation function for Ipv6 packets.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number.
*
* @param[out] hashModePtr              - (pointer to)the Ipv6 hash mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or hash mode
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTrunkHashIpv6ModeGet
(
    IN GT_U8                                devNum,
    OUT CPSS_DXCH_TRUNK_IPV6_HASH_MODE_ENT   *hashModePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTrunkHashIpv6ModeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, hashModePtr));

    rc = internal_cpssDxChTrunkHashIpv6ModeGet(devNum, hashModePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, hashModePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTrunkHashIpAddMacModeSet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         Set the use of mac address bits to trunk hash calculation when packet is
*         IP and the "Ip trunk hash mode enabled".
*         Note:
*         1. Not relevant to NON Ip packets.
*         2. Not relevant to multi-destination packets (include routed IPM).
*         3. Not relevant when cpssDxChTrunkHashGlobalModeSet(devNum,
*         CPSS_DXCH_TRUNK_LBH_INGRESS_PORT_E)
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number.
* @param[in] enable                   - enable/disable feature
*                                      GT_FALSE - If the packet is an IP packet MAC data is not
*                                      added to the Trunk load balancing hash.
*                                      GT_TRUE - The following function is added to the trunk load
*                                      balancing hash:
*                                      MACTrunkHash = MAC_SA[5:0]^MAC_DA[5:0].
*                                      NOTE: When the packet is not an IP packet and
*                                      <TrunkLBH Mode> = 0, the trunk load balancing
*                                      hash = MACTrunkHash, regardless of this setting.
*                                      If the packet is IPv4/6-over-X tunnel-terminated,
*                                      the mode is always GT_FALSE (since there is no
*                                      passenger packet MAC header).
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTrunkHashIpAddMacModeSet
(
    IN GT_U8    devNum,
    IN GT_BOOL  enable
)
{
    GT_U32      regAddr;         /* register address                         */
    GT_U32      value;           /* value to read from register              */
    GT_U32      startBit;        /* the word's bit at which the field starts */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr =
            PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).trunkHashSimpleHash.trunkHashConfigReg0;
        startBit  = 7;
    }
    else if (PRV_CPSS_DXCH_XCAT_FAMILY_CHECK_MAC(devNum))
    {
        /* xCat and above */
        regAddr =
            PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->ttiRegs.trunkHashCfg[0];
        startBit  = 7;
    }
    else
    {
        /* DXCH1-3 */
        regAddr =
            PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bridgeRegs.l2IngressControl;
        startBit  = 2;
    }

    value = (enable == GT_TRUE) ? 1 : 0;

    return prvCpssHwPpSetRegField(devNum, regAddr, startBit, 1, value);
}

/**
* @internal cpssDxChTrunkHashIpAddMacModeSet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         Set the use of mac address bits to trunk hash calculation when packet is
*         IP and the "Ip trunk hash mode enabled".
*         Note:
*         1. Not relevant to NON Ip packets.
*         2. Not relevant to multi-destination packets (include routed IPM).
*         3. Not relevant when cpssDxChTrunkHashGlobalModeSet(devNum,
*         CPSS_DXCH_TRUNK_LBH_INGRESS_PORT_E)
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number.
* @param[in] enable                   - enable/disable feature
*                                      GT_FALSE - If the packet is an IP packet MAC data is not
*                                      added to the Trunk load balancing hash.
*                                      GT_TRUE - The following function is added to the trunk load
*                                      balancing hash:
*                                      MACTrunkHash = MAC_SA[5:0]^MAC_DA[5:0].
*                                      NOTE: When the packet is not an IP packet and
*                                      <TrunkLBH Mode> = 0, the trunk load balancing
*                                      hash = MACTrunkHash, regardless of this setting.
*                                      If the packet is IPv4/6-over-X tunnel-terminated,
*                                      the mode is always GT_FALSE (since there is no
*                                      passenger packet MAC header).
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTrunkHashIpAddMacModeSet
(
    IN GT_U8    devNum,
    IN GT_BOOL  enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTrunkHashIpAddMacModeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enable));

    rc = internal_cpssDxChTrunkHashIpAddMacModeSet(devNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}



/**
* @internal internal_cpssDxChTrunkHashIpAddMacModeGet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         Get the use of mac address bits to trunk hash calculation when packet is
*         IP and the "Ip trunk hash mode enabled".
*         Note:
*         1. Not relevant to NON Ip packets.
*         2. Not relevant to multi-destination packets (include routed IPM).
*         3. Not relevant when cpssDxChTrunkHashGlobalModeSet(devNum,
*         CPSS_DXCH_TRUNK_LBH_INGRESS_PORT_E)
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number.
*
* @param[out] enablePtr                - (pointer to)enable/disable feature
*                                      GT_FALSE - If the packet is an IP packet MAC data is not
*                                      added to the Trunk load balancing hash.
*                                      GT_TRUE - The following function is added to the trunk load
*                                      balancing hash:
*                                      MACTrunkHash = MAC_SA[5:0]^MAC_DA[5:0].
*                                      NOTE: When the packet is not an IP packet and
*                                      <TrunkLBH Mode> = 0, the trunk load balancing
*                                      hash = MACTrunkHash, regardless of this setting.
*                                      If the packet is IPv4/6-over-X tunnel-terminated,
*                                      the mode is always GT_FALSE (since there is no
*                                      passenger packet MAC header).
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTrunkHashIpAddMacModeGet
(
    IN GT_U8    devNum,
    OUT GT_BOOL  *enablePtr
)
{
    GT_STATUS   rc;              /* returned code                            */
    GT_U32      regAddr;         /* register address                         */
    GT_U32      value;           /* value to read from register              */
    GT_U32      startBit;        /* the word's bit at which the field starts */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr =
            PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).trunkHashSimpleHash.trunkHashConfigReg0;
        startBit  = 7;
    }
    else if (PRV_CPSS_DXCH_XCAT_FAMILY_CHECK_MAC(devNum))
    {
        /* xCat and above */
        regAddr =
            PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->ttiRegs.trunkHashCfg[0];
        startBit  = 7;
    }
    else
    {
        /* DXCH1-3 */
        regAddr =
            PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bridgeRegs.l2IngressControl;
        startBit  = 2;
    }

    rc = prvCpssHwPpGetRegField(
        devNum, regAddr, startBit, 1, &value);

    *enablePtr = value ? GT_TRUE : GT_FALSE;

    return rc;
}

/**
* @internal cpssDxChTrunkHashIpAddMacModeGet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         Get the use of mac address bits to trunk hash calculation when packet is
*         IP and the "Ip trunk hash mode enabled".
*         Note:
*         1. Not relevant to NON Ip packets.
*         2. Not relevant to multi-destination packets (include routed IPM).
*         3. Not relevant when cpssDxChTrunkHashGlobalModeSet(devNum,
*         CPSS_DXCH_TRUNK_LBH_INGRESS_PORT_E)
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number.
*
* @param[out] enablePtr                - (pointer to)enable/disable feature
*                                      GT_FALSE - If the packet is an IP packet MAC data is not
*                                      added to the Trunk load balancing hash.
*                                      GT_TRUE - The following function is added to the trunk load
*                                      balancing hash:
*                                      MACTrunkHash = MAC_SA[5:0]^MAC_DA[5:0].
*                                      NOTE: When the packet is not an IP packet and
*                                      <TrunkLBH Mode> = 0, the trunk load balancing
*                                      hash = MACTrunkHash, regardless of this setting.
*                                      If the packet is IPv4/6-over-X tunnel-terminated,
*                                      the mode is always GT_FALSE (since there is no
*                                      passenger packet MAC header).
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTrunkHashIpAddMacModeGet
(
    IN GT_U8    devNum,
    OUT GT_BOOL  *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTrunkHashIpAddMacModeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enablePtr));

    rc = internal_cpssDxChTrunkHashIpAddMacModeGet(devNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal sip5HashNumRightShiftBitsSet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         set the number of bits for 'right shift' <rightShift> on the :
*         memberIndex = (<packet_hash> <numOfMembersInTrunk>) >> <rightShift>.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - the device number.
* @param[in] hashClient               - hash client (see CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_ENT)
* @param[in] rightShiftNumBits        - number of right shift (0..16)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number
* @retval GT_OUT_OF_RANGE          - rightShiftNumBits not in valid range (0..16)
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS sip5HashNumRightShiftBitsSet
(
    IN GT_U8                                devNum,
    IN CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_ENT hashClient,
    IN GT_U32                               rightShiftNumBits
)
{
    GT_U32      regAddr;         /* register address                         */
    GT_U32      startBitOffset;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_CH1_E| CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
          CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);

    if(rightShiftNumBits > 16)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    switch(hashClient)
    {
        case CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_TRUNK_E:
            regAddr = PRV_DXCH_REG1_UNIT_EQ_MAC(devNum).L2ECMP.trunkLFSRConfig;
            startBitOffset = 1;
            break;
        case CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_L2_ECMP_E:
            regAddr = PRV_DXCH_REG1_UNIT_EQ_MAC(devNum).L2ECMP.ePortECMPLFSRConfig;
            startBitOffset = 1;
            break;
        case CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_L3_ECMP_E:
            regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->IPvX.ECMPRoutingConfig.ECMPConfig;
            startBitOffset = 12;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    return prvCpssHwPpSetRegField(devNum, regAddr, startBitOffset, 5, rightShiftNumBits);
}

/**
* @internal sip5HashBitsSelectionSet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         Determines the number of bits used to calculate the Index of the trunk member.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] hashClient               - hash client (see CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_ENT)
* @param[in] startBit                 - start bit (APPLICABLE RANGES: 0..31)
* @param[in] numOfBits                - number of bits (APPLICABLE RANGES: 1..16)
*                                      NOTE: For TRUNK_DESIGNATED_TABLE, IPCL, IOAM, EOAM
* @param[in] numOfBits                must not exceed value 6.
*                                      For Ingress pipe clients (IPCL, IOAM, TRUNK,
*                                      L3ECMP, L2ECMP, TRUNK_DESIGNATED_TABLE)
* @param[in] startBit                 + numOfBits) must not exceed value 32.
*                                      For egress pipe clients (EOAM)
* @param[in] startBit                 + numOfBits) must not exceed value 12.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_OUT_OF_RANGE          - on out of range parameter
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS sip5HashBitsSelectionSet
(
    IN GT_U8                                devNum,
    IN CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_ENT hashClient,
    IN GT_U32                               startBit,
    IN GT_U32                               numOfBits
)
{
    GT_U32    regAddr;         /* register address                         */
    GT_U32    value;
    GT_U32    startBitOffset;
    GT_U32    lastBitOffset;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_CH1_E| CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
          CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);

    if(numOfBits == 0 || startBit >= 32)
    {
        /* check individual parameters */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE,
            "numOfBits == 0 || startBit[%d] >= 32", startBit);
    }

    switch(hashClient)
    {
        case CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_TRUNK_DESIGNATED_TABLE_E:
        case CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_IPCL_E:
        case CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_INGRESS_OAM_E:
        case CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_EGRESS_OAM_E:
            if(numOfBits > 6)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE,
                    "numOfBits[%d]must not exceed 6", numOfBits);
            }
            break;
        case CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_TRUNK_E:
        case CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_L2_ECMP_E:
        case CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_L3_ECMP_E:
            if(numOfBits > 16)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE,
                    "numOfBits[%d]must not exceed 16", numOfBits);
            }
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    switch(hashClient)
    {
        case CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_TRUNK_E:
        case CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_TRUNK_DESIGNATED_TABLE_E:
        case CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_L2_ECMP_E:
        case CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_L3_ECMP_E:
        case CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_IPCL_E:
        case CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_INGRESS_OAM_E:
            if((startBit + numOfBits) > 32)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE,
                    "((startBit[%d] + numOfBits[%d]) = [%d]must not exceed 32",
                    startBit, numOfBits, (startBit  + numOfBits));
            }
            break;
        case CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_EGRESS_OAM_E:
            if((startBit + numOfBits) > 12)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE,
                    "((startBit[%d] + numOfBits[%d]) = [%d]must not exceed 12",
                    startBit, numOfBits, (startBit  + numOfBits));
            }
            break;
        default:
            /* Not reached */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    switch(hashClient)
    {
        case CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_TRUNK_E:
            regAddr = PRV_DXCH_REG1_UNIT_EQ_MAC(devNum).L2ECMP.trunkHashBitSelectionConfig;
            startBitOffset = 0;
            lastBitOffset = 5;
            break;
        case CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_TRUNK_DESIGNATED_TABLE_E:
            regAddr = PRV_DXCH_REG1_UNIT_EQ_MAC(devNum).preEgrEngineGlobalConfig.cscdTrunkHashBitSelectionConfig;
            startBitOffset = 0;
            lastBitOffset = 5;
            break;
        case CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_L2_ECMP_E:
            regAddr = PRV_DXCH_REG1_UNIT_EQ_MAC(devNum).L2ECMP.ePortECMPHashBitSelectionConfig;
            startBitOffset = 0;
            lastBitOffset = 5;
            break;
        case CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_L3_ECMP_E:
            regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->IPvX.ECMPRoutingConfig.ECMPConfig;
            startBitOffset = 1;
            lastBitOffset = 6;
            break;
        case CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_IPCL_E:
            regAddr = PRV_DXCH_REG1_UNIT_PCL_MAC(devNum).policyEngineConfig;
            startBitOffset = 18;
            lastBitOffset = 23;
            break;
        case CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_INGRESS_OAM_E:
            regAddr = PRV_DXCH_REG1_UNIT_OAM_MAC(devNum, 0).hashBitsSelection;
            startBitOffset = 0;
            lastBitOffset = 5;
            break;
        case CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_EGRESS_OAM_E:
            regAddr = PRV_DXCH_REG1_UNIT_OAM_MAC(devNum, 1).hashBitsSelection;
            startBitOffset = 0;
            lastBitOffset = 5;
            break;
        default:
            /* Not reached */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    value = ((startBit << startBitOffset) |                                    /* < Hash Bits First Bit> */
            ((startBit + numOfBits - 1) << lastBitOffset )) >> startBitOffset; /* < Hash Bits Last Bit>  */

    return prvCpssHwPpSetRegField(devNum, regAddr ,startBitOffset ,10 ,value);
}

/**
* @internal prvCpssDxChTrunkHashBitsSelectionGet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         Get the number of bits used to calculate the Index of the trunk member.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] hashClient               - hash client (see CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_ENT)
*
* @param[out] startBitPtr              - (pointer to) start bit
* @param[out] numOfBitsPtr             - (pointer to) number of bits
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChTrunkHashBitsSelectionGet
(
    IN GT_U8                                devNum,
    IN CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_ENT hashClient,
    OUT GT_U32                              *startBitPtr,
    OUT GT_U32                              *numOfBitsPtr
)
{
    GT_STATUS   rc;              /* returned code                            */
    GT_U32      regAddr;         /* register address                         */
    GT_U32    value;
    GT_U32    startBit;
    GT_U32    endBit;
    GT_U32    startBitOffset;
    GT_U32    lastBitOffset;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_CH1_E| CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
          CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);

    CPSS_NULL_PTR_CHECK_MAC(startBitPtr);
    CPSS_NULL_PTR_CHECK_MAC(numOfBitsPtr);

    switch(hashClient)
    {
        case CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_TRUNK_E:
            regAddr = PRV_DXCH_REG1_UNIT_EQ_MAC(devNum).L2ECMP.trunkHashBitSelectionConfig;
            startBitOffset = 0;
            lastBitOffset = 5;
            break;
        case CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_TRUNK_DESIGNATED_TABLE_E:
            regAddr = PRV_DXCH_REG1_UNIT_EQ_MAC(devNum).preEgrEngineGlobalConfig.cscdTrunkHashBitSelectionConfig;
            startBitOffset = 0;
            lastBitOffset = 5;
            break;
        case CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_L2_ECMP_E:
            regAddr = PRV_DXCH_REG1_UNIT_EQ_MAC(devNum).L2ECMP.ePortECMPHashBitSelectionConfig;
            startBitOffset = 0;
            lastBitOffset = 5;
            break;
        case CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_L3_ECMP_E:
            regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->IPvX.ECMPRoutingConfig.ECMPConfig;
            startBitOffset = 1;
            lastBitOffset = 6;
            break;
        case CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_IPCL_E:
            regAddr = PRV_DXCH_REG1_UNIT_PCL_MAC(devNum).policyEngineConfig;
            startBitOffset = 18;
            lastBitOffset = 23;
            break;
        case CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_INGRESS_OAM_E:
            regAddr = PRV_DXCH_REG1_UNIT_OAM_MAC(devNum, 0).hashBitsSelection;
            startBitOffset = 0;
            lastBitOffset = 5;
            break;
        case CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_EGRESS_OAM_E:
            regAddr = PRV_DXCH_REG1_UNIT_OAM_MAC(devNum, 1).hashBitsSelection;
            startBitOffset = 0;
            lastBitOffset = 5;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssHwPpGetRegField(devNum, regAddr, startBitOffset, 10, &value);
    if(rc != GT_OK)
    {
        return rc;
    }
    value = value << startBitOffset;
    startBit = U32_GET_FIELD_MAC(value, startBitOffset, 5);
    endBit   = U32_GET_FIELD_MAC(value, lastBitOffset, 5);

    *startBitPtr  = startBit;
    *numOfBitsPtr = endBit + 1 - startBit;

    return GT_OK;
}

/**
* @internal sip5HashNumBitsSetClient function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         set the number of bits for client.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - the device number.
* @param[in] startBit                 - start bit (APPLICABLE RANGES: 0..31)
* @param[in] numOfBits                - number of bits (APPLICABLE RANGES: 1..16)
*                                      NOTE: For TRUNK_DESIGNATED_TABLE, IPCL, IOAM, EOAM
* @param[in] numOfBits                must not exceed value 6.
*                                      For Ingress pipe clients (IPCL, IOAM, TRUNK,
*                                      L3ECMP, L2ECMP, TRUNK_DESIGNATED_TABLE)
* @param[in] startBit                 + numOfBits) must not exceed value 32.
*                                      For egress pipe clients (EOAM)
* @param[in] startBit                 + numOfBits) must not exceed value 12.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number
* @retval GT_OUT_OF_RANGE          - numOfBits not in valid range (1..16)
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS sip5HashNumBitsSetClient
(
    IN GT_U8    devNum,
    IN GT_U32   startBit,
    IN GT_U32   numOfBits
)
{
    GT_STATUS rc;

    rc = cpssDxChTrunkHashNumBitsSet(devNum,CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_TRUNK_E  ,startBit,numOfBits);
    if(rc != GT_OK)
    {
        return rc;
    }

    rc = cpssDxChTrunkHashNumBitsSet(devNum,CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_L2_ECMP_E,startBit,numOfBits);
    return rc;
}



/**
* @internal internal_cpssDxChTrunkHashGlobalModeSet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         Set the general hashing mode of trunk hash generation.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number.
* @param[in] hashMode                 - hash mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or hash mode
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTrunkHashGlobalModeSet
(
    IN GT_U8    devNum,
    IN CPSS_DXCH_TRUNK_LBH_GLOBAL_MODE_ENT  hashMode
)
{
    GT_STATUS   rc;              /* returned code                            */
    GT_U32      regAddr;         /* register address                         */
    GT_U32      value;           /* value to write to register 1             */
    GT_U32      value1;          /* value to write to register 2             */
    GT_U32      startBit;        /* the word's bit at which the field starts */
    GT_U32      numRshiftBits;   /* number of bits for 'Right shift'         */
    GT_U32      hashFunctionSelection;/* when CRC mode : is CRC_32 or CRC_6/CRC_16 modes */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    switch(hashMode)
    {
        case CPSS_DXCH_TRUNK_LBH_PACKETS_INFO_E:
            value = 0;
            value1 = 0;/* disable CRC hash mode - relevant to trunkCrcHashSupported */
            break;
        case CPSS_DXCH_TRUNK_LBH_INGRESS_PORT_E:
            value = 1;
            value1 = 0;/* disable CRC hash mode - relevant to trunkCrcHashSupported */
            break;
        case CPSS_DXCH_TRUNK_LBH_PACKETS_INFO_CRC_E:
            if(PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.featureInfo.trunkCrcHashSupported == GT_FALSE)
            {
                /* the device not support the CRC hash feature */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            value = 0;/* don't care */
            value1 = 1;/* enable CRC hash mode */
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr =
            PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).trunkHashSimpleHash.trunkHashConfigReg0;
        startBit  = 3;
    }
    else if (PRV_CPSS_DXCH_XCAT_FAMILY_CHECK_MAC(devNum))
    {
        /* xCat and above */
        regAddr =
            PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->ttiRegs.trunkHashCfg[0];
        startBit  = 3;
    }
    else
    {
        /* DXCH1-3 */
        regAddr =
            PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bridgeRegs.l2IngressControl;
        startBit  = 9;
    }

    rc =  prvCpssHwPpSetRegField(devNum, regAddr, startBit, 1, value);
    if(rc != GT_OK)
    {
        return rc;
    }

    if(PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.featureInfo.trunkCrcHashSupported == GT_TRUE)
    {
        if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
        {
            regAddr =
                PRV_DXCH_REG1_UNIT_PCL_MAC(devNum).HASH.CRCHashConfig;

            /*need to set the number of bits to use*/
            /* this is 'basic hash' mode , and for CRC 6/16 */
            numRshiftBits = SIP5_LFSR_LEGACY_6_BITS_CNS; /* 6 bits of hash - that comes from the TTI/PCL unit */
            if(value1)/*set CRC mode*/
            {
                /*check if CRC_32 mode is set in the register ... */
                /*<Hash Function Selection>*/
                rc =  prvCpssHwPpGetRegField(devNum, regAddr, 3, 1, &hashFunctionSelection);
                if(hashFunctionSelection)
                {
                    /* using CRC_32 mode */
                    numRshiftBits = SIP5_LFSR_MAX_12_BITS_CNS;
                }
            }

            /* set the number of right shift bits needed */
            /* this configuration may be changed by:
               cpssDxChTrunkHashNumBitsSet
               cpssDxChBrgL2EcmpMemberSelectionModeSet
               cpssDxChTrunkMemberSelectionModeSet
               cpssDxChTrunkInit
               cpssDxChTrunkHashCrcParametersSet
            */
            rc = sip5HashNumBitsSetClient(devNum, 0 , numRshiftBits);
            if(rc != GT_OK)
            {
                return rc;
            }
        }
        else
        {
            regAddr =
                PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->pclRegs.crcHashConfigurationReg;
        }

        rc =  prvCpssHwPpSetRegField(devNum, regAddr, 0, 1, value1);
        if(rc != GT_OK)
        {
            return rc;
        }
    }


    return GT_OK;
}

/**
* @internal cpssDxChTrunkHashGlobalModeSet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         Set the general hashing mode of trunk hash generation.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number.
* @param[in] hashMode                 - hash mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or hash mode
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTrunkHashGlobalModeSet
(
    IN GT_U8    devNum,
    IN CPSS_DXCH_TRUNK_LBH_GLOBAL_MODE_ENT  hashMode
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTrunkHashGlobalModeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, hashMode));

    rc = internal_cpssDxChTrunkHashGlobalModeSet(devNum, hashMode);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, hashMode));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTrunkHashGlobalModeGet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         Get the general hashing mode of trunk hash generation.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number.
*
* @param[out] hashModePtr              - (pointer to)hash mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTrunkHashGlobalModeGet
(
    IN GT_U8    devNum,
    OUT CPSS_DXCH_TRUNK_LBH_GLOBAL_MODE_ENT  *hashModePtr
)
{
    GT_STATUS   rc;              /* returned code                            */
    GT_U32      regAddr;         /* register address                         */
    GT_U32      value;           /* value to read from register              */
    GT_U32      startBit;        /* the word's bit at which the field starts */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(hashModePtr);

    if(PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.featureInfo.trunkCrcHashSupported == GT_TRUE)
    {
        if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
        {
            regAddr =
                PRV_DXCH_REG1_UNIT_PCL_MAC(devNum).HASH.CRCHashConfig;
        }
        else
        {
            regAddr =
                PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->pclRegs.crcHashConfigurationReg;
        }

        rc =  prvCpssHwPpGetRegField(devNum, regAddr, 0, 1, &value);
        if(rc != GT_OK)
        {
            return rc;
        }

        if(value == 1)
        {
            /* using the CRC hash mode */
            *hashModePtr = CPSS_DXCH_TRUNK_LBH_PACKETS_INFO_CRC_E;
            return GT_OK;
        }

        /* NOT using the CRC hash mode - so check the other register ... */
    }

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr =
            PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).trunkHashSimpleHash.trunkHashConfigReg0;
        startBit  = 3;
    }
    else if (PRV_CPSS_DXCH_XCAT_FAMILY_CHECK_MAC(devNum))
    {
        /* xCat and above */
        regAddr =
            PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->ttiRegs.trunkHashCfg[0];
        startBit  = 3;
    }
    else
    {
        /* DXCH1-3 */
        regAddr =
            PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bridgeRegs.l2IngressControl;
        startBit  = 9;
    }

    rc = prvCpssHwPpGetRegField(
        devNum, regAddr, startBit, 1, &value);

    *hashModePtr =
        value
            ? CPSS_DXCH_TRUNK_LBH_INGRESS_PORT_E
            : CPSS_DXCH_TRUNK_LBH_PACKETS_INFO_E;

    return rc;
}

/**
* @internal cpssDxChTrunkHashGlobalModeGet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         Get the general hashing mode of trunk hash generation.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number.
*
* @param[out] hashModePtr              - (pointer to)hash mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTrunkHashGlobalModeGet
(
    IN GT_U8    devNum,
    OUT CPSS_DXCH_TRUNK_LBH_GLOBAL_MODE_ENT  *hashModePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTrunkHashGlobalModeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, hashModePtr));

    rc = internal_cpssDxChTrunkHashGlobalModeGet(devNum, hashModePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, hashModePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTrunkHashDesignatedTableModeSet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         Set the hash mode of the designated trunk table .
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number.
* @param[in] mode                     - The designated table hashing mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number or mode
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTrunkHashDesignatedTableModeSet
(
    IN GT_U8   devNum,
    IN CPSS_DXCH_TRUNK_DESIGNATED_TABLE_MODE_ENT    mode
)
{
    GT_STATUS   rc;      /* return code */
    GT_U32      regAddr; /*register addresses */
    GT_U32      value2,value3,value4; /* value of registers */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    switch(mode)
    {
        case CPSS_DXCH_TRUNK_DESIGNATED_TABLE_USE_INGRESS_HASH_E:
            value2 = 0;
            value3 = 0;
            value4 = 0;
            break;
        case CPSS_DXCH_TRUNK_DESIGNATED_TABLE_USE_INGRESS_HASH_AND_VID_E:
            value2 = 1;
            value3 = 2;
            value4 = 0;
            break;
        case CPSS_DXCH_TRUNK_DESIGNATED_TABLE_USE_SOURCE_INFO_E:
            if(0 == PRV_CPSS_DXCH_LION_FAMILY_CHECK_MAC(devNum))
            {
                /* those devices not support this option */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            value2 = 0;/* not used for Lion, used for xCat2 */
            value3 = 3;/* not used for xCat2, used for Lion */
            value4 = 1;/* not used for Lion, used for xCat2 */
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_EGF_EFT_MAC(devNum).egrFilterConfigs.egrFiltersGlobal;

        return prvCpssHwPpSetRegField(devNum, regAddr, 2, 2, value3);
    }
    else
    if(PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(devNum))
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->txqVer1.egr.filterConfig.globalEnables;

        return prvCpssHwPpSetRegField(devNum, regAddr, 5, 2, value3);
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->egrTxQConf.txQueueConfigExt;
        rc = prvCpssHwPpSetRegField(devNum, regAddr, 24, 1, value2);
        if(rc != GT_OK)
        {
            return rc;
        }

        /* xCat2 and above - configure McTrunkHashSrcInfoEn bit */
        if(PRV_CPSS_DXCH_XCAT2_FAMILY_CHECK_MAC(devNum))
        {
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->egrTxQConf.txQueueMiscCtrl;
            rc = prvCpssHwPpSetRegField(devNum, regAddr, 8, 1, value4);
            if(rc != GT_OK)
            {
                return rc;
            }
        }
        return GT_OK;
    }
}

/**
* @internal cpssDxChTrunkHashDesignatedTableModeSet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         Set the hash mode of the designated trunk table .
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number.
* @param[in] mode                     - The designated table hashing mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number or mode
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTrunkHashDesignatedTableModeSet
(
    IN GT_U8   devNum,
    IN CPSS_DXCH_TRUNK_DESIGNATED_TABLE_MODE_ENT    mode
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTrunkHashDesignatedTableModeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, mode));

    rc = internal_cpssDxChTrunkHashDesignatedTableModeSet(devNum, mode);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, mode));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTrunkHashDesignatedTableModeGet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         Get the hash mode of the designated trunk table .
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number.
*
* @param[out] modePtr                  - (pointer to) The designated table hashing mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTrunkHashDesignatedTableModeGet
(
    IN GT_U8   devNum,
    OUT CPSS_DXCH_TRUNK_DESIGNATED_TABLE_MODE_ENT *modePtr
)
{
    GT_STATUS   rc;      /* return code */
    GT_U32      regAddr; /*register addresses */
    GT_U32      value2,value3,value4; /* value of registers */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(modePtr);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_EGF_EFT_MAC(devNum).egrFilterConfigs.egrFiltersGlobal;

        rc = prvCpssHwPpGetRegField(devNum, regAddr, 2, 2, &value3);
    }
    else
    if(PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(devNum))
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->txqVer1.egr.filterConfig.globalEnables;
        rc = prvCpssHwPpGetRegField(devNum, regAddr, 5, 2, &value3);
    }
    else
    {
        value4 = 0;
        /* xCat2 and above - check McTrunkHashSrcInfoEn bit */
        if(PRV_CPSS_DXCH_XCAT2_FAMILY_CHECK_MAC(devNum))
        {
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->egrTxQConf.txQueueMiscCtrl;
            rc = prvCpssHwPpGetRegField(devNum, regAddr, 8, 1, &value4);
            if(rc != GT_OK)
            {
                return rc;
            }
        }

        if( 1 == value4 )
        {
            value3 = 3;
        }
        else
        {
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->egrTxQConf.txQueueConfigExt;
            rc =  prvCpssHwPpGetRegField(devNum, regAddr, 24, 1, &value2);
            if(rc != GT_OK)
            {
                return rc;
            }

            if( 0 == value2 )
            {
                value3 = 0;
            }
            else
            {
                value3 = 2;
            }
        }
    }

    switch(value3)
    {
        case 0:
            *modePtr = CPSS_DXCH_TRUNK_DESIGNATED_TABLE_USE_INGRESS_HASH_E;
            break;
        case 2:
            *modePtr = CPSS_DXCH_TRUNK_DESIGNATED_TABLE_USE_INGRESS_HASH_AND_VID_E;
            break;
        case 3:
        default:
            *modePtr = CPSS_DXCH_TRUNK_DESIGNATED_TABLE_USE_SOURCE_INFO_E;
            break;
    }

    return GT_OK;
}

/**
* @internal cpssDxChTrunkHashDesignatedTableModeGet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         Get the hash mode of the designated trunk table .
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number.
*
* @param[out] modePtr                  - (pointer to) The designated table hashing mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTrunkHashDesignatedTableModeGet
(
    IN GT_U8   devNum,
    OUT CPSS_DXCH_TRUNK_DESIGNATED_TABLE_MODE_ENT *modePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTrunkHashDesignatedTableModeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, modePtr));

    rc = internal_cpssDxChTrunkHashDesignatedTableModeGet(devNum, modePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, modePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTrunkHashNumBitsSet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes.
*         Set the number of bits for client.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - the device number.
* @param[in] hashClient               - hash client (see CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_ENT)
* @param[in] startBit                 - start bit (APPLICABLE RANGES: 0..31)
* @param[in] numOfBits                - number of bits (APPLICABLE RANGES: 1..16)
*                                      NOTE: For TRUNK_DESIGNATED_TABLE, IPCL, IOAM, EOAM
* @param[in] numOfBits                must not exceed value 6.
*                                      For Ingress pipe clients (IPCL, IOAM, TRUNK,
*                                      L3ECMP, L2ECMP, TRUNK_DESIGNATED_TABLE)
* @param[in] startBit                 + numOfBits) must not exceed value 32.
*                                      For egress pipe clients (EOAM)
* @param[in] startBit                 + numOfBits) must not exceed value 12.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number
* @retval GT_OUT_OF_RANGE          - startBit or numOfBits not in valid range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note startBit and numOfBits values may also be changed by:
*       - cpssDxChBrgL2EcmpMemberSelectionModeSet for L2 ECMP client to (0 6),
*       (6 6), (0 12) depending on member selection mode.
*       - cpssDxChIpEcmpHashNumBitsSet for L3 ECMP client to any value passed
*       to the function.
*       - cpssDxChTrunkMemberSelectionModeSet for Trunk ECMP client to (0 6),
*       (6 6), (0 12) depending on member selection mode.
*       - cpssDxChTrunkHashGlobalModeSet for Trunk ECMP and L2 ECMP clients to
*       (0, 6) or if CRC_32_MODE then use (0,12).
*       - cpssDxChTrunkInit for Trunk ECMP and L2 ECMP clients to (0, 6).
*       - cpssDxChTrunkHashCrcParametersSet for Trunk ECMP and L2 ECMP clients
*       to (0, 6) or if CRC_32_MODE then use (0,12).
*
*/
static GT_STATUS internal_cpssDxChTrunkHashNumBitsSet
(
    IN GT_U8                                devNum,
    IN CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_ENT hashClient,
    IN GT_U32                               startBit,
    IN GT_U32                               numOfBits
)
{
    GT_STATUS rc;
    GT_U32   origStartBit;
    GT_U32   origNumOfBits;
    GT_BOOL  needToSetRshift;

    /* NOTE: LFSR must be set before 'used bits' when LFSR changed to new higher
        value. because:
        if set 'used bits' first to new higher value ... the device may send
        traffic to members outside of the actual members !!! ERROR !!! ERROR !!!

        but if set LFSR first to new higher value ... the device will select
        member from subset of the existing members ... but WILL NOT drop and
        will not send unexpected port
    */

    /* NOTE: 'used bits' must be set before LFSR when LFSR changed to new lower
        value. because: similar logic to the above explanation .
    */

    switch(hashClient)
    {
        case CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_TRUNK_E:
        case CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_L2_ECMP_E:
        case CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_L3_ECMP_E:
            needToSetRshift = GT_TRUE;
            break;
        case CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_TRUNK_DESIGNATED_TABLE_E:
        case CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_IPCL_E:
        case CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_INGRESS_OAM_E:
        case CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_EGRESS_OAM_E:
            needToSetRshift = GT_FALSE;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* get trunk used bits */
    rc = prvCpssDxChTrunkHashBitsSelectionGet(devNum, hashClient, &origStartBit,
        &origNumOfBits);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* LFSR must be set before 'used bits' when LFSR changed to new higher
       value */
    if(numOfBits >= origNumOfBits)
    {
        /* set trunk LFSR */
        if(GT_TRUE == needToSetRshift)
        {
            rc = sip5HashNumRightShiftBitsSet(devNum, hashClient, numOfBits);
            if(rc != GT_OK)
            {
                return rc;
            }
        }

        /* set trunk used bits */
        rc = sip5HashBitsSelectionSet(devNum, hashClient, startBit, numOfBits);
        if(rc != GT_OK)
        {
            return rc;
        }
    }
    else /*'used bits' must be set before LFSR when LFSR changed to new lower
           value.*/
    {
        /* set client used bits */
        rc = sip5HashBitsSelectionSet(devNum, hashClient, startBit, numOfBits);
        if(rc != GT_OK)
        {
            return rc;
        }

        /* set client LFSR */
        if(GT_TRUE == needToSetRshift)
        {
            rc = sip5HashNumRightShiftBitsSet(devNum, hashClient, numOfBits);
            if(rc != GT_OK)
            {
                return rc;
            }
        }
    }

    return GT_OK;
}

/**
* @internal cpssDxChTrunkHashNumBitsSet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes.
*         Set the number of bits for client.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - the device number.
* @param[in] hashClient               - hash client (see CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_ENT)
* @param[in] startBit                 - start bit (APPLICABLE RANGES: 0..31)
* @param[in] numOfBits                - number of bits (APPLICABLE RANGES: 1..16)
*                                      NOTE: For TRUNK_DESIGNATED_TABLE, IPCL, IOAM, EOAM
* @param[in] numOfBits                must not exceed value 6.
*                                      For Ingress pipe clients (IPCL, IOAM, TRUNK,
*                                      L3ECMP, L2ECMP, TRUNK_DESIGNATED_TABLE)
* @param[in] startBit                 + numOfBits) must not exceed value 32.
*                                      For egress pipe clients (EOAM)
* @param[in] startBit                 + numOfBits) must not exceed value 12.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number
* @retval GT_OUT_OF_RANGE          - startBit or numOfBits not in valid range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note startBit and numOfBits values may also be changed by:
*       - cpssDxChBrgL2EcmpMemberSelectionModeSet for L2 ECMP client to (0 6),
*       (6 6), (0 12) depending on member selection mode.
*       - cpssDxChIpEcmpHashNumBitsSet for L3 ECMP client to any value passed
*       to the function.
*       - cpssDxChTrunkMemberSelectionModeSet for Trunk ECMP client to (0 6),
*       (6 6), (0 12) depending on member selection mode.
*       - cpssDxChTrunkHashGlobalModeSet for Trunk ECMP and L2 ECMP clients to
*       (0, 6) or if CRC_32_MODE then use (0,12).
*       - cpssDxChTrunkInit for Trunk ECMP and L2 ECMP clients to (0, 6).
*       - cpssDxChTrunkHashCrcParametersSet for Trunk ECMP and L2 ECMP clients
*       to (0, 6) or if CRC_32_MODE then use (0,12).
*
*/
GT_STATUS cpssDxChTrunkHashNumBitsSet
(
    IN GT_U8                                devNum,
    IN CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_ENT hashClient,
    IN GT_U32                               startBit,
    IN GT_U32                               numOfBits
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTrunkHashNumBitsSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, hashClient, startBit, numOfBits));

    rc = internal_cpssDxChTrunkHashNumBitsSet(devNum, hashClient, startBit, numOfBits);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, hashClient, startBit, numOfBits));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTrunkHashNumBitsGet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes.
*         Get the number of bits for client.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - the device number.
* @param[in] hashClient               - hash client (see CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_ENT)
* @param[in] startBitPtr              - (pointer to) start bit
* @param[in] numOfBitsPtr             - (pointer to) number of bits
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - NULL pointer to startBitPtr or numOfBitsPtr
* @retval GT_BAD_PARAM             - bad device number
* @retval GT_OUT_OF_RANGE          - startBit or numOfBits not in valid range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTrunkHashNumBitsGet
(
    IN  GT_U8                                devNum,
    IN  CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_ENT hashClient,
    OUT GT_U32                               *startBitPtr,
    OUT GT_U32                               *numOfBitsPtr
)
{
    return prvCpssDxChTrunkHashBitsSelectionGet(devNum, hashClient, startBitPtr,
        numOfBitsPtr);
}

/**
* @internal cpssDxChTrunkHashNumBitsGet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes.
*         Get the number of bits for client.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - the device number.
* @param[in] hashClient               - hash client (see CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_ENT)
* @param[in] startBitPtr              - (pointer to) start bit
* @param[in] numOfBitsPtr             - (pointer to) number of bits
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - NULL pointer to startBitPtr or numOfBitsPtr
* @retval GT_BAD_PARAM             - bad device number
* @retval GT_OUT_OF_RANGE          - startBit or numOfBits not in valid range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTrunkHashNumBitsGet
(
    IN  GT_U8                                devNum,
    IN  CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_ENT hashClient,
    OUT GT_U32                               *startBitPtr,
    OUT GT_U32                               *numOfBitsPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTrunkHashNumBitsGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, hashClient, startBitPtr, numOfBitsPtr));

    rc = internal_cpssDxChTrunkHashNumBitsGet(devNum, hashClient, startBitPtr, numOfBitsPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, hashClient, startBitPtr, numOfBitsPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTrunkDbEnabledMembersGet function
* @endinternal
*
* @brief   Function Relevant mode : High level mode
*         return the enabled members of the trunk
*         function uses the DB (no HW operations)
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number .
* @param[in] trunkId                  - the trunk id.
* @param[in,out] numOfEnabledMembersPtr   - (pointer to) max num of enabled members to
*                                      retrieve - this value refer to the number of
*                                      members that the array of enabledMembersArray[]
*                                      can retrieve.
* @param[in,out] numOfEnabledMembersPtr   - (pointer to) the actual num of enabled members
*                                      in the trunk (up to CPSS_TRUNK_MAX_NUM_OF_MEMBERS_CNS)
*
* @param[out] enabledMembersArray[]    - (array of) enabled members of the trunk
*                                      array was allocated by the caller
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_INITIALIZED       -the trunk library was not initialized for the device
* @retval GT_BAD_PARAM             - bad device number , or
*                                       bad trunkId number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_BAD_STATE             - the trunk-type is not one of: CPSS_TRUNK_TYPE_FREE_E or CPSS_TRUNK_TYPE_REGULAR_E
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTrunkDbEnabledMembersGet
(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              trunkId,
    INOUT GT_U32                *numOfEnabledMembersPtr,
    OUT CPSS_TRUNK_MEMBER_STC   enabledMembersArray[]
)
{
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    return prvCpssGenericTrunkDbEnabledMembersGet(devNum,trunkId,
                numOfEnabledMembersPtr,enabledMembersArray);
}

/**
* @internal cpssDxChTrunkDbEnabledMembersGet function
* @endinternal
*
* @brief   Function Relevant mode : High level mode
*         return the enabled members of the trunk
*         function uses the DB (no HW operations)
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number .
* @param[in] trunkId                  - the trunk id.
* @param[in,out] numOfEnabledMembersPtr   - (pointer to) max num of enabled members to
*                                      retrieve - this value refer to the number of
*                                      members that the array of enabledMembersArray[]
*                                      can retrieve.
* @param[in,out] numOfEnabledMembersPtr   - (pointer to) the actual num of enabled members
*                                      in the trunk (up to CPSS_TRUNK_MAX_NUM_OF_MEMBERS_CNS)
*
* @param[out] enabledMembersArray[]    - (array of) enabled members of the trunk
*                                      array was allocated by the caller
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_INITIALIZED       -the trunk library was not initialized for the device
* @retval GT_BAD_PARAM             - bad device number , or
*                                       bad trunkId number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_BAD_STATE             - the trunk-type is not one of: CPSS_TRUNK_TYPE_FREE_E or CPSS_TRUNK_TYPE_REGULAR_E
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTrunkDbEnabledMembersGet
(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              trunkId,
    INOUT GT_U32                *numOfEnabledMembersPtr,
    OUT CPSS_TRUNK_MEMBER_STC   enabledMembersArray[]
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTrunkDbEnabledMembersGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, trunkId, numOfEnabledMembersPtr, enabledMembersArray));

    rc = internal_cpssDxChTrunkDbEnabledMembersGet(devNum, trunkId, numOfEnabledMembersPtr, enabledMembersArray);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, trunkId, numOfEnabledMembersPtr, enabledMembersArray));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTrunkDbDisabledMembersGet function
* @endinternal
*
* @brief   Function Relevant mode : High level mode
*         return the disabled members of the trunk.
*         function uses the DB (no HW operations)
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number .
* @param[in] trunkId                  - the trunk id.
* @param[in,out] numOfDisabledMembersPtr  - (pointer to) max num of disabled members to
*                                      retrieve - this value refer to the number of
*                                      members that the array of enabledMembersArray[]
*                                      can retrieve.
* @param[in,out] numOfDisabledMembersPtr  -(pointer to) the actual num of disabled members
*                                      in the trunk (up to CPSS_TRUNK_MAX_NUM_OF_MEMBERS_CNS)
*
* @param[out] disabledMembersArray[]   - (array of) disabled members of the trunk
*                                      array was allocated by the caller
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_INITIALIZED       -the trunk library was not initialized for the device
* @retval GT_BAD_PARAM             - bad device number , or
*                                       bad trunkId number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_BAD_STATE             - the trunk-type is not one of: CPSS_TRUNK_TYPE_FREE_E or CPSS_TRUNK_TYPE_REGULAR_E
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTrunkDbDisabledMembersGet
(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              trunkId,
    INOUT GT_U32                *numOfDisabledMembersPtr,
    OUT CPSS_TRUNK_MEMBER_STC   disabledMembersArray[]
)
{
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    return prvCpssGenericTrunkDbDisabledMembersGet(devNum,trunkId,
                numOfDisabledMembersPtr,disabledMembersArray);
}

/**
* @internal cpssDxChTrunkDbDisabledMembersGet function
* @endinternal
*
* @brief   Function Relevant mode : High level mode
*         return the disabled members of the trunk.
*         function uses the DB (no HW operations)
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number .
* @param[in] trunkId                  - the trunk id.
* @param[in,out] numOfDisabledMembersPtr  - (pointer to) max num of disabled members to
*                                      retrieve - this value refer to the number of
*                                      members that the array of enabledMembersArray[]
*                                      can retrieve.
* @param[in,out] numOfDisabledMembersPtr  -(pointer to) the actual num of disabled members
*                                      in the trunk (up to CPSS_TRUNK_MAX_NUM_OF_MEMBERS_CNS)
*
* @param[out] disabledMembersArray[]   - (array of) disabled members of the trunk
*                                      array was allocated by the caller
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_INITIALIZED       -the trunk library was not initialized for the device
* @retval GT_BAD_PARAM             - bad device number , or
*                                       bad trunkId number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_BAD_STATE             - the trunk-type is not one of: CPSS_TRUNK_TYPE_FREE_E or CPSS_TRUNK_TYPE_REGULAR_E
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTrunkDbDisabledMembersGet
(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              trunkId,
    INOUT GT_U32                *numOfDisabledMembersPtr,
    OUT CPSS_TRUNK_MEMBER_STC   disabledMembersArray[]
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTrunkDbDisabledMembersGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, trunkId, numOfDisabledMembersPtr, disabledMembersArray));

    rc = internal_cpssDxChTrunkDbDisabledMembersGet(devNum, trunkId, numOfDisabledMembersPtr, disabledMembersArray);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, trunkId, numOfDisabledMembersPtr, disabledMembersArray));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTrunkDbIsMemberOfTrunk function
* @endinternal
*
* @brief   Function Relevant mode : High level mode
*         Checks if a member (device,port) is a trunk member.
*         if it is trunk member the function retrieve the trunkId of the trunk.
*         function uses the DB (no HW operations)
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number.
* @param[in] memberPtr                - (pointer to) the member to check if is trunk member
*
* @param[out] trunkIdPtr               - (pointer to) trunk id of the port .
*                                      this pointer allocated by the caller.
*                                      this can be NULL pointer if the caller not require the
*                                      trunkId(only wanted to know that the member belongs to a
*                                      trunk)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_INITIALIZED       -the trunk library was not initialized for the device
* @retval GT_BAD_PARAM             - bad device number
* @retval GT_NOT_FOUND             - the pair (devNum,port) not a trunk member
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTrunkDbIsMemberOfTrunk
(
    IN  GT_U8                   devNum,
    IN  CPSS_TRUNK_MEMBER_STC   *memberPtr,
    OUT GT_TRUNK_ID             *trunkIdPtr
)
{
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    return prvCpssGenericTrunkDbIsMemberOfTrunk(devNum,memberPtr,trunkIdPtr);
}

/**
* @internal cpssDxChTrunkDbIsMemberOfTrunk function
* @endinternal
*
* @brief   Function Relevant mode : High level mode
*         Checks if a member (device,port) is a trunk member.
*         if it is trunk member the function retrieve the trunkId of the trunk.
*         function uses the DB (no HW operations)
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number.
* @param[in] memberPtr                - (pointer to) the member to check if is trunk member
*
* @param[out] trunkIdPtr               - (pointer to) trunk id of the port .
*                                      this pointer allocated by the caller.
*                                      this can be NULL pointer if the caller not require the
*                                      trunkId(only wanted to know that the member belongs to a
*                                      trunk)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_INITIALIZED       -the trunk library was not initialized for the device
* @retval GT_BAD_PARAM             - bad device number
* @retval GT_NOT_FOUND             - the pair (devNum,port) not a trunk member
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTrunkDbIsMemberOfTrunk
(
    IN  GT_U8                   devNum,
    IN  CPSS_TRUNK_MEMBER_STC   *memberPtr,
    OUT GT_TRUNK_ID             *trunkIdPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTrunkDbIsMemberOfTrunk);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, memberPtr, trunkIdPtr));

    rc = internal_cpssDxChTrunkDbIsMemberOfTrunk(devNum, memberPtr, trunkIdPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, memberPtr, trunkIdPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTrunkDbTrunkTypeGet function
* @endinternal
*
* @brief   Function Relevant mode : High level mode
*         Get the trunk type.
*         function uses the DB (no HW operations)
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number.
* @param[in] trunkId                  - the trunk id.
*
* @param[out] typePtr                  - (pointer to) the trunk type
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_INITIALIZED       -the trunk library was not initialized for the device
* @retval GT_BAD_PARAM             - bad device number , or bad trunkId number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTrunkDbTrunkTypeGet
(
    IN  GT_U8                   devNum,
    IN  GT_TRUNK_ID             trunkId,
    OUT CPSS_TRUNK_TYPE_ENT     *typePtr
)
{
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    return prvCpssGenericTrunkDbTrunkTypeGet(devNum,trunkId,typePtr);
}

/**
* @internal cpssDxChTrunkDbTrunkTypeGet function
* @endinternal
*
* @brief   Function Relevant mode : High level mode
*         Get the trunk type.
*         function uses the DB (no HW operations)
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number.
* @param[in] trunkId                  - the trunk id.
*
* @param[out] typePtr                  - (pointer to) the trunk type
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_INITIALIZED       -the trunk library was not initialized for the device
* @retval GT_BAD_PARAM             - bad device number , or bad trunkId number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTrunkDbTrunkTypeGet
(
    IN  GT_U8                   devNum,
    IN  GT_TRUNK_ID             trunkId,
    OUT CPSS_TRUNK_TYPE_ENT     *typePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTrunkDbTrunkTypeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, trunkId, typePtr));

    rc = internal_cpssDxChTrunkDbTrunkTypeGet(devNum, trunkId, typePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, trunkId, typePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChTrunkDesignatedMemberSet function
* @endinternal
*
* @brief   Function Relevant mode : High Level mode
*         This function Configures per-trunk the designated member -
*         value should be stored (to DB) even designated member is not currently
*         a member of Trunk.
*         Setting value replace previously assigned designated member.
*         NOTE that:
*         under normal operation this should not be used with cascade Trunks,
*         due to the fact that in this case it affects all types of traffic -
*         not just Multi-destination as in regular Trunks.
*         Diagram 1 : Regular operation - Traffic distribution on all enabled
*         members (when no specific designated member defined)
*         %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*         index \ member %  M1 %  M2  %  M3 % M4 %
*         %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*         Index 0    %  1  %  0  %  0  % 0  %
*         %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*         Index 1    %  0  %  1  %  0  % 0  %
*         %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*         Index 2    %  0  %  0  %  1  % 0  %
*         %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*         Index 3    %  0  %  0  %  0  % 1  %
*         %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*         Index 4    %  1  %  0  %  0  % 0  %
*         %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*         Index 5    %  0  %  1  %  0  % 0  %
*         %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*         Index 6    %  0  %  0  %  1  % 0  %
*         %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*         Index 7    %  0  %  0  %  0  % 1  %
*         %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*         Diagram 2: Traffic distribution once specific designated member defined
*         (M3 in this case - which is currently enabled member in trunk)
*         %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*         index \ member %  M1 %  M2  %  M3 % M4 %
*         %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*         Index 0    %  0  %  0  %  1  % 0  %
*         %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*         Index 1    %  0  %  0  %  1  % 0  %
*         %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*         Index 2    %  0  %  0  %  1  % 0  %
*         %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*         Index 3    %  0  %  0  %  1  % 0  %
*         %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*         Index 4    %  0  %  0  %  1  % 0  %
*         %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*         Index 5    %  0  %  0  %  1  % 0  %
*         %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*         Index 6    %  0  %  0  %  1  % 0  %
*         %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*         Index 7    %  0  %  0  %  1  % 0  %
*         %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] trunkId                  - the trunk id.
* @param[in] enable                   - enable/disable designated trunk member.
*                                      GT_TRUE -
*                                      1. Clears all current Trunk member's designated bits
*                                      2. If input designated member, is currently an
* @param[in] enable                   at Trunk (in local device) enable its
*                                      bits on all indexes
*                                      3. Store designated member at the DB (new DB parameter
*                                      should be created for that)
*                                      GT_FALSE -
*                                      1. Redistribute current Trunk members bits (on all enabled members)
*                                      2. Clear designated member at  the DB
* @param[in] memberPtr                - (pointer to)the designated member we set to the trunk.
*                                      relevant only when enable = GT_TRUE
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_INITIALIZED       -the trunk library was not initialized for the device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or
*                                       bad trunkId number , or
*                                       bad member parameters :
*                                       (device & 0xE0) != 0 means that the HW can't support
*                                       this value , since HW has 5 bit
*                                       for device number
*                                       (port & 0xC0) != 0 means that the HW can't support
*                                       this value , since HW has 6 bit
*                                       for port number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_ALREADY_EXIST         - this member already exists in another trunk.
* @retval GT_BAD_STATE             - the trunk-type is not one of: CPSS_TRUNK_TYPE_FREE_E or CPSS_TRUNK_TYPE_REGULAR_E
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTrunkDesignatedMemberSet
(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              trunkId,
    IN GT_BOOL                  enable,
    IN CPSS_TRUNK_MEMBER_STC    *memberPtr
)
{
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    return prvCpssGenericTrunkDesignatedMemberSet(devNum,trunkId,
            enable,memberPtr);
}

/**
* @internal cpssDxChTrunkDesignatedMemberSet function
* @endinternal
*
* @brief   Function Relevant mode : High Level mode
*         This function Configures per-trunk the designated member -
*         value should be stored (to DB) even designated member is not currently
*         a member of Trunk.
*         Setting value replace previously assigned designated member.
*         NOTE that:
*         under normal operation this should not be used with cascade Trunks,
*         due to the fact that in this case it affects all types of traffic -
*         not just Multi-destination as in regular Trunks.
*         Diagram 1 : Regular operation - Traffic distribution on all enabled
*         members (when no specific designated member defined)
*         %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*         index \ member %  M1 %  M2  %  M3 % M4 %
*         %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*         Index 0    %  1  %  0  %  0  % 0  %
*         %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*         Index 1    %  0  %  1  %  0  % 0  %
*         %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*         Index 2    %  0  %  0  %  1  % 0  %
*         %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*         Index 3    %  0  %  0  %  0  % 1  %
*         %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*         Index 4    %  1  %  0  %  0  % 0  %
*         %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*         Index 5    %  0  %  1  %  0  % 0  %
*         %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*         Index 6    %  0  %  0  %  1  % 0  %
*         %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*         Index 7    %  0  %  0  %  0  % 1  %
*         %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*         Diagram 2: Traffic distribution once specific designated member defined
*         (M3 in this case - which is currently enabled member in trunk)
*         %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*         index \ member %  M1 %  M2  %  M3 % M4 %
*         %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*         Index 0    %  0  %  0  %  1  % 0  %
*         %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*         Index 1    %  0  %  0  %  1  % 0  %
*         %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*         Index 2    %  0  %  0  %  1  % 0  %
*         %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*         Index 3    %  0  %  0  %  1  % 0  %
*         %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*         Index 4    %  0  %  0  %  1  % 0  %
*         %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*         Index 5    %  0  %  0  %  1  % 0  %
*         %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*         Index 6    %  0  %  0  %  1  % 0  %
*         %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*         Index 7    %  0  %  0  %  1  % 0  %
*         %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] trunkId                  - the trunk id.
* @param[in] enable                   - enable/disable designated trunk member.
*                                      GT_TRUE -
*                                      1. Clears all current Trunk member's designated bits
*                                      2. If input designated member, is currently an
* @param[in] enable                   at Trunk (in local device) enable its
*                                      bits on all indexes
*                                      3. Store designated member at the DB (new DB parameter
*                                      should be created for that)
*                                      GT_FALSE -
*                                      1. Redistribute current Trunk members bits (on all enabled members)
*                                      2. Clear designated member at  the DB
* @param[in] memberPtr                - (pointer to)the designated member we set to the trunk.
*                                      relevant only when enable = GT_TRUE
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_INITIALIZED       -the trunk library was not initialized for the device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or
*                                       bad trunkId number , or
*                                       bad member parameters :
*                                       (device & 0xE0) != 0 means that the HW can't support
*                                       this value , since HW has 5 bit
*                                       for device number
*                                       (port & 0xC0) != 0 means that the HW can't support
*                                       this value , since HW has 6 bit
*                                       for port number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_ALREADY_EXIST         - this member already exists in another trunk.
* @retval GT_BAD_STATE             - the trunk-type is not one of: CPSS_TRUNK_TYPE_FREE_E or CPSS_TRUNK_TYPE_REGULAR_E
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTrunkDesignatedMemberSet
(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              trunkId,
    IN GT_BOOL                  enable,
    IN CPSS_TRUNK_MEMBER_STC    *memberPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTrunkDesignatedMemberSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, trunkId, enable, memberPtr));

    rc = internal_cpssDxChTrunkDesignatedMemberSet(devNum, trunkId, enable, memberPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, trunkId, enable, memberPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTrunkDbDesignatedMemberGet function
* @endinternal
*
* @brief   Function Relevant mode : High Level mode
*         This function get Configuration per-trunk the designated member -
*         value should be stored (to DB) even designated member is not currently
*         a member of Trunk.
*         function uses the DB (no HW operations)
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] trunkId                  - the trunk id.
*
* @param[out] enablePtr                - (pointer to) enable/disable designated trunk member.
*                                      GT_TRUE -
*                                      1. Clears all current Trunk member's designated bits
*                                      2. If input designated member, is currently an
*                                      enabled-member at Trunk (in local device) enable its
*                                      bits on all indexes
*                                      3. Store designated member at the DB (new DB parameter
*                                      should be created for that)
*                                      GT_FALSE -
*                                      1. Redistribute current Trunk members bits (on all enabled members)
*                                      2. Clear designated member at  the DB
* @param[out] memberPtr                - (pointer to) the designated member of the trunk.
*                                      relevant only when enable = GT_TRUE
*
* @retval GT_OK                    - on success
* @retval GT_NOT_INITIALIZED       -the trunk library was not initialized for the device
* @retval GT_BAD_PARAM             - bad device number , or
*                                       bad trunkId number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_BAD_STATE             - the trunk-type is not one of: CPSS_TRUNK_TYPE_FREE_E or CPSS_TRUNK_TYPE_REGULAR_E
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTrunkDbDesignatedMemberGet
(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              trunkId,
    OUT GT_BOOL                 *enablePtr,
    OUT CPSS_TRUNK_MEMBER_STC   *memberPtr
)
{
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    return prvCpssGenericTrunkDbDesignatedMemberGet(devNum,trunkId,
            enablePtr,memberPtr);
}

/**
* @internal cpssDxChTrunkDbDesignatedMemberGet function
* @endinternal
*
* @brief   Function Relevant mode : High Level mode
*         This function get Configuration per-trunk the designated member -
*         value should be stored (to DB) even designated member is not currently
*         a member of Trunk.
*         function uses the DB (no HW operations)
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] trunkId                  - the trunk id.
*
* @param[out] enablePtr                - (pointer to) enable/disable designated trunk member.
*                                      GT_TRUE -
*                                      1. Clears all current Trunk member's designated bits
*                                      2. If input designated member, is currently an
*                                      enabled-member at Trunk (in local device) enable its
*                                      bits on all indexes
*                                      3. Store designated member at the DB (new DB parameter
*                                      should be created for that)
*                                      GT_FALSE -
*                                      1. Redistribute current Trunk members bits (on all enabled members)
*                                      2. Clear designated member at  the DB
* @param[out] memberPtr                - (pointer to) the designated member of the trunk.
*                                      relevant only when enable = GT_TRUE
*
* @retval GT_OK                    - on success
* @retval GT_NOT_INITIALIZED       -the trunk library was not initialized for the device
* @retval GT_BAD_PARAM             - bad device number , or
*                                       bad trunkId number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_BAD_STATE             - the trunk-type is not one of: CPSS_TRUNK_TYPE_FREE_E or CPSS_TRUNK_TYPE_REGULAR_E
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTrunkDbDesignatedMemberGet
(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              trunkId,
    OUT GT_BOOL                 *enablePtr,
    OUT CPSS_TRUNK_MEMBER_STC   *memberPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTrunkDbDesignatedMemberGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, trunkId, enablePtr, memberPtr));

    rc = internal_cpssDxChTrunkDbDesignatedMemberGet(devNum, trunkId, enablePtr, memberPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, trunkId, enablePtr, memberPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChTrunkMembersSet function
* @endinternal
*
* @brief   Function Relevant mode : High Level mode
*         This function set the trunk with the specified enable and disabled
*         members.
*         this setting override the previous setting of the trunk members.
*         the user can "invalidate/unset" trunk entry by setting :
*         numOfEnabledMembers = 0 and numOfDisabledMembers = 0
*         This function support next "set entry" options :
*         1. "reset" the entry
*         function will remove the previous settings
*         2. set entry after the entry was empty
*         function will set new settings
*         3. set entry with the same members that it is already hold
*         function will rewrite the HW entries as it was
*         4. set entry with different setting then previous setting
*         a. function will remove the previous settings
*         b. function will set new settings
*         Notes about designated trunk table:
*         If (no designated defined)
*         distribute MC/Cascade trunk traffic among the enabled members
*         else
*         1. Set all member ports bits with 0
*         2. If designated member is one of the enabled members, set its relevant
*         bits on all indexes to 1.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] trunkId                  - trunk id
* @param[in] enabledMembersArray[]    - (array of) members to set in this trunk as enabled
*                                      members .
*                                      (this parameter ignored if numOfEnabledMembers = 0)
* @param[in] numOfEnabledMembers      - number of enabled members in the array.
* @param[in] disabledMembersArray[]   - (array of) members to set in this trunk as disabled
*                                      members .
*                                      (this parameter ignored if numOfDisabledMembers = 0)
* @param[in] numOfDisabledMembers     - number of disabled members in the array.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_NOT_INITIALIZED       -the trunk library was not initialized for the device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - when the sum of number of enabled members + number of
*                                       disabled members exceed the number of maximum number
* @retval of members in trunk (total of 0 - 8 members allowed)
* @retval GT_BAD_PARAM             - bad device number , or
*                                       bad trunkId number , or
*                                       bad members parameters :
*                                       (device & 0xE0) != 0 means that the HW can't support
*                                       this value , since HW has 5 bit
*                                       for device number
*                                       (port & 0xC0) != 0 means that the HW can't support
*                                       this value , since HW has 6 bit
*                                       for port number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_ALREADY_EXIST         - one of the members already exists in another trunk
* @retval GT_BAD_STATE             - the trunk-type is not one of: CPSS_TRUNK_TYPE_FREE_E or CPSS_TRUNK_TYPE_REGULAR_E
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTrunkMembersSet
(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              trunkId,
    IN GT_U32                   numOfEnabledMembers,
    IN CPSS_TRUNK_MEMBER_STC    enabledMembersArray[],
    IN GT_U32                   numOfDisabledMembers,
    IN CPSS_TRUNK_MEMBER_STC    disabledMembersArray[]
)
{
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    return prvCpssGenericTrunkMembersSet(devNum,trunkId,
            numOfEnabledMembers,enabledMembersArray,
            numOfDisabledMembers,disabledMembersArray);
}

/**
* @internal cpssDxChTrunkMembersSet function
* @endinternal
*
* @brief   Function Relevant mode : High Level mode
*         This function set the trunk with the specified enable and disabled
*         members.
*         this setting override the previous setting of the trunk members.
*         the user can "invalidate/unset" trunk entry by setting :
*         numOfEnabledMembers = 0 and numOfDisabledMembers = 0
*         This function support next "set entry" options :
*         1. "reset" the entry
*         function will remove the previous settings
*         2. set entry after the entry was empty
*         function will set new settings
*         3. set entry with the same members that it is already hold
*         function will rewrite the HW entries as it was
*         4. set entry with different setting then previous setting
*         a. function will remove the previous settings
*         b. function will set new settings
*         Notes about designated trunk table:
*         If (no designated defined)
*         distribute MC/Cascade trunk traffic among the enabled members
*         else
*         1. Set all member ports bits with 0
*         2. If designated member is one of the enabled members, set its relevant
*         bits on all indexes to 1.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] trunkId                  - trunk id
* @param[in] enabledMembersArray[]    - (array of) members to set in this trunk as enabled
*                                      members .
*                                      (this parameter ignored if numOfEnabledMembers = 0)
* @param[in] numOfEnabledMembers      - number of enabled members in the array.
* @param[in] disabledMembersArray[]   - (array of) members to set in this trunk as disabled
*                                      members .
*                                      (this parameter ignored if numOfDisabledMembers = 0)
* @param[in] numOfDisabledMembers     - number of disabled members in the array.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_NOT_INITIALIZED       -the trunk library was not initialized for the device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - when the sum of number of enabled members + number of
*                                       disabled members exceed the number of maximum number
* @retval of members in trunk (total of 0 - 8 members allowed)
* @retval GT_BAD_PARAM             - bad device number , or
*                                       bad trunkId number , or
*                                       bad members parameters :
*                                       (device & 0xE0) != 0 means that the HW can't support
*                                       this value , since HW has 5 bit
*                                       for device number
*                                       (port & 0xC0) != 0 means that the HW can't support
*                                       this value , since HW has 6 bit
*                                       for port number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_ALREADY_EXIST         - one of the members already exists in another trunk
* @retval GT_BAD_STATE             - the trunk-type is not one of: CPSS_TRUNK_TYPE_FREE_E or CPSS_TRUNK_TYPE_REGULAR_E
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTrunkMembersSet
(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              trunkId,
    IN GT_U32                   numOfEnabledMembers,
    IN CPSS_TRUNK_MEMBER_STC    enabledMembersArray[],
    IN GT_U32                   numOfDisabledMembers,
    IN CPSS_TRUNK_MEMBER_STC    disabledMembersArray[]
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTrunkMembersSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, trunkId, numOfEnabledMembers, enabledMembersArray, numOfDisabledMembers, disabledMembersArray));

    rc = internal_cpssDxChTrunkMembersSet(devNum, trunkId, numOfEnabledMembers, enabledMembersArray, numOfDisabledMembers, disabledMembersArray);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, trunkId, numOfEnabledMembers, enabledMembersArray, numOfDisabledMembers, disabledMembersArray));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTrunkCascadeTrunkPortsSet function
* @endinternal
*
* @brief   Function Relevant mode : High Level mode
*         This function sets the 'cascade' trunk with the specified 'Local ports'
*         overriding any previous setting.
*         The cascade trunk may be invalidated/unset by portsMembersPtr = NULL.
*         Local ports are ports of only configured device.
*         Cascade trunk is:
*         - members are ports of only configured device pointed by devNum
*         - trunk members table is empty (see cpssDxChTrunkTableEntrySet)
*         Therefore it cannot be used as target by ingress engines like FDB,
*         Router, TTI, Ingress PCL and so on.
*         - members ports trunk ID are set (see cpssDxChTrunkPortTrunkIdSet).
*         Therefore packets ingresses in member ports are associated with trunk
*         - all members are enabled only and cannot be disabled.
*         - may be used for cascade traffic and pointed by the 'Device map table'
*         as the local target to reach to the 'Remote device'.
*         (For 'Device map table' refer to cpssDxChCscdDevMapTableSet(...))
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] trunkId                  - trunk id
* @param[in] portsMembersPtr          - (pointer to) local ports bitmap to be members of the
*                                      cascade trunk.
*                                      NULL - meaning that the trunk-id is 'invalidated' and
*                                      trunk-type will be changed to : CPSS_TRUNK_TYPE_FREE_E
*                                      not-NULL - meaning that the trunk-type will be : CPSS_TRUNK_TYPE_CASCADE_E
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_NOT_INITIALIZED       - the trunk library was not initialized for the device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - there are ports in the bitmap that not supported by
*                                       the device.
* @retval GT_BAD_PARAM             - bad device number , or bad trunkId number , or number
*                                       of ports (in the bitmap) larger then the number of
*                                       entries in the 'Designated trunk table'
* @retval GT_BAD_STATE             - the trunk-type is not one of: CPSS_TRUNK_TYPE_FREE_E or CPSS_TRUNK_TYPE_CASCADE_E
* @retval GT_ALREADY_EXIST         - one of the members already exists in another trunk ,
*                                       or this trunk hold members defined using cpssDxChTrunkMembersSet(...)
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. This function does not set ports are 'Cascade ports' (and also not
*       check that ports are 'cascade').
*       2. This function sets only next tables :
*       a. the designated trunk table:
*       distribute MC/Cascade trunk traffic among the members
*       b. the 'Non-trunk' table entry.
*       c. 'per port' trunkId
*       3. because this function not set the 'Trunk members' table entry , the
*       application should not point to this trunk from any ingress unit , like:
*       FDB , PCL action redirect , NH , TTI action redirect , PVE ...
*       (it should be pointed ONLY from the device map table)
*       4. this API supports only trunks with types : CPSS_TRUNK_TYPE_FREE_E or
*       CPSS_TRUNK_TYPE_CASCADE_E.
*       5. next APIs are not supported from trunk with type : CPSS_TRUNK_TYPE_CASCADE_E
*       cpssDxChTrunkMembersSet ,
*       cpssDxChTrunkMemberAdd , cpssDxChTrunkMemberRemove,
*       cpssDxChTrunkMemberEnable , cpssDxChTrunkMemberDisable
*       cpssDxChTrunkDbEnabledMembersGet , cpssDxChTrunkDbDisabledMembersGet
*       cpssDxChTrunkDesignatedMemberSet , cpssDxChTrunkDbDesignatedMemberGet
*       Comparing the 2 function :
*       cpssDxChTrunkCascadeTrunkPortsSet  |  cpssDxChTrunkMembersSet
*       ----------------------------------------------------------------------------
*       1. purpose 'Cascade trunk'        | 1. purpose 'Network trunk' , and
*       |  also 'Cascade trunk' with up to
*       |  8 members
*       ----------------------------------------------------------------------------
*       2. supported number of members depends  | 2. supports up to 8 members
*       on number of entries in the      |  (also in Lion).
*       'Designated trunk table'       |
*       -- Lion supports 64 entries (so up to |
*       64 ports in the 'Cascade trunk'). |
*       -- all other devices supports 8    |
*       entries (so up to 8 ports in the  |
*       'Cascade trunk').         |
*       ----------------------------------------------------------------------------
*       3. manipulate next trunk tables :    | 3. manipulate all trunk tables :
*       'Per port' trunk-id ,         | 'Per port' trunk-id , 'Trunk members',
*       'Non-trunk' , 'Designated trunk'   | 'Non-trunk' , 'Designated trunk' tables.
*       ----------------------------------------------------------------------------
*       4. ingress unit must not point to this  | 4. no restriction on ingress/egress
*       trunk (because 'Trunk members' entry |  units.
*       hold no ports)            |
*
*/
static GT_STATUS internal_cpssDxChTrunkCascadeTrunkPortsSet
(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              trunkId,
    IN CPSS_PORTS_BMP_STC       *portsMembersPtr
)
{
    GT_STATUS   rc;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    CASCADE_TRUNK_ID_FOR_SEPERATION_FROM_REGULARE_TRUNK_TABLE_CHECK(devNum,trunkId);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE &&
       portsMembersPtr)/* BMP can be NULL !!! */
    {
        /* check that the device supports all the physical ports that defined in the BMP */
        rc = prvCpssDxChTablePortsBmpCheck(devNum , portsMembersPtr);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    return prvCpssGenericTrunkCascadeTrunkPortsSet(devNum,trunkId,portsMembersPtr);
}

/**
* @internal cpssDxChTrunkCascadeTrunkPortsSet function
* @endinternal
*
* @brief   Function Relevant mode : High Level mode
*         This function sets the 'cascade' trunk with the specified 'Local ports'
*         overriding any previous setting.
*         The cascade trunk may be invalidated/unset by portsMembersPtr = NULL.
*         Local ports are ports of only configured device.
*         Cascade trunk is:
*         - members are ports of only configured device pointed by devNum
*         - trunk members table is empty (see cpssDxChTrunkTableEntrySet)
*         Therefore it cannot be used as target by ingress engines like FDB,
*         Router, TTI, Ingress PCL and so on.
*         - members ports trunk ID are set (see cpssDxChTrunkPortTrunkIdSet).
*         Therefore packets ingresses in member ports are associated with trunk
*         - all members are enabled only and cannot be disabled.
*         - may be used for cascade traffic and pointed by the 'Device map table'
*         as the local target to reach to the 'Remote device'.
*         (For 'Device map table' refer to cpssDxChCscdDevMapTableSet(...))
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] trunkId                  - trunk id
* @param[in] portsMembersPtr          - (pointer to) local ports bitmap to be members of the
*                                      cascade trunk.
*                                      NULL - meaning that the trunk-id is 'invalidated' and
*                                      trunk-type will be changed to : CPSS_TRUNK_TYPE_FREE_E
*                                      not-NULL - meaning that the trunk-type will be : CPSS_TRUNK_TYPE_CASCADE_E
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_NOT_INITIALIZED       - the trunk library was not initialized for the device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - there are ports in the bitmap that not supported by
*                                       the device.
* @retval GT_BAD_PARAM             - bad device number , or bad trunkId number , or number
*                                       of ports (in the bitmap) larger then the number of
*                                       entries in the 'Designated trunk table'
* @retval GT_BAD_STATE             - the trunk-type is not one of: CPSS_TRUNK_TYPE_FREE_E or CPSS_TRUNK_TYPE_CASCADE_E
* @retval GT_ALREADY_EXIST         - one of the members already exists in another trunk ,
*                                       or this trunk hold members defined using cpssDxChTrunkMembersSet(...)
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. This function does not set ports are 'Cascade ports' (and also not
*       check that ports are 'cascade').
*       2. This function sets only next tables :
*       a. the designated trunk table:
*       distribute MC/Cascade trunk traffic among the members
*       b. the 'Non-trunk' table entry.
*       c. 'per port' trunkId
*       3. because this function not set the 'Trunk members' table entry , the
*       application should not point to this trunk from any ingress unit , like:
*       FDB , PCL action redirect , NH , TTI action redirect , PVE ...
*       (it should be pointed ONLY from the device map table)
*       4. this API supports only trunks with types : CPSS_TRUNK_TYPE_FREE_E or
*       CPSS_TRUNK_TYPE_CASCADE_E.
*       5. next APIs are not supported from trunk with type : CPSS_TRUNK_TYPE_CASCADE_E
*       cpssDxChTrunkMembersSet ,
*       cpssDxChTrunkMemberAdd , cpssDxChTrunkMemberRemove,
*       cpssDxChTrunkMemberEnable , cpssDxChTrunkMemberDisable
*       cpssDxChTrunkDbEnabledMembersGet , cpssDxChTrunkDbDisabledMembersGet
*       cpssDxChTrunkDesignatedMemberSet , cpssDxChTrunkDbDesignatedMemberGet
*       Comparing the 2 function :
*       cpssDxChTrunkCascadeTrunkPortsSet  |  cpssDxChTrunkMembersSet
*       ----------------------------------------------------------------------------
*       1. purpose 'Cascade trunk'        | 1. purpose 'Network trunk' , and
*       |  also 'Cascade trunk' with up to
*       |  8 members
*       ----------------------------------------------------------------------------
*       2. supported number of members depends  | 2. supports up to 8 members
*       on number of entries in the      |  (also in Lion).
*       'Designated trunk table'       |
*       -- Lion supports 64 entries (so up to |
*       64 ports in the 'Cascade trunk'). |
*       -- all other devices supports 8    |
*       entries (so up to 8 ports in the  |
*       'Cascade trunk').         |
*       ----------------------------------------------------------------------------
*       3. manipulate next trunk tables :    | 3. manipulate all trunk tables :
*       'Per port' trunk-id ,         | 'Per port' trunk-id , 'Trunk members',
*       'Non-trunk' , 'Designated trunk'   | 'Non-trunk' , 'Designated trunk' tables.
*       ----------------------------------------------------------------------------
*       4. ingress unit must not point to this  | 4. no restriction on ingress/egress
*       trunk (because 'Trunk members' entry |  units.
*       hold no ports)            |
*
*/
GT_STATUS cpssDxChTrunkCascadeTrunkPortsSet
(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              trunkId,
    IN CPSS_PORTS_BMP_STC       *portsMembersPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTrunkCascadeTrunkPortsSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, trunkId, portsMembersPtr));

    rc = internal_cpssDxChTrunkCascadeTrunkPortsSet(devNum, trunkId, portsMembersPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, trunkId, portsMembersPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTrunkCascadeTrunkPortsGet function
* @endinternal
*
* @brief   Function Relevant mode : High Level mode
*         Gets the 'Local ports' of the 'cascade' trunk .
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] trunkId                  - trunk id
*
* @param[out] portsMembersPtr          - (pointer to) local ports bitmap of the cascade trunk.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_NOT_INITIALIZED       - the trunk library was not initialized for the device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or bad trunkId number
* @retval GT_BAD_STATE             - the trunk-type is not one of: CPSS_TRUNK_TYPE_FREE_E or CPSS_TRUNK_TYPE_CASCADE_E
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTrunkCascadeTrunkPortsGet
(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              trunkId,
    OUT CPSS_PORTS_BMP_STC       *portsMembersPtr
)
{
    GT_STATUS   rc;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    CASCADE_TRUNK_ID_FOR_SEPERATION_FROM_REGULARE_TRUNK_TABLE_CHECK(devNum,trunkId);

    rc = prvCpssGenericTrunkCascadeTrunkPortsGet(devNum,trunkId,portsMembersPtr);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        if(rc != GT_OK)
        {
            return rc;
        }
        /* adjusted physical ports of BMP to hold only bmp of existing ports*/
        rc = prvCpssDxChTablePortsBmpAdjustToDevice(devNum , portsMembersPtr , portsMembersPtr);
    }

    return rc;
}

/**
* @internal cpssDxChTrunkCascadeTrunkPortsGet function
* @endinternal
*
* @brief   Function Relevant mode : High Level mode
*         Gets the 'Local ports' of the 'cascade' trunk .
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] trunkId                  - trunk id
*
* @param[out] portsMembersPtr          - (pointer to) local ports bitmap of the cascade trunk.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_NOT_INITIALIZED       - the trunk library was not initialized for the device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or bad trunkId number
* @retval GT_BAD_STATE             - the trunk-type is not one of: CPSS_TRUNK_TYPE_FREE_E or CPSS_TRUNK_TYPE_CASCADE_E
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTrunkCascadeTrunkPortsGet
(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              trunkId,
    OUT CPSS_PORTS_BMP_STC       *portsMembersPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTrunkCascadeTrunkPortsGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, trunkId, portsMembersPtr));

    rc = internal_cpssDxChTrunkCascadeTrunkPortsGet(devNum, trunkId, portsMembersPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, trunkId, portsMembersPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTrunkCascadeTrunkWithWeightedPortsSet function
* @endinternal
*
* @brief   Function Relevant mode : High Level mode
*         This function sets the 'cascade' trunk with the Weighted specified 'Local ports'
*         overriding any previous setting.
*         the weights effect the number of times that each member will get representation
*         in the 'designated table' .
*         The cascade trunk may be invalidated by numOfMembers = 0.
*         Local ports are ports of only configured device.
*         Cascade trunk is:
*         - members are ports of only configured device pointed by devNum
*         - trunk members table is empty (see cpssDxChTrunkTableEntrySet)
*         Therefore it cannot be used as target by ingress engines like FDB,
*         Router, TTI, Ingress PCL and so on.
*         - members ports trunk ID are set (see cpssDxChTrunkPortTrunkIdSet).
*         Therefore packets ingresses in member ports are associated with trunk
*         - all members are enabled only and cannot be disabled.
*         - may be used for cascade traffic and pointed by the 'Device map table'
*         as the local target to reach to the 'Remote device'.
*         (For 'Device map table' refer to cpssDxChCscdDevMapTableSet(...))
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] trunkId                  - trunk id
* @param[in] numOfMembers             - number of members in the array.
*                                      value 0 - meaning that the trunk-id is 'invalidated' and
*                                      trunk-type will be changed to : CPSS_TRUNK_TYPE_FREE_E
*                                      value != 0 - meaning that the trunk-type will be : CPSS_TRUNK_TYPE_CASCADE_E
* @param[in] weightedMembersArray[]   - (array of) members of the cascade trunk.
*                                      each member hold relative weight (relative to Weight of all members)
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_NOT_INITIALIZED       - the trunk library was not initialized for the device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - there are members that not supported by the device.
* @retval GT_BAD_PARAM             - bad device number , or bad trunkId number , or bad total
*                                       weights (see restrictions below)
* @retval GT_BAD_STATE             - the trunk-type is not one of: CPSS_TRUNK_TYPE_FREE_E or CPSS_TRUNK_TYPE_CASCADE_E
* @retval GT_ALREADY_EXIST         - one of the members already exists in another trunk ,
*                                       or this trunk hold members defined using cpssDxChTrunkMembersSet(...)
* @retval GT_BAD_PTR               - when numOfMembers != 0 and weightedMembersArray = NULL
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. This function does not set ports as 'Cascade ports' (and also not
*       check that ports are 'cascade').
*       2. This function sets only next tables :
*       a. the designated trunk table:
*       distribute MC/Cascade trunk traffic among the members according to their weight
*       b. the 'Non-trunk' table entry.
*       c. 'per port' trunkId
*       3. because this function not set the 'Trunk members' table entry , the
*       application should not point to this trunk from any ingress unit , like:
*       FDB , PCL action redirect , NH , TTI action redirect , PVE ...
*       (it should be pointed ONLY from the device map table)
*       4. this API supports only trunks with types : CPSS_TRUNK_TYPE_FREE_E or
*       CPSS_TRUNK_TYPE_CASCADE_E.
*       5. next APIs are not supported from trunk with type : CPSS_TRUNK_TYPE_CASCADE_E
*       cpssDxChTrunkMembersSet ,
*       cpssDxChTrunkMemberAdd , cpssDxChTrunkMemberRemove,
*       cpssDxChTrunkMemberEnable , cpssDxChTrunkMemberDisable
*       cpssDxChTrunkDbEnabledMembersGet , cpssDxChTrunkDbDisabledMembersGet
*       cpssDxChTrunkDesignatedMemberSet , cpssDxChTrunkDbDesignatedMemberGet
*       6. the total weights of all the trunk members is restricted to :
*       a. must be equal to power of 2 (1,2,4,8,16,32,64...)
*       b. must not be larger then actual designated table size of the device.
*       (Lion ,Lion2 : 64 , other devices : 8)
*       c. not relevant when single member exists
*       Comparing the 2 function :
*       cpssDxChTrunkCascadeTrunkWithWeightedPortsSet |  cpssDxChTrunkMembersSet
*       ----------------------------------------------------------------------------
*       1. purpose 'Cascade trunk'        | 1. purpose 'Network trunk' , and
*       |  also 'Cascade trunk' with up to
*       |  8 members
*       ----------------------------------------------------------------------------
*       2. supported number of members depends  | 2. supports up to 8 members
*       on number of entries in the      |  (also in Lion).
*       'Designated trunk table'       |
*       -- Lion supports 64 entries (so up to |
*       64 ports in the 'Cascade trunk'). |
*       -- all other devices supports 8    |
*       entries (so up to 8 ports in the  |
*       'Cascade trunk').         |
*       ----------------------------------------------------------------------------
*       3. manipulate next trunk tables :    | 3. manipulate all trunk tables :
*       'Per port' trunk-id          | 'Per port' trunk-id , 'Trunk members',
*       'Non-trunk' , 'Designated trunk'   | 'Non-trunk' , 'Designated trunk' tables.
*       ----------------------------------------------------------------------------
*       4. ingress unit must not point to this  | 4. no restriction on ingress/egress
*       trunk (because 'Trunk members' entry |  units.
*       hold no ports)            |
*
*/
static GT_STATUS internal_cpssDxChTrunkCascadeTrunkWithWeightedPortsSet
(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              trunkId,
    IN GT_U32                   numOfMembers,
    IN CPSS_TRUNK_WEIGHTED_MEMBER_STC       weightedMembersArray[]
)
{
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    CASCADE_TRUNK_ID_FOR_SEPERATION_FROM_REGULARE_TRUNK_TABLE_CHECK(devNum,trunkId);

    return prvCpssGenericTrunkCascadeTrunkWithWeightedPortsSet(devNum,trunkId,numOfMembers,weightedMembersArray);
}

/**
* @internal cpssDxChTrunkCascadeTrunkWithWeightedPortsSet function
* @endinternal
*
* @brief   Function Relevant mode : High Level mode
*         This function sets the 'cascade' trunk with the Weighted specified 'Local ports'
*         overriding any previous setting.
*         the weights effect the number of times that each member will get representation
*         in the 'designated table' .
*         The cascade trunk may be invalidated by numOfMembers = 0.
*         Local ports are ports of only configured device.
*         Cascade trunk is:
*         - members are ports of only configured device pointed by devNum
*         - trunk members table is empty (see cpssDxChTrunkTableEntrySet)
*         Therefore it cannot be used as target by ingress engines like FDB,
*         Router, TTI, Ingress PCL and so on.
*         - members ports trunk ID are set (see cpssDxChTrunkPortTrunkIdSet).
*         Therefore packets ingresses in member ports are associated with trunk
*         - all members are enabled only and cannot be disabled.
*         - may be used for cascade traffic and pointed by the 'Device map table'
*         as the local target to reach to the 'Remote device'.
*         (For 'Device map table' refer to cpssDxChCscdDevMapTableSet(...))
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] trunkId                  - trunk id
* @param[in] numOfMembers             - number of members in the array.
*                                      value 0 - meaning that the trunk-id is 'invalidated' and
*                                      trunk-type will be changed to : CPSS_TRUNK_TYPE_FREE_E
*                                      value != 0 - meaning that the trunk-type will be : CPSS_TRUNK_TYPE_CASCADE_E
* @param[in] weightedMembersArray[]   - (array of) members of the cascade trunk.
*                                      each member hold relative weight (relative to Weight of all members)
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_NOT_INITIALIZED       - the trunk library was not initialized for the device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - there are members that not supported by the device.
* @retval GT_BAD_PARAM             - bad device number , or bad trunkId number , or bad total
*                                       weights (see restrictions below)
* @retval GT_BAD_STATE             - the trunk-type is not one of: CPSS_TRUNK_TYPE_FREE_E or CPSS_TRUNK_TYPE_CASCADE_E
* @retval GT_ALREADY_EXIST         - one of the members already exists in another trunk ,
*                                       or this trunk hold members defined using cpssDxChTrunkMembersSet(...)
* @retval GT_BAD_PTR               - when numOfMembers != 0 and weightedMembersArray = NULL
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. This function does not set ports as 'Cascade ports' (and also not
*       check that ports are 'cascade').
*       2. This function sets only next tables :
*       a. the designated trunk table:
*       distribute MC/Cascade trunk traffic among the members according to their weight
*       b. the 'Non-trunk' table entry.
*       c. 'per port' trunkId
*       3. because this function not set the 'Trunk members' table entry , the
*       application should not point to this trunk from any ingress unit , like:
*       FDB , PCL action redirect , NH , TTI action redirect , PVE ...
*       (it should be pointed ONLY from the device map table)
*       4. this API supports only trunks with types : CPSS_TRUNK_TYPE_FREE_E or
*       CPSS_TRUNK_TYPE_CASCADE_E.
*       5. next APIs are not supported from trunk with type : CPSS_TRUNK_TYPE_CASCADE_E
*       cpssDxChTrunkMembersSet ,
*       cpssDxChTrunkMemberAdd , cpssDxChTrunkMemberRemove,
*       cpssDxChTrunkMemberEnable , cpssDxChTrunkMemberDisable
*       cpssDxChTrunkDbEnabledMembersGet , cpssDxChTrunkDbDisabledMembersGet
*       cpssDxChTrunkDesignatedMemberSet , cpssDxChTrunkDbDesignatedMemberGet
*       6. the total weights of all the trunk members is restricted to :
*       a. must be equal to power of 2 (1,2,4,8,16,32,64...)
*       b. must not be larger then actual designated table size of the device.
*       (Lion ,Lion2 : 64 , other devices : 8)
*       c. not relevant when single member exists
*       Comparing the 2 function :
*       cpssDxChTrunkCascadeTrunkWithWeightedPortsSet |  cpssDxChTrunkMembersSet
*       ----------------------------------------------------------------------------
*       1. purpose 'Cascade trunk'        | 1. purpose 'Network trunk' , and
*       |  also 'Cascade trunk' with up to
*       |  8 members
*       ----------------------------------------------------------------------------
*       2. supported number of members depends  | 2. supports up to 8 members
*       on number of entries in the      |  (also in Lion).
*       'Designated trunk table'       |
*       -- Lion supports 64 entries (so up to |
*       64 ports in the 'Cascade trunk'). |
*       -- all other devices supports 8    |
*       entries (so up to 8 ports in the  |
*       'Cascade trunk').         |
*       ----------------------------------------------------------------------------
*       3. manipulate next trunk tables :    | 3. manipulate all trunk tables :
*       'Per port' trunk-id          | 'Per port' trunk-id , 'Trunk members',
*       'Non-trunk' , 'Designated trunk'   | 'Non-trunk' , 'Designated trunk' tables.
*       ----------------------------------------------------------------------------
*       4. ingress unit must not point to this  | 4. no restriction on ingress/egress
*       trunk (because 'Trunk members' entry |  units.
*       hold no ports)            |
*
*/
GT_STATUS cpssDxChTrunkCascadeTrunkWithWeightedPortsSet
(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              trunkId,
    IN GT_U32                   numOfMembers,
    IN CPSS_TRUNK_WEIGHTED_MEMBER_STC       weightedMembersArray[]
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTrunkCascadeTrunkWithWeightedPortsSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, trunkId, numOfMembers, weightedMembersArray));

    rc = internal_cpssDxChTrunkCascadeTrunkWithWeightedPortsSet(devNum, trunkId, numOfMembers, weightedMembersArray);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, trunkId, numOfMembers, weightedMembersArray));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTrunkCascadeTrunkWithWeightedPortsGet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         This function gets the Weighted 'Local ports' of 'cascade' trunk .
*         the weights reflect the number of times that each member is represented
*         in the 'designated table'
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] trunkId                  - trunk id
* @param[in,out] numOfMembersPtr          - (pointer to) max num of members to retrieve - this value refer to the number of
*                                      members that the array of weightedMembersArray[] can retrieve.
*                                      OUTPUTS:
* @param[in,out] numOfMembersPtr          - (pointer to) the actual num of members in the trunk
*                                      (up to : Lion,Lion2 : 64 , others : 8)
* @param[in] weightedMembersArray[]   - (array of) members that are members of the cascade trunk.
*                                      each member hold relative weight (relative to Weight of all members)
* @param[in,out] numOfMembersPtr          - (pointer to) the actual num of members in the trunk
*                                      (up to : Lion,Lion2 : 64 , others : 8)
*
* @param[out] weightedMembersArray[]   - (array of) members that are members of the cascade trunk.
*                                      each member hold relative weight (relative to Weight of all members)
*                                      OUTPUTS:
*                                      None.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_INITIALIZED       -the trunk library was not initialized for the device
* @retval GT_BAD_PARAM             - bad device number , or
*                                       bad trunkId number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_BAD_STATE             - the trunk-type is not one of: CPSS_TRUNK_TYPE_FREE_E or CPSS_TRUNK_TYPE_CASCADE_E
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTrunkCascadeTrunkWithWeightedPortsGet
(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              trunkId,
    INOUT GT_U32                   *numOfMembersPtr,
    OUT CPSS_TRUNK_WEIGHTED_MEMBER_STC       weightedMembersArray[]
)
{
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    CASCADE_TRUNK_ID_FOR_SEPERATION_FROM_REGULARE_TRUNK_TABLE_CHECK(devNum,trunkId);

    return prvCpssGenericTrunkCascadeTrunkWithWeightedPortsGet(devNum,trunkId,numOfMembersPtr,weightedMembersArray);
}

/**
* @internal cpssDxChTrunkCascadeTrunkWithWeightedPortsGet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         This function gets the Weighted 'Local ports' of 'cascade' trunk .
*         the weights reflect the number of times that each member is represented
*         in the 'designated table'
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] trunkId                  - trunk id
* @param[in,out] numOfMembersPtr          - (pointer to) max num of members to retrieve - this value refer to the number of
*                                      members that the array of weightedMembersArray[] can retrieve.
*                                      OUTPUTS:
* @param[in,out] numOfMembersPtr          - (pointer to) the actual num of members in the trunk
*                                      (up to : Lion,Lion2 : 64 , others : 8)
* @param[in] weightedMembersArray[]   - (array of) members that are members of the cascade trunk.
*                                      each member hold relative weight (relative to Weight of all members)
* @param[in,out] numOfMembersPtr          - (pointer to) the actual num of members in the trunk
*                                      (up to : Lion,Lion2 : 64 , others : 8)
*
* @param[out] weightedMembersArray[]   - (array of) members that are members of the cascade trunk.
*                                      each member hold relative weight (relative to Weight of all members)
*                                      OUTPUTS:
*                                      None.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_INITIALIZED       -the trunk library was not initialized for the device
* @retval GT_BAD_PARAM             - bad device number , or
*                                       bad trunkId number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_BAD_STATE             - the trunk-type is not one of: CPSS_TRUNK_TYPE_FREE_E or CPSS_TRUNK_TYPE_CASCADE_E
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTrunkCascadeTrunkWithWeightedPortsGet
(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              trunkId,
    INOUT GT_U32                   *numOfMembersPtr,
    OUT CPSS_TRUNK_WEIGHTED_MEMBER_STC       weightedMembersArray[]
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTrunkCascadeTrunkWithWeightedPortsGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, trunkId, numOfMembersPtr, weightedMembersArray));

    rc = internal_cpssDxChTrunkCascadeTrunkWithWeightedPortsGet(devNum, trunkId, numOfMembersPtr, weightedMembersArray);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, trunkId, numOfMembersPtr, weightedMembersArray));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}



/**
* @internal internal_cpssDxChTrunkMemberAdd function
* @endinternal
*
* @brief   Function Relevant mode : High Level mode
*         This function add member to the trunk in the device.
*         If member is already in this trunk , function do nothing and
*         return GT_OK.
*         Notes about designated trunk table:
*         If (no designated defined)
*         re-distribute MC/Cascade trunk traffic among the enabled members,
*         now taking into account also the added member
*         else
*         1. If added member is not the designated member - set its relevant bits to 0
*         2. If added member is the designated member & it's enabled,
*         set its relevant bits on all indexes to 1.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number on which to add member to the trunk
* @param[in] trunkId                  - the trunk id.
* @param[in] memberPtr                - (pointer to)the member to add to the trunk.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_INITIALIZED       -the trunk library was not initialized for the device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or
*                                       bad trunkId number , or
*                                       bad member parameters :
*                                       (device & 0xE0) != 0 means that the HW can't support
*                                       this value , since HW has 5 bit
*                                       for device number
*                                       (port & 0xC0) != 0 means that the HW can't support
*                                       this value , since HW has 6 bit
*                                       for port number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_ALREADY_EXIST         - this member already exists in another trunk.
* @retval GT_FULL                  - trunk already contains maximum supported members
* @retval GT_BAD_STATE             - the trunk-type is not one of: CPSS_TRUNK_TYPE_FREE_E or CPSS_TRUNK_TYPE_REGULAR_E
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTrunkMemberAdd
(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              trunkId,
    IN CPSS_TRUNK_MEMBER_STC    *memberPtr
)
{
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    return prvCpssGenericTrunkMemberAdd(devNum,trunkId,memberPtr);
}

/**
* @internal cpssDxChTrunkMemberAdd function
* @endinternal
*
* @brief   Function Relevant mode : High Level mode
*         This function add member to the trunk in the device.
*         If member is already in this trunk , function do nothing and
*         return GT_OK.
*         Notes about designated trunk table:
*         If (no designated defined)
*         re-distribute MC/Cascade trunk traffic among the enabled members,
*         now taking into account also the added member
*         else
*         1. If added member is not the designated member - set its relevant bits to 0
*         2. If added member is the designated member & it's enabled,
*         set its relevant bits on all indexes to 1.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number on which to add member to the trunk
* @param[in] trunkId                  - the trunk id.
* @param[in] memberPtr                - (pointer to)the member to add to the trunk.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_INITIALIZED       -the trunk library was not initialized for the device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or
*                                       bad trunkId number , or
*                                       bad member parameters :
*                                       (device & 0xE0) != 0 means that the HW can't support
*                                       this value , since HW has 5 bit
*                                       for device number
*                                       (port & 0xC0) != 0 means that the HW can't support
*                                       this value , since HW has 6 bit
*                                       for port number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_ALREADY_EXIST         - this member already exists in another trunk.
* @retval GT_FULL                  - trunk already contains maximum supported members
* @retval GT_BAD_STATE             - the trunk-type is not one of: CPSS_TRUNK_TYPE_FREE_E or CPSS_TRUNK_TYPE_REGULAR_E
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTrunkMemberAdd
(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              trunkId,
    IN CPSS_TRUNK_MEMBER_STC    *memberPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTrunkMemberAdd);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, trunkId, memberPtr));

    rc = internal_cpssDxChTrunkMemberAdd(devNum, trunkId, memberPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, trunkId, memberPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTrunkMemberRemove function
* @endinternal
*
* @brief   Function Relevant mode : High Level mode
*         This function remove member from a trunk in the device.
*         If member not exists in this trunk , function do nothing and
*         return GT_OK.
*         Notes about designated trunk table:
*         If (no designated defined)
*         re-distribute MC/Cascade trunk traffic among the enabled members -
*         now taking into account also the removed member
*         else
*         1. If removed member is not the designated member - nothing to do
*         2. If removed member is the designated member set its relevant bits
*         on all indexes to 1.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number on which to remove member from the trunk
* @param[in] trunkId                  - the trunk id.
* @param[in] memberPtr                - (pointer to)the member to remove from the trunk.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_INITIALIZED       -the trunk library was not initialized for the device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or
*                                       bad trunkId number , or
*                                       bad member parameters :
*                                       (device & 0xE0) != 0 means that the HW can't support
*                                       this value , since HW has 5 bit
*                                       for device number
*                                       (port & 0xC0) != 0 means that the HW can't support
*                                       this value , since HW has 6 bit
*                                       for port number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_BAD_STATE             - the trunk-type is not one of: CPSS_TRUNK_TYPE_FREE_E or CPSS_TRUNK_TYPE_REGULAR_E
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTrunkMemberRemove
(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              trunkId,
    IN CPSS_TRUNK_MEMBER_STC    *memberPtr
)
{
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    return prvCpssGenericTrunkMemberRemove(devNum,trunkId,memberPtr);
}

/**
* @internal cpssDxChTrunkMemberRemove function
* @endinternal
*
* @brief   Function Relevant mode : High Level mode
*         This function remove member from a trunk in the device.
*         If member not exists in this trunk , function do nothing and
*         return GT_OK.
*         Notes about designated trunk table:
*         If (no designated defined)
*         re-distribute MC/Cascade trunk traffic among the enabled members -
*         now taking into account also the removed member
*         else
*         1. If removed member is not the designated member - nothing to do
*         2. If removed member is the designated member set its relevant bits
*         on all indexes to 1.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number on which to remove member from the trunk
* @param[in] trunkId                  - the trunk id.
* @param[in] memberPtr                - (pointer to)the member to remove from the trunk.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_INITIALIZED       -the trunk library was not initialized for the device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or
*                                       bad trunkId number , or
*                                       bad member parameters :
*                                       (device & 0xE0) != 0 means that the HW can't support
*                                       this value , since HW has 5 bit
*                                       for device number
*                                       (port & 0xC0) != 0 means that the HW can't support
*                                       this value , since HW has 6 bit
*                                       for port number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_BAD_STATE             - the trunk-type is not one of: CPSS_TRUNK_TYPE_FREE_E or CPSS_TRUNK_TYPE_REGULAR_E
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTrunkMemberRemove
(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              trunkId,
    IN CPSS_TRUNK_MEMBER_STC    *memberPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTrunkMemberRemove);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, trunkId, memberPtr));

    rc = internal_cpssDxChTrunkMemberRemove(devNum, trunkId, memberPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, trunkId, memberPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTrunkMemberDisable function
* @endinternal
*
* @brief   Function Relevant mode : High Level mode
*         This function disable (enabled)existing member of trunk in the device.
*         If member is already disabled in this trunk , function do nothing and
*         return GT_OK.
*         Notes about designated trunk table:
*         If (no designated defined)
*         re-distribute MC/Cascade trunk traffic among the enabled members -
*         now taking into account also the disabled member
*         else
*         1. If disabled member is not the designated member - set its relevant bits to 0
*         2. If disabled member is the designated member set its relevant bits
*         on all indexes to 0.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number on which to disable member in the trunk
* @param[in] trunkId                  - the trunk id.
* @param[in] memberPtr                - (pointer to)the member to disable in the trunk.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_INITIALIZED       -the trunk library was not initialized for the device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or
*                                       bad trunkId number , or
*                                       bad member parameters :
*                                       (device & 0xE0) != 0 means that the HW can't support
*                                       this value , since HW has 5 bit
*                                       for device number
*                                       (port & 0xC0) != 0 means that the HW can't support
*                                       this value , since HW has 6 bit
*                                       for port number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_FOUND             - this member not found (member not exist) in the trunk
* @retval GT_BAD_STATE             - the trunk-type is not one of: CPSS_TRUNK_TYPE_FREE_E or CPSS_TRUNK_TYPE_REGULAR_E
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTrunkMemberDisable
(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              trunkId,
    IN CPSS_TRUNK_MEMBER_STC    *memberPtr
)
{
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    return prvCpssGenericTrunkMemberDisable(devNum,trunkId,memberPtr);
}

/**
* @internal cpssDxChTrunkMemberDisable function
* @endinternal
*
* @brief   Function Relevant mode : High Level mode
*         This function disable (enabled)existing member of trunk in the device.
*         If member is already disabled in this trunk , function do nothing and
*         return GT_OK.
*         Notes about designated trunk table:
*         If (no designated defined)
*         re-distribute MC/Cascade trunk traffic among the enabled members -
*         now taking into account also the disabled member
*         else
*         1. If disabled member is not the designated member - set its relevant bits to 0
*         2. If disabled member is the designated member set its relevant bits
*         on all indexes to 0.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number on which to disable member in the trunk
* @param[in] trunkId                  - the trunk id.
* @param[in] memberPtr                - (pointer to)the member to disable in the trunk.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_INITIALIZED       -the trunk library was not initialized for the device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or
*                                       bad trunkId number , or
*                                       bad member parameters :
*                                       (device & 0xE0) != 0 means that the HW can't support
*                                       this value , since HW has 5 bit
*                                       for device number
*                                       (port & 0xC0) != 0 means that the HW can't support
*                                       this value , since HW has 6 bit
*                                       for port number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_FOUND             - this member not found (member not exist) in the trunk
* @retval GT_BAD_STATE             - the trunk-type is not one of: CPSS_TRUNK_TYPE_FREE_E or CPSS_TRUNK_TYPE_REGULAR_E
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTrunkMemberDisable
(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              trunkId,
    IN CPSS_TRUNK_MEMBER_STC    *memberPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTrunkMemberDisable);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, trunkId, memberPtr));

    rc = internal_cpssDxChTrunkMemberDisable(devNum, trunkId, memberPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, trunkId, memberPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTrunkMemberEnable function
* @endinternal
*
* @brief   Function Relevant mode : High Level mode
*         This function enable (disabled)existing member of trunk in the device.
*         If member is already enabled in this trunk , function do nothing and
*         return GT_OK.
*         Notes about designated trunk table:
*         If (no designated defined)
*         re-distribute MC/Cascade trunk traffic among the enabled members -
*         now taking into account also the enabled member
*         else
*         1. If enabled member is not the designated member - set its relevant bits to 0
*         2. If enabled member is the designated member set its relevant bits
*         on all indexes to 1.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number on which to enable member in the trunk
* @param[in] trunkId                  - the trunk id.
* @param[in] memberPtr                - (pointer to)the member to enable in the trunk.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_INITIALIZED       -the trunk library was not initialized for the device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or
*                                       bad trunkId number , or
*                                       bad member parameters :
*                                       (device & 0xE0) != 0 means that the HW can't support
*                                       this value , since HW has 5 bit
*                                       for device number
*                                       (port & 0xC0) != 0 means that the HW can't support
*                                       this value , since HW has 6 bit
*                                       for port number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_FOUND             - this member not found (member not exist) in the trunk
* @retval GT_BAD_STATE             - the trunk-type is not one of: CPSS_TRUNK_TYPE_FREE_E or CPSS_TRUNK_TYPE_REGULAR_E
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTrunkMemberEnable
(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              trunkId,
    IN CPSS_TRUNK_MEMBER_STC    *memberPtr
)
{
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    return prvCpssGenericTrunkMemberEnable(devNum,trunkId,memberPtr);
}

/**
* @internal cpssDxChTrunkMemberEnable function
* @endinternal
*
* @brief   Function Relevant mode : High Level mode
*         This function enable (disabled)existing member of trunk in the device.
*         If member is already enabled in this trunk , function do nothing and
*         return GT_OK.
*         Notes about designated trunk table:
*         If (no designated defined)
*         re-distribute MC/Cascade trunk traffic among the enabled members -
*         now taking into account also the enabled member
*         else
*         1. If enabled member is not the designated member - set its relevant bits to 0
*         2. If enabled member is the designated member set its relevant bits
*         on all indexes to 1.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number on which to enable member in the trunk
* @param[in] trunkId                  - the trunk id.
* @param[in] memberPtr                - (pointer to)the member to enable in the trunk.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_INITIALIZED       -the trunk library was not initialized for the device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or
*                                       bad trunkId number , or
*                                       bad member parameters :
*                                       (device & 0xE0) != 0 means that the HW can't support
*                                       this value , since HW has 5 bit
*                                       for device number
*                                       (port & 0xC0) != 0 means that the HW can't support
*                                       this value , since HW has 6 bit
*                                       for port number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_FOUND             - this member not found (member not exist) in the trunk
* @retval GT_BAD_STATE             - the trunk-type is not one of: CPSS_TRUNK_TYPE_FREE_E or CPSS_TRUNK_TYPE_REGULAR_E
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTrunkMemberEnable
(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              trunkId,
    IN CPSS_TRUNK_MEMBER_STC    *memberPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTrunkMemberEnable);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, trunkId, memberPtr));

    rc = internal_cpssDxChTrunkMemberEnable(devNum, trunkId, memberPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, trunkId, memberPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChTrunkNonTrunkPortsAdd function
* @endinternal
*
* @brief   Function Relevant mode : Low Level mode
*         add the ports to the trunk's non-trunk entry .
*         NOTE : the ports are add to the "non trunk" table only and not effect
*         other trunk relate tables/registers.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number .
* @param[in] trunkId                  - the trunk id - in this API  can be ZERO !
* @param[in] nonTrunkPortsBmpPtr      - (pointer to)bitmap of ports to add to
*                                      "non-trunk members"
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_INITIALIZED       -the trunk library was not initialized for the device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or
*                                       bad trunkId number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTrunkNonTrunkPortsAdd
(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              trunkId,
    IN CPSS_PORTS_BMP_STC       *nonTrunkPortsBmpPtr
)
{
    GT_STATUS   rc;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(nonTrunkPortsBmpPtr);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        /* check that the device supports all the physical ports that defined in the BMP */
        rc = prvCpssDxChTablePortsBmpCheck(devNum , nonTrunkPortsBmpPtr);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    return prvCpssGenericTrunkNonTrunkPortsAdd(devNum,trunkId,nonTrunkPortsBmpPtr);
}

/**
* @internal cpssDxChTrunkNonTrunkPortsAdd function
* @endinternal
*
* @brief   Function Relevant mode : Low Level mode
*         add the ports to the trunk's non-trunk entry .
*         NOTE : the ports are add to the "non trunk" table only and not effect
*         other trunk relate tables/registers.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number .
* @param[in] trunkId                  - the trunk id - in this API  can be ZERO !
* @param[in] nonTrunkPortsBmpPtr      - (pointer to)bitmap of ports to add to
*                                      "non-trunk members"
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_INITIALIZED       -the trunk library was not initialized for the device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or
*                                       bad trunkId number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTrunkNonTrunkPortsAdd
(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              trunkId,
    IN CPSS_PORTS_BMP_STC       *nonTrunkPortsBmpPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTrunkNonTrunkPortsAdd);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, trunkId, nonTrunkPortsBmpPtr));

    rc = internal_cpssDxChTrunkNonTrunkPortsAdd(devNum, trunkId, nonTrunkPortsBmpPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, trunkId, nonTrunkPortsBmpPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChTrunkNonTrunkPortsRemove function
* @endinternal
*
* @brief   Function Relevant mode : Low Level mode
*         Removes the ports from the trunk's non-trunk entry .
*         NOTE : the ports are removed from the "non trunk" table only and not
*         effect other trunk relate tables/registers.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number .
* @param[in] trunkId                  - the trunk id - in this API  can be ZERO !
* @param[in] nonTrunkPortsBmpPtr      - (pointer to)bitmap of ports to remove from
*                                      "non-trunk members"
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_INITIALIZED       -the trunk library was not initialized for the device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or
*                                       bad trunkId number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTrunkNonTrunkPortsRemove
(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              trunkId,
    IN CPSS_PORTS_BMP_STC       *nonTrunkPortsBmpPtr
)
{
    GT_STATUS   rc;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(nonTrunkPortsBmpPtr);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        /* check that the device supports all the physical ports that defined in the BMP */
        rc = prvCpssDxChTablePortsBmpCheck(devNum , nonTrunkPortsBmpPtr);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    return prvCpssGenericTrunkNonTrunkPortsRemove(devNum,trunkId,nonTrunkPortsBmpPtr);
}

/**
* @internal cpssDxChTrunkNonTrunkPortsRemove function
* @endinternal
*
* @brief   Function Relevant mode : Low Level mode
*         Removes the ports from the trunk's non-trunk entry .
*         NOTE : the ports are removed from the "non trunk" table only and not
*         effect other trunk relate tables/registers.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number .
* @param[in] trunkId                  - the trunk id - in this API  can be ZERO !
* @param[in] nonTrunkPortsBmpPtr      - (pointer to)bitmap of ports to remove from
*                                      "non-trunk members"
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_INITIALIZED       -the trunk library was not initialized for the device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or
*                                       bad trunkId number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTrunkNonTrunkPortsRemove
(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              trunkId,
    IN CPSS_PORTS_BMP_STC       *nonTrunkPortsBmpPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTrunkNonTrunkPortsRemove);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, trunkId, nonTrunkPortsBmpPtr));

    rc = internal_cpssDxChTrunkNonTrunkPortsRemove(devNum, trunkId, nonTrunkPortsBmpPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, trunkId, nonTrunkPortsBmpPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal sip5TrunkL2EcmpReservedNullPortSet function
* @endinternal
*
* @brief   Function Relevant mode : All modes
*         Set entry in L2 ECMP table with the 'NULL port' for the use of all
*         'empty trunks'.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] hwDevNum                 - HW device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on other error.
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS sip5TrunkL2EcmpReservedNullPortSet
(
    IN  GT_U8                                devNum,
    IN  GT_HW_DEV_NUM                        hwDevNum
)
{
    GT_U32  data;

    L2_ECMP_NULL_PORT_BOOKMARK
    /* we need extra index to be untouched (last index) to hold the NULL port */
    data = hwDevNum |
           (PRV_CPSS_DEV_TRUNK_INFO_MAC(devNum)->nullPort << 10);
    return prvCpssDxChWriteTableEntry(devNum,
        CPSS_DXCH_SIP5_TABLE_EQ_L2_ECMP_E,
        PRV_CPSS_DXCH_PP_MAC(devNum)->bridge.l2EcmpIndex_NULL_port,
        &data);
}

/**
* @internal internal_cpssDxChTrunkInit function
* @endinternal
*
* @brief   Function Relevant mode : All modes
*         CPSS DxCh Trunk initialization of PP Tables/registers and
*         SW shadow data structures, all ports are set as non-trunk ports.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] maxNumberOfTrunks        - maximum number of trunk groups.(0.."max number of trunks")
*                                      when this number is 0 , there will be no shadow used
*                                      Note:
*                                      that means that API's that used shadow will FAIL.
* @param[in] trunkMembersMode         - type of how the CPSS SW will fill the HW with
*                                      trunk members
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on other error.
* @retval GT_BAD_PARAM             - wrong devNum or bad trunkLbhMode
* @retval GT_OUT_OF_RANGE          - the numberOfTrunks > "max number of trunks"
* @retval GT_BAD_STATE             - if library already initialized with different
*                                       number of trunks than requested
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTrunkInit
(
    IN  GT_U8       devNum,
    IN  GT_U32      maxNumberOfTrunks,
    IN  CPSS_DXCH_TRUNK_MEMBERS_MODE_ENT trunkMembersMode
)
{
    GT_STATUS   rc;
    DXCH_TRUNK_SPECIFIC_INFO_STC    *extraTrunksInfoPtr;/*(pointer to)extra trunk info*/
    EXTRA_TRUNK_ENTRY_STC           *currWaTrunkInfoPtr;
    GT_U32  ii,jj;
    GT_U32      numRshiftBits;   /* number of bits for 'Right shift'         */
    PRV_CPSS_TRUNK_LOAD_BALANCE_TYPE_ENT    genericLbType;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    if(1 ==
        PRV_CPSS_DXCH_CHECK_TRUNK_WRONG_SOURCE_PORT_TO_CPU_WA_MAC(devNum))
    {
        if(PRV_CPSS_DXCH_PP_MAC(devNum)->errata.
            info_PRV_CPSS_DXCH_XCAT_TRUNK_WRONG_SOURCE_PORT_TO_CPU_WA_E.
            extraTrunksInfoPtr == NULL)
        {
            extraTrunksInfoPtr = cpssOsMalloc(sizeof(DXCH_TRUNK_SPECIFIC_INFO_STC));
            if(extraTrunksInfoPtr == NULL)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
            }

            cpssOsMemSet(extraTrunksInfoPtr,0,sizeof(DXCH_TRUNK_SPECIFIC_INFO_STC));

            PRV_CPSS_DXCH_PP_MAC(devNum)->errata.
                info_PRV_CPSS_DXCH_XCAT_TRUNK_WRONG_SOURCE_PORT_TO_CPU_WA_E.
                extraTrunksInfoPtr = extraTrunksInfoPtr;

            currWaTrunkInfoPtr = &extraTrunksInfoPtr->extraTrunkInfoArray[0];

            for(ii = 0;
                ii < XCAT_NUM_TRUNKS_FOR_WA_WRONG_SOURCE_PORT_TO_CPU;
                ii++ , currWaTrunkInfoPtr++)
            {
                for(jj = 0; jj < PRV_CPSS_TRUNK_8_MAX_NUM_OF_MEMBERS_CNS; jj++)
                {
                    currWaTrunkInfoPtr->errata.
                        info_WRONG_TRUNK_ID_SOURCE_PORT_INFORMATION_OF_PACKET_TO_CPU.
                            upToMemberMap[jj].port = INVALID_TRUNK_MEMBER_VALUE_CNS;

                    currWaTrunkInfoPtr->errata.
                        info_WRONG_TRUNK_ID_SOURCE_PORT_INFORMATION_OF_PACKET_TO_CPU.
                            upToMemberMap[jj].hwDevice = INVALID_TRUNK_MEMBER_VALUE_CNS;

                    /* set the UPs to use */
                    currWaTrunkInfoPtr->errata.
                        info_WRONG_TRUNK_ID_SOURCE_PORT_INFORMATION_OF_PACKET_TO_CPU.
                            upToUseArr[jj] = (GT_U8)jj;
                }
            }
        }
    }

    switch(trunkMembersMode)
    {
        case CPSS_DXCH_TRUNK_MEMBERS_MODE_NATIVE_E:
            genericLbType = PRV_CPSS_TRUNK_LOAD_BALANCE_NATIVE_E;
            break;
        case CPSS_DXCH_TRUNK_MEMBERS_MODE_SOHO_EMULATION_E:
            genericLbType = PRV_CPSS_TRUNK_LOAD_BALANCE_SX_EMULATION_E;
            break;
        case CPSS_DXCH_TRUNK_MEMBERS_MODE_FLEX_E:
            if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_FALSE)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "the Device not supports trunk 'flex' mode");
            }

            genericLbType = PRV_CPSS_TRUNK_LOAD_BALANCE_FLEX_E;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssGenericTrunkInit(devNum,maxNumberOfTrunks,genericLbType);
    if(rc != GT_OK)
    {
        return rc;
    }

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        /* this is 'basic hash' mode , and for CRC 6/16 */
        numRshiftBits = 6; /* 6 bits of hash - that comes from the TTI/PCL unit */

        /* set the number of right shift bits needed */
        /* this configuration may be changed by:
           cpssDxChTrunkHashNumBitsSet
           cpssDxChBrgL2EcmpMemberSelectionModeSet
           cpssDxChTrunkMemberSelectionModeSet
           cpssDxChTrunkHashGlobalModeSet
           cpssDxChTrunkHashCrcParametersSet
        */
        rc =  sip5HashNumBitsSetClient(devNum, 0 ,numRshiftBits);
        if(rc != GT_OK)
        {
            return rc;
        }

        /* NOTE:
           after cpss will support 'crc32' in cpssDxChTrunkHashCrcParametersSet
           we may change numRshiftBits to be 16 for this mode */


        /* set the device to support designatedTrunkTableSplit */
        /* <Separate MC UC For Designated> */
        rc = prvCpssHwPpSetRegField(devNum,
            PRV_DXCH_REG1_UNIT_EGF_SHT_MAC(devNum).global.SHTGlobalConfigs,
            5,1,1);
        if(rc != GT_OK)
        {
            return rc;
        }

        L2_ECMP_NULL_PORT_BOOKMARK
        /* we need extra index to be untouched (last index) to hold the NULL port */
        rc = sip5TrunkL2EcmpReservedNullPortSet(devNum,
            PRV_CPSS_HW_DEV_NUM_MAC(devNum));
        if(rc != GT_OK)
        {
            return rc;
        }

        if(genericLbType != PRV_CPSS_TRUNK_LOAD_BALANCE_FLEX_E)
        {
            /* achieve 'legacy mode' by 'hybrid mode' of 'All trunks with 8 members'
               (on expense of 1/2 of 'L2 ECMP' members table)*/
            rc = cpssDxChTrunkFlexInfoSet(devNum,
                TRUNK_ID_ALL_CNS/*hybrid mode indication*/,
                0,/*don't care*/
                PRV_CPSS_TRUNK_8_MAX_NUM_OF_MEMBERS_CNS);
            if(rc != GT_OK)
            {
                return rc;
            }
        }
    }


    return GT_OK;
}

/**
* @internal cpssDxChTrunkInit function
* @endinternal
*
* @brief   Function Relevant mode : All modes
*         CPSS DxCh Trunk initialization of PP Tables/registers and
*         SW shadow data structures, all ports are set as non-trunk ports.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] maxNumberOfTrunks        - maximum number of trunk groups.(0.."max number of trunks")
*                                      when this number is 0 , there will be no shadow used
*                                      Note:
*                                      that means that API's that used shadow will FAIL.
* @param[in] trunkMembersMode         - type of how the CPSS SW will fill the HW with
*                                      trunk members
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on other error.
* @retval GT_BAD_PARAM             - wrong devNum or bad trunkLbhMode
* @retval GT_OUT_OF_RANGE          - the numberOfTrunks > "max number of trunks"
* @retval GT_BAD_STATE             - if library already initialized with different
*                                       number of trunks than requested
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTrunkInit
(
    IN  GT_U8       devNum,
    IN  GT_U32      maxNumberOfTrunks,
    IN  CPSS_DXCH_TRUNK_MEMBERS_MODE_ENT trunkMembersMode
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTrunkInit);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, maxNumberOfTrunks, trunkMembersMode));

    rc = internal_cpssDxChTrunkInit(devNum, maxNumberOfTrunks, trunkMembersMode);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, maxNumberOfTrunks, trunkMembersMode));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTrunkDbInitInfoGet function
* @endinternal
*
* @brief   Function Relevant mode : All modes
*         Get the parameters that given during cpssDxChTrunkInit(...) .
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] maxNumberOfTrunksPtr     - (pointer to) maximum number of trunk groups.
*                                      if NULL - ignored
* @param[out] trunkMembersModePtr      - (pointer to) type of how the CPSS SW will fill the HW with
*                                      trunk members
*                                      if NULL - ignored
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on other error.
* @retval GT_BAD_PARAM             - wrong devNum.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTrunkDbInitInfoGet
(
    IN   GT_U8       devNum,
    OUT  GT_U32      *maxNumberOfTrunksPtr,
    OUT  CPSS_DXCH_TRUNK_MEMBERS_MODE_ENT *trunkMembersModePtr
)
{
    PRV_CPSS_TRUNK_DB_INFO_STC *devTrunkInfoPtr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    devTrunkInfoPtr = PRV_CPSS_DEV_TRUNK_INFO_MAC(devNum);
    if(devTrunkInfoPtr->initDone == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, "init was not done");
    }

    if(trunkMembersModePtr)
    {
        switch(devTrunkInfoPtr->loadBalanceType)
        {
            case PRV_CPSS_TRUNK_LOAD_BALANCE_NATIVE_E:
                *trunkMembersModePtr = CPSS_DXCH_TRUNK_MEMBERS_MODE_NATIVE_E;
                break;
            case PRV_CPSS_TRUNK_LOAD_BALANCE_SX_EMULATION_E:
                *trunkMembersModePtr = CPSS_DXCH_TRUNK_MEMBERS_MODE_SOHO_EMULATION_E;
                break;
            case PRV_CPSS_TRUNK_LOAD_BALANCE_FLEX_E:
                *trunkMembersModePtr = CPSS_DXCH_TRUNK_MEMBERS_MODE_FLEX_E;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "unknown DB mode [%d] \n",
                    devTrunkInfoPtr->loadBalanceType);
        }
    }

    if(maxNumberOfTrunksPtr)
    {
        *maxNumberOfTrunksPtr =  devTrunkInfoPtr->numberOfTrunks;
    }


    return GT_OK;
}

/**
* @internal cpssDxChTrunkDbInitInfoGet function
* @endinternal
*
* @brief   Function Relevant mode : All modes
*         Get the parameters that given during cpssDxChTrunkInit(...) .
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] maxNumberOfTrunksPtr     - (pointer to) maximum number of trunk groups.
*                                      if NULL - ignored
* @param[out] trunkMembersModePtr      - (pointer to) type of how the CPSS SW will fill the HW with
*                                      trunk members
*                                      if NULL - ignored
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on other error.
* @retval GT_BAD_PARAM             - wrong devNum.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTrunkDbInitInfoGet
(
    IN   GT_U8       devNum,
    OUT  GT_U32      *maxNumberOfTrunksPtr,
    OUT  CPSS_DXCH_TRUNK_MEMBERS_MODE_ENT *trunkMembersModePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTrunkDbInitInfoGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, maxNumberOfTrunksPtr, trunkMembersModePtr));

    rc = internal_cpssDxChTrunkDbInitInfoGet(devNum, maxNumberOfTrunksPtr, trunkMembersModePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, maxNumberOfTrunksPtr, trunkMembersModePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvCpssDxChTrunkFamilyInit function
* @endinternal
*
* @brief   This function sets CPSS private DxCh family info about trunk behavior
*         that relate to the family and not to specific device.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devFamily                - device family.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - on failure.
* @retval GT_NOT_INITIALIZED       - the family was not initialized properly
*/
GT_STATUS prvCpssDxChTrunkFamilyInit(
    IN CPSS_PP_FAMILY_TYPE_ENT  devFamily
)
{
    PRV_CPSS_FAMILY_TRUNK_BIND_FUNC_STC *bindFuncPtr;

    if(prvCpssFamilyInfoArray[devFamily] == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    bindFuncPtr = &prvCpssFamilyInfoArray[devFamily]->trunkInfo.boundFunc;

    bindFuncPtr->dbFlagsInitFunc             = trunkDbFlagsInit;

    bindFuncPtr->portTrunkIdSetFunc          = cpssDxChTrunkPortTrunkIdSet;
    bindFuncPtr->membersSetFunc              = cpssDxChTrunkTableEntrySet;
    bindFuncPtr->nonMembersBmpSetFunc        = cpssDxChTrunkNonTrunkPortsEntrySet;
    bindFuncPtr->nonMembersBmpGetFunc        = cpssDxChTrunkNonTrunkPortsEntryGet;
    bindFuncPtr->designatedMembersBmpSetFunc = cpssDxChTrunkDesignatedPortsEntrySet;
    bindFuncPtr->designatedMembersBmpGetFunc = cpssDxChTrunkDesignatedPortsEntryGet;

    return GT_OK;
}

/**
* @internal internal_cpssDxChTrunkHashMplsModeEnableSet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         Enable/Disable the device from considering the MPLS information,
*         when calculating the trunk hashing index for a packet.
*         Note:
*         1. Not relevant to NON-MPLS packets.
*         2. Relevant when cpssDxChTrunkHashGlobalModeSet(devNum,
*         CPSS_DXCH_TRUNK_LBH_PACKETS_INFO_E)
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - the device number.
* @param[in] enable                   - GT_FALSE - MPLS parameter are not used in trunk hash index
*                                      GT_TRUE  - The following function is added to the trunk load
*                                      balancing hash:
*                                      MPLSTrunkHash = (mpls_label0[5:0] & mpls_trunk_lbl0_mask) ^
*                                      (mpls_label1[5:0] & mpls_trunk_lbl1_mask) ^
*                                      (mpls_label2[5:0] & mpls_trunk_lbl2_mask)
*                                      NOTE:
*                                      If any of MPLS Labels 0:2 do not exist in the packet,
*                                      the default value 0x0 is used for TrunkHash calculation
*                                      instead.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTrunkHashMplsModeEnableSet
(
    IN GT_U8   devNum,
    IN GT_BOOL enable
)
{
    GT_U32      regAddr;         /* register address                         */
    GT_U32      value;           /* value to write to register               */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr =
            PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).trunkHashSimpleHash.trunkHashConfigReg0;
    }
    else
    {
        regAddr =
        PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->ttiRegs.trunkHashCfg[0];
    }

    value = (enable == GT_TRUE) ? 1 : 0;

    return prvCpssHwPpSetRegField(
        devNum, regAddr, 6, 1, value);
}

/**
* @internal cpssDxChTrunkHashMplsModeEnableSet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         Enable/Disable the device from considering the MPLS information,
*         when calculating the trunk hashing index for a packet.
*         Note:
*         1. Not relevant to NON-MPLS packets.
*         2. Relevant when cpssDxChTrunkHashGlobalModeSet(devNum,
*         CPSS_DXCH_TRUNK_LBH_PACKETS_INFO_E)
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - the device number.
* @param[in] enable                   - GT_FALSE - MPLS parameter are not used in trunk hash index
*                                      GT_TRUE  - The following function is added to the trunk load
*                                      balancing hash:
*                                      MPLSTrunkHash = (mpls_label0[5:0] & mpls_trunk_lbl0_mask) ^
*                                      (mpls_label1[5:0] & mpls_trunk_lbl1_mask) ^
*                                      (mpls_label2[5:0] & mpls_trunk_lbl2_mask)
*                                      NOTE:
*                                      If any of MPLS Labels 0:2 do not exist in the packet,
*                                      the default value 0x0 is used for TrunkHash calculation
*                                      instead.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTrunkHashMplsModeEnableSet
(
    IN GT_U8   devNum,
    IN GT_BOOL enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTrunkHashMplsModeEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enable));

    rc = internal_cpssDxChTrunkHashMplsModeEnableSet(devNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChTrunkHashMplsModeEnableGet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         Get trunk MPLS hash mode
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - the device number.
*
* @param[out] enablePtr                - (pointer to)the MPLS hash mode.
*                                      GT_FALSE - MPLS parameter are not used in trunk hash index
*                                      GT_TRUE  - The following function is added to the trunk load
*                                      balancing hash:
*                                      MPLSTrunkHash = (mpls_label0[5:0] & mpls_trunk_lbl0_mask) ^
*                                      (mpls_label1[5:0] & mpls_trunk_lbl1_mask) ^
*                                      (mpls_label2[5:0] & mpls_trunk_lbl2_mask)
*                                      NOTE:
*                                      If any of MPLS Labels 0:2 do not exist in the packet,
*                                      the default value 0x0 is used for TrunkHash calculation
*                                      instead.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTrunkHashMplsModeEnableGet
(
    IN GT_U8   devNum,
    OUT GT_BOOL *enablePtr
)
{
    GT_STATUS   rc;              /* return code                              */
    GT_U32      regAddr;         /* register address                         */
    GT_U32      value;           /* value to write to register               */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E);

    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr =
            PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).trunkHashSimpleHash.trunkHashConfigReg0;
    }
    else
    {
        regAddr =
            PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->ttiRegs.trunkHashCfg[0];
    }

    rc = prvCpssHwPpGetRegField(devNum, regAddr, 6, 1, &value);

    *enablePtr = value ? GT_TRUE : GT_FALSE;

    return rc;
}

/**
* @internal cpssDxChTrunkHashMplsModeEnableGet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         Get trunk MPLS hash mode
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - the device number.
*
* @param[out] enablePtr                - (pointer to)the MPLS hash mode.
*                                      GT_FALSE - MPLS parameter are not used in trunk hash index
*                                      GT_TRUE  - The following function is added to the trunk load
*                                      balancing hash:
*                                      MPLSTrunkHash = (mpls_label0[5:0] & mpls_trunk_lbl0_mask) ^
*                                      (mpls_label1[5:0] & mpls_trunk_lbl1_mask) ^
*                                      (mpls_label2[5:0] & mpls_trunk_lbl2_mask)
*                                      NOTE:
*                                      If any of MPLS Labels 0:2 do not exist in the packet,
*                                      the default value 0x0 is used for TrunkHash calculation
*                                      instead.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTrunkHashMplsModeEnableGet
(
    IN GT_U8   devNum,
    OUT GT_BOOL *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTrunkHashMplsModeEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enablePtr));

    rc = internal_cpssDxChTrunkHashMplsModeEnableGet(devNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvCpssDxChTrunkHashMaskPositionGet function
* @endinternal
*
* @brief   Get trunk hash mask control register addres and bit position
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - the device number.
* @param[in] maskedField              - field to apply the mask on
*
* @param[out] regAddrPtr               - (pointer to) address of hash control register
* @param[out] fieldOffsetPtr           - (pointer to) offset of field in register in bits (APPLICABLE RANGES: 0..31)
* @param[out] fieldSizePtr             - (pointer to) size of field in register in bits (APPLICABLE RANGES: 0..31)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - bad regIndex
*/
static GT_STATUS prvCpssDxChTrunkHashMaskPositionGet
(
    IN  GT_U8                         devNum,
    IN CPSS_DXCH_TRUNK_LBH_MASK_ENT   maskedField,
    OUT GT_U32                        *regAddrPtr,
    OUT GT_U32                        *fieldOffsetPtr,
    OUT GT_U32                        *fieldSizePtr
)
{
    /* all mask are 6-bit sized */
    *fieldSizePtr   = 6;

    switch (maskedField)
    {
        case CPSS_DXCH_TRUNK_LBH_MASK_MAC_DA_E:
            if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
            {
                *regAddrPtr =
                    PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).trunkHashSimpleHash.trunkHashConfigReg1;
            }
            else
            {
                *regAddrPtr =
                    PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->ttiRegs.trunkHashCfg[1];
            }
            *fieldOffsetPtr = 6;
            break;
        case CPSS_DXCH_TRUNK_LBH_MASK_MAC_SA_E:
            if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
            {
                *regAddrPtr =
                    PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).trunkHashSimpleHash.trunkHashConfigReg1;
            }
            else
            {
                *regAddrPtr =
                    PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->ttiRegs.trunkHashCfg[1];
            }
            *fieldOffsetPtr = 0;
            break;
        case CPSS_DXCH_TRUNK_LBH_MASK_MPLS_LABEL0_E:
            if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
            {
                *regAddrPtr =
                    PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).trunkHashSimpleHash.trunkHashConfigReg2;
            }
            else
            {
                *regAddrPtr =
                    PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->ttiRegs.trunkHashCfg[2];
            }
            *fieldOffsetPtr = 0;
            break;
        case CPSS_DXCH_TRUNK_LBH_MASK_MPLS_LABEL1_E:
            if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
            {
                *regAddrPtr =
                    PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).trunkHashSimpleHash.trunkHashConfigReg2;
            }
            else
            {
                *regAddrPtr =
                    PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->ttiRegs.trunkHashCfg[2];
            }
            *fieldOffsetPtr = 6;
            break;
        case CPSS_DXCH_TRUNK_LBH_MASK_MPLS_LABEL2_E:
            if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
            {
                *regAddrPtr =
                    PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).trunkHashSimpleHash.trunkHashConfigReg2;
            }
            else
            {
                *regAddrPtr =
                    PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->ttiRegs.trunkHashCfg[2];
            }
            *fieldOffsetPtr = 12;
            break;
        case CPSS_DXCH_TRUNK_LBH_MASK_IPV4_DIP_E:
            if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
            {
                *regAddrPtr =
                    PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).trunkHashSimpleHash.trunkHashConfigReg1;
            }
            else
            {
                *regAddrPtr =
                    PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->ttiRegs.trunkHashCfg[1];
            }
            *fieldOffsetPtr = 18;
            break;
        case CPSS_DXCH_TRUNK_LBH_MASK_IPV4_SIP_E:
            if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
            {
                *regAddrPtr =
                    PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).trunkHashSimpleHash.trunkHashConfigReg1;
            }
            else
            {
                *regAddrPtr =
                    PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->ttiRegs.trunkHashCfg[1];
            }
            *fieldOffsetPtr = 12;
            break;
        case CPSS_DXCH_TRUNK_LBH_MASK_IPV6_DIP_E:
            if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
            {
                *regAddrPtr =
                    PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).trunkHashSimpleHash.trunkHashConfigReg0;
            }
            else
            {
                *regAddrPtr =
                    PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->ttiRegs.trunkHashCfg[0];
            }
            *fieldOffsetPtr = 14;
            break;
        case CPSS_DXCH_TRUNK_LBH_MASK_IPV6_SIP_E:
            if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
            {
                *regAddrPtr =
                    PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).trunkHashSimpleHash.trunkHashConfigReg0;
            }
            else
            {
                *regAddrPtr =
                    PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->ttiRegs.trunkHashCfg[0];
            }
            *fieldOffsetPtr = 8;
            break;
        case CPSS_DXCH_TRUNK_LBH_MASK_IPV6_FLOW_E:
            if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
            {
                *regAddrPtr =
                    PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).trunkHashSimpleHash.trunkHashConfigReg1;
            }
            else
            {
                *regAddrPtr =
                    PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->ttiRegs.trunkHashCfg[1];
            }
            *fieldOffsetPtr = 24;
            break;
        case CPSS_DXCH_TRUNK_LBH_MASK_L4_DST_PORT_E:
            if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
            {
                *regAddrPtr =
                    PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).trunkHashSimpleHash.trunkHashConfigReg2;
            }
            else
            {
                *regAddrPtr =
                    PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->ttiRegs.trunkHashCfg[2];
            }
            *fieldOffsetPtr = 24;
            break;
        case CPSS_DXCH_TRUNK_LBH_MASK_L4_SRC_PORT_E:
            if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
            {
                *regAddrPtr =
                    PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).trunkHashSimpleHash.trunkHashConfigReg2;
            }
            else
            {
                *regAddrPtr =
                    PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->ttiRegs.trunkHashCfg[2];
            }
            *fieldOffsetPtr = 18;
            break;
        default : CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal internal_cpssDxChTrunkHashMaskSet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         Set the masks for the various packet fields being used at the Trunk
*         hash calculations
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - the device number.
* @param[in] maskedField              - field to apply the mask on
* @param[in] maskValue                - The mask value to be used (APPLICABLE RANGES: 0..0x3F)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or maskedField
* @retval GT_OUT_OF_RANGE          - maskValue > 0x3F
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTrunkHashMaskSet
(
    IN GT_U8                          devNum,
    IN CPSS_DXCH_TRUNK_LBH_MASK_ENT   maskedField,
    IN GT_U8                          maskValue
)
{
    GT_STATUS rc;            /* return code                         */
    GT_U32    regAddr;       /* address of hash control register    */
    GT_U32    fieldOffset;   /* offset of field in register in bits */
    GT_U32    fieldSize;     /* size of field in register in bits   */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E);

    rc = prvCpssDxChTrunkHashMaskPositionGet(
        devNum, maskedField, &regAddr, &fieldOffset, &fieldSize);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (maskValue >= (GT_U8)(1 << fieldSize))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    return prvCpssHwPpSetRegField(
        devNum, regAddr, fieldOffset, fieldSize, maskValue);
}

/**
* @internal cpssDxChTrunkHashMaskSet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         Set the masks for the various packet fields being used at the Trunk
*         hash calculations
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - the device number.
* @param[in] maskedField              - field to apply the mask on
* @param[in] maskValue                - The mask value to be used (APPLICABLE RANGES: 0..0x3F)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or maskedField
* @retval GT_OUT_OF_RANGE          - maskValue > 0x3F
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTrunkHashMaskSet
(
    IN GT_U8                          devNum,
    IN CPSS_DXCH_TRUNK_LBH_MASK_ENT   maskedField,
    IN GT_U8                          maskValue
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTrunkHashMaskSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, maskedField, maskValue));

    rc = internal_cpssDxChTrunkHashMaskSet(devNum, maskedField, maskValue);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, maskedField, maskValue));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTrunkHashMaskGet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         Get the masks for the various packet fields being used at the Trunk
*         hash calculations
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - the device number.
* @param[in] maskedField              - field to apply the mask on
*
* @param[out] maskValuePtr             - (pointer to)The mask value to be used (APPLICABLE RANGES: 0..0x3F)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or maskedField
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTrunkHashMaskGet
(
    IN GT_U8                          devNum,
    IN CPSS_DXCH_TRUNK_LBH_MASK_ENT   maskedField,
    OUT GT_U8                         *maskValuePtr
)
{
    GT_STATUS rc;            /* return code                         */
    GT_U32    regAddr;       /* address of hash control register    */
    GT_U32    fieldOffset;   /* offset of field in register in bits */
    GT_U32    fieldSize;     /* size of field in register in bits   */
    GT_U32    value;         /* field value                         */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E);

    CPSS_NULL_PTR_CHECK_MAC(maskValuePtr);

    rc = prvCpssDxChTrunkHashMaskPositionGet(
        devNum, maskedField, &regAddr, &fieldOffset, &fieldSize);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssHwPpGetRegField(
        devNum, regAddr, fieldOffset, fieldSize, &value);

    *maskValuePtr = (GT_U8)value;

    return rc;
}

/**
* @internal cpssDxChTrunkHashMaskGet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         Get the masks for the various packet fields being used at the Trunk
*         hash calculations
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - the device number.
* @param[in] maskedField              - field to apply the mask on
*
* @param[out] maskValuePtr             - (pointer to)The mask value to be used (APPLICABLE RANGES: 0..0x3F)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or maskedField
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTrunkHashMaskGet
(
    IN GT_U8                          devNum,
    IN CPSS_DXCH_TRUNK_LBH_MASK_ENT   maskedField,
    OUT GT_U8                         *maskValuePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTrunkHashMaskGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, maskedField, maskValuePtr));

    rc = internal_cpssDxChTrunkHashMaskGet(devNum, maskedField, maskValuePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, maskedField, maskValuePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvCpssDxChTrunkHashShiftPositionGet function
* @endinternal
*
* @brief   Get trunk hash shift control register addres and bit position
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - the device number.
* @param[in] protocolStack            - Set the shift to either IPv4 or IPv6 IP addresses.
* @param[in] isSrcIp                  - GT_TRUE  = Set the shift to IPv4/6 source addresses.
*                                      GT_FALSE = Set the shift to IPv4/6 destination addresses.
*
* @param[out] regAddrPtr               - (pointer to) address of hash control register
* @param[out] fieldOffsetPtr           - (pointer to) offset of field in register in bits (APPLICABLE RANGES: 0..31)
* @param[out] fieldSizePtr             - (pointer to) size of field in register in bits (APPLICABLE RANGES: 0..31)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - bad regIndex
*/
static GT_STATUS prvCpssDxChTrunkHashShiftPositionGet
(
    IN  GT_U8                         devNum,
    IN CPSS_IP_PROTOCOL_STACK_ENT     protocolStack,
    IN GT_BOOL                        isSrcIp,
    OUT GT_U32                        *regAddrPtr,
    OUT GT_U32                        *fieldOffsetPtr,
    OUT GT_U32                        *fieldSizePtr
)
{
    /* all shiftes are in the same register */
    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        *regAddrPtr =
            PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).trunkHashSimpleHash.trunkHashConfigReg0;
    }
    else
    {
        *regAddrPtr =
            PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->ttiRegs.trunkHashCfg[0];
    }

    switch (protocolStack)
    {
        case CPSS_IP_PROTOCOL_IPV6_E:
            *fieldSizePtr   = 4;
            *fieldOffsetPtr = (isSrcIp == GT_FALSE) ? 24 : 28;
            break;
        case CPSS_IP_PROTOCOL_IPV4_E:
            *fieldSizePtr   = 2;
            *fieldOffsetPtr = (isSrcIp == GT_FALSE) ? 20 : 22;
            break;
        default : CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal internal_cpssDxChTrunkHashIpShiftSet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         Set the shift being done to IP addresses prior to hash calculations.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - the device number.
* @param[in] protocolStack            - Set the shift to either IPv4 or IPv6 IP addresses.
* @param[in] isSrcIp                  - GT_TRUE  = Set the shift to IPv4/6 source addresses.
*                                      GT_FALSE = Set the shift to IPv4/6 destination addresses.
* @param[in] shiftValue               - The shift to be done.
*                                      IPv4 valid shift: (APPLICABLE RANGES: 0..3) bytes (Value = 0: no shift).
*                                      IPv6 valid shift: (APPLICABLE RANGES: 0..15) bytes (Value = 0: no shift).
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or protocolStack
* @retval GT_OUT_OF_RANGE          - shiftValue > 3 for IPv4 , shiftValue > 15 for IPv6
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTrunkHashIpShiftSet
(
    IN GT_U8                        devNum,
    IN CPSS_IP_PROTOCOL_STACK_ENT   protocolStack,
    IN GT_BOOL                      isSrcIp,
    IN GT_U32                       shiftValue
)
{
    GT_STATUS rc;            /* return code                         */
    GT_U32    regAddr;       /* address of hash control register    */
    GT_U32    fieldOffset;   /* offset of field in register in bits */
    GT_U32    fieldSize;     /* size of field in register in bits   */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E);

    rc = prvCpssDxChTrunkHashShiftPositionGet(
        devNum, protocolStack, isSrcIp, &regAddr, &fieldOffset, &fieldSize);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (shiftValue >= (GT_U32)(1 << fieldSize))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    return prvCpssHwPpSetRegField(
        devNum, regAddr, fieldOffset, fieldSize, shiftValue);
}

/**
* @internal cpssDxChTrunkHashIpShiftSet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         Set the shift being done to IP addresses prior to hash calculations.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - the device number.
* @param[in] protocolStack            - Set the shift to either IPv4 or IPv6 IP addresses.
* @param[in] isSrcIp                  - GT_TRUE  = Set the shift to IPv4/6 source addresses.
*                                      GT_FALSE = Set the shift to IPv4/6 destination addresses.
* @param[in] shiftValue               - The shift to be done.
*                                      IPv4 valid shift: (APPLICABLE RANGES: 0..3) bytes (Value = 0: no shift).
*                                      IPv6 valid shift: (APPLICABLE RANGES: 0..15) bytes (Value = 0: no shift).
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or protocolStack
* @retval GT_OUT_OF_RANGE          - shiftValue > 3 for IPv4 , shiftValue > 15 for IPv6
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTrunkHashIpShiftSet
(
    IN GT_U8                        devNum,
    IN CPSS_IP_PROTOCOL_STACK_ENT   protocolStack,
    IN GT_BOOL                      isSrcIp,
    IN GT_U32                       shiftValue
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTrunkHashIpShiftSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, protocolStack, isSrcIp, shiftValue));

    rc = internal_cpssDxChTrunkHashIpShiftSet(devNum, protocolStack, isSrcIp, shiftValue);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, protocolStack, isSrcIp, shiftValue));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTrunkHashIpShiftGet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         Get the shift being done to IP addresses prior to hash calculations.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - the device number.
* @param[in] protocolStack            - Set the shift to either IPv4 or IPv6 IP addresses.
* @param[in] isSrcIp                  - GT_TRUE  = Set the shift to IPv4/6 source addresses.
*                                      GT_FALSE = Set the shift to IPv4/6 destination addresses.
*
* @param[out] shiftValuePtr            - (pointer to) The shift to be done.
*                                      IPv4 valid shift: (APPLICABLE RANGES: 0..3) bytes (Value = 0: no shift).
*                                      IPv6 valid shift: (APPLICABLE RANGES: 0..15) bytes (Value = 0: no shift).
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or protocolStack
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTrunkHashIpShiftGet
(
    IN GT_U8                        devNum,
    IN CPSS_IP_PROTOCOL_STACK_ENT   protocolStack,
    IN GT_BOOL                      isSrcIp,
    OUT GT_U32                      *shiftValuePtr
)
{
    GT_STATUS rc;            /* return code                         */
    GT_U32    regAddr;       /* address of hash control register    */
    GT_U32    fieldOffset;   /* offset of field in register in bits */
    GT_U32    fieldSize;     /* size of field in register in bits   */
    GT_U32    value;         /* field value                         */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E);

    CPSS_NULL_PTR_CHECK_MAC(shiftValuePtr);

    rc = prvCpssDxChTrunkHashShiftPositionGet(
        devNum, protocolStack, isSrcIp, &regAddr, &fieldOffset, &fieldSize);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssHwPpGetRegField(
        devNum, regAddr, fieldOffset, fieldSize, &value);

    *shiftValuePtr = value;

    return rc;
}

/**
* @internal cpssDxChTrunkHashIpShiftGet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         Get the shift being done to IP addresses prior to hash calculations.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - the device number.
* @param[in] protocolStack            - Set the shift to either IPv4 or IPv6 IP addresses.
* @param[in] isSrcIp                  - GT_TRUE  = Set the shift to IPv4/6 source addresses.
*                                      GT_FALSE = Set the shift to IPv4/6 destination addresses.
*
* @param[out] shiftValuePtr            - (pointer to) The shift to be done.
*                                      IPv4 valid shift: (APPLICABLE RANGES: 0..3) bytes (Value = 0: no shift).
*                                      IPv6 valid shift: (APPLICABLE RANGES: 0..15) bytes (Value = 0: no shift).
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or protocolStack
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTrunkHashIpShiftGet
(
    IN GT_U8                        devNum,
    IN CPSS_IP_PROTOCOL_STACK_ENT   protocolStack,
    IN GT_BOOL                      isSrcIp,
    OUT GT_U32                      *shiftValuePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTrunkHashIpShiftGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, protocolStack, isSrcIp, shiftValuePtr));

    rc = internal_cpssDxChTrunkHashIpShiftGet(devNum, protocolStack, isSrcIp, shiftValuePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, protocolStack, isSrcIp, shiftValuePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvCpssDxChTrunkHwDevNumSet function
* @endinternal
*
* @brief   Update trunk logic prior change of HW device number.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] hwDevNum                 - HW device number
*                                      NOTE: at this stage the PRV_CPSS_HW_DEV_NUM_MAC(devNum) hold
*                                      the 'old hwDevNum' !!!
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvCpssDxChTrunkHwDevNumSet
(
    IN GT_U8            devNum,
    IN GT_HW_DEV_NUM    hwDevNum
)
{
    GT_U32          value;/*value to write to register */
    GT_U32          port;/* port index*/
    GT_U32          additionalInfoBmp; /* bitmap for additional WA info */
    GT_U32          regAddr; /* base address */
    GT_HW_DEV_NUM   oldHwDevNum;/* hw dev num before the change to 'hwDevNum'*/
    GT_STATUS       rc;

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        if(PRV_CPSS_DXCH_PP_MAC(devNum)->bridge.l2EcmpIndex_NULL_port)
        {
            L2_ECMP_NULL_PORT_BOOKMARK
            /* we need extra index to be untouched (last index) to hold the NULL port */
            rc = sip5TrunkL2EcmpReservedNullPortSet(devNum,hwDevNum);
            if(rc != GT_OK)
            {
                return rc;
            }
        }
        else
        {
            /* the trunk init was not called yet ... so we not need to modify HW */
        }
    }

    if(0 ==
        PRV_CPSS_DXCH_CHECK_TRUNK_WRONG_SOURCE_PORT_TO_CPU_WA_MAC(devNum))
    {
        return  GT_OK;
    }

    oldHwDevNum = PRV_CPSS_HW_DEV_NUM_MAC(devNum);

    if(oldHwDevNum == hwDevNum)
    {
        return GT_OK;
    }

    additionalInfoBmp = PRV_CPSS_DXCH_PP_MAC(devNum)->errata.
        info_PRV_CPSS_DXCH_XCAT_TRUNK_WRONG_SOURCE_PORT_TO_CPU_WA_E.
        additionalInfoBmp;

    if(((additionalInfoBmp >> 1) & 0x1) == 0)
    {
        return GT_OK;
    }

    if (((additionalInfoBmp >> 2) & 0x1) == 0)
    {
        regAddr = 0x0f808000;
    }
    else
    {
        regAddr = 0x0f80A000;
    }

    for(port = 0 ; port < 64 ; port++ , regAddr+=4)
    {

        rc = prvCpssHwPpReadRegister(devNum,
                                    regAddr + oldHwDevNum*256 ,&value);
        if(rc !=GT_OK)
        {
            return rc;
        }
        rc = prvCpssHwPpWriteRegister(devNum,
                                    regAddr + hwDevNum*256 ,value);
        if(rc !=GT_OK)
        {
            return rc;
        }
        /* invalidate old entries */
        rc = prvCpssHwPpWriteRegister(devNum,
                                    regAddr + oldHwDevNum*256 ,0);
        if(rc !=GT_OK)
        {
            return rc;
        }

    }

    return GT_OK;
}


/**
* @internal internal_cpssDxChTrunkHashCrcParametersSet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         Set the CRC hash parameter , relevant for CPSS_DXCH_TRUNK_LBH_PACKETS_INFO_CRC_E .
*
* @note   APPLICABLE DEVICES:      Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3.
*
* @param[in] devNum                   - The device number.
* @param[in] crcMode                  - The CRC mode .
* @param[in] crcSeed                  - The seed used by the CRC computation .
*                                      when crcMode is CRC_6 mode : crcSeed (APPLICABLE RANGES: 0..0x3f) (6 bits value)
*                                      when crcMode is CRC_16 mode : crcSeed (APPLICABLE RANGES: 0..0xffff) (16 bits value)
*                                      when crcMode is CRC_32 mode : crcSeed (32 bits value)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or crcMode
* @retval GT_OUT_OF_RANGE          - crcSeed out of range.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note related to feature 'CRC hash mode'
*
*/
static GT_STATUS internal_cpssDxChTrunkHashCrcParametersSet
(
    IN GT_U8                                devNum,
    IN CPSS_DXCH_TRUNK_LBH_CRC_MODE_ENT     crcMode,
    IN GT_U32                               crcSeed
)
{
    GT_STATUS   rc;
    GT_U32    regAddr;       /* register address    */
    GT_U32    value;         /* value to write to the register */
    GT_U32    mask;          /* mask the bits to write to the register */
    GT_U32    mask1 ;        /* mask1 the bits to write to the register */
    GT_U32    value1;        /* value1 to write to the register */
    GT_U32    numRshiftBits;   /* number of bits for 'Right shift'         */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_FALSE)
    {
        mask1 = 0;  /* value 0 to support generic code */
        value1= 0;  /* value 0 to support generic code */
    }
    else
    {
        mask1 = BIT_3;/* <Hash Function Selection> (6 or 32 bits hash) */
        value1= 0;
    }


    switch(crcMode)
    {
        case CPSS_DXCH_TRUNK_LBH_CRC_6_MODE_E:
            if(crcSeed >= BIT_6)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
            }

            mask  = (GT_U32)(BIT_1 | (0x3F << 8));/* bit 1 and bits 8..13 */
            value = BIT_1 | crcSeed << 8;/* bit 1 with value 1 , bits 8..13 with value of seed */

            break;
        case CPSS_DXCH_TRUNK_LBH_CRC_16_AND_PEARSON_MODE_E:
            if(crcSeed >= BIT_16)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
            }

            mask  = (GT_U32)(BIT_1 | (0xFFFF << 16));/* bit 1 and bits 16..31 */
            value = crcSeed << 16; /* bit 1 with value 0 , bits 16..31 with value of seed */
            break;
        case CPSS_DXCH_TRUNK_LBH_CRC_32_MODE_E:
            if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_FALSE)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            mask  = 0;
            value = 0;

            value1 = BIT_3;/* set to 1 <Hash Function Selection> */
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        if(crcMode == CPSS_DXCH_TRUNK_LBH_CRC_32_MODE_E)
        {
            regAddr = PRV_DXCH_REG1_UNIT_PCL_MAC(devNum).HASH.CRC32Seed;
            rc = prvCpssHwPpWriteRegister(devNum,regAddr,crcSeed);
            if(rc != GT_OK)
            {
                return rc;
            }

            numRshiftBits = SIP5_LFSR_MAX_12_BITS_CNS;
        }
        else
        {
            /* this is 'basic hash' mode , and for CRC 6/16 */
            numRshiftBits = SIP5_LFSR_LEGACY_6_BITS_CNS; /* 6 bits of hash - that comes from the TTI/PCL unit */
        }

        /* set the number of right shift bits needed */
        /* this configuration may be changed by:
           cpssDxChTrunkHashNumBitsSet
           cpssDxChBrgL2EcmpMemberSelectionModeSet
           cpssDxChTrunkMemberSelectionModeSet
           cpssDxChTrunkInit
           cpssDxChTrunkHashGlobalModeSet
        */
        rc = sip5HashNumBitsSetClient(devNum, 0 , numRshiftBits);
        if(rc != GT_OK)
        {
            return rc;
        }

        regAddr =
            PRV_DXCH_REG1_UNIT_PCL_MAC(devNum).HASH.CRCHashConfig;

        /* combine the values and masks */
        value |= value1;
        mask  |= mask1;

    }
    else
    {
        regAddr =
            PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->pclRegs.crcHashConfigurationReg;
    }

    return prvCpssHwPpWriteRegBitMask(devNum, regAddr, mask, value);
}

/**
* @internal cpssDxChTrunkHashCrcParametersSet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         Set the CRC hash parameter , relevant for CPSS_DXCH_TRUNK_LBH_PACKETS_INFO_CRC_E .
*
* @note   APPLICABLE DEVICES:      Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3.
*
* @param[in] devNum                   - The device number.
* @param[in] crcMode                  - The CRC mode .
* @param[in] crcSeed                  - The seed used by the CRC computation .
*                                      when crcMode is CRC_6 mode : crcSeed (APPLICABLE RANGES: 0..0x3f) (6 bits value)
*                                      when crcMode is CRC_16 mode : crcSeed (APPLICABLE RANGES: 0..0xffff) (16 bits value)
*                                      when crcMode is CRC_32 mode : crcSeed (32 bits value)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or crcMode
* @retval GT_OUT_OF_RANGE          - crcSeed out of range.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note related to feature 'CRC hash mode'
*
*/
GT_STATUS cpssDxChTrunkHashCrcParametersSet
(
    IN GT_U8                                devNum,
    IN CPSS_DXCH_TRUNK_LBH_CRC_MODE_ENT     crcMode,
    IN GT_U32                               crcSeed
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTrunkHashCrcParametersSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, crcMode, crcSeed));

    rc = internal_cpssDxChTrunkHashCrcParametersSet(devNum, crcMode, crcSeed);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, crcMode, crcSeed));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTrunkHashCrcParametersGet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         Get the CRC hash parameter , relevant for CPSS_DXCH_TRUNK_LBH_PACKETS_INFO_CRC_E .
*
* @note   APPLICABLE DEVICES:      Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3.
*
* @param[in] devNum                   - The device number.
*
* @param[out] crcModePtr               - (pointer to) The CRC mode .
* @param[out] crcSeedPtr               - (pointer to) The seed used by the CRC computation .
*                                      when crcMode is CRC_6 mode : crcSeed  (6 bits value)
*                                      when crcMode is CRC_16 mode : crcSeed (16 bits value)
*                                      when crcMode is CRC_32 mode : crcSeed (32 bits value)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note related to feature 'CRC hash mode'
*
*/
static GT_STATUS internal_cpssDxChTrunkHashCrcParametersGet
(
    IN GT_U8                                devNum,
    OUT CPSS_DXCH_TRUNK_LBH_CRC_MODE_ENT     *crcModePtr,
    OUT GT_U32                               *crcSeedPtr
)
{
    GT_STATUS rc;            /* return code         */
    GT_U32    regAddr;       /* register address    */
    GT_U32    value;         /* value read from the register */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E);

    CPSS_NULL_PTR_CHECK_MAC(crcModePtr);
    CPSS_NULL_PTR_CHECK_MAC(crcSeedPtr);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr =
            PRV_DXCH_REG1_UNIT_PCL_MAC(devNum).HASH.CRCHashConfig;
    }
    else
    {
        regAddr =
            PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->pclRegs.crcHashConfigurationReg;
    }
    rc = prvCpssHwPpReadRegister(devNum, regAddr, &value);
    if(rc != GT_OK)
    {
        return rc;
    }

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE &&
       U32_GET_FIELD_MAC(value,3,1) /*<Hash Function Selection>*/ )
    {
        *crcModePtr = CPSS_DXCH_TRUNK_LBH_CRC_32_MODE_E;
        regAddr = PRV_DXCH_REG1_UNIT_PCL_MAC(devNum).HASH.CRC32Seed;
        rc = prvCpssHwPpReadRegister(devNum,regAddr,crcSeedPtr);
        if(rc != GT_OK)
        {
            return rc;
        }
    }
    else
    if(U32_GET_FIELD_MAC(value,1,1))  /* CRC Hash Sub-Mode */
    {
        *crcModePtr = CPSS_DXCH_TRUNK_LBH_CRC_6_MODE_E;
        *crcSeedPtr = U32_GET_FIELD_MAC(value,8,6);
    }
    else
    {
        *crcModePtr = CPSS_DXCH_TRUNK_LBH_CRC_16_AND_PEARSON_MODE_E;
        *crcSeedPtr = U32_GET_FIELD_MAC(value,16,16);
    }

    return GT_OK;
}

/**
* @internal cpssDxChTrunkHashCrcParametersGet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         Get the CRC hash parameter , relevant for CPSS_DXCH_TRUNK_LBH_PACKETS_INFO_CRC_E .
*
* @note   APPLICABLE DEVICES:      Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3.
*
* @param[in] devNum                   - The device number.
*
* @param[out] crcModePtr               - (pointer to) The CRC mode .
* @param[out] crcSeedPtr               - (pointer to) The seed used by the CRC computation .
*                                      when crcMode is CRC_6 mode : crcSeed  (6 bits value)
*                                      when crcMode is CRC_16 mode : crcSeed (16 bits value)
*                                      when crcMode is CRC_32 mode : crcSeed (32 bits value)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note related to feature 'CRC hash mode'
*
*/
GT_STATUS cpssDxChTrunkHashCrcParametersGet
(
    IN GT_U8                                devNum,
    OUT CPSS_DXCH_TRUNK_LBH_CRC_MODE_ENT     *crcModePtr,
    OUT GT_U32                               *crcSeedPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTrunkHashCrcParametersGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, crcModePtr, crcSeedPtr));

    rc = internal_cpssDxChTrunkHashCrcParametersGet(devNum, crcModePtr, crcSeedPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, crcModePtr, crcSeedPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTrunkPortHashMaskInfoSet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         Set port-based hash mask info.
*
* @note   APPLICABLE DEVICES:      Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3.
*
* @param[in] devNum                   - The device number.
* @param[in] portNum                  - The port number.
* @param[in] overrideEnable           - enable/disable the override
* @param[in] index                    - the  to use when 'Override enabled'.
*                                      (APPLICABLE RANGES: 0..15) , relevant only when overrideEnable = GT_TRUE
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or portNum
* @retval GT_OUT_OF_RANGE          - when overrideEnable is enabled and index out of range.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note related to feature 'CRC hash mode'
*
*/
static GT_STATUS internal_cpssDxChTrunkPortHashMaskInfoSet
(
    IN GT_U8        devNum,
    IN GT_PORT_NUM  portNum,
    IN GT_BOOL      overrideEnable,
    IN GT_U32       index
)
{
    GT_STATUS rc = GT_OK;
    GT_U32    value;         /* value to write to the register */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E);

    if(overrideEnable == GT_TRUE)
    {
        if(index >= BIT_4)
        {
            /* 4 bits in HW */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
        }

        /* set the index, enable the override bit */
        value = BIT_0 | index << 1;
    }
    else
    {
        /* set the index = 0, disable the override bit */
        value = 0;
    }

    TRUNK_PORT_CHECK_MAC(devNum,portNum);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        /* write to pre-tti-lookup-ingress-eport table */
         rc = prvCpssDxChWriteTableEntryField(devNum,
                                            CPSS_DXCH_SIP5_TABLE_POST_TTI_LOOKUP_INGRESS_EPORT_E,
                                            portNum,
                                            PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                            SIP5_TTI_EPORT_ATTRIBUTES_TABLE_FIELDS_OVERRIDE_MASK_HASH_EN_E, /* field name */
                                            2,/* 2 fields */
                                            value);
    }
    else
    {
        /* trunk id of port control it accessed via interface to :
           Port<n> VLAN and QoS Configuration Entry
           bit 66 , 67..70
        */
        rc = prvCpssDxChWriteTableEntryField(devNum,
                                            CPSS_DXCH_TABLE_PORT_VLAN_QOS_E,
                                            portNum,
                                            PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,/* global bit */
                                            66,/* start bit 66 , 67..70 */
                                            5,/* 5 bits */
                                            value);
    }
    return rc;
}

/**
* @internal cpssDxChTrunkPortHashMaskInfoSet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         Set port-based hash mask info.
*
* @note   APPLICABLE DEVICES:      Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3.
*
* @param[in] devNum                   - The device number.
* @param[in] portNum                  - The port number.
* @param[in] overrideEnable           - enable/disable the override
* @param[in] index                    - the  to use when 'Override enabled'.
*                                      (APPLICABLE RANGES: 0..15) , relevant only when overrideEnable = GT_TRUE
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or portNum
* @retval GT_OUT_OF_RANGE          - when overrideEnable is enabled and index out of range.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note related to feature 'CRC hash mode'
*
*/
GT_STATUS cpssDxChTrunkPortHashMaskInfoSet
(
    IN GT_U8        devNum,
    IN GT_PORT_NUM  portNum,
    IN GT_BOOL      overrideEnable,
    IN GT_U32       index
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTrunkPortHashMaskInfoSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, overrideEnable, index));

    rc = internal_cpssDxChTrunkPortHashMaskInfoSet(devNum, portNum, overrideEnable, index);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, overrideEnable, index));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTrunkPortHashMaskInfoGet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         Get port-based hash mask info.
*
* @note   APPLICABLE DEVICES:      Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3.
*
* @param[in] devNum                   - The device number.
* @param[in] portNum                  - The port number.
*
* @param[out] overrideEnablePtr        - (pointer to)enable/disable the override
* @param[out] indexPtr                 - (pointer to)the index to use when 'Override enabled'.
*                                      (APPLICABLE RANGES: 0..15) , relevant only when overrideEnable = GT_TRUE
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or portNum
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note related to feature 'CRC hash mode'
*
*/
static GT_STATUS internal_cpssDxChTrunkPortHashMaskInfoGet
(
    IN GT_U8        devNum,
    IN GT_PORT_NUM  portNum,
    OUT GT_BOOL     *overrideEnablePtr,
    OUT GT_U32      *indexPtr
)
{
    GT_STATUS rc=GT_OK;      /* return code         */
    GT_U32    value;         /* value read from the register */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E);

    CPSS_NULL_PTR_CHECK_MAC(overrideEnablePtr);
    CPSS_NULL_PTR_CHECK_MAC(indexPtr);

    TRUNK_PORT_CHECK_MAC(devNum,portNum);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
         /* read from post-tti-lookup-ingress-eport table */
        rc = prvCpssDxChReadTableEntryField(devNum,
                                            CPSS_DXCH_SIP5_TABLE_POST_TTI_LOOKUP_INGRESS_EPORT_E,
                                            portNum,
                                            PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                            SIP5_TTI_EPORT_ATTRIBUTES_TABLE_FIELDS_OVERRIDE_MASK_HASH_EN_E, /* field name */
                                            2,/* 2 fields */
                                            &value);
    }
    else
    {
        /* trunk id of port control it accessed via interface to :
           Port<n> VLAN and QoS Configuration Entry
           bit 66 , 67..70
        */
        rc = prvCpssDxChReadTableEntryField(devNum,
                                            CPSS_DXCH_TABLE_PORT_VLAN_QOS_E,
                                            portNum,
                                            PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,/* global bit */
                                            66,/* start bit 66 , 67..70 */
                                            5,/* 5 bits */
                                            &value);
    }

    if(value & 1)
    {
        *overrideEnablePtr = GT_TRUE;
    }
    else
    {
        *overrideEnablePtr = GT_FALSE;
    }

    *indexPtr = value >> 1;

    return rc;
}

/**
* @internal cpssDxChTrunkPortHashMaskInfoGet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         Get port-based hash mask info.
*
* @note   APPLICABLE DEVICES:      Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3.
*
* @param[in] devNum                   - The device number.
* @param[in] portNum                  - The port number.
*
* @param[out] overrideEnablePtr        - (pointer to)enable/disable the override
* @param[out] indexPtr                 - (pointer to)the index to use when 'Override enabled'.
*                                      (APPLICABLE RANGES: 0..15) , relevant only when overrideEnable = GT_TRUE
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or portNum
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note related to feature 'CRC hash mode'
*
*/
GT_STATUS cpssDxChTrunkPortHashMaskInfoGet
(
    IN GT_U8        devNum,
    IN GT_PORT_NUM  portNum,
    OUT GT_BOOL     *overrideEnablePtr,
    OUT GT_U32      *indexPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTrunkPortHashMaskInfoGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, overrideEnablePtr, indexPtr));

    rc = internal_cpssDxChTrunkPortHashMaskInfoGet(devNum, portNum, overrideEnablePtr, indexPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, overrideEnablePtr, indexPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal swap32BitsAndShift function
* @endinternal
*
* @brief   swap length bits value and right shift it back to bit 0
*
* @param[in] valueToSwap              - the value to swap
* @param[in] numOfBits                - number of bits in the value
*                                       the swapped value after right shifted back to start at bit 0.
*/
static GT_U32 swap32BitsAndShift(
    IN GT_U32   valueToSwap,
    IN GT_U32   numOfBits
)
{
    GT_U32  tmp; /* temp value*/
    GT_U32  swapped;/* the swapped value */

    tmp = valueToSwap >> 24;
    swapped  = (U8_SWAP_BITS(tmp)) << 0;
    tmp = valueToSwap >> 16;
    swapped |= (U8_SWAP_BITS(tmp)) << 8;
    tmp = valueToSwap >> 8;
    swapped |= (U8_SWAP_BITS(tmp)) << 16;
    tmp = valueToSwap >> 0;
    swapped |= (U8_SWAP_BITS(tmp)) << 24;

    return swapped >> (32-numOfBits);
}

/**
* @internal hashMaskFieldDestinationGet function
* @endinternal
*
* @brief   get fields distance and number info and pointer to first field
*
* @param[in] devNum                   - the device number.
* @param[in] entryPtr                 - (pointer to) entry of type CPSS_DXCH_TRUNK_LBH_CRC_MASK_ENTRY_STC.
*
* @param[out] numOfFieldsPtr           - (pointer to) the number of GT_U32 fields in entryPtr
* @param[out] fieldsDestinationPtr     - (pointer to) the distance between 2 fields in entryPtr (value is in GT_U32 steps)
* @param[out] fieldPtrPtr              -(pointer to) pointer to first GT_U32 field in  entryPtr
*                                       none
*/
static void hashMaskFieldDestinationGet
(
    IN GT_U8        devNum,
    IN CPSS_DXCH_TRUNK_LBH_CRC_MASK_ENTRY_STC *entryPtr,
    OUT GT_U32  *numOfFieldsPtr,
    OUT GT_U32  *fieldsDestinationPtr,
    OUT GT_U32  **fieldPtrPtr
)
{

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        *fieldsDestinationPtr =
            (sip5_hashMaskCrcFieldsInfo[1].offsetOfSwFieldForStcStart -
             sip5_hashMaskCrcFieldsInfo[0].offsetOfSwFieldForStcStart) / sizeof(GT_U32);

        /* pointer to first field */
        *fieldPtrPtr = (GT_U32*)(((char*)entryPtr) + sip5_hashMaskCrcFieldsInfo[0].offsetOfSwFieldForStcStart);

        *numOfFieldsPtr = sizeof(sip5_hashMaskCrcFieldsInfo) / sizeof(sip5_hashMaskCrcFieldsInfo[0]);
    }
    else
    {
        *fieldsDestinationPtr =
            (hashMaskCrcFieldsInfo[1].offsetOfSwFieldForStcStart -
             hashMaskCrcFieldsInfo[0].offsetOfSwFieldForStcStart) / sizeof(GT_U32);

        /* pointer to first field */
        *fieldPtrPtr = (GT_U32*)(((char*)entryPtr) + hashMaskCrcFieldsInfo[0].offsetOfSwFieldForStcStart);

        *numOfFieldsPtr = sizeof(hashMaskCrcFieldsInfo) / sizeof(hashMaskCrcFieldsInfo[0]);
    }

    return ;
}

/**
* @internal internal_cpssDxChTrunkHashMaskCrcEntrySet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         Set the entry of masks in the specified index in 'CRC hash mask table'.
*
* @note   APPLICABLE DEVICES:      Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3.
*
* @param[in] devNum                   - the device number.
* @param[in] index                    - the table index.
*                                      (APPLICABLE RANGES: Lion, xCat2, Lion2 0..27).
*                                      (APPLICABLE RANGES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon 0..31).
* @param[in] entryPtr                 - (pointer to) The entry.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or index
* @retval GT_OUT_OF_RANGE          - one of the fields in entryPtr are out of range
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note related to feature 'CRC hash mode'
*
*/
static GT_STATUS internal_cpssDxChTrunkHashMaskCrcEntrySet
(
    IN GT_U8                          devNum,
    IN GT_U32                         index,
    IN CPSS_DXCH_TRUNK_LBH_CRC_MASK_ENTRY_STC *entryPtr
)
{
    GT_U32  mask;
    GT_U32  length;/*field length*/
    GT_U32  fieldMaskBmp;/* field mask bmp */
    GT_U32  fieldType;/*field type*/
    GT_U32  *fieldPtr;/* pointer to the current field in the entry */
    GT_U32  hwEntry[HASH_MASK_CRC_ENTRY_NUM_WORDS_CNS];/* values read from HW and need to combined to write back to HW */
    GT_U32  numOfFields;/* number of fields in the entry */
    GT_U32  fieldsDestination;/* 2 fields destination (value is in GT_U32 steps)*/
    FIELD_INFO_STC *hashMaskCrcFieldsInfoPtr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E);

    HASH_MASK_CRC_INDEX_CHECK_MAC(devNum,index);
    CPSS_NULL_PTR_CHECK_MAC(entryPtr);

    /* NOTE: the index is checked in prvCpssDxChWriteTableEntry() */

    /* get fields distance and number info and pointer to first field */
    hashMaskFieldDestinationGet(devNum,entryPtr,&numOfFields,&fieldsDestination,&fieldPtr);

    cpssOsMemSet(hwEntry,0,sizeof(hwEntry));

    hashMaskCrcFieldsInfoPtr = (PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE) ?
        sip5_hashMaskCrcFieldsInfo :
        hashMaskCrcFieldsInfo;


    for(fieldType = 0 ; fieldType < numOfFields ; fieldType++ , fieldPtr += fieldsDestination)
    {
        length = hashMaskCrcFieldsInfoPtr[fieldType].length;
        /* mask according to length of field */
        mask = BIT_MASK_MAC(length);

        if(hashMaskCrcFieldsInfoPtr[fieldType].offset)
        {
            /* the value is shifted , so need to shift the mask */
            mask <<= hashMaskCrcFieldsInfoPtr[fieldType].offset;
        }

        fieldMaskBmp = *fieldPtr;

        if((fieldMaskBmp & (~mask)))
        {
            /* there are bits in the bitmap that are not supported by the
               device : 'out of range' */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
        }

        if(hashMaskCrcFieldsInfoPtr[fieldType].offset)
        {
            /* the value is shifted and we already shifted the mask to ,
               so we need to 'restore' the values to start at bit 0 */
            fieldMaskBmp >>= hashMaskCrcFieldsInfoPtr[fieldType].offset;
            mask         >>= hashMaskCrcFieldsInfoPtr[fieldType].offset;
        }

        if(hashMaskCrcFieldsInfoPtr[fieldType].needToSwap == GT_TRUE)
        {
            /* swap the values that the caller set */
            /* the swap will swap bits of the value and shift it to start from bit 0 */
            fieldMaskBmp = swap32BitsAndShift(fieldMaskBmp,length);
        }

        /* set values from caller */
        U32_SET_FIELD_IN_ENTRY_MAC(hwEntry,hashMaskCrcFieldsInfoPtr[fieldType].startBit,length,
            fieldMaskBmp);
    }

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum))
    {
        /* handle fields that are not 'bitmap of mask' */
        U32_SET_FIELD_IN_ENTRY_MAC(hwEntry,70,1,BOOL2BIT_MAC(entryPtr->symmetricMacAddrEnable)  );
        U32_SET_FIELD_IN_ENTRY_MAC(hwEntry,71,1,BOOL2BIT_MAC(entryPtr->symmetricIpv4AddrEnable) );
        U32_SET_FIELD_IN_ENTRY_MAC(hwEntry,72,1,BOOL2BIT_MAC(entryPtr->symmetricIpv6AddrEnable) );
        U32_SET_FIELD_IN_ENTRY_MAC(hwEntry,73,1,BOOL2BIT_MAC(entryPtr->symmetricL4PortEnable)   );
    }

    /* write the table entry */
    return prvCpssDxChWriteTableEntry(devNum,
                                   CPSS_DXCH_LION_TABLE_TRUNK_HASH_MASK_CRC_E,
                                   index,
                                   hwEntry);
}

/**
* @internal cpssDxChTrunkHashMaskCrcEntrySet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         Set the entry of masks in the specified index in 'CRC hash mask table'.
*
* @note   APPLICABLE DEVICES:      Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3.
*
* @param[in] devNum                   - the device number.
* @param[in] index                    - the table index.
*                                      (APPLICABLE RANGES: Lion, xCat2, Lion2 0..27).
*                                      (APPLICABLE RANGES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon 0..31).
* @param[in] entryPtr                 - (pointer to) The entry.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or index
* @retval GT_OUT_OF_RANGE          - one of the fields in entryPtr are out of range
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note related to feature 'CRC hash mode'
*
*/
GT_STATUS cpssDxChTrunkHashMaskCrcEntrySet
(
    IN GT_U8                          devNum,
    IN GT_U32                         index,
    IN CPSS_DXCH_TRUNK_LBH_CRC_MASK_ENTRY_STC *entryPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTrunkHashMaskCrcEntrySet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, index, entryPtr));

    rc = internal_cpssDxChTrunkHashMaskCrcEntrySet(devNum, index, entryPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, index, entryPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTrunkHashMaskCrcEntryGet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         Get the entry of masks in the specified index in 'CRC hash mask table'.
*
* @note   APPLICABLE DEVICES:      Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3.
*
* @param[in] devNum                   - the device number.
* @param[in] index                    - the table index.
*                                      (APPLICABLE RANGES: Lion, xCat2, Lion2 0..27).
*                                      (APPLICABLE RANGES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon 0..31).
*
* @param[out] entryPtr                 - (pointer to) The entry.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or index
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note related to feature 'CRC hash mode'
*
*/
static GT_STATUS internal_cpssDxChTrunkHashMaskCrcEntryGet
(
    IN GT_U8                          devNum,
    IN GT_U32                         index,
    OUT CPSS_DXCH_TRUNK_LBH_CRC_MASK_ENTRY_STC *entryPtr
)
{
    GT_STATUS rc;            /* return code         */
    GT_U32  length;/*field length*/
    GT_U32  fieldMaskBmp;/* field mask bmp */
    GT_U32  fieldType;/*field type*/
    GT_U32  *fieldPtr;/* pointer to the current field in the entry */
    GT_U32  hwEntry[HASH_MASK_CRC_ENTRY_NUM_WORDS_CNS];/* values read from HW and need to combined to write back to HW */
    GT_U32  numOfFields;/* number of fields in the entry */
    GT_U32  fieldsDestination;/* 2 fields destination (value is in GT_U32 steps)*/
    FIELD_INFO_STC *hashMaskCrcFieldsInfoPtr;
    GT_U32  tmpValue;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E);

    HASH_MASK_CRC_INDEX_CHECK_MAC(devNum,index);
    CPSS_NULL_PTR_CHECK_MAC(entryPtr);

    /* get the table entry */
    rc = prvCpssDxChReadTableEntry(devNum,
                                   CPSS_DXCH_LION_TABLE_TRUNK_HASH_MASK_CRC_E,
                                   index,
                                   hwEntry);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* get fields distance and number info and pointer to first field */
    hashMaskFieldDestinationGet(devNum,entryPtr,&numOfFields,&fieldsDestination,&fieldPtr);

    hashMaskCrcFieldsInfoPtr = (PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE) ?
        sip5_hashMaskCrcFieldsInfo :
        hashMaskCrcFieldsInfo;


    for(fieldType = 0 ; fieldType < numOfFields ; fieldType++ , fieldPtr += fieldsDestination)
    {
        length = hashMaskCrcFieldsInfoPtr[fieldType].length;

        fieldMaskBmp = 0;

        /* get values from HW */
        U32_GET_FIELD_IN_ENTRY_MAC(hwEntry,hashMaskCrcFieldsInfoPtr[fieldType].startBit,length,
            fieldMaskBmp);

        if(hashMaskCrcFieldsInfoPtr[fieldType].needToSwap == GT_TRUE)
        {
            /* swap the values that the HW returned */
            /* the swap will swap bits of the value and shift it to start from bit 0 */
            fieldMaskBmp = swap32BitsAndShift(fieldMaskBmp,length);
        }

        if(hashMaskCrcFieldsInfoPtr[fieldType].offset)
        {
            /* the value need to be shifted because it not start at bit 0 */
            fieldMaskBmp <<= hashMaskCrcFieldsInfoPtr[fieldType].offset;
        }

        *fieldPtr = fieldMaskBmp;
    }

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum))
    {
        /* handle fields that are not 'bitmap of mask' */
        U32_GET_FIELD_IN_ENTRY_MAC(hwEntry,70,1,tmpValue);
        entryPtr->symmetricMacAddrEnable = BIT2BOOL_MAC(tmpValue);

        U32_GET_FIELD_IN_ENTRY_MAC(hwEntry,71,1,tmpValue);
        entryPtr->symmetricIpv4AddrEnable = BIT2BOOL_MAC(tmpValue);

        U32_GET_FIELD_IN_ENTRY_MAC(hwEntry,72,1,tmpValue);
        entryPtr->symmetricIpv6AddrEnable = BIT2BOOL_MAC(tmpValue);

        U32_GET_FIELD_IN_ENTRY_MAC(hwEntry,73,1,tmpValue);
        entryPtr->symmetricL4PortEnable = BIT2BOOL_MAC(tmpValue);
    }



    return GT_OK;
}

/**
* @internal cpssDxChTrunkHashMaskCrcEntryGet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         Get the entry of masks in the specified index in 'CRC hash mask table'.
*
* @note   APPLICABLE DEVICES:      Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3.
*
* @param[in] devNum                   - the device number.
* @param[in] index                    - the table index.
*                                      (APPLICABLE RANGES: Lion, xCat2, Lion2 0..27).
*                                      (APPLICABLE RANGES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon 0..31).
*
* @param[out] entryPtr                 - (pointer to) The entry.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or index
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note related to feature 'CRC hash mode'
*
*/
GT_STATUS cpssDxChTrunkHashMaskCrcEntryGet
(
    IN GT_U8                          devNum,
    IN GT_U32                         index,
    OUT CPSS_DXCH_TRUNK_LBH_CRC_MASK_ENTRY_STC *entryPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTrunkHashMaskCrcEntryGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, index, entryPtr));

    rc = internal_cpssDxChTrunkHashMaskCrcEntryGet(devNum, index, entryPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, index, entryPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal calcHashFor70BytesCrc32 function
* @endinternal
*
* @brief   Calculate CRC 32 bit for 70 bytes input data.
*         CRC-32 hash value calculation is performed using the CRC-32-IEEE 802.3 polynomial
* @param[in] d[560]                   -  array of 70 bytes data for CRC32.
* @param[in] d[560]                   -  array of 70 bytes data for CRC32.
*                                       32 bit CRC.
*/
static GT_U32 calcHashFor70BytesCrc32
(
    GT_U32 crc32Seed,
    const GT_U8 d[560])
{
/*****************************************************/
    /* http://www.easics.com/webtools/crctool
       polynomial: (0 1 2 4 5 7 8 10 11 12 16 22 23 26 32)
       data width: 560
       convention: the first serial bit is d[559] */
/*****************************************************/
    GT_U32 newcrc = 0;

/* The following code is not needed in PSS and excluded from PSS compilation. */
#ifndef CPSS_PRODUCT
    GT_U32 c[32];
    GT_U32 i;
    for (i = 0; i < 32; i++)
    {
        c[i] = U32_GET_FIELD_MAC(crc32Seed, i ,1);
    }

    SET_BIT_FROM_VALUE_MAC(newcrc,0, d[558] ^ d[557] ^ d[556] ^ d[555] ^ d[554] ^ d[551] ^ d[549] ^ d[545] ^ d[542] ^ d[540] ^ d[539] ^ d[535] ^ d[534] ^ d[532] ^ d[531] ^ d[530] ^d[529] ^ d[528] ^ d[526] ^ d[525] ^ d[522] ^ d[521] ^ d[519] ^ d[518] ^ d[516] ^ d[514] ^ d[512] ^ d[511] ^ d[510] ^ d[508] ^ d[507] ^ d[506] ^ d[502] ^ d[501] ^ d[500] ^ d[495] ^ d[494] ^ d[493] ^ d[492] ^ d[491] ^ d[490] ^ d[489] ^ d[488] ^ d[486] ^ d[483] ^ d[482] ^ d[481] ^ d[480] ^ d[479] ^ d[477] ^ d[476] ^ d[472] ^ d[470] ^ d[468] ^ d[465] ^ d[464] ^ d[462] ^ d[461] ^ d[458] ^ d[452] ^ d[450] ^ d[449] ^ d[448] ^ d[444] ^ d[437] ^ d[436] ^ d[434] ^ d[433] ^ d[424] ^ d[422] ^ d[419] ^ d[418] ^ d[416] ^ d[414] ^ d[412] ^ d[409] ^ d[408] ^ d[407] ^ d[405] ^ d[404] ^ d[400] ^ d[399] ^ d[398] ^ d[396] ^ d[393] ^ d[392] ^ d[391] ^ d[390] ^ d[388] ^ d[387] ^ d[386] ^ d[381] ^ d[378] ^ d[376] ^ d[374] ^ d[372] ^ d[369] ^ d[368] ^ d[366] ^ d[363] ^ d[362] ^ d[359] ^ d[358] ^ d[357] ^ d[353] ^ d[349] ^ d[348] ^ d[347] ^ d[345] ^ d[344] ^ d[342] ^ d[341] ^ d[339] ^ d[338] ^ d[337] ^ d[335] ^ d[334] ^ d[333] ^ d[328] ^ d[327] ^ d[322] ^ d[321] ^ d[320] ^ d[319] ^ d[318] ^ d[317] ^ d[315] ^ d[312] ^ d[310] ^ d[309] ^ d[305] ^ d[303] ^ d[302] ^ d[300] ^ d[299] ^ d[298] ^ d[297] ^ d[296] ^ d[295] ^ d[294] ^ d[292] ^ d[290] ^ d[288] ^ d[287] ^ d[286] ^ d[283] ^ d[279] ^ d[277] ^ d[276] ^ d[274] ^ d[273] ^ d[269] ^ d[268] ^ d[265] ^ d[264] ^ d[261] ^ d[259] ^ d[257] ^ d[255] ^ d[252] ^ d[248] ^ d[243] ^ d[237] ^ d[234] ^ d[230] ^ d[228] ^ d[227] ^ d[226] ^ d[224] ^ d[216] ^ d[214] ^ d[212] ^ d[210] ^ d[209] ^ d[208] ^ d[207] ^ d[203] ^ d[202] ^ d[201] ^ d[199] ^ d[198] ^ d[197] ^ d[194] ^ d[193] ^ d[192] ^ d[191] ^ d[190] ^ d[188] ^ d[186] ^ d[183] ^ d[182] ^ d[172] ^ d[171] ^ d[170] ^ d[169] ^ d[167] ^ d[166] ^ d[162] ^ d[161] ^ d[158] ^ d[156] ^ d[155] ^ d[151] ^ d[149] ^ d[144] ^ d[143] ^ d[137] ^ d[136] ^ d[135] ^ d[134] ^ d[132] ^ d[128] ^ d[127] ^ d[126] ^ d[125] ^ d[123] ^ d[119] ^ d[118] ^ d[117] ^ d[116] ^ d[114] ^ d[113] ^ d[111] ^ d[110] ^ d[106] ^ d[104] ^ d[103] ^ d[101] ^ d[99] ^ d[98] ^ d[97] ^ d[96] ^ d[95] ^ d[94] ^ d[87] ^ d[85] ^ d[84] ^ d[83] ^ d[82] ^ d[81] ^ d[79] ^ d[73] ^ d[72] ^ d[68] ^ d[67] ^ d[66] ^ d[65] ^ d[63] ^ d[61] ^ d[60] ^ d[58] ^ d[55] ^ d[54] ^ d[53] ^ d[50] ^ d[48] ^ d[47] ^ d[45] ^ d[44] ^ d[37] ^ d[34] ^ d[32] ^ d[31] ^ d[30] ^ d[29] ^ d[28] ^ d[26] ^ d[25] ^ d[24] ^ d[16] ^ d[12] ^ d[10] ^ d[9] ^ d[6] ^ d[0] ^ c[0] ^ c[1] ^ c[2] ^ c[3] ^ c[4] ^ c[6] ^ c[7] ^ c[11] ^ c[12] ^ c[14] ^ c[17] ^ c[21] ^ c[23] ^ c[26] ^ c[27] ^ c[28] ^ c[29] ^ c[30]);

    SET_BIT_FROM_VALUE_MAC(newcrc,1, d[559] ^ d[554] ^ d[552] ^ d[551] ^ d[550] ^ d[549] ^ d[546] ^ d[545] ^ d[543] ^ d[542] ^ d[541] ^ d[539] ^ d[536] ^ d[534] ^ d[533] ^ d[528] ^d[527] ^ d[525] ^ d[523] ^ d[521] ^ d[520] ^ d[518] ^ d[517] ^ d[516] ^ d[515] ^ d[514] ^ d[513] ^ d[510] ^ d[509] ^ d[506] ^ d[503] ^ d[500] ^ d[496] ^ d[488] ^ d[487] ^ d[486] ^ d[484] ^ d[479] ^ d[478] ^ d[476] ^ d[473] ^ d[472] ^ d[471] ^ d[470] ^ d[469] ^ d[468] ^ d[466] ^ d[464] ^ d[463] ^ d[461] ^ d[459] ^ d[458] ^ d[453] ^ d[452] ^ d[451] ^ d[448] ^ d[445] ^ d[444] ^ d[438] ^ d[436] ^ d[435] ^ d[433] ^ d[425] ^ d[424] ^ d[423] ^ d[422] ^ d[420] ^ d[418] ^ d[417] ^ d[416] ^ d[415] ^ d[414] ^ d[413] ^ d[412] ^ d[410] ^ d[407] ^ d[406] ^ d[404] ^ d[401] ^ d[398] ^ d[397] ^ d[396] ^ d[394] ^ d[390] ^ d[389] ^ d[386] ^ d[382] ^ d[381] ^ d[379] ^ d[378] ^ d[377] ^ d[376] ^ d[375] ^ d[374] ^ d[373] ^ d[372] ^ d[370] ^ d[368] ^ d[367] ^ d[366] ^ d[364] ^ d[362] ^ d[360] ^ d[357] ^ d[354] ^ d[353] ^ d[350] ^ d[347] ^ d[346] ^ d[344] ^ d[343] ^ d[341] ^ d[340] ^ d[337] ^ d[336] ^ d[333] ^ d[329] ^ d[327] ^ d[323] ^ d[317] ^ d[316] ^ d[315] ^ d[313] ^ d[312] ^ d[311] ^ d[309] ^ d[306] ^ d[305] ^ d[304] ^ d[302] ^ d[301] ^ d[294] ^ d[293] ^ d[292] ^ d[291] ^ d[290] ^ d[289] ^ d[286] ^ d[284] ^ d[283] ^ d[280] ^ d[279] ^ d[278] ^ d[276] ^ d[275] ^ d[273] ^ d[270] ^ d[268] ^ d[266] ^ d[264] ^ d[262] ^ d[261] ^ d[260] ^ d[259] ^ d[258] ^ d[257] ^ d[256] ^ d[255] ^ d[253] ^ d[252] ^ d[249] ^ d[248] ^ d[244] ^ d[243] ^ d[238] ^ d[237] ^ d[235] ^ d[234] ^ d[231] ^ d[230] ^ d[229] ^ d[226] ^ d[225] ^ d[224] ^ d[217] ^ d[216] ^ d[215] ^ d[214] ^ d[213] ^ d[212] ^ d[211] ^ d[207] ^ d[204] ^ d[201] ^ d[200] ^ d[197] ^ d[195] ^ d[190] ^ d[189] ^ d[188] ^ d[187] ^ d[186] ^ d[184] ^ d[182] ^ d[173] ^ d[169] ^ d[168] ^ d[166] ^ d[163] ^ d[161] ^ d[159] ^ d[158] ^ d[157] ^ d[155] ^ d[152] ^ d[151] ^ d[150] ^ d[149] ^ d[145] ^ d[143] ^ d[138] ^ d[134] ^ d[133] ^ d[132] ^ d[129] ^ d[125] ^ d[124] ^ d[123] ^ d[120] ^ d[116] ^ d[115] ^ d[113] ^ d[112] ^ d[110] ^ d[107] ^ d[106] ^ d[105] ^ d[103] ^ d[102] ^ d[101] ^ d[100] ^ d[94] ^ d[88] ^ d[87] ^ d[86] ^ d[81] ^ d[80] ^ d[79] ^ d[74] ^ d[72] ^ d[69] ^ d[65] ^ d[64] ^ d[63] ^ d[62] ^ d[60] ^ d[59] ^ d[58] ^ d[56] ^ d[53] ^ d[51] ^ d[50] ^ d[49] ^ d[47] ^ d[46] ^ d[44] ^ d[38] ^ d[37] ^ d[35] ^ d[34] ^ d[33] ^ d[28] ^ d[27] ^ d[24] ^ d[17] ^ d[16] ^ d[13] ^ d[12] ^ d[11] ^ d[9] ^ d[7] ^ d[6] ^ d[1] ^ d[0] ^ c[0] ^ c[5] ^ c[6] ^ c[8] ^ c[11] ^ c[13] ^ c[14] ^ c[15] ^ c[17] ^ c[18] ^ c[21] ^ c[22] ^ c[23] ^ c[24] ^ c[26] ^ c[31]);

    SET_BIT_FROM_VALUE_MAC(newcrc,2, d[558] ^ d[557] ^ d[556] ^ d[554] ^ d[553] ^ d[552] ^ d[550] ^ d[549] ^ d[547] ^ d[546] ^ d[545] ^ d[544] ^ d[543] ^ d[539] ^ d[537] ^ d[532] ^ d[531] ^ d[530] ^ d[525] ^ d[524] ^ d[517] ^ d[515] ^ d[512] ^ d[508] ^ d[506] ^ d[504] ^ d[502] ^ d[500] ^ d[497] ^ d[495] ^ d[494] ^ d[493] ^ d[492] ^ d[491] ^ d[490] ^ d[487] ^ d[486] ^ d[485] ^ d[483] ^ d[482] ^ d[481] ^ d[476] ^ d[474] ^ d[473] ^ d[471] ^ d[469] ^ d[468] ^ d[467] ^ d[461] ^ d[460] ^ d[459] ^ d[458] ^ d[454] ^ d[453] ^ d[450] ^ d[448] ^ d[446] ^ d[445] ^ d[444] ^ d[439] ^ d[433] ^ d[426] ^ d[425] ^ d[423] ^ d[422] ^ d[421] ^ d[417] ^ d[415] ^ d[413] ^ d[412] ^ d[411] ^ d[409] ^ d[404] ^ d[402] ^ d[400] ^ d[397] ^ d[396] ^ d[395] ^ d[393] ^ d[392] ^ d[388] ^ d[386] ^ d[383] ^ d[382] ^ d[381] ^ d[380] ^ d[379] ^ d[377] ^ d[375] ^ d[373] ^ d[372] ^ d[371] ^ d[367] ^ d[366] ^ d[365] ^ d[362] ^ d[361] ^ d[359] ^ d[357] ^ d[355] ^ d[354] ^ d[353] ^ d[351] ^ d[349] ^ d[339] ^ d[335] ^ d[333] ^ d[330] ^ d[327] ^ d[324] ^ d[322] ^ d[321] ^ d[320] ^ d[319] ^ d[316] ^ d[315] ^ d[314] ^ d[313] ^ d[309] ^ d[307] ^ d[306] ^ d[300] ^ d[299] ^ d[298] ^ d[297] ^ d[296] ^ d[293] ^ d[291] ^ d[288] ^ d[286] ^ d[285] ^ d[284] ^ d[283] ^ d[281] ^ d[280] ^ d[273] ^ d[271] ^ d[268] ^ d[267] ^ d[264] ^ d[263] ^ d[262] ^ d[260] ^ d[258] ^ d[256] ^ d[255] ^ d[254] ^ d[253] ^ d[252] ^ d[250] ^ d[249] ^ d[248] ^ d[245] ^ d[244] ^ d[243] ^ d[239] ^ d[238] ^ d[237] ^ d[236] ^ d[235] ^ d[234] ^ d[232] ^ d[231] ^ d[228] ^ d[225] ^ d[224] ^ d[218] ^ d[217] ^ d[215] ^ d[213] ^ d[210] ^ d[209] ^ d[207] ^ d[205] ^ d[203] ^ d[199] ^ d[197] ^ d[196] ^ d[194] ^ d[193] ^ d[192] ^ d[189] ^ d[187] ^ d[186] ^ d[185] ^ d[182] ^ d[174] ^ d[172] ^ d[171] ^ d[166] ^ d[164] ^ d[161] ^ d[160] ^ d[159] ^ d[155] ^ d[153] ^ d[152] ^ d[150] ^ d[149] ^ d[146] ^ d[143] ^ d[139] ^ d[137] ^ d[136] ^ d[133] ^ d[132] ^ d[130] ^ d[128] ^ d[127] ^ d[124] ^ d[123] ^ d[121] ^ d[119] ^ d[118] ^ d[110] ^ d[108] ^ d[107] ^ d[102] ^ d[99] ^ d[98] ^ d[97] ^ d[96] ^ d[94] ^ d[89] ^ d[88] ^ d[85] ^ d[84] ^ d[83] ^ d[80] ^ d[79] ^ d[75] ^ d[72] ^ d[70] ^ d[68] ^ d[67] ^ d[64] ^ d[59] ^ d[58] ^ d[57] ^ d[55] ^ d[53] ^ d[52] ^ d[51] ^ d[44] ^ d[39] ^ d[38] ^ d[37] ^ d[36] ^ d[35] ^ d[32] ^ d[31] ^ d[30] ^ d[26] ^ d[24] ^ d[18] ^ d[17] ^ d[16] ^ d[14] ^ d[13] ^ d[9] ^ d[8] ^ d[7] ^ d[6] ^ d[2] ^ d[1] ^ d[0] ^ c[2] ^ c[3] ^ c[4] ^ c[9] ^ c[11] ^ c[15] ^ c[16] ^ c[17] ^ c[18] ^ c[19] ^ c[21] ^ c[22] ^ c[24] ^ c[25] ^ c[26] ^ c[28] ^ c[29] ^ c[30]);

    SET_BIT_FROM_VALUE_MAC(newcrc,3, d[559] ^ d[558] ^ d[557] ^ d[555] ^ d[554] ^ d[553] ^ d[551] ^ d[550] ^ d[548] ^ d[547] ^ d[546] ^ d[545] ^ d[544] ^ d[540] ^ d[538] ^ d[533] ^ d[532] ^ d[531] ^ d[526] ^ d[525] ^ d[518] ^ d[516] ^ d[513] ^ d[509] ^ d[507] ^ d[505] ^ d[503] ^ d[501] ^ d[498] ^ d[496] ^ d[495] ^ d[494] ^ d[493] ^ d[492] ^ d[491] ^ d[488] ^ d[487] ^ d[486] ^ d[484] ^ d[483] ^ d[482] ^ d[477] ^ d[475] ^ d[474] ^ d[472] ^ d[470] ^ d[469] ^ d[468] ^ d[462] ^ d[461] ^ d[460] ^ d[459] ^ d[455] ^ d[454] ^ d[451] ^ d[449] ^ d[447] ^ d[446] ^ d[445] ^ d[440] ^ d[434] ^ d[427] ^ d[426] ^ d[424] ^ d[423] ^ d[422] ^ d[418] ^ d[416] ^ d[414] ^ d[413] ^ d[412] ^ d[410] ^ d[405] ^ d[403] ^ d[401] ^ d[398] ^ d[397] ^ d[396] ^ d[394] ^ d[393] ^ d[389] ^ d[387] ^ d[384] ^ d[383] ^ d[382] ^ d[381] ^ d[380] ^ d[378] ^ d[376] ^ d[374] ^ d[373] ^ d[372] ^ d[368] ^ d[367] ^ d[366] ^ d[363] ^ d[362] ^ d[360] ^ d[358] ^ d[356] ^ d[355] ^ d[354] ^ d[352] ^ d[350] ^ d[340] ^ d[336] ^ d[334] ^ d[331] ^ d[328] ^ d[325] ^ d[323] ^ d[322] ^ d[321] ^ d[320] ^ d[317] ^ d[316] ^ d[315] ^ d[314] ^ d[310] ^ d[308] ^ d[307] ^ d[301] ^ d[300] ^ d[299] ^ d[298] ^ d[297] ^ d[294] ^ d[292] ^ d[289] ^ d[287] ^ d[286] ^ d[285] ^ d[284] ^ d[282] ^ d[281] ^ d[274] ^ d[272] ^ d[269] ^ d[268] ^ d[265] ^ d[264] ^ d[263] ^ d[261] ^ d[259] ^ d[257] ^ d[256] ^ d[255] ^ d[254] ^ d[253] ^ d[251] ^ d[250] ^ d[249] ^ d[246] ^ d[245] ^ d[244] ^ d[240] ^ d[239] ^ d[238] ^ d[237] ^ d[236] ^ d[235] ^ d[233] ^ d[232] ^ d[229] ^ d[226] ^ d[225] ^ d[219] ^ d[218] ^ d[216] ^ d[214] ^ d[211] ^ d[210] ^ d[208] ^ d[206] ^ d[204] ^ d[200] ^ d[198] ^ d[197] ^ d[195] ^ d[194] ^ d[193] ^ d[190] ^ d[188] ^ d[187] ^ d[186] ^ d[183] ^ d[175] ^ d[173] ^ d[172] ^ d[167] ^ d[165] ^ d[162] ^ d[161] ^ d[160] ^ d[156] ^ d[154] ^ d[153] ^ d[151] ^ d[150] ^ d[147] ^ d[144] ^ d[140] ^ d[138] ^ d[137] ^ d[134] ^ d[133] ^ d[131] ^ d[129] ^ d[128] ^ d[125] ^ d[124] ^ d[122] ^ d[120] ^ d[119] ^ d[111] ^ d[109] ^ d[108] ^ d[103] ^ d[100] ^ d[99] ^ d[98] ^ d[97] ^ d[95] ^ d[90] ^ d[89] ^ d[86] ^ d[85] ^ d[84] ^ d[81] ^ d[80] ^ d[76] ^ d[73] ^ d[71] ^ d[69] ^ d[68] ^ d[65] ^ d[60] ^ d[59] ^ d[58] ^ d[56] ^ d[54] ^ d[53] ^ d[52] ^ d[45] ^ d[40] ^ d[39] ^ d[38] ^ d[37] ^ d[36] ^ d[33] ^ d[32] ^ d[31] ^ d[27] ^ d[25] ^ d[19] ^ d[18] ^ d[17] ^ d[15] ^ d[14] ^ d[10] ^ d[9] ^ d[8] ^ d[7] ^ d[3] ^ d[2] ^ d[1] ^ c[3] ^ c[4] ^ c[5] ^ c[10] ^ c[12] ^ c[16] ^ c[17] ^ c[18] ^ c[19] ^ c[20] ^ c[22] ^ c[23] ^ c[25] ^ c[26] ^ c[27] ^ c[29] ^ c[30] ^ c[31]);

    SET_BIT_FROM_VALUE_MAC(newcrc,4, d[559] ^ d[557] ^ d[552] ^ d[548] ^ d[547] ^ d[546] ^ d[542] ^ d[541] ^ d[540] ^ d[535] ^ d[533] ^ d[531] ^ d[530] ^ d[529] ^ d[528] ^ d[527] ^ d[525] ^ d[522] ^ d[521] ^ d[518] ^ d[517] ^ d[516] ^ d[512] ^ d[511] ^ d[507] ^ d[504] ^ d[501] ^ d[500] ^ d[499] ^ d[497] ^ d[496] ^ d[491] ^ d[490] ^ d[487] ^ d[486] ^ d[485] ^ d[484] ^ d[482] ^ d[481] ^ d[480] ^ d[479] ^ d[478] ^ d[477] ^ d[475] ^ d[473] ^ d[472] ^ d[471] ^ d[469] ^ d[468] ^ d[465] ^ d[464] ^ d[463] ^ d[460] ^ d[458] ^ d[456] ^ d[455] ^ d[449] ^ d[447] ^ d[446] ^ d[444] ^ d[441] ^ d[437] ^ d[436] ^ d[435] ^ d[434] ^ d[433] ^ d[428] ^ d[427] ^ d[425] ^ d[423] ^ d[422] ^ d[418] ^ d[417] ^ d[416] ^ d[415] ^ d[413] ^ d[412] ^ d[411] ^ d[409] ^ d[408] ^ d[407] ^ d[406] ^ d[405] ^ d[402] ^ d[400] ^ d[397] ^ d[396] ^ d[395] ^ d[394] ^ d[393] ^ d[392] ^ d[391] ^ d[387] ^ d[386] ^ d[385] ^ d[384] ^ d[383] ^ d[382] ^ d[379] ^ d[378] ^ d[377] ^ d[376] ^ d[375] ^ d[373] ^ d[372] ^ d[367] ^ d[366] ^ d[364] ^ d[362] ^ d[361] ^ d[358] ^ d[356] ^ d[355] ^ d[351] ^ d[349] ^ d[348] ^ d[347] ^ d[345] ^ d[344] ^ d[342] ^ d[339] ^ d[338] ^ d[334] ^ d[333] ^ d[332] ^ d[329] ^ d[328] ^ d[327] ^ d[326] ^ d[324] ^ d[323] ^ d[320] ^ d[319] ^ d[316] ^ d[312] ^ d[311] ^ d[310] ^ d[308] ^ d[305] ^ d[303] ^ d[301] ^ d[297] ^ d[296] ^ d[294] ^ d[293] ^ d[292] ^ d[285] ^ d[282] ^ d[279] ^ d[277] ^ d[276] ^ d[275] ^ d[274] ^ d[270] ^ d[268] ^ d[266] ^ d[262] ^ d[261] ^ d[260] ^ d[259] ^ d[258] ^ d[256] ^ d[254] ^ d[251] ^ d[250] ^ d[248] ^ d[247] ^ d[246] ^ d[245] ^ d[243] ^ d[241] ^ d[240] ^ d[239] ^ d[238] ^ d[236] ^ d[233] ^ d[228] ^ d[224] ^ d[220] ^ d[219] ^ d[217] ^ d[216] ^ d[215] ^ d[214] ^ d[211] ^ d[210] ^ d[208] ^ d[205] ^ d[203] ^ d[202] ^ d[197] ^ d[196] ^ d[195] ^ d[193] ^ d[192] ^ d[190] ^ d[189] ^ d[187] ^ d[186] ^ d[184] ^ d[183] ^ d[182] ^ d[176] ^ d[174] ^ d[173] ^ d[172] ^ d[171] ^ d[170] ^ d[169] ^ d[168] ^ d[167] ^ d[163] ^ d[158] ^ d[157] ^ d[156] ^ d[154] ^ d[152] ^ d[149] ^ d[148] ^ d[145] ^ d[144] ^ d[143] ^ d[141] ^ d[139] ^ d[138] ^ d[137] ^ d[136] ^ d[130] ^ d[129] ^ d[128] ^ d[127] ^ d[121] ^ d[120] ^ d[119] ^ d[118] ^ d[117] ^ d[116] ^ d[114] ^ d[113] ^ d[112] ^ d[111] ^ d[109] ^ d[106] ^ d[103] ^ d[100] ^ d[97] ^ d[95] ^ d[94] ^ d[91] ^ d[90] ^ d[86] ^ d[84] ^ d[83] ^ d[79] ^ d[77] ^ d[74] ^ d[73] ^ d[70] ^ d[69] ^ d[68] ^ d[67] ^ d[65] ^ d[63] ^ d[59] ^ d[58] ^ d[57] ^ d[50] ^ d[48] ^ d[47] ^ d[46] ^ d[45] ^ d[44] ^ d[41] ^ d[40] ^ d[39] ^ d[38] ^ d[33] ^ d[31] ^ d[30] ^ d[29] ^ d[25] ^ d[24] ^ d[20] ^ d[19] ^ d[18] ^ d[15] ^ d[12] ^ d[11] ^ d[8] ^ d[6] ^ d[4] ^ d[3] ^ d[2] ^ d[0] ^ c[0] ^ c[1] ^ c[2] ^ c[3] ^ c[5] ^ c[7] ^ c[12] ^ c[13] ^ c[14] ^ c[18] ^ c[19] ^ c[20] ^ c[24] ^ c[29] ^ c[31]);

    SET_BIT_FROM_VALUE_MAC(newcrc,5, d[557] ^ d[556] ^ d[555] ^ d[554] ^ d[553] ^ d[551] ^ d[548] ^ d[547] ^ d[545] ^ d[543] ^ d[541] ^ d[540] ^ d[539] ^ d[536] ^ d[535] ^ d[525] ^ d[523] ^ d[521] ^ d[517] ^ d[516] ^ d[514] ^ d[513] ^ d[511] ^ d[510] ^ d[507] ^ d[506] ^ d[505] ^ d[498] ^ d[497] ^ d[495] ^ d[494] ^ d[493] ^ d[490] ^ d[489] ^ d[487] ^ d[485] ^ d[478] ^ d[477] ^ d[474] ^ d[473] ^ d[469] ^ d[468] ^ d[466] ^ d[462] ^ d[459] ^ d[458] ^ d[457] ^ d[456] ^ d[452] ^ d[449] ^ d[447] ^ d[445] ^ d[444] ^ d[442] ^ d[438] ^ d[435] ^ d[433] ^ d[429] ^ d[428] ^ d[426] ^ d[423] ^ d[422] ^ d[417] ^ d[413] ^ d[410] ^ d[406] ^ d[405] ^ d[404] ^ d[403] ^ d[401] ^ d[400] ^ d[399] ^ d[397] ^ d[395] ^ d[394] ^ d[391] ^ d[390] ^ d[385] ^ d[384] ^ d[383] ^ d[381] ^ d[380] ^ d[379] ^ d[377] ^ d[373] ^ d[372] ^ d[369] ^ d[367] ^ d[366] ^ d[365] ^ d[358] ^ d[356] ^ d[353] ^ d[352] ^ d[350] ^ d[347] ^ d[346] ^ d[344] ^ d[343] ^ d[342] ^ d[341] ^ d[340] ^ d[338] ^ d[337] ^ d[330] ^ d[329] ^ d[325] ^ d[324] ^ d[322] ^ d[319] ^ d[318] ^ d[315] ^ d[313] ^ d[311] ^ d[310] ^ d[306] ^ d[305] ^ d[304] ^ d[303] ^ d[300] ^ d[299] ^ d[296] ^ d[293] ^ d[292] ^ d[290] ^ d[288] ^ d[287] ^ d[280] ^ d[279] ^ d[278] ^ d[275] ^ d[274] ^ d[273] ^ d[271] ^ d[268] ^ d[267] ^ d[265] ^ d[264] ^ d[263] ^ d[262] ^ d[260] ^ d[251] ^ d[249] ^ d[247] ^ d[246] ^ d[244] ^ d[243] ^ d[242] ^ d[241] ^ d[240] ^ d[239] ^ d[230] ^ d[229] ^ d[228] ^ d[227] ^ d[226] ^ d[225] ^ d[224] ^ d[221] ^ d[220] ^ d[218] ^ d[217] ^ d[215] ^ d[214] ^ d[211] ^ d[210] ^ d[208] ^ d[207] ^ d[206] ^ d[204] ^ d[202] ^ d[201] ^ d[199] ^ d[196] ^ d[192] ^ d[187] ^ d[186] ^ d[185] ^ d[184] ^ d[182] ^ d[177] ^ d[175] ^ d[174] ^ d[173] ^ d[168] ^ d[167] ^ d[166] ^ d[164] ^ d[162] ^ d[161] ^ d[159] ^ d[157] ^ d[156] ^ d[153] ^ d[151] ^ d[150] ^ d[146] ^ d[145] ^ d[143] ^ d[142] ^ d[140] ^ d[139] ^ d[138] ^ d[136] ^ d[135] ^ d[134] ^ d[132] ^ d[131] ^ d[130] ^ d[129] ^ d[127] ^ d[126] ^ d[125] ^ d[123] ^ d[122] ^ d[121] ^ d[120] ^ d[116] ^ d[115] ^ d[112] ^ d[111] ^ d[107] ^ d[106] ^ d[103] ^ d[99] ^ d[97] ^ d[94] ^ d[92] ^ d[91] ^ d[83] ^ d[82] ^ d[81] ^ d[80] ^ d[79] ^ d[78] ^ d[75] ^ d[74] ^ d[73] ^ d[72] ^ d[71] ^ d[70] ^ d[69] ^ d[67] ^ d[65] ^ d[64] ^ d[63] ^ d[61] ^ d[59] ^ d[55] ^ d[54] ^ d[53] ^ d[51] ^ d[50] ^ d[49] ^ d[46] ^ d[44] ^ d[42] ^ d[41] ^ d[40] ^ d[39] ^ d[37] ^ d[29] ^ d[28] ^ d[24] ^ d[21] ^ d[20] ^ d[19] ^ d[13] ^ d[10] ^ d[7] ^ d[6] ^ d[5] ^ d[4] ^ d[3] ^ d[1] ^ d[0] ^ c[7] ^ c[8] ^ c[11] ^ c[12] ^ c[13] ^ c[15] ^ c[17] ^ c[19] ^ c[20] ^ c[23] ^ c[25] ^ c[26] ^ c[27] ^ c[28] ^ c[29]);

    SET_BIT_FROM_VALUE_MAC(newcrc,6, d[558] ^ d[557] ^ d[556] ^ d[555] ^ d[554] ^ d[552] ^ d[549] ^ d[548] ^ d[546] ^ d[544] ^ d[542] ^ d[541] ^ d[540] ^ d[537] ^ d[536] ^ d[526] ^ d[524] ^ d[522] ^ d[518] ^ d[517] ^ d[515] ^ d[514] ^ d[512] ^ d[511] ^ d[508] ^ d[507] ^ d[506] ^ d[499] ^ d[498] ^ d[496] ^ d[495] ^ d[494] ^ d[491] ^ d[490] ^ d[488] ^ d[486] ^ d[479] ^ d[478] ^ d[475] ^ d[474] ^ d[470] ^ d[469] ^ d[467] ^ d[463] ^ d[460] ^ d[459] ^ d[458] ^ d[457] ^ d[453] ^ d[450] ^ d[448] ^ d[446] ^ d[445] ^ d[443] ^ d[439] ^ d[436] ^ d[434] ^ d[430] ^ d[429] ^ d[427] ^ d[424] ^ d[423] ^ d[418] ^ d[414] ^ d[411] ^ d[407] ^ d[406] ^ d[405] ^ d[404] ^ d[402] ^ d[401] ^ d[400] ^ d[398] ^ d[396] ^ d[395] ^ d[392] ^ d[391] ^ d[386] ^ d[385] ^ d[384] ^ d[382] ^ d[381] ^ d[380] ^ d[378] ^ d[374] ^ d[373] ^ d[370] ^ d[368] ^ d[367] ^ d[366] ^ d[359] ^ d[357] ^ d[354] ^ d[353] ^ d[351] ^ d[348] ^ d[347] ^ d[345] ^ d[344] ^ d[343] ^ d[342] ^ d[341] ^ d[339] ^ d[338] ^ d[331] ^ d[330] ^ d[326] ^ d[325] ^ d[323] ^ d[320] ^ d[319] ^ d[316] ^ d[314] ^ d[312] ^ d[311] ^ d[307] ^ d[306] ^ d[305] ^ d[304] ^ d[301] ^ d[300] ^ d[297] ^ d[294] ^ d[293] ^ d[291] ^ d[289] ^ d[288] ^ d[281] ^ d[280] ^ d[279] ^ d[276] ^ d[275] ^ d[274] ^ d[272] ^ d[269] ^ d[268] ^ d[266] ^ d[265] ^ d[264] ^ d[263] ^ d[261] ^ d[252] ^ d[250] ^ d[248] ^ d[247] ^ d[245] ^ d[244] ^ d[243] ^ d[242] ^ d[241] ^ d[240] ^ d[231] ^ d[230] ^ d[229] ^ d[228] ^ d[227] ^ d[226] ^ d[225] ^ d[222] ^ d[221] ^ d[219] ^ d[218] ^ d[216] ^ d[215] ^ d[212] ^ d[211] ^ d[209] ^ d[208] ^ d[207] ^ d[205] ^ d[203] ^ d[202] ^ d[200] ^ d[197] ^ d[193] ^ d[188] ^ d[187] ^ d[186] ^ d[185] ^ d[183] ^ d[178] ^ d[176] ^ d[175] ^ d[174] ^ d[169] ^ d[168] ^ d[167] ^ d[165] ^ d[163] ^ d[162] ^ d[160] ^ d[158] ^ d[157] ^ d[154] ^ d[152] ^ d[151] ^ d[147] ^ d[146] ^ d[144] ^ d[143] ^ d[141] ^ d[140] ^ d[139] ^ d[137] ^ d[136] ^ d[135] ^ d[133] ^ d[132] ^ d[131] ^ d[130] ^ d[128] ^ d[127] ^ d[126] ^ d[124] ^ d[123] ^ d[122] ^ d[121] ^ d[117] ^ d[116] ^ d[113] ^ d[112] ^ d[108] ^ d[107] ^ d[104] ^ d[100] ^ d[98] ^ d[95] ^ d[93] ^ d[92] ^ d[84] ^ d[83] ^ d[82] ^ d[81] ^ d[80] ^ d[79] ^ d[76] ^ d[75] ^ d[74] ^ d[73] ^ d[72] ^ d[71] ^ d[70] ^ d[68] ^ d[66] ^ d[65] ^ d[64] ^ d[62] ^ d[60] ^ d[56] ^ d[55] ^ d[54] ^ d[52] ^ d[51] ^ d[50] ^ d[47] ^ d[45] ^ d[43] ^ d[42] ^ d[41] ^ d[40] ^ d[38] ^ d[30] ^ d[29] ^ d[25] ^ d[22] ^ d[21] ^ d[20] ^ d[14] ^ d[11] ^ d[8] ^ d[7] ^ d[6] ^ d[5] ^ d[4] ^ d[2] ^ d[1] ^ c[8] ^ c[9] ^ c[12] ^ c[13] ^ c[14] ^ c[16] ^ c[18] ^ c[20] ^ c[21] ^ c[24] ^ c[26] ^ c[27] ^ c[28] ^ c[29] ^ c[30]);

    SET_BIT_FROM_VALUE_MAC(newcrc,7, d[559] ^ d[554] ^ d[553] ^ d[551] ^ d[550] ^ d[547] ^ d[543] ^ d[541] ^ d[540] ^ d[539] ^ d[538] ^ d[537] ^ d[535] ^ d[534] ^ d[532] ^ d[531] ^ d[530] ^ d[529] ^ d[528] ^ d[527] ^ d[526] ^ d[523] ^ d[522] ^ d[521] ^ d[515] ^ d[514] ^ d[513] ^ d[511] ^ d[510] ^ d[509] ^ d[506] ^ d[502] ^ d[501] ^ d[499] ^ d[497] ^ d[496] ^ d[494] ^ d[493] ^ d[490] ^ d[488] ^ d[487] ^ d[486] ^ d[483] ^ d[482] ^ d[481] ^ d[477] ^ d[475] ^ d[472] ^ d[471] ^ d[465] ^ d[462] ^ d[460] ^ d[459] ^ d[454] ^ d[452] ^ d[451] ^ d[450] ^ d[448] ^ d[447] ^ d[446] ^ d[440] ^ d[436] ^ d[435] ^ d[434] ^ d[433] ^ d[431] ^ d[430] ^ d[428] ^ d[425] ^ d[422] ^ d[418] ^ d[416] ^ d[415] ^ d[414] ^ d[409] ^ d[406] ^ d[404] ^ d[403] ^ d[402] ^ d[401] ^ d[400] ^ d[398] ^ d[397] ^ d[391] ^ d[390] ^ d[388] ^ d[385] ^ d[383] ^ d[382] ^ d[379] ^ d[378] ^ d[376] ^ d[375] ^ d[372] ^ d[371] ^ d[367] ^ d[366] ^ d[363] ^ d[362] ^ d[360] ^ d[359] ^ d[357] ^ d[355] ^ d[354] ^ d[353] ^ d[352] ^ d[347] ^ d[346] ^ d[343] ^ d[341] ^ d[340] ^ d[338] ^ d[337] ^ d[335] ^ d[334] ^ d[333] ^ d[332] ^ d[331] ^ d[328] ^ d[326] ^ d[324] ^ d[322] ^ d[319] ^ d[318] ^ d[313] ^ d[310] ^ d[309] ^ d[308] ^ d[307] ^ d[306] ^ d[303] ^ d[301] ^ d[300] ^ d[299] ^ d[297] ^ d[296] ^ d[289] ^ d[288] ^ d[287] ^ d[286] ^ d[283] ^ d[282] ^ d[281] ^ d[280] ^ d[279] ^ d[275] ^ d[274] ^ d[270] ^ d[268] ^ d[267] ^ d[266] ^ d[262] ^ d[261] ^ d[259] ^ d[257] ^ d[255] ^ d[253] ^ d[252] ^ d[251] ^ d[249] ^ d[246] ^ d[245] ^ d[244] ^ d[242] ^ d[241] ^ d[237] ^ d[234] ^ d[232] ^ d[231] ^ d[229] ^ d[224] ^ d[223] ^ d[222] ^ d[220] ^ d[219] ^ d[217] ^ d[214] ^ d[213] ^ d[207] ^ d[206] ^ d[204] ^ d[202] ^ d[199] ^ d[197] ^ d[193] ^ d[192] ^ d[191] ^ d[190] ^ d[189] ^ d[187] ^ d[184] ^ d[183] ^ d[182] ^ d[179] ^ d[177] ^ d[176] ^ d[175] ^ d[172] ^ d[171] ^ d[168] ^ d[167] ^ d[164] ^ d[163] ^ d[162] ^ d[159] ^ d[156] ^ d[153] ^ d[152] ^ d[151] ^ d[149] ^ d[148] ^ d[147] ^ d[145] ^ d[143] ^ d[142] ^ d[141] ^ d[140] ^ d[138] ^ d[135] ^ d[133] ^ d[131] ^ d[129] ^ d[126] ^ d[124] ^ d[122] ^ d[119] ^ d[116] ^ d[111] ^ d[110] ^ d[109] ^ d[108] ^ d[106] ^ d[105] ^ d[104] ^ d[103] ^ d[98] ^ d[97] ^ d[95] ^ d[93] ^ d[87] ^ d[80] ^ d[79] ^ d[77] ^ d[76] ^ d[75] ^ d[74] ^ d[71] ^ d[69] ^ d[68] ^ d[60] ^ d[58] ^ d[57] ^ d[56] ^ d[54] ^ d[52] ^ d[51] ^ d[50] ^ d[47] ^ d[46] ^ d[45] ^ d[43] ^ d[42] ^ d[41] ^ d[39] ^ d[37] ^ d[34] ^ d[32] ^ d[29] ^ d[28] ^ d[25] ^ d[24] ^ d[23] ^ d[22] ^ d[21] ^ d[16] ^ d[15] ^ d[10] ^ d[8] ^ d[7] ^ d[5] ^ d[3] ^ d[2] ^ d[0] ^ c[0] ^ c[1] ^ c[2] ^ c[3] ^ c[4] ^ c[6] ^ c[7] ^ c[9] ^ c[10] ^ c[11] ^ c[12] ^ c[13] ^ c[15] ^ c[19] ^ c[22] ^ c[23] ^ c[25] ^ c[26] ^ c[31]);

    SET_BIT_FROM_VALUE_MAC(newcrc,8, d[558] ^ d[557] ^ d[556] ^ d[552] ^ d[549] ^ d[548] ^ d[545] ^ d[544] ^ d[541] ^ d[538] ^ d[536] ^ d[534] ^ d[533] ^ d[527] ^ d[526] ^ d[525] ^ d[524] ^ d[523] ^ d[521] ^ d[519] ^ d[518] ^ d[515] ^ d[508] ^ d[506] ^ d[503] ^ d[501] ^ d[498] ^ d[497] ^ d[493] ^ d[492] ^ d[490] ^ d[487] ^ d[486] ^ d[484] ^ d[481] ^ d[480] ^ d[479] ^ d[478] ^ d[477] ^ d[473] ^ d[470] ^ d[468] ^ d[466] ^ d[465] ^ d[464] ^ d[463] ^ d[462] ^ d[460] ^ d[458] ^ d[455] ^ d[453] ^ d[451] ^ d[450] ^ d[447] ^ d[444] ^ d[441] ^ d[435] ^ d[433] ^ d[432] ^ d[431] ^ d[429] ^ d[426] ^ d[424] ^ d[423] ^ d[422] ^ d[418] ^ d[417] ^ d[415] ^ d[414] ^ d[412] ^ d[410] ^ d[409] ^ d[408] ^ d[403] ^ d[402] ^ d[401] ^ d[400] ^ d[396] ^ d[393] ^ d[390] ^ d[389] ^ d[388] ^ d[387] ^ d[384] ^ d[383] ^ d[381] ^ d[380] ^ d[379] ^ d[378] ^ d[377] ^ d[374] ^ d[373] ^ d[369] ^ d[367] ^ d[366] ^ d[364] ^ d[362] ^ d[361] ^ d[360] ^ d[359] ^ d[357] ^ d[356] ^ d[355] ^ d[354] ^ d[349] ^ d[345] ^ d[337] ^ d[336] ^ d[332] ^ d[329] ^ d[328] ^ d[325] ^ d[323] ^ d[322] ^ d[321] ^ d[318] ^ d[317] ^ d[315] ^ d[314] ^ d[312] ^ d[311] ^ d[308] ^ d[307] ^ d[305] ^ d[304] ^ d[303] ^ d[301] ^ d[299] ^ d[296] ^ d[295] ^ d[294] ^ d[292] ^ d[289] ^ d[286] ^ d[284] ^ d[282] ^ d[281] ^ d[280] ^ d[279] ^ d[277] ^ d[275] ^ d[274] ^ d[273] ^ d[271] ^ d[267] ^ d[265] ^ d[264] ^ d[263] ^ d[262] ^ d[261] ^ d[260] ^ d[259] ^ d[258] ^ d[257] ^ d[256] ^ d[255] ^ d[254] ^ d[253] ^ d[250] ^ d[248] ^ d[247] ^ d[246] ^ d[245] ^ d[242] ^ d[238] ^ d[237] ^ d[235] ^ d[234] ^ d[233] ^ d[232] ^ d[228] ^ d[227] ^ d[226] ^ d[225] ^ d[223] ^ d[221] ^ d[220] ^ d[218] ^ d[216] ^ d[215] ^ d[212] ^ d[210] ^ d[209] ^ d[205] ^ d[202] ^ d[201] ^ d[200] ^ d[199] ^ d[197] ^ d[186] ^ d[185] ^ d[184] ^ d[182] ^ d[180] ^ d[178] ^ d[177] ^ d[176] ^ d[173] ^ d[171] ^ d[170] ^ d[168] ^ d[167] ^ d[166] ^ d[165] ^ d[164] ^ d[163] ^ d[162] ^ d[161] ^ d[160] ^ d[158] ^ d[157] ^ d[156] ^ d[155] ^ d[154] ^ d[153] ^ d[152] ^ d[151] ^ d[150] ^ d[148] ^ d[146] ^ d[142] ^ d[141] ^ d[139] ^ d[137] ^ d[135] ^ d[130] ^ d[128] ^ d[126] ^ d[120] ^ d[119] ^ d[118] ^ d[116] ^ d[114] ^ d[113] ^ d[112] ^ d[109] ^ d[107] ^ d[105] ^ d[103] ^ d[101] ^ d[97] ^ d[95] ^ d[88] ^ d[87] ^ d[85] ^ d[84] ^ d[83] ^ d[82] ^ d[80] ^ d[79] ^ d[78] ^ d[77] ^ d[76] ^ d[75] ^ d[73] ^ d[70] ^ d[69] ^ d[68] ^ d[67] ^ d[66] ^ d[65] ^ d[63] ^ d[60] ^ d[59] ^ d[57] ^ d[54] ^ d[52] ^ d[51] ^ d[50] ^ d[46] ^ d[45] ^ d[43] ^ d[42] ^ d[40] ^ d[38] ^ d[37] ^ d[35] ^ d[34] ^ d[33] ^ d[32] ^ d[31] ^ d[28] ^ d[23] ^ d[22] ^ d[17] ^ d[12] ^ d[11] ^ d[10] ^ d[8] ^ d[4] ^ d[3] ^ d[1] ^ d[0] ^ c[5] ^ c[6] ^ c[8] ^ c[10] ^ c[13] ^ c[16] ^ c[17] ^ c[20] ^ c[21] ^ c[24] ^ c[28] ^ c[29] ^ c[30]);

    SET_BIT_FROM_VALUE_MAC(newcrc,9, d[559] ^ d[558] ^ d[557] ^ d[553] ^ d[550] ^ d[549] ^ d[546] ^ d[545] ^ d[542] ^ d[539] ^ d[537] ^ d[535] ^ d[534] ^ d[528] ^ d[527] ^ d[526] ^ d[525] ^ d[524] ^ d[522] ^ d[520] ^ d[519] ^ d[516] ^ d[509] ^ d[507] ^ d[504] ^ d[502] ^ d[499] ^ d[498] ^ d[494] ^ d[493] ^ d[491] ^ d[488] ^ d[487] ^ d[485] ^ d[482] ^ d[481] ^ d[480] ^ d[479] ^ d[478] ^ d[474] ^ d[471] ^ d[469] ^ d[467] ^ d[466] ^ d[465] ^ d[464] ^ d[463] ^ d[461] ^ d[459] ^ d[456] ^ d[454] ^ d[452] ^ d[451] ^ d[448] ^ d[445] ^ d[442] ^ d[436] ^ d[434] ^ d[433] ^ d[432] ^ d[430] ^ d[427] ^ d[425] ^ d[424] ^ d[423] ^ d[419] ^ d[418] ^ d[416] ^ d[415] ^ d[413] ^ d[411] ^ d[410] ^ d[409] ^ d[404] ^ d[403] ^ d[402] ^ d[401] ^ d[397] ^ d[394] ^ d[391] ^ d[390] ^ d[389] ^ d[388] ^ d[385] ^ d[384] ^ d[382] ^ d[381] ^ d[380] ^ d[379] ^ d[378] ^ d[375] ^ d[374] ^ d[370] ^ d[368] ^ d[367] ^ d[365] ^ d[363] ^ d[362] ^ d[361] ^ d[360] ^ d[358] ^ d[357] ^ d[356] ^ d[355] ^ d[350] ^ d[346] ^ d[338] ^ d[337] ^ d[333] ^ d[330] ^ d[329] ^ d[326] ^ d[324] ^ d[323] ^ d[322] ^ d[319] ^ d[318] ^ d[316] ^ d[315] ^ d[313] ^ d[312] ^ d[309] ^ d[308] ^ d[306] ^ d[305] ^ d[304] ^ d[302] ^ d[300] ^ d[297] ^ d[296] ^ d[295] ^ d[293] ^ d[290] ^ d[287] ^ d[285] ^ d[283] ^ d[282] ^ d[281] ^ d[280] ^ d[278] ^ d[276] ^ d[275] ^ d[274] ^ d[272] ^ d[268] ^ d[266] ^ d[265] ^ d[264] ^ d[263] ^ d[262] ^ d[261] ^ d[260] ^ d[259] ^ d[258] ^ d[257] ^ d[256] ^ d[255] ^ d[254] ^ d[251] ^ d[249] ^ d[248] ^ d[247] ^ d[246] ^ d[243] ^ d[239] ^ d[238] ^ d[236] ^ d[235] ^ d[234] ^ d[233] ^ d[229] ^ d[228] ^ d[227] ^ d[226] ^ d[224] ^ d[222] ^ d[221] ^ d[219] ^ d[217] ^ d[216] ^ d[213] ^ d[211] ^ d[210] ^ d[206] ^ d[203] ^ d[202] ^ d[201] ^ d[200] ^ d[198] ^ d[187] ^ d[186] ^ d[185] ^ d[183] ^ d[181] ^ d[179] ^ d[178] ^ d[177] ^ d[174] ^ d[172] ^ d[171] ^ d[169] ^ d[168] ^ d[167] ^ d[166] ^ d[165] ^ d[164] ^ d[163] ^ d[162] ^ d[161] ^ d[159] ^ d[158] ^ d[157] ^ d[156] ^ d[155] ^ d[154] ^ d[153] ^ d[152] ^ d[151] ^ d[149] ^ d[147] ^ d[143] ^ d[142] ^ d[140] ^ d[138] ^ d[136] ^ d[131] ^ d[129] ^ d[127] ^ d[121] ^ d[120] ^ d[119] ^ d[117] ^ d[115] ^ d[114] ^ d[113] ^ d[110] ^ d[108] ^ d[106] ^ d[104] ^ d[102] ^ d[98] ^ d[96] ^ d[89] ^ d[88] ^ d[86] ^ d[85] ^ d[84] ^ d[83] ^ d[81] ^ d[80] ^ d[79] ^ d[78] ^ d[77] ^ d[76] ^ d[74] ^ d[71] ^ d[70] ^ d[69] ^ d[68] ^ d[67] ^ d[66] ^ d[64] ^ d[61] ^ d[60] ^ d[58] ^ d[55] ^ d[53] ^ d[52] ^ d[51] ^ d[47] ^ d[46] ^ d[44] ^ d[43] ^ d[41] ^ d[39] ^ d[38] ^ d[36] ^ d[35] ^ d[34] ^ d[33] ^ d[32] ^ d[29] ^ d[24] ^ d[23] ^ d[18] ^ d[13] ^ d[12] ^ d[11] ^ d[9] ^ d[5] ^ d[4] ^ d[2] ^ d[1] ^ c[0] ^ c[6] ^ c[7] ^ c[9] ^ c[11] ^ c[14] ^ c[17] ^ c[18] ^ c[21] ^ c[22] ^ c[25] ^ c[29] ^ c[30] ^ c[31]);

    SET_BIT_FROM_VALUE_MAC(newcrc,10, d[559] ^ d[557] ^ d[556] ^ d[555] ^ d[550] ^ d[549] ^ d[547] ^ d[546] ^ d[545] ^ d[543] ^ d[542] ^ d[539] ^ d[538] ^ d[536] ^ d[534] ^ d[532] ^ d[531] ^ d[530] ^ d[527] ^ d[523] ^ d[522] ^ d[520] ^ d[519] ^ d[518] ^ d[517] ^ d[516] ^ d[514] ^ d[512] ^ d[511] ^ d[507] ^ d[506] ^ d[505] ^ d[503] ^ d[502] ^ d[501] ^ d[499] ^ d[493] ^ d[491] ^ d[490] ^ d[477] ^ d[476] ^ d[475] ^ d[467] ^ d[466] ^ d[461] ^ d[460] ^ d[458] ^ d[457] ^ d[455] ^ d[453] ^ d[450] ^ d[448] ^ d[446] ^ d[444] ^ d[443] ^ d[436] ^ d[435] ^ d[431] ^ d[428] ^ d[426] ^ d[425] ^ d[422] ^ d[420] ^ d[418] ^ d[417] ^ d[411] ^ d[410] ^ d[409] ^ d[408] ^ d[407] ^ d[403] ^ d[402] ^ d[400] ^ d[399] ^ d[396] ^ d[395] ^ d[393] ^ d[389] ^ d[388] ^ d[387] ^ d[385] ^ d[383] ^ d[382] ^ d[380] ^ d[379] ^ d[378] ^ d[375] ^ d[374] ^ d[372] ^ d[371] ^ d[364] ^ d[361] ^ d[356] ^ d[353] ^ d[351] ^ d[349] ^ d[348] ^ d[345] ^ d[344] ^ d[342] ^ d[341] ^ d[337] ^ d[335] ^ d[333] ^ d[331] ^ d[330] ^ d[328] ^ d[325] ^ d[324] ^ d[323] ^ d[322] ^ d[321] ^ d[318] ^ d[316] ^ d[315] ^ d[314] ^ d[313] ^ d[312] ^ d[307] ^ d[306] ^ d[302] ^ d[301] ^ d[300] ^ d[299] ^ d[295] ^ d[292] ^ d[291] ^ d[290] ^ d[287] ^ d[284] ^ d[282] ^ d[281] ^ d[275] ^ d[274] ^ d[268] ^ d[267] ^ d[266] ^ d[263] ^ d[262] ^ d[260] ^ d[258] ^ d[256] ^ d[250] ^ d[249] ^ d[247] ^ d[244] ^ d[243] ^ d[240] ^ d[239] ^ d[236] ^ d[235] ^ d[229] ^ d[226] ^ d[225] ^ d[224] ^ d[223] ^ d[222] ^ d[220] ^ d[218] ^ d[217] ^ d[216] ^ d[211] ^ d[210] ^ d[209] ^ d[208] ^ d[204] ^ d[198] ^ d[197] ^ d[194] ^ d[193] ^ d[192] ^ d[191] ^ d[190] ^ d[187] ^ d[184] ^ d[183] ^ d[180] ^ d[179] ^ d[178] ^ d[175] ^ d[173] ^ d[171] ^ d[168] ^ d[165] ^ d[164] ^ d[163] ^ d[161] ^ d[160] ^ d[159] ^ d[157] ^ d[154] ^ d[153] ^ d[152] ^ d[151] ^ d[150] ^ d[149] ^ d[148] ^ d[141] ^ d[139] ^ d[136] ^ d[135] ^ d[134] ^ d[130] ^ d[127] ^ d[126] ^ d[125] ^ d[123] ^ d[122] ^ d[121] ^ d[120] ^ d[119] ^ d[117] ^ d[115] ^ d[113] ^ d[110] ^ d[109] ^ d[107] ^ d[106] ^ d[105] ^ d[104] ^ d[101] ^ d[98] ^ d[96] ^ d[95] ^ d[94] ^ d[90] ^ d[89] ^ d[86] ^ d[83] ^ d[80] ^ d[78] ^ d[77] ^ d[75] ^ d[73] ^ d[71] ^ d[70] ^ d[69] ^ d[66] ^ d[63] ^ d[62] ^ d[60] ^ d[59] ^ d[58] ^ d[56] ^ d[55] ^ d[52] ^ d[50] ^ d[42] ^ d[40] ^ d[39] ^ d[36] ^ d[35] ^ d[33] ^ d[32] ^ d[31] ^ d[29] ^ d[28] ^ d[26] ^ d[19] ^ d[16] ^ d[14] ^ d[13] ^ d[9] ^ d[5] ^ d[3] ^ d[2] ^ d[0] ^ c[2] ^ c[3] ^ c[4] ^ c[6] ^ c[8] ^ c[10] ^ c[11] ^ c[14] ^ c[15] ^ c[17] ^ c[18] ^ c[19] ^ c[21] ^ c[22] ^ c[27] ^ c[28] ^ c[29] ^ c[31]);

    SET_BIT_FROM_VALUE_MAC(newcrc,11, d[555] ^ d[554] ^ d[550] ^ d[549] ^ d[548] ^ d[547] ^ d[546] ^ d[545] ^ d[544] ^ d[543] ^ d[542] ^ d[537] ^ d[534] ^ d[533] ^ d[530] ^ d[529] ^ d[526] ^ d[525] ^ d[524] ^ d[523] ^ d[522] ^ d[520] ^ d[517] ^ d[516] ^ d[515] ^ d[514] ^ d[513] ^ d[511] ^ d[510] ^ d[504] ^ d[503] ^ d[501] ^ d[495] ^ d[493] ^ d[490] ^ d[489] ^ d[488] ^ d[486] ^ d[483] ^ d[482] ^ d[481] ^ d[480] ^ d[479] ^ d[478] ^ d[472] ^ d[470] ^ d[467] ^ d[465] ^ d[464] ^ d[459] ^ d[456] ^ d[454] ^ d[452] ^ d[451] ^ d[450] ^ d[448] ^ d[447] ^ d[445] ^ d[434] ^ d[433] ^ d[432] ^ d[429] ^ d[427] ^ d[426] ^ d[424] ^ d[423] ^ d[422] ^ d[421] ^ d[416] ^ d[414] ^ d[411] ^ d[410] ^ d[407] ^ d[405] ^ d[403] ^ d[401] ^ d[399] ^ d[398] ^ d[397] ^ d[394] ^ d[393] ^ d[392] ^ d[391] ^ d[389] ^ d[387] ^ d[384] ^ d[383] ^ d[380] ^ d[379] ^ d[378] ^ d[375] ^ d[374] ^ d[373] ^ d[369] ^ d[368] ^ d[366] ^ d[365] ^ d[363] ^ d[359] ^ d[358] ^ d[354] ^ d[353] ^ d[352] ^ d[350] ^ d[348] ^ d[347] ^ d[346] ^ d[344] ^ d[343] ^ d[341] ^ d[339] ^ d[337] ^ d[336] ^ d[335] ^ d[333] ^ d[332] ^ d[331] ^ d[329] ^ d[328] ^ d[327] ^ d[326] ^ d[325] ^ d[324] ^ d[323] ^ d[321] ^ d[320] ^ d[318] ^ d[316] ^ d[314] ^ d[313] ^ d[312] ^ d[310] ^ d[309] ^ d[308] ^ d[307] ^ d[305] ^ d[301] ^ d[299] ^ d[298] ^ d[297] ^ d[295] ^ d[294] ^ d[293] ^ d[291] ^ d[290] ^ d[287] ^ d[286] ^ d[285] ^ d[282] ^ d[279] ^ d[277] ^ d[275] ^ d[274] ^ d[273] ^ d[267] ^ d[265] ^ d[263] ^ d[255] ^ d[252] ^ d[251] ^ d[250] ^ d[245] ^ d[244] ^ d[243] ^ d[241] ^ d[240] ^ d[236] ^ d[234] ^ d[228] ^ d[225] ^ d[223] ^ d[221] ^ d[219] ^ d[218] ^ d[217] ^ d[216] ^ d[214] ^ d[211] ^ d[208] ^ d[207] ^ d[205] ^ d[203] ^ d[202] ^ d[201] ^ d[197] ^ d[195] ^ d[190] ^ d[186] ^ d[185] ^ d[184] ^ d[183] ^ d[182] ^ d[181] ^ d[180] ^ d[179] ^ d[176] ^ d[174] ^ d[171] ^ d[170] ^ d[167] ^ d[165] ^ d[164] ^ d[160] ^ d[156] ^ d[154] ^ d[153] ^ d[152] ^ d[150] ^ d[144] ^ d[143] ^ d[142] ^ d[140] ^ d[134] ^ d[132] ^ d[131] ^ d[125] ^ d[124] ^ d[122] ^ d[121] ^ d[120] ^ d[119] ^ d[117] ^ d[113] ^ d[108] ^ d[107] ^ d[105] ^ d[104] ^ d[103] ^ d[102] ^ d[101] ^ d[98] ^ d[94] ^ d[91] ^ d[90] ^ d[85] ^ d[83] ^ d[82] ^ d[78] ^ d[76] ^ d[74] ^ d[73] ^ d[71] ^ d[70] ^ d[68] ^ d[66] ^ d[65] ^ d[64] ^ d[59] ^ d[58] ^ d[57] ^ d[56] ^ d[55] ^ d[54] ^ d[51] ^ d[50] ^ d[48] ^ d[47] ^ d[45] ^ d[44] ^ d[43] ^ d[41] ^ d[40] ^ d[36] ^ d[33] ^ d[31] ^ d[28] ^ d[27] ^ d[26] ^ d[25] ^ d[24] ^ d[20] ^ d[17] ^ d[16] ^ d[15] ^ d[14] ^ d[12] ^ d[9] ^ d[4] ^ d[3] ^ d[1] ^ d[0] ^ c[1] ^ c[2] ^ c[5] ^ c[6] ^ c[9] ^ c[14] ^ c[15] ^ c[16] ^ c[17] ^ c[18] ^ c[19] ^ c[20] ^ c[21] ^ c[22] ^ c[26] ^ c[27]);

    SET_BIT_FROM_VALUE_MAC(newcrc,12, d[558] ^ d[557] ^ d[554] ^ d[550] ^ d[548] ^ d[547] ^ d[546] ^ d[544] ^ d[543] ^ d[542] ^ d[540] ^ d[539] ^ d[538] ^ d[532] ^ d[529] ^ d[528] ^ d[527] ^ d[524] ^ d[523] ^ d[522] ^ d[519] ^ d[517] ^ d[515] ^ d[510] ^ d[508] ^ d[507] ^ d[506] ^ d[505] ^ d[504] ^ d[501] ^ d[500] ^ d[496] ^ d[495] ^ d[493] ^ d[492] ^ d[488] ^ d[487] ^ d[486] ^ d[484] ^ d[477] ^ d[476] ^ d[473] ^ d[472] ^ d[471] ^ d[470] ^ d[466] ^ d[464] ^ d[462] ^ d[461] ^ d[460] ^ d[458] ^ d[457] ^ d[455] ^ d[453] ^ d[451] ^ d[450] ^ d[446] ^ d[444] ^ d[437] ^ d[436] ^ d[435] ^ d[430] ^ d[428] ^ d[427] ^ d[425] ^ d[423] ^ d[419] ^ d[418] ^ d[417] ^ d[416] ^ d[415] ^ d[414] ^ d[411] ^ d[409] ^ d[407] ^ d[406] ^ d[405] ^ d[402] ^ d[396] ^ d[395] ^ d[394] ^ d[391] ^ d[387] ^ d[386] ^ d[385] ^ d[384] ^ d[380] ^ d[379] ^ d[378] ^ d[375] ^ d[372] ^ d[370] ^ d[368] ^ d[367] ^ d[364] ^ d[363] ^ d[362] ^ d[360] ^ d[358] ^ d[357] ^ d[355] ^ d[354] ^ d[351] ^ d[341] ^ d[340] ^ d[339] ^ d[336] ^ d[335] ^ d[332] ^ d[330] ^ d[329] ^ d[326] ^ d[325] ^ d[324] ^ d[320] ^ d[318] ^ d[314] ^ d[313] ^ d[312] ^ d[311] ^ d[308] ^ d[306] ^ d[305] ^ d[303] ^ d[297] ^ d[291] ^ d[290] ^ d[280] ^ d[279] ^ d[278] ^ d[277] ^ d[275] ^ d[273] ^ d[269] ^ d[266] ^ d[265] ^ d[261] ^ d[259] ^ d[257] ^ d[256] ^ d[255] ^ d[253] ^ d[251] ^ d[248] ^ d[246] ^ d[245] ^ d[244] ^ d[243] ^ d[242] ^ d[241] ^ d[235] ^ d[234] ^ d[230] ^ d[229] ^ d[228] ^ d[227] ^ d[222] ^ d[220] ^ d[219] ^ d[218] ^ d[217] ^ d[216] ^ d[215] ^ d[214] ^ d[210] ^ d[207] ^ d[206] ^ d[204] ^ d[201] ^ d[199] ^ d[197] ^ d[196] ^ d[194] ^ d[193] ^ d[192] ^ d[190] ^ d[188] ^ d[187] ^ d[185] ^ d[184] ^ d[181] ^ d[180] ^ d[177] ^ d[175] ^ d[170] ^ d[169] ^ d[168] ^ d[167] ^ d[165] ^ d[162] ^ d[158] ^ d[157] ^ d[156] ^ d[154] ^ d[153] ^ d[149] ^ d[145] ^ d[141] ^ d[137] ^ d[136] ^ d[134] ^ d[133] ^ d[128] ^ d[127] ^ d[122] ^ d[121] ^ d[120] ^ d[119] ^ d[117] ^ d[116] ^ d[113] ^ d[111] ^ d[110] ^ d[109] ^ d[108] ^ d[105] ^ d[102] ^ d[101] ^ d[98] ^ d[97] ^ d[96] ^ d[94] ^ d[92] ^ d[91] ^ d[87] ^ d[86] ^ d[85] ^ d[82] ^ d[81] ^ d[77] ^ d[75] ^ d[74] ^ d[73] ^ d[71] ^ d[69] ^ d[68] ^ d[63] ^ d[61] ^ d[59] ^ d[57] ^ d[56] ^ d[54] ^ d[53] ^ d[52] ^ d[51] ^ d[50] ^ d[49] ^ d[47] ^ d[46] ^ d[42] ^ d[41] ^ d[31] ^ d[30] ^ d[27] ^ d[24] ^ d[21] ^ d[18] ^ d[17] ^ d[15] ^ d[13] ^ d[12] ^ d[9] ^ d[6] ^ d[5] ^ d[4] ^ d[2] ^ d[1] ^ d[0] ^ c[0] ^ c[1] ^ c[4] ^ c[10] ^ c[11] ^ c[12] ^ c[14] ^ c[15] ^ c[16] ^ c[18] ^ c[19] ^ c[20] ^ c[22] ^ c[26] ^ c[29] ^ c[30]);

    SET_BIT_FROM_VALUE_MAC(newcrc,13, d[559] ^ d[558] ^ d[555] ^ d[551] ^ d[549] ^ d[548] ^ d[547] ^ d[545] ^ d[544] ^ d[543] ^ d[541] ^ d[540] ^ d[539] ^ d[533] ^ d[530] ^ d[529] ^ d[528] ^ d[525] ^ d[524] ^ d[523] ^ d[520] ^ d[518] ^ d[516] ^ d[511] ^ d[509] ^ d[508] ^ d[507] ^ d[506] ^ d[505] ^ d[502] ^ d[501] ^ d[497] ^ d[496] ^ d[494] ^ d[493] ^ d[489] ^ d[488] ^ d[487] ^ d[485] ^ d[478] ^ d[477] ^ d[474] ^ d[473] ^ d[472] ^ d[471] ^ d[467] ^ d[465] ^ d[463] ^ d[462] ^ d[461] ^ d[459] ^ d[458] ^ d[456] ^ d[454] ^ d[452] ^ d[451] ^ d[447] ^ d[445] ^ d[438] ^ d[437] ^ d[436] ^ d[431] ^ d[429] ^ d[428] ^ d[426] ^ d[424] ^ d[420] ^ d[419] ^ d[418] ^ d[417] ^ d[416] ^ d[415] ^ d[412] ^ d[410] ^ d[408] ^ d[407] ^ d[406] ^ d[403] ^ d[397] ^ d[396] ^ d[395] ^ d[392] ^ d[388] ^ d[387] ^ d[386] ^ d[385] ^ d[381] ^ d[380] ^ d[379] ^ d[376] ^ d[373] ^ d[371] ^ d[369] ^ d[368] ^ d[365] ^ d[364] ^ d[363] ^ d[361] ^ d[359] ^ d[358] ^ d[356] ^ d[355] ^ d[352] ^ d[342] ^ d[341] ^ d[340] ^ d[337] ^ d[336] ^ d[333] ^ d[331] ^ d[330] ^ d[327] ^ d[326] ^ d[325] ^ d[321] ^ d[319] ^ d[315] ^ d[314] ^ d[313] ^ d[312] ^ d[309] ^ d[307] ^ d[306] ^ d[304] ^ d[298] ^ d[292] ^ d[291] ^ d[281] ^ d[280] ^ d[279] ^ d[278] ^ d[276] ^ d[274] ^ d[270] ^ d[267] ^ d[266] ^ d[262] ^ d[260] ^ d[258] ^ d[257] ^ d[256] ^ d[254] ^ d[252] ^ d[249] ^ d[247] ^ d[246] ^ d[245] ^ d[244] ^ d[243] ^ d[242] ^ d[236] ^ d[235] ^ d[231] ^ d[230] ^ d[229] ^ d[228] ^ d[223] ^ d[221] ^ d[220] ^ d[219] ^ d[218] ^ d[217] ^ d[216] ^ d[215] ^ d[211] ^ d[208] ^ d[207] ^ d[205] ^ d[202] ^ d[200] ^ d[198] ^ d[197] ^ d[195] ^ d[194] ^ d[193] ^ d[191] ^ d[189] ^ d[188] ^ d[186] ^ d[185] ^ d[182] ^ d[181] ^ d[178] ^ d[176] ^ d[171] ^ d[170] ^ d[169] ^ d[168] ^ d[166] ^ d[163] ^ d[159] ^ d[158] ^ d[157] ^ d[155] ^ d[154] ^ d[150] ^ d[146] ^ d[142] ^ d[138] ^ d[137] ^ d[135] ^ d[134] ^ d[129] ^ d[128] ^ d[123] ^ d[122] ^ d[121] ^ d[120] ^ d[118] ^ d[117] ^ d[114] ^ d[112] ^ d[111] ^ d[110] ^ d[109] ^ d[106] ^ d[103] ^ d[102] ^ d[99] ^ d[98] ^ d[97] ^ d[95] ^ d[93] ^ d[92] ^ d[88] ^ d[87] ^ d[86] ^ d[83] ^ d[82] ^ d[78] ^ d[76] ^ d[75] ^ d[74] ^ d[72] ^ d[70] ^ d[69] ^ d[64] ^ d[62] ^ d[60] ^ d[58] ^ d[57] ^ d[55] ^ d[54] ^ d[53] ^ d[52] ^ d[51] ^ d[50] ^ d[48] ^ d[47] ^ d[43] ^ d[42] ^ d[32] ^ d[31] ^ d[28] ^ d[25] ^ d[22] ^ d[19] ^ d[18] ^ d[16] ^ d[14] ^ d[13] ^ d[10] ^ d[7] ^ d[6] ^ d[5] ^ d[3] ^ d[2] ^ d[1] ^ c[0] ^ c[1] ^ c[2] ^ c[5] ^ c[11] ^ c[12] ^ c[13] ^ c[15] ^ c[16] ^ c[17] ^ c[19] ^ c[20] ^ c[21] ^ c[23] ^ c[27] ^ c[30] ^ c[31]);

    SET_BIT_FROM_VALUE_MAC(newcrc,14, d[559] ^ d[556] ^ d[552] ^ d[550] ^ d[549] ^ d[548] ^ d[546] ^ d[545] ^ d[544] ^ d[542] ^ d[541] ^ d[540] ^ d[534] ^ d[531] ^ d[530] ^ d[529] ^ d[526] ^ d[525] ^ d[524] ^ d[521] ^ d[519] ^ d[517] ^ d[512] ^ d[510] ^ d[509] ^ d[508] ^ d[507] ^ d[506] ^ d[503] ^ d[502] ^ d[498] ^ d[497] ^ d[495] ^ d[494] ^ d[490] ^ d[489] ^ d[488] ^ d[486] ^ d[479] ^ d[478] ^ d[475] ^ d[474] ^ d[473] ^ d[472] ^ d[468] ^ d[466] ^ d[464] ^ d[463] ^ d[462] ^ d[460] ^ d[459] ^ d[457] ^ d[455] ^ d[453] ^ d[452] ^ d[448] ^ d[446] ^ d[439] ^ d[438] ^ d[437] ^ d[432] ^ d[430] ^ d[429] ^ d[427] ^ d[425] ^ d[421] ^ d[420] ^ d[419] ^ d[418] ^ d[417] ^ d[416] ^ d[413] ^ d[411] ^ d[409] ^ d[408] ^ d[407] ^ d[404] ^ d[398] ^ d[397] ^ d[396] ^ d[393] ^ d[389] ^ d[388] ^ d[387] ^ d[386] ^ d[382] ^ d[381] ^ d[380] ^ d[377] ^ d[374] ^ d[372] ^ d[370] ^ d[369] ^ d[366] ^ d[365] ^ d[364] ^ d[362] ^ d[360] ^ d[359] ^ d[357] ^ d[356] ^ d[353] ^ d[343] ^ d[342] ^ d[341] ^ d[338] ^ d[337] ^ d[334] ^ d[332] ^ d[331] ^ d[328] ^ d[327] ^ d[326] ^ d[322] ^ d[320] ^ d[316] ^ d[315] ^ d[314] ^ d[313] ^ d[310] ^ d[308] ^ d[307] ^ d[305] ^ d[299] ^ d[293] ^ d[292] ^ d[282] ^ d[281] ^ d[280] ^ d[279] ^ d[277] ^ d[275] ^ d[271] ^ d[268] ^ d[267] ^ d[263] ^ d[261] ^ d[259] ^ d[258] ^ d[257] ^ d[255] ^ d[253] ^ d[250] ^ d[248] ^ d[247] ^ d[246] ^ d[245] ^ d[244] ^ d[243] ^ d[237] ^ d[236] ^ d[232] ^ d[231] ^ d[230] ^ d[229] ^ d[224] ^ d[222] ^ d[221] ^ d[220] ^ d[219] ^ d[218] ^ d[217] ^ d[216] ^ d[212] ^ d[209] ^ d[208] ^ d[206] ^ d[203] ^ d[201] ^ d[199] ^ d[198] ^ d[196] ^ d[195] ^ d[194] ^ d[192] ^ d[190] ^ d[189] ^ d[187] ^ d[186] ^ d[183] ^ d[182] ^ d[179] ^ d[177] ^ d[172] ^ d[171] ^ d[170] ^ d[169] ^ d[167] ^ d[164] ^ d[160] ^ d[159] ^ d[158] ^ d[156] ^ d[155] ^ d[151] ^ d[147] ^ d[143] ^ d[139] ^ d[138] ^ d[136] ^ d[135] ^ d[130] ^ d[129] ^ d[124] ^ d[123] ^ d[122] ^ d[121] ^ d[119] ^ d[118] ^ d[115] ^ d[113] ^ d[112] ^ d[111] ^ d[110] ^ d[107] ^ d[104] ^ d[103] ^ d[100] ^ d[99] ^ d[98] ^ d[96] ^ d[94] ^ d[93] ^ d[89] ^ d[88] ^ d[87] ^ d[84] ^ d[83] ^ d[79] ^ d[77] ^ d[76] ^ d[75] ^ d[73] ^ d[71] ^ d[70] ^ d[65] ^ d[63] ^ d[61] ^ d[59] ^ d[58] ^ d[56] ^ d[55] ^ d[54] ^ d[53] ^ d[52] ^ d[51] ^ d[49] ^ d[48] ^ d[44] ^ d[43] ^ d[33] ^ d[32] ^ d[29] ^ d[26] ^ d[23] ^ d[20] ^ d[19] ^ d[17] ^ d[15] ^ d[14] ^ d[11] ^ d[8] ^ d[7] ^ d[6] ^ d[4] ^ d[3] ^ d[2] ^ c[1] ^ c[2] ^ c[3] ^ c[6] ^ c[12] ^ c[13] ^ c[14] ^ c[16] ^ c[17] ^ c[18] ^ c[20] ^ c[21] ^ c[22] ^ c[24] ^ c[28] ^ c[31]);

    SET_BIT_FROM_VALUE_MAC(newcrc,15, d[557] ^ d[553] ^ d[551] ^ d[550] ^ d[549] ^ d[547] ^ d[546] ^ d[545] ^ d[543] ^ d[542] ^ d[541] ^ d[535] ^ d[532] ^ d[531] ^ d[530] ^ d[527] ^ d[526] ^ d[525] ^ d[522] ^ d[520] ^ d[518] ^ d[513] ^ d[511] ^ d[510] ^ d[509] ^ d[508] ^ d[507] ^ d[504] ^ d[503] ^ d[499] ^ d[498] ^ d[496] ^ d[495] ^ d[491] ^ d[490] ^ d[489] ^ d[487] ^ d[480] ^ d[479] ^ d[476] ^ d[475] ^ d[474] ^ d[473] ^ d[469] ^ d[467] ^ d[465] ^ d[464] ^ d[463] ^ d[461] ^ d[460] ^ d[458] ^ d[456] ^ d[454] ^ d[453] ^ d[449] ^ d[447] ^ d[440] ^ d[439] ^ d[438] ^ d[433] ^ d[431] ^ d[430] ^ d[428] ^ d[426] ^ d[422] ^ d[421] ^ d[420] ^ d[419] ^ d[418] ^ d[417] ^ d[414] ^ d[412] ^ d[410] ^ d[409] ^ d[408] ^ d[405] ^ d[399] ^ d[398] ^ d[397] ^ d[394] ^ d[390] ^ d[389] ^ d[388] ^ d[387] ^ d[383] ^ d[382] ^ d[381] ^ d[378] ^ d[375] ^ d[373] ^ d[371] ^ d[370] ^ d[367] ^ d[366] ^ d[365] ^ d[363] ^ d[361] ^ d[360] ^ d[358] ^ d[357] ^ d[354] ^ d[344] ^ d[343] ^ d[342] ^ d[339] ^ d[338] ^ d[335] ^ d[333] ^ d[332] ^ d[329] ^ d[328] ^ d[327] ^ d[323] ^ d[321] ^ d[317] ^ d[316] ^ d[315] ^ d[314] ^ d[311] ^ d[309] ^ d[308] ^ d[306] ^ d[300] ^ d[294] ^ d[293] ^ d[283] ^ d[282] ^ d[281] ^ d[280] ^ d[278] ^ d[276] ^ d[272] ^ d[269] ^ d[268] ^ d[264] ^ d[262] ^ d[260] ^ d[259] ^ d[258] ^ d[256] ^ d[254] ^ d[251] ^ d[249] ^ d[248] ^ d[247] ^ d[246] ^ d[245] ^ d[244] ^ d[238] ^ d[237] ^ d[233] ^ d[232] ^ d[231] ^ d[230] ^ d[225] ^ d[223] ^ d[222] ^ d[221] ^ d[220] ^ d[219] ^ d[218] ^ d[217] ^ d[213] ^ d[210] ^ d[209] ^ d[207] ^ d[204] ^ d[202] ^ d[200] ^ d[199] ^ d[197] ^ d[196] ^ d[195] ^ d[193] ^ d[191] ^ d[190] ^ d[188] ^ d[187] ^ d[184] ^ d[183] ^ d[180] ^ d[178] ^ d[173] ^ d[172] ^ d[171] ^ d[170] ^ d[168] ^ d[165] ^ d[161] ^ d[160] ^ d[159] ^ d[157] ^ d[156] ^ d[152] ^ d[148] ^ d[144] ^ d[140] ^ d[139] ^ d[137] ^ d[136] ^ d[131] ^ d[130] ^ d[125] ^ d[124] ^ d[123] ^ d[122] ^ d[120] ^ d[119] ^ d[116] ^ d[114] ^ d[113] ^ d[112] ^ d[111] ^ d[108] ^ d[105] ^ d[104] ^ d[101] ^ d[100] ^ d[99] ^ d[97] ^ d[95] ^ d[94] ^ d[90] ^ d[89] ^ d[88] ^ d[85] ^ d[84] ^ d[80] ^ d[78] ^ d[77] ^ d[76] ^ d[74] ^ d[72] ^ d[71] ^ d[66] ^ d[64] ^ d[62] ^ d[60] ^ d[59] ^ d[57] ^ d[56] ^ d[55] ^ d[54] ^ d[53] ^ d[52] ^ d[50] ^ d[49] ^ d[45] ^ d[44] ^ d[34] ^ d[33] ^ d[30] ^ d[27] ^ d[24] ^ d[21] ^ d[20] ^ d[18] ^ d[16] ^ d[15] ^ d[12] ^ d[9] ^ d[8] ^ d[7] ^ d[5] ^ d[4] ^ d[3] ^ c[2] ^ c[3] ^ c[4] ^ c[7] ^ c[13] ^ c[14] ^ c[15] ^ c[17] ^ c[18] ^ c[19] ^ c[21] ^ c[22] ^ c[23] ^ c[25] ^ c[29]);

    SET_BIT_FROM_VALUE_MAC(newcrc,16, d[557] ^ d[556] ^ d[555] ^ d[552] ^ d[550] ^ d[549] ^ d[548] ^ d[547] ^ d[546] ^ d[545] ^ d[544] ^ d[543] ^ d[540] ^ d[539] ^ d[536] ^ d[535] ^ d[534] ^ d[533] ^ d[530] ^ d[529] ^ d[527] ^ d[525] ^ d[523] ^ d[522] ^ d[518] ^ d[516] ^ d[509] ^ d[507] ^ d[506] ^ d[505] ^ d[504] ^ d[502] ^ d[501] ^ d[499] ^ d[497] ^ d[496] ^ d[495] ^ d[494] ^ d[493] ^ d[489] ^ d[486] ^ d[483] ^ d[482] ^ d[479] ^ d[475] ^ d[474] ^ d[472] ^ d[466] ^ d[459] ^ d[458] ^ d[457] ^ d[455] ^ d[454] ^ d[452] ^ d[449] ^ d[444] ^ d[441] ^ d[440] ^ d[439] ^ d[437] ^ d[436] ^ d[433] ^ d[432] ^ d[431] ^ d[429] ^ d[427] ^ d[424] ^ d[423] ^ d[421] ^ d[420] ^ d[416] ^ d[415] ^ d[414] ^ d[413] ^ d[412] ^ d[411] ^ d[410] ^ d[408] ^ d[407] ^ d[406] ^ d[405] ^ d[404] ^ d[396] ^ d[395] ^ d[393] ^ d[392] ^ d[389] ^ d[387] ^ d[386] ^ d[384] ^ d[383] ^ d[382] ^ d[381] ^ d[379] ^ d[378] ^ d[371] ^ d[369] ^ d[367] ^ d[364] ^ d[363] ^ d[361] ^ d[357] ^ d[355] ^ d[353] ^ d[349] ^ d[348] ^ d[347] ^ d[343] ^ d[342] ^ d[341] ^ d[340] ^ d[338] ^ d[337] ^ d[336] ^ d[335] ^ d[330] ^ d[329] ^ d[327] ^ d[324] ^ d[321] ^ d[320] ^ d[319] ^ d[316] ^ d[307] ^ d[305] ^ d[303] ^ d[302] ^ d[301] ^ d[300] ^ d[299] ^ d[298] ^ d[297] ^ d[296] ^ d[292] ^ d[290] ^ d[288] ^ d[287] ^ d[286] ^ d[284] ^ d[282] ^ d[281] ^ d[276] ^ d[274] ^ d[270] ^ d[268] ^ d[264] ^ d[263] ^ d[260] ^ d[250] ^ d[249] ^ d[247] ^ d[246] ^ d[245] ^ d[243] ^ d[239] ^ d[238] ^ d[237] ^ d[233] ^ d[232] ^ d[231] ^ d[230] ^ d[228] ^ d[227] ^ d[223] ^ d[222] ^ d[221] ^ d[220] ^ d[219] ^ d[218] ^ d[216] ^ d[212] ^ d[211] ^ d[209] ^ d[207] ^ d[205] ^ d[202] ^ d[200] ^ d[199] ^ d[196] ^ d[193] ^ d[190] ^ d[189] ^ d[186] ^ d[185] ^ d[184] ^ d[183] ^ d[182] ^ d[181] ^ d[179] ^ d[174] ^ d[173] ^ d[170] ^ d[167] ^ d[160] ^ d[157] ^ d[156] ^ d[155] ^ d[153] ^ d[151] ^ d[145] ^ d[144] ^ d[143] ^ d[141] ^ d[140] ^ d[138] ^ d[136] ^ d[135] ^ d[134] ^ d[131] ^ d[128] ^ d[127] ^ d[124] ^ d[121] ^ d[120] ^ d[119] ^ d[118] ^ d[116] ^ d[115] ^ d[112] ^ d[111] ^ d[110] ^ d[109] ^ d[105] ^ d[104] ^ d[103] ^ d[102] ^ d[100] ^ d[99] ^ d[97] ^ d[94] ^ d[91] ^ d[90] ^ d[89] ^ d[87] ^ d[86] ^ d[84] ^ d[83] ^ d[82] ^ d[78] ^ d[77] ^ d[75] ^ d[68] ^ d[66] ^ d[57] ^ d[56] ^ d[51] ^ d[48] ^ d[47] ^ d[46] ^ d[44] ^ d[37] ^ d[35] ^ d[32] ^ d[30] ^ d[29] ^ d[26] ^ d[24] ^ d[22] ^ d[21] ^ d[19] ^ d[17] ^ d[13] ^ d[12] ^ d[8] ^ d[5] ^ d[4] ^ d[0] ^ c[1] ^ c[2] ^ c[5] ^ c[6] ^ c[7] ^ c[8] ^ c[11] ^ c[12] ^ c[15] ^ c[16] ^ c[17] ^ c[18] ^ c[19] ^ c[20] ^ c[21] ^ c[22] ^ c[24] ^ c[27] ^ c[28] ^ c[29]);

    SET_BIT_FROM_VALUE_MAC(newcrc,17, d[558] ^ d[557] ^ d[556] ^ d[553] ^ d[551] ^ d[550] ^ d[549] ^ d[548] ^ d[547] ^ d[546] ^ d[545] ^ d[544] ^ d[541] ^ d[540] ^ d[537] ^ d[536] ^ d[535] ^ d[534] ^ d[531] ^ d[530] ^ d[528] ^ d[526] ^ d[524] ^ d[523] ^ d[519] ^ d[517] ^ d[510] ^ d[508] ^ d[507] ^ d[506] ^ d[505] ^ d[503] ^ d[502] ^ d[500] ^ d[498] ^ d[497] ^ d[496] ^ d[495] ^ d[494] ^ d[490] ^ d[487] ^ d[484] ^ d[483] ^ d[480] ^ d[476] ^ d[475] ^ d[473] ^ d[467] ^ d[460] ^ d[459] ^ d[458] ^ d[456] ^ d[455] ^ d[453] ^ d[450] ^ d[445] ^ d[442] ^ d[441] ^ d[440] ^ d[438] ^ d[437] ^ d[434] ^ d[433] ^ d[432] ^ d[430] ^ d[428] ^ d[425] ^ d[424] ^ d[422] ^ d[421] ^ d[417] ^ d[416] ^ d[415] ^ d[414] ^ d[413] ^ d[412] ^ d[411] ^ d[409] ^ d[408] ^ d[407] ^ d[406] ^ d[405] ^ d[397] ^ d[396] ^ d[394] ^ d[393] ^ d[390] ^ d[388] ^ d[387] ^ d[385] ^ d[384] ^ d[383] ^ d[382] ^ d[380] ^ d[379] ^ d[372] ^ d[370] ^ d[368] ^ d[365] ^ d[364] ^ d[362] ^ d[358] ^ d[356] ^ d[354] ^ d[350] ^ d[349] ^ d[348] ^ d[344] ^ d[343] ^ d[342] ^ d[341] ^ d[339] ^ d[338] ^ d[337] ^ d[336] ^ d[331] ^ d[330] ^ d[328] ^ d[325] ^ d[322] ^ d[321] ^ d[320] ^ d[317] ^ d[308] ^ d[306] ^ d[304] ^ d[303] ^ d[302] ^ d[301] ^ d[300] ^ d[299] ^ d[298] ^ d[297] ^ d[293] ^ d[291] ^ d[289] ^ d[288] ^ d[287] ^ d[285] ^ d[283] ^ d[282] ^ d[277] ^ d[275] ^ d[271] ^ d[269] ^ d[265] ^ d[264] ^ d[261] ^ d[251] ^ d[250] ^ d[248] ^ d[247] ^ d[246] ^ d[244] ^ d[240] ^ d[239] ^ d[238] ^ d[234] ^ d[233] ^ d[232] ^ d[231] ^ d[229] ^ d[228] ^ d[224] ^ d[223] ^ d[222] ^ d[221] ^ d[220] ^ d[219] ^ d[217] ^ d[213] ^ d[212] ^ d[210] ^ d[208] ^ d[206] ^ d[203] ^ d[201] ^ d[200] ^ d[197] ^ d[194] ^ d[191] ^ d[190] ^ d[187] ^ d[186] ^ d[185] ^ d[184] ^ d[183] ^ d[182] ^ d[180] ^ d[175] ^ d[174] ^ d[171] ^ d[168] ^ d[161] ^ d[158] ^ d[157] ^ d[156] ^ d[154] ^ d[152] ^ d[146] ^ d[145] ^ d[144] ^ d[142] ^ d[141] ^ d[139] ^ d[137] ^ d[136] ^ d[135] ^ d[132] ^ d[129] ^ d[128] ^ d[125] ^ d[122] ^ d[121] ^ d[120] ^ d[119] ^ d[117] ^ d[116] ^ d[113] ^ d[112] ^ d[111] ^ d[110] ^ d[106] ^ d[105] ^ d[104] ^ d[103] ^ d[101] ^ d[100] ^ d[98] ^ d[95] ^ d[92] ^ d[91] ^ d[90] ^ d[88] ^ d[87] ^ d[85] ^ d[84] ^ d[83] ^ d[79] ^ d[78] ^ d[76] ^ d[69] ^ d[67] ^ d[58] ^ d[57] ^ d[52] ^ d[49] ^ d[48] ^ d[47] ^ d[45] ^ d[38] ^ d[36] ^ d[33] ^ d[31] ^ d[30] ^ d[27] ^ d[25] ^ d[23] ^ d[22] ^ d[20] ^ d[18] ^ d[14] ^ d[13] ^ d[9] ^ d[6] ^ d[5] ^ d[1] ^ c[0] ^ c[2] ^ c[3] ^ c[6] ^ c[7] ^ c[8] ^ c[9] ^ c[12] ^ c[13] ^ c[16] ^ c[17] ^ c[18] ^ c[19] ^ c[20] ^ c[21] ^ c[22] ^ c[23] ^ c[25] ^ c[28] ^ c[29] ^ c[30]);

    SET_BIT_FROM_VALUE_MAC(newcrc,18, d[559] ^ d[558] ^ d[557] ^ d[554] ^ d[552] ^ d[551] ^ d[550] ^ d[549] ^ d[548] ^ d[547] ^ d[546] ^ d[545] ^ d[542] ^ d[541] ^ d[538] ^ d[537] ^ d[536] ^ d[535] ^ d[532] ^ d[531] ^ d[529] ^ d[527] ^ d[525] ^ d[524] ^ d[520] ^ d[518] ^ d[511] ^ d[509] ^ d[508] ^ d[507] ^ d[506] ^ d[504] ^ d[503] ^ d[501] ^ d[499] ^ d[498] ^ d[497] ^ d[496] ^ d[495] ^ d[491] ^ d[488] ^ d[485] ^ d[484] ^ d[481] ^ d[477] ^ d[476] ^ d[474] ^ d[468] ^ d[461] ^ d[460] ^ d[459] ^ d[457] ^ d[456] ^ d[454] ^ d[451] ^ d[446] ^ d[443] ^ d[442] ^ d[441] ^ d[439] ^ d[438] ^ d[435] ^ d[434] ^ d[433] ^ d[431] ^ d[429] ^ d[426] ^ d[425] ^ d[423] ^ d[422] ^ d[418] ^ d[417] ^ d[416] ^ d[415] ^ d[414] ^ d[413] ^ d[412] ^ d[410] ^ d[409] ^ d[408] ^ d[407] ^ d[406] ^ d[398] ^ d[397] ^ d[395] ^ d[394] ^ d[391] ^ d[389] ^ d[388] ^ d[386] ^ d[385] ^ d[384] ^ d[383] ^ d[381] ^ d[380] ^ d[373] ^ d[371] ^ d[369] ^ d[366] ^ d[365] ^ d[363] ^ d[359] ^ d[357] ^ d[355] ^ d[351] ^ d[350] ^ d[349] ^ d[345] ^ d[344] ^ d[343] ^ d[342] ^ d[340] ^ d[339] ^ d[338] ^ d[337] ^ d[332] ^ d[331] ^ d[329] ^ d[326] ^ d[323] ^ d[322] ^ d[321] ^ d[318] ^ d[309] ^ d[307] ^ d[305] ^ d[304] ^ d[303] ^ d[302] ^ d[301] ^ d[300] ^ d[299] ^ d[298] ^ d[294] ^ d[292] ^ d[290] ^ d[289] ^ d[288] ^ d[286] ^ d[284] ^ d[283] ^ d[278] ^ d[276] ^ d[272] ^ d[270] ^ d[266] ^ d[265] ^ d[262] ^ d[252] ^ d[251] ^ d[249] ^ d[248] ^ d[247] ^ d[245] ^ d[241] ^ d[240] ^ d[239] ^ d[235] ^ d[234] ^ d[233] ^ d[232] ^ d[230] ^ d[229] ^ d[225] ^ d[224] ^ d[223] ^ d[222] ^ d[221] ^ d[220] ^ d[218] ^ d[214] ^ d[213] ^ d[211] ^ d[209] ^ d[207] ^ d[204] ^ d[202] ^ d[201] ^ d[198] ^ d[195] ^ d[192] ^ d[191] ^ d[188] ^ d[187] ^ d[186] ^ d[185] ^ d[184] ^ d[183] ^ d[181] ^ d[176] ^ d[175] ^ d[172] ^ d[169] ^ d[162] ^ d[159] ^ d[158] ^ d[157] ^ d[155] ^ d[153] ^ d[147] ^ d[146] ^ d[145] ^ d[143] ^ d[142] ^ d[140] ^ d[138] ^ d[137] ^ d[136] ^ d[133] ^ d[130] ^ d[129] ^ d[126] ^ d[123] ^ d[122] ^ d[121] ^ d[120] ^ d[118] ^ d[117] ^ d[114] ^ d[113] ^ d[112] ^ d[111] ^ d[107] ^ d[106] ^ d[105] ^ d[104] ^ d[102] ^ d[101] ^ d[99] ^ d[96] ^ d[93] ^ d[92] ^ d[91] ^ d[89] ^ d[88] ^ d[86] ^ d[85] ^ d[84] ^ d[80] ^ d[79] ^ d[77] ^ d[70] ^ d[68] ^ d[59] ^ d[58] ^ d[53] ^ d[50] ^ d[49] ^ d[48] ^ d[46] ^ d[39] ^ d[37] ^ d[34] ^ d[32] ^ d[31] ^ d[28] ^ d[26] ^ d[24] ^ d[23] ^ d[21] ^ d[19] ^ d[15] ^ d[14] ^ d[10] ^ d[7] ^ d[6] ^ d[2] ^ c[1] ^ c[3] ^ c[4] ^ c[7] ^ c[8] ^ c[9] ^ c[10] ^ c[13] ^ c[14] ^ c[17] ^ c[18] ^ c[19] ^ c[20] ^ c[21] ^ c[22] ^ c[23] ^ c[24] ^ c[26] ^ c[29] ^ c[30] ^ c[31]);

    SET_BIT_FROM_VALUE_MAC(newcrc,19, d[559] ^ d[558] ^ d[555] ^ d[553] ^ d[552] ^ d[551] ^ d[550] ^ d[549] ^ d[548] ^ d[547] ^ d[546] ^ d[543] ^ d[542] ^ d[539] ^ d[538] ^ d[537] ^ d[536] ^ d[533] ^ d[532] ^ d[530] ^ d[528] ^ d[526] ^ d[525] ^ d[521] ^ d[519] ^ d[512] ^ d[510] ^ d[509] ^ d[508] ^ d[507] ^ d[505] ^ d[504] ^ d[502] ^ d[500] ^ d[499] ^ d[498] ^ d[497] ^ d[496] ^ d[492] ^ d[489] ^ d[486] ^ d[485] ^ d[482] ^ d[478] ^ d[477] ^ d[475] ^ d[469] ^ d[462] ^ d[461] ^ d[460] ^ d[458] ^ d[457] ^ d[455] ^ d[452] ^ d[447] ^ d[444] ^ d[443] ^ d[442] ^ d[440] ^ d[439] ^ d[436] ^ d[435] ^ d[434] ^ d[432] ^ d[430] ^ d[427] ^ d[426] ^ d[424] ^ d[423] ^ d[419] ^ d[418] ^ d[417] ^ d[416] ^ d[415] ^ d[414] ^ d[413] ^ d[411] ^ d[410] ^ d[409] ^ d[408] ^ d[407] ^ d[399] ^ d[398] ^ d[396] ^ d[395] ^ d[392] ^ d[390] ^ d[389] ^ d[387] ^ d[386] ^ d[385] ^ d[384] ^ d[382] ^ d[381] ^ d[374] ^ d[372] ^ d[370] ^ d[367] ^ d[366] ^ d[364] ^ d[360] ^ d[358] ^ d[356] ^ d[352] ^ d[351] ^ d[350] ^ d[346] ^ d[345] ^ d[344] ^ d[343] ^ d[341] ^ d[340] ^ d[339] ^ d[338] ^ d[333] ^ d[332] ^ d[330] ^ d[327] ^ d[324] ^ d[323] ^ d[322] ^ d[319] ^ d[310] ^ d[308] ^ d[306] ^ d[305] ^ d[304] ^ d[303] ^ d[302] ^ d[301] ^ d[300] ^ d[299] ^ d[295] ^ d[293] ^ d[291] ^ d[290] ^ d[289] ^ d[287] ^ d[285] ^ d[284] ^ d[279] ^ d[277] ^ d[273] ^ d[271] ^ d[267] ^ d[266] ^ d[263] ^ d[253] ^ d[252] ^ d[250] ^ d[249] ^ d[248] ^ d[246] ^ d[242] ^ d[241] ^ d[240] ^ d[236] ^ d[235] ^ d[234] ^ d[233] ^ d[231] ^ d[230] ^ d[226] ^ d[225] ^ d[224] ^ d[223] ^ d[222] ^ d[221] ^ d[219] ^ d[215] ^ d[214] ^ d[212] ^ d[210] ^ d[208] ^ d[205] ^ d[203] ^ d[202] ^ d[199] ^ d[196] ^ d[193] ^ d[192] ^ d[189] ^ d[188] ^ d[187] ^ d[186] ^ d[185] ^ d[184] ^ d[182] ^ d[177] ^ d[176] ^ d[173] ^ d[170] ^ d[163] ^ d[160] ^ d[159] ^ d[158] ^ d[156] ^ d[154] ^ d[148] ^ d[147] ^ d[146] ^ d[144] ^ d[143] ^ d[141] ^ d[139] ^ d[138] ^ d[137] ^ d[134] ^ d[131] ^ d[130] ^ d[127] ^ d[124] ^ d[123] ^ d[122] ^ d[121] ^ d[119] ^ d[118] ^ d[115] ^ d[114] ^ d[113] ^ d[112] ^ d[108] ^ d[107] ^ d[106] ^ d[105] ^ d[103] ^ d[102] ^ d[100] ^ d[97] ^ d[94] ^ d[93] ^ d[92] ^ d[90] ^ d[89] ^ d[87] ^ d[86] ^ d[85] ^ d[81] ^ d[80] ^ d[78] ^ d[71] ^ d[69] ^ d[60] ^ d[59] ^ d[54] ^ d[51] ^ d[50] ^ d[49] ^ d[47] ^ d[40] ^ d[38] ^ d[35] ^ d[33] ^ d[32] ^ d[29] ^ d[27] ^ d[25] ^ d[24] ^ d[22] ^ d[20] ^ d[16] ^ d[15] ^ d[11] ^ d[8] ^ d[7] ^ d[3] ^ c[0] ^ c[2] ^ c[4] ^ c[5] ^ c[8] ^ c[9] ^ c[10] ^ c[11] ^ c[14] ^ c[15] ^ c[18] ^ c[19] ^ c[20] ^ c[21] ^ c[22] ^ c[23] ^ c[24] ^ c[25] ^ c[27] ^ c[30] ^ c[31]);

    SET_BIT_FROM_VALUE_MAC(newcrc,20, d[559] ^ d[556] ^ d[554] ^ d[553] ^ d[552] ^ d[551] ^ d[550] ^ d[549] ^ d[548] ^ d[547] ^ d[544] ^ d[543] ^ d[540] ^ d[539] ^ d[538] ^ d[537] ^ d[534] ^ d[533] ^ d[531] ^ d[529] ^ d[527] ^ d[526] ^ d[522] ^ d[520] ^ d[513] ^ d[511] ^ d[510] ^ d[509] ^ d[508] ^ d[506] ^ d[505] ^ d[503] ^ d[501] ^ d[500] ^ d[499] ^ d[498] ^ d[497] ^ d[493] ^ d[490] ^ d[487] ^ d[486] ^ d[483] ^ d[479] ^ d[478] ^ d[476] ^ d[470] ^ d[463] ^ d[462] ^ d[461] ^ d[459] ^ d[458] ^ d[456] ^ d[453] ^ d[448] ^ d[445] ^ d[444] ^ d[443] ^ d[441] ^ d[440] ^ d[437] ^ d[436] ^ d[435] ^ d[433] ^ d[431] ^ d[428] ^ d[427] ^ d[425] ^ d[424] ^ d[420] ^ d[419] ^ d[418] ^ d[417] ^ d[416] ^ d[415] ^ d[414] ^ d[412] ^ d[411] ^ d[410] ^ d[409] ^ d[408] ^ d[400] ^ d[399] ^ d[397] ^ d[396] ^ d[393] ^ d[391] ^ d[390] ^ d[388] ^ d[387] ^ d[386] ^ d[385] ^ d[383] ^ d[382] ^ d[375] ^ d[373] ^ d[371] ^ d[368] ^ d[367] ^ d[365] ^ d[361] ^ d[359] ^ d[357] ^ d[353] ^ d[352] ^ d[351] ^ d[347] ^ d[346] ^ d[345] ^ d[344] ^ d[342] ^ d[341] ^ d[340] ^ d[339] ^ d[334] ^ d[333] ^ d[331] ^ d[328] ^ d[325] ^ d[324] ^ d[323] ^ d[320] ^ d[311] ^ d[309] ^ d[307] ^ d[306] ^ d[305] ^ d[304] ^ d[303] ^ d[302] ^ d[301] ^ d[300] ^ d[296] ^ d[294] ^ d[292] ^ d[291] ^ d[290] ^ d[288] ^ d[286] ^ d[285] ^ d[280] ^ d[278] ^ d[274] ^ d[272] ^ d[268] ^ d[267] ^ d[264] ^ d[254] ^ d[253] ^ d[251] ^ d[250] ^ d[249] ^ d[247] ^ d[243] ^ d[242] ^ d[241] ^ d[237] ^ d[236] ^ d[235] ^ d[234] ^ d[232] ^ d[231] ^ d[227] ^ d[226] ^ d[225] ^ d[224] ^ d[223] ^ d[222] ^ d[220] ^ d[216] ^ d[215] ^ d[213] ^ d[211] ^ d[209] ^ d[206] ^ d[204] ^ d[203] ^ d[200] ^ d[197] ^ d[194] ^ d[193] ^ d[190] ^ d[189] ^ d[188] ^ d[187] ^ d[186] ^ d[185] ^ d[183] ^ d[178] ^ d[177] ^ d[174] ^ d[171] ^ d[164] ^ d[161] ^ d[160] ^ d[159] ^ d[157] ^ d[155] ^ d[149] ^ d[148] ^ d[147] ^ d[145] ^ d[144] ^ d[142] ^ d[140] ^ d[139] ^ d[138] ^ d[135] ^ d[132] ^ d[131] ^ d[128] ^ d[125] ^ d[124] ^ d[123] ^ d[122] ^ d[120] ^ d[119] ^ d[116] ^ d[115] ^ d[114] ^ d[113] ^ d[109] ^ d[108] ^ d[107] ^ d[106] ^ d[104] ^ d[103] ^ d[101] ^ d[98] ^ d[95] ^ d[94] ^ d[93] ^ d[91] ^ d[90] ^ d[88] ^ d[87] ^ d[86] ^ d[82] ^ d[81] ^ d[79] ^ d[72] ^ d[70] ^ d[61] ^ d[60] ^ d[55] ^ d[52] ^ d[51] ^ d[50] ^ d[48] ^ d[41] ^ d[39] ^ d[36] ^ d[34] ^ d[33] ^ d[30] ^ d[28] ^ d[26] ^ d[25] ^ d[23] ^ d[21] ^ d[17] ^ d[16] ^ d[12] ^ d[9] ^ d[8] ^ d[4] ^ c[1] ^ c[3] ^ c[5] ^ c[6] ^ c[9] ^ c[10] ^ c[11] ^ c[12] ^ c[15] ^ c[16] ^ c[19] ^ c[20] ^ c[21] ^ c[22] ^ c[23] ^ c[24] ^ c[25] ^ c[26] ^ c[28] ^ c[31]);

    SET_BIT_FROM_VALUE_MAC(newcrc,21, d[557] ^ d[555] ^ d[554] ^ d[553] ^ d[552] ^ d[551] ^ d[550] ^ d[549] ^ d[548] ^ d[545] ^ d[544] ^ d[541] ^ d[540] ^ d[539] ^ d[538] ^ d[535] ^ d[534] ^ d[532] ^ d[530] ^ d[528] ^ d[527] ^ d[523] ^ d[521] ^ d[514] ^ d[512] ^ d[511] ^ d[510] ^ d[509] ^ d[507] ^ d[506] ^ d[504] ^ d[502] ^ d[501] ^ d[500] ^ d[499] ^ d[498] ^ d[494] ^ d[491] ^ d[488] ^ d[487] ^ d[484] ^ d[480] ^ d[479] ^ d[477] ^ d[471] ^ d[464] ^ d[463] ^ d[462] ^ d[460] ^ d[459] ^ d[457] ^ d[454] ^ d[449] ^ d[446] ^ d[445] ^ d[444] ^ d[442] ^ d[441] ^ d[438] ^ d[437] ^ d[436] ^ d[434] ^ d[432] ^ d[429] ^ d[428] ^ d[426] ^ d[425] ^ d[421] ^ d[420] ^ d[419] ^ d[418] ^ d[417] ^ d[416] ^ d[415] ^ d[413] ^ d[412] ^ d[411] ^ d[410] ^ d[409] ^ d[401] ^ d[400] ^ d[398] ^ d[397] ^ d[394] ^ d[392] ^ d[391] ^ d[389] ^ d[388] ^ d[387] ^ d[386] ^ d[384] ^ d[383] ^ d[376] ^ d[374] ^ d[372] ^ d[369] ^ d[368] ^ d[366] ^ d[362] ^ d[360] ^ d[358] ^ d[354] ^ d[353] ^ d[352] ^ d[348] ^ d[347] ^ d[346] ^ d[345] ^ d[343] ^ d[342] ^ d[341] ^ d[340] ^ d[335] ^ d[334] ^ d[332] ^ d[329] ^ d[326] ^ d[325] ^ d[324] ^ d[321] ^ d[312] ^ d[310] ^ d[308] ^ d[307] ^ d[306] ^ d[305] ^ d[304] ^ d[303] ^ d[302] ^ d[301] ^ d[297] ^ d[295] ^ d[293] ^ d[292] ^ d[291] ^ d[289] ^ d[287] ^ d[286] ^ d[281] ^ d[279] ^ d[275] ^ d[273] ^ d[269] ^ d[268] ^ d[265] ^ d[255] ^ d[254] ^ d[252] ^ d[251] ^ d[250] ^ d[248] ^ d[244] ^ d[243] ^ d[242] ^ d[238] ^ d[237] ^ d[236] ^ d[235] ^ d[233] ^ d[232] ^ d[228] ^ d[227] ^ d[226] ^ d[225] ^ d[224] ^ d[223] ^ d[221] ^ d[217] ^ d[216] ^ d[214] ^ d[212] ^ d[210] ^ d[207] ^ d[205] ^ d[204] ^ d[201] ^ d[198] ^ d[195] ^ d[194] ^ d[191] ^ d[190] ^ d[189] ^ d[188] ^ d[187] ^ d[186] ^ d[184] ^ d[179] ^ d[178] ^ d[175] ^ d[172] ^ d[165] ^ d[162] ^ d[161] ^ d[160] ^ d[158] ^ d[156] ^ d[150] ^ d[149] ^ d[148] ^ d[146] ^ d[145] ^ d[143] ^ d[141] ^ d[140] ^ d[139] ^ d[136] ^ d[133] ^ d[132] ^ d[129] ^ d[126] ^ d[125] ^ d[124] ^ d[123] ^ d[121] ^ d[120] ^ d[117] ^ d[116] ^ d[115] ^ d[114] ^ d[110] ^ d[109] ^ d[108] ^ d[107] ^ d[105] ^ d[104] ^ d[102] ^ d[99] ^ d[96] ^ d[95] ^ d[94] ^ d[92] ^ d[91] ^ d[89] ^ d[88] ^ d[87] ^ d[83] ^ d[82] ^ d[80] ^ d[73] ^ d[71] ^ d[62] ^ d[61] ^ d[56] ^ d[53] ^ d[52] ^ d[51] ^ d[49] ^ d[42] ^ d[40] ^ d[37] ^ d[35] ^ d[34] ^ d[31] ^ d[29] ^ d[27] ^ d[26] ^ d[24] ^ d[22] ^ d[18] ^ d[17] ^ d[13] ^ d[10] ^ d[9] ^ d[5] ^ c[0] ^ c[2] ^ c[4] ^ c[6] ^ c[7] ^ c[10] ^ c[11] ^ c[12] ^ c[13] ^ c[16] ^ c[17] ^ c[20] ^ c[21] ^ c[22] ^ c[23] ^ c[24] ^ c[25] ^ c[26] ^ c[27] ^ c[29]);

    SET_BIT_FROM_VALUE_MAC(newcrc,22, d[557] ^ d[553] ^ d[552] ^ d[550] ^ d[546] ^ d[541] ^ d[536] ^ d[534] ^ d[533] ^ d[532] ^ d[530] ^ d[526] ^ d[525] ^ d[524] ^ d[521] ^ d[519] ^ d[518] ^ d[516] ^ d[515] ^ d[514] ^ d[513] ^ d[506] ^ d[505] ^ d[503] ^ d[499] ^ d[494] ^ d[493] ^ d[491] ^ d[490] ^ d[486] ^ d[485] ^ d[483] ^ d[482] ^ d[479] ^ d[478] ^ d[477] ^ d[476] ^ d[470] ^ d[468] ^ d[463] ^ d[462] ^ d[460] ^ d[455] ^ d[452] ^ d[449] ^ d[448] ^ d[447] ^ d[446] ^ d[445] ^ d[444] ^ d[443] ^ d[442] ^ d[439] ^ d[438] ^ d[436] ^ d[435] ^ d[434] ^ d[430] ^ d[429] ^ d[427] ^ d[426] ^ d[424] ^ d[421] ^ d[420] ^ d[417] ^ d[413] ^ d[411] ^ d[410] ^ d[409] ^ d[408] ^ d[407] ^ d[405] ^ d[404] ^ d[402] ^ d[401] ^ d[400] ^ d[396] ^ d[395] ^ d[391] ^ d[389] ^ d[386] ^ d[385] ^ d[384] ^ d[381] ^ d[378] ^ d[377] ^ d[376] ^ d[375] ^ d[374] ^ d[373] ^ d[372] ^ d[370] ^ d[368] ^ d[367] ^ d[366] ^ d[362] ^ d[361] ^ d[358] ^ d[357] ^ d[355] ^ d[354] ^ d[346] ^ d[345] ^ d[343] ^ d[339] ^ d[338] ^ d[337] ^ d[336] ^ d[334] ^ d[330] ^ d[328] ^ d[326] ^ d[325] ^ d[321] ^ d[320] ^ d[319] ^ d[318] ^ d[317] ^ d[315] ^ d[313] ^ d[312] ^ d[311] ^ d[310] ^ d[308] ^ d[307] ^ d[306] ^ d[304] ^ d[300] ^ d[299] ^ d[297] ^ d[295] ^ d[293] ^ d[286] ^ d[283] ^ d[282] ^ d[280] ^ d[279] ^ d[277] ^ d[273] ^ d[270] ^ d[268] ^ d[266] ^ d[265] ^ d[264] ^ d[261] ^ d[259] ^ d[257] ^ d[256] ^ d[253] ^ d[251] ^ d[249] ^ d[248] ^ d[245] ^ d[244] ^ d[239] ^ d[238] ^ d[236] ^ d[233] ^ d[230] ^ d[229] ^ d[225] ^ d[222] ^ d[218] ^ d[217] ^ d[216] ^ d[215] ^ d[214] ^ d[213] ^ d[212] ^ d[211] ^ d[210] ^ d[209] ^ d[207] ^ d[206] ^ d[205] ^ d[203] ^ d[201] ^ d[198] ^ d[197] ^ d[196] ^ d[195] ^ d[194] ^ d[193] ^ d[189] ^ d[187] ^ d[186] ^ d[185] ^ d[183] ^ d[182] ^ d[180] ^ d[179] ^ d[176] ^ d[173] ^ d[172] ^ d[171] ^ d[170] ^ d[169] ^ d[167] ^ d[163] ^ d[159] ^ d[158] ^ d[157] ^ d[156] ^ d[155] ^ d[150] ^ d[147] ^ d[146] ^ d[143] ^ d[142] ^ d[141] ^ d[140] ^ d[136] ^ d[135] ^ d[133] ^ d[132] ^ d[130] ^ d[128] ^ d[124] ^ d[123] ^ d[122] ^ d[121] ^ d[119] ^ d[115] ^ d[114] ^ d[113] ^ d[109] ^ d[108] ^ d[105] ^ d[104] ^ d[101] ^ d[100] ^ d[99] ^ d[98] ^ d[94] ^ d[93] ^ d[92] ^ d[90] ^ d[89] ^ d[88] ^ d[87] ^ d[85] ^ d[82] ^ d[79] ^ d[74] ^ d[73] ^ d[68] ^ d[67] ^ d[66] ^ d[65] ^ d[62] ^ d[61] ^ d[60] ^ d[58] ^ d[57] ^ d[55] ^ d[52] ^ d[48] ^ d[47] ^ d[45] ^ d[44] ^ d[43] ^ d[41] ^ d[38] ^ d[37] ^ d[36] ^ d[35] ^ d[34] ^ d[31] ^ d[29] ^ d[27] ^ d[26] ^ d[24] ^ d[23] ^ d[19] ^ d[18] ^ d[16] ^ d[14] ^ d[12] ^ d[11] ^ d[9] ^ d[0] ^ c[2] ^ c[4] ^ c[5] ^ c[6] ^ c[8] ^ c[13] ^ c[18] ^ c[22] ^ c[24] ^ c[25] ^ c[29]);

    SET_BIT_FROM_VALUE_MAC(newcrc,23, d[557] ^ d[556] ^ d[555] ^ d[553] ^ d[549] ^ d[547] ^ d[545] ^ d[540] ^ d[539] ^ d[537] ^ d[533] ^ d[532] ^ d[530] ^ d[529] ^ d[528] ^ d[527] ^ d[521] ^ d[520] ^ d[518] ^ d[517] ^ d[515] ^ d[512] ^ d[511] ^ d[510] ^ d[508] ^ d[504] ^ d[502] ^ d[501] ^ d[493] ^ d[490] ^ d[489] ^ d[488] ^ d[487] ^ d[484] ^ d[482] ^ d[481] ^ d[478] ^ d[476] ^ d[472] ^ d[471] ^ d[470] ^ d[469] ^ d[468] ^ d[465] ^ d[463] ^ d[462] ^ d[458] ^ d[456] ^ d[453] ^ d[452] ^ d[447] ^ d[446] ^ d[445] ^ d[443] ^ d[440] ^ d[439] ^ d[435] ^ d[434] ^ d[433] ^ d[431] ^ d[430] ^ d[428] ^ d[427] ^ d[425] ^ d[424] ^ d[421] ^ d[419] ^ d[416] ^ d[411] ^ d[410] ^ d[407] ^ d[406] ^ d[404] ^ d[403] ^ d[402] ^ d[401] ^ d[400] ^ d[399] ^ d[398] ^ d[397] ^ d[393] ^ d[391] ^ d[388] ^ d[385] ^ d[382] ^ d[381] ^ d[379] ^ d[377] ^ d[375] ^ d[373] ^ d[372] ^ d[371] ^ d[367] ^ d[366] ^ d[357] ^ d[356] ^ d[355] ^ d[353] ^ d[349] ^ d[348] ^ d[346] ^ d[345] ^ d[342] ^ d[341] ^ d[340] ^ d[334] ^ d[333] ^ d[331] ^ d[329] ^ d[328] ^ d[326] ^ d[317] ^ d[316] ^ d[315] ^ d[314] ^ d[313] ^ d[311] ^ d[310] ^ d[308] ^ d[307] ^ d[303] ^ d[302] ^ d[301] ^ d[299] ^ d[297] ^ d[295] ^ d[292] ^ d[290] ^ d[288] ^ d[286] ^ d[284] ^ d[281] ^ d[280] ^ d[279] ^ d[278] ^ d[277] ^ d[276] ^ d[273] ^ d[271] ^ d[268] ^ d[267] ^ d[266] ^ d[264] ^ d[262] ^ d[261] ^ d[260] ^ d[259] ^ d[258] ^ d[255] ^ d[254] ^ d[250] ^ d[249] ^ d[248] ^ d[246] ^ d[245] ^ d[243] ^ d[240] ^ d[239] ^ d[231] ^ d[228] ^ d[227] ^ d[224] ^ d[223] ^ d[219] ^ d[218] ^ d[217] ^ d[215] ^ d[213] ^ d[211] ^ d[209] ^ d[206] ^ d[204] ^ d[203] ^ d[201] ^ d[196] ^ d[195] ^ d[193] ^ d[192] ^ d[191] ^ d[187] ^ d[184] ^ d[182] ^ d[181] ^ d[180] ^ d[177] ^ d[174] ^ d[173] ^ d[169] ^ d[168] ^ d[167] ^ d[166] ^ d[164] ^ d[162] ^ d[161] ^ d[160] ^ d[159] ^ d[157] ^ d[155] ^ d[149] ^ d[148] ^ d[147] ^ d[142] ^ d[141] ^ d[135] ^ d[133] ^ d[132] ^ d[131] ^ d[129] ^ d[128] ^ d[127] ^ d[126] ^ d[124] ^ d[122] ^ d[120] ^ d[119] ^ d[118] ^ d[117] ^ d[115] ^ d[113] ^ d[111] ^ d[109] ^ d[105] ^ d[104] ^ d[103] ^ d[102] ^ d[100] ^ d[98] ^ d[97] ^ d[96] ^ d[93] ^ d[91] ^ d[90] ^ d[89] ^ d[88] ^ d[87] ^ d[86] ^ d[85] ^ d[84] ^ d[82] ^ d[81] ^ d[80] ^ d[79] ^ d[75] ^ d[74] ^ d[73] ^ d[72] ^ d[69] ^ d[65] ^ d[62] ^ d[60] ^ d[59] ^ d[56] ^ d[55] ^ d[54] ^ d[50] ^ d[49] ^ d[47] ^ d[46] ^ d[42] ^ d[39] ^ d[38] ^ d[36] ^ d[35] ^ d[34] ^ d[31] ^ d[29] ^ d[27] ^ d[26] ^ d[20] ^ d[19] ^ d[17] ^ d[16] ^ d[15] ^ d[13] ^ d[9] ^ d[6] ^ d[1] ^ d[0] ^ c[0] ^ c[1] ^ c[2] ^ c[4] ^ c[5] ^ c[9] ^ c[11] ^ c[12] ^ c[17] ^ c[19] ^ c[21] ^ c[25] ^ c[27] ^ c[28] ^ c[29]);

    SET_BIT_FROM_VALUE_MAC(newcrc,24, d[558] ^ d[557] ^ d[556] ^ d[554] ^ d[550] ^ d[548] ^ d[546] ^ d[541] ^ d[540] ^ d[538] ^ d[534] ^ d[533] ^ d[531] ^ d[530] ^ d[529] ^ d[528] ^ d[522] ^ d[521] ^ d[519] ^ d[518] ^ d[516] ^ d[513] ^ d[512] ^ d[511] ^ d[509] ^ d[505] ^ d[503] ^ d[502] ^ d[494] ^ d[491] ^ d[490] ^ d[489] ^ d[488] ^ d[485] ^ d[483] ^ d[482] ^ d[479] ^ d[477] ^ d[473] ^ d[472] ^ d[471] ^ d[470] ^ d[469] ^ d[466] ^ d[464] ^ d[463] ^ d[459] ^ d[457] ^ d[454] ^ d[453] ^ d[448] ^ d[447] ^ d[446] ^ d[444] ^ d[441] ^ d[440] ^ d[436] ^ d[435] ^ d[434] ^ d[432] ^ d[431] ^ d[429] ^ d[428] ^ d[426] ^ d[425] ^ d[422] ^ d[420] ^ d[417] ^ d[412] ^ d[411] ^ d[408] ^ d[407] ^ d[405] ^ d[404] ^ d[403] ^ d[402] ^ d[401] ^ d[400] ^ d[399] ^ d[398] ^ d[394] ^ d[392] ^ d[389] ^ d[386] ^ d[383] ^ d[382] ^ d[380] ^ d[378] ^ d[376] ^ d[374] ^ d[373] ^ d[372] ^ d[368] ^ d[367] ^ d[358] ^ d[357] ^ d[356] ^ d[354] ^ d[350] ^ d[349] ^ d[347] ^ d[346] ^ d[343] ^ d[342] ^ d[341] ^ d[335] ^ d[334] ^ d[332] ^ d[330] ^ d[329] ^ d[327] ^ d[318] ^ d[317] ^ d[316] ^ d[315] ^ d[314] ^ d[312] ^ d[311] ^ d[309] ^ d[308] ^ d[304] ^ d[303] ^ d[302] ^ d[300] ^ d[298] ^ d[296] ^ d[293] ^ d[291] ^ d[289] ^ d[287] ^ d[285] ^ d[282] ^ d[281] ^ d[280] ^ d[279] ^ d[278] ^ d[277] ^ d[274] ^ d[272] ^ d[269] ^ d[268] ^ d[267] ^ d[265] ^ d[263] ^ d[262] ^ d[261] ^ d[260] ^ d[259] ^ d[256] ^ d[255] ^ d[251] ^ d[250] ^ d[249] ^ d[247] ^ d[246] ^ d[244] ^ d[241] ^ d[240] ^ d[232] ^ d[229] ^ d[228] ^ d[225] ^ d[224] ^ d[220] ^ d[219] ^ d[218] ^ d[216] ^ d[214] ^ d[212] ^ d[210] ^ d[207] ^ d[205] ^ d[204] ^ d[202] ^ d[197] ^ d[196] ^ d[194] ^ d[193] ^ d[192] ^ d[188] ^ d[185] ^ d[183] ^ d[182] ^ d[181] ^ d[178] ^ d[175] ^ d[174] ^ d[170] ^ d[169] ^ d[168] ^ d[167] ^ d[165] ^ d[163] ^ d[162] ^ d[161] ^ d[160] ^ d[158] ^ d[156] ^ d[150] ^ d[149] ^ d[148] ^ d[143] ^ d[142] ^ d[136] ^ d[134] ^ d[133] ^ d[132] ^ d[130] ^ d[129] ^ d[128] ^ d[127] ^ d[125] ^ d[123] ^ d[121] ^ d[120] ^ d[119] ^ d[118] ^ d[116] ^ d[114] ^ d[112] ^ d[110] ^ d[106] ^ d[105] ^ d[104] ^ d[103] ^ d[101] ^ d[99] ^ d[98] ^ d[97] ^ d[94] ^ d[92] ^ d[91] ^ d[90] ^ d[89] ^ d[88] ^ d[87] ^ d[86] ^ d[85] ^ d[83] ^ d[82] ^ d[81] ^ d[80] ^ d[76] ^ d[75] ^ d[74] ^ d[73] ^ d[70] ^ d[66] ^ d[63] ^ d[61] ^ d[60] ^ d[57] ^ d[56] ^ d[55] ^ d[51] ^ d[50] ^ d[48] ^ d[47] ^ d[43] ^ d[40] ^ d[39] ^ d[37] ^ d[36] ^ d[35] ^ d[32] ^ d[30] ^ d[28] ^ d[27] ^ d[21] ^ d[20] ^ d[18] ^ d[17] ^ d[16] ^ d[14] ^ d[10] ^ d[7] ^ d[2] ^ d[1] ^ c[0] ^ c[1] ^ c[2] ^ c[3] ^ c[5] ^ c[6] ^ c[10] ^ c[12] ^ c[13] ^ c[18] ^ c[20] ^ c[22] ^ c[26] ^ c[28] ^ c[29] ^ c[30]);

    SET_BIT_FROM_VALUE_MAC(newcrc,25, d[559] ^ d[558] ^ d[557] ^ d[555] ^ d[551] ^ d[549] ^ d[547] ^ d[542] ^ d[541] ^ d[539] ^ d[535] ^ d[534] ^ d[532] ^ d[531] ^ d[530] ^ d[529] ^ d[523] ^ d[522] ^ d[520] ^ d[519] ^ d[517] ^ d[514] ^ d[513] ^ d[512] ^ d[510] ^ d[506] ^ d[504] ^ d[503] ^ d[495] ^ d[492] ^ d[491] ^ d[490] ^ d[489] ^ d[486] ^ d[484] ^ d[483] ^ d[480] ^ d[478] ^ d[474] ^ d[473] ^ d[472] ^ d[471] ^ d[470] ^ d[467] ^ d[465] ^ d[464] ^ d[460] ^ d[458] ^ d[455] ^ d[454] ^ d[449] ^ d[448] ^ d[447] ^ d[445] ^ d[442] ^ d[441] ^ d[437] ^ d[436] ^ d[435] ^ d[433] ^ d[432] ^ d[430] ^ d[429] ^ d[427] ^ d[426] ^ d[423] ^ d[421] ^ d[418] ^ d[413] ^ d[412] ^ d[409] ^ d[408] ^ d[406] ^ d[405] ^ d[404] ^ d[403] ^ d[402] ^ d[401] ^ d[400] ^ d[399] ^ d[395] ^ d[393] ^ d[390] ^ d[387] ^ d[384] ^ d[383] ^ d[381] ^ d[379] ^ d[377] ^ d[375] ^ d[374] ^ d[373] ^ d[369] ^ d[368] ^ d[359] ^ d[358] ^ d[357] ^ d[355] ^ d[351] ^ d[350] ^ d[348] ^ d[347] ^ d[344] ^ d[343] ^ d[342] ^ d[336] ^ d[335] ^ d[333] ^ d[331] ^ d[330] ^ d[328] ^ d[319] ^ d[318] ^ d[317] ^ d[316] ^ d[315] ^ d[313] ^ d[312] ^ d[310] ^ d[309] ^ d[305] ^ d[304] ^ d[303] ^ d[301] ^ d[299] ^ d[297] ^ d[294] ^ d[292] ^ d[290] ^ d[288] ^ d[286] ^ d[283] ^ d[282] ^ d[281] ^ d[280] ^ d[279] ^ d[278] ^ d[275] ^ d[273] ^ d[270] ^ d[269] ^ d[268] ^ d[266] ^ d[264] ^ d[263] ^ d[262] ^ d[261] ^ d[260] ^ d[257] ^ d[256] ^ d[252] ^ d[251] ^ d[250] ^ d[248] ^ d[247] ^ d[245] ^ d[242] ^ d[241] ^ d[233] ^ d[230] ^ d[229] ^ d[226] ^ d[225] ^ d[221] ^ d[220] ^ d[219] ^ d[217] ^ d[215] ^ d[213] ^ d[211] ^ d[208] ^ d[206] ^ d[205] ^ d[203] ^ d[198] ^ d[197] ^ d[195] ^ d[194] ^ d[193] ^ d[189] ^ d[186] ^ d[184] ^ d[183] ^ d[182] ^ d[179] ^ d[176] ^ d[175] ^ d[171] ^ d[170] ^ d[169] ^ d[168] ^ d[166] ^ d[164] ^ d[163] ^ d[162] ^ d[161] ^ d[159] ^ d[157] ^ d[151] ^ d[150] ^ d[149] ^ d[144] ^ d[143] ^ d[137] ^ d[135] ^ d[134] ^ d[133] ^ d[131] ^ d[130] ^ d[129] ^ d[128] ^ d[126] ^ d[124] ^ d[122] ^ d[121] ^ d[120] ^ d[119] ^ d[117] ^ d[115] ^ d[113] ^ d[111] ^ d[107] ^ d[106] ^ d[105] ^ d[104] ^ d[102] ^ d[100] ^ d[99] ^ d[98] ^ d[95] ^ d[93] ^ d[92] ^ d[91] ^ d[90] ^ d[89] ^ d[88] ^ d[87] ^ d[86] ^ d[84] ^ d[83] ^ d[82] ^ d[81] ^ d[77] ^ d[76] ^ d[75] ^ d[74] ^ d[71] ^ d[67] ^ d[64] ^ d[62] ^ d[61] ^ d[58] ^ d[57] ^ d[56] ^ d[52] ^ d[51] ^ d[49] ^ d[48] ^ d[44] ^ d[41] ^ d[40] ^ d[38] ^ d[37] ^ d[36] ^ d[33] ^ d[31] ^ d[29] ^ d[28] ^ d[22] ^ d[21] ^ d[19] ^ d[18] ^ d[17] ^ d[15] ^ d[11] ^ d[8] ^ d[3] ^ d[2] ^ c[1] ^ c[2] ^ c[3] ^ c[4] ^ c[6] ^ c[7] ^ c[11] ^ c[13] ^ c[14] ^ c[19] ^ c[21] ^ c[23] ^ c[27] ^ c[29] ^ c[30] ^ c[31]);

    SET_BIT_FROM_VALUE_MAC(newcrc,26, d[559] ^ d[557] ^ d[555] ^ d[554] ^ d[552] ^ d[551] ^ d[550] ^ d[549] ^ d[548] ^ d[545] ^ d[543] ^ d[539] ^ d[536] ^ d[534] ^ d[533] ^ d[529] ^ d[528] ^ d[526] ^ d[525] ^ d[524] ^ d[523] ^ d[522] ^ d[520] ^ d[519] ^ d[516] ^ d[515] ^ d[513] ^ d[512] ^ d[510] ^ d[508] ^ d[506] ^ d[505] ^ d[504] ^ d[502] ^ d[501] ^ d[500] ^ d[496] ^ d[495] ^ d[494] ^ d[489] ^ d[488] ^ d[487] ^ d[486] ^ d[485] ^ d[484] ^ d[483] ^ d[482] ^ d[480] ^ d[477] ^ d[476] ^ d[475] ^ d[474] ^ d[473] ^ d[471] ^ d[470] ^ d[466] ^ d[464] ^ d[462] ^ d[459] ^ d[458] ^ d[456] ^ d[455] ^ d[452] ^ d[446] ^ d[444] ^ d[443] ^ d[442] ^ d[438] ^ d[431] ^ d[430] ^ d[428] ^ d[427] ^ d[418] ^ d[416] ^ d[413] ^ d[412] ^ d[410] ^ d[408] ^ d[406] ^ d[403] ^ d[402] ^ d[401] ^ d[399] ^ d[398] ^ d[394] ^ d[393] ^ d[392] ^ d[390] ^ d[387] ^ d[386] ^ d[385] ^ d[384] ^ d[382] ^ d[381] ^ d[380] ^ d[375] ^ d[372] ^ d[370] ^ d[368] ^ d[366] ^ d[363] ^ d[362] ^ d[360] ^ d[357] ^ d[356] ^ d[353] ^ d[352] ^ d[351] ^ d[347] ^ d[343] ^ d[342] ^ d[341] ^ d[339] ^ d[338] ^ d[336] ^ d[335] ^ d[333] ^ d[332] ^ d[331] ^ d[329] ^ d[328] ^ d[327] ^ d[322] ^ d[321] ^ d[316] ^ d[315] ^ d[314] ^ d[313] ^ d[312] ^ d[311] ^ d[309] ^ d[306] ^ d[304] ^ d[303] ^ d[299] ^ d[297] ^ d[296] ^ d[294] ^ d[293] ^ d[292] ^ d[291] ^ d[290] ^ d[289] ^ d[288] ^ d[286] ^ d[284] ^ d[282] ^ d[281] ^ d[280] ^ d[277] ^ d[273] ^ d[271] ^ d[270] ^ d[268] ^ d[267] ^ d[263] ^ d[262] ^ d[259] ^ d[258] ^ d[255] ^ d[253] ^ d[251] ^ d[249] ^ d[246] ^ d[242] ^ d[237] ^ d[231] ^ d[228] ^ d[224] ^ d[222] ^ d[221] ^ d[220] ^ d[218] ^ d[210] ^ d[208] ^ d[206] ^ d[204] ^ d[203] ^ d[202] ^ d[201] ^ d[197] ^ d[196] ^ d[195] ^ d[193] ^ d[192] ^ d[191] ^ d[188] ^ d[187] ^ d[186] ^ d[185] ^ d[184] ^ d[182] ^ d[180] ^ d[177] ^ d[176] ^ d[166] ^ d[165] ^ d[164] ^ d[163] ^ d[161] ^ d[160] ^ d[156] ^ d[155] ^ d[152] ^ d[150] ^ d[149] ^ d[145] ^ d[143] ^ d[138] ^ d[137] ^ d[131] ^ d[130] ^ d[129] ^ d[128] ^ d[126] ^ d[122] ^ d[121] ^ d[120] ^ d[119] ^ d[117] ^ d[113] ^ d[112] ^ d[111] ^ d[110] ^ d[108] ^ d[107] ^ d[105] ^ d[104] ^ d[100] ^ d[98] ^ d[97] ^ d[95] ^ d[93] ^ d[92] ^ d[91] ^ d[90] ^ d[89] ^ d[88] ^ d[81] ^ d[79] ^ d[78] ^ d[77] ^ d[76] ^ d[75] ^ d[73] ^ d[67] ^ d[66] ^ d[62] ^ d[61] ^ d[60] ^ d[59] ^ d[57] ^ d[55] ^ d[54] ^ d[52] ^ d[49] ^ d[48] ^ d[47] ^ d[44] ^ d[42] ^ d[41] ^ d[39] ^ d[38] ^ d[31] ^ d[28] ^ d[26] ^ d[25] ^ d[24] ^ d[23] ^ d[22] ^ d[20] ^ d[19] ^ d[18] ^ d[10] ^ d[6] ^ d[4] ^ d[3] ^ d[0] ^ c[0] ^ c[1] ^ c[5] ^ c[6] ^ c[8] ^ c[11] ^ c[15] ^ c[17] ^ c[20] ^ c[21] ^ c[22] ^ c[23] ^ c[24] ^ c[26] ^ c[27] ^ c[29] ^ c[31]);

    SET_BIT_FROM_VALUE_MAC(newcrc,27, d[558] ^ d[556] ^ d[555] ^ d[553] ^ d[552] ^ d[551] ^ d[550] ^ d[549] ^ d[546] ^ d[544] ^ d[540] ^ d[537] ^ d[535] ^ d[534] ^ d[530] ^ d[529] ^d[527] ^ d[526] ^ d[525] ^ d[524] ^ d[523] ^ d[521] ^ d[520] ^ d[517] ^ d[516] ^ d[514] ^ d[513] ^ d[511] ^ d[509] ^ d[507] ^ d[506] ^ d[505] ^d[503] ^ d[502] ^ d[501] ^ d[497] ^ d[496] ^ d[495] ^ d[490] ^ d[489] ^ d[488] ^ d[487] ^ d[486] ^ d[485] ^ d[484] ^ d[483] ^ d[481] ^ d[478] ^d[477] ^ d[476] ^ d[475] ^ d[474] ^ d[472] ^ d[471] ^ d[467] ^ d[465] ^ d[463] ^ d[460] ^ d[459] ^ d[457] ^ d[456] ^ d[453] ^ d[447] ^ d[445] ^d[444] ^ d[443] ^ d[439] ^ d[432] ^ d[431] ^ d[429] ^ d[428] ^ d[419] ^ d[417] ^ d[414] ^ d[413] ^ d[411] ^ d[409] ^ d[407] ^ d[404] ^ d[403] ^d[402] ^ d[400] ^ d[399] ^ d[395] ^ d[394] ^ d[393] ^ d[391] ^ d[388] ^ d[387] ^ d[386] ^ d[385] ^ d[383] ^ d[382] ^ d[381] ^ d[376] ^ d[373] ^d[371] ^ d[369] ^ d[367] ^ d[364] ^ d[363] ^ d[361] ^ d[358] ^ d[357] ^ d[354] ^ d[353] ^ d[352] ^ d[348] ^ d[344] ^ d[343] ^ d[342] ^ d[340] ^d[339] ^ d[337] ^ d[336] ^ d[334] ^ d[333] ^ d[332] ^ d[330] ^ d[329] ^ d[328] ^ d[323] ^ d[322] ^ d[317] ^ d[316] ^ d[315] ^ d[314] ^ d[313] ^d[312] ^ d[310] ^ d[307] ^ d[305] ^ d[304] ^ d[300] ^ d[298] ^ d[297] ^ d[295] ^ d[294] ^ d[293] ^ d[292] ^ d[291] ^ d[290] ^ d[289] ^ d[287] ^d[285] ^ d[283] ^ d[282] ^ d[281] ^ d[278] ^ d[274] ^ d[272] ^ d[271] ^ d[269] ^ d[268] ^ d[264] ^ d[263] ^ d[260] ^ d[259] ^ d[256] ^ d[254] ^d[252] ^ d[250] ^ d[247] ^ d[243] ^ d[238] ^ d[232] ^ d[229] ^ d[225] ^ d[223] ^ d[222] ^ d[221] ^ d[219] ^ d[211] ^ d[209] ^ d[207] ^ d[205] ^d[204] ^ d[203] ^ d[202] ^ d[198] ^ d[197] ^ d[196] ^ d[194] ^ d[193] ^ d[192] ^ d[189] ^ d[188] ^ d[187] ^ d[186] ^ d[185] ^ d[183] ^ d[181] ^d[178] ^ d[177] ^ d[167] ^ d[166] ^ d[165] ^ d[164] ^ d[162] ^ d[161] ^ d[157] ^ d[156] ^ d[153] ^ d[151] ^ d[150] ^ d[146] ^ d[144] ^ d[139] ^d[138] ^ d[132] ^ d[131] ^ d[130] ^ d[129] ^ d[127] ^ d[123] ^ d[122] ^ d[121] ^ d[120] ^ d[118] ^ d[114] ^ d[113] ^ d[112] ^ d[111] ^ d[109] ^d[108] ^ d[106] ^ d[105] ^ d[101] ^ d[99] ^ d[98] ^ d[96] ^ d[94] ^ d[93] ^ d[92] ^ d[91] ^ d[90] ^ d[89] ^ d[82] ^ d[80] ^ d[79] ^ d[78] ^ d[77] ^d[76] ^ d[74] ^ d[68] ^ d[67] ^ d[63] ^ d[62] ^ d[61] ^ d[60] ^ d[58] ^ d[56] ^ d[55] ^ d[53] ^ d[50] ^ d[49] ^ d[48] ^ d[45] ^ d[43] ^ d[42] ^ d[40] ^d[39] ^ d[32] ^ d[29] ^ d[27] ^ d[26] ^ d[25] ^ d[24] ^ d[23] ^ d[21] ^ d[20] ^ d[19] ^ d[11] ^ d[7] ^ d[5] ^ d[4] ^ d[1] ^ c[1] ^ c[2] ^ c[6] ^ c[7] ^c[9] ^ c[12] ^ c[16] ^ c[18] ^ c[21] ^ c[22] ^ c[23] ^ c[24] ^ c[25] ^ c[27] ^ c[28] ^ c[30]);

    SET_BIT_FROM_VALUE_MAC(newcrc,28, d[559] ^ d[557] ^ d[556] ^ d[554] ^ d[553] ^ d[552] ^ d[551] ^ d[550] ^ d[547] ^ d[545] ^ d[541] ^ d[538] ^ d[536] ^ d[535] ^ d[531] ^ d[530] ^ d[528] ^ d[527] ^ d[526] ^ d[525] ^ d[524] ^ d[522] ^ d[521] ^ d[518] ^ d[517] ^ d[515] ^ d[514] ^ d[512] ^ d[510] ^ d[508] ^ d[507] ^ d[506] ^ d[504] ^ d[503] ^ d[502] ^ d[498] ^ d[497] ^ d[496] ^ d[491] ^ d[490] ^ d[489] ^ d[488] ^ d[487] ^ d[486] ^ d[485] ^ d[484] ^ d[482] ^ d[479] ^ d[478] ^ d[477] ^ d[476] ^ d[475] ^ d[473] ^ d[472] ^ d[468] ^ d[466] ^ d[464] ^ d[461] ^ d[460] ^ d[458] ^ d[457] ^ d[454] ^ d[448] ^ d[446] ^ d[445] ^ d[444] ^ d[440] ^ d[433] ^ d[432] ^ d[430] ^ d[429] ^ d[420] ^ d[418] ^ d[415] ^ d[414] ^ d[412] ^ d[410] ^ d[408] ^ d[405] ^ d[404] ^ d[403] ^ d[401] ^ d[400] ^ d[396] ^ d[395] ^ d[394] ^ d[392] ^ d[389] ^ d[388] ^ d[387] ^ d[386] ^ d[384] ^ d[383] ^ d[382] ^ d[377] ^ d[374] ^ d[372] ^ d[370] ^ d[368] ^ d[365] ^ d[364] ^ d[362] ^ d[359] ^ d[358] ^ d[355] ^ d[354] ^ d[353] ^ d[349] ^ d[345] ^ d[344] ^ d[343] ^ d[341] ^ d[340] ^ d[338] ^ d[337] ^ d[335] ^ d[334] ^ d[333] ^ d[331] ^ d[330] ^ d[329] ^ d[324] ^ d[323] ^ d[318] ^ d[317] ^ d[316] ^ d[315] ^ d[314] ^ d[313] ^ d[311] ^ d[308] ^ d[306] ^ d[305] ^ d[301] ^ d[299] ^ d[298] ^ d[296] ^ d[295] ^ d[294] ^ d[293] ^ d[292] ^ d[291] ^ d[290] ^ d[288] ^ d[286] ^ d[284] ^ d[283] ^ d[282] ^ d[279] ^ d[275] ^ d[273] ^ d[272] ^ d[270] ^ d[269] ^ d[265] ^ d[264] ^ d[261] ^ d[260] ^ d[257] ^ d[255] ^ d[253] ^ d[251] ^ d[248] ^ d[244] ^ d[239] ^ d[233] ^ d[230] ^ d[226] ^ d[224] ^ d[223] ^ d[222] ^ d[220] ^ d[212] ^ d[210] ^ d[208] ^ d[206] ^ d[205] ^ d[204] ^ d[203] ^ d[199] ^ d[198] ^ d[197] ^ d[195] ^ d[194] ^ d[193] ^ d[190] ^ d[189] ^ d[188] ^ d[187] ^ d[186] ^ d[184] ^ d[182] ^ d[179] ^ d[178] ^ d[168] ^ d[167] ^ d[166] ^ d[165] ^ d[163] ^ d[162] ^ d[158] ^ d[157] ^ d[154] ^ d[152] ^ d[151] ^ d[147] ^ d[145] ^ d[140] ^ d[139] ^ d[133] ^ d[132] ^ d[131] ^ d[130] ^ d[128] ^ d[124] ^ d[123] ^ d[122] ^ d[121] ^ d[119] ^ d[115] ^ d[114] ^ d[113] ^ d[112] ^ d[110] ^ d[109] ^ d[107] ^ d[106] ^ d[102] ^ d[100] ^ d[99] ^ d[97] ^ d[95] ^ d[94] ^ d[93] ^ d[92] ^ d[91] ^ d[90] ^ d[83] ^ d[81] ^ d[80] ^ d[79] ^ d[78] ^ d[77] ^ d[75] ^ d[69] ^ d[68] ^ d[64] ^ d[63] ^ d[62] ^ d[61] ^ d[59] ^ d[57] ^ d[56] ^ d[54] ^ d[51] ^ d[50] ^ d[49] ^ d[46] ^ d[44] ^ d[43] ^ d[41] ^ d[40] ^ d[33] ^ d[30] ^ d[28] ^ d[27] ^ d[26] ^ d[25] ^ d[24] ^ d[22] ^ d[21] ^ d[20] ^ d[12] ^ d[8] ^ d[6] ^ d[5] ^ d[2] ^ c[0] ^ c[2] ^ c[3] ^ c[7] ^ c[8] ^ c[10] ^ c[13] ^ c[17] ^ c[19] ^ c[22] ^ c[23] ^ c[24] ^ c[25] ^ c[26] ^ c[28] ^ c[29] ^ c[31]);

    SET_BIT_FROM_VALUE_MAC(newcrc,29, d[558] ^ d[557] ^ d[555] ^ d[554] ^ d[553] ^ d[552] ^ d[551] ^ d[548] ^ d[546] ^ d[542] ^ d[539] ^ d[537] ^ d[536] ^ d[532] ^ d[531] ^ d[529] ^ d[528] ^ d[527] ^ d[526] ^ d[525] ^ d[523] ^ d[522] ^ d[519] ^ d[518] ^ d[516] ^ d[515] ^ d[513] ^ d[511] ^ d[509] ^ d[508] ^ d[507] ^ d[505] ^ d[504] ^ d[503] ^ d[499] ^ d[498] ^ d[497] ^ d[492] ^ d[491] ^ d[490] ^ d[489] ^ d[488] ^ d[487] ^ d[486] ^ d[485] ^ d[483] ^ d[480] ^ d[479] ^ d[478] ^ d[477] ^ d[476] ^ d[474] ^ d[473] ^ d[469] ^ d[467] ^ d[465] ^ d[462] ^ d[461] ^ d[459] ^ d[458] ^ d[455] ^ d[449] ^ d[447] ^ d[446] ^ d[445] ^ d[441] ^ d[434] ^ d[433] ^ d[431] ^ d[430] ^ d[421] ^ d[419] ^ d[416] ^ d[415] ^ d[413] ^ d[411] ^ d[409] ^ d[406] ^ d[405] ^ d[404] ^ d[402] ^ d[401] ^ d[397] ^ d[396] ^ d[395] ^ d[393] ^ d[390] ^ d[389] ^ d[388] ^ d[387] ^ d[385] ^ d[384] ^ d[383] ^ d[378] ^ d[375] ^ d[373] ^ d[371] ^ d[369] ^ d[366] ^ d[365] ^ d[363] ^ d[360] ^ d[359] ^ d[356] ^ d[355] ^ d[354] ^ d[350] ^ d[346] ^ d[345] ^ d[344] ^ d[342] ^ d[341] ^ d[339] ^ d[338] ^ d[336] ^ d[335] ^ d[334] ^ d[332] ^ d[331] ^ d[330] ^ d[325] ^ d[324] ^ d[319] ^ d[318] ^ d[317] ^ d[316] ^ d[315] ^ d[314] ^ d[312] ^ d[309] ^ d[307] ^ d[306] ^ d[302] ^ d[300] ^ d[299] ^ d[297] ^ d[296] ^ d[295] ^ d[294] ^ d[293] ^ d[292] ^ d[291] ^ d[289] ^ d[287] ^ d[285] ^ d[284] ^ d[283] ^ d[280] ^ d[276] ^ d[274] ^ d[273] ^ d[271] ^ d[270] ^ d[266] ^ d[265] ^ d[262] ^ d[261] ^ d[258] ^ d[256] ^ d[254] ^ d[252] ^ d[249] ^ d[245] ^ d[240] ^ d[234] ^ d[231] ^ d[227] ^ d[225] ^ d[224] ^ d[223] ^ d[221] ^ d[213] ^ d[211] ^ d[209] ^ d[207] ^ d[206] ^ d[205] ^ d[204] ^ d[200] ^ d[199] ^ d[198] ^ d[196] ^ d[195] ^ d[194] ^ d[191] ^ d[190] ^ d[189] ^ d[188] ^ d[187] ^ d[185] ^ d[183] ^ d[180] ^ d[179] ^ d[169] ^ d[168] ^ d[167] ^ d[166] ^ d[164] ^ d[163] ^ d[159] ^ d[158] ^ d[155] ^ d[153] ^ d[152] ^ d[148] ^ d[146] ^ d[141] ^ d[140] ^ d[134] ^ d[133] ^ d[132] ^ d[131] ^ d[129] ^ d[125] ^ d[124] ^ d[123] ^ d[122] ^ d[120] ^ d[116] ^ d[115] ^ d[114] ^ d[113] ^ d[111] ^ d[110] ^ d[108] ^ d[107] ^ d[103] ^ d[101] ^ d[100] ^ d[98] ^ d[96] ^ d[95] ^ d[94] ^ d[93] ^ d[92] ^ d[91] ^ d[84] ^ d[82] ^ d[81] ^ d[80] ^ d[79] ^ d[78] ^ d[76] ^ d[70] ^ d[69] ^ d[65] ^ d[64] ^ d[63] ^ d[62] ^ d[60] ^ d[58] ^ d[57] ^ d[55] ^ d[52] ^ d[51] ^ d[50] ^ d[47] ^ d[45] ^ d[44] ^ d[42] ^ d[41] ^ d[34] ^ d[31] ^ d[29] ^ d[28] ^ d[27] ^ d[26] ^ d[25] ^ d[23] ^ d[22] ^ d[21] ^ d[13] ^ d[9] ^ d[7] ^ d[6] ^ d[3] ^ c[0] ^ c[1] ^ c[3] ^ c[4] ^ c[8] ^ c[9] ^ c[11] ^ c[14] ^ c[18] ^ c[20] ^ c[23] ^ c[24] ^ c[25] ^ c[26] ^ c[27] ^ c[29] ^ c[30]);

    SET_BIT_FROM_VALUE_MAC(newcrc,30, d[559] ^ d[558] ^ d[556] ^ d[555] ^ d[554] ^ d[553] ^ d[552] ^ d[549] ^ d[547] ^ d[543] ^ d[540] ^ d[538] ^ d[537] ^ d[533] ^ d[532] ^ d[530] ^ d[529] ^ d[528] ^ d[527] ^ d[526] ^ d[524] ^ d[523] ^ d[520] ^ d[519] ^ d[517] ^ d[516] ^ d[514] ^ d[512] ^ d[510] ^ d[509] ^ d[508] ^ d[506] ^ d[505] ^ d[504] ^ d[500] ^ d[499] ^ d[498] ^ d[493] ^ d[492] ^ d[491] ^ d[490] ^ d[489] ^ d[488] ^ d[487] ^ d[486] ^ d[484] ^ d[481] ^ d[480] ^ d[479] ^ d[478] ^ d[477] ^ d[475] ^ d[474] ^ d[470] ^ d[468] ^ d[466] ^ d[463] ^ d[462] ^ d[460] ^ d[459] ^ d[456] ^ d[450] ^ d[448] ^ d[447] ^ d[446] ^ d[442] ^ d[435] ^ d[434] ^ d[432] ^ d[431] ^ d[422] ^ d[420] ^ d[417] ^ d[416] ^ d[414] ^ d[412] ^ d[410] ^ d[407] ^ d[406] ^ d[405] ^ d[403] ^ d[402] ^ d[398] ^ d[397] ^ d[396] ^ d[394] ^ d[391] ^ d[390] ^ d[389] ^ d[388] ^ d[386] ^ d[385] ^ d[384] ^ d[379] ^ d[376] ^ d[374] ^ d[372] ^ d[370] ^ d[367] ^ d[366] ^ d[364] ^ d[361] ^ d[360] ^ d[357] ^ d[356] ^ d[355] ^ d[351] ^ d[347] ^ d[346] ^ d[345] ^ d[343] ^ d[342] ^ d[340] ^ d[339] ^ d[337] ^ d[336] ^ d[335] ^ d[333] ^ d[332] ^ d[331] ^ d[326] ^ d[325] ^ d[320] ^ d[319] ^ d[318] ^ d[317] ^ d[316] ^ d[315] ^ d[313] ^ d[310] ^ d[308] ^ d[307] ^ d[303] ^ d[301] ^ d[300] ^ d[298] ^ d[297] ^ d[296] ^ d[295] ^ d[294] ^ d[293] ^ d[292] ^ d[290] ^ d[288] ^ d[286] ^ d[285] ^ d[284] ^ d[281] ^ d[277] ^ d[275] ^ d[274] ^ d[272] ^ d[271] ^ d[267] ^ d[266] ^ d[263] ^ d[262] ^ d[259] ^ d[257] ^ d[255] ^ d[253] ^ d[250] ^ d[246] ^ d[241] ^ d[235] ^ d[232] ^ d[228] ^ d[226] ^ d[225] ^ d[224] ^ d[222] ^ d[214] ^ d[212] ^ d[210] ^ d[208] ^ d[207] ^ d[206] ^ d[205] ^ d[201] ^ d[200] ^ d[199] ^ d[197] ^ d[196] ^ d[195] ^ d[192] ^ d[191] ^ d[190] ^ d[189] ^ d[188] ^ d[186] ^ d[184] ^ d[181] ^ d[180] ^ d[170] ^ d[169] ^ d[168] ^ d[167] ^ d[165] ^ d[164] ^ d[160] ^ d[159] ^ d[156] ^ d[154] ^ d[153] ^ d[149] ^ d[147] ^ d[142] ^ d[141] ^ d[135] ^ d[134] ^ d[133] ^ d[132] ^ d[130] ^ d[126] ^ d[125] ^ d[124] ^ d[123] ^ d[121] ^ d[117] ^ d[116] ^ d[115] ^ d[114] ^ d[112] ^ d[111] ^ d[109] ^ d[108] ^ d[104] ^ d[102] ^ d[101] ^ d[99] ^ d[97] ^ d[96] ^ d[95] ^ d[94] ^ d[93] ^ d[92] ^ d[85] ^ d[83] ^ d[82] ^ d[81] ^ d[80] ^ d[79] ^ d[77] ^ d[71] ^ d[70] ^ d[66] ^ d[65] ^ d[64] ^ d[63] ^ d[61] ^ d[59] ^ d[58] ^ d[56] ^ d[53] ^ d[52] ^ d[51] ^ d[48] ^ d[46] ^ d[45] ^ d[43] ^ d[42] ^ d[35] ^ d[32] ^ d[30] ^ d[29] ^ d[28] ^ d[27] ^ d[26] ^ d[24] ^ d[23] ^ d[22] ^ d[14] ^ d[10] ^ d[8] ^ d[7] ^ d[4] ^ c[0] ^ c[1] ^ c[2] ^ c[4] ^ c[5] ^ c[9] ^ c[10] ^ c[12] ^ c[15] ^ c[19] ^ c[21] ^ c[24] ^ c[25] ^ c[26] ^ c[27] ^ c[28] ^ c[30] ^ c[31]);

    SET_BIT_FROM_VALUE_MAC(newcrc,31, d[559] ^ d[557] ^ d[556] ^ d[555] ^ d[554] ^ d[553] ^ d[550] ^ d[548] ^ d[544] ^ d[541] ^ d[539] ^ d[538] ^ d[534] ^ d[533] ^ d[531] ^ d[530] ^ d[529] ^ d[528] ^ d[527] ^ d[525] ^ d[524] ^ d[521] ^ d[520] ^ d[518] ^ d[517] ^ d[515] ^ d[513] ^ d[511] ^ d[510] ^ d[509] ^ d[507] ^ d[506] ^ d[505] ^ d[501] ^ d[500] ^ d[499] ^ d[494] ^ d[493] ^ d[492] ^ d[491] ^ d[490] ^ d[489] ^ d[488] ^ d[487] ^ d[485] ^ d[482] ^ d[481] ^ d[480] ^ d[479] ^ d[478] ^ d[476] ^ d[475] ^ d[471] ^ d[469] ^ d[467] ^ d[464] ^ d[463] ^ d[461] ^ d[460] ^ d[457] ^ d[451] ^ d[449] ^ d[448] ^ d[447] ^ d[443] ^ d[436] ^ d[435] ^ d[433] ^ d[432] ^ d[423] ^ d[421] ^ d[418] ^ d[417] ^ d[415] ^ d[413] ^ d[411] ^ d[408] ^ d[407] ^ d[406] ^ d[404] ^ d[403] ^ d[399] ^ d[398] ^ d[397] ^ d[395] ^ d[392] ^ d[391] ^ d[390] ^ d[389] ^ d[387] ^ d[386] ^ d[385] ^ d[380] ^ d[377] ^ d[375] ^ d[373] ^ d[371] ^ d[368] ^ d[367] ^ d[365] ^ d[362] ^ d[361] ^ d[358] ^ d[357] ^ d[356] ^ d[352] ^ d[348] ^ d[347] ^ d[346] ^ d[344] ^ d[343] ^ d[341] ^ d[340] ^ d[338] ^ d[337] ^ d[336] ^ d[334] ^ d[333] ^ d[332] ^ d[327] ^ d[326] ^ d[321] ^ d[320] ^ d[319] ^ d[318] ^ d[317] ^ d[316] ^ d[314] ^ d[311] ^ d[309] ^ d[308] ^ d[304] ^ d[302] ^ d[301] ^ d[299] ^ d[298] ^ d[297] ^ d[296] ^ d[295] ^ d[294] ^ d[293] ^ d[291] ^ d[289] ^ d[287] ^ d[286] ^ d[285] ^ d[282] ^ d[278] ^ d[276] ^ d[275] ^ d[273] ^ d[272] ^ d[268] ^ d[267] ^ d[264] ^ d[263] ^ d[260] ^ d[258] ^ d[256] ^ d[254] ^ d[251] ^ d[247] ^ d[242] ^ d[236] ^ d[233] ^ d[229] ^ d[227] ^ d[226] ^ d[225] ^ d[223] ^ d[215] ^ d[213] ^ d[211] ^ d[209] ^ d[208] ^ d[207] ^ d[206] ^ d[202] ^ d[201] ^ d[200] ^ d[198] ^ d[197] ^ d[196] ^ d[193] ^ d[192] ^ d[191] ^ d[190] ^ d[189] ^ d[187] ^ d[185] ^ d[182] ^ d[181] ^ d[171] ^ d[170] ^ d[169] ^ d[168] ^ d[166] ^ d[165] ^ d[161] ^ d[160] ^ d[157] ^ d[155] ^ d[154] ^ d[150] ^ d[148] ^ d[143] ^ d[142] ^ d[136] ^ d[135] ^ d[134] ^ d[133] ^ d[131] ^ d[127] ^ d[126] ^ d[125] ^ d[124] ^ d[122] ^ d[118] ^ d[117] ^ d[116] ^ d[115] ^ d[113] ^ d[112] ^ d[110] ^ d[109] ^ d[105] ^ d[103] ^ d[102] ^ d[100] ^ d[98] ^ d[97] ^ d[96] ^ d[95] ^ d[94] ^ d[93] ^ d[86] ^ d[84] ^ d[83] ^ d[82] ^ d[81] ^ d[80] ^ d[78] ^ d[72] ^ d[71] ^ d[67] ^ d[66] ^ d[65] ^ d[64] ^ d[62] ^ d[60] ^ d[59] ^ d[57] ^ d[54] ^ d[53] ^ d[52] ^ d[49] ^ d[47] ^ d[46] ^ d[44] ^ d[43] ^ d[36] ^ d[33] ^ d[31] ^ d[30] ^ d[29] ^ d[28] ^ d[27] ^ d[25] ^ d[24] ^ d[23] ^ d[15] ^ d[11] ^ d[9] ^ d[8] ^ d[5] ^ c[0] ^ c[1] ^ c[2] ^ c[3] ^ c[5] ^ c[6] ^ c[10] ^ c[11] ^ c[13] ^ c[16] ^ c[20] ^ c[22] ^ c[25] ^ c[26] ^ c[27] ^ c[28] ^ c[29] ^ c[31]);
#endif
    return newcrc;
}


/**
* @internal hashIndexCalculate_paramsCheck function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         check parameters for : Calculate hash index of member load-balancing group for given hash client.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - the device number.
* @param[in] packetType               - packet type.
*                                      hashClientType     - client that uses HASH.
* @param[in] numberOfMembers          - number of members in load-balancing group.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or index
* @retval GT_OUT_OF_RANGE          - one of the fields are out of range
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This API is relevant when <Random Enable> flag = GT_FALSE.
*       It determines whether the load balancing is based on the
*       ingress hash or on a 16-bit pseudo-random.
*       The only supported hash mode is Enhanced CRC-based hash mode.
*
*/
static GT_STATUS hashIndexCalculate_paramsCheck
(
    IN GT_U8                                        devNum,
    IN CPSS_DXCH_PCL_PACKET_TYPE_ENT                packetType,
    IN CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_ENT         hashEntityType,
    IN GT_U32                                       numberOfMembers,
    OUT GT_U32                                      *startBitPtr,
    OUT GT_U32                                      *numOfBitsInHashPtr,
    OUT GT_U32                                      *crcSeedPtr,
    OUT GT_U32                                      *maskIndexPtr
)
{
    GT_STATUS rc;
    CPSS_DXCH_TRUNK_LBH_CRC_MODE_ENT crcMode;
    GT_U32  maskIndex;

    if ((numberOfMembers == 0) || (numberOfMembers > BIT_12))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
            "numberOfMembers if out of range[1..4096]",
            numberOfMembers);
    }

    rc  = prvCpssDxChTrunkHashBitsSelectionGet(devNum,hashEntityType, startBitPtr, numOfBitsInHashPtr);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "cpssDxChIpEcmpHashNumBitsGet : failed");
    }

    rc =  cpssDxChTrunkHashCrcParametersGet( devNum, &crcMode,crcSeedPtr);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "cpssDxChTrunkHashCrcParametersGet : failed");
    }

    if (crcMode != CPSS_DXCH_TRUNK_LBH_CRC_32_MODE_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED,
            "CRC mode must be 'CPSS_DXCH_TRUNK_LBH_CRC_32_MODE_E' but is [%d]",
            crcMode);
    }

    /* calculate mask index for given packet type */
    switch(packetType)
    {
        case CPSS_DXCH_PCL_PACKET_TYPE_IPV4_TCP_E:       maskIndex = 0;         break;
        case CPSS_DXCH_PCL_PACKET_TYPE_IPV4_UDP_E:       maskIndex = 1;         break;
        case CPSS_DXCH_PCL_PACKET_TYPE_MPLS_E:           maskIndex = 2;         break;
        case CPSS_DXCH_PCL_PACKET_TYPE_IPV4_FRAGMENT_E:  maskIndex = 3;         break;
        case CPSS_DXCH_PCL_PACKET_TYPE_IPV4_OTHER_E:     maskIndex = 4;         break;
        case CPSS_DXCH_PCL_PACKET_TYPE_ETHERNET_OTHER_E: maskIndex = 5;         break;
        case CPSS_DXCH_PCL_PACKET_TYPE_IPV6_OTHER_E:     maskIndex = 6;         break;
        case CPSS_DXCH_PCL_PACKET_TYPE_IPV6_TCP_E:       maskIndex = 7;         break;
        case CPSS_DXCH_PCL_PACKET_TYPE_IPV6_UDP_E:       maskIndex = 8;         break;
        case CPSS_DXCH_PCL_PACKET_TYPE_UDE_E:            maskIndex = 9;         break;
        case CPSS_DXCH_PCL_PACKET_TYPE_UDE1_E:           maskIndex = 10;        break;
        case CPSS_DXCH_PCL_PACKET_TYPE_UDE2_E:           maskIndex = 11;        break;
        case CPSS_DXCH_PCL_PACKET_TYPE_UDE3_E:           maskIndex = 12;        break;
        case CPSS_DXCH_PCL_PACKET_TYPE_UDE4_E:           maskIndex = 13;        break;
        case CPSS_DXCH_PCL_PACKET_TYPE_UDE5_E:           maskIndex = 14;        break;
        case CPSS_DXCH_PCL_PACKET_TYPE_UDE6_E:           maskIndex = 15;        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "unknown packetType[%d]",packetType);
    }

    *maskIndexPtr = maskIndex;

    return GT_OK;
}


/**
* @internal hashIndexCalculate function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         Calculate hash index of member load-balancing group for given hash client.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - the device number.
* @param[in] packetType               - packet type.
* @param[in] hashInputDataPtr         - (pointer to) hash input data.
*                                      hashClientType     - client that uses HASH.
* @param[in] numberOfMembers          - number of members in load-balancing group.
* @param[in] hashDataPtr              - allocated array for the HASH data.
* @param[in] saltDataPtr              - allocated array for the SALT data.
* @param[in] crc32DataPtr             - allocated array for the CRC32 data.
*
* @param[out] hashIndexPtr             - (pointer to) calculated by means of ingress
*                                      hash index of load-balancing group member.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or index
* @retval GT_OUT_OF_RANGE          - one of the fields are out of range
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This API is relevant when <Random Enable> flag = GT_FALSE.
*       It determines whether the load balancing is based on the
*       ingress hash or on a 16-bit pseudo-random.
*       The only supported hash mode is Enhanced CRC-based hash mode.
*
*/
static GT_STATUS hashIndexCalculate
(
    IN GT_U8                                        devNum,
    IN CPSS_DXCH_PCL_PACKET_TYPE_ENT                packetType,
    IN CPSS_DXCH_TRUNK_LBH_INPUT_DATA_STC           *hashInputDataPtr,
    IN CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_ENT         hashEntityType,
    IN GT_U32                                       numberOfMembers,
    IN GT_U8                                        *hashDataPtr,
    IN GT_U8                                        *saltDataPtr,
    IN GT_U8                                        *crc32DataPtr,
    OUT GT_U32                                      *hashIndexPtr
)
{
    GT_U32 maskBmpHwEntry[HASH_MASK_CRC_ENTRY_NUM_WORDS_CNS];
    GT_U32 maskIndex;
    GT_U8 mask;
    GT_U8 pattern;
    GT_U32 offset;
    GT_U32 startBit;
    GT_U32 numOfBitsInHash;
    GT_U8  saltValue;
    GT_U32 i,j;
    GT_BOOL carry = 0;
    GT_U32 finalInputHash;
    GT_U32 crcSeed;
    GT_STATUS rc;
    GT_U32 hashBitsValue;
    CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_ENT prv_hashEntityType;
    GT_U32 ipLen = 0;
    GT_U32 tmpValue = 0;

    switch(hashEntityType)
    {
        case CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_TRUNK_E:
            prv_hashEntityType = CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_TRUNK_E;
            break;
        case CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_L2_ECMP_E:
            prv_hashEntityType = CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_L2_ECMP_E;
            break;
        case CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_L3_ECMP_E:
            prv_hashEntityType = CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_L3_ECMP_E;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "unknown hashEntityType[%d]",hashEntityType);
    }


    /*check parameters and get needed in*/
    rc = hashIndexCalculate_paramsCheck(devNum,packetType,prv_hashEntityType,numberOfMembers,
                &startBit,&numOfBitsInHash,&crcSeed,&maskIndex);
    if (rc != GT_OK)
    {
        return rc;
    }

    maskIndex += 16;
    /* read mask from hw */
    rc = prvCpssDxChReadTableEntry(devNum,
                                   CPSS_DXCH_LION_TABLE_TRUNK_HASH_MASK_CRC_E,
                                   maskIndex,
                                   maskBmpHwEntry);
    if(rc != GT_OK)
    {
        return rc;
    }


    /* check symmetricL4PortEnable */
    U32_GET_FIELD_IN_ENTRY_MAC(maskBmpHwEntry,73,1,tmpValue);
    if (tmpValue)
    {
        for ( i = 0; i < 2; i++ )
        {
            hashInputDataPtr->l4DstPortArray[i] += hashInputDataPtr->l4SrcPortArray[i];
            hashInputDataPtr->l4SrcPortArray[i] = 0;
        }
    }

    /* check symmetricIpv4AddrEnable */
    U32_GET_FIELD_IN_ENTRY_MAC(maskBmpHwEntry,71,1,tmpValue);
    if (tmpValue)
    {
        ipLen = 16;

        for ( i = 12; i < ipLen; i++ )
        {
            hashInputDataPtr->ipDipArray[i] += hashInputDataPtr->ipSipArray[i];
            hashInputDataPtr->ipSipArray[i] = 0;
        }
    }

    /* check symmetricIpv6AddrEnable */
    U32_GET_FIELD_IN_ENTRY_MAC(maskBmpHwEntry,72,1,tmpValue);
    if (tmpValue)
    {
        ipLen = 12;

        for ( i = 0; i < ipLen; i++ )
        {
            hashInputDataPtr->ipDipArray[i] += hashInputDataPtr->ipSipArray[i];
            hashInputDataPtr->ipSipArray[i] = 0;
        }
    }

    /* check symmetricMacAddrEnable */
    U32_GET_FIELD_IN_ENTRY_MAC(maskBmpHwEntry,70,1,tmpValue);
    if (tmpValue)
    {
        for ( i = 0; i < 6; i++ )
        {
            hashInputDataPtr->macDaArray[i] += hashInputDataPtr->macSaArray[i];
            hashInputDataPtr->macSaArray[i] = 0;
        }
    }

    /* make AND for pattern and MASK to get real data for hash calculation per bit: 560 bit */
    offset = 0;
    /* L4 target port - bits 15:0 */
    for ( i = 0; i < 16; i++ )
    {
        mask = GET_BIT_FROM_MASK_BMP_MAC(maskBmpHwEntry,offset+i);
        if(mask == 0) continue;
        pattern = GET_REVERSE_BYTE_ORDER_BIT____FROM_NETWORK_ORDER_BYTE_MAC(hashInputDataPtr->l4DstPortArray,i);
        hashDataPtr[offset+i] = pattern & mask;
    }
    offset = 16;
    /* L4 source port - bits 31:16 */
    for ( i = 0; i < 16; i++ )
    {
        mask = GET_BIT_FROM_MASK_BMP_MAC(maskBmpHwEntry,offset+i);
        if(mask == 0) continue;
        pattern = GET_REVERSE_BYTE_ORDER_BIT____FROM_NETWORK_ORDER_BYTE_MAC(hashInputDataPtr->l4SrcPortArray,i);
        hashDataPtr[offset+i] = pattern & mask;
    }

    offset = 32;
    /* IPv6 flow - bits 51:32 */
    for ( i = 0; i < 20; i++ )
    {
        mask = GET_BIT_FROM_MASK_BMP_MAC(maskBmpHwEntry,offset+i);
        if(mask == 0) continue;
        pattern = GET_REVERSE_BYTE_ORDER_BIT____FROM_NETWORK_ORDER_BYTE_MAC(hashInputDataPtr->ipv6FlowArray,i);
        hashDataPtr[offset+i] = pattern & mask;
    }
    offset = 52;
    /* 55:52 reserved : set to 0 */
    offset = 56;
    /* IP DIP - 183:56 */
    for ( i = 0; i < 128; i++ )
    {
        mask = GET_BIT_FROM_MASK_BMP_MAC(maskBmpHwEntry,offset+i);
        if(mask == 0) continue;
        pattern = GET_REVERSE_BYTE_ORDER_BIT____FROM_NETWORK_ORDER_BYTE_MAC(hashInputDataPtr->ipDipArray,i);
        hashDataPtr[offset+i] = pattern & mask;
    }
    offset = 184;
    /* IP SIP - 311:184 */
    for ( i = 0; i < 128; i++ )
    {
        mask = GET_BIT_FROM_MASK_BMP_MAC(maskBmpHwEntry,offset+i);
        if(mask == 0) continue;
        pattern = GET_REVERSE_BYTE_ORDER_BIT____FROM_NETWORK_ORDER_BYTE_MAC(hashInputDataPtr->ipSipArray,i);
        hashDataPtr[offset+i] = pattern & mask;
    }
    /* MAC DA  - 359:312 */
    offset = 312;
    for ( i = 0; i < 48; i++ )
    {
        mask = GET_BIT_FROM_MASK_BMP_MAC(maskBmpHwEntry,offset+i);
        if(mask == 0) continue;
        pattern = GET_REVERSE_BYTE_ORDER_BIT____FROM_NETWORK_ORDER_BYTE_MAC(hashInputDataPtr->macDaArray,i);
        hashDataPtr[offset+i] = pattern & mask;
    }
    /* MAC SA - 407:360 */
    offset = 360;
    for ( i = 0; i < 48; i++ )
    {
        mask = GET_BIT_FROM_MASK_BMP_MAC(maskBmpHwEntry,offset+i);
        if(mask == 0) continue;
        pattern = GET_REVERSE_BYTE_ORDER_BIT____FROM_NETWORK_ORDER_BYTE_MAC(hashInputDataPtr->macSaArray,i);
        hashDataPtr[offset+i] = pattern & mask;
    }

    /* MPLS label 0/ evlan : 427:408*/
    offset = 408;
    for ( i = 0; i < 20; i++ )
    {
        mask = GET_BIT_FROM_MASK_BMP_MAC(maskBmpHwEntry,offset+i);
        if(mask == 0) continue;
        pattern = GET_REVERSE_BYTE_ORDER_BIT____FROM_NETWORK_ORDER_BYTE_MAC(hashInputDataPtr->mplsLabel0Array,i);
        hashDataPtr[offset+i] = pattern & mask;
    }
    offset = 428;
    /* 431: 428 - reserved : set to 0 */

    /* MPLS label 1 / Original Source ePort : 451:432*/
    offset = 432;
    for ( i = 0; i < 20; i++ )
    {
        mask = GET_BIT_FROM_MASK_BMP_MAC(maskBmpHwEntry,offset+i);
        if(mask == 0) continue;
        pattern = GET_REVERSE_BYTE_ORDER_BIT____FROM_NETWORK_ORDER_BYTE_MAC(hashInputDataPtr->mplsLabel1Array,i);
        hashDataPtr[offset+i] = pattern & mask;
    }
    offset = 452;
    /* 455:452 - reserved : set to 0 */

    /* MPLS label 2 / Local Source ePort : 475:456 */
    offset = 456;
    for ( i = 0; i < 20; i++ )
    {
        mask = GET_BIT_FROM_MASK_BMP_MAC(maskBmpHwEntry,offset+i);
        if(mask == 0) continue;
        pattern = GET_REVERSE_BYTE_ORDER_BIT____FROM_NETWORK_ORDER_BYTE_MAC(hashInputDataPtr->mplsLabel2Array,i);
        hashDataPtr[offset+i] = pattern & mask;
    }

    offset = 476;
    /* 479:476 - reserved : set to 0 */

    /* Local Dev Source Port - 487:480*/
    offset = 480;
    for ( i = 0; i < 8; i++ )
    {
        mask = GET_BIT_FROM_MASK_BMP_MAC(maskBmpHwEntry,offset+i);
        if(mask == 0) continue;
        pattern = GET_REVERSE_BYTE_ORDER_BIT____FROM_NETWORK_ORDER_BYTE_MAC(hashInputDataPtr->localSrcPortArray,i);
        hashDataPtr[offset+i] = pattern & mask;
    }

    /* User Defined Bytes - 559:488 */
    offset = 488;
    /* it is started from udb14 till udb22 */
    for ( i = 0; i < 72; i++ )
    {
        mask = GET_BIT_FROM_MASK_BMP_MAC(maskBmpHwEntry,offset+i);
        if(mask == 0) continue;
        pattern = (hashInputDataPtr->udbsArray[14 + (i>>3)] & (1<<(i & 7))) ? 1 : 0;
        hashDataPtr[offset+i] = pattern & mask;
    }

    /* read salt data  and arrange it in array 560 bit */
    for (i = 0; i < 70; i++ )
    {
        rc =  cpssDxChTrunkHashCrcSaltByteGet(devNum, i, &saltValue);
        if (rc != GT_OK)
        {
            return rc;
        }
        for (j = 0; j < 8; j++ )
        {
            saltDataPtr[8*i+j] = U32_GET_FIELD_MAC(saltValue, j ,1);
        }
    }

    /* calculate data for CRC32 */
    for(i = 0; i < 560; i++)
    {
        /* NOTE: 1-bit Full Adder */
        crc32DataPtr[i] = (GT_U8)(hashDataPtr[i] ^ saltDataPtr[i] ^ carry);
        carry = ((saltDataPtr[i] && hashDataPtr[i]) || (carry && (saltDataPtr[i] ^ hashDataPtr[i]))) && (i%8 != 7); /* no carry passed to following byte*/
    }

    finalInputHash = calcHashFor70BytesCrc32(crcSeed,crc32DataPtr);

    CPSS_LOG_INFORMATION_MAC("Crc32 hash [0x%x] \n",finalInputHash);

    /* now calculate hash index of load-balancing group member*/
    hashBitsValue =  U32_GET_FIELD_MAC(finalInputHash, startBit, numOfBitsInHash);
    *hashIndexPtr = (numberOfMembers * hashBitsValue) >> numOfBitsInHash;
    *hashIndexPtr = *hashIndexPtr & 0x00000FFF;


    CPSS_LOG_INFORMATION_MAC("hashIndex[%d] \n",(*hashIndexPtr));

    return GT_OK;

}
/**
* @internal internal_cpssDxChTrunkHashIndexCalculate function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         Calculate hash index of member load-balancing group for given hash client.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - the device number.
* @param[in] packetType               - packet type.
* @param[in] hashInputDataPtr         - (pointer to) hash input data.
*                                      hashClientType     - client that uses HASH.
*                                      (APPLICABLE VALUES: CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_TRUNK_E,
*                                      CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_L2_ECMP_E,
*                                      CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_L3_ECMP_E)
* @param[in] numberOfMembers          - number of members in load-balancing group.
*
* @param[out] hashIndexPtr             - (pointer to) calculated by means of ingress
*                                      hash index of load-balancing group member.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or index
* @retval GT_OUT_OF_RANGE          - one of the fields are out of range
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This API is relevant when <Random Enable> flag = GT_FALSE.
*       It determines whether the load balancing is based on the
*       ingress hash or on a 16-bit pseudo-random.
*       The only supported hash mode is Enhanced CRC-based hash mode.
*
*/
static GT_STATUS internal_cpssDxChTrunkHashIndexCalculate
(
    IN GT_U8                                        devNum,
    IN CPSS_DXCH_PCL_PACKET_TYPE_ENT                packetType,
    IN CPSS_DXCH_TRUNK_LBH_INPUT_DATA_STC           *hashInputDataPtr,
    IN CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_ENT         hashEntityType,
    IN GT_U32                                       numberOfMembers,
    OUT GT_U32                                      *hashIndexPtr
)
{
    GT_STATUS rc;
    GT_U8  *hashDataPtr;
    GT_U8  *saltDataPtr;
    GT_U8  *crc32DataPtr;
    GT_U32  sizeOfHashCalc;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_CH1_E| CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
          CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);
    CPSS_NULL_PTR_CHECK_MAC(hashInputDataPtr);
    CPSS_NULL_PTR_CHECK_MAC(hashIndexPtr);

    sizeOfHashCalc = sizeof(GT_U8)*560;

    hashDataPtr  = cpssOsMalloc(sizeOfHashCalc);
    saltDataPtr  = cpssOsMalloc(sizeOfHashCalc);
    crc32DataPtr = cpssOsMalloc(sizeOfHashCalc);

    if(crc32DataPtr && saltDataPtr && hashDataPtr)
    {
        cpssOsMemSet(hashDataPtr,0,sizeOfHashCalc);
        cpssOsMemSet(saltDataPtr,0,sizeOfHashCalc);
        cpssOsMemSet(crc32DataPtr,0,sizeOfHashCalc);

        rc = hashIndexCalculate(
            devNum,
            packetType,
            hashInputDataPtr,
            hashEntityType,
            numberOfMembers,
            hashDataPtr,
            saltDataPtr,
            crc32DataPtr,
            hashIndexPtr);
    }
    else
    {
        rc = GT_OUT_OF_CPU_MEM;
    }

    if(hashDataPtr)
    {
        cpssOsFree(hashDataPtr);
    }

    if(saltDataPtr)
    {
        cpssOsFree(saltDataPtr);
    }

    if(crc32DataPtr)
    {
        cpssOsFree(crc32DataPtr);
    }

    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    return rc;
}

/**
* @internal cpssDxChTrunkHashIndexCalculate function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         Calculate hash index of member load-balancing group for given hash client.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - the device number.
* @param[in] packetType               - packet type.
* @param[in] hashInputDataPtr         - (pointer to) hash input data.
*                                      hashClientType     - client that uses HASH.
*                                      (APPLICABLE VALUES: CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_TRUNK_E,
*                                      CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_L2_ECMP_E,
*                                      CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_L3_ECMP_E)
* @param[in] numberOfMembers          - number of members in load-balancing group.
*
* @param[out] hashIndexPtr             - (pointer to) calculated by means of ingress
*                                      hash index of load-balancing group member.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or index
* @retval GT_OUT_OF_RANGE          - one of the fields are out of range
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This API is relevant when <Random Enable> flag = GT_FALSE.
*       It determines whether the load balancing is based on the
*       ingress hash or on a 16-bit pseudo-random.
*       The only supported hash mode is Enhanced CRC-based hash mode.
*
*/
GT_STATUS cpssDxChTrunkHashIndexCalculate
(
    IN GT_U8                                        devNum,
    IN CPSS_DXCH_PCL_PACKET_TYPE_ENT                packetType,
    IN CPSS_DXCH_TRUNK_LBH_INPUT_DATA_STC           *hashInputDataPtr,
    IN CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_ENT         hashEntityType,
    IN GT_U32                                       numberOfMembers,
    OUT GT_U32                                      *hashIndexPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTrunkHashIndexCalculate);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, packetType, hashInputDataPtr, hashEntityType, numberOfMembers, hashIndexPtr));

    rc = internal_cpssDxChTrunkHashIndexCalculate(devNum, packetType, hashInputDataPtr, hashEntityType, numberOfMembers, hashIndexPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, packetType, hashInputDataPtr, hashEntityType, numberOfMembers, hashIndexPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}



/**
* @internal internal_cpssDxChTrunkHashPearsonValueSet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         Set the Pearson hash value for the specific index.
*         NOTE: the Pearson hash used when CRC-16 mode is used.
*
* @note   APPLICABLE DEVICES:      Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3.
*
* @param[in] devNum                   - the device number.
* @param[in] index                    - the table  (APPLICABLE RANGES: 0..63)
* @param[in] value                    - the Pearson hash  (APPLICABLE RANGES: 0..63)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or index
* @retval GT_OUT_OF_RANGE          - value > 63
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note related to feature 'CRC hash mode'
*
*/
static GT_STATUS internal_cpssDxChTrunkHashPearsonValueSet
(
    IN GT_U8                          devNum,
    IN GT_U32                         index,
    IN GT_U32                         value
)
{
    GT_U32    regAddr;       /* register address    */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E);

    if(index > 63)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(value >= BIT_6)
    {
        /* 6 bits in HW */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr =
            PRV_DXCH_REG1_UNIT_PCL_MAC(devNum).HASH.pearsonHashTable[index / 4];
    }
    else
    {
        regAddr =
            PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->pclRegs.pearsonHashTableReg[index / 4];
    }

    return prvCpssHwPpSetRegField(devNum, regAddr, (index % 4)* 8 , 6 ,value);
}

/**
* @internal cpssDxChTrunkHashPearsonValueSet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         Set the Pearson hash value for the specific index.
*         NOTE: the Pearson hash used when CRC-16 mode is used.
*
* @note   APPLICABLE DEVICES:      Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3.
*
* @param[in] devNum                   - the device number.
* @param[in] index                    - the table  (APPLICABLE RANGES: 0..63)
* @param[in] value                    - the Pearson hash  (APPLICABLE RANGES: 0..63)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or index
* @retval GT_OUT_OF_RANGE          - value > 63
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note related to feature 'CRC hash mode'
*
*/
GT_STATUS cpssDxChTrunkHashPearsonValueSet
(
    IN GT_U8                          devNum,
    IN GT_U32                         index,
    IN GT_U32                         value
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTrunkHashPearsonValueSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, index, value));

    rc = internal_cpssDxChTrunkHashPearsonValueSet(devNum, index, value);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, index, value));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTrunkHashPearsonValueGet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         Get the Pearson hash value for the specific index.
*         NOTE: the Pearson hash used when CRC-16 mode is used.
*
* @note   APPLICABLE DEVICES:      Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3.
*
* @param[in] devNum                   - the device number.
* @param[in] index                    - the table  (APPLICABLE RANGES: 0..63)
*
* @param[out] valuePtr                 - (pointer to) the Pearson hash value (APPLICABLE RANGES: 0..63)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or index
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note related to feature 'CRC hash mode'
*
*/
static GT_STATUS internal_cpssDxChTrunkHashPearsonValueGet
(
    IN GT_U8                          devNum,
    IN GT_U32                         index,
    OUT GT_U32                        *valuePtr
)
{
    GT_U32    regAddr;       /* register address    */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E);

    CPSS_NULL_PTR_CHECK_MAC(valuePtr);

    if(index > 63)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr =
            PRV_DXCH_REG1_UNIT_PCL_MAC(devNum).HASH.pearsonHashTable[index / 4];
    }
    else
    {
        regAddr =
            PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->pclRegs.pearsonHashTableReg[index / 4];
    }

    return prvCpssHwPpGetRegField(devNum, regAddr, (index % 4) * 8 , 6 ,valuePtr);
}

/**
* @internal cpssDxChTrunkHashPearsonValueGet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         Get the Pearson hash value for the specific index.
*         NOTE: the Pearson hash used when CRC-16 mode is used.
*
* @note   APPLICABLE DEVICES:      Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3.
*
* @param[in] devNum                   - the device number.
* @param[in] index                    - the table  (APPLICABLE RANGES: 0..63)
*
* @param[out] valuePtr                 - (pointer to) the Pearson hash value (APPLICABLE RANGES: 0..63)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or index
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note related to feature 'CRC hash mode'
*
*/
GT_STATUS cpssDxChTrunkHashPearsonValueGet
(
    IN GT_U8                          devNum,
    IN GT_U32                         index,
    OUT GT_U32                        *valuePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTrunkHashPearsonValueGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, index, valuePtr));

    rc = internal_cpssDxChTrunkHashPearsonValueGet(devNum, index, valuePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, index, valuePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvCpssDxChTrunkDesignatedTableForSrcPortHashMappingSet function
* @endinternal
*
* @brief   the function sets the designated device table with the portsArr[].trunkPort
*         in indexes that match hash (%8 or %64(modulo)) on the ports in portsArr[].srcPort
*         this to allow 'Src port' trunk hash for traffic sent to the specified
*         trunk.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] numOfPorts               - number of pairs in array portsArr[].
* @param[in] portsArr[]               - (array of) pairs of 'source ports' ,'trunk ports'
*                                      for the source port hash.
* @param[in] mode                     - hash  (%8 or %64 (modulo))
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or trunkId or port in portsArr[].srcPort or
*                                       port in portsArr[].trunkPort or mode
*                                       or map two Source ports that falls into same Source hash index
*                                       into different trunk member ports
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device (device is not multi-port groups device)
*/
GT_STATUS prvCpssDxChTrunkDesignatedTableForSrcPortHashMappingSet
(
    IN GT_U8             devNum,
    IN GT_U32            numOfPorts,
    IN CPSS_DXCH_MULTI_PORT_GROUP_SRC_PORT_HASH_PAIR_STC  portsArr[],
    IN CPSS_DXCH_MULTI_PORT_GROUP_SRC_HASH_MODE_ENT  mode
)
{
    GT_STATUS rc;/* return code */
    GT_U32  ii,jj;/* iterators*/
    CPSS_PORTS_BMP_STC  designatedPorts; /* bmp of current ports in designated entry */
    GT_U32  portsToBeDesignatedArr[MAX_DESIGNATED_TRUNK_ENTRIES_NUM_CNS];/* list of trunk ports that need to be
                    designated for the trunk in the designated trunk table in
                    the same index as they appear in array of portsToBeDesignatedArr[]*/
    GT_U32  hashMode;/* hash mode %8 or %64 */
    GT_U32  numOfDesignatedTrunkEntriesHw;/* number of entries in trunk designated port table in HW */
    GT_PORT_NUM     portNum;/*temp port num*/

    numOfDesignatedTrunkEntriesHw = PRV_CPSS_DEV_TRUNK_INFO_MAC(devNum)->numOfDesignatedTrunkEntriesHw;

    switch(mode)
    {
        case CPSS_DXCH_MULTI_PORT_GROUP_SRC_HASH_MODE_MODULO_8_E:
            hashMode = 8;
            break;
        case CPSS_DXCH_MULTI_PORT_GROUP_SRC_HASH_MODE_MODULO_64_E:
            hashMode = 64;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(hashMode > numOfDesignatedTrunkEntriesHw)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(hashMode > MAX_DESIGNATED_TRUNK_ENTRIES_NUM_CNS)
    {
        /* should update MAX_DESIGNATED_TRUNK_ENTRIES_NUM_CNS */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    /* check ports numbers validity */
    for(ii = 0 ; ii < numOfPorts; ii++)
    {
        PRV_CPSS_DXCH_DUAL_HW_DEVICE_AND_PORT_CHECK_MAC(PRV_CPSS_HW_DEV_NUM_MAC(devNum),portsArr[ii].srcPort);
        portNum = PRV_CPSS_DXCH_DUAL_HW_DEVICE_CONVERT_DEV_MAC(PRV_CPSS_HW_DEV_NUM_MAC(devNum),portsArr[ii].srcPort);

        if(portNum >= BIT_6)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        PRV_CPSS_DXCH_DUAL_HW_DEVICE_AND_PORT_CHECK_MAC(PRV_CPSS_HW_DEV_NUM_MAC(devNum),portsArr[ii].srcPort);
        portNum = PRV_CPSS_DXCH_DUAL_HW_DEVICE_CONVERT_DEV_MAC(PRV_CPSS_HW_DEV_NUM_MAC(devNum),portsArr[ii].srcPort);

        if(portNum >= BIT_6)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }

    /* NOTE :
        when hashMode == 8 and numOfDesignatedTrunkEntriesHw == 64
        application ask for hash of %8 , although HW do %64 .
        so we duplicate the first 8 (0..7) entries 7 more times to indexes :
        8..15 , 16..23 , 24..31 , 32..39 , 40..47 , 48..55 , 56..63
    */

    /* need to override the trunk designated table */
    /* to set 8/64 entries to be appropriate for 'src port' hashing */

    /* start with 'not set yet' values */
    for(jj = 0 ; jj < hashMode; jj++)
    {
        portsToBeDesignatedArr[jj] = 0xFF;
    }

    /* fill in index of %8/64 of the 'source ports' with value of a trunk member */
    for(jj = 0 ; jj < numOfPorts; jj++)
    {
        if(portsToBeDesignatedArr[portsArr[jj].srcPort % hashMode] != 0xFF &&
           portsToBeDesignatedArr[portsArr[jj].srcPort % hashMode] != portsArr[jj].trunkPort)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        /* actual balance done according to 'global port' and not 'local port'*/
        portsToBeDesignatedArr[portsArr[jj].srcPort % hashMode] =
            portsArr[jj].trunkPort;
    }

    /* now fill all the 'not yet set' entries */
    ii = 0;
    for(jj = 0 ; jj < hashMode; jj++)
    {
        if(portsToBeDesignatedArr[jj] == 0xFF)
        {
            portsToBeDesignatedArr[jj] = portsArr[ii++].trunkPort;

            if(ii == numOfPorts)
            {
                /* rewind the index in portsArr[] */
                ii = 0;
            }
        }
    }

    /* now update the actual HW table */
    for(jj = 0 ; jj < numOfDesignatedTrunkEntriesHw; jj++)
    {
        rc = cpssDxChTrunkDesignatedPortsEntryGet(devNum, jj, &designatedPorts);
        if(rc != GT_OK)
        {
            return rc;
        }

        /* clear the ports set by CPSS */
        for(ii = 0 ; ii < numOfPorts ; ii++)
        {
            CPSS_PORTS_BMP_PORT_CLEAR_MAC((&designatedPorts),portsArr[ii].trunkPort);
        }

        /* set proper designated port */
        CPSS_PORTS_BMP_PORT_SET_MAC((&designatedPorts),portsToBeDesignatedArr[jj % hashMode]);


        rc = cpssDxChTrunkDesignatedPortsEntrySet(devNum, jj, &designatedPorts);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal internal_cpssDxChTrunkMcLocalSwitchingEnableSet function
* @endinternal
*
* @brief   Enable/Disable sending multi-destination packets back to its source
*         trunk on the local device.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number .
* @param[in] trunkId                  - the trunk id.
* @param[in] enable                   - Boolean value:
*                                      GT_TRUE  - multi-destination packets may be sent back to
*                                      their source trunk on the local device.
*                                      GT_FALSE - multi-destination packets are not sent back to
*                                      their source trunk on the local device.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_INITIALIZED       -the trunk library was not initialized for the device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or bad trunkId number
* @retval GT_BAD_STATE             - the trunk-type is not one of: CPSS_TRUNK_TYPE_FREE_E or CPSS_TRUNK_TYPE_REGULAR_E
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. the behavior of multi-destination traffic ingress from trunk is
*       not-affected by setting of cpssDxChBrgVlanLocalSwitchingEnableSet
*       and not-affected by setting of cpssDxChBrgPortEgressMcastLocalEnable
*       2. the functionality manipulates the 'non-trunk' table entry of the trunkId
*
*/
static GT_STATUS internal_cpssDxChTrunkMcLocalSwitchingEnableSet
(
    IN GT_U8            devNum,
    IN GT_TRUNK_ID      trunkId,
    IN GT_BOOL          enable
)
{
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    /* call the generic API */
    return prvCpssGenericTrunkMcLocalSwitchingEnableSet(devNum,trunkId,enable);
}

/**
* @internal cpssDxChTrunkMcLocalSwitchingEnableSet function
* @endinternal
*
* @brief   Enable/Disable sending multi-destination packets back to its source
*         trunk on the local device.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number .
* @param[in] trunkId                  - the trunk id.
* @param[in] enable                   - Boolean value:
*                                      GT_TRUE  - multi-destination packets may be sent back to
*                                      their source trunk on the local device.
*                                      GT_FALSE - multi-destination packets are not sent back to
*                                      their source trunk on the local device.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_INITIALIZED       -the trunk library was not initialized for the device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or bad trunkId number
* @retval GT_BAD_STATE             - the trunk-type is not one of: CPSS_TRUNK_TYPE_FREE_E or CPSS_TRUNK_TYPE_REGULAR_E
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. the behavior of multi-destination traffic ingress from trunk is
*       not-affected by setting of cpssDxChBrgVlanLocalSwitchingEnableSet
*       and not-affected by setting of cpssDxChBrgPortEgressMcastLocalEnable
*       2. the functionality manipulates the 'non-trunk' table entry of the trunkId
*
*/
GT_STATUS cpssDxChTrunkMcLocalSwitchingEnableSet
(
    IN GT_U8            devNum,
    IN GT_TRUNK_ID      trunkId,
    IN GT_BOOL          enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTrunkMcLocalSwitchingEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, trunkId, enable));

    rc = internal_cpssDxChTrunkMcLocalSwitchingEnableSet(devNum, trunkId, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, trunkId, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTrunkDbMcLocalSwitchingEnableGet function
* @endinternal
*
* @brief   Get the current status of Enable/Disable sending multi-destination packets
*         back to its source trunk on the local device.
*         function uses the DB (no HW operations)
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number .
* @param[in] trunkId                  - the trunk id.
*
* @param[out] enablePtr                - (pointer to) Boolean value:
*                                      GT_TRUE  - multi-destination packets may be sent back to
*                                      their source trunk on the local device.
*                                      GT_FALSE - multi-destination packets are not sent back to
*                                      their source trunk on the local device.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_INITIALIZED       -the trunk library was not initialized for the device
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_BAD_PARAM             - bad device number , or bad trunkId number
* @retval GT_BAD_STATE             - the trunk-type is not one of: CPSS_TRUNK_TYPE_FREE_E or CPSS_TRUNK_TYPE_REGULAR_E
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTrunkDbMcLocalSwitchingEnableGet
(
    IN GT_U8            devNum,
    IN GT_TRUNK_ID      trunkId,
    OUT GT_BOOL          *enablePtr
)
{
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    /* call the generic API */
    return prvCpssGenericTrunkDbMcLocalSwitchingEnableGet(devNum,trunkId,enablePtr);
}

/**
* @internal cpssDxChTrunkDbMcLocalSwitchingEnableGet function
* @endinternal
*
* @brief   Get the current status of Enable/Disable sending multi-destination packets
*         back to its source trunk on the local device.
*         function uses the DB (no HW operations)
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number .
* @param[in] trunkId                  - the trunk id.
*
* @param[out] enablePtr                - (pointer to) Boolean value:
*                                      GT_TRUE  - multi-destination packets may be sent back to
*                                      their source trunk on the local device.
*                                      GT_FALSE - multi-destination packets are not sent back to
*                                      their source trunk on the local device.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_INITIALIZED       -the trunk library was not initialized for the device
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_BAD_PARAM             - bad device number , or bad trunkId number
* @retval GT_BAD_STATE             - the trunk-type is not one of: CPSS_TRUNK_TYPE_FREE_E or CPSS_TRUNK_TYPE_REGULAR_E
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTrunkDbMcLocalSwitchingEnableGet
(
    IN GT_U8            devNum,
    IN GT_TRUNK_ID      trunkId,
    OUT GT_BOOL          *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTrunkDbMcLocalSwitchingEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, trunkId, enablePtr));

    rc = internal_cpssDxChTrunkDbMcLocalSwitchingEnableGet(devNum, trunkId, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, trunkId, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChTrunkMemberSelectionModeSet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         Determines the number of bits used to calculate the Index of the trunk member.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] selectionMode            - member selection mode
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The selection mode configuration may be changed by:
*       - cpssDxChTrunkHashNumBitsSet.
*       - cpssDxChTrunkHashGlobalModeSet.
*       - cpssDxChTrunkInit.
*       - cpssDxChTrunkHashCrcParametersSet.
*
*/
static GT_STATUS internal_cpssDxChTrunkMemberSelectionModeSet
(
    IN GT_U8                                devNum,
    IN CPSS_DXCH_MEMBER_SELECTION_MODE_ENT  selectionMode
)
{
    GT_U32    startBit;
    GT_U32    numOfBits;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_CH1_E| CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
          CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    switch(selectionMode)
    {
        case CPSS_DXCH_MEMBER_SELECTION_MODE_6_LSB_E:
            startBit = 0;
            numOfBits = 6;
            break;
        case CPSS_DXCH_MEMBER_SELECTION_MODE_6_MSB_E:
            startBit  = 6;
            numOfBits = 6;
            break;
        case CPSS_DXCH_MEMBER_SELECTION_MODE_12_BITS_E:
            startBit  = 0;
            numOfBits = 12;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    return cpssDxChTrunkHashNumBitsSet(devNum,CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_TRUNK_E,startBit,numOfBits);
}

/**
* @internal cpssDxChTrunkMemberSelectionModeSet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         Determines the number of bits used to calculate the Index of the trunk member.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] selectionMode            - member selection mode
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The selection mode configuration may be changed by:
*       - cpssDxChTrunkHashNumBitsSet.
*       - cpssDxChTrunkHashGlobalModeSet.
*       - cpssDxChTrunkInit.
*       - cpssDxChTrunkHashCrcParametersSet.
*
*/
GT_STATUS cpssDxChTrunkMemberSelectionModeSet
(
    IN GT_U8                                devNum,
    IN CPSS_DXCH_MEMBER_SELECTION_MODE_ENT  selectionMode
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTrunkMemberSelectionModeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, selectionMode));

    rc = internal_cpssDxChTrunkMemberSelectionModeSet(devNum, selectionMode);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, selectionMode));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTrunkMemberSelectionModeGet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         Return the number of bits used to calculate the Index of the trunk member.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] selectionModePtr         - (pointer to) member selection mode
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTrunkMemberSelectionModeGet
(
    IN  GT_U8                                 devNum,
    OUT CPSS_DXCH_MEMBER_SELECTION_MODE_ENT   *selectionModePtr
)
{
    GT_STATUS   rc;
    GT_U32    startBit;
    GT_U32    numOfBits;

    /* check parameters */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_CH1_E| CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
          CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(selectionModePtr);

    rc = prvCpssDxChTrunkHashBitsSelectionGet(devNum,CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_TRUNK_E,&startBit,&numOfBits);
    if(rc != GT_OK)
    {
        return rc;
    }

    if(startBit == 0)
    {
        if(numOfBits == 6)
        {
            *selectionModePtr = CPSS_DXCH_MEMBER_SELECTION_MODE_6_LSB_E;
        }
        else if((numOfBits == 12))
        {
            *selectionModePtr = CPSS_DXCH_MEMBER_SELECTION_MODE_12_BITS_E;
        }
        else
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }
    else
    if(startBit == 6 && numOfBits == 6)
    {
        *selectionModePtr = CPSS_DXCH_MEMBER_SELECTION_MODE_6_MSB_E;
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal cpssDxChTrunkMemberSelectionModeGet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         Return the number of bits used to calculate the Index of the trunk member.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] selectionModePtr         - (pointer to) member selection mode
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTrunkMemberSelectionModeGet
(
    IN  GT_U8                                 devNum,
    OUT CPSS_DXCH_MEMBER_SELECTION_MODE_ENT   *selectionModePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTrunkMemberSelectionModeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, selectionModePtr));

    rc = internal_cpssDxChTrunkMemberSelectionModeGet(devNum, selectionModePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, selectionModePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTrunkLearnPrioritySet function
* @endinternal
*
* @brief   Set Learn priority per trunk ID, used for Mac SA moved logic
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] trunkId                  - trunk ID
* @param[in] learnPriority            - Learn priority can be Low or High
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note There is a parallel API for port configuration: cpssDxChBrgFdbLearnPrioritySet
*
*/
static GT_STATUS internal_cpssDxChTrunkLearnPrioritySet
(
    IN GT_U8                                devNum,
    IN GT_TRUNK_ID                          trunkId,
    IN CPSS_DXCH_FDB_LEARN_PRIORITY_ENT     learnPriority
)
{
    GT_U32    hwData;      /* value to write */
    GT_U32    index;
    GT_U32    startBit;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_CH1_E| CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
          CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    if(trunkId > PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_TRUNK_ID_MAC(devNum))
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    switch(learnPriority)
    {
        case CPSS_DXCH_FDB_LEARN_PRIORITY_LOW_E:
            hwData=0;
            break;
        case CPSS_DXCH_FDB_LEARN_PRIORITY_HIGH_E:
            hwData=1;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    index = trunkId >> 3; /* /8 */
    startBit = 0 + (trunkId & 0x7) * 5;/* %8 */

    /* set Source-Trunk-Attribute table */
    return prvCpssDxChWriteTableEntryField(devNum,
                                            CPSS_DXCH_SIP5_TABLE_BRIDGE_SOURCE_TRUNK_ATTRIBUTE_E,
                                            index,
                                            PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
                                            startBit, /* start bit */
                                            1, /* 1 bit */
                                            hwData);
}

/**
* @internal cpssDxChTrunkLearnPrioritySet function
* @endinternal
*
* @brief   Set Learn priority per trunk ID, used for Mac SA moved logic
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] trunkId                  - trunk ID
* @param[in] learnPriority            - Learn priority can be Low or High
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note There is a parallel API for port configuration: cpssDxChBrgFdbLearnPrioritySet
*
*/
GT_STATUS cpssDxChTrunkLearnPrioritySet
(
    IN GT_U8                                devNum,
    IN GT_TRUNK_ID                          trunkId,
    IN CPSS_DXCH_FDB_LEARN_PRIORITY_ENT     learnPriority
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTrunkLearnPrioritySet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, trunkId, learnPriority));

    rc = internal_cpssDxChTrunkLearnPrioritySet(devNum, trunkId, learnPriority);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, trunkId, learnPriority));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTrunkLearnPriorityGet function
* @endinternal
*
* @brief   Get Learn priority per trunk ID
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] trunkId                  - trunk ID
*
* @param[out] learnPriorityPtr         - (pointer to)Learn priority can be Low or High
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note There is a parallel API for port configuration: cpssDxChBrgFdbLearnPriorityGet
*
*/
static GT_STATUS internal_cpssDxChTrunkLearnPriorityGet
(
    IN  GT_U8                               devNum,
    IN  GT_TRUNK_ID                         trunkId,
    OUT CPSS_DXCH_FDB_LEARN_PRIORITY_ENT    *learnPriorityPtr
)
{
    GT_STATUS rc;
    GT_U32    hwData;
    GT_U32    index;
    GT_U32    startBit;

    /* check parameters */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_CH1_E| CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
          CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(learnPriorityPtr);

    if(trunkId > PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_TRUNK_ID_MAC(devNum))
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    index = trunkId >> 3; /* /8 */
    startBit = 0 + (trunkId & 0x7) * 5;/* %8 */

    /* read from Source-Trunk-Attribute table */
    rc = prvCpssDxChReadTableEntryField(devNum,
                                        CPSS_DXCH_SIP5_TABLE_BRIDGE_SOURCE_TRUNK_ATTRIBUTE_E,
                                        index,
                                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
                                        startBit,/* start bit */
                                        1,/* 1 bit        */
                                        &hwData);
    if(rc!=GT_OK)
        return rc;

    switch(hwData)
    {
        case 0:
            *learnPriorityPtr=CPSS_DXCH_FDB_LEARN_PRIORITY_LOW_E;
            break;
        case 1:
            *learnPriorityPtr=CPSS_DXCH_FDB_LEARN_PRIORITY_HIGH_E;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_VALUE, LOG_ERROR_NO_MSG);
    }

    return rc;
}

/**
* @internal cpssDxChTrunkLearnPriorityGet function
* @endinternal
*
* @brief   Get Learn priority per trunk ID
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] trunkId                  - trunk ID
*
* @param[out] learnPriorityPtr         - (pointer to)Learn priority can be Low or High
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note There is a parallel API for port configuration: cpssDxChBrgFdbLearnPriorityGet
*
*/
GT_STATUS cpssDxChTrunkLearnPriorityGet
(
    IN  GT_U8                               devNum,
    IN  GT_TRUNK_ID                         trunkId,
    OUT CPSS_DXCH_FDB_LEARN_PRIORITY_ENT    *learnPriorityPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTrunkLearnPriorityGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, trunkId, learnPriorityPtr));

    rc = internal_cpssDxChTrunkLearnPriorityGet(devNum, trunkId, learnPriorityPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, trunkId, learnPriorityPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTrunkUserGroupSet function
* @endinternal
*
* @brief   Set User Group per trunk ID, used for Mac SA moved logic
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] trunkId                  - trunk ID
* @param[in] userGroup                - user group
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note There is a parallel API for port configuration: cpssDxChBrgFdbUserGroupSet
*
*/
static GT_STATUS internal_cpssDxChTrunkUserGroupSet
(
    IN GT_U8            devNum,
    IN GT_TRUNK_ID      trunkId,
    IN GT_U32           userGroup
)
{
    GT_U32    index;
    GT_U32    startBit;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_CH1_E| CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
          CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    if(trunkId > PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_TRUNK_ID_MAC(devNum))
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    if(userGroup >= BIT_4)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    index = trunkId >> 3; /* /8 */
    startBit = 1 + (trunkId & 0x7) * 5;/* %8 */

   /* set Source-Trunk-Attribute table */
    return prvCpssDxChWriteTableEntryField(devNum,
                                            CPSS_DXCH_SIP5_TABLE_BRIDGE_SOURCE_TRUNK_ATTRIBUTE_E,
                                            index,
                                            PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
                                            startBit, /* start bit */
                                            4, /* 4 bit */
                                            userGroup);
}

/**
* @internal cpssDxChTrunkUserGroupSet function
* @endinternal
*
* @brief   Set User Group per trunk ID, used for Mac SA moved logic
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] trunkId                  - trunk ID
* @param[in] userGroup                - user group
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note There is a parallel API for port configuration: cpssDxChBrgFdbUserGroupSet
*
*/
GT_STATUS cpssDxChTrunkUserGroupSet
(
    IN GT_U8            devNum,
    IN GT_TRUNK_ID      trunkId,
    IN GT_U32           userGroup
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTrunkUserGroupSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, trunkId, userGroup));

    rc = internal_cpssDxChTrunkUserGroupSet(devNum, trunkId, userGroup);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, trunkId, userGroup));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTrunkUserGroupGet function
* @endinternal
*
* @brief   Get User Group per trunk ID, used for Mac SA moved logic
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] trunkId                  - trunk ID
*
* @param[out] userGroupPtr             - (pointer to) user group
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note There is a parallel API for port configuration: cpssDxChBrgFdbUserGroupGet
*
*/
static GT_STATUS internal_cpssDxChTrunkUserGroupGet
(
    IN  GT_U8           devNum,
    IN GT_TRUNK_ID      trunkId,
    OUT GT_U32          *userGroupPtr
)
{
    GT_STATUS rc;
    GT_U32    hwData;
    GT_U32    index;
    GT_U32    startBit;

    /* check parameters */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_CH1_E| CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
          CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(userGroupPtr);

    if(trunkId > PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_TRUNK_ID_MAC(devNum))
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    index = trunkId >> 3; /* /8 */
    startBit = 1 + (trunkId & 0x7) * 5;/* %8 */

    /* read from Source-Trunk-Attribute table */
    rc = prvCpssDxChReadTableEntryField(devNum,
                                        CPSS_DXCH_SIP5_TABLE_BRIDGE_SOURCE_TRUNK_ATTRIBUTE_E,
                                        index,
                                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
                                        startBit,/* start bit */
                                        4,/* 4bit        */
                                        &hwData);

    *userGroupPtr=hwData;

    return rc;
}

/**
* @internal cpssDxChTrunkUserGroupGet function
* @endinternal
*
* @brief   Get User Group per trunk ID, used for Mac SA moved logic
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] trunkId                  - trunk ID
*
* @param[out] userGroupPtr             - (pointer to) user group
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note There is a parallel API for port configuration: cpssDxChBrgFdbUserGroupGet
*
*/
GT_STATUS cpssDxChTrunkUserGroupGet
(
    IN  GT_U8           devNum,
    IN GT_TRUNK_ID      trunkId,
    OUT GT_U32          *userGroupPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTrunkUserGroupGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, trunkId, userGroupPtr));

    rc = internal_cpssDxChTrunkUserGroupGet(devNum, trunkId, userGroupPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, trunkId, userGroupPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTrunkHashCrcSaltByteSet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         Set the CRC Salt value for specific byte .
*         relevant for CPSS_DXCH_TRUNK_LBH_PACKETS_INFO_CRC_E , CPSS_DXCH_TRUNK_LBH_CRC_32_MODE_E.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - The device number.
* @param[in] byteIndex                - the byte index one of the 70 bytes (APPLICABLE RANGES: 0..69)
* @param[in] saltValue                - the salt value of the byte (8 bits)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or byteIndex
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note related to feature 'CRC 32 hash mode'
*
*/
static GT_STATUS internal_cpssDxChTrunkHashCrcSaltByteSet
(
    IN GT_U8    devNum,
    IN GT_U32   byteIndex,
    IN GT_U8    saltValue
)
{
    GT_STATUS   rc;                     /* return value */
    GT_U32  regValue0,regValue1=0;      /*register 0,1 values that may hold the 8 bit value */
    GT_U32  regAddr0,regAddr1=0;        /*register 0,1 addresses that may hold the 8 bit value */
    GT_U32  index0,index1=0;            /*register 0,1 indexes in  CRC32Salt[] */
    GT_U32  startBit0;                  /*start bit in first register*/
    GT_U32  numBitsInReg = 28;          /*number of bits used in each register of CRC32Salt[]*/
    GT_U32  numBitsReg0,numBitsReg1=0;  /*register 0,1 num of bits that may hold the 8 bit value*/
    GT_U32  bitIndex;                   /*bit index (converted from byteIndex) */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_CH1_E| CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
          CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);

    if(byteIndex >= CRC_NUM_BYTES_USED_FOR_GENERATION_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    bitIndex = 8 * byteIndex;

    index0    = bitIndex / numBitsInReg; /* 28 bits in register */
    startBit0 = bitIndex % numBitsInReg; /* 28 bits in register */
    regValue0 = saltValue;

    if((startBit0 + 8) > numBitsInReg)
    {
        /* start of the 8 bits in one register and others in next register */
        index1 = index0 + 1;
        numBitsReg0 = numBitsInReg - startBit0;
        numBitsReg1 = 8 - numBitsReg0;

        regValue1  = saltValue >> numBitsReg0;
    }
    else
    {
        /* all 8 bits in the same register , no need for second register */
        numBitsReg0 = 8;
    }

    /* just to be on the safe side ... */
    if(index0 >= NUM_SULT_REG_MAC(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    else
    if(index1 >= NUM_SULT_REG_MAC(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regAddr0 = PRV_DXCH_REG1_UNIT_PCL_MAC(devNum).HASH.CRC32Salt[index0];

    rc = prvCpssHwPpSetRegField(devNum,regAddr0,startBit0,numBitsReg0,regValue0);
    if(rc != GT_OK)
    {
        return rc;
    }

    if(index1 != 0)
    {
        regAddr1 = PRV_DXCH_REG1_UNIT_PCL_MAC(devNum).HASH.CRC32Salt[index1];

        rc = prvCpssHwPpSetRegField(devNum,regAddr1,0,numBitsReg1,regValue1);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal cpssDxChTrunkHashCrcSaltByteSet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         Set the CRC Salt value for specific byte .
*         relevant for CPSS_DXCH_TRUNK_LBH_PACKETS_INFO_CRC_E , CPSS_DXCH_TRUNK_LBH_CRC_32_MODE_E.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - The device number.
* @param[in] byteIndex                - the byte index one of the 70 bytes (APPLICABLE RANGES: 0..69)
* @param[in] saltValue                - the salt value of the byte (8 bits)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or byteIndex
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note related to feature 'CRC 32 hash mode'
*
*/
GT_STATUS cpssDxChTrunkHashCrcSaltByteSet
(
    IN GT_U8    devNum,
    IN GT_U32   byteIndex,
    IN GT_U8    saltValue
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTrunkHashCrcSaltByteSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, byteIndex, saltValue));

    rc = internal_cpssDxChTrunkHashCrcSaltByteSet(devNum, byteIndex, saltValue);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, byteIndex, saltValue));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTrunkHashCrcSaltByteGet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         Get the CRC Salt value for specific byte .
*         relevant for CPSS_DXCH_TRUNK_LBH_PACKETS_INFO_CRC_E , CPSS_DXCH_TRUNK_LBH_CRC_32_MODE_E.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - The device number.
* @param[in] byteIndex                - the byte index one of the 70 bytes (APPLICABLE RANGES: 0..69)
*
* @param[out] saltValuePtr             - (pointer to)the salt value of the byte (8 bits)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or byteIndex
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note related to feature 'CRC 32 hash mode'
*
*/
static GT_STATUS internal_cpssDxChTrunkHashCrcSaltByteGet
(
    IN GT_U8    devNum,
    IN GT_U32   byteIndex,
    OUT GT_U8    *saltValuePtr
)
{
    GT_STATUS   rc;                     /* return value */
    GT_U32  regValue0,regValue1=0;      /*register 0,1 values that may hold the 8 bit value */
    GT_U32  regAddr0,regAddr1=0;        /*register 0,1 addresses that may hold the 8 bit value */
    GT_U32  index0,index1=0;            /*register 0,1 indexes in  CRC32Salt[] */
    GT_U32  startBit0;                  /*start bit in first register*/
    GT_U32  numBitsInReg = 28;          /*number of bits used in each register of CRC32Salt[]*/
    GT_U32  numBitsReg0,numBitsReg1=0;  /*register 0,1 num of bits that may hold the 8 bit value*/
    GT_U32  bitIndex;                   /*bit index (converted from byteIndex) */
    GT_U32  saltValue;                  /*the salt value (8 bits) that read from the register(s)*/

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_CH1_E| CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
          CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);

    if(byteIndex >= CRC_NUM_BYTES_USED_FOR_GENERATION_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    CPSS_NULL_PTR_CHECK_MAC(saltValuePtr);

    bitIndex = 8 * byteIndex;

    index0    = bitIndex / numBitsInReg; /* 28 bits in register */
    startBit0 = bitIndex % numBitsInReg; /* 28 bits in register */

    if((startBit0 + 8) > numBitsInReg)
    {
        /* start of the 8 bits in one register and others in next register */
        index1 = index0 + 1;
        numBitsReg0 = numBitsInReg - startBit0;
        numBitsReg1 = 8 - numBitsReg0;
    }
    else
    {
        /* all 8 bits in the same register , no need for second register */
        numBitsReg0 = 8;
    }

    /* just to be on the safe side ... */
    if(index0 >= NUM_SULT_REG_MAC(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    else
    if(index1 >= NUM_SULT_REG_MAC(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regAddr0 = PRV_DXCH_REG1_UNIT_PCL_MAC(devNum).HASH.CRC32Salt[index0];

    rc = prvCpssHwPpGetRegField(devNum,regAddr0,startBit0,numBitsReg0,&regValue0);
    if(rc != GT_OK)
    {
        return rc;
    }

    saltValue = regValue0;

    if(index1 != 0)
    {
        regAddr1 = PRV_DXCH_REG1_UNIT_PCL_MAC(devNum).HASH.CRC32Salt[index1];

        rc = prvCpssHwPpGetRegField(devNum,regAddr1,0,numBitsReg1,&regValue1);
        if(rc != GT_OK)
        {
            return rc;
        }

        saltValue |= regValue1 << numBitsReg0;
    }

    *saltValuePtr = (GT_U8)saltValue;


    return GT_OK;
}

/**
* @internal cpssDxChTrunkHashCrcSaltByteGet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         Get the CRC Salt value for specific byte .
*         relevant for CPSS_DXCH_TRUNK_LBH_PACKETS_INFO_CRC_E , CPSS_DXCH_TRUNK_LBH_CRC_32_MODE_E.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - The device number.
* @param[in] byteIndex                - the byte index one of the 70 bytes (APPLICABLE RANGES: 0..69)
*
* @param[out] saltValuePtr             - (pointer to)the salt value of the byte (8 bits)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or byteIndex
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note related to feature 'CRC 32 hash mode'
*
*/
GT_STATUS cpssDxChTrunkHashCrcSaltByteGet
(
    IN GT_U8    devNum,
    IN GT_U32   byteIndex,
    OUT GT_U8    *saltValuePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTrunkHashCrcSaltByteGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, byteIndex, saltValuePtr));

    rc = internal_cpssDxChTrunkHashCrcSaltByteGet(devNum, byteIndex, saltValuePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, byteIndex, saltValuePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTrunkHashMaskCrcParamOverrideSet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         Set which fields of the CRC hash input bytes are override by other fields.
*         for example : option to use FlowID instead of UDB21,UDB22.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - the device number.
* @param[in] fieldType                - the field type that used to override.
* @param[in] override                 -  the original field.
*                                      GT_TRUE  - override.
*                                      GT_FALSE - not override.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or fieldType
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note related to feature 'CRC hash mode'
*
*/
static GT_STATUS internal_cpssDxChTrunkHashMaskCrcParamOverrideSet
(
    IN GT_U8                                            devNum,
    IN CPSS_DXCH_TRUNK_HASH_MASK_CRC_PARAM_OVERRIDE_ENT fieldType,
    IN GT_BOOL                                          override
)
{
    GT_U32  regAddr;     /*register address*/
    GT_U32  bitIndex;    /*bit index in the register*/
    GT_U32  value;       /*value to write to the register*/

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_CH1_E| CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
          CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);

    regAddr = PRV_DXCH_REG1_UNIT_PCL_MAC(devNum).HASH.CRCHashConfig;

    value = BOOL2BIT_MAC(override);

    switch(fieldType)
    {
        case CPSS_DXCH_TRUNK_HASH_MASK_CRC_PARAM_OVERRIDE_UDB21_UDB22_BY_FLOW_ID_E:
            bitIndex = 2;
            break;
        case CPSS_DXCH_TRUNK_HASH_MASK_CRC_PARAM_OVERRIDE_MPLS_LABEL0_BY_EVLAN_E:
            bitIndex = 7;
            break;
        case CPSS_DXCH_TRUNK_HASH_MASK_CRC_PARAM_OVERRIDE_MPLS_LABEL1_BY_ORIGINAL_SOURCE_EPORT_E:
            bitIndex = 4;
            break;
        case CPSS_DXCH_TRUNK_HASH_MASK_CRC_PARAM_OVERRIDE_MPLS_LABEL2_BY_LOCAL_SOURCE_EPORT_E:
            bitIndex = 5;
            break;
        case CPSS_DXCH_TRUNK_HASH_MASK_CRC_PARAM_OVERRIDE_IPV6_ADDR_24MSB_BY_24UDBS_E:
            bitIndex = 6;

            value = 1 - value;/* when '24 udbs overrive 24 MSB of ipv6 addr' the field should be 0 */
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    return prvCpssHwPpSetRegField(devNum, regAddr, bitIndex, 1, value);
}

/**
* @internal cpssDxChTrunkHashMaskCrcParamOverrideSet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         Set which fields of the CRC hash input bytes are override by other fields.
*         for example : option to use FlowID instead of UDB21,UDB22.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - the device number.
* @param[in] fieldType                - the field type that used to override.
* @param[in] override                 -  the original field.
*                                      GT_TRUE  - override.
*                                      GT_FALSE - not override.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or fieldType
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note related to feature 'CRC hash mode'
*
*/
GT_STATUS cpssDxChTrunkHashMaskCrcParamOverrideSet
(
    IN GT_U8                                            devNum,
    IN CPSS_DXCH_TRUNK_HASH_MASK_CRC_PARAM_OVERRIDE_ENT fieldType,
    IN GT_BOOL                                          override
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTrunkHashMaskCrcParamOverrideSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, fieldType, override));

    rc = internal_cpssDxChTrunkHashMaskCrcParamOverrideSet(devNum, fieldType, override);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, fieldType, override));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTrunkHashMaskCrcParamOverrideGet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         Get which fields of the CRC hash input bytes are override by other fields.
*         for example : option to use FlowID instead of UDB21,UDB22.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - the device number.
* @param[in] fieldType                - the field type that used to override.
*
* @param[out] overridePtr              - (pointer to)override the original field.
*                                      GT_TRUE  - override.
*                                      GT_FALSE - not override.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or fieldType
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note related to feature 'CRC hash mode'
*
*/
static GT_STATUS internal_cpssDxChTrunkHashMaskCrcParamOverrideGet
(
    IN GT_U8                                            devNum,
    IN CPSS_DXCH_TRUNK_HASH_MASK_CRC_PARAM_OVERRIDE_ENT fieldType,
    OUT GT_BOOL                                          *overridePtr
)
{
    GT_STATUS   rc;      /*return code*/
    GT_U32  regAddr;     /*register address*/
    GT_U32  bitIndex;    /*bit index in the register*/
    GT_U32  value;       /*value read from the register*/

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_CH1_E| CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
          CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);

    CPSS_NULL_PTR_CHECK_MAC(overridePtr);

    regAddr = PRV_DXCH_REG1_UNIT_PCL_MAC(devNum).HASH.CRCHashConfig;

    switch(fieldType)
    {
        case CPSS_DXCH_TRUNK_HASH_MASK_CRC_PARAM_OVERRIDE_UDB21_UDB22_BY_FLOW_ID_E:
            bitIndex = 2;
            break;
        case CPSS_DXCH_TRUNK_HASH_MASK_CRC_PARAM_OVERRIDE_MPLS_LABEL0_BY_EVLAN_E:
            bitIndex = 7;
            break;
        case CPSS_DXCH_TRUNK_HASH_MASK_CRC_PARAM_OVERRIDE_MPLS_LABEL1_BY_ORIGINAL_SOURCE_EPORT_E:
            bitIndex = 4;
            break;
        case CPSS_DXCH_TRUNK_HASH_MASK_CRC_PARAM_OVERRIDE_MPLS_LABEL2_BY_LOCAL_SOURCE_EPORT_E:
            bitIndex = 5;
            break;
        case CPSS_DXCH_TRUNK_HASH_MASK_CRC_PARAM_OVERRIDE_IPV6_ADDR_24MSB_BY_24UDBS_E:
            bitIndex = 6;
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssHwPpGetRegField(devNum, regAddr, bitIndex, 1, &value);

    if(fieldType == CPSS_DXCH_TRUNK_HASH_MASK_CRC_PARAM_OVERRIDE_IPV6_ADDR_24MSB_BY_24UDBS_E)
    {
        /* when '24 udbs overrive 24 MSB of ipv6 addr' the field should be 0 */
        value = 1 - value;
    }

    *overridePtr = BIT2BOOL_MAC(value);

    return rc;
}

/**
* @internal cpssDxChTrunkHashMaskCrcParamOverrideGet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         Get which fields of the CRC hash input bytes are override by other fields.
*         for example : option to use FlowID instead of UDB21,UDB22.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - the device number.
* @param[in] fieldType                - the field type that used to override.
*
* @param[out] overridePtr              - (pointer to)override the original field.
*                                      GT_TRUE  - override.
*                                      GT_FALSE - not override.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or fieldType
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note related to feature 'CRC hash mode'
*
*/
GT_STATUS cpssDxChTrunkHashMaskCrcParamOverrideGet
(
    IN GT_U8                                            devNum,
    IN CPSS_DXCH_TRUNK_HASH_MASK_CRC_PARAM_OVERRIDE_ENT fieldType,
    OUT GT_BOOL                                          *overridePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTrunkHashMaskCrcParamOverrideGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, fieldType, overridePtr));

    rc = internal_cpssDxChTrunkHashMaskCrcParamOverrideGet(devNum, fieldType, overridePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, fieldType, overridePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTrunkPortMcEnableSet function
* @endinternal
*
* @brief   Function Relevant mode : High Level mode
*         This function sets the mode how multicast destination traffic egress the
*         selected port while it is a trunk member.
*         see details in description of CPSS_TRUNK_DESIGNATED_TABLE_MC_PORT_MODE_E
*
* @note   APPLICABLE DEVICES:      xCat3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number.
* @param[in] mode                     - the 'designated mc table'  of the port
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_NOT_INITIALIZED       - the trunk library was not initialized for the device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device or port number or mode
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note If disabled multicast destination packets would not egress through the
*       configured port.
*       This configuration has no influence on unicast destination traffic or
*       when the port is not a trunk member.
*
*/
static GT_STATUS internal_cpssDxChTrunkPortMcEnableSet
(
    IN GT_U8                  devNum,
    IN GT_PHYSICAL_PORT_NUM   portNum,
    IN CPSS_TRUNK_DESIGNATED_TABLE_MC_PORT_MODE_ENT mode
)
{
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E |CPSS_XCAT_E |
         CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_ENHANCED_PHY_PORT_CHECK_MAC(devNum,portNum);

    return prvCpssGenericTrunkPortMcEnableSet(devNum,portNum,mode);
}

/**
* @internal cpssDxChTrunkPortMcEnableSet function
* @endinternal
*
* @brief   Function Relevant mode : High Level mode
*         This function sets the mode how multicast destination traffic egress the
*         selected port while it is a trunk member.
*         see details in description of CPSS_TRUNK_DESIGNATED_TABLE_MC_PORT_MODE_E
*
* @note   APPLICABLE DEVICES:      xCat3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number.
* @param[in] mode                     - the 'designated mc table'  of the port
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_NOT_INITIALIZED       - the trunk library was not initialized for the device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device or port number or mode
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note If disabled multicast destination packets would not egress through the
*       configured port.
*       This configuration has no influence on unicast destination traffic or
*       when the port is not a trunk member.
*
*/
GT_STATUS cpssDxChTrunkPortMcEnableSet
(
    IN GT_U8                  devNum,
    IN GT_PHYSICAL_PORT_NUM   portNum,
    IN CPSS_TRUNK_DESIGNATED_TABLE_MC_PORT_MODE_ENT mode
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTrunkPortMcEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, mode));

    rc = internal_cpssDxChTrunkPortMcEnableSet(devNum, portNum, mode);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, mode));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTrunkDbPortMcEnableGet function
* @endinternal
*
* @brief   Function Relevant mode : High Level mode
*         This function gets the mode how multicast destination traffic egress the
*         selected port while it is a trunk member.
*         see details in description of CPSS_TRUNK_DESIGNATED_TABLE_MC_PORT_MODE_ENT
*
* @note   APPLICABLE DEVICES:      xCat3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number.
*
* @param[out] modePtr                  - (pointer to) the 'designated mc table' mode of the port
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_NOT_INITIALIZED       - the trunk library was not initialized for the device
* @retval GT_BAD_PARAM             - bad device or port number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The function output is retrieved from CPSS related trunk DB.
*       This configuration has no influence on unicast destination traffic or
*       when the port is not a trunk member.
*
*/
static GT_STATUS internal_cpssDxChTrunkDbPortMcEnableGet
(
    IN  GT_U8                  devNum,
    IN  GT_PHYSICAL_PORT_NUM   portNum,
    OUT CPSS_TRUNK_DESIGNATED_TABLE_MC_PORT_MODE_ENT  *modePtr
)
{
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E |CPSS_XCAT_E |
         CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_ENHANCED_PHY_PORT_CHECK_MAC(devNum,portNum);

    return prvCpssGenericTrunkDbPortMcEnableGet(devNum,portNum,modePtr);
}

/**
* @internal cpssDxChTrunkDbPortMcEnableGet function
* @endinternal
*
* @brief   Function Relevant mode : High Level mode
*         This function gets the mode how multicast destination traffic egress the
*         selected port while it is a trunk member.
*         see details in description of CPSS_TRUNK_DESIGNATED_TABLE_MC_PORT_MODE_ENT
*
* @note   APPLICABLE DEVICES:      xCat3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number.
*
* @param[out] modePtr                  - (pointer to) the 'designated mc table' mode of the port
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_NOT_INITIALIZED       - the trunk library was not initialized for the device
* @retval GT_BAD_PARAM             - bad device or port number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The function output is retrieved from CPSS related trunk DB.
*       This configuration has no influence on unicast destination traffic or
*       when the port is not a trunk member.
*
*/
GT_STATUS cpssDxChTrunkDbPortMcEnableGet
(
    IN  GT_U8                  devNum,
    IN  GT_PHYSICAL_PORT_NUM   portNum,
    OUT CPSS_TRUNK_DESIGNATED_TABLE_MC_PORT_MODE_ENT  *modePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTrunkDbPortMcEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, modePtr));

    rc = internal_cpssDxChTrunkDbPortMcEnableGet(devNum, portNum, modePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, modePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTrunkDbPortTrunkIdModeSet function
* @endinternal
*
* @brief   Function Relevant mode : High level mode
*         Function allow application to be responsible for the 'TrunkId of the port'.
*         in High level mode APIs the 'TrunkId of the port' is set internally be
*         the CPSS and may override the current value of the 'TrunkId of the port'.
*         This API allows application to notify the CPSS to not manage the 'TrunkId
*         of the port'.
*         function cpssDxChTrunkPortTrunkIdSet(...) allow application to manage
*         the trunkId of the port
*         NOTE: this is 'DB' operation (database) , without HW access.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] portNum                  - physical port number.
* @param[in] manageMode               - the management mode.
*
* @retval GT_OK                    - successful completion
* @retval GT_FAIL                  - an error occurred.
* @retval GT_BAD_PARAM             - bad device number , or
*                                       bad port number , or manageMode
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTrunkDbPortTrunkIdModeSet
(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN CPSS_TRUNK_PORT_TRUNK_ID_MANAGEMENT_MODE_ENT manageMode
)
{
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    return prvCpssGenericTrunkDbPortTrunkIdModeSet(devNum,portNum,manageMode);
}

/**
* @internal cpssDxChTrunkDbPortTrunkIdModeSet function
* @endinternal
*
* @brief   Function Relevant mode : High level mode
*         Function allow application to be responsible for the 'TrunkId of the port'.
*         in High level mode APIs the 'TrunkId of the port' is set internally be
*         the CPSS and may override the current value of the 'TrunkId of the port'.
*         This API allows application to notify the CPSS to not manage the 'TrunkId
*         of the port'.
*         function cpssDxChTrunkPortTrunkIdSet(...) allow application to manage
*         the trunkId of the port
*         NOTE: this is 'DB' operation (database) , without HW access.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] portNum                  - physical port number.
* @param[in] manageMode               - the management mode.
*
* @retval GT_OK                    - successful completion
* @retval GT_FAIL                  - an error occurred.
* @retval GT_BAD_PARAM             - bad device number , or
*                                       bad port number , or manageMode
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTrunkDbPortTrunkIdModeSet
(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN CPSS_TRUNK_PORT_TRUNK_ID_MANAGEMENT_MODE_ENT manageMode
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTrunkDbPortTrunkIdModeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, manageMode));

    rc = internal_cpssDxChTrunkDbPortTrunkIdModeSet(devNum, portNum, manageMode);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, manageMode));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChTrunkDbPortTrunkIdModeGet function
* @endinternal
*
* @brief   Function Relevant mode : High level mode
*         Function get the 'management mode' of the port's trunkId.
*         NOTE: this is 'DB' operation (database) , without HW access.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] portNum                  - physical port number.
*
* @param[out] manageModePtr            - (pointer to) the management mode.
*
* @retval GT_OK                    - successful completion
* @retval GT_FAIL                  - an error occurred.
* @retval GT_BAD_PARAM             - bad device number , or
*                                       bad port number , or manageMode
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTrunkDbPortTrunkIdModeGet
(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    OUT CPSS_TRUNK_PORT_TRUNK_ID_MANAGEMENT_MODE_ENT *manageModePtr
)
{
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    return prvCpssGenericTrunkDbPortTrunkIdModeGet(devNum,portNum,manageModePtr);
}

/**
* @internal cpssDxChTrunkDbPortTrunkIdModeGet function
* @endinternal
*
* @brief   Function Relevant mode : High level mode
*         Function get the 'management mode' of the port's trunkId.
*         NOTE: this is 'DB' operation (database) , without HW access.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] portNum                  - physical port number.
*
* @param[out] manageModePtr            - (pointer to) the management mode.
*
* @retval GT_OK                    - successful completion
* @retval GT_FAIL                  - an error occurred.
* @retval GT_BAD_PARAM             - bad device number , or
*                                       bad port number , or manageMode
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTrunkDbPortTrunkIdModeGet
(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    OUT CPSS_TRUNK_PORT_TRUNK_ID_MANAGEMENT_MODE_ENT *manageModePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTrunkDbPortTrunkIdModeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, manageModePtr));

    rc = internal_cpssDxChTrunkDbPortTrunkIdModeGet(devNum, portNum, manageModePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, manageModePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvTrunkFlexInfoSet function
* @endinternal
*
* @brief   In mode of 'CPSS_DXCH_TRUNK_MEMBERS_MODE_FLEX_E' state per 'regular' trunk
*         the 'start index' in the 'L2 ECMP' table , and the number of consecutive
*         entries that will be reserved for this trunk(in 'L2 ECMP').
*         NOTE:
*         1. See more details about this mode in description of 'CPSS_DXCH_TRUNK_MEMBERS_MODE_FLEX_E'
*         2. This API is NOT intend to 'move' trunks from one range in the 'L2 ECMP'
*         to another range in this table.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - the device number .
* @param[in] trunkId                  - the trunk id.
* @param[in] l2EcmpStartIndex         - The 'start index' in the 'L2 ECMP' table.
*                                      (APPLICABLE RANGES: 0..('L2 ECMP' table size - 1))
* @param[in] maxNumOfMembers          - 1. The number of consecutive entries that will be reserved
*                                      for this trunk(in 'L2 ECMP')
*                                      2. The trunk will not be able to hold more members
*                                      than this value.
*                                      (APPLICABLE RANGES: 0..4096)
* @param[in] fixedSize                - indication that we are called 'in loop' and all trunks get
*                                      the same size.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_INITIALIZED       -the trunk library was not initialized for the device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or bad trunkId number
* @retval GT_OUT_OF_RANGE          - l2EcmpStartIndex is out of range , or maxNumOfMembers is out of range
* @retval GT_BAD_STATE             - 1. mode is not CPSS_DXCH_TRUNK_MEMBERS_MODE_FLEX_E
* @retval 2. the trunk             -type is CPSS_TRUNK_TYPE_REGULAR_E and
*                                       l2EcmpStartIndex is different from already exists or
*                                       maxNumOfMembers is less than current HW value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. For 'better' load balance on 'small number' of flows , the CPSS will
*       set <Number Of L2 ECMP Paths> in the LTT with value in the range of
*       'actual number of members' ... maxNumOfMembers.
*       2. if the trunk already used as 'regular' trunk (hold members), the application
*       can increase 'maxNumOfMembers' or decrease if not less than current HW value.
*       (need to keep l2EcmpStartIndex as already exist)
*
*/
static GT_STATUS prvTrunkFlexInfoSet
(
    IN GT_U8            devNum,
    IN GT_TRUNK_ID      trunkId,
    IN GT_U32           l2EcmpStartIndex,
    IN GT_U32           maxNumOfMembers,
    IN GT_BOOL          fixedSize
)
{
    GT_STATUS   rc;
    PRV_CPSS_TRUNK_DB_INFO_STC *devTrunkInfoPtr = PRV_CPSS_DEV_TRUNK_INFO_MAC(devNum);
    PRV_CPSS_TRUNK_ENTRY_INFO_STC *trunkEntryPtr;
    GT_U32  baseStartMembersIndex;
    GT_U32  relativeStartMembersIndex;
    GT_U32  numL2EcmpPaths;
    GT_U32  numBits_L2_ECMP_Start_Index;/* number of bits in <L2_ECMP_Start_Index> */
    GT_U32  data;

    if(fixedSize == GT_FALSE &&
       devTrunkInfoPtr->loadBalanceType != PRV_CPSS_TRUNK_LOAD_BALANCE_FLEX_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "Trunk Mode must be 'flex'");
    }

    rc = prvCpssGenericTrunkDbIsValid(devNum,trunkId);
    if(rc != GT_OK)
    {
        return rc;
    }

    trunkEntryPtr = &devTrunkInfoPtr->trunksArray[trunkId];

    if(trunkEntryPtr->flexInfo.myTrunkMaxNumOfMembers)
    {
        if(trunkEntryPtr->flexInfo.myTrunkStartIndexInMembersTable != l2EcmpStartIndex)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,
                "Not supporting 'movement' in L2ECMP from[%d] to [%d] in trunkId[%d]",
                trunkEntryPtr->flexInfo.myTrunkStartIndexInMembersTable,
                l2EcmpStartIndex,
                trunkId);
        }

        /* check if we not try to reduce to value that is less than current HW value */
        if(trunkEntryPtr->flexInfo.myTrunkMaxNumOfMembers > maxNumOfMembers)
        {
            numBits_L2_ECMP_Start_Index = NUMBITS_L2_ECMP_START_INDEX_MAC(devNum);

            /* we need to read the trunk LTT and get the current number of members */
            rc = prvCpssDxChReadTableEntry(devNum,
                CPSS_DXCH_SIP5_TABLE_EQ_TRUNK_LTT_E,
                trunkId,
                &data);
            if(rc != GT_OK)
            {
                return rc;
            }

            if(data & 1)  /*<ECMP Enable>*/
            {
                /*<Number of L2 ECMP Paths>*/
                numL2EcmpPaths = data >> (2+numBits_L2_ECMP_Start_Index);

                numL2EcmpPaths++;/* value in HW is 1 less than actual number of members */

                if(numL2EcmpPaths > maxNumOfMembers)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,
                        "Not supporting reduce on max num of members from[%d] to [%d] in trunkId[%d] , because HW currently uses[%d] members",
                        trunkEntryPtr->flexInfo.myTrunkMaxNumOfMembers,
                        maxNumOfMembers,
                        trunkId,
                        numL2EcmpPaths);
                }
            }
        }

    }

    if(fixedSize == GT_TRUE)
    {
        /* the fixed size is reserving consecutive index at the end of the L2ECMP table */
        baseStartMembersIndex =
            PRV_CPSS_DXCH_PP_MAC(devNum)->bridge.l2EcmpNumMembers -
            PRV_CPSS_DXCH_PP_MAC(devNum)->bridge.l2EcmpNumMembers_usedByTrunk;
    }
    else
    {
        /* in the fully flex mode , all the L2 ecmp table can be used */
        baseStartMembersIndex = 0;
    }

    if(l2EcmpStartIndex < baseStartMembersIndex)
    {
        /* should not happen ... but code exists to make sure that
            relativeStartMembersIndex will not be 'negative' */

        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE,
            "l2EcmpStartIndex [%d] must not be less than [%d] in trunkId[%d]",
            l2EcmpStartIndex,
            baseStartMembersIndex,
            trunkId);
    }

    relativeStartMembersIndex = l2EcmpStartIndex - baseStartMembersIndex;

    /* update the info in the DB */
    trunkEntryPtr->flexInfo.myTrunkStartIndexInMembersTable = l2EcmpStartIndex;
    trunkEntryPtr->flexInfo.myTrunkMaxNumOfMembers          = maxNumOfMembers;

    /* NOTE: the HW will be changed ONLY when there will be manipulation on the
       trunk members ! */
    devTrunkInfoPtr->trunksArray[trunkId].membersPtr =
            &devTrunkInfoPtr->allMembersArray[relativeStartMembersIndex];
    devTrunkInfoPtr->trunksArray[trunkId].isLocalMembersPtr =
            &devTrunkInfoPtr->allMembersIsLocalArray[relativeStartMembersIndex];

    return GT_OK;
}

/**
* @internal internal_cpssDxChTrunkFlexInfoSet function
* @endinternal
*
* @brief   In mode of 'CPSS_DXCH_TRUNK_MEMBERS_MODE_FLEX_E' state per 'regular' trunk
*         the 'start index' in the 'L2 ECMP' table , and the number of consecutive
*         entries that will be reserved for this trunk(in 'L2 ECMP').
*         NOTE:
*         1. See more details about this mode in description of 'CPSS_DXCH_TRUNK_MEMBERS_MODE_FLEX_E'
*         2. This API is NOT intend to 'move' trunks from one range in the 'L2 ECMP'
*         to another range in this table.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - the device number .
* @param[in] trunkId                  - the trunk id.
* @param[in] l2EcmpStartIndex         - The 'start index' in the 'L2 ECMP' table.
*                                      (APPLICABLE RANGES: 0..('L2 ECMP' table size - 1))
* @param[in] maxNumOfMembers          - 1. The number of consecutive entries that will be reserved
*                                      for this trunk(in 'L2 ECMP')
*                                      2. The trunk will not be able to hold more members
*                                      than this value.
*                                      (APPLICABLE RANGES: 0..4096)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_INITIALIZED       -the trunk library was not initialized for the device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or bad trunkId number
* @retval GT_OUT_OF_RANGE          - l2EcmpStartIndex is out of range , or maxNumOfMembers is out of range
* @retval GT_BAD_STATE             - 1. mode is not CPSS_DXCH_TRUNK_MEMBERS_MODE_FLEX_E
* @retval 2. the trunk             -type is CPSS_TRUNK_TYPE_REGULAR_E and
*                                       l2EcmpStartIndex is different from already exists or
*                                       maxNumOfMembers is less than current HW value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. For 'better' load balance on 'small number' of flows , the CPSS will
*       set <Number Of L2 ECMP Paths> in the LTT with value in the range of
*       'actual number of members' ... maxNumOfMembers.
*       2. if the trunk already used as 'regular' trunk (hold members), the application
*       can increase 'maxNumOfMembers' or decrease if not less than current HW value.
*       (need to keep l2EcmpStartIndex as already exist)
*
*/
static GT_STATUS internal_cpssDxChTrunkFlexInfoSet
(
    IN GT_U8            devNum,
    IN GT_TRUNK_ID      trunkId,
    IN GT_U32           l2EcmpStartIndex,
    IN GT_U32           maxNumOfMembers
)
{
    GT_STATUS   rc;
    PRV_CPSS_TRUNK_DB_INFO_STC *devTrunkInfoPtr = NULL;
    GT_U32  origValue_l2EcmpNumMembers_usedByTrunk;
    GT_U32  trunkIdIndex;
    GT_U32  l2EcmpNumMembers;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
         CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);

    l2EcmpNumMembers = PRV_CPSS_DXCH_PP_MAC(devNum)->bridge.l2EcmpNumMembers;
    L2_ECMP_NULL_PORT_BOOKMARK
    /* we use last index to hold the NULL port */
    l2EcmpNumMembers --;

    if(trunkId == TRUNK_ID_ALL_CNS)
    {
        /* indication to set all the trunks with the same max size */
        devTrunkInfoPtr = PRV_CPSS_DEV_TRUNK_INFO_MAC(devNum);

        origValue_l2EcmpNumMembers_usedByTrunk =
            PRV_CPSS_DXCH_PP_MAC(devNum)->bridge.l2EcmpNumMembers_usedByTrunk;

        if(l2EcmpNumMembers <
            (maxNumOfMembers * devTrunkInfoPtr->numberOfTrunks))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
                "can't support [%d] trunks with max [%d] members ,as the device limited to [%d] members",
                devTrunkInfoPtr->numberOfTrunks,
                maxNumOfMembers,
                l2EcmpNumMembers);
        }


        /* we need to 'reserve' in the L2ECMP table entries that will be used
           by the trunks

            modify it before the LOOP because the function prvTrunkFlexInfoSet(...)
            need to use the updated value
        */
        PRV_CPSS_DXCH_PP_MAC(devNum)->bridge.l2EcmpNumMembers_usedByTrunk =
            maxNumOfMembers * devTrunkInfoPtr->numberOfTrunks;
        L2_ECMP_NULL_PORT_BOOKMARK
        /* we use last index to hold the NULL port */
        PRV_CPSS_DXCH_PP_MAC(devNum)->bridge.l2EcmpNumMembers_usedByTrunk++;

        l2EcmpStartIndex = l2EcmpNumMembers;

        for (trunkIdIndex = devTrunkInfoPtr->numberOfTrunks ; trunkIdIndex > 0 ; trunkIdIndex --)
        {
            l2EcmpStartIndex -= maxNumOfMembers;
            rc = prvTrunkFlexInfoSet(devNum,(GT_TRUNK_ID)trunkIdIndex, l2EcmpStartIndex , maxNumOfMembers , GT_TRUE);
            if(rc != GT_OK)
            {
                /* restore value */
                PRV_CPSS_DXCH_PP_MAC(devNum)->bridge.l2EcmpNumMembers_usedByTrunk =
                    origValue_l2EcmpNumMembers_usedByTrunk;

                return rc;
            }
        }

        /* state that all trunks use the same value of 'max' members */
        devTrunkInfoPtr->maxNumMembersInTrunk_isPerTrunk = GT_FALSE;
        devTrunkInfoPtr->maxNumMembersInTrunk  = maxNumOfMembers;
        devTrunkInfoPtr->maxNumMembersInDevice =
            maxNumOfMembers * devTrunkInfoPtr->numberOfTrunks;

        /* set the updated limits */
        devTrunkInfoPtr->numNonTrunkEntriesHw = devTrunkInfoPtr->numberOfTrunks;
        devTrunkInfoPtr->numTrunksSupportedHw = devTrunkInfoPtr->numberOfTrunks;
    }
    else
    {
        /* need to check each one , before 'adding' them together
           to avoid 'GT_U32' wraparound problem */
        if(l2EcmpStartIndex > l2EcmpNumMembers ||
            maxNumOfMembers > l2EcmpNumMembers)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE,
            "l2EcmpStartIndex[%d] and maxNumOfMembers[%d]  must NOT be more than [%d]",
            l2EcmpStartIndex,
            maxNumOfMembers,
            l2EcmpNumMembers);
        }
        else
        if((l2EcmpStartIndex+maxNumOfMembers) >
            l2EcmpNumMembers)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE,
            "l2EcmpStartIndex[%d] + maxNumOfMembers[%d]  = [%d] must NOT be more than [%d]",
            l2EcmpStartIndex,
            maxNumOfMembers,
            l2EcmpStartIndex+maxNumOfMembers,
            l2EcmpNumMembers);
        }

        /* set specific trunk with the needed info */
        rc = prvTrunkFlexInfoSet(devNum,trunkId, l2EcmpStartIndex , maxNumOfMembers , GT_FALSE);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal cpssDxChTrunkFlexInfoSet function
* @endinternal
*
* @brief   In mode of 'CPSS_DXCH_TRUNK_MEMBERS_MODE_FLEX_E' state per 'regular' trunk
*         the 'start index' in the 'L2 ECMP' table , and the number of consecutive
*         entries that will be reserved for this trunk(in 'L2 ECMP').
*         NOTE:
*         1. See more details about this mode in description of 'CPSS_DXCH_TRUNK_MEMBERS_MODE_FLEX_E'
*         2. This API is NOT intend to 'move' trunks from one range in the 'L2 ECMP'
*         to another range in this table.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - the device number .
* @param[in] trunkId                  - the trunk id.
* @param[in] l2EcmpStartIndex         - The 'start index' in the 'L2 ECMP' table.
*                                      (APPLICABLE RANGES: 0..('L2 ECMP' table size - 1))
* @param[in] maxNumOfMembers          - 1. The number of consecutive entries that will be reserved
*                                      for this trunk(in 'L2 ECMP')
*                                      2. The trunk will not be able to hold more members
*                                      than this value.
*                                      (APPLICABLE RANGES: 0..4096)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_INITIALIZED       -the trunk library was not initialized for the device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or bad trunkId number
* @retval GT_OUT_OF_RANGE          - l2EcmpStartIndex is out of range , or maxNumOfMembers is out of range
* @retval GT_BAD_STATE             - 1. mode is not CPSS_DXCH_TRUNK_MEMBERS_MODE_FLEX_E
* @retval 2. the trunk             -type is CPSS_TRUNK_TYPE_REGULAR_E and
*                                       l2EcmpStartIndex is different from already exists or
*                                       maxNumOfMembers is less than current HW value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. For 'better' load balance on 'small number' of flows , the CPSS will
*       set <Number Of L2 ECMP Paths> in the LTT with value in the range of
*       'actual number of members' ... maxNumOfMembers.
*       2. if the trunk already used as 'regular' trunk (hold members), the application
*       can increase 'maxNumOfMembers' or decrease if not less than current HW value.
*       (need to keep l2EcmpStartIndex as already exist)
*
*/
GT_STATUS cpssDxChTrunkFlexInfoSet
(
    IN GT_U8            devNum,
    IN GT_TRUNK_ID      trunkId,
    IN GT_U32           l2EcmpStartIndex,
    IN GT_U32           maxNumOfMembers
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTrunkFlexInfoSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, trunkId, l2EcmpStartIndex,maxNumOfMembers));

    rc = internal_cpssDxChTrunkFlexInfoSet(devNum, trunkId, l2EcmpStartIndex,maxNumOfMembers);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, trunkId, l2EcmpStartIndex,maxNumOfMembers));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTrunkDbFlexInfoGet function
* @endinternal
*
* @brief   Get per trunk the 'start index' in the 'L2 ECMP' table , and the number of consecutive
*         entries reserved for this trunk(in 'L2 ECMP').
*         function uses the DB (no HW operations)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - the device number .
* @param[in] trunkId                  - the trunk id.
*
* @param[out] l2EcmpStartIndexPtr      - (pointer to) The 'start index' in the 'L2 ECMP' table.
* @param[out] maxNumOfMembersPtr       - (pointer to) The number of consecutive entries reserved
*                                      for this trunk(in 'L2 ECMP')
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_INITIALIZED       -the trunk library was not initialized for the device
*                                       or the trunk was not set with such info.
* @retval GT_BAD_PARAM             - bad device number , or bad trunkId number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. Function retrieve from DB (database) info that was set in cpssDxChTrunkFlexInfoSet()
*       2. function will return 'not initialized' for trunk that was not set via
*       cpssDxChTrunkFlexInfoSet()
*
*/
static GT_STATUS internal_cpssDxChTrunkDbFlexInfoGet
(
    IN GT_U8            devNum,
    IN GT_TRUNK_ID      trunkId,
    OUT GT_U32          *l2EcmpStartIndexPtr,
    OUT GT_U32          *maxNumOfMembersPtr
)
{
    GT_STATUS   rc;
    PRV_CPSS_TRUNK_DB_INFO_STC *devTrunkInfoPtr;
    PRV_CPSS_TRUNK_ENTRY_INFO_STC *trunkEntryPtr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
         CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);

    CPSS_NULL_PTR_CHECK_MAC(l2EcmpStartIndexPtr);
    CPSS_NULL_PTR_CHECK_MAC(maxNumOfMembersPtr);

    devTrunkInfoPtr = PRV_CPSS_DEV_TRUNK_INFO_MAC(devNum);

    if(devTrunkInfoPtr->loadBalanceType != PRV_CPSS_TRUNK_LOAD_BALANCE_FLEX_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, "Trunk Mode must be 'flex'");
    }

    rc = prvCpssGenericTrunkDbIsValid(devNum,trunkId);
    if(rc != GT_OK)
    {
        return rc;
    }

    trunkEntryPtr = &devTrunkInfoPtr->trunksArray[trunkId];

    /* get the info from the DB (not from HW ... HW hold 'current' number of used members) */
    *l2EcmpStartIndexPtr = trunkEntryPtr->flexInfo.myTrunkStartIndexInMembersTable;
    *maxNumOfMembersPtr  = trunkEntryPtr->flexInfo.myTrunkMaxNumOfMembers;

    return GT_OK;
}


/**
* @internal cpssDxChTrunkDbFlexInfoGet function
* @endinternal
*
* @brief   Get per trunk the 'start index' in the 'L2 ECMP' table , and the number of consecutive
*         entries reserved for this trunk(in 'L2 ECMP').
*         function uses the DB (no HW operations)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - the device number .
* @param[in] trunkId                  - the trunk id.
*
* @param[out] l2EcmpStartIndexPtr      - (pointer to) The 'start index' in the 'L2 ECMP' table.
* @param[out] maxNumOfMembersPtr       - (pointer to) The number of consecutive entries reserved
*                                      for this trunk(in 'L2 ECMP')
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_INITIALIZED       -the trunk library was not initialized for the device
*                                       or the trunk was not set with such info.
* @retval GT_BAD_PARAM             - bad device number , or bad trunkId number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. Function retrieve from DB (database) info that was set in cpssDxChTrunkFlexInfoSet()
*       2. function will return 'not initialized' for trunk that was not set via
*       cpssDxChTrunkFlexInfoSet()
*
*/
GT_STATUS cpssDxChTrunkDbFlexInfoGet
(
    IN GT_U8            devNum,
    IN GT_TRUNK_ID      trunkId,
    OUT GT_U32          *l2EcmpStartIndexPtr,
    OUT GT_U32          *maxNumOfMembersPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTrunkDbFlexInfoGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, trunkId, l2EcmpStartIndexPtr,maxNumOfMembersPtr));

    rc = internal_cpssDxChTrunkDbFlexInfoGet(devNum, trunkId, l2EcmpStartIndexPtr,maxNumOfMembersPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, trunkId, l2EcmpStartIndexPtr,maxNumOfMembersPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTrunkEcmpLttTableSet function
* @endinternal
*
* @brief   Function Relevant mode : Low Level mode
*         DESCRIPTION:
*         Set Trunk ECMP LTT entry info.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] trunkId                  - the  (LTT index)
*                                      (APPLICABLE RANGES: 0..4095)
* @param[in] ecmpLttInfoPtr           - (pointer to) ECMP LTT Information
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameters
* @retval GT_OUT_OF_RANGE          - on out of range ecmpStartIndex or ecmpNumOfPaths
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTrunkEcmpLttTableSet
(
    IN GT_U8                                devNum,
    IN GT_TRUNK_ID                          trunkId,
    IN CPSS_DXCH_TRUNK_ECMP_LTT_ENTRY_STC  *ecmpLttInfoPtr
)
{
    GT_STATUS   rc;
    GT_U32  numBits_L2_ECMP_Start_Index;/* number of bits in <L2_ECMP_Start_Index> */
    GT_U32  data;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
         CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);

    CPSS_NULL_PTR_CHECK_MAC(ecmpLttInfoPtr);

    if(trunkId > PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.trunksNum)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
            "trunkId[%d] must NOT be more than [%d] \n",
            trunkId ,
            PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.trunksNum);
    }

    if(ecmpLttInfoPtr->ecmpNumOfPaths > _4K)/* = 4K is ok !! */
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
            "ecmpNumOfPaths must not be more than [%d] to fit the HW field \n",
            _4K);
    }

    if(ecmpLttInfoPtr->ecmpNumOfPaths == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
            "ecmpNumOfPaths must not be 0 \n");
    }

    if((ecmpLttInfoPtr->ecmpStartIndex + ecmpLttInfoPtr->ecmpNumOfPaths) >
        PRV_CPSS_DXCH_PP_MAC(devNum)->bridge.l2EcmpNumMembers)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
            "(ecmpStartIndex[%d]+ecmpNumOfPaths[%d])=[%d] must NOT be more than [%d] \n",
            ecmpLttInfoPtr->ecmpStartIndex ,
            ecmpLttInfoPtr->ecmpNumOfPaths ,
            (ecmpLttInfoPtr->ecmpStartIndex + ecmpLttInfoPtr->ecmpNumOfPaths),
            PRV_CPSS_DXCH_PP_MAC(devNum)->bridge.l2EcmpNumMembers);
    }

    numBits_L2_ECMP_Start_Index = NUMBITS_L2_ECMP_START_INDEX_MAC(devNum);

    data = BOOL2BIT_MAC(ecmpLttInfoPtr->ecmpEnable) << 0 |
           BOOL2BIT_MAC(ecmpLttInfoPtr->ecmpRandomPathEnable) << 1 | /*<Random ECMP Path Enable>*/
           ((ecmpLttInfoPtr->ecmpNumOfPaths-1) << (2+numBits_L2_ECMP_Start_Index)) | /*<Number of L2 ECMP Paths>*/
           (ecmpLttInfoPtr->ecmpStartIndex << 2);/*<L2 ECMP Start Index>*/

    rc = prvCpssDxChWriteTableEntry(devNum,
        CPSS_DXCH_SIP5_TABLE_EQ_TRUNK_LTT_E,
        trunkId,
        &data);

    return rc;

}

/**
* @internal cpssDxChTrunkEcmpLttTableSet function
* @endinternal
*
* @brief   Function Relevant mode : Low Level mode
*         DESCRIPTION:
*         Set Trunk ECMP LTT entry info.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] trunkId                  - the  (LTT index)
*                                      (APPLICABLE RANGES: 0..4095)
* @param[in] ecmpLttInfoPtr           - (pointer to) ECMP LTT Information
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameters
* @retval GT_OUT_OF_RANGE          - on out of range ecmpStartIndex or ecmpNumOfPaths
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTrunkEcmpLttTableSet
(
    IN GT_U8                                devNum,
    IN GT_TRUNK_ID                          trunkId,
    IN CPSS_DXCH_TRUNK_ECMP_LTT_ENTRY_STC  *ecmpLttInfoPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTrunkEcmpLttTableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, trunkId, ecmpLttInfoPtr));

    rc = internal_cpssDxChTrunkEcmpLttTableSet(devNum, trunkId, ecmpLttInfoPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, trunkId, ecmpLttInfoPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTrunkEcmpLttTableGet function
* @endinternal
*
* @brief   Get Trunk ECMP LTT entry info.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] trunkId                  - the  (LTT index)
*                                      (APPLICABLE RANGES: 0..4095)
*
* @param[out] ecmpLttInfoPtr           - (pointer to) ECMP LTT Information
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTrunkEcmpLttTableGet
(
    IN  GT_U8                                devNum,
    IN GT_TRUNK_ID                          trunkId,
    OUT CPSS_DXCH_TRUNK_ECMP_LTT_ENTRY_STC  *ecmpLttInfoPtr
)
{
    GT_STATUS   rc;
    GT_U32  numBits_L2_ECMP_Start_Index;/* number of bits in <L2_ECMP_Start_Index> */
    GT_U32  data;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
         CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);

    CPSS_NULL_PTR_CHECK_MAC(ecmpLttInfoPtr);

    if(trunkId > PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.trunksNum)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
            "trunkId[%d] must NOT be more than [%d] \n",
            trunkId ,
            PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.trunksNum);
    }

    rc = prvCpssDxChReadTableEntry(devNum,
        CPSS_DXCH_SIP5_TABLE_EQ_TRUNK_LTT_E,
        trunkId,
        &data);
    if(rc != GT_OK)
    {
        return rc;
    }

    numBits_L2_ECMP_Start_Index = NUMBITS_L2_ECMP_START_INDEX_MAC(devNum);

    ecmpLttInfoPtr->ecmpEnable = BIT2BOOL_MAC(U32_GET_FIELD_MAC(data,0,1));
    ecmpLttInfoPtr->ecmpRandomPathEnable = BIT2BOOL_MAC(U32_GET_FIELD_MAC(data,1,1)); /*<Random ECMP Path Enable>*/
    ecmpLttInfoPtr->ecmpNumOfPaths = 1 + U32_GET_FIELD_MAC(data,(2+numBits_L2_ECMP_Start_Index), 12);
    ecmpLttInfoPtr->ecmpStartIndex = U32_GET_FIELD_MAC(data,2,numBits_L2_ECMP_Start_Index);

    return GT_OK;
}

/**
* @internal cpssDxChTrunkEcmpLttTableGet function
* @endinternal
*
* @brief   Get Trunk ECMP LTT entry info.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] trunkId                  - the  (LTT index)
*                                      (APPLICABLE RANGES: 0..4095)
*
* @param[out] ecmpLttInfoPtr           - (pointer to) ECMP LTT Information
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTrunkEcmpLttTableGet
(
    IN  GT_U8                                devNum,
    IN GT_TRUNK_ID                          trunkId,
    OUT CPSS_DXCH_TRUNK_ECMP_LTT_ENTRY_STC  *ecmpLttInfoPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTrunkEcmpLttTableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, trunkId, ecmpLttInfoPtr));

    rc = internal_cpssDxChTrunkEcmpLttTableGet(devNum, trunkId, ecmpLttInfoPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, trunkId, ecmpLttInfoPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTrunkDestroy function
* @endinternal
*
* @brief   Function Relevant mode : All modes
*         CPSS DxCh Trunk Destroy all relevant DB info.
*         No HW modifications are done.
*         Purpose :
*         Debug tool to allow to call cpssDxChTrunkInit(...) again with any new valid parameters.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on other error.
* @retval GT_BAD_PARAM             - wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTrunkDestroy
(
    IN  GT_U8       devNum
)
{
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    FREE_PTR_MAC(PRV_CPSS_DXCH_PP_MAC(devNum)->errata.
            info_PRV_CPSS_DXCH_XCAT_TRUNK_WRONG_SOURCE_PORT_TO_CPU_WA_E.
            extraTrunksInfoPtr);

    return prvCpssGenericTrunkDestroy(devNum);
}

/**
* @internal cpssDxChTrunkDestroy function
* @endinternal
*
* @brief   Function Relevant mode : All modes
*         CPSS DxCh Trunk Destroy all relevant DB info.
*         No HW modifications are done.
*         Purpose :
*         Debug tool to allow to call cpssDxChTrunkInit(...) again with any new valid parameters.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on other error.
* @retval GT_BAD_PARAM             - wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTrunkDestroy
(
    IN  GT_U8                                devNum
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTrunkDestroy);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum));

    rc = internal_cpssDxChTrunkDestroy(devNum);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

