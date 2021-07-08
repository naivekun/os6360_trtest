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
* @file prvCpssDxChPcl.h
*
* @brief CPSS DXCH private PCL lib API implementation.
*
* @version   50
********************************************************************************
*/
#ifndef __prvCpssDxChPclh
#define __prvCpssDxChPclh

#include <cpss/dxCh/dxChxGen/pcl/cpssDxChPcl.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwTables.h>
#include <cpss/dxCh/dxChxGen/tcam/private/prvCpssDxChTcam.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* there are 50 UDBs for IPCL/EPCL in SIP5 devices */
#define PRV_CPSS_DXCH_PCL_UDBS_NUM_CNS    50


/* Convert Attribute modify QoS command to hardware value */
#define PRV_CPSS_PCL_CONVERT_ATTR_MODIFY_TO_HW_VAL_MAC(_val, _cmd)          \
    switch (_cmd)                                                       \
    {                                                                   \
        case CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E:              \
            _val = 0;                                                   \
            break;                                                      \
        case CPSS_PACKET_ATTRIBUTE_MODIFY_ENABLE_E:                     \
            _val = 1;                                                   \
            break;                                                      \
        case CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E:                    \
            _val = 2;                                                   \
            break;                                                      \
        default:                                                        \
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);                                    \
    }

/* Convert hardware value to Attribute modify QoS command */
#define PRV_CPSS_PCL_CONVERT_HW_VAL_TO_ATTR_MODIFY_MAC(_cmd, _val)          \
    switch (_val)                                                       \
    {                                                                   \
        case 0:                                                         \
            _cmd = CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E;        \
            break;                                                      \
        case 1:                                                         \
            _cmd = CPSS_PACKET_ATTRIBUTE_MODIFY_ENABLE_E;               \
            break;                                                      \
        case 2:                                                         \
            _cmd = CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;              \
            break;                                                      \
        default:                                                        \
            _cmd =  (_val);                                             \
    }


/* check that the devFamily is one of DxCh xCat and above */
/*                      - else return GT_BAD_PARAM        */
#define PRV_CPSS_DXCH_PCL_XCAT_DEV_CHECK_MAC(devNum)                 \
    if (! PRV_CPSS_DXCH_XCAT_FAMILY_CHECK_MAC(devNum))               \
    {                                                                \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);                                         \
    }



/* check rule size and index macro */
#define PRV_CPSS_DXCH_PCL_RULE_SIZE_INDEX_CHECK_MAC(_devNum, _ruleSize, _ruleIndex)  \
    {                                                                                \
        GT_STATUS _rc;                                                               \
        GT_U32    _absIndex;                                                         \
                                                                                     \
        _rc = prvCpssDxChPclTcamRuleAbsoluteIndexGet(                                \
            _devNum, _ruleSize, _ruleIndex, 0/*ruleSegment*/, &_absIndex);           \
        if (_rc != GT_OK)                                                            \
        {                                                                            \
            return _rc;                                                              \
        }                                                                            \
    }

/* Check PCL direction and return GT_BAD_PARAM if wrong */
#define PRV_CPSS_DXCH_PCL_DIRECTION_CHECK_MAC(_dir) \
    switch(_dir)                                    \
    {                                               \
        case CPSS_PCL_DIRECTION_INGRESS_E:          \
        case CPSS_PCL_DIRECTION_EGRESS_E:           \
            break;                                  \
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);               \
    }

/* get rule size value macro */
#define PRV_CPSS_DXCH_PCL_GET_RULE_SIZE_VALUE_MAC(_ruleSizeType, _ruleSizeVal) \
    switch (_ruleSizeType)                                                     \
    {                                                                          \
        case CPSS_PCL_RULE_SIZE_STD_E: _ruleSizeVal = 1;                       \
            break;                                                             \
        case CPSS_PCL_RULE_SIZE_EXT_E: _ruleSizeVal = 2;                       \
            break;                                                             \
        case CPSS_PCL_RULE_SIZE_ULTRA_E: _ruleSizeVal = 3;                     \
            break;                                                             \
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);                                          \
    }

/* Converts PCL rule format to rule size value */
#define PRV_CPSS_DXCH_PCL_CONVERT_RULE_FORMAT_TO_TCAM_RULE_SIZE_VAL_MAC(_tcamRuleSize, _ruleFormat) \
    switch (_ruleFormat)                                                                            \
    {                                                                                               \
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_10_E:                                            \
        case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_10_E:                                             \
            _tcamRuleSize = CPSS_DXCH_TCAM_RULE_SIZE_10_B_E;                                        \
            break;                                                                                  \
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_20_E:                                            \
        case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_20_E:                                             \
            _tcamRuleSize = CPSS_DXCH_TCAM_RULE_SIZE_20_B_E;                                        \
            break;                                                                                  \
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_40_E:                                            \
        case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_40_E:                                             \
            _tcamRuleSize = CPSS_DXCH_TCAM_RULE_SIZE_40_B_E;                                        \
            break;                                                                                  \
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_50_E:                                            \
        case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_50_E:                                             \
            _tcamRuleSize = CPSS_DXCH_TCAM_RULE_SIZE_50_B_E;                                        \
            break;                                                                                  \
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_NOT_IPV6_E:                                      \
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L2_E:                                       \
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L4_E:                                       \
        case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_NOT_IPV6_E:                                       \
        case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L2_E:                                        \
        case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L4_E:                                        \
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_UDB_E:                                           \
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_IPV4_PORT_VLAN_QOS_E:                            \
        case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV4_RACL_VACL_E:                                 \
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_60_E:                                            \
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_60_NO_FIXED_FIELDS_E:                            \
        case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_60_E:                                             \
            _tcamRuleSize = CPSS_DXCH_TCAM_RULE_SIZE_60_B_E;                                        \
            break;                                                                                  \
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_ULTRA_IPV6_PORT_VLAN_QOS_E:                          \
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_ULTRA_IPV6_ROUTED_ACL_QOS_E:                         \
        case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_ULTRA_IPV6_RACL_VACL_E:                               \
            _tcamRuleSize = CPSS_DXCH_TCAM_RULE_SIZE_80_B_E;                                        \
            break;                                                                                  \
        default:                                                                                    \
            _tcamRuleSize = CPSS_DXCH_TCAM_RULE_SIZE_30_B_E;                                        \
            break;                                                                                  \
    }


/* Converts PCL rule size to rule size value */
#define PRV_CPSS_DXCH_PCL_CONVERT_RULE_SIZE_TO_TCAM_RULE_SIZE_VAL_MAC(_tcamRuleSize, _pclRuleSize)  \
    switch (_pclRuleSize)                                                                           \
    {                                                                                               \
        case CPSS_PCL_RULE_SIZE_STD_E:                                                              \
            _tcamRuleSize = CPSS_DXCH_TCAM_RULE_SIZE_30_B_E;                                        \
            break;                                                                                  \
        case CPSS_PCL_RULE_SIZE_EXT_E:                                                              \
            _tcamRuleSize = CPSS_DXCH_TCAM_RULE_SIZE_60_B_E;                                        \
            break;                                                                                  \
        case CPSS_PCL_RULE_SIZE_ULTRA_E:                                                            \
             _tcamRuleSize = CPSS_DXCH_TCAM_RULE_SIZE_80_B_E;                                       \
            break;                                                                                  \
        case CPSS_PCL_RULE_SIZE_10_BYTES_E:                                                         \
             _tcamRuleSize = CPSS_DXCH_TCAM_RULE_SIZE_10_B_E;                                       \
            break;                                                                                  \
        case CPSS_PCL_RULE_SIZE_20_BYTES_E:                                                         \
             _tcamRuleSize = CPSS_DXCH_TCAM_RULE_SIZE_20_B_E;                                       \
            break;                                                                                  \
        case CPSS_PCL_RULE_SIZE_40_BYTES_E:                                                         \
             _tcamRuleSize = CPSS_DXCH_TCAM_RULE_SIZE_40_B_E;                                       \
            break;                                                                                  \
        case CPSS_PCL_RULE_SIZE_50_BYTES_E:                                                         \
             _tcamRuleSize = CPSS_DXCH_TCAM_RULE_SIZE_50_B_E;                                       \
            break;                                                                                  \
        default:                                                                                    \
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);                                                                    \
            break;                                                                                  \
    }

/* Check PCL lookup number and return GT_BAD_PARAM if wrong */
#define PRV_CPSS_DXCH_PCL_LOOKUP_NUM_CHECK_MAC(_lookup) \
    if(CPSS_PCL_LOOKUP_0_1_E < (_lookup))               \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

/* number of TCAM raws in the DxCh3 devices */
#define PRV_DXCH3_PCL_TCAM_RAW_NUMBER_CNS           3584

/* number of IPCL Configuration Table rows in the Dx devices */
#define PRV_DXCH_IPCL_CONF_TBL_ROW_NUMBER_CNS(dev)   \
        (GT_U32)(                                    \
        PRV_CPSS_SIP_5_20_CHECK_MAC(dev) ? (_4K + 512) : /*4608*/ \
        PRV_CPSS_SIP_5_CHECK_MAC(dev)    ? (_4K + 256) : /*4352*/ \
        (_4K + 128))/*4224*/

/* number of EPCL Configuration Table rows in the Dx devices */
#define PRV_DXCH_EPCL_CONF_TBL_ROW_NUMBER_CNS(dev)    \
        (GT_U32)(                                     \
        PRV_CPSS_SIP_5_20_CHECK_MAC(dev) ? (_4K + 512) : /*4608*/ \
        PRV_CPSS_SIP_5_CHECK_MAC(dev)    ? (_4K + 256) : /*4352*/ \
        (_4K + 64))/*4160*/

/*
 * typedef enum PRV_CPSS_DXCH_PCL_TCAM_OPERATION
 *
 * Description:TCAM operations
 *
 * Enumerations:
 *
 *    PRV_CPSS_DXCH_PCL_TCAM_READ_ACTION         - read action
 *    PRV_CPSS_DXCH_PCL_TCAM_WRITE_ACTION        - write action
 *    PRV_CPSS_DXCH_PCL_TCAM_WR_PATTERN_VALID    -
 *                                 write TCAM pattern and valid
 *    PRV_CPSS_DXCH_PCL_TCAM_WR_PATTERN_INVALID  -
 *                                 write TCAM pattern and invalid
 *    PRV_CPSS_DXCH_PCL_TCAM_WRITE_MASK          - write TCAM mask
 *
 */
typedef enum
{
    PRV_CPSS_DXCH_PCL_TCAM_READ_ACTION,
    PRV_CPSS_DXCH_PCL_TCAM_WRITE_ACTION,
    PRV_CPSS_DXCH_PCL_TCAM_WR_PATTERN_VALID,
    PRV_CPSS_DXCH_PCL_TCAM_WR_PATTERN_INVALID,
    PRV_CPSS_DXCH_PCL_TCAM_WRITE_MASK

} PRV_CPSS_DXCH_PCL_TCAM_OPERATION;

/**
* @struct PRV_CPSS_DXCH_PCL_LOOKUP_CFG_HW_STC
 *
 * @brief PCL Configuration Table Entry Per Lookup Attributes.
 * All values converted to HW format
*/
typedef struct{

    /** Enable Lookup bit */
    GT_U8 enableLookup;

    /** PCL */
    GT_U32 pclId;

    /** @brief For xCat and above devices.
     *  Enables lookup0_0.
     */
    GT_U8 dualLookup;

    /** @brief PCL
     *  For xCat and above devices.
     */
    GT_U32 pclIdL01;

    /** @brief type of TCAM search key for NON
     *  nonIpKeyType is a standard/extended key bit value
     */
    GT_U32 nonIpKeyType;

    /** @brief type of TCAM search key for IPV4 packets
     *  ipv4KeyType is L2QOS/L4 bit
     *  regarless than for exteded key it used only for IPV6
     */
    GT_U32 ipv4KeyType;

    /** @brief type of TCAM search key for IPV6 packets
     *  ipv6KeyType is IPV6-DIP LOOKUP1 value
     *  regarless it used only for standard IPV6 keys
     *  The "entry valid" bit 28 always set to 1 (valid)
     *  The Write function ingnores not relevant fields, The read function
     *  returns zeros in not relevant fields.
     */
    GT_U32 ipv6KeyType;

    /** replace 32 UDBs in the key by 256 */
    GT_U32 udbKeyBitmapEnable;

    /** @brief TCAM Segment Mode (for parallel lookup)
     *  Comments:
     *  For Ch+ devices
     */
    GT_U32 tcamSegmentMode;

    /** @brief Specifies the EM Profile Select number
     *  comments: for sip6
     */
    GT_U32 exactMatchLookupSubProfileId;

} PRV_CPSS_DXCH_PCL_LOOKUP_CFG_HW_STC;

/* amount of fields in Lookup configuration entry         */
/* fields described in PRV_CPSS_DXCH_PCL_LOOKUP_CFG_HW_STC */
#define PRV_CPSS_DXCH_PCL_LOOKUP_CFG_HW_FIELDS_AMOUNT_CNS 10

/* maximal number of Lookup configuration fields */
/* added "1" for DxCh1 "entry valid" bit         */
#define PRV_CPSS_DXCH_PCL_LOOKUP_CFG_HW_FIELDS_MAX_NUM_CNS \
    (PRV_CPSS_DXCH_PCL_LOOKUP_CFG_HW_FIELDS_AMOUNT_CNS + 1)

/**
* @struct PRV_PCL_CFG_LOOKUP_POS_STC
 *
 * @brief PCL Configuration Table Entry Per Lookup bit offsets and lengths.
 * Not relevant data length specified as 0;
*/
typedef struct{

    /** Offset of Enable Lookup bit */
    GT_U8 enableLookupOff;

    /** Length of Enable Lookup bit */
    GT_U8 enableLookupLen;

    /** Offset of Lookp0_0 PCL */
    GT_U8 pclIdOff;

    /** Length of Lookp0_0 PCL */
    GT_U8 pclIdLen;

    /** Offset of type of TCAM search key for NON */
    GT_U8 nonIpKeyTypeOff;

    /** Length of type of TCAM search key for NON */
    GT_U8 nonIpKeyTypeLen;

    /** Offset of type of TCAM search key for IPV4 packets */
    GT_U8 ipv4KeyTypeOff;

    /** Length of type of TCAM search key for IPV4 packets */
    GT_U8 ipv4KeyTypeLen;

    /** Offset of type of TCAM search key for IPV6 packets */
    GT_U8 ipv6KeyTypeOff;

    /** Length of type of TCAM search key for IPV6 packets */
    GT_U8 ipv6KeyTypeLen;

    /** Offset of Enable Lookup0_1 bit */
    GT_U8 dualLookupOff;

    /** Length of Enable Lookup bit */
    GT_U8 dualLookupLen;

    /** Offset of Lookup0_1 PCL */
    GT_U8 pclIdL01Off;

    /** @brief Length of Lookup0_1 PCL
     *  Comments:
     *  dualLookup - only for Ch3 devices IPCL lookup0
     *  For Ch+ devices
     *  nonIpKeyType is a standard/extended key bit value
     *  ipv4KeyType is L2QOS/L4 bit
     *  regarless than for exteded key it used only for IPV6
     *  ipv6KeyType is IPV6-DIP LOOKUP1 value
     *  regarless it used only for standard IPV6 keys
     *  The "entry valid" bit 28 always set to 1 (valid)
     *  The Write function ingnores not relevant fields, The read function
     *  returns zeros in not relevant fields.
     *  Not relevant offset (xxxOff) is 0xFF, not relevant length ( xxxLen) is 0.
     */
    GT_U8 pclIdL01Len;

    GT_U8 udbKeyBitmapEnableOff;

    GT_U8 udbKeyBitmapEnableLen;

    GT_U8 tcamSegmentModeOff;

    GT_U8 tcamSegmentModeLen;
    /** Offset of em Lookup SubProfile Id bit */
    GT_U8 exactMatchLookupSubProfileIdOff;
    /** Length of em Lookup SubProfile Id bit */
    GT_U8 exactMatchLookupSubProfileIdLen;

} PRV_PCL_CFG_LOOKUP_POS_STC;

/**
* @struct PRV_CPSS_DXCH_PCL_RULE_STORE_PORT_GROUP_MODIFIED_FLD_STC
 *
 * @brief Fields being converted due to port group.
*/
typedef struct{

    /** ruleSize in standard rules */
    GT_U32 ruleSize;

    /** GT_TRUE */
    GT_BOOL isEgress;

    /** srcPort field mask. */
    GT_U32 srcPortMask;

    /** srcPort field pattern. */
    GT_U32 srcPortPattern;

    /** portListBmp field mask. */
    GT_U32 portListBmpMask;

    /** @brief portListBmp field pattern.
     *  Comments:
     */
    GT_U32 portListBmpPattern;

} PRV_CPSS_DXCH_PCL_RULE_STORE_PORT_GROUP_MODIFIED_FLD_STC;

/**
* @struct PRV_CPSS_DXCH_PCL_RULE_STC_PORT_GROUP_MODIFIED_PTRS_STC
 *
 * @brief pointers to Sources of fields being converted due to port group.
*/
typedef struct{

    GT_U32 *srcPortPtr;

    CPSS_PORTS_BMP_STC *portListBmpPtr;

} PRV_CPSS_DXCH_PCL_RULE_STC_PORT_GROUP_MODIFIED_PTRS_STC;

/**
* @struct PRV_CPSS_DXCH_PCL_RULE_SOURCE_PORT_GROUP_MODIFIED_FLD_STC
 *
 * @brief Sources of fields being converted due to port group.
*/
typedef struct{

    /** ruleSize in standard rules */
    GT_U32 ruleSize;

    /** GT_TRUE */
    GT_BOOL isEgress;

    /** srcPort field mask. */
    GT_U32 srcPortMask;

    /** @brief srcPort field pattern.
     *  portListBmpMaskPtr  - (pointer to) portListBmp field mask.
     *  portListBmpPatternPtr - (pointer to) portListBmp field pattern.
     *  Comments:
     */
    GT_U32 srcPortPattern;

    CPSS_PORTS_BMP_STC *portListBmpMaskPtr;

    CPSS_PORTS_BMP_STC *portListBmpPatternPtr;

} PRV_CPSS_DXCH_PCL_RULE_SOURCE_PORT_GROUP_MODIFIED_FLD_STC;


/**
* @internal prvCpssDxChLion2PclTcamParityCalc function
* @endinternal
*
* @brief   Function calculates parity values for Policy TCAM standard rules.
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] xDataArr[8]              - X data array.
* @param[in] yDataArr[8]              - Y data array.
*
* @param[out] xParityPtr               - (pointer to) calculated X parity
* @param[out] yParityPtr               - (pointer to) calculated Y parity
*                                       None.
*/
GT_VOID prvCpssDxChLion2PclTcamParityCalc
(
    IN  GT_U32 xDataArr[8],
    IN  GT_U32 yDataArr[8],
    OUT GT_U32 *xParityPtr,
    OUT GT_U32 *yParityPtr
);


/**
* @internal prvCpssDxChPclLookupParamCheck function
* @endinternal
*
* @brief   Checks lookup paraqmeters
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] direction                - Policy Engine direction, Ingress or Egress
* @param[in] lookupNum                - lookup number: 0,1
* @param[in] subLookupNum             - Sub lookup Number - for ingress lookup0
*                                      means 0: lookup0_0, 1: lookup0_1,
*                                      for other cases not relevant.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
GT_STATUS prvCpssDxChPclLookupParamCheck
(
    IN  GT_U8                         devNum,
    IN  CPSS_PCL_DIRECTION_ENT        direction,
    IN  CPSS_PCL_LOOKUP_NUMBER_ENT    lookupNum,
    IN  GT_U32                        subLookupNum
);

/**
* @internal prvCpssDxChPclTcamRuleAbsoluteIndexGet function
* @endinternal
*
* @brief   Gets absolute rule index, i.e. it's action index in action table.
*         It also is an index of the first standard-sized part of the rule.
*         The application rule index is an amount of rules with the same
*         size that can be placed before the given rule
*         In complicated TCAM
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] ruleSize                 - sizeof rule, measured in standard rules
* @param[in] ruleIndex                - index of rule
* @param[in] ruleSegment              - the number of STD-rule sized segment of rule
*
* @param[out] ruleAbsIndexPtr          - (pointer to) rule absolute index
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on failure.
*/
GT_STATUS prvCpssDxChPclTcamRuleAbsoluteIndexGet
(
    IN  GT_U8                             devNum,
    IN  GT_U32                            ruleSize,
    IN  GT_U32                            ruleIndex,
    IN  GT_U32                            ruleSegment,
    OUT GT_U32                            *ruleAbsIndexPtr
);

/**
* @internal prvCpssDxChPclTcamOperationWait function
* @endinternal
*
* @brief   Waits for operation completion (if started)
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
*/
GT_STATUS prvCpssDxChPclTcamOperationWait
(
    IN  GT_U8                         devNum
);

/**
* @internal prvCpssDxChPclTcamOperationStart function
* @endinternal
*
* @brief   Starts Read/Write/Invalidate rule(standard and extended)
*         Mask/Pattern/Action
*         Does exchance between intermediate regsters set and TCAM
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] ruleSize                 - sizeof rule, measured in standard rules
* @param[in] ruleIndex                - index of rule, (APPLICABLE RANGES: 0..1023), extended (APPLICABLE RANGES: 0..511)
* @param[in] waitPrevReady            - GT_TRUE - to wait that previous operation finished
* @param[in] tcamOperation            - read Action, write Mask/Pattern/Action, invalidate
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
*/
GT_STATUS prvCpssDxChPclTcamOperationStart
(
    IN  GT_U8                             devNum,
    IN  GT_U32                            ruleSize,
    IN  GT_U32                            ruleIndex,
    IN  GT_BOOL                           waitPrevReady,
    IN  PRV_CPSS_DXCH_PCL_TCAM_OPERATION  tcamOperation
);

/**
* @internal prvCpssDxChPclTcamOperDataPut function
* @endinternal
*
* @brief   Writes PCE(standard and extended) Mask/Pattern/Action data
*         to addressable regster set for use by TCAM operation
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] ruleSize                 - sizeof rule, measured in standard rules
* @param[in] tcamOperation            - write Mask/Pattern/Action
* @param[in] dataPtr                  - data - 3 words for action,
*                                      6/12 bytes for pattern/mask
*
* @retval GT_OK                    - on success.
* @retval other                    - on failure.
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
*/
GT_STATUS prvCpssDxChPclTcamOperDataPut
(
    IN  GT_U8                             devNum,
    IN  GT_U32                            ruleSize,
    IN  PRV_CPSS_DXCH_PCL_TCAM_OPERATION  tcamOperation,
    IN  GT_U32                            *dataPtr
);

/**
* @internal prvCpssDxChPclTcamActionGet function
* @endinternal
*
* @brief   Reads PCE(standard and extended) Action data
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portGroupId              - The port group Id. relevant only to 'multi-port-groups' devices.
*                                      (APPLICABLE DEVICES Lion; Lion2; Falcon)
*                                      supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
* @param[in] ruleSize                 - sizeof rule, measured in standard rules
* @param[in] ruleIndex                - index of PCE  - owner of action to read
* @param[in] waitPrevReady            - GT_TRUE - to wait that previous operation finished
*                                      relevant only for DxCh1 and DxCh2 indirect read
*                                      ignored for DxCh3 and above devices
*
* @param[out] dataPtr                  - HW data for action
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
*/
GT_STATUS prvCpssDxChPclTcamActionGet
(
    IN  GT_U8                         devNum,
    IN  GT_U32                        portGroupId,
    IN  GT_U32                        ruleSize,
    IN  GT_U32                        ruleIndex,
    IN  GT_BOOL                       waitPrevReady,
    OUT GT_U32                        *dataPtr
);

/**
* @internal prvCpssDxChPclTcamReadStdRuleAddrGet function
* @endinternal
*
* @brief   Calculate standard rule Mask/Pattern direct read address
*         both data and control registers
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] ruleIndex                - index of rule  - owner of action to read
* @param[in] wordIndex                - index of word in the rule
* @param[in] pattern                  - GT_TRUE - pattern, else mask
*
* @param[out] addrPtr                  - address of:  6 short rule words for pattern/mask
*                                      the long rule represented as two shorts
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
*
* @note For Cheetah line consists of 8 registers
*       0,1,2,4,5,6 - 32-bit data,
*       3,7 - control (bit1 - "valid", bit3 - "long")
*       For Cheetah2 line consists of 10 registers
*       called in the document "data0-4" and "control0-4"
*       the control1 and control2 also contain 16 bits of data.
*       The short rule contains 24 8 = 192 bits,
*       "control" means bit19-18 "compMode", bit17 "valid"
*       "compMode" is 1 - short, 2 - long in pattern and 3 in mask
*       The representation in this program is
*       word0 - data-bit31:0
*       word1 - "control"
*       word2 - data-bit63:32
*       word3 - "control" data-bit79:64
*       word4 - data-bit111:80
*       word5 - "control" data-bit127:112
*       word6 - data-bit159:128
*       word7 - "control"
*       word8 - data-bit191:160
*       word9 - "control"
*       For Cheetah3 line consists of 8 registers
*       called in the document "data0-3" and "control0-3
*       the control0-3 also contain 16 bits of data each (bits 15:0).
*       The short rule contains 24 8 = 192 bits,
*       "control" means bit19-18 "compMode", bit17 "valid" , bit16 "spare"
*       "compMode" is 1 - standard, 2 - extended, 3 - ultra
*       The representation in this program is
*       word0 - data-bit31:0
*       word1 - "control" data-bit47:32
*       word2 - data-bit79:48
*       word3 - "control" data-bit95:80
*       word4 - data-bit127:96
*       word5 - "control" data-bit143:128
*       word6 - data-bit175:144
*       word7 - "control" data-bit191:176
*
*/
GT_STATUS prvCpssDxChPclTcamReadStdRuleAddrGet
(
    IN  GT_U8                         devNum,
    IN  GT_U32                        ruleIndex,
    IN  GT_U32                        wordIndex,
    IN  GT_BOOL                       pattern,
    OUT GT_U32                        *addrPtr
);

/**
* @internal prvCpssDxChPclTcamStdRuleDataGet function
* @endinternal
*
* @brief   Reads standard rule Mask/Pattern data
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2.
* @note   NOT APPLICABLE DEVICES:  DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] ruleIndex                - index of rule  - owner of action to read
* @param[in] pattern                  - GT_TRUE - pattern, else mask
*
* @param[out] dataPtr                  - data:  6 words for pattern/mask
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
*
* @note Cheetah and Cheetah2, not relevant for Cheetah3
*
*/
GT_STATUS prvCpssDxChPclTcamStdRuleDataGet
(
    IN  GT_U8                         devNum,
    IN  GT_U32                        ruleIndex,
    IN  GT_BOOL                       pattern,
    OUT GT_U32                        *dataPtr
);

/**
* @internal prvCpssDxChPclTcamRuleDataGet function
* @endinternal
*
* @brief   Reads rule Mask/Pattern data
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portGroupId              - The port group Id. relevant only to 'multi-port-groups' devices.
*                                      (APPLICABLE DEVICES Lion; Lion2; Falcon)
*                                      supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
* @param[in] ruleSize                 - sizeof rule, measured in standard rules
* @param[in] ruleIndex                - index of rule  - owner of action to read
*
* @param[out] maskPtr                  - data:  (ruleSize  6) words for mask
* @param[out] patternPtr               - data:  (ruleSize  6) words for pattern
*                                      for xCat2 and above: (ruleSize  7) words
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
*/
GT_STATUS prvCpssDxChPclTcamRuleDataGet
(
    IN  GT_U8                         devNum,
    IN  GT_U32                        portGroupId,
    IN  GT_U32                        ruleSize,
    IN  GT_U32                        ruleIndex,
    OUT GT_U32                        *maskPtr,
    OUT GT_U32                        *patternPtr
);

/**
* @internal prvCpssDxChPclTcamStdRuleStateGet function
* @endinternal
*
* @brief   Reads standard rule control and converts it to state
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2.
* @note   NOT APPLICABLE DEVICES:  DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] ruleIndex                - index of rule  - owner of action to read
*
* @param[out] validPtr                 -  GT_TRUE - valid
* @param[out] ruleSizePtr              -  sizeof rule, measured in standard rules
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
*
* @note Cheetah and Cheetah2, not relevant for Cheetah3
*
*/
GT_STATUS prvCpssDxChPclTcamStdRuleStateGet
(
    IN  GT_U8                         devNum,
    IN  GT_U32                        ruleIndex,
    OUT GT_BOOL                       *validPtr,
    OUT GT_U32                        *ruleSizePtr
);

/**
* @internal prvCpssDxChPclTcamRuleStateGet function
* @endinternal
*
* @brief   Reads standard rule control and converts it to state
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portGroupId              - The port group Id. relevant only to 'multi-port-groups' devices.
*                                      (APPLICABLE DEVICES Lion; Lion2; Falcon)
*                                      supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
* @param[in] ruleSize                 - sizeof rule, measured in standard rules
* @param[in] ruleIndex                - index of rule  - owner of action to read
*
* @param[out] validPtr                 -  GT_TRUE - valid
* @param[out] ruleFoundSizePtr         -  sizeof rule, measured in standard rules
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
*/
GT_STATUS prvCpssDxChPclTcamRuleStateGet
(
    IN  GT_U8                         devNum,
    IN  GT_U32                        portGroupId,
    IN  GT_U32                        ruleSize,
    IN  GT_U32                        ruleIndex,
    OUT GT_BOOL                       *validPtr,
    OUT GT_U32                        *ruleFoundSizePtr
);

/**
* @internal prvCpssDxChPclTcamRuleWrite function
* @endinternal
*
* @brief   Writes standard or extended rule Action, Mask and Pattern to TCAM
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portGroupId              - The port group Id. relevant only to 'multi-port-groups' devices.
*                                      (APPLICABLE DEVICES Lion; Lion2; Falcon)
*                                      supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
* @param[in] ruleSize                 - sizeof rule, measured in standard rules
* @param[in] ruleIndex                - index of rule  - owner of action to read
* @param[in] validRule                - GT_TRUE - valid, GT_FALSE - invalid
* @param[in] waitPrevReady            - GT_TRUE - to wait that previous operation finished
* @param[in] actionPtr                - action
* @param[in] maskPtr                  - mask
* @param[in] patternPtr               - pattern
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
*
* @note For all devices the mask and pattern of rule
*       is a sequence standard sized segments.
*       For all CH devices, xCat and Lion such segment is
*       192 bits in 6 32-bit words.
*       For xCat2 and above devices such segment is
*       206 bits in 7 32-bit words, padded 18 bits unused.
*
*/
GT_STATUS prvCpssDxChPclTcamRuleWrite
(
    IN  GT_U8                         devNum,
    IN  GT_U32                        portGroupId,
    IN  GT_U32                        ruleSize,
    IN  GT_U32                        ruleIndex,
    IN  GT_BOOL                       validRule,
    IN  GT_BOOL                       waitPrevReady,
    IN  GT_U32                        *actionPtr,
    IN  GT_U32                        *maskPtr,
    IN  GT_U32                        *patternPtr
);

