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
* @file prvCpssDxChBrgFdbHash.h
*
* @brief MAC hash struct implementation.
*
* @version   9
********************************************************************************
*/
#ifndef __prvCpssDxChBrgFdbHashh
#define __prvCpssDxChBrgFdbHashh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpss/generic/bridge/cpssGenBrgFdb.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgFdbHash.h>

/**
* @struct PRV_CPSS_DXCH_MAC_HASH_STC
 *
 * @brief struct contains the hardware parameters for hash mac Address
 * calculation.
*/
typedef struct{

    /** the VLAN lookup mode. */
    CPSS_MAC_VL_ENT vlanMode;

    /** the FDB hash function mode. */
    CPSS_MAC_HASH_FUNC_MODE_ENT hashMode;

    /** the entries number in the hash table. */
    CPSS_DXCH_BRG_FDB_TBL_SIZE_ENT size;

    /** @brief number of banks in the FDB.
     *  ch1,2,3,xcat,xcat2,lion,lion2 - 4 banks. single hash.
     *  Bobcat2,Caelum,Bobcat3,Aldrin - 16 banks. support 2 hash modes:
     *  legacy (crc/xor) - single hash , 'emulation' as 4 banks.
     *  CRC multi hash - 16 hash functions.
     *  this parameter currently used only when 'CRC multi hash'
     */
    GT_U32 numOfBanks;

    /** @brief indication that the FDB hash uses 16 bits of FID.
     *  GT_TRUE - use 16 bits FID
     *  GT_FALSE - use 12 bits FID
     */
    GT_BOOL fid16BitHashEn;

    /** @brief 16 MSbits mode for of DATA into the hash function
     *  relevant to hashMode = CRC_MULTI_HASH
     */
    CPSS_DXCH_BRG_FDB_CRC_HASH_UPPER_BITS_MODE_ENT crcHashUpperBitsMode;

} PRV_CPSS_DXCH_MAC_HASH_STC;

/* macro to check boundary on FDB bank index - return '_retVal' on error */
#define PRV_CPSS_DXCH_FDB_BANK_INDEX_CHECK_MAC(_dev,_bankIndex,_retVal)          \
    if(_bankIndex >= PRV_CPSS_DXCH_PP_MAC(_dev)->bridge.fdbHashParams.numOfBanks)\
    {                                                                            \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);                                                     \
    }

/* macro to check boundary on FDB bank index - return GT_BAD_PARAM on error */
#define PRV_CPSS_DXCH_FDB_BANK_INDEX_BAD_PARAM_CHECK_MAC(_dev,_bankIndex)    \
    PRV_CPSS_DXCH_FDB_BANK_INDEX_CHECK_MAC(_dev,_bankIndex,GT_BAD_PARAM)

/* macro to check boundary on FDB bank index - return GT_OUT_OF_RANGE on error */
#define PRV_CPSS_DXCH_FDB_BANK_INDEX_OUT_OF_RANGE_CHECK_MAC(_dev,_bankIndex)    \
    PRV_CPSS_DXCH_FDB_BANK_INDEX_CHECK_MAC(_dev,_bankIndex,GT_OUT_OF_RANGE)

#define PRV_CPSS_DXCH_FDB_FIELD_NOT_USED_CNS  0xFFFFFFFF
/**
* @struct PRV_CPSS_DXCH_FDB_SPECIAL_MUXED_FIELDS_STC
 *
 * @brief Muxed fields from the FDB that depend on :
 * in SIP5 : maxLengthSrcIdInFdbEn , tag1VidFdbEn
 * in SIP6 : sip6FdbMacEntryMuxingMode or sip6FdbIpmcEntryMuxingMode
 * field with value : PRV_CPSS_DXCH_FDB_FIELD_NOT_USED_CNS , means 'not used'
*/
typedef struct{

    /** source ID */
    GT_U32 srcId;

    /** user defined byte */
    GT_U32 udb;

    /** original VID1 */
    GT_U32 origVid1;

    /** DA access level */
    GT_U32 daAccessLevel;

    /** @brief SA access level
     *  NOTE: in SIP6 this field is NOT muxed (meaning always valid)
     */
    GT_U32 saAccessLevel;

} PRV_CPSS_DXCH_FDB_SPECIAL_MUXED_FIELDS_STC;

/* number of words in the Au (address update) message */
#define CPSS_DXCH_FDB_AU_WORDS_SIZE_CNS   8


/**
* @internal prvCpssDxChBrgFdbAuMsgSpecialMuxedFieldsGet function
* @endinternal
*
* @brief   Get Muxed fields from the (FDB unit) Au Msg format that depend on :
*         1. vid1_assignment_mode
*         2. src_id_length_in_fdb
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] hwDataArr[CPSS_DXCH_FDB_AU_WORDS_SIZE_CNS] - (pointer to) Au Msg format
* @param[in] entryType                - entry type :
*                                      0 -- mac entry
*                                      1 -- ipmcV4 entry
*                                      2 -- ipmcV6 entry
*
* @param[out] specialFieldsPtr         - (pointer to) special muxed fields values
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on bad parameter error
*/
GT_STATUS prvCpssDxChBrgFdbAuMsgSpecialMuxedFieldsGet
(
    IN GT_U8                        devNum,
    IN GT_U32                       hwDataArr[CPSS_DXCH_FDB_AU_WORDS_SIZE_CNS],
    IN GT_U32                       entryType,
    OUT PRV_CPSS_DXCH_FDB_SPECIAL_MUXED_FIELDS_STC *specialFieldsPtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssDxChBrgFdbHashh */

