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
* @file cpssDxChBrgFdbHash.h
*
* @brief Hash calculate for MAC address table implementation.
*
* @version   8
********************************************************************************
*/
#ifndef __cpssDxChBrgFdbHashh
#define __cpssDxChBrgFdbHashh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpss/generic/bridge/cpssGenBrgFdb.h>

/**
* @enum CPSS_DXCH_BRG_FDB_TBL_SIZE_ENT
 *
 * @brief Enumeration of FDB Table size in entries
*/
typedef enum{

    /** 8K FDB table entries. */
    CPSS_DXCH_BRG_FDB_TBL_SIZE_8K_E = 0,

    /** 16K FDB table entries. */
    CPSS_DXCH_BRG_FDB_TBL_SIZE_16K_E,

    /** 32K FDB table entries. */
    CPSS_DXCH_BRG_FDB_TBL_SIZE_32K_E,

    /** 64K FDB table entries. */
    CPSS_DXCH_BRG_FDB_TBL_SIZE_64K_E,

    /** 128K FDB table entries. */
    CPSS_DXCH_BRG_FDB_TBL_SIZE_128K_E,

    /** 256K FDB table entries. */
    CPSS_DXCH_BRG_FDB_TBL_SIZE_256K_E

} CPSS_DXCH_BRG_FDB_TBL_SIZE_ENT;

/**
* @enum CPSS_DXCH_BRG_FDB_CRC_HASH_UPPER_BITS_MODE_ENT
 *
 * @brief Enumeration how the CRC hash (CPSS_MAC_HASH_FUNC_CRC_E or CPSS_MAC_HASH_FUNC_CRC_MULTI_HASH_E)
 * will use 16 'most upper bits' for lookup key of type 'MAC+FID'
 * (CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E)
 * relevant when using 'Independent VLAN Learning' (CPSS_IVL_E)
*/
typedef enum{

    /** use 16 bits 0. */
    CPSS_DXCH_BRG_FDB_CRC_HASH_UPPER_BITS_MODE_ALL_ZERO_E,

    /** use 16 bits of FID. */
    CPSS_DXCH_BRG_FDB_CRC_HASH_UPPER_BITS_MODE_USE_FID_E,

    /** use 16 LSBits of MAC. */
    CPSS_DXCH_BRG_FDB_CRC_HASH_UPPER_BITS_MODE_USE_MAC_E

} CPSS_DXCH_BRG_FDB_CRC_HASH_UPPER_BITS_MODE_ENT;


/**
* @struct CPSS_DXCH_BRG_FDB_HASH_PARAMS_STC
 *
 * @brief struct contains the parameters for FDB table hash calculation.
*/
typedef struct{

    /** the VLAN lookup mode. */
    CPSS_MAC_VL_ENT vlanMode;

    /** the entries number in the FDB table. */
    CPSS_DXCH_BRG_FDB_TBL_SIZE_ENT size;

    /** @brief indication that the FDB hash uses 16 bits of FID.
     *  GT_TRUE - use 16 bits FID
     *  GT_FALSE - use 12 bits FID
     */
    GT_BOOL fid16BitHashEn;

    /** @brief 16 MSbits mode for of DATA into the hash function
     *  relevant to hashMode = CRC_MULTI_HASH
     */
    CPSS_DXCH_BRG_FDB_CRC_HASH_UPPER_BITS_MODE_ENT crcHashUpperBitsMode;

    /** @brief Use zero for initialization value of CRC function.
     *  Relevant only for CPSS_MAC_HASH_FUNC_CRC_E.
     *  Must be set as GT_TRUE for Bobcat2, Caelum, Bobcat3, Aldrin, AC3X devices.
     *  Must be set as GT_FALSE for other devices.
     */
    GT_BOOL useZeroInitValueForCrcHash;

} CPSS_DXCH_BRG_FDB_HASH_PARAMS_STC;


/**
* @internal cpssDxChBrgFdbCrcHashUpperBitsModeSet function
* @endinternal
*
* @brief   Set mode how the CRC hash (CPSS_MAC_HASH_FUNC_CRC_E or
*         CPSS_MAC_HASH_FUNC_CRC_MULTI_HASH_E)
*         will use 16 'most upper bits' for lookup key of type 'MAC+FID'
*         (CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E)
*         relevant when using 'Independent VLAN Learning' (CPSS_IVL_E)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] mode                     - the upper bits mode
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgFdbCrcHashUpperBitsModeSet
(
    IN  GT_U8           devNum,
    IN  CPSS_DXCH_BRG_FDB_CRC_HASH_UPPER_BITS_MODE_ENT     mode
);

/**
* @internal cpssDxChBrgFdbCrcHashUpperBitsModeGet function
* @endinternal
*
* @brief   Get mode how the CRC hash (CPSS_MAC_HASH_FUNC_CRC_E or
*         CPSS_MAC_HASH_FUNC_CRC_MULTI_HASH_E)
*         will use 16 'most upper bits' for lookup key of type 'MAC+FID'
*         (CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E)
*         relevant when using 'Independent VLAN Learning' (CPSS_IVL_E)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] modePtr                  - (pointer to) the upper bits mode
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgFdbCrcHashUpperBitsModeGet
(
    IN  GT_U8           devNum,
    OUT  CPSS_DXCH_BRG_FDB_CRC_HASH_UPPER_BITS_MODE_ENT     *modePtr
);


/**
* @internal cpssDxChBrgFdbHashCalc function
* @endinternal
*
* @brief   This function calculates the hash index for the FDB table.
*         The FDB table holds 4 types of entries :
*         1. for specific mac address and VLAN id.
*         2. for specific src IP, dst IP and VLAN id.
*         3. for specific mac address, VLAN id and VID1.
*         (APPLICABLE DEVICES Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2)
*         4. for specific src IP, dst IP, VLAN id and VID1.
*         (APPLICABLE DEVICES Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2)
*         for more details see CPSS_MAC_ENTRY_EXT_KEY_STC description.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  Falcon.
* @note   not supported by Falcon because this API relevant to 'xor'/'crc' modes
*         that not supported by the device.
*
* @param[in] devNum                   - device number
* @param[in] macEntryKeyPtr           - pointer to key parameters of the mac entry
*                                      according to the entry type:
*                                      MAC Address entry -> MAC Address + vlan ID.
*                                      IPv4/IPv6 entry -> srcIP + dstIP + vald ID.
*
* @param[out] hashPtr                  - (pointer to) the hash index.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - wrong devNum, bad vlan-Id
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - on not supported hash function for key type
*/
GT_STATUS cpssDxChBrgFdbHashCalc
(
    IN  GT_U8           devNum,
    IN  CPSS_MAC_ENTRY_EXT_KEY_STC   *macEntryKeyPtr,
    OUT GT_U32          *hashPtr
);


/**
* @internal cpssDxChBrgFdbHashByParamsCalc function
* @endinternal
*
* @brief   This function calculates the hash index for the FDB table.
*         The FDB table holds 4 types of entries :
*         1. for specific mac address and VLAN id.
*         2. for specific src IP, dst IP and VLAN id.
*         3. for specific mac address, VLAN id and VID1.
*         (APPLICABLE DEVICES Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
*         4. for specific src IP, dst IP, VLAN id and VID1.
*         (APPLICABLE DEVICES Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
*         for more details see CPSS_MAC_ENTRY_EXT_KEY_STC description.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] hashMode                 - hash mode
* @param[in] hashParamsPtr            - (pointer to) HASH parameters
* @param[in] macEntryKeyPtr           - (pointer to) key parameters of the mac entry
*                                      according to the entry type:
*                                      MAC Address entry -> MAC Address + vlan ID.
*                                      IPv4/IPv6 entry -> srcIP + dstIP + vald ID.
*
* @param[out] hashPtr                  - (pointer to) the hash index.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - bad vlan-Id
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - on not supported hash function for key type
*/
GT_STATUS cpssDxChBrgFdbHashByParamsCalc
(
    IN  CPSS_MAC_HASH_FUNC_MODE_ENT        hashMode,
    IN  CPSS_DXCH_BRG_FDB_HASH_PARAMS_STC *hashParamsPtr,
    IN  CPSS_MAC_ENTRY_EXT_KEY_STC        *macEntryKeyPtr,
    OUT GT_U32                            *hashPtr
);

/**
* @internal cpssDxChBrgFdbHashCrcMultiResultsByParamsCalc function
* @endinternal
*
* @brief   This function calculates the CRC multiple hash results
*         (indexes into the FDB table).
*         NOTE: the function do not access the HW , and do only SW calculations.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] hashParamsPtr            - (pointer to) HASH parameters
* @param[in] entryKeyPtr              - (pointer to) entry key
* @param[in] multiHashStartBankIndex  - the first bank for 'multi hash' result.
*                                      (APPLICABLE RANGES: 0..15)
* @param[in] numOfBanks               - number of banks for 'multi hash' result.
*                                      this value indicates the number of elements that will be
*                                      retrieved by crcMultiHashArr[]
*                                      restriction of (numOfBanks + multiHashStartBankIndex) <= 16 .
*                                      (APPLICABLE RANGES: 1..16)
*
* @param[out] crcMultiHashArr[]        - (array of) 'multi hash' CRC results. index in this array is 'bank Id'
*                                      (index 0 will hold value relate to bank multiHashStartBankIndex).
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - wrong parameters in entryKeyPtr.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgFdbHashCrcMultiResultsByParamsCalc
(
    IN  CPSS_DXCH_BRG_FDB_HASH_PARAMS_STC *hashParamsPtr,
    IN  CPSS_MAC_ENTRY_EXT_KEY_STC        *entryKeyPtr,
    IN  GT_U32                             multiHashStartBankIndex,
    IN  GT_U32                             numOfBanks,
    OUT GT_U32                             crcMultiHashArr[]
);


/**
* @internal cpssDxChBrgFdbHashCrcMultiResultsCalc function
* @endinternal
*
* @brief   This function calculates the CRC multiple hash results
*         (indexes into the FDB table).
*         NOTE: the function do not access the HW , and do only SW calculations.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] entryKeyPtr              - (pointer to) entry key
* @param[in] multiHashStartBankIndex  - the first bank for 'multi hash' result.
*                                      (APPLICABLE RANGES: 0..15)
* @param[in] numOfBanks               - number of banks for 'multi hash' result.
*                                      this value indicates the number of elements that will be
*                                      retrieved by crcMultiHashArr[]
*                                      restriction of (numOfBanks + multiHashStartBankIndex) <= 16 .
*                                      (APPLICABLE RANGES: 1..16)
*
* @param[out] crcMultiHashArr[]        - (array of) 'multi hash' CRC results. index in this array is 'bank Id'
*                                      (index 0 will hold value relate to bank multiHashStartBankIndex).
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - wrong devNum or parameters in entryKeyPtr.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgFdbHashCrcMultiResultsCalc
(
    IN  GT_U8           devNum,
    IN  CPSS_MAC_ENTRY_EXT_KEY_STC   *entryKeyPtr,
    IN  GT_U32       multiHashStartBankIndex,
    IN  GT_U32       numOfBanks,
    OUT GT_U32       crcMultiHashArr[]
);

/**
* @internal cpssDxChBrgFdbHashRequestSend function
* @endinternal
*
* @brief   The function Send Hash request (HR) message to PP to generate all values
*         of hash results that relate to the 'Entry KEY'.
*         The function checks that AU messaging is ready before using it.
*         The function does not check that HR message was processed by PP.
*         For the results Application can call cpssDxChBrgFdbHashResultsGet.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] entryKeyPtr              - (pointer to) entry key
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or entryKeyPtr->entryType
* @retval GT_OUT_OF_RANGE          - one of the parameters of entryKeyPtr is out of range
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_STATE             - the PP is not ready to get a message from CPU.
*                                       (PP still busy with previous message)
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Application should synchronize call of cpssDxChBrgFdbMacEntrySet,
*       cpssDxChBrgFdbQaSend , cpssDxChBrgFdbMacEntryDelete , cpssDxChBrgFdbHashRequestSend
*       , cpssDxChBrgFdbPortGroupMacEntrySet, cpssDxChBrgFdbPortGroupQaSend ,
*       cpssDxChBrgFdbPortGroupMacEntryDelete , cpssDxChBrgFdbMacEntryMove functions.
*       Invocations of these functions should be mutual exclusive because they
*       use same HW resources.
*       Application can check that the HR message processing has completed by
*       cpssDxChBrgFdbFromCpuAuMsgStatusGet or cpssDxChBrgFdbPortGroupFromCpuAuMsgStatusGet.
*
*/
GT_STATUS cpssDxChBrgFdbHashRequestSend
(
    IN  GT_U8                       devNum,
    IN  CPSS_MAC_ENTRY_EXT_KEY_STC   *entryKeyPtr
);

/**
* @internal cpssDxChBrgFdbHashResultsGet function
* @endinternal
*
* @brief   The function return from the HW the result of last performed AU message
*         from the CPU to PP.
*         Meaning the hash results updated after every call to one of the next:
*         cpssDxChBrgFdbQaSend , cpssDxChBrgFdbMacEntryDelete , cpssDxChBrgFdbHashRequestSend
*         , cpssDxChBrgFdbPortGroupMacEntrySet, cpssDxChBrgFdbPortGroupQaSend ,
*         cpssDxChBrgFdbPortGroupMacEntryDelete , cpssDxChBrgFdbMacEntryMove functions.
*         Application can check that any of those message processing has completed by
*         cpssDxChBrgFdbFromCpuAuMsgStatusGet or cpssDxChBrgFdbPortGroupFromCpuAuMsgStatusGet.
*         Retrieving the hash calc are optional per type:
*         1. xorHashPtr = NULL --> no retrieve XOR calc
*         2. crcHashPtr = NULL --> no retrieve CRC calc
*         3. numOfBanks = 0  --> no retrieve of 'multi hash' CRC results
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] multiHashStartBankIndex  - the first bank for 'multi hash' result.
*                                      this parameter relevant only when numOfBanks != 0
*                                      (APPLICABLE RANGES: 0..15)
* @param[in] numOfBanks               - number of banks for 'multi hash' result.
*                                      this value indicates the number of elements that will be
*                                      retrieved by crcMultiHashArr[]
*                                      value 0 means that no need to retrieve any 'multi hash' result.
*                                      restriction of (numOfBanks + multiHashStartBankIndex) <= 16 when numOfBanks !=0.
*                                      (APPLICABLE RANGES: 0..16)
* @param[in,out] xorHashPtr               - (pointer to) to receive the XOR hash result.
*                                      NULL value means that caller not need to retrieve this value.
* @param[in,out] crcHashPtr               - (pointer to) to receive the CRC hash result.
*                                      NULL value means that caller not need to retrieve this value.
*
* @param[out] crcMultiHashArr[]        - (array of) 'multi hash' CRC results. index in this array is 'bank Id'.
*                                      NULL is allowed only when numOfBanks = 0
*                                      (index 0 will hold value relate to bank multiHashStartBankIndex).
* @param[in,out] xorHashPtr               - (pointer to) the XOR hash result.
*                                      ignored when NULL pointer.
* @param[in,out] crcHashPtr               - (pointer to) the CRC hash result.
*                                      ignored when NULL pointer.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or multiHashStartBankIndex or numOfBanks
* @retval GT_OUT_OF_RANGE          - one of the parameters of entryKeyPtr is out of range
* @retval GT_BAD_PTR               - crcMultiHashArr[] is NULL pointer when numOfBanks != 0
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgFdbHashResultsGet
(
    IN  GT_U8        devNum,
    IN  GT_U32       multiHashStartBankIndex,
    IN  GT_U32       numOfBanks,
    OUT GT_U32       crcMultiHashArr[],
    INOUT GT_U32     *xorHashPtr,
    INOUT GT_U32     *crcHashPtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChBrgFdbHashh */


