/*******************************************************************************
*              (c), Copyright 2006, Marvell International Ltd.                 *
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
* @file prvCpssGenDiag.h
*
* @brief Internal CPSS functions.
*
* @version   13
********************************************************************************
*/
#ifndef __prvCpssGenDiagh
#define __prvCpssGenDiagh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/generic/diag/cpssDiag.h>
#include <cpss/extServices/os/gtOs/gtGenTypes.h>
#include <cpss/generic/cpssHwInit/cpssHwInit.h>
#include <cpss/common/diag/private/prvCpssCommonDiag.h>

/* defines the maximum number of words (masks) in one entry */
#define PRV_DIAG_MAX_MASK_NUM_CNS     32

/* defines the maximum size of one entry in bytes */
#define PRV_DIAG_MAX_BURST_SIZE_CNS   128

/*******************************************************************************
* PRV_CPSS_DIAG_HW_PP_RAM_BUF_MEM_WRITE_FUNC
*
* DESCRIPTION:
*       BUFFER_DRAM (buff mem) memory write.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; ExMxPm; ExMx.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum      - The PP device number to write to.
*       portGroupId - the port group Id , to support multi-port-group devices
*                     that need to access specific port group
*       addr        - Address offset to write to.
*       length      - length in WORDS (32 bit) must be in portion of 8.
*       dataPtr     - An array containing the data to be written.
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*       None
*
*******************************************************************************/
typedef GT_STATUS (*PRV_CPSS_DIAG_HW_PP_RAM_BUF_MEM_WRITE_FUNC)
(
    IN GT_U8  devNum,
    IN GT_U32 portGroupId,
    IN GT_U32 addr,
    IN GT_U32 length,
    IN GT_U32_PTR dataPtr
);

/*******************************************************************************
* PRV_CPSS_DIAG_HW_PP_RAM_BUF_MEM_READ_FUNC
*
* DESCRIPTION:
*       Read the memory WORD by WORD.(32 bits).
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; ExMxPm; ExMx.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum   : device number.
*       portGroupId : the port group Id , to support multi-port-group devices
*                     that need to access specific port group
*       addr     : addr to start writing from.
*       length   : length in WORDS (32 bit) must be in portion of 8.
*
* OUTPUTS:
*       dataPtr  : An array containing the read data.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*       None
*
*******************************************************************************/
typedef GT_STATUS (*PRV_CPSS_DIAG_HW_PP_RAM_BUF_MEM_READ_FUNC)
(
    IN GT_U8  devNum,
    IN GT_U32 portGroupId,
    IN GT_U32 addr,
    IN GT_U32 length,
    OUT GT_U32_PTR dataPtr
);

/*******************************************************************************
* PRV_CPSS_DIAG_HW_PP_RAM_MEM_COMPARE_FUNC
*
* DESCRIPTION:
*       Memory Compare function (for TCAM memories).
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; ExMxPm; ExMx.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       writeBufferPtr   : write buffer.
*       readBufferPtr    : read buffer.
*       sizeInBytes      : size of the memory to compare in bytes.
*       maskIndexArrPtr  : the index to start, in the masks array.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_TRUE   - when read and write buffers are equal
*       GT_FALSE  - when read and write buffers are not equal
*
* COMMENTS:
*       None
*
*******************************************************************************/
typedef GT_BOOL (*PRV_CPSS_DIAG_HW_PP_RAM_MEM_COMPARE_FUNC)
(
    IN GT_U32   *writeBufferPtr,
    IN GT_U32   *readBufferPtr,
    IN GT_U32   sizeInBytes,
    IN GT_U32   *maskIndexArrPtr
);

/*******************************************************************************
* PRV_CPSS_DIAG_MEMORY_TEST_FUNC
*
* DESCRIPTION:
*       Performs memory test on a specified memory location and size for a
*       specified memory type.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; ExMxPm; ExMx.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum      - The device number to test
*       memType     - The packet processor memory type to verify.
*       startOffset - The offset address to start the test from.
*       size        - The memory size in byte to test (start from offset).
*       profile     - The test profile.
*
* OUTPUTS:
*       testStatusPtr  - GT_TRUE if the test succeeded or GT_FALSE for failure
*       addrPtr        - Address offset of memory error, if testStatusPtr is
*                        GT_FALSE. Irrelevant if testStatusPtr is GT_TRUE.
*       readValPtr     - Contains the value read from the register which caused
*                        the failure if testStatusPtr is GT_FALSE. Irrelevant if
*                        testStatusPtr is GT_TRUE.
*       writeValPtr    - Contains the value written to the register which caused
*                        the failure if testStatusPtr is GT_FALSE. Irrelevant if
*                        testStatusPtr is GT_TRUE.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*       The test is done by writing and reading a test pattern.
*       startOffset must be aligned to 4 Bytes and size must be in 4 bytes
*       resolution.
*       The function may be called after Phase 2 initialization.
*       The test is destructive and leaves the memory corrupted.
*
*******************************************************************************/
typedef GT_STATUS (*PRV_CPSS_DIAG_MEMORY_TEST_FUNC)
(
    IN GT_U8                          devNum,
    IN CPSS_DIAG_PP_MEM_TYPE_ENT      memType,
    IN GT_U32                         startOffset,
    IN GT_U32                         size,
    IN CPSS_DIAG_TEST_PROFILE_ENT     profile,
    OUT GT_BOOL                       *testStatusPtr,
    OUT GT_U32                        *addrPtr,
    OUT GT_U32                        *readValPtr,
    OUT GT_U32                        *writeValPtr
);

/**
* @struct PRV_CPSS_DIAG_RAM_SPECIAL_TREATMENT_FUNC_STC
 *
 * @brief Special RAM treatment functions
*/
typedef struct{

    /** @brief BUFFER_DRAM (buff mem) memory
     *  write function
     */
    PRV_CPSS_DIAG_HW_PP_RAM_BUF_MEM_WRITE_FUNC prvCpssDiagHwPpRamBufMemWriteFuncPtr;

    /** @brief WORD by WORD memory reading
     *  function
     */
    PRV_CPSS_DIAG_HW_PP_RAM_BUF_MEM_READ_FUNC prvCpssDiagHwPpRamInWordsReadFuncPtr;

    /** writing to table entry function */
    PRV_CPSS_DIAG_HW_PP_RAM_BUF_MEM_WRITE_FUNC prvCpssDiagHwPpMemoryWriteFuncPtr;

    /** reading from table entry function */
    PRV_CPSS_DIAG_HW_PP_RAM_BUF_MEM_READ_FUNC prvCpssDiagHwPpMemoryReadFuncPtr;

    /** @brief Special comparing function for
     *  memories when read data is different
     *  from writen one.
     *  E.g. TCAM where device converts
     *  writen mask/pattern data to X/Y
     *  representation. But read data is
     *  always in X/Y format.
     */
    PRV_CPSS_DIAG_HW_PP_RAM_MEM_COMPARE_FUNC prvCpssDiagHwPpMemoryCompareFuncPtr;

} PRV_CPSS_DIAG_RAM_SPECIAL_TREATMENT_FUNC_STC;

/**
* @internal prvCpssDiagBurstSizeSet function
* @endinternal
*
* @brief   This interface change the burst size.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; ExMxPm; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] burstSize                : new burst size (in bytes)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDiagBurstSizeSet
(
    IN GT_U32 burstSize
);

/**
* @internal prvCpssDiagBurstSizeGet function
* @endinternal
*
* @brief   This interface routine return the burst size.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; ExMxPm; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_U32                   - burst size (in bytes)
*/
GT_U32 prvCpssDiagBurstSizeGet
(
    GT_VOID
);

/**
* @internal prvCpssDiagMemMaskArraySet function
* @endinternal
*
* @brief   fill memMaskArSize with value of maskArrSize
*         fill memMaskArray with data recieved from maskArray
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; ExMxPm; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] maskArrSize              - number of words to use as masks
* @param[in] maskArray[]              - original data to fill the maskMemArray with.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note mask array is declared to diagnose specific memory areas
*       such as SALSA MAC or VLAN tables where each element contains
*       several words and each word uses certain set of bits only.
*       Mask array size is equal to area's element words number
*       and every word in the array is a mask to corresponding
*       element's word (mask bit n = 1 if bit n is used in the word)
*       In the regular memory case mask array should contain only
*       one word with all bits = 1, i.e. FFFFFFFF
*
*/
GT_STATUS prvCpssDiagMemMaskArraySet
(
    IN  GT_U32 maskArrSize,
    IN  GT_U32 maskArray[]
);

/**
* @internal prvCpssDiagAnyMemTest function
* @endinternal
*
* @brief   This routine preforms a memory check.
*         calling that function with the memory profile.
*         like random / toggle / aa_55 / incremental.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; ExMxPm; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of only valid Port Groups.
* @param[in] memBase                  - memory pointer to start the test from
* @param[in] size                     - the  of memory to check (in bytes)
* @param[in] profile                  - the test profile
* @param[in] specialRamFuncs          - Special RAM treatment functions
*
* @param[out] testStatusPtr            - GT_TRUE if the test succeeded or GT_FALSE for failure
* @param[out] addrPtr                  - Address offset of memory error, if testStatusPtr is
*                                      GT_FALSE. Irrelevant if testStatusPtr is GT_TRUE.
* @param[out] readValPtr               - Contains the value read from the register which caused
*                                      the failure if testStatusPtr is GT_FALSE. Irrelevant if
* @param[out] testStatusPtr            is GT_TRUE.
* @param[out] writeValPtr              - Contains the value written to the register which caused
*                                      the failure if testStatusPtr is GT_FALSE. Irrelevant if
* @param[out] testStatusPtr            is GT_TRUE.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This function depends on init the "memDiagBurstSize" by
*       prvCpssDiagSetBurstSize().
*
*/
GT_STATUS prvCpssDiagAnyMemTest
(
    IN GT_U8                                             devNum,
    IN GT_U32                                            portGroupsBmp,
    IN GT_U32                                            memBase,
    IN GT_U32                                            size,
    IN CPSS_DIAG_TEST_PROFILE_ENT                        profile,
    IN PRV_CPSS_DIAG_RAM_SPECIAL_TREATMENT_FUNC_STC      specialRamFuncs,
    OUT GT_BOOL                                          *testStatusPtr,
    OUT GT_U32                                           *addrPtr,
    OUT GT_U32                                           *readValPtr,
    OUT GT_U32                                           *writeValPtr
);

/**
* @internal prvCpssDiagHwPpRamWrite function
* @endinternal
*
* @brief   Write to PP's RAM.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; ExMxPm; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The PP device number to write to.
* @param[in] portGroupId              - the port group Id , to support multi-port-group
*                                      devices that need to access specific port group
* @param[in] addr                     - Address offset to write to.
* @param[in] length                   - Number of Words (4 byte) to write.
* @param[in] specialRamFuncs          - Special RAM treatment functions
* @param[in] dataPtr                  - An array containing the data to be written.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDiagHwPpRamWrite
(
    IN GT_U8                                             devNum,
    IN GT_U32                                            portGroupId,
    IN GT_U32                                            addr,
    IN GT_U32                                            length,
    IN PRV_CPSS_DIAG_RAM_SPECIAL_TREATMENT_FUNC_STC      specialRamFuncs,
    IN GT_U32_PTR                                        dataPtr
);

/**
* @internal prvCpssDiagHwPpRamRead function
* @endinternal
*
* @brief   Read from PP's RAM.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; ExMxPm; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The PP device number to read from.
* @param[in] portGroupId              - the port group Id , to support multi-port-group
*                                      devices that need to access specific port group
* @param[in] addr                     - Address offset to read from.
* @param[in] length                   - Number of Words (4 byte) to read.
* @param[in] specialRamFuncs          - Special RAM treatment functions
*
* @param[out] dataPtr                  - An array containing the read data.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDiagHwPpRamRead
(
    IN GT_U8                                             devNum,
    IN GT_U32                                            portGroupId,
    IN GT_U32                                            addr,
    IN GT_U32                                            length,
    IN PRV_CPSS_DIAG_RAM_SPECIAL_TREATMENT_FUNC_STC      specialRamFuncs,
    OUT GT_U32                                           *dataPtr
);

/**
* @internal prvCpssDiagHwPpRamMemoryCompare function
* @endinternal
*
* @brief   Compare characters from the object pointed to by 'writeBufferPtr' to
*         the object pointed to by 'readBufferPtr', by checking words and applying
*         masks.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; ExMxPm; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] writeBufferPtr           - writen data
* @param[in] readBufferPtr            - read data
* @param[in] size                     -  of memory to compare in bytes
* @param[in] maskArrIndxPtr           - pointer to mask index
* @param[in] specialRamFuncs          - Special RAM treatment functions
*
* @retval GT_TRUE                  - when content of read and write buffers are equal
* @retval GT_FALSE                 - when content of read and write buffers are not equal
*/
GT_BOOL prvCpssDiagHwPpRamMemoryCompare
(
    IN GT_U32                                           *writeBufferPtr,
    IN GT_U32                                           *readBufferPtr,
    IN GT_U32                                           size,
    IN GT_U32                                           *maskArrIndxPtr,
    IN  PRV_CPSS_DIAG_RAM_SPECIAL_TREATMENT_FUNC_STC    specialRamFuncs
);

/**
* @internal prvCpssDiagMemoryForAllProfiles function
* @endinternal
*
* @brief   Performs all the pattern tests for a specified memory type and size.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; ExMxPm; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The device number to test
* @param[in] memType                  - The packet processor memory type to verify.
* @param[in] size                     - The memory  in byte to test (start from offset).
* @param[in] diagMemTestFunc          - Memory test function pointer
*
* @param[out] testStatusPtr            - GT_TRUE if the test succeeded or GT_FALSE for failure
* @param[out] addrPtr                  - Address offset of memory error, if testStatusPtr is
*                                      GT_FALSE. Irrelevant if testStatusPtr is GT_TRUE.
* @param[out] readValPtr               - Contains the value read from the register which caused
*                                      the failure if testStatusPtr is GT_FALSE. Irrelevant if
* @param[out] testStatusPtr            is GT_TRUE.
* @param[out] writeValPtr              - Contains the value written to the register which caused
*                                      the failure if testStatusPtr is GT_FALSE. Irrelevant if
* @param[out] testStatusPtr            is GT_TRUE.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - diagMemTestFunc is NULL
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The test is done by calling cpssExMxDiagMemTest for all the patterns.
*
*/
GT_STATUS prvCpssDiagMemoryForAllProfiles
(
    IN GT_U8                           devNum,
    IN CPSS_DIAG_PP_MEM_TYPE_ENT       memType,
    IN GT_U32                          size,
    IN PRV_CPSS_DIAG_MEMORY_TEST_FUNC  diagMemTestFunc,
    OUT GT_BOOL                        *testStatusPtr,
    OUT GT_U32                         *addrPtr,
    OUT GT_U32                         *readValPtr,
    OUT GT_U32                         *writeValPtr
);

/**
* @internal prvCpssDiagPhyRegWrite function
* @endinternal
*
* @brief   Performs single 32 bit data write to one of the PHY registers.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; ExMxPm; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] baseAddr                 - The base address to access the device
* @param[in] ifChannel                - interface channel (PCI/SMI/TWSI)
* @param[in] smiRegOffset             - The SMI register offset
* @param[in] phyAddr                  - phy address to access
* @param[in] offset                   - PHY register offset
* @param[in] data                     -  to write
* @param[in] doByteSwap               - GT_TRUE:  byte swap will be done on the written data
*                                      GT_FALSE: byte swap will not be done on the written data
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PARAM             - on wrong input parameter
*
* @note The function may be called before Phase 1 initialization.
*
*/
GT_STATUS prvCpssDiagPhyRegWrite
(
    IN GT_U32                          baseAddr,
    IN CPSS_PP_INTERFACE_CHANNEL_ENT   ifChannel,
    IN GT_U32                          smiRegOffset,
    IN GT_U32                          phyAddr,
    IN GT_U32                          offset,
    IN GT_U32                          data,
    IN GT_BOOL                         doByteSwap
);

/**
* @internal prvCpssDiagPhyRegRead function
* @endinternal
*
* @brief   Performs single 32 bit data read from one of the PHY registers.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; ExMxPm; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] baseAddr                 - The base address to access the device
* @param[in] ifChannel                - interface channel (PCI/SMI/TWSI)
* @param[in] smiRegOffset             - The SMI register offset
* @param[in] phyAddr                  - phy address to access
* @param[in] offset                   - PHY register offset
* @param[in] doByteSwap               - GT_TRUE:  byte swap will be done on the read data
*                                      GT_FALSE: byte swap will not be done on the read data
*
* @param[out] dataPtr                  - read data
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PARAM             - on wrong input parameter
* @retval GT_BAD_PTR               - on NULL pointer
*
* @note The function may be called before Phase 1 initialization.
*
*/
GT_STATUS prvCpssDiagPhyRegRead
(
    IN GT_U32                          baseAddr,
    IN CPSS_PP_INTERFACE_CHANNEL_ENT   ifChannel,
    IN GT_U32                          smiRegOffset,
    IN GT_U32                          phyAddr,
    IN GT_U32                          offset,
    OUT GT_U32                         *dataPtr,
    IN GT_BOOL                         doByteSwap
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssGenDiagh */


