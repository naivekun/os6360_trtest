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
* @file cpssPxDiagBistUT.c
*
* @brief Unit tests for cpssPxDiagBistUT, that provides
* CPSS Pipe BIST related API
*
* @version   1
********************************************************************************
*/
/* includes */
/* the define of UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC must come from C files that
   already fixed the types of ports from GT_U8 !

   NOTE: must come before ANY include to H files !!!!
*/
#define UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC

#include <cpss/px/diag/cpssPxDiag.h>
#include <cpss/px/diag/private/prvCpssPxDiagDataIntegrityMainMappingDb.h>
#include <cpss/px/diag/private/prvCpssPxDiag.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxDiagBistResultsGet
(
    IN  GT_U8                                       devNum,
    OUT CPSS_PX_DIAG_BIST_STATUS_ENT                *resultsStatusPtr,
    OUT CPSS_PX_DIAG_BIST_RESULT_STC                resultsArr[],
    INOUT GT_U32                                    *resultsNumPtr
)
*/
UTF_TEST_CASE_MAC(cpssPxDiagBistResultsGet)
{
/*
    ITERATE_DEVICES (Pipe)
    1.1. Call with non-NULL pointers.
    Expected: GT_OK.
    1.2. Call with resultsStatusPtr [NULL], other params same as in 1.1.
    Expected: GT_BAD_PTR.
    1.3. Call with resultsArr [NULL], other params same as in 1.1.
    Expected: GT_BAD_PTR.
    1.4. Call with resultsNumPtr [NULL], other params same as in 1.1.
    Expected: GT_BAD_PTR.

*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    CPSS_PX_DIAG_BIST_STATUS_ENT    resultsStatus;
    CPSS_PX_DIAG_BIST_RESULT_STC    results;
    GT_U32                          resultsNum;


    cpssOsBzero((GT_VOID*)&results, sizeof(results));

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with non-NULL pointers.
            Expected: GT_OK.
        */
        resultsNum = 0;
        st = cpssPxDiagBistResultsGet(dev, &resultsStatus, &results, &resultsNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with resultsStatusPtr [NULL], other params same as in 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssPxDiagBistResultsGet(dev, NULL, &results, &resultsNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /*
            1.3. Call with resultsArr [NULL], other params same as in 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssPxDiagBistResultsGet(dev, &resultsStatus, NULL, &resultsNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /*
            1.4. Call with resultsNumPtr [NULL], other params same as in 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssPxDiagBistResultsGet(dev, &resultsStatus, &results, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxDiagBistResultsGet(dev, &resultsStatus, &results, &resultsNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxDiagBistResultsGet(dev, &resultsStatus, &results, &resultsNum);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxDiagBistTriggerAllSet
(
    IN  GT_U8                                       devNum
)
*/
UTF_TEST_CASE_MAC(cpssPxDiagBistTriggerAllSet)
{
/*
    ITERATE_DEVICES (Pipe)
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    CPSS_PX_DIAG_BIST_STATUS_ENT        resultsStatus;
    CPSS_PX_DIAG_BIST_RESULT_STC        results;
    GT_U32                              resultsNum = 1;
    CPSS_PX_DIAG_BIST_STATUS_ENT        expectedStatus;


    cpssOsBzero((GT_VOID*)&results, sizeof(results));

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* Mask data integrity events caused by BIST */
        st = cpssEventDeviceMaskSet(dev,
                                    CPSS_PP_DATA_INTEGRITY_ERROR_E,
                                    CPSS_EVENT_MASK_E);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            Do BIST.
            Expected: GT_OK.
        */
        st = cpssPxDiagBistTriggerAllSet(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            Check BIST results.
            Expected: GT_OK, and BIST pass.
        */
        cpssOsTimerWkAfter(50);

        st = cpssPxDiagBistResultsGet(dev, &resultsStatus, &results, &resultsNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

#ifndef ASIC_SIMULATION
        expectedStatus = CPSS_PX_DIAG_BIST_STATUS_PASS_E;
#else /* ASIC_SIMULATION */
        if (GT_TRUE == prvUtfIsGmCompilation())
        {
            expectedStatus = CPSS_PX_DIAG_BIST_STATUS_PASS_E;
        }
        else
        {
            expectedStatus = CPSS_PX_DIAG_BIST_STATUS_NOT_READY_E;
        }
#endif /* ASIC_SIMULATION */

        UTF_VERIFY_EQUAL1_STRING_MAC(expectedStatus, resultsStatus,
                                     "got other BIST status: %d", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxDiagBistTriggerAllSet(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxDiagBistTriggerAllSet(dev);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxDiagBistTriggerAllSetWithError
(
    IN  GT_U8                                       devNum
)
*/
UTF_TEST_CASE_MAC(cpssPxDiagBistTriggerAllSetWithError)
{
/*
    ITERATE_DEVICES (Pipe)
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    GT_U32  pipe;
    GT_U32  client;
    GT_U32  ram;
    GT_U32  ii;

    CPSS_PX_DIAG_BIST_STATUS_ENT    resultsStatus;
    CPSS_PX_DIAG_BIST_RESULT_STC    results;
    GT_U32                          resultsNum;
    PRV_CPSS_DFX_DIAG_DATA_INTEGRITY_MAP_DB_STC    *dbArrayPtr; /* pointer to data integrity DB */
    GT_U32                                          dbArrayEntryNum; /* size of data integrity DB */
    GT_U32                          counter;
#ifdef ASIC_SIMULATION
    CPSS_PX_DIAG_BIST_STATUS_ENT    expectedStatus;
#endif /* ASIC_SIMULATION */

    cpssOsBzero((GT_VOID*)&results, sizeof(results));

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* Mask data integrity events caused by BIST */
        st = cpssEventDeviceMaskSet(dev,
                                    CPSS_PP_DATA_INTEGRITY_ERROR_E,
                                    CPSS_EVENT_MASK_E);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* assign DB pointer and size */
        prvCpssPxDiagDataIntegrityDbPointerSet(&dbArrayPtr, &dbArrayEntryNum);
        if ((dbArrayPtr == NULL) || (dbArrayEntryNum == 0))
        {
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_FAIL, st, dev);
            continue;
        }

        /* loop over all memories and inject errors */
        for( ii = 0 ; ii < dbArrayEntryNum ; ii++ )
        {
            pipe = dbArrayPtr[ii].key/4096;
            client = ((dbArrayPtr[ii].key)%4096)/128;

            if (prvCpssPxDiagBistCheckSkipOptionalClient(pipe, client, GT_TRUE))
                continue;

            ram = ((dbArrayPtr[ii].key)%4096)%128;

          /*  PRV_UTF_LOG3_MAC("Set inject erroe for memory %d %d %d\n", pipe, client, ram); */

            st = prvCpssDfxMemoryErrorInjectionSet(dev, pipe, client, ram,
                 CPSS_DIAG_DATA_INTEGRITY_ERROR_INJECT_MODE_SINGLE_E ,GT_TRUE);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
                Do BIST.
                Expected: GT_OK.
            */
            st = cpssPxDiagBistTriggerAllSet(dev);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*PRV_UTF_LOG3_MAC("BIST done for memory %d %d %d\n", pipe, client, ram); */

            /*
                Check BIST results.
                Expected: GT_OK, and BIST fail with correct RAM indication.
            */

            /* 100 milliseconds is enough for poling */
            #ifdef ASIC_SIMULATION
            counter = 1;
            #else
            counter = 100;
            #endif

            resultsStatus = CPSS_PX_DIAG_BIST_STATUS_NOT_READY_E;

            while ((counter--) && (resultsStatus == CPSS_PX_DIAG_BIST_STATUS_NOT_READY_E))
            {
                cpssOsTimerWkAfter(1);

                resultsNum = 1;
                st = cpssPxDiagBistResultsGet(dev, &resultsStatus, &results, &resultsNum);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
            }
#ifndef ASIC_SIMULATION
            UTF_VERIFY_EQUAL2_STRING_MAC(CPSS_PX_DIAG_BIST_STATUS_FAIL_E,
                                         resultsStatus,
                                         "got other BIST status: %d, %d", ii, dev);
            UTF_VERIFY_EQUAL2_STRING_MAC(1, resultsNum,
                                         "got other BIST error number: %d, %d", ii, dev);
            UTF_VERIFY_EQUAL2_STRING_MAC(pipe, results.location.dfxPipeId,
                                         "got other pipeId: %d, %d", ii, dev);
            UTF_VERIFY_EQUAL2_STRING_MAC(client, results.location.dfxClientId,
                                         "got other clientId: %d, %d", ii, dev);
            UTF_VERIFY_EQUAL2_STRING_MAC(ram, results.location.dfxMemoryId,
                                         "got other memoryId: %d, %d", ii, dev);
            UTF_VERIFY_EQUAL2_STRING_MAC(dbArrayPtr[ii].memType,
                                         results.memType,
                                         "got other memory type: %d, %d", ii, dev);
#else /* ASIC_SIMULATION */
            if (GT_TRUE == prvUtfIsGmCompilation())
            {
                expectedStatus = CPSS_PX_DIAG_BIST_STATUS_PASS_E;
            }
            else
            {
                expectedStatus = CPSS_PX_DIAG_BIST_STATUS_NOT_READY_E;
            }

            UTF_VERIFY_EQUAL1_STRING_MAC(expectedStatus, resultsStatus,
                                     "got other BIST status: %d", dev);
#endif /* ASIC_SIMULATION */

            /*PRV_UTF_LOG3_MAC("Stop error for memory %d %d %d\n", pipe, client, ram); */
            st = prvCpssDfxMemoryErrorInjectionSet(dev, pipe, client, ram,
                 CPSS_DIAG_DATA_INTEGRITY_ERROR_INJECT_MODE_SINGLE_E ,GT_FALSE);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        }
    }
}

/*----------------------------------------------------------------------------*/
/*
 * Configuration of cpssPxDiagBist suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssPxDiagBist)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxDiagBistResultsGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxDiagBistTriggerAllSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxDiagBistTriggerAllSetWithError)

UTF_SUIT_END_TESTS_MAC(cpssPxDiagBist)