/**
* @internal prvCpssDxChPclTcamRuleActionUpdate function
* @endinternal
*
* @brief   Writes standard or extended rule Action to TCAM
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portGroupId              - The port group Id. relevant only to 'multi-port-groups' devices.
*                                      (APPLICABLE DEVICES Lion; Lion2; Falcon)
*                                      supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
* @param[in] ruleSize                 - sizeof rule, measured in standard rules
* @param[in] ruleIndex                - index of rule  - owner of action to read
* @param[in] actionPtr                - action
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
*/
GT_STATUS prvCpssDxChPclTcamRuleActionUpdate
(
    IN  GT_U8                         devNum,
    IN  GT_U32                        portGroupId,
    IN  GT_U32                        ruleSize,
    IN  GT_U32                        ruleIndex,
    IN  GT_U32                        *actionPtr
);

/**
* @internal prvCpssDxChPclRuleCopy function
* @endinternal
*
* @brief   The function dedicated for one of port groups of multi port group device or
*         for not multi port group device.
*         The function copies the Rule's mask, pattern and action to new TCAM position.
*         The source Rule is not invalidated by the function. To implement move Policy
*         Rule from old position to new one at first cpssDxChPclRuleCopy should be
*         called. And after this cpssDxChPclRuleInvalidate should be used to invalidate
*         Rule in old position
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portGroupId              - The port group Id. relevant only to 'multi-port-groups' devices.
*                                      (APPLICABLE DEVICES Lion; Lion2; Falcon)
*                                      supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
* @param[in] ruleSizeVal              - size of Rule in STD rules.
* @param[in] ruleSrcIndex             - index of the rule in the TCAM from which pattern,
*                                      mask and action are taken.
* @param[in] ruleDstIndex             - index of the rule in the TCAM to which pattern,
*                                      mask and action are placed
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPclRuleCopy
(
    IN GT_U8                              devNum,
    IN GT_U32                             portGroupId,
    IN GT_U32                             ruleSizeVal,
    IN GT_U32                             ruleSrcIndex,
    IN GT_U32                             ruleDstIndex
);

/**
* @internal prvCpssDxChPclSip5RuleCopy function
* @endinternal
*
* @brief   This function copies the Rule's mask, pattern and action to new TCAM position.
*         The source Rule is not invalidated by the function. To implement move TCAM
*         Rule from old position to new one at first prvCpssDxChTcamRuleCopy
*         should be called. And after this cpssDxChTcamGortGroupRuleValidStatusSet should
*         be used to invalidate rule in old position.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupId              - The port group Id. relevant only to 'multi-port-groups' devices.
*                                      (APPLICABLE DEVICES None)
*                                      supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
* @param[in] ruleSize                 - size of rule.
* @param[in] ruleSrcIndex             - index of the rule in the TCAM from which pattern,
*                                      mask and action are taken.
* @param[in] ruleDstIndex             - index of the rule in the TCAM to which pattern,
*                                      mask and action are placed
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter, and when the rule cannot start in a given index.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
*/
GT_STATUS prvCpssDxChPclSip5RuleCopy
(
    IN  GT_U8                         devNum,
    IN  GT_U32                        portGroupId,
    IN  CPSS_DXCH_TCAM_RULE_SIZE_ENT  ruleSize,
    IN  GT_U32                        ruleSrcIndex,
    IN  GT_U32                        ruleDstIndex
);

/**
* @internal prvCpssDxChPclTcamRuleActionSw2HwConvert function
* @endinternal
*
* @brief   Converts TCAM Action from SW to HW format
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] swActionPtr              - action in SW format
*
* @param[out] hwActionPtr              - action in HW format (4 words)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong parameters.
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range.
*/
GT_STATUS prvCpssDxChPclTcamRuleActionSw2HwConvert
(
    IN  GT_U8                         devNum,
    IN  CPSS_DXCH_PCL_ACTION_STC      *swActionPtr,
    OUT GT_U32                        *hwActionPtr
);

/**
* @internal prvCpssDxChPclTcamRuleActionHw2SwConvert function
* @endinternal
*
* @brief   Converts TCAM Action from HW to SW format
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] direction                - Policy direction:
*                                      Ingress or Egress
*                                      Needed for parsing
* @param[in] hwActionPtr              - action in HW format (4 words)
*
* @param[out] swActionPtr              - action in SW format
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong parameters.
* @retval GT_BAD_STATE             - one of the values not supported.
*/
GT_STATUS prvCpssDxChPclTcamRuleActionHw2SwConvert
(
    IN  GT_U8                         devNum,
    IN CPSS_PCL_DIRECTION_ENT         direction,
    IN  GT_U32                        *hwActionPtr,
    OUT CPSS_DXCH_PCL_ACTION_STC      *swActionPtr
);

/**
* @internal prvCpssDxChPclTcamRuleDataSw2HwConvert function
* @endinternal
*
* @brief   Converts TCAM pattern or mask from SW to HW format
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] ruleFormat               - one of 13 supported key formats
* @param[in] swDataPtr                - rule pattern or mask in SW format
*
* @param[out] hwDataPtr                - rule pattern or mask in HW format (6 or 12 words)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong parameters.
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range.
*/
GT_STATUS prvCpssDxChPclTcamRuleDataSw2HwConvert
(
    IN  GT_U8                              devNum,
    IN  CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT ruleFormat,
    IN  CPSS_DXCH_PCL_RULE_FORMAT_UNT      *swDataPtr,
    OUT GT_U32                             *hwDataPtr
);

/**
* @internal prvCpssDxChPclTcamRuleDataHw2SwConvert function
* @endinternal
*
* @brief   Converts TCAM pattern or mask from HW to SW format
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number
* @param[in] ruleFormat               - format of the Rule.
* @param[in] hwDataPtr                - rule pattern or mask in HW format.
*
* @param[out] swDataPtr                - rule pattern or mask in SW format
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong parameters.
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range.
*/
GT_STATUS prvCpssDxChPclTcamRuleDataHw2SwConvert
(
    IN  GT_U8                              devNum,
    IN  CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT ruleFormat,
    IN  GT_U32                             *hwDataPtr,
    OUT CPSS_DXCH_PCL_RULE_FORMAT_UNT      *swDataPtr
);

/**
* @internal prvCpssDxChPclCfgTblEntryIndexGet function
* @endinternal
*
* @brief   Calculates the index of PCL Configuration table entry's for given interface.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] interfaceInfoPtr         - interface data: either port or VLAN
* @param[in] direction                - Policy direction:
*                                      Ingress or Egress
*
* @param[out] indexPtr                 - (pointer to) calculated index
* @param[out] portGroupIdPtr           - (pointer to) the port group Id that the index refers to.
*                                      needed to support multi-port-groups device.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_HW_ERROR              - on HW error
*/
GT_STATUS prvCpssDxChPclCfgTblEntryIndexGet
(
    IN  GT_U8                           devNum,
    IN  CPSS_INTERFACE_INFO_STC         *interfaceInfoPtr,
    IN  CPSS_PCL_DIRECTION_ENT          direction,
    OUT GT_U32                          *indexPtr,
    OUT GT_U32                          *portGroupIdPtr
);

/**
* @internal prvCpssDxChPclCfgTblHwSelectTblAndPos function
* @endinternal
*
* @brief   Selects the PCL Configuration table HW id and the field positions descriptor.
*         The position descriptor is a SW structure that contains offsets lengts
*         of the relevasnt subfields of the table entry
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] direction                - Policy direction:
*                                      Ingress or Egress
* @param[in] lookupNum                - Lookup number:
*                                      lookup0 or lookup1
* @param[in] subLookupNum             - Sub lookup Number (APPLICABLE DEVICES xCat2)
*
* @param[out] tableIdPtr               - (pointer to) HW table Id
* @param[out] posPtrPtr                - (pointer to) (pointer to) position descriptor
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
*/
GT_STATUS prvCpssDxChPclCfgTblHwSelectTblAndPos
(
    IN  GT_U8                                  devNum,
    IN  CPSS_PCL_DIRECTION_ENT                 direction,
    IN  CPSS_PCL_LOOKUP_NUMBER_ENT             lookupNum,
    IN  GT_U32                                 subLookupNum,
    OUT CPSS_DXCH_TABLE_ENT                *tableIdPtr,
    OUT PRV_PCL_CFG_LOOKUP_POS_STC             **posPtrPtr
);

/**
* @internal prvCpssDxChPclCfgTblHwWrite function
* @endinternal
*
* @brief   Write PCL Configuration table entry (fields in HW format)
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portGroupId              - port group Id , to support multi-port-groups device
* @param[in] direction                - Policy direction:
*                                      Ingress or Egress
* @param[in] entryIndex               - entry Index
* @param[in] lookupNum                - Lookup number:
*                                      lookup0 or lookup1
* @param[in] subLookupNum             - Sub lookup Number (APPLICABLE DEVICES xCat2)
* @param[in] lookupCfgPtr             - HW lookup configuration
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
*/
GT_STATUS prvCpssDxChPclCfgTblHwWrite
(
    IN GT_U8                                  devNum,
    IN GT_U32                                 portGroupId,
    IN CPSS_PCL_DIRECTION_ENT                 direction,
    IN GT_U32                                 entryIndex,
    IN CPSS_PCL_LOOKUP_NUMBER_ENT             lookupNum,
    IN  GT_U32                                subLookupNum,
    IN PRV_CPSS_DXCH_PCL_LOOKUP_CFG_HW_STC    *lookupCfgPtr
);

/**
* @internal prvCpssDxChPclCfgTblHwRead function
* @endinternal
*
* @brief   Read PCL Configuration table entry (fields in HW format)
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portGroupId              - port group Id , to support multi-port-groups device
* @param[in] direction                - Policy direction:
*                                      Ingress or Egress
* @param[in] entryIndex               - entry Index
* @param[in] lookupNum                - Lookup number:
*                                      lookup0 or lookup1
* @param[in] subLookupNum             - Sub lookup Number (APPLICABLE DEVICES xCat2)
*
* @param[out] lookupCfgPtr             - HW lookup configuration
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
*/
GT_STATUS prvCpssDxChPclCfgTblHwRead
(
    IN  GT_U8                                  devNum,
    IN  GT_U32                                 portGroupId,
    IN  CPSS_PCL_DIRECTION_ENT                 direction,
    IN  GT_U32                                 entryIndex,
    IN  CPSS_PCL_LOOKUP_NUMBER_ENT             lookupNum,
    IN  GT_U32                                 subLookupNum,
    OUT PRV_CPSS_DXCH_PCL_LOOKUP_CFG_HW_STC    *lookupCfgPtr
);

/* CH3 specific utils */

/* DXCH3 devices keeps X/Y tcam data format                     */
/* (0,0) means "match any value", (1,1) - "match nothing"       */
/* (0,1) - "match 0", (1,0) - "match 1"                         */
/* such conversions will be provided for one-to-one mapping     */

/* mask | pattern | X | Y |  comment            */
/*  0   | 0       | 0 | 0 |  "match any value"  */
/*  0   | 1       | 1 | 1 |  "match nothing"    */
/*  1   | 0       | 0 | 1 |  "match 0"          */
/*  1   | 1       | 1 | 0 | " match 1"          */


/* mask and pattern -> X */
#define PRV_CPSS_DXCH_PCL_MP_TO_X_MAC(_m, _p) (_p)

/* mask and pattern -> Y */
#define PRV_CPSS_DXCH_PCL_MP_TO_Y_MAC(_m, _p) (_m ^ _p)

/* XY-> mask */
#define PRV_CPSS_DXCH_PCL_XY_TO_M_MAC(_x, _y) (_x ^ _y)

/* XY-> pattern */
#define PRV_CPSS_DXCH_PCL_XY_TO_P_MAC(_x, _y) (_x)

/**
* @internal prvCpssDxChPclTcamStdRuleDataXandYGet function
* @endinternal
*
* @brief   Reads standard rule X and Y data
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - device number
* @param[in] portGroupId              - The port group Id. relevant only to 'multi-port-groups' devices.
*                                      (APPLICABLE DEVICES Lion; Lion2; Falcon)
*                                      supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
* @param[in] ruleIndex                - index of rule
*
* @param[out] xPtr                     - data:  6 words for X
* @param[out] yPtr                     - data:  6 words for Y
*                                      For xCat2 and above devices - 7 words.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
*/
GT_STATUS prvCpssDxChPclTcamStdRuleDataXandYGet
(
    IN  GT_U8                         devNum,
    IN  GT_U32                        portGroupId,
    IN  GT_U32                        ruleIndex,
    OUT GT_U32                        *xPtr,
    OUT GT_U32                        *yPtr
);

/**
* @internal prvCpssDxChPclTcamStdRuleStateXandYGet function
* @endinternal
*
* @brief   Reads standard rule X and Y control bits of the 0-th 48-bit column of 4
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] portGroupId              - The port group Id. relevant only to 'multi-port-groups' devices.
*                                      (APPLICABLE DEVICES Lion)
*                                      supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
* @param[in] ruleIndex                - index of rule
*
* @param[out] xValidPtr                -  xValidPtr
* @param[out] yValidPtr                -  yValidPtr
* @param[out] xSizePtr                 -  xSizePtr
* @param[out] ySizePtr                 -  ySizePtr
* @param[out] xSparePtr                -  xSparePtr
* @param[out] ySparePtr                -  ySparePtr
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
*/
GT_STATUS prvCpssDxChPclTcamStdRuleStateXandYGet
(
    IN  GT_U8                         devNum,
    IN  GT_U32                        portGroupId,
    IN  GT_U32                        ruleIndex,
    OUT GT_U32                        *xValidPtr,
    OUT GT_U32                        *yValidPtr,
    OUT GT_U32                        *xSizePtr,
    OUT GT_U32                        *ySizePtr,
    OUT GT_U32                        *xSparePtr,
    OUT GT_U32                        *ySparePtr
);

/**
* @internal prvCpssDxChPclTcamStdRuleValidStateXandYSet function
* @endinternal
*
* @brief   Writes standard rule X and Y valid bits of the 0-th 48-bit column of 4
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] portGroupId              - The port group Id. relevant only to 'multi-port-groups' devices.
*                                      (APPLICABLE DEVICES Lion)
*                                      supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
* @param[in] ruleIndex                - index of rule
* @param[in] xValid                   -  xValid
* @param[in] yValid                   -  yValid
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
*/
GT_STATUS prvCpssDxChPclTcamStdRuleValidStateXandYSet
(
    IN  GT_U8                         devNum,
    IN  GT_U32                        portGroupId,
    IN  GT_U32                        ruleIndex,
    IN  GT_U32                        xValid,
    IN  GT_U32                        yValid
);

/**
* @internal prvCpssDxChPclTcamStdRuleValidStateSet function
* @endinternal
*
* @brief   Writes standard rule X and Y valid bits
*         The algorithms for xCat2 and above and
*         the algorithm for DxCh3, xCat, xCat3, Lion are different.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - device number
* @param[in] portGroupId              - The port group Id. relevant only to 'multi-port-groups' devices.
*                                      (APPLICABLE DEVICES Lion; Lion2; Falcon)
*                                      supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
* @param[in] ruleIndex                - index of rule
* @param[in] valid                    -  valid
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
*/
GT_STATUS prvCpssDxChPclTcamStdRuleValidStateSet
(
    IN  GT_U8                         devNum,
    IN  GT_U32                        portGroupId,
    IN  GT_U32                        ruleIndex,
    IN  GT_BOOL                       valid
);

/**
* @internal prvCpssDxChPclTcamStdRuleXandYSet function
* @endinternal
*
* @brief   Writes standard rule X and Y data and control bits
*         The first 48-bit column control bits supposed to be invalid and will
*         be overwritten last
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] portGroupId              - The port group Id. relevant only to 'multi-port-groups' devices.
*                                      (APPLICABLE DEVICES Lion)
*                                      supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
* @param[in] ruleIndex                - index of rule
* @param[in] xPtr                     - data:  6 words for X
* @param[in] yPtr                     - data:  6 words for Y
* @param[in] xValid                   -  bit of the word0
* @param[in] yValid                   -  bit of the word0
* @param[in] xValid                   -   bit of the words 1-5
* @param[in] yValid                   -   bit of the words 1-5
* @param[in] xSize                    - xSize
* @param[in] ySize                    - ySize
* @param[in] xSpare                   - xSpare
* @param[in] ySpare                   - ySpare
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
*/
GT_STATUS prvCpssDxChPclTcamStdRuleXandYSet
(
    IN  GT_U8                         devNum,
    IN  GT_U32                        portGroupId,
    IN  GT_U32                        ruleIndex,
    IN  GT_U32                        *xPtr,
    IN  GT_U32                        *yPtr,
    IN  GT_U32                        xValid0,
    IN  GT_U32                        yValid0,
    IN  GT_U32                        xValid,
    IN  GT_U32                        yValid,
    IN  GT_U32                        xSize,
    IN  GT_U32                        ySize,
    IN  GT_U32                        xSpare,
    IN  GT_U32                        ySpare
);

/**
* @internal prvCpssDxChPclTcamStdRuleSizeAndValidSet function
* @endinternal
*
* @brief   Writes standard rule X and Y data and control bits
*         The previous TCAM rule state supposed to be invalid and will
*         be overwritten last.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - device number
* @param[in] portGroupId              - The port group Id. relevant only to 'multi-port-groups' devices.
*                                      (APPLICABLE DEVICES Lion; Lion2; Falcon)
*                                      supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
* @param[in] ruleIndex                - index of rule
* @param[in] xPtr                     - data:  6 words for X
* @param[in] yPtr                     - data:  6 words for Y
* @param[in] xPtr                     and yPtr for xCat2 and above devices are 7 words
*                                      The contain 206-bit data starting with pclId and
*                                      not contain the 2-bit rule size.
* @param[in] validRule                - valid
* @param[in] ruleSize                 - rule Size (APPLICABLE RANGES: 1..3)
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
*/
GT_STATUS prvCpssDxChPclTcamStdRuleSizeAndValidSet
(
    IN  GT_U8                         devNum,
    IN  GT_U32                        portGroupId,
    IN  GT_U32                        ruleIndex,
    IN  GT_U32                        *xPtr,
    IN  GT_U32                        *yPtr,
    IN  GT_BOOL                       validRule,
    IN  GT_U32                        ruleSize
);

/**
* @internal prvCpssDxChPclRuleDataTo7WordsAdjust function
* @endinternal
*
* @brief   The function adjusts the data converted from SW to HW format
*         to 7 words per standard rule format - devices that use only 2 first
*         bits as control bits (from 52 4 bits of standard rule).
*         1) Drugging high rule segments by such way that each 6-word segment
*         will be padded by the 7-th word contains zero.
*         2) For egress rules the first segment of the rule left shifted to 1
*         because the "bit0 == valid" not exists for xCat2 and above devices.
*
* @note   APPLICABLE DEVICES:      xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion.
*
* @param[in] isEgress                 - GT_TRUE egress format, GT_FALSE ingress format.
* @param[in] ruleSize                 - amount of Standard segments of rule.
* @param[in,out] hwMaskArr[]              - array with  pattern converted to HW format
* @param[in,out] hwPatternArr[]           - array with pattern converted to HW format
* @param[in,out] hwMaskArr[]              - array with  pattern converted to HW format
* @param[in,out] hwPatternArr[]           - array with pattern converted to HW format
*                                       None.
*
* @note The xCat2 specific "key size" bits are added at low level write
*       function prvCpssDxChPclTcamStdRuleSizeAndValidSet.
*
*/
GT_VOID prvCpssDxChPclRuleDataTo7WordsAdjust
(
    IN     GT_BOOL                                  isEgress,
    IN     GT_U32                                   ruleSize,
    INOUT  GT_U32                                   hwMaskArr[],
    INOUT  GT_U32                                   hwPatternArr[]
);

/**
* @internal prvCpssDxChPclRuleGetDirAndSize function
* @endinternal
*
* @brief   The function Gets Direction and size of rule format
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] ruleFormat               - format of the Rule.
*
* @param[out] isEgressPtr              - (pointer to) GT_TRUE - egress, GT_FALSE - ingress
* @param[out] ruleSizePtr              - (pointer to) size of the rule in standard rules
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
GT_STATUS prvCpssDxChPclRuleGetDirAndSize
(
    IN   CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT ruleFormat,
    OUT  GT_BOOL                            *isEgressPtr,
    OUT  GT_U32                             *ruleSizePtr
);

/**
* @internal prvCpssDxChPclRuleGetModifiedFieldsSourcesPointers function
* @endinternal
*
* @brief   The function Gets pointers of source fields modified for port group id.
*
* @note   APPLICABLE DEVICES:      Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3.
*
* @param[in] ruleFormat               - format of the Rule.
* @param[in] rulePtr                  - (pointer to) rule.
*
* @param[out] fldSrcPtr                - (pointer to) structure to get pointers into rulePtr .
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
GT_STATUS prvCpssDxChPclRuleGetModifiedFieldsSourcesPointers
(
    IN   CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT                      ruleFormat,
    IN   CPSS_DXCH_PCL_RULE_FORMAT_UNT                           *rulePtr,
    OUT  PRV_CPSS_DXCH_PCL_RULE_STC_PORT_GROUP_MODIFIED_PTRS_STC *fldSrcPtr
);

/**
* @internal prvCpssDxChPclRuleGetModifiedFieldsSources function
* @endinternal
*
* @brief   The function Gets source of fields modified for port group id.
*         and the port list bmp needed from xCat2.
*
* @note   APPLICABLE DEVICES:      Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3.
*
* @param[in] ruleFormat               - format of the Rule.
* @param[in] maskPtr                  - (pointer to) rule mask
* @param[in] patternPtr               - (pointer to) rule pattren
*
* @param[out] fldSrcPtr                - (pointer to) structure to get results in it.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*
* @note The function used also for xCat2 becase the port list bitmap
*       treated only via this function.
*
*/
GT_STATUS prvCpssDxChPclRuleGetModifiedFieldsSources
(
    IN   CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT                        ruleFormat,
    IN   CPSS_DXCH_PCL_RULE_FORMAT_UNT                             *maskPtr,
    IN   CPSS_DXCH_PCL_RULE_FORMAT_UNT                             *patternPtr,
    OUT  PRV_CPSS_DXCH_PCL_RULE_SOURCE_PORT_GROUP_MODIFIED_FLD_STC *fldSrcPtr
);

/**
* @internal prvCpssDxChPclRuleDataConvertForPortGroup function
* @endinternal
*
* @brief   The function converts source port and port bitmap fields values
*         according to port-group Id.
*
* @note   APPLICABLE DEVICES:      Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3.
*
* @param[in] devNum                   - device number
* @param[in] portGroupId              - port Groups Id for multi port group devices
* @param[in] fldSrcPtr                - source values of the fields
*
* @param[out] storeFldPtr              - converted values of the fields
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
GT_STATUS prvCpssDxChPclRuleDataConvertForPortGroup
(
    IN   GT_U8                                                     devNum,
    IN   GT_U32                                                    portGroupId,
    IN   PRV_CPSS_DXCH_PCL_RULE_SOURCE_PORT_GROUP_MODIFIED_FLD_STC *fldSrcPtr,
    OUT  PRV_CPSS_DXCH_PCL_RULE_STORE_PORT_GROUP_MODIFIED_FLD_STC  *storeFldPtr
);

/**
* @internal prvCpssDxChPclRuleDataStorePortGroupModifiedFields function
* @endinternal
*
* @brief   The function stores such fields that values of them
*         depend of port-group owning the TCAM
*         For Bobcat2; Caelum; Bobcat3; Aldrin they are source port and port list bitmap.
*         The field value calculation done out of this function.
*         This function just adds the values to rule in HW format.
*
* @note   APPLICABLE DEVICES:      Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3.
*
* @param[in] devNum                   - device number
* @param[in] addedFldPtr              - fields with port-group depended values
* @param[in,out] hwMaskArr[]              - array with mask converted to HW format
* @param[in,out] hwPatternArr[]           - array with pattern converted to HW format
* @param[in,out] hwMaskArr[]              - array with mask adjusted for xCat2
* @param[in,out] hwPatternArr[]           - array with pattern adjusted for xCat2
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong addedFldPtr data
*/
GT_STATUS prvCpssDxChPclRuleDataStorePortGroupModifiedFields
(
    IN     GT_U8                                                    devNum,
    IN     PRV_CPSS_DXCH_PCL_RULE_STORE_PORT_GROUP_MODIFIED_FLD_STC *addedFldPtr,
    INOUT  GT_U32                                                   hwMaskArr[],
    INOUT  GT_U32                                                   hwPatternArr[]
);

/**
* @internal prvCpssDxChPclRuleDataLoadPortGroupModifiedFields function
* @endinternal
*
* @brief   The function loads such fields that values of them
*         depend of port-group owning the TCAM
*         For Bobcat2; Caelum; Bobcat3; Aldrin they are source port and port list bitmap.
*         This function just loads the values from rule in HW format.
*
* @note   APPLICABLE DEVICES:      Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3.
*
* @param[in] devNum                   - device number
* @param[in] hwMaskArr[]              - array with mask converted to HW format
* @param[in] hwPatternArr[]           - array with pattern converted to HW format
* @param[in,out] addedFldPtr              -  isEgress and ruleSize are input fields
* @param[in,out] addedFldPtr              - fields with port-group depended values
*                                       None.
*/
GT_STATUS prvCpssDxChPclRuleDataLoadPortGroupModifiedFields
(
    IN     GT_U8                                                    devNum,
    IN     GT_U32                                                   hwMaskArr[],
    IN     GT_U32                                                   hwPatternArr[],
    INOUT  PRV_CPSS_DXCH_PCL_RULE_STORE_PORT_GROUP_MODIFIED_FLD_STC *addedFldPtr
);

/**
* @internal prvCpssDxChPclRuleDataAccumulateFromPortGroup function
* @endinternal
*
* @brief   The function accumulates source port and port bitmap fields values
*         using values retrieved to port-group Id.
*         Conversion done if needed.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] portGroupId              - port Groups Id for multi port group devices
* @param[in] storeFldPtr              - (pointer to)HW values of the fields
*
* @param[out] maskSrcPtrsPtr           - (pointer to)pointers into the mask rule structure
* @param[out] patternSrcPtrsPtr        - (pointer to)pointers into the pattern rule structure
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
GT_STATUS prvCpssDxChPclRuleDataAccumulateFromPortGroup
(
    IN   GT_U8                                                      devNum,
    IN   GT_U32                                                     portGroupId,
    IN   PRV_CPSS_DXCH_PCL_RULE_STORE_PORT_GROUP_MODIFIED_FLD_STC   *storeFldPtr,
    OUT  PRV_CPSS_DXCH_PCL_RULE_STC_PORT_GROUP_MODIFIED_PTRS_STC    *maskSrcPtrsPtr,
    OUT  PRV_CPSS_DXCH_PCL_RULE_STC_PORT_GROUP_MODIFIED_PTRS_STC    *patternSrcPtrsPtr
);

/**
* @internal prvCpssDxChPclRuleDataSw2HwConvert function
* @endinternal
*
* @brief   The function Converts the Policy Rule Mask, Pattern and Action
*         to HW format
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] ruleFormat               - format of the Rule.
* @param[in] maskPtr                  - rule mask. The rule mask is AND styled one. Mask
*                                      bit's 0 means don't care bit (corresponding bit in
*                                      the pattern is not using in the TCAM lookup).
*                                      Mask bit's 1 means that corresponding bit in the
*                                      pattern is using in the TCAM lookup.
*                                      The format of mask is defined by ruleFormat
* @param[in] patternPtr               - rule pattern.
*                                      The format of pattern is defined by ruleFormat
* @param[in] actionPtr                - Policy rule action that applied on packet if packet's
*                                      search key matched with masked pattern.
*
* @param[out] hwMaskArr[]              - array for mask converted to HW format
* @param[out] hwPatternArr[]           - array for pattern converted to HW format
* @param[out] hwActionArr[]            - array for action converted to HW format
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note NONE
*
*/
GT_STATUS prvCpssDxChPclRuleDataSw2HwConvert
(
    IN   GT_U8                              devNum,
    IN   CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT ruleFormat,
    IN   CPSS_DXCH_PCL_RULE_FORMAT_UNT      *maskPtr,
    IN   CPSS_DXCH_PCL_RULE_FORMAT_UNT      *patternPtr,
    IN   CPSS_DXCH_PCL_ACTION_STC           *actionPtr,
    OUT  GT_U32                             hwMaskArr[],
    OUT  GT_U32                             hwPatternArr[],
    OUT  GT_U32                             hwActionArr[]
);

/**
* @internal prvCpssDxChPclTcamRuleModifiedWrite function
* @endinternal
*
* @brief   Writes rule Action, Mask and Pattern to TCAM.
*         Supports rules of any sizes.
*         Supports SrcPort field modification for multi port group devices
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion; Lion2; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] ruleSize                 - sizeof rule, measured in standard rules
* @param[in] ruleIndex                - index of rule  - owner of action to read
* @param[in] egressRule               - GT_TRUE - egress Rule, GT_FALSE ingress
* @param[in] validRule                - GT_TRUE - valid, GT_FALSE - invalid
* @param[in] waitPrevReady            - GT_TRUE - to wait that previous operation finished
*                                      relevant only for DxCh1 and DxCh2 devices,
*                                      ignored for DxCh3 and above devices
* @param[in] actionPtr                - action
* @param[in] maskPtr                  - mask
* @param[in] patternPtr               - pattern
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
*/
GT_STATUS prvCpssDxChPclTcamRuleModifiedWrite
(
    IN  GT_U8                         devNum,
    IN  GT_PORT_GROUPS_BMP            portGroupsBmp,
    IN  GT_U32                        ruleSize,
    IN  GT_U32                        ruleIndex,
    IN  GT_BOOL                       egressRule,
    IN  GT_BOOL                       validRule,
    IN  GT_BOOL                       waitPrevReady,
    IN  GT_U32                        *actionPtr,
    IN  GT_U32                        *maskPtr,
    IN  GT_U32                        *patternPtr
);

/**
* @internal prvCpssDxChPclActionSizeAndAllignmentGet function
* @endinternal
*
* @brief   Gets action size and alignment.
*         extern for parsing in log
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] sizePtr                  - (pointer to) amount words in HW action
* @param[out] alignmentPtr             - (pointer to) alignment of HW action in words
*                                       None.
*/
extern GT_VOID prvCpssDxChPclActionSizeAndAllignmentGet
(
    IN  GT_U8                         devNum,
    OUT GT_U32                        *sizePtr,
    OUT GT_U32                        *alignmentPtr
);

/* size of HW <==> SW Key TYpe conversion tables */
#define PRV_CPSS_DXCH_PCL_CFG_MAX_HW_KEY_TYPES_CNS  8

/* Packet type Ehernet Other */
#define PRV_CPSS_DXCH_PCL_CFG_PKT_TYPE_NOT_IP_CNS   0

/* Packet type IPV4 */
#define PRV_CPSS_DXCH_PCL_CFG_PKT_TYPE_IPV4_CNS     1

/* Packet type IPV6 */
#define PRV_CPSS_DXCH_PCL_CFG_PKT_TYPE_IPV6_CNS     2

/* reserved HW value stamp  */
#define PRV_CPSS_DXCH_PCL_CFG_DUMMY_SW_KEY_TYPE_CNS 0xFF

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssDxChPclh */

