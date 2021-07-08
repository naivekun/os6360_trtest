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
* @file prvCpssGenDiag.c
*
* @brief Internal CPSS functions.
*
* @version   21
********************************************************************************
*/

#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/generic/diag/private/prvCpssGenDiag.h>
#include <cpssDriver/pp/hardware/prvCpssDrvHwCntl.h>
#include <cpss/generic/cpssHwInit/private/prvCpssHwInit.h>
#include <cpss/generic/config/private/prvCpssConfigTypes.h>

#ifdef ASIC_SIMULATION
    #include <asicSimulation/SCIB/scib.h>
#endif /*ASIC_SIMULATION*/

#ifdef CPSS_DEBUG_DIAG
#define DBG_INFO(x)     cpssOsPrintf x
#else
#define DBG_INFO(x)
#endif

/*******************************************************************************
* Internal usage environment parameters
*******************************************************************************/

#define DIAG_BYTE_SWAP_MAC(doSwap, data)            \
    (doSwap == GT_TRUE) ? (BYTESWAP_MAC(data)) : data;

/* Macro to calculate the memory address from the device, memory size and memory
   base address */
#define CALC_MEM_ADDR_MAC(memBase,memSize)          \
    memBase + memSize

/* Macro to calculate address completion for PHY access */
#define PRV_DIAG_CALC_PHY_ACCESS_COMPLETION_MAC(value)  ((value) >> 27) & 0x1

#ifdef GM_USED  /* the GM supports address completion */
#elif defined ASIC_SIMULATION   /* cause compilation error , since not supports address completion */
#else /* not GM_USED and not ASIC_SIMULATION*/
#endif
/* offset of the address completion for PEX 8 completion regions */
#define PEX_MBUS_ADDR_COMP_REG_MAC(_index) (0x120 + (4 * _index))
/* bits of address passes as is throw PCI window */
#define NOT_ADDRESS_COMPLETION_BITS_NUM_CNS 19
/* bits of address extracted from address completion registers */
#define ADDRESS_COMPLETION_BITS_MASK_CNS    (0xFFFFFFFF << NOT_ADDRESS_COMPLETION_BITS_NUM_CNS)


/* use region 2 for address completion */
#define DIAG_ADDR_COMPLETION_REGION_CNS 2

/* define max number of iteration for status checking*/
#define PRV_GEN_DIAG_MAX_NUM_OF_ITERATIONS 66000000

/* MACRO for checking if number of iterations is less than limit*/
#define  _GEN_DIAG_MAX_NUM_ITERATIONS_CHECK_CNS(currCount)    \
    if (currCount >= PRV_GEN_DIAG_MAX_NUM_OF_ITERATIONS) \
       CPSS_LOG_ERROR_AND_RETURN_MAC(GT_TIMEOUT, prvCpssLogErrorMsgGlobalIteratorNotValid, currCount);


#ifndef ASIC_SIMULATION
    #define  PRV_CPSS_GEN_DIAG_MAX_NUM_ITERATIONS_CHECK_CNS(currIter)           \
        _GEN_DIAG_MAX_NUM_ITERATIONS_CHECK_CNS(currIter)
#else /* ASIC_SIMULATION */
    /* for the GM most operations ended in context of 'triggering' except for
       FDB 'aging daemon' */
    /* for non GM (WM): we need to allow the 'other task' to process the operation
       that we wait for */
    #define  PRV_CPSS_GEN_DIAG_MAX_NUM_ITERATIONS_CHECK_CNS(currIter)         \
        {                                                                     \
            /* use 'tmp' because the currIter used with ++ at the caller */   \
            GT_U32  tmpIter = currIter;                                       \
            _GEN_DIAG_MAX_NUM_ITERATIONS_CHECK_CNS(tmpIter)                   \
            if((tmpIter) >= 2000)/*in any case 20 seconds are enough */       \
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_TIMEOUT, LOG_ERROR_NO_MSG);  \
            else if(tmpIter > 3)                                          \
                cpssOsTimerWkAfter(10);                                   \
            else /* iteration 0,1,2,3 */                                  \
                cpssOsTimerWkAfter(0); /* allow context switch */         \
        }
#endif /*ASIC_SIMULATION*/


/*******************************************************************************
* Internal static function declaration
*******************************************************************************/
static void setWriteBuffer
(
    IN  GT_U32  bufferSize,
    IN  GT_U32  background,
    OUT GT_U32* writeBufferPtr
);

static GT_STATUS testAnyMemToggle
(
    IN  GT_U8                                             devNum,
    IN  GT_U32                                             portGroupId,
    IN  GT_U32                                            memBase,
    IN  GT_U32                                            size,
    IN  PRV_CPSS_DIAG_RAM_SPECIAL_TREATMENT_FUNC_STC      specialRamFuncs,
    OUT GT_BOOL                                           *testStatusPtr,
    OUT GT_U32                                            *addrPtr,
    OUT GT_U32                                            *readValPtr,
    OUT GT_U32                                            *writeValPtr
);

static GT_STATUS testAnyMemIncremental
(
    IN  GT_U8                                             devNum,
    IN  GT_U32                                            portGroupId,
    IN  GT_U32                                            memBase,
    IN  GT_U32                                            size,
    IN  PRV_CPSS_DIAG_RAM_SPECIAL_TREATMENT_FUNC_STC      specialRamFuncs,
    OUT GT_BOOL                                           *testStatusPtr,
    OUT GT_U32                                            *addrPtr,
    OUT GT_U32                                            *readValPtr,
    OUT GT_U32                                            *writeValPtr
);

static GT_STATUS testAnyMem_AA_55
(
    IN  GT_U8                                             devNum,
    IN  GT_U32                                            portGroupId,
    IN  GT_U32                                            memBase,
    IN  GT_U32                                            size,
    IN  PRV_CPSS_DIAG_RAM_SPECIAL_TREATMENT_FUNC_STC      specialRamFuncs,
    OUT GT_BOOL                                           *testStatusPtr,
    OUT GT_U32                                            *addrPtr,
    OUT GT_U32                                            *readValPtr,
    OUT GT_U32                                            *writeValPtr
);

static GT_STATUS testAnyMemRandom
(
    IN  GT_U8                                             devNum,
    IN  GT_U32                                            portGroupId,
    IN  GT_U32                                            memBase,
    IN  GT_U32                                            size,
    IN  PRV_CPSS_DIAG_RAM_SPECIAL_TREATMENT_FUNC_STC      specialRamFuncs,
    OUT GT_BOOL                                           *testStatusPtr,
    OUT GT_U32                                            *addrPtr,
    OUT GT_U32                                            *readValPtr,
    OUT GT_U32                                            *writeValPtr
);

static GT_BOOL testMemCmp
(
    IN GT_U32                  *str1Ptr,
    IN GT_U32                  *str2Ptr,
    IN GT_U32                   size,
    IN GT_U32                  *maskArrIndxPtr
);


/* the background data is kept in here  we assume no more than 16*4=64B burst*/
static GT_U32   backgroundBuffer[PRV_DIAG_MAX_BURST_SIZE_CNS];
/* the data is kept in here  we assume no more than 16*4=64B burst*/
static GT_U32   writeBuffer[PRV_DIAG_MAX_BURST_SIZE_CNS];
/* the data is kept in here  we assume no more than 16*4=64B burst*/
static GT_U32   readBuffer[PRV_DIAG_MAX_BURST_SIZE_CNS];

/**********************************************************************
* The following Mask data is declared to diagnose a specific memory
* areas such as SALSA MAC or VLAN tables where each element contains
* several words and each word uses certain set of bits only.
* Mask Array Size is equal to area's element words number
* and every word in the Mask Array is a mask to corresponding
* element's word (mask bit n = 1 if bit n is used in the word)
* In the regular memory case Mask Array contains only one word
* with all bits = 1 (i.e. FFFFFFFF) and Mask Array Size = 1.
**********************************************************************/
static GT_U32   memMaskArSize;
static GT_U32   memMaskArray[PRV_DIAG_MAX_MASK_NUM_CNS];

static GT_U32   memDiagBurstSize = 4; /* memDiagBurstSize is in bytes */
static GT_BOOL  testInProgress = GT_FALSE ;

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
)
{
    if((testInProgress == GT_FALSE &&
       (burstSize % 4)  == 0x0)     &&
       (burstSize <= PRV_DIAG_MAX_BURST_SIZE_CNS))
    {
        memDiagBurstSize = burstSize;
        return GT_OK;
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }
}

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
)
{
    return memDiagBurstSize;
}

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
)
{
    GT_U32 bufferPos;

    if((testInProgress == GT_FALSE) && (maskArrSize <= PRV_DIAG_MAX_MASK_NUM_CNS))
    {
       memMaskArSize = maskArrSize;
       for (bufferPos=0; bufferPos < maskArrSize; bufferPos++)
       {
           memMaskArray[bufferPos] = maskArray[bufferPos];
       }
       return GT_OK;
    }
    else
       CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);

}

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
)
{
    GT_U32    portGroupId;      /* port group id    */
    GT_STATUS ret = GT_OK;      /* return code      */

    *writeValPtr = 0x0;
    *readValPtr  = 0x0;
    *addrPtr     = memBase;

    if (testInProgress == GT_TRUE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    testInProgress = GT_TRUE ;

    /* loop on all active port groups in the bmp */
    PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_IN_BMP_MAC(devNum,portGroupsBmp,portGroupId)
    {
        switch (profile)
        {
            case CPSS_DIAG_TEST_BIT_TOGGLE_E:
                ret = testAnyMemToggle(devNum, portGroupId, memBase, size,
                                       specialRamFuncs, testStatusPtr, addrPtr,
                                       readValPtr, writeValPtr);
                break;
            case CPSS_DIAG_TEST_INCREMENTAL_E:
                ret = testAnyMemIncremental(devNum, portGroupId, memBase, size,
                                            specialRamFuncs, testStatusPtr, addrPtr,
                                            readValPtr, writeValPtr);
                break;
            case CPSS_DIAG_TEST_AA_55_E:
                ret = testAnyMem_AA_55(devNum, portGroupId, memBase, size,
                                       specialRamFuncs, testStatusPtr, addrPtr,
                                       readValPtr, writeValPtr);
                break;
            case CPSS_DIAG_TEST_RANDOM_E:
                ret = testAnyMemRandom(devNum, portGroupId, memBase, size,
                                       specialRamFuncs, testStatusPtr, addrPtr,
                                       readValPtr, writeValPtr);
                break;
        default:
            testInProgress = GT_FALSE;
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }
    PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_IN_BMP_MAC(devNum,portGroupsBmp,portGroupId)


    testInProgress = GT_FALSE;
    return ret;
}

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
)
{
    GT_STATUS rc;
    if (specialRamFuncs.prvCpssDiagHwPpRamBufMemWriteFuncPtr != NULL)
    {
        rc = specialRamFuncs.prvCpssDiagHwPpRamBufMemWriteFuncPtr
            (devNum, portGroupId, addr, length, dataPtr); /* Buff Mem addr space */
    }
    else if (specialRamFuncs.prvCpssDiagHwPpMemoryWriteFuncPtr != NULL)
    {
        rc = specialRamFuncs.prvCpssDiagHwPpMemoryWriteFuncPtr
            (devNum, portGroupId, addr, length, dataPtr);
    }
    else
    {
        rc = prvCpssHwPpPortGroupWriteRam(devNum, portGroupId, addr, length,
                                             dataPtr);
    }
    return rc;
}

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
)
{
    GT_STATUS rc;
    if (specialRamFuncs.prvCpssDiagHwPpRamInWordsReadFuncPtr != NULL)
    {
        rc = specialRamFuncs.prvCpssDiagHwPpRamInWordsReadFuncPtr
            (devNum, portGroupId, addr, length, dataPtr); /* Buff Mem addr space */
    }
    else if (specialRamFuncs.prvCpssDiagHwPpMemoryReadFuncPtr != NULL)
    {
        rc = specialRamFuncs.prvCpssDiagHwPpMemoryReadFuncPtr
            (devNum, portGroupId, addr, length, dataPtr);
    }
    else
    {
        rc = prvCpssHwPpPortGroupReadRam(devNum, portGroupId, addr, length,
                                            dataPtr);
    }
    return rc;
}

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
)
{
    GT_STATUS rc;
    if (specialRamFuncs.prvCpssDiagHwPpMemoryCompareFuncPtr != NULL)
    {
        rc = specialRamFuncs.prvCpssDiagHwPpMemoryCompareFuncPtr
            (writeBufferPtr, readBufferPtr, size, maskArrIndxPtr);
        return rc;
    }
    rc = testMemCmp(writeBufferPtr, readBufferPtr, size, maskArrIndxPtr);
    return rc;
}

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
)
{
    GT_STATUS status;

    if (diagMemTestFunc == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, LOG_ERROR_NO_MSG);
    }

    /* AA_55 test */
    status = diagMemTestFunc (devNum, memType, 0, size, CPSS_DIAG_TEST_AA_55_E,
                              testStatusPtr, addrPtr, readValPtr, writeValPtr);
    PRV_CPSS_DIAG_CHECK_TEST_RESULT_MAC(status, *testStatusPtr);

    /* Random test */
    status = diagMemTestFunc (devNum, memType, 0, size, CPSS_DIAG_TEST_RANDOM_E,
                              testStatusPtr, addrPtr, readValPtr, writeValPtr);
    PRV_CPSS_DIAG_CHECK_TEST_RESULT_MAC(status, *testStatusPtr);

    /* incremental test */
    return diagMemTestFunc (devNum, memType, 0, size, CPSS_DIAG_TEST_INCREMENTAL_E,
                            testStatusPtr, addrPtr, readValPtr, writeValPtr);
}

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
)
{
    GT_STATUS status;
    GT_U32 value;           /* value to write into the register */
    GT_U32 completion;
    GT_U32  retryCount = 0;             /* Counter for busy wait loops */

    /* check input parameters */
    if (ifChannel >= CPSS_CHANNEL_LAST_E || phyAddr >= BIT_5 ||
        offset >= BIT_5 || data >= BIT_16)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    value = (data) | (((phyAddr) & 0x1F) << 16) | (((offset) & 0x1F) << 21);

    /* write value */
    status = prvCpssDiagRegWrite (baseAddr, ifChannel,
                                  CPSS_DIAG_PP_REG_INTERNAL_E, smiRegOffset,
                                  value, doByteSwap);
    if (status != GT_OK)
    {
        return status;
    }

    /* check if write operation has finished */
    do
    {
        status = prvCpssDiagRegRead (baseAddr, ifChannel,
                                     CPSS_DIAG_PP_REG_INTERNAL_E, smiRegOffset,
                                     &value, doByteSwap);
        if (status != GT_OK)
        {
            return status;
        }
        completion = PRV_DIAG_CALC_PHY_ACCESS_COMPLETION_MAC(value);

        /* check that the number of iterations does not exceed the limit */
        PRV_CPSS_GEN_DIAG_MAX_NUM_ITERATIONS_CHECK_CNS(retryCount++);
    }while (completion);

    return GT_OK;
}

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
)
{
    GT_STATUS status;
    GT_U32 value;           /* value to read from the register */
    GT_U32 completion;
    GT_U32  retryCount = 0;             /* Counter for busy wait loops */


    CPSS_NULL_PTR_CHECK_MAC(dataPtr);

    /* check input parameters */
    if (ifChannel >= CPSS_CHANNEL_LAST_E || phyAddr >= BIT_5 ||
        offset >= BIT_5)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    value = (((phyAddr) & 0x1F) << 16) | (((offset) & 0x1F) << 21) | (1 << 26);

    status = prvCpssDiagRegWrite (baseAddr, ifChannel,
                                  CPSS_DIAG_PP_REG_INTERNAL_E, smiRegOffset,
                                  value, doByteSwap);
    if (status != GT_OK)
    {
        return status;
    }

    /* check if read operation has finished */
    do
    {
        status = prvCpssDiagRegRead (baseAddr, ifChannel,
                                     CPSS_DIAG_PP_REG_INTERNAL_E, smiRegOffset,
                                     &value, doByteSwap);
        if (status != GT_OK)
        {
            return status;
        }
        completion = PRV_DIAG_CALC_PHY_ACCESS_COMPLETION_MAC(value);
        /* check that the number of iterations does not exceed the limit */
        PRV_CPSS_GEN_DIAG_MAX_NUM_ITERATIONS_CHECK_CNS(retryCount++);
    }while (!completion);

    *dataPtr = (GT_U16)((value) & 0xFFFF);
    return GT_OK;
}

/***************************** Static Functions ******************************/

/**
* @internal setWriteBuffer function
* @endinternal
*
* @brief   set all the buffer with background.
*         fill position "pos" with word.
*         writeBuffer is in bytes.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; ExMxPm; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] bufferSize               - number of words to fill into the buffer.
* @param[in] background               - data to fill the memory with
*
* @param[out] writeBufferPtr           - the buffer to write into.
*                                       None
*/
static void setWriteBuffer
(
    IN  GT_U32  bufferSize,
    IN  GT_U32  background,
    OUT GT_U32  *writeBufferPtr
)
{
    GT_U32 bufferPos=0;

    for (bufferPos=0 ;bufferPos< (bufferSize/4) ;bufferPos++)
    {
        writeBufferPtr[bufferPos] = background;
    }
}

/**
* @internal testAnyMemToggle function
* @endinternal
*
* @brief   This routine preforms toggle memory check.
*         it scane every bit individualy.
*         chaging every bit and checking that none of the other bit has changed.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; ExMxPm; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portGroupId              - the port group Id , to support multi-port-group devices
*                                      that need to access specific port group
* @param[in] memBase                  - memory pointer to start the test from
* @param[in] size                     - the  of memory to check (in bytes)
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
*
* @note This function depends on init the "memDiagBurstSize" by
*       prvCpssDiagSetBurstSize().
*
*/
static GT_STATUS testAnyMemToggle
(
    IN  GT_U8                                             devNum,
    IN  GT_U32                                             portGroupId,
    IN  GT_U32                                            memBase,
    IN  GT_U32                                            size,
    IN  PRV_CPSS_DIAG_RAM_SPECIAL_TREATMENT_FUNC_STC      specialRamFuncs,
    OUT GT_BOOL                                           *testStatusPtr,
    OUT GT_U32                                            *addrPtr,
    OUT GT_U32                                            *readValPtr,
    OUT GT_U32                                            *writeValPtr
)
{
    GT_U32    i,j,k;               /* loops                               */
    GT_U32    phase;               /* test phase                          */
    GT_STATUS ret ;
    GT_32     retCmp;               /* ret from memCmp        */
    GT_U32    background;           /* the RAM background     */
    GT_U32    currentWord;          /* current written word   */
    GT_U32    currentAddr = 0;      /* current tested address */
    GT_U32    currentAddrScan ;     /*                        */
    GT_U32    currentAddrVerify ;   /*                        */
    GT_U32    currentAddrOverride ; /*                        */
    GT_U32    numberOfBitsInBurst ;
    GT_U32    bitNumberInWord ;
    GT_U32    wordOffsetInBurst ;
    GT_U32    wordMask ;
    GT_U32    currentToggle ;
    GT_U32    saveMaskPos;
    GT_U32    maskArrayPos;         /* current mask number to use */

    numberOfBitsInBurst =  memDiagBurstSize * 8 ;
    saveMaskPos = 0;
    *testStatusPtr = GT_TRUE;

    /*
    fill all DRAM with 0x00000000,
    for each bit, set bit to '1' verify all other bits are '0' and restore.
    fill all DRAM with 0xFFFFFFFF
    for each bit, set bit to '0' verify all other bits are '1' and restore.
    */
    DBG_INFO(("Toggle test\n"));

    /* Phase 0 - set 0x00000000 and Toggle '1' */
    /* Phase 1 - set 0xFFFFFFFF and Toggle '0' */
    for(phase = 0;phase < 2;phase++)
    {
        background = (phase == 0) ? 0x00000000:0xFFFFFFFF;
        /* reset the tested area */
        setWriteBuffer(memDiagBurstSize ,background, backgroundBuffer);
        for(i = 0; i < size; i += memDiagBurstSize)
        {
            currentAddr = CALC_MEM_ADDR_MAC(memBase, i);
            /* prvCpssDiagHwPpRamWrite get params in words */
            ret = prvCpssDiagHwPpRamWrite (devNum , portGroupId, currentAddr,
                                           memDiagBurstSize/4, specialRamFuncs,
                                           backgroundBuffer);
            if(ret != GT_OK)
            {
                DBG_INFO(("Failed on write.\n"));
                return ret;
            }
        }
        /* start toggling */
        for(i = 0; i < size; i += memDiagBurstSize)
        {
            currentAddrScan = CALC_MEM_ADDR_MAC(memBase, i);

            /* scan all word bits */
            for(j = 0; j < numberOfBitsInBurst; j++)
            {
                bitNumberInWord   = j % 32 ;
                wordOffsetInBurst = j / 32 ;
                currentToggle = (1 << bitNumberInWord);
                currentWord = (phase == 0) ? (currentToggle):
                                            ~(currentToggle);
                /*as Burst as Mask are aligned to table entries */
                maskArrayPos = wordOffsetInBurst;
                wordMask = memMaskArray[maskArrayPos];
                /* do the check only for unmasked bits */
                if((currentToggle & wordMask) != 0x0)
                {
                    /* reset the array with background which work with words */
                    setWriteBuffer(memDiagBurstSize,background,writeBuffer);
                    /* set the checked bit */
                    writeBuffer[wordOffsetInBurst] = currentWord ;
                    ret = prvCpssDiagHwPpRamWrite (devNum, portGroupId,
                                                   currentAddrScan,
                                                   (memDiagBurstSize/4),
                                                   specialRamFuncs,
                                                   writeBuffer);
                    if(ret != GT_OK)
                    {
                        DBG_INFO(("Failed on write.\n"));
                        return ret;
                    }
                    /* verify all other address are 0x00000000/0xffffffff */
                    for(k = 0; k < size; k += memDiagBurstSize)
                    {
                        maskArrayPos = saveMaskPos;
                        currentAddrVerify = CALC_MEM_ADDR_MAC(memBase, k);
                        ret = prvCpssDiagHwPpRamRead(devNum, portGroupId,
                                                     currentAddrVerify,
                                                     (memDiagBurstSize/4),
                                                     specialRamFuncs,
                                                     readBuffer);
                        if(ret != GT_OK)
                        {
                            DBG_INFO(("Failed on read.\n"));
                            return ret;
                        }

                        retCmp = prvCpssDiagHwPpRamMemoryCompare(
                                   writeBuffer
                                  ,readBuffer
                                  ,memDiagBurstSize
                                  ,&maskArrayPos
                                  ,specialRamFuncs) ;
                        if(/* its not the same burst */
                           (i != k && retCmp == GT_TRUE) ||
                           /* its the same burst     */
                           (i == k && retCmp == GT_FALSE))
                        {

                            *writeValPtr = writeBuffer[0];
                            *readValPtr = readBuffer[0];
                            *addrPtr = currentAddr;
                            *testStatusPtr = GT_FALSE;
                            DBG_INFO(("Test failed: Addr 0x%x, Val - 0x%x.\n"
                                ,currentAddrVerify
                                ,readBuffer[0]) );
                            return GT_OK;
                        }
                    }
                }
            }
            /* restore current address to zero and move to the next addr */
            currentAddrOverride = CALC_MEM_ADDR_MAC(memBase, i);
            /* reset the tested burst */
            ret = prvCpssDiagHwPpRamWrite (devNum, portGroupId,
                                           currentAddrOverride,
                                           (memDiagBurstSize/4),
                                           specialRamFuncs,
                                           backgroundBuffer);
            if(ret != GT_OK)
            {
                DBG_INFO(("Failed on write.\n"));
                return ret;
            }
        }
    }
    return GT_OK;
}

/**
* @internal testAnyMemIncremental function
* @endinternal
*
* @brief   This routine preforms Incremental memory check.
*         1st it fill the all memory with incremental numbers.
*         2nd it scane all the memory hopping to find the number we just wrote.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; ExMxPm; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portGroupId              - the port group Id , to support multi-port-group devices
*                                      that need to access specific port group
* @param[in] memBase                  - memory pointer to start the test from
* @param[in] size                     - the  of memory to check (in bytes)
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
*
* @note This function depends on init the "memDiagBurstSize" by
*       prvCpssDiagSetBurstSize().
*
*/
static GT_STATUS testAnyMemIncremental
(
    IN  GT_U8                                             devNum,
    IN GT_U32                                             portGroupId,
    IN  GT_U32                                            memBase,
    IN  GT_U32                                            size,
    IN  PRV_CPSS_DIAG_RAM_SPECIAL_TREATMENT_FUNC_STC      specialRamFuncs,
    OUT GT_BOOL                                           *testStatusPtr,
    OUT GT_U32                                            *addrPtr,
    OUT GT_U32                                            *readValPtr,
    OUT GT_U32                                            *writeValPtr
)
{
    GT_U32    i;                   /* loops                               */
    GT_STATUS ret ;
    GT_U32    currentAddr = 0;     /* current tested address              */
    GT_U32    incrementVal ;
    GT_U32    incrementLoop ;
    GT_U32    maskArrayPos;

    maskArrayPos = 0;
    *testStatusPtr = GT_TRUE;

    DBG_INFO(("Incremental test\n"));
    /* fill all the memory with incremental value */
    /* ========================================== */
    incrementVal = 0;
    for(i = 0; i < size; i += memDiagBurstSize)
    {
        /* fill the backgroundBuffer with incremental value */
        for(incrementLoop = 0 ;
            incrementLoop < (memDiagBurstSize/4) ;
            ++incrementLoop)
        {
            backgroundBuffer[incrementLoop] = incrementVal ;
            ++incrementVal ;
        }

        currentAddr = CALC_MEM_ADDR_MAC(memBase, i);

        /* prvCpssDiagHwPpRamWrite params in words */
        ret = prvCpssDiagHwPpRamWrite (devNum, portGroupId, currentAddr,
                                       (memDiagBurstSize/4), specialRamFuncs,
                                       backgroundBuffer);
        if(ret != GT_OK)
        {
            DBG_INFO(("Failed on write.\n"));
            return ret;
        }
    }
    /* check the all memory for the inserted values */
    /* ============================================ */
    incrementVal = 0;
    for(i = 0; i < size; i += memDiagBurstSize)
    {
        /* fill the backgroundBuffer with incremental value */
        for(incrementLoop = 0 ;
            incrementLoop < (memDiagBurstSize/4) ;
            ++incrementLoop)
        {
            backgroundBuffer[incrementLoop] = incrementVal ;
            ++incrementVal ;
        }
        /* The following IF operator has been removed as erroneous:
           it eliminates from check the Current memory burst when
           incrementVal is used masked bits for the first word
           of the NEXT memory burst. Moreover this method cannot
           be applied for different masks declared in Mask Array  */
        /**********************************************************/
        /* do the check only for masked bits */
        /* if((incrementVal & ~wordMask) == 0x0) */
        /**********************************************************/

        /* compare the backgroundBuffer with the Memory Burst */
        {
            currentAddr = CALC_MEM_ADDR_MAC(memBase, i);

            ret = prvCpssDiagHwPpRamRead(devNum, portGroupId, currentAddr,
                                         (memDiagBurstSize/4), specialRamFuncs,
                                         readBuffer);
            if(ret != GT_OK)
            {
                DBG_INFO(("Failed on read.\n"));
                return ret;
            }

            ret = prvCpssDiagHwPpRamMemoryCompare(
                     backgroundBuffer
                    ,readBuffer
                    ,memDiagBurstSize
                    ,&maskArrayPos
                    ,specialRamFuncs) ;
            if(ret == GT_FALSE) /* its not the same data */
            {
                *testStatusPtr = GT_FALSE;
                *writeValPtr = backgroundBuffer[0];
                *readValPtr = readBuffer[0];
                *addrPtr = currentAddr;
                DBG_INFO(("Test failed: Addr 0x%x, Val - 0x%x.\n"
                  ,currentAddr
                  ,readBuffer[0]) );
                return GT_OK;
            }
        }
    }

    return GT_OK;
}

/**
* @internal testAnyMem_AA_55 function
* @endinternal
*
* @brief   This routine preforms check the memory with Const Var.
*         1st fill all the memry with a patern.
*         2nd it check that all the memry has that patern.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; ExMxPm; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portGroupId              - the port group Id , to support multi-port-group devices
*                                      that need to access specific port group
* @param[in] memBase                  - memory pointer to start the test from
* @param[in] size                     - the  of memory to check (in bytes)
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
*
* @note This function depends on init the "memDiagBurstSize" by
*       prvCpssDiagSetBurstSize().
*
*/
static GT_STATUS testAnyMem_AA_55
(
    IN  GT_U8                                             devNum,
    IN GT_U32                                             portGroupId,
    IN  GT_U32                                            memBase,
    IN  GT_U32                                            size,
    IN  PRV_CPSS_DIAG_RAM_SPECIAL_TREATMENT_FUNC_STC      specialRamFuncs,
    OUT GT_BOOL                                           *testStatusPtr,
    OUT GT_U32                                            *addrPtr,
    OUT GT_U32                                            *readValPtr,
    OUT GT_U32                                            *writeValPtr
)
{
    GT_U32    i;                   /* loops                               */
    GT_U32    phase;               /* test phase                          */
    GT_STATUS ret ;
    GT_32     retCmp;              /* ret from memCmp                     */
    GT_U32    background;          /* the RAM background                  */
    GT_U32    currentAddr = 0;     /* current tested address              */
    GT_U32    maskArrayPos;
    GT_U32    saveMaskPos;          /* to save the first pos in maskArray */

    saveMaskPos = 0;
    *testStatusPtr = GT_TRUE;

    /* 0xAAAAAAAA/0x55555555 test */
    /* ========================== */
    DBG_INFO(("AA-55 test\n"));
    /* set test bacground */
    background = 0x55555555;

    /* start test phase */
    for(phase = 0;phase < 4;phase++)
    {
        maskArrayPos = saveMaskPos;
        setWriteBuffer(memDiagBurstSize,background,backgroundBuffer);
        for(i=0; i < size; i+= memDiagBurstSize)
        {
            currentAddr = CALC_MEM_ADDR_MAC(memBase, i);

            ret = prvCpssDiagHwPpRamWrite (devNum, portGroupId, currentAddr,
                                           (memDiagBurstSize/4),
                                           specialRamFuncs,
                                           backgroundBuffer);
            if(ret != GT_OK)
            {
                DBG_INFO(("Failed on write.\n"));
                return ret;
            }
        }

        /* verify background and write ~background */
        for(i=0; i < size; i += memDiagBurstSize)
        {
            currentAddr = CALC_MEM_ADDR_MAC(memBase, i);

            ret = prvCpssDiagHwPpRamRead(devNum, portGroupId, currentAddr,
                                         (memDiagBurstSize/4), specialRamFuncs,
                                         readBuffer);
            if(ret != GT_OK)
            {
                DBG_INFO(("Failed on read.\n"));
                return ret;
            }

            retCmp=prvCpssDiagHwPpRamMemoryCompare(
                      backgroundBuffer
                     ,readBuffer
                     ,memDiagBurstSize
                     ,&maskArrayPos
                     ,specialRamFuncs) ;

            if(retCmp == GT_FALSE) /* not the same */
            {
                *testStatusPtr = GT_FALSE;
                *writeValPtr = backgroundBuffer[0];
                *readValPtr = readBuffer[0];
                *addrPtr = currentAddr;
                DBG_INFO(("Test failed: Addr 0x%x, Val - 0x%x.\n"
                  ,currentAddr
                  ,readBuffer[0]) );
                return GT_OK;
            }
        }
        /* change back ground polarity */
        background = ~background;
    }

    return GT_OK;
}

/**
* @internal testAnyMemRandom function
* @endinternal
*
* @brief   This routine preforms check the memory with random values.
*         1st fill all the memry with a patern.
*         2nd it check that all the memry has that patern.
*         Note that the pattern repeats itself every memDiagBurstSize.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; ExMxPm; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portGroupId              - the port group Id , to support multi-port-group devices
*                                      that need to access specific port group
* @param[in] memBase                  - memory pointer to start the test from
* @param[in] size                     - the  of memory to check (in bytes)
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
*
* @note This function depends on init the "memDiagBurstSize" by
*       prvCpssDiagSetBurstSize().
*
*/
static GT_STATUS testAnyMemRandom
(
    IN  GT_U8                                             devNum,
    IN GT_U32                                             portGroupId,
    IN  GT_U32                                            memBase,
    IN  GT_U32                                            size,
    IN  PRV_CPSS_DIAG_RAM_SPECIAL_TREATMENT_FUNC_STC      specialRamFuncs,
    OUT GT_BOOL                                           *testStatusPtr,
    OUT GT_U32                                            *addrPtr,
    OUT GT_U32                                            *readValPtr,
    OUT GT_U32                                            *writeValPtr
)
{
    GT_U32    i;                   /* loops                               */
    GT_STATUS ret ;
    GT_U32    currentAddr = 0;     /* current tested address              */
    GT_U32    incrementLoop ;
    GT_U32    maskArrayPos;

    maskArrayPos = 0;
    *testStatusPtr = GT_TRUE;

    DBG_INFO(("Random test\n"));

    /* fill the backgroundBuffer with random values */
    for(incrementLoop = 0 ;
        incrementLoop < (memDiagBurstSize/4) ;
        ++incrementLoop)
    {
        backgroundBuffer[incrementLoop] = cpssOsRand ();
    }

    /* fill all the memory with random value */
    /* ===================================== */
    for(i = 0; i < size; i += memDiagBurstSize)
    {
        currentAddr = CALC_MEM_ADDR_MAC(memBase, i);

        /* prvCpssDiagHwPpRamWrite params in words */
        ret = prvCpssDiagHwPpRamWrite (devNum, portGroupId, currentAddr,
                                       (memDiagBurstSize/4), specialRamFuncs,
                                       backgroundBuffer);
        if(ret != GT_OK)
        {
            DBG_INFO(("Failed on write.\n"));
            return ret;
        }
    }
    /* check the all memory for the inserted values */
    /* ============================================ */
    for(i = 0; i < size; i += memDiagBurstSize)
    {
        /* compare the backgroundBuffer with the Memory Burst */
        {
            currentAddr = CALC_MEM_ADDR_MAC(memBase, i);

            ret = prvCpssDiagHwPpRamRead(devNum, portGroupId, currentAddr,
                                         (memDiagBurstSize/4), specialRamFuncs,
                                         readBuffer);
            if(ret != GT_OK)
            {
                DBG_INFO(("Failed on read.\n"));
                return ret;
            }

            ret = prvCpssDiagHwPpRamMemoryCompare(
                     backgroundBuffer
                    ,readBuffer
                    ,memDiagBurstSize
                    ,&maskArrayPos
                    ,specialRamFuncs) ;
            if(ret == GT_FALSE) /* its not the same data */
            {
                *testStatusPtr = GT_FALSE;
                *writeValPtr = backgroundBuffer[0];
                *readValPtr = readBuffer[0];
                *addrPtr = currentAddr;
                DBG_INFO(("Test failed: Addr 0x%x, Val - 0x%x.\n"
                  ,currentAddr
                  ,readBuffer[0]) );
                return GT_OK;
            }
        }
    }

    return GT_OK;
}

/**
* @internal testMemCmp function
* @endinternal
*
* @brief   Compare 'size' characters from the object pointed to by 'str1Ptr' to
*         the object pointed to by 'str2Ptr'.
*         checking words.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; ExMxPm; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] str1Ptr                  - first memory area
* @param[in] str2Ptr                  - second memory area
* @param[in] size                     -  of memory to compare
* @param[in] maskArrIndxPtr           - pointer to mask index
*
* @retval GT_FALSE                 - if str1Ptr is different from str2Ptr
* @retval GT_TRUE                  - if str1Ptr is equal to str2Ptr
*/
static GT_BOOL testMemCmp
(
    IN GT_U32   *str1Ptr,
    IN GT_U32   *str2Ptr,
    IN GT_U32   size,
    IN GT_U32   *maskArrIndxPtr
)
{
    GT_U32 loop ;
    GT_UINTPTR str1_Ptr ;
    GT_UINTPTR str2_Ptr ;
    GT_U32 str1Mask;
    GT_U32 str2Mask;

    if((size % 4) != 0) /* check words */
    {
        return GT_FALSE;
    }

    for(loop = 0 ;loop < size ;loop+=4)
    {
        str1_Ptr  = (GT_UINTPTR)(str1Ptr) + loop ;
        str2_Ptr  = (GT_UINTPTR)(str2Ptr) + loop ;
        str1Mask = ((GT_U32)(*((GT_U32 *)str1_Ptr)) &
                      (memMaskArray[*maskArrIndxPtr]));
        str2Mask = ((GT_U32)(*((GT_U32 *)str2_Ptr)) &
                      (memMaskArray[*maskArrIndxPtr]));
        *maskArrIndxPtr = ((*maskArrIndxPtr) + 1) % memMaskArSize ;
        if(str1Mask != str2Mask)
        {
            DBG_INFO(("Failed on word 0x%x: str1Mask = 0x%x, str2Mask = 0x%x.\n",
                     (loop / 4), str1Mask, str2Mask));
            return GT_FALSE;
        }
    }
    return GT_TRUE ;
}




