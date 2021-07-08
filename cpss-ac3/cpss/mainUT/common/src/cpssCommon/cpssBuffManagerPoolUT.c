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
* @file cpssBuffManagerPoolUT.c
*
* @brief Unit tests for cpssBuffManagerPool, that provides
* Buffer management module.
*
* @version   11
********************************************************************************
*/

/* includes */
#include <cpssCommon/cpssBuffManagerPool.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>

/* Invalid enum */
#define BUFF_MNG_INVALID_ENUM_CNS             0x5AAAAAA5

#if __WORDSIZE == 64
#define REQBUFFERSIZE_MIN 8
#else
#define REQBUFFERSIZE_MIN 4
#endif

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssBmPoolCreate
(
    IN   GT_U32                         reqBufferSize,
    IN   CPSS_BM_POOL_ALIGNMENT_ENT     alignment,
    IN   GT_U32                         numOfBuffers,
    OUT  CPSS_BM_POOL_ID                *pPoolId
)
*/
UTF_TEST_CASE_MAC(cpssBmPoolCreate)
{
/*
    ITERATE_DEVICES
    1. Call with reqBufferSize [4 / 0xFF],
                 alignment [CPSS_BM_POOL_1_BYTE_ALIGNMENT_E / 
                            CPSS_BM_POOL_4_BYTE_ALIGNMENT_E],
                 numOfBuffers [1 / 10]
                 and non-NULL pPoolId.
    Expected: GT_OK.
    2. Call with out of range reqBufferSize [0]
                 and other params from 1.
    Expected: NOT GT_OK.
    3. Call with out of range alignment [0x5AAAAAA5]
                 and other params from 1.
    Expected: GT_BAD_PARAM.
    4. Call with out of range numOfBuffers [0]
                 and other params from 1.
    Expected: NOT GT_OK.
    5. Call with pPoolId [CPSS_BM_POOL_NULL_ID]
                 and other params from 1.
    Expected: GT_BAD_PTR.
    6. Call cpssBmPoolDelete with created poolId to delete created pool.
    Expected: GT_OK.
*/
    GT_STATUS   st = GT_OK;
    
    GT_U32                      reqBufferSize = 0;
    CPSS_BM_POOL_ALIGNMENT_ENT  alignment     = CPSS_BM_POOL_1_BYTE_ALIGNMENT_E;
    GT_U32                      numOfBuffers  = 0;
    CPSS_BM_POOL_ID             pPoolId, pPoolId1, pPoolId2;
    GT_BOOL                     isCreated1    = GT_FALSE;
    GT_BOOL                     isCreated2    = GT_FALSE;


    /*
        1. Call with reqBufferSize [4 / 0xFF],
                     alignment [CPSS_BM_POOL_1_BYTE_ALIGNMENT_E / 
                                CPSS_BM_POOL_4_BYTE_ALIGNMENT_E],
                     numOfBuffers [1 / 10]
                     and non-NULL pPoolId.
        Expected: GT_OK.
    */

    /* call with reqBufferSize = REQBUFFERSIZE_MIN */
    reqBufferSize = REQBUFFERSIZE_MIN;
    alignment     = CPSS_BM_POOL_1_BYTE_ALIGNMENT_E;
    numOfBuffers  = 1;

    st = cpssBmPoolCreate(reqBufferSize, alignment, numOfBuffers, &pPoolId);
    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, reqBufferSize, alignment, numOfBuffers);

    isCreated1 = (GT_OK == st) ? GT_TRUE : GT_FALSE;

    pPoolId1 = pPoolId;

    /* call with reqBufferSize = 0xFF */
    reqBufferSize = 0xFF;
    alignment     = CPSS_BM_POOL_4_BYTE_ALIGNMENT_E;
    numOfBuffers  = 10;

    st = cpssBmPoolCreate(reqBufferSize, alignment, numOfBuffers, &pPoolId);
    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, reqBufferSize, alignment, numOfBuffers);

    isCreated2 = (GT_OK == st) ? GT_TRUE : GT_FALSE;

    pPoolId2 = pPoolId;

    /*
        2. Call with out of range reqBufferSize [0]
                     and other params from 1.
        Expected: NOT GT_OK.
    */
    reqBufferSize = 0;

    st = cpssBmPoolCreate(reqBufferSize, alignment, numOfBuffers, &pPoolId);
    UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, reqBufferSize);

    reqBufferSize = 0xFF;

    /*
        3. Call with out of range alignment [0x5AAAAAA5]
                     and other params from 1.
        Expected: GT_BAD_PARAM.
    */
    alignment = BUFF_MNG_INVALID_ENUM_CNS;

    st = cpssBmPoolCreate(reqBufferSize, alignment, numOfBuffers, &pPoolId);
    UTF_VERIFY_NOT_EQUAL1_STRING_MAC(GT_OK, st, "alignment = %d", alignment);

    alignment = CPSS_BM_POOL_4_BYTE_ALIGNMENT_E;

    /*
        4. Call with out of range numOfBuffers [0]
                     and other params from 1.
        Expected: NOT GT_OK.
    */
    numOfBuffers = 0;

    st = cpssBmPoolCreate(reqBufferSize, alignment, numOfBuffers, &pPoolId);
    UTF_VERIFY_NOT_EQUAL1_STRING_MAC(GT_OK, st, "numOfBuffers = %d", numOfBuffers);

    numOfBuffers = 10;

    /*
        5. Call with pPoolId [CPSS_BM_POOL_NULL_ID]
                     and other params from 1.
        Expected: GT_BAD_PTR.
    */
    st = cpssBmPoolCreate(reqBufferSize, alignment, numOfBuffers, CPSS_BM_POOL_NULL_ID);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_BAD_PTR, st, "%pPoolId = NULL");

    /*
        6. Call cpssBmPoolDelete with created poolId to delete created pool.
        Expected: GT_OK.
    */
    if (isCreated1)
    {
        st = cpssBmPoolDelete(pPoolId1);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssBmPoolDelete: %d", pPoolId1);
    }

    if (isCreated2)
    {
        st = cpssBmPoolDelete(pPoolId2);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssBmPoolDelete: %d", pPoolId2);
    }    
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssBmPoolCreateDma
(
    IN   GT_U32                     reqBufferSize,
    IN   CPSS_BM_POOL_ALIGNMENT_ENT alignment,
    IN   GT_U32                     numOfBuffers,
    OUT  CPSS_BM_POOL_ID            *pPoolId
)
*/
UTF_TEST_CASE_MAC(cpssBmPoolCreateDma)
{
/*
    ITERATE_DEVICES
    1. Call with reqBufferSize [4 / 0xFF],
                 alignment [CPSS_BM_POOL_1_BYTE_ALIGNMENT_E / 
                            CPSS_BM_POOL_4_BYTE_ALIGNMENT_E],
                 numOfBuffers [1 / 10]
                 and non-NULL pPoolId.
    Expected: GT_OK.
    2. Call with out of range reqBufferSize [0]
                 and other params from 1.
    Expected: NOT GT_OK.
    3. Call with out of range alignment [0x5AAAAAA5]
                 and other params from 1.
    Expected: GT_BAD_PARAM.
    4. Call with out of range numOfBuffers [0]
                 and other params from 1.
    Expected: NOT GT_OK.
    5. Call with pPoolId [CPSS_BM_POOL_NULL_ID]
                 and other params from 1.
    Expected: GT_BAD_PTR.
    6. Call cpssBmPoolDelete with created poolId to delete created pool.
    Expected: GT_OK.
*/
    GT_STATUS   st = GT_OK;
    
    GT_U32                      reqBufferSize = 0;
    CPSS_BM_POOL_ALIGNMENT_ENT  alignment     = CPSS_BM_POOL_1_BYTE_ALIGNMENT_E;
    GT_U32                      numOfBuffers  = 0;
    CPSS_BM_POOL_ID             pPoolId, pPoolId1, pPoolId2;
    GT_BOOL                     isCreated1    = GT_FALSE;
    GT_BOOL                     isCreated2    = GT_FALSE;


    /*
        1. Call with reqBufferSize [4 / 0xFF],
                     alignment [CPSS_BM_POOL_1_BYTE_ALIGNMENT_E / 
                                CPSS_BM_POOL_4_BYTE_ALIGNMENT_E],
                     numOfBuffers [1 / 10]
                     and non-NULL pPoolId.
        Expected: GT_OK.
    */

    /* call with reqBufferSize = REQBUFFERSIZE_MIN */
    reqBufferSize = REQBUFFERSIZE_MIN;
    alignment     = CPSS_BM_POOL_1_BYTE_ALIGNMENT_E;
    numOfBuffers  = 1;

    st = cpssBmPoolCreateDma(reqBufferSize, alignment, numOfBuffers, &pPoolId);
    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, reqBufferSize, alignment, numOfBuffers);

    isCreated1 = (GT_OK == st) ? GT_TRUE : GT_FALSE;

    pPoolId1 = pPoolId;

    /* call with reqBufferSize = 0xFF */
    reqBufferSize = 0xFF;
    alignment     = CPSS_BM_POOL_4_BYTE_ALIGNMENT_E;
    numOfBuffers  = 10;

    st = cpssBmPoolCreateDma(reqBufferSize, alignment, numOfBuffers, &pPoolId);
    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, reqBufferSize, alignment, numOfBuffers);

    isCreated2 = (GT_OK == st) ? GT_TRUE : GT_FALSE;

    pPoolId2 = pPoolId;

    /*
        2. Call with out of range reqBufferSize [0]
                     and other params from 1.
        Expected: NOT GT_OK.
    */
    reqBufferSize = 0;

    st = cpssBmPoolCreateDma(reqBufferSize, alignment, numOfBuffers, &pPoolId);
    UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, reqBufferSize);

    reqBufferSize = 0xFF;

    /*
        3. Call with out of range alignment [0x5AAAAAA5]
                     and other params from 1.
        Expected: GT_BAD_PARAM.
    */
    alignment = BUFF_MNG_INVALID_ENUM_CNS;

    st = cpssBmPoolCreateDma(reqBufferSize, alignment, numOfBuffers, &pPoolId);
    UTF_VERIFY_NOT_EQUAL1_STRING_MAC(GT_OK, st, "alignment = %d", alignment);

    alignment = CPSS_BM_POOL_4_BYTE_ALIGNMENT_E;

    /*
        4. Call with out of range numOfBuffers [0]
                     and other params from 1.
        Expected: NOT GT_OK.
    */
    numOfBuffers = 0;

    st = cpssBmPoolCreateDma(reqBufferSize, alignment, numOfBuffers, &pPoolId);
    UTF_VERIFY_NOT_EQUAL1_STRING_MAC(GT_OK, st, "numOfBuffers = %d", numOfBuffers);

    numOfBuffers = 10;

    /*
        5. Call with pPoolId [CPSS_BM_POOL_NULL_ID]
                     and other params from 1.
        Expected: GT_BAD_PTR.
    */
    st = cpssBmPoolCreateDma(reqBufferSize, alignment, numOfBuffers, CPSS_BM_POOL_NULL_ID);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_BAD_PTR, st, "%pPoolId = NULL");

    /*
        6. Call cpssBmPoolDelete with created poolId to delete created pool.
        Expected: GT_OK.
    */
    if (isCreated1)
    {
        st = cpssBmPoolDelete(pPoolId1);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssBmPoolDelete: %d", pPoolId1);
    }

    if (isCreated2)
    {
        st = cpssBmPoolDelete(pPoolId2);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssBmPoolDelete: %d", pPoolId2);
    }    
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssBmPoolReCreate
(
    IN  CPSS_BM_POOL_ID  poolId
)
*/
UTF_TEST_CASE_MAC(cpssBmPoolReCreate)
{
/*
    ITERATE_DEVICES
    1. Call cpssBmPoolCreate with reqBufferSize [4],
                                  alignment [CPSS_BM_POOL_1_BYTE_ALIGNMENT_E],
                                  numOfBuffers [2]
                                  and non-NULL pPoolId.
    Expected: GT_OK.
    2. Call with poolId of created pool.
    Expected: GT_OK.
    3. Call with pPoolId [CPSS_BM_POOL_NULL_ID].
    Expected: GT_BAD_VALUE.
    4. Call cpssBmPoolDelete with created poolId to delete created pool.
    Expected: GT_OK.
*/
    GT_STATUS   st = GT_OK;
    
    GT_U32                      reqBufferSize = 0;
    CPSS_BM_POOL_ALIGNMENT_ENT  alignment     = CPSS_BM_POOL_1_BYTE_ALIGNMENT_E;
    GT_U32                      numOfBuffers  = 0;
    CPSS_BM_POOL_ID             pPoolId, pCreatedPoolId;
    GT_BOOL                     isCreated     = GT_FALSE;


    /*
        1. Call cpssBmPoolCreate with reqBufferSize [4],
                                      alignment [CPSS_BM_POOL_1_BYTE_ALIGNMENT_E],
                                      numOfBuffers [2]
                                      and non-NULL pPoolId.
        Expected: GT_OK.
    */
    reqBufferSize = REQBUFFERSIZE_MIN;
    alignment     = CPSS_BM_POOL_1_BYTE_ALIGNMENT_E;
    numOfBuffers  = 2;

    st = cpssBmPoolCreate(reqBufferSize, alignment, numOfBuffers, &pPoolId);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssBmPoolCreate: %d, %d, %d",
                                 reqBufferSize, alignment, numOfBuffers);

    isCreated = (GT_OK == st) ? GT_TRUE : GT_FALSE;

    pCreatedPoolId = pPoolId;

    /*
        2. Call with poolId of created pool.
        Expected: GT_OK.
    */
    st = cpssBmPoolReCreate(pPoolId);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, pPoolId);

    /*
        3. Call with pPoolId [CPSS_BM_POOL_NULL_ID].
        Expected: GT_BAD_VALUE.
    */
    st = cpssBmPoolReCreate(CPSS_BM_POOL_NULL_ID);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_BAD_VALUE, st, "pPoolId = NULL");

    /*
        4. Call cpssBmPoolDelete with created poolId to delete created pool.
        Expected: GT_OK.
    */
    if (isCreated)
    {
        st = cpssBmPoolDelete(pCreatedPoolId);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssBmPoolDelete: %d", pCreatedPoolId);
    }
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssBmPoolDelete
(
    IN  CPSS_BM_POOL_ID  poolId
)
*/
UTF_TEST_CASE_MAC(cpssBmPoolDelete)
{
/*
    ITERATE_DEVICES
    1. Call with cpssBmPoolReCreate reqBufferSize [4],
                                    alignment [CPSS_BM_POOL_1_BYTE_ALIGNMENT_E],
                                    numOfBuffers [1]
                                    and non-NULL pPoolId.
    Expected: GT_OK.
    2. Call with poolId of created pool.
    Expected: GT_OK.
    3. Call with pPoolId [CPSS_BM_POOL_NULL_ID].
    Expected: GT_NOT_INITIALIZED.
*/
    GT_STATUS   st = GT_OK;
    
    GT_U32                      reqBufferSize = 0;
    CPSS_BM_POOL_ALIGNMENT_ENT  alignment     = CPSS_BM_POOL_1_BYTE_ALIGNMENT_E;
    GT_U32                      numOfBuffers  = 0;
    CPSS_BM_POOL_ID             pPoolId;


    /*
        1. Call cpssBmPoolCreate with reqBufferSize [4],
                                      alignment [CPSS_BM_POOL_1_BYTE_ALIGNMENT_E],
                                      numOfBuffers [2]
                                      and non-NULL pPoolId.
        Expected: GT_OK.
    */
    reqBufferSize = REQBUFFERSIZE_MIN;
    alignment     = CPSS_BM_POOL_1_BYTE_ALIGNMENT_E;
    numOfBuffers  = 2;

    st = cpssBmPoolCreate(reqBufferSize, alignment, numOfBuffers, &pPoolId);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssBmPoolCreate: %d, %d, %d",
                                 reqBufferSize, alignment, numOfBuffers);

    if (GT_OK == st)
    {
        /*
            2. Call with poolId of created pool.
            Expected: GT_OK.
        */
        st = cpssBmPoolDelete(pPoolId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, pPoolId);
    }
    
    /*
        3. Call with pPoolId [CPSS_BM_POOL_NULL_ID].
        Expected: GT_NOT_INITIALIZED.
    */
    st = cpssBmPoolDelete(CPSS_BM_POOL_NULL_ID);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_NOT_INITIALIZED, st, "pPoolId = NULL");
}

/*----------------------------------------------------------------------------*/
/*
GT_VOID *cpssBmPoolBufGet
(
    IN  CPSS_BM_POOL_ID poolId
)
*/
UTF_TEST_CASE_MAC(cpssBmPoolBufGet)
{
/*
    ITERATE_DEVICES
    1. Call cpssBmPoolCreate with reqBufferSize [4],
                                  alignment [CPSS_BM_POOL_1_BYTE_ALIGNMENT_E],
                                  numOfBuffers [2]
                                  and non-NULL pPoolId.
    Expected: GT_OK.
    2. Call with poolId of created pool.
    Expected: non NULL.
    3. Call with pPoolId [CPSS_BM_POOL_NULL_ID].
    Expected: GT_BAD_PTR.
    4. Call cpssBmPoolBufFree with created poolId and pBuf to free buffer.
    Expected: GT_OK.
    5. Call cpssBmPoolDelete with created poolId to delete created pool.
    Expected: GT_OK.
*/
    GT_STATUS   st = GT_OK;
    
    GT_U32                      reqBufferSize = 0;
    CPSS_BM_POOL_ALIGNMENT_ENT  alignment     = CPSS_BM_POOL_1_BYTE_ALIGNMENT_E;
    GT_U32                      numOfBuffers  = 0;
    CPSS_BM_POOL_ID             pPoolId;
    GT_BOOL                     isCreated     = GT_FALSE;
    GT_VOID*                    pBuf          = NULL;


    /*
        1. Call cpssBmPoolCreate with reqBufferSize [4],
                                      alignment [CPSS_BM_POOL_1_BYTE_ALIGNMENT_E],
                                      numOfBuffers [2]
                                      and non-NULL pPoolId.
        Expected: GT_OK.
    */
    reqBufferSize = REQBUFFERSIZE_MIN;
    alignment     = CPSS_BM_POOL_1_BYTE_ALIGNMENT_E;
    numOfBuffers  = 2;

    st = cpssBmPoolCreate(reqBufferSize, alignment, numOfBuffers, &pPoolId);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssBmPoolCreate: %d, %d, %d",
                                 reqBufferSize, alignment, numOfBuffers);

    isCreated = (GT_OK == st) ? GT_TRUE : GT_FALSE;

    /*
        2. Call with poolId of created pool.
        Expected: GT_OK.
    */
    pBuf = cpssBmPoolBufGet(pPoolId);
    UTF_VERIFY_NOT_EQUAL1_PARAM_MAC((GT_UINTPTR) NULL, (GT_UINTPTR) pBuf, pPoolId);

    /*
        4. Call cpssBmPoolBufFree with created poolId and pBuf to free buffer.
        Expected: GT_OK.
    */
    st = cpssBmPoolBufFree(pPoolId, pBuf);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssBmPoolBufFree: %d", pPoolId);
    
    /*
        5. Call cpssBmPoolDelete with created poolId to delete created pool.
        Expected: GT_OK.
    */
    if (isCreated)
    {
        st = cpssBmPoolDelete(pPoolId);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssBmPoolDelete: %d", pPoolId);
    }
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssBmPoolBufFree
(
    IN  CPSS_BM_POOL_ID poolId,
    IN  GT_VOID   *pBuf
)
*/
UTF_TEST_CASE_MAC(cpssBmPoolBufFree)
{
/*
    ITERATE_DEVICES
    1. Call cpssBmPoolCreate with reqBufferSize [4],
                                  alignment [CPSS_BM_POOL_1_BYTE_ALIGNMENT_E],
                                  numOfBuffers [2]
                                  and non-NULL pPoolId.
    Expected: GT_OK.
    2. Call cpssBmPoolBufGet two times with created poolId to get buffers.
    Expected: not NULL buffers.
    3. Call with created poolId and pBuf to free buffer.
    Expected: GT_OK.
    4. Call cpssBmPoolDelete with created poolId to delete created pool.
    Expected: GT_BAD_STATE (not all buffers are free).
    5. Call with created poolId and pBuf to free buffer.
    Expected: GT_OK.
    6. Call with pPoolId [CPSS_BM_POOL_NULL_ID].
    Expected: GT_NOT_INITIALIZED.
    7. Call with pBuf [CPSS_BM_POOL_NULL_ID].
    Expected: GT_BAD_PTR.
    8. Call cpssBmPoolDelete with created poolId to delete created pool.
    Expected: GT_OK.
*/
    GT_STATUS   st = GT_OK;
    
    GT_U32                      reqBufferSize = 0;
    CPSS_BM_POOL_ALIGNMENT_ENT  alignment     = CPSS_BM_POOL_1_BYTE_ALIGNMENT_E;
    GT_U32                      numOfBuffers  = 0;
    CPSS_BM_POOL_ID             pPoolId;
    GT_BOOL                     isCreated     = GT_FALSE;
    GT_VOID*                    pBuf1         = NULL;
    GT_VOID*                    pBuf2         = NULL;


    /*
        1. Call cpssBmPoolCreate with reqBufferSize [4],
                                      alignment [CPSS_BM_POOL_1_BYTE_ALIGNMENT_E],
                                      numOfBuffers [2]
                                      and non-NULL pPoolId.
        Expected: GT_OK.
    */
    reqBufferSize = REQBUFFERSIZE_MIN;
    alignment     = CPSS_BM_POOL_1_BYTE_ALIGNMENT_E;
    numOfBuffers  = 2;

    st = cpssBmPoolCreate(reqBufferSize, alignment, numOfBuffers, &pPoolId);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssBmPoolCreate: %d, %d, %d",
                                 reqBufferSize, alignment, numOfBuffers);

    isCreated = (GT_OK == st) ? GT_TRUE : GT_FALSE;

    /*
        2. Call cpssBmPoolBufGet two times with created poolId to get buffers.
        Expected: not NULL buffers.
    */
    pBuf1 = cpssBmPoolBufGet(pPoolId);
    UTF_VERIFY_NOT_EQUAL0_STRING_MAC((GT_UINTPTR) NULL, (GT_UINTPTR) pBuf1, "cpssBmPoolBufGet");

    pBuf2 = cpssBmPoolBufGet(pPoolId);
    UTF_VERIFY_NOT_EQUAL0_STRING_MAC((GT_UINTPTR) NULL, (GT_UINTPTR) pBuf2, "cpssBmPoolBufGet");

    /*
        3. Call with created poolId and pBuf to free buffer.
        Expected: GT_OK.
    */
    st = cpssBmPoolBufFree(pPoolId, pBuf1);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, pPoolId);

    /*
        4. Call cpssBmPoolDelete with created poolId to delete created pool.
        Expected: GT_BAD_STATE (not all buffers are free).
    */
    st = cpssBmPoolDelete(pPoolId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_STATE, st, "cpssBmPoolDelete: %d", pPoolId);

    /*
        5. Call cpssBmPoolBufFree with created poolId and pBuf to free buffer.
        Expected: GT_OK.
    */
    st = cpssBmPoolBufFree(pPoolId, pBuf2);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, pPoolId);

    /*
        6. Call with pPoolId [CPSS_BM_POOL_NULL_ID].
        Expected: GT_NOT_INITIALIZED.
    */
    st = cpssBmPoolBufFree(CPSS_BM_POOL_NULL_ID, pBuf1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_NOT_INITIALIZED, st, "pPoolId = NULL");

    /*
        7. Call with pBuf [NULL].
        Expected: GT_BAD_PTR.
    */
    st = cpssBmPoolBufFree(pPoolId, NULL);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_BAD_PTR, st, "pBuf = NULL");

    /*
        8. Call cpssBmPoolDelete with created poolId to delete created pool.
        Expected: GT_OK.
    */
    if (isCreated)
    {
        st = cpssBmPoolDelete(pPoolId);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssBmPoolDelete: %d", pPoolId);
    }
}

/*----------------------------------------------------------------------------*/
/*
GT_U32 cpssBmPoolBufSizeGet
(
    IN        CPSS_BM_POOL_ID  poolId
)
*/
UTF_TEST_CASE_MAC(cpssBmPoolBufSizeGet)
{
/*
    ITERATE_DEVICES
    1. Call cpssBmPoolCreate with reqBufferSize [4],
                                  alignment [CPSS_BM_POOL_1_BYTE_ALIGNMENT_E],
                                  numOfBuffers [2]
                                  and non-NULL pPoolId.
    Expected: GT_OK.
    2. Call with created poolId.
    Expected: same size as was created.
    3. Call with pPoolId [CPSS_BM_POOL_NULL_ID].
    Expected: GT_NOT_INITIALIZED.
    4. Call cpssBmPoolDelete with created poolId to delete created pool.
    Expected: GT_OK.
*/
    GT_STATUS   st = GT_OK;
    
    GT_U32                      reqBufferSize = 0;
    GT_U32                      bufSizeGet    = 0;
    CPSS_BM_POOL_ALIGNMENT_ENT  alignment     = CPSS_BM_POOL_1_BYTE_ALIGNMENT_E;
    GT_U32                      numOfBuffers  = 0;
    CPSS_BM_POOL_ID             pPoolId;
    GT_BOOL                     isCreated     = GT_FALSE;


    /*
        1. Call cpssBmPoolCreate with reqBufferSize [4],
                                      alignment [CPSS_BM_POOL_1_BYTE_ALIGNMENT_E],
                                      numOfBuffers [2]
                                      and non-NULL pPoolId.
        Expected: GT_OK.
    */
    reqBufferSize = REQBUFFERSIZE_MIN;
    alignment     = CPSS_BM_POOL_1_BYTE_ALIGNMENT_E;
    numOfBuffers  = 2;

    st = cpssBmPoolCreate(reqBufferSize, alignment, numOfBuffers, &pPoolId);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssBmPoolCreate: %d, %d, %d",
                                 reqBufferSize, alignment, numOfBuffers);

    isCreated = (GT_OK == st) ? GT_TRUE : GT_FALSE;

    /*
        2. Call with created poolId.
        Expected: same size as was created.
    */
    bufSizeGet = cpssBmPoolBufSizeGet(pPoolId);
    UTF_VERIFY_EQUAL1_STRING_MAC(reqBufferSize, bufSizeGet,
                       "get another reqBufferSize than was set: %d", pPoolId);

    /*
        3. Call with pPoolId [CPSS_BM_POOL_NULL_ID].
        Expected: GT_NOT_INITIALIZED.
    */
    bufSizeGet = cpssBmPoolBufSizeGet(CPSS_BM_POOL_NULL_ID);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_NOT_INITIALIZED, bufSizeGet, "pPoolId = NULL");

    /*
        4. Call cpssBmPoolDelete with created poolId to delete created pool.
        Expected: GT_OK.
    */
    if (isCreated)
    {
        st = cpssBmPoolDelete(pPoolId);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssBmPoolDelete: %d", pPoolId);
    }
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssBmPoolExpand
(
    IN   CPSS_BM_POOL_ID        poolId,
    IN   GT_U32            numOfBuffers
)
*/
UTF_TEST_CASE_MAC(cpssBmPoolExpand)
{
/*
    ITERATE_DEVICES
    1. Call cpssBmPoolCreate with reqBufferSize [4],
                                  alignment [CPSS_BM_POOL_1_BYTE_ALIGNMENT_E],
                                  numOfBuffers [2]
                                  and non-NULL pPoolId.
    Expected: GT_OK.
    2. Call with created poolId and numOfBuffers [10].
    Expected: GT_OK and numOfBuffers = 10 + 2.
    3. Call cpssBmPoolBufFreeCntGet with created poolId.
    Expected: numOfBuffers = 10 + 2.
    4. Call with pPoolId [CPSS_BM_POOL_NULL_ID].
    Expected: GT_NOT_INITIALIZED.
    5. Call with out of range numOfBuffers [0]
                 and other params from 1.
    Expected: GT_BAD_VALUE.
    6. Call cpssBmPoolDelete with created poolId to delete created pool.
    Expected: GT_OK.
*/
    GT_STATUS   st = GT_OK;
    
    GT_U32                      reqBufferSize = 0;
    CPSS_BM_POOL_ALIGNMENT_ENT  alignment     = CPSS_BM_POOL_1_BYTE_ALIGNMENT_E;
    GT_U32                      numOfBuffers  = 0;
    GT_U32                      numOfBufAdded = 0;
    GT_U32                      numOfBufGet   = 0;
    CPSS_BM_POOL_ID             pPoolId;
    GT_BOOL                     isCreated     = GT_FALSE;


    /*
        1. Call cpssBmPoolCreate with reqBufferSize [4],
                                      alignment [CPSS_BM_POOL_1_BYTE_ALIGNMENT_E],
                                      numOfBuffers [2]
                                      and non-NULL pPoolId.
        Expected: GT_OK.
    */
    reqBufferSize = REQBUFFERSIZE_MIN;
    alignment     = CPSS_BM_POOL_1_BYTE_ALIGNMENT_E;
    numOfBuffers  = 2;

    st = cpssBmPoolCreate(reqBufferSize, alignment, numOfBuffers, &pPoolId);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssBmPoolCreate: %d, %d, %d",
                                 reqBufferSize, alignment, numOfBuffers);

    isCreated = (GT_OK == st) ? GT_TRUE : GT_FALSE;

    /*
        2. Call with created poolId and numOfBuffers [10].
        Expected: GT_OK and numOfBuffers = 10 + 2.
    */
    numOfBufAdded = 10;

    st = cpssBmPoolExpand(pPoolId, numOfBufAdded);
    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, pPoolId, numOfBufAdded);

    /*
        3. Call cpssBmPoolBufFreeCntGet with created poolId.
        Expected: numOfBuffers = 10 + 2.
    */
    numOfBufGet = cpssBmPoolBufFreeCntGet(pPoolId);
    UTF_VERIFY_EQUAL1_STRING_MAC(numOfBuffers + numOfBufAdded, numOfBufGet,
                       "get another numOfBuffers than was set: %d", pPoolId);

    /*
        4. Call with pPoolId [CPSS_BM_POOL_NULL_ID].
        Expected: GT_NOT_INITIALIZED.
    */
    st = cpssBmPoolExpand(CPSS_BM_POOL_NULL_ID, numOfBufAdded);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_NOT_INITIALIZED, st, "pPoolId = NULL");

    /*
        5. Call with out of range numOfBuffers [0]
                     and other params from 1.
        Expected: GT_BAD_VALUE.
    */
    numOfBufAdded = 0;

    st = cpssBmPoolExpand(pPoolId, numOfBufAdded);
    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_VALUE, st, pPoolId, numOfBufAdded);

    /*
        6. Call cpssBmPoolDelete with created poolId to delete created pool.
        Expected: GT_OK.
    */
    if (isCreated)
    {
        st = cpssBmPoolDelete(pPoolId);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssBmPoolDelete: %d", pPoolId);
    }
}

/*----------------------------------------------------------------------------*/
/*
GT_U32 cpssBmPoolBufFreeCntGet
(
    IN        CPSS_BM_POOL_ID  poolId
)
*/
UTF_TEST_CASE_MAC(cpssBmPoolBufFreeCntGet)
{
/*
    ITERATE_DEVICES
    1. Call cpssBmPoolCreate with reqBufferSize [4],
                                  alignment [CPSS_BM_POOL_1_BYTE_ALIGNMENT_E],
                                  numOfBuffers [2]
                                  and non-NULL pPoolId.
    Expected: GT_OK.
    2. Call with created poolId.
    Expected: same numOfBuffers as was created.
    3. Call cpssBmPoolBufGet two times with created poolId to get buffers.
    Expected: not NULL buffers.
    4. Call with created poolId.
    Expected: numOfBuffers - 1.
    5. Call with pPoolId [CPSS_BM_POOL_NULL_ID].
    Expected: GT_NOT_INITIALIZED.
    6. Call cpssBmPoolBufFree with created poolId and pBuf to free buffer.
    Expected: GT_OK.
    7. Call cpssBmPoolDelete with created poolId to delete created pool.
    Expected: GT_OK.
*/
    GT_STATUS   st = GT_OK;
    
    GT_U32                      reqBufferSize = 0;
    CPSS_BM_POOL_ALIGNMENT_ENT  alignment     = CPSS_BM_POOL_1_BYTE_ALIGNMENT_E;
    GT_U32                      numOfBuffers  = 0;
    GT_U32                      numOfBufGet   = 0;
    CPSS_BM_POOL_ID             pPoolId;
    GT_BOOL                     isCreated     = GT_FALSE;
    GT_VOID*                    pBuf          = NULL;


    /*
        1. Call cpssBmPoolCreate with reqBufferSize [4],
                                      alignment [CPSS_BM_POOL_1_BYTE_ALIGNMENT_E],
                                      numOfBuffers [2]
                                      and non-NULL pPoolId.
        Expected: GT_OK.
    */
    reqBufferSize = REQBUFFERSIZE_MIN;
    alignment     = CPSS_BM_POOL_1_BYTE_ALIGNMENT_E;
    numOfBuffers  = 2;

    st = cpssBmPoolCreate(reqBufferSize, alignment, numOfBuffers, &pPoolId);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssBmPoolCreate: %d, %d, %d",
                                 reqBufferSize, alignment, numOfBuffers);

    isCreated = (GT_OK == st) ? GT_TRUE : GT_FALSE;

    /*
        2. Call with created poolId.
        Expected: same numOfBuffers as was created.
    */
    numOfBufGet = cpssBmPoolBufFreeCntGet(pPoolId);
    UTF_VERIFY_EQUAL1_STRING_MAC(numOfBuffers, numOfBufGet,
                       "get another numOfBuffers than was set: %d", pPoolId);

    /*
        3. Call cpssBmPoolBufGet two times with created poolId to get buffers.
        Expected: not NULL buffers.
    */
    pBuf = cpssBmPoolBufGet(pPoolId);
    UTF_VERIFY_NOT_EQUAL1_PARAM_MAC((GT_UINTPTR) NULL, (GT_UINTPTR) pBuf, pPoolId);

    /*
        4. Call with created poolId.
        Expected: numOfBuffers - 1.
    */
    numOfBufGet = cpssBmPoolBufFreeCntGet(pPoolId);
    UTF_VERIFY_EQUAL1_STRING_MAC(numOfBuffers - 1, numOfBufGet,
                       "get another numOfBuffers than was set: %d", pPoolId);

    /*
        5. Call with pPoolId [CPSS_BM_POOL_NULL_ID].
        Expected: GT_NOT_INITIALIZED.
    */
    numOfBufGet = cpssBmPoolBufFreeCntGet(CPSS_BM_POOL_NULL_ID);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_NOT_INITIALIZED, numOfBufGet, "pPoolId = NULL");

    /*
        6. Call cpssBmPoolBufFree with created poolId and pBuf to free buffer.
        Expected: GT_OK.
    */
    st = cpssBmPoolBufFree(pPoolId, pBuf);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssBmPoolBufFree: %d", pPoolId);

    /*
        7. Call cpssBmPoolDelete with created poolId to delete created pool.
        Expected: GT_OK.
    */
    if (isCreated)
    {
        st = cpssBmPoolDelete(pPoolId);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssBmPoolDelete: %d", pPoolId);
    }
}


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

/*
 * Configuration of cpssBuffManagerPool suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssBuffManagerPool)
    UTF_SUIT_DECLARE_TEST_MAC(cpssBmPoolCreate)
    UTF_SUIT_DECLARE_TEST_MAC(cpssBmPoolCreateDma)
    UTF_SUIT_DECLARE_TEST_MAC(cpssBmPoolReCreate)
    UTF_SUIT_DECLARE_TEST_MAC(cpssBmPoolDelete)
    UTF_SUIT_DECLARE_TEST_MAC(cpssBmPoolBufGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssBmPoolBufFree)
    UTF_SUIT_DECLARE_TEST_MAC(cpssBmPoolBufSizeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssBmPoolExpand)
    UTF_SUIT_DECLARE_TEST_MAC(cpssBmPoolBufFreeCntGet)
UTF_SUIT_END_TESTS_MAC(cpssBuffManagerPool)

