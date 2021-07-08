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
* @file cpssDxChDiagBistUT.c
*
* @brief Unit tests for cpssDxChDiagBistUT, that provides
* CPSS DXCH BIST related API
*
* @version   5
********************************************************************************
*/
/* includes */
/* the define of UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC must come from C files that
   already fixed the types of ports from GT_U8 !

   NOTE: must come before ANY include to H files !!!!
*/
#define UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC

#include <cpss/dxCh/dxChxGen/diag/cpssDxChDiag.h>
#include <cpss/dxCh/dxChxGen/diag/private/prvCpssDxChDiag.h>
#include <cpss/dxCh/dxChxGen/diag/private/prvCpssDxChDiagDataIntegrityMainMappingDb.h>
#include <cpss/dxCh/dxChxGen/tmGlue/cpssDxChTmGlueDram.h>
#include <cpss/dxCh/dxChxGen/tmGlue/private/prvCpssDxChTmGlue.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChDiagBistResultsGet
(
    IN  GT_U8                                       devNum,
    OUT CPSS_DXCH_DIAG_BIST_STATUS_ENT              *resultsStatusPtr,
    OUT CPSS_DXCH_DIAG_BIST_RESULT_STC              resultsArr[],
    INOUT GT_U32                                    *resultsNumPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChDiagBistResultsGet)
{
/*
    ITERATE_DEVICES (Bobcat2, Cetus, Aldrin, AC3X, Bobcat3)
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

    CPSS_DXCH_DIAG_BIST_STATUS_ENT  resultsStatus;
    CPSS_DXCH_DIAG_BIST_RESULT_STC  results;
    GT_U32                          resultsNum;


    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E);

    cpssOsBzero((GT_VOID*)&results, sizeof(results));

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with non-NULL pointers.
            Expected: GT_OK.
        */
        resultsNum = 0;
        st = cpssDxChDiagBistResultsGet(dev, &resultsStatus, &results, &resultsNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with resultsStatusPtr [NULL], other params same as in 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChDiagBistResultsGet(dev, NULL, &results, &resultsNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /*
            1.3. Call with resultsArr [NULL], other params same as in 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChDiagBistResultsGet(dev, &resultsStatus, NULL, &resultsNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /*
            1.4. Call with resultsNumPtr [NULL], other params same as in 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChDiagBistResultsGet(dev, &resultsStatus, &results, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChDiagBistResultsGet(dev, &resultsStatus, &results, &resultsNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChDiagBistResultsGet(dev, &resultsStatus, &results, &resultsNum);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChDiagBistTriggerAllSet
(
    IN  GT_U8                                       devNum
)
*/
UTF_TEST_CASE_MAC(cpssDxChDiagBistTriggerAllSet)
{
/*
    ITERATE_DEVICES (Bobcat2, Cetus, Aldrin, AC3X, Bobcat3)
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    CPSS_DXCH_DIAG_BIST_STATUS_ENT  resultsStatus;
    CPSS_DXCH_DIAG_BIST_RESULT_STC  results;
    GT_U32                          resultsNum = 1;

    CPSS_DXCH_DIAG_BIST_STATUS_ENT  expectedStatus;


    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E);
    cpssOsBzero((GT_VOID*)&results, sizeof(results));

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
        st = cpssDxChDiagBistTriggerAllSet(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            Check BIST results.
            Expected: GT_OK, and BIST pass.
        */
        cpssOsTimerWkAfter(50);

        st = cpssDxChDiagBistResultsGet(dev, &resultsStatus, &results, &resultsNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

#ifndef ASIC_SIMULATION
        expectedStatus = CPSS_DXCH_DIAG_BIST_STATUS_PASS_E;
#else /* ASIC_SIMULATION */
        if (GT_TRUE == prvUtfIsGmCompilation())
        {
            expectedStatus = CPSS_DXCH_DIAG_BIST_STATUS_PASS_E;
        }
        else
        {
            expectedStatus = CPSS_DXCH_DIAG_BIST_STATUS_NOT_READY_E;
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
        st = cpssDxChDiagBistTriggerAllSet(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChDiagBistTriggerAllSet(dev);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChDiagBistTriggerAllSet
(
    IN  GT_U8                                       devNum
)
*/
UTF_TEST_CASE_MAC(cpssDxChDiagBistTriggerAllSetWithError)
{
/*
    ITERATE_DEVICES (Bobcat2, Cetus, Aldrin, AC3X, Bobcat3)
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    GT_U32  pipe;
    GT_U32  client;
    GT_U32  ram;
    GT_U32  ii;

    CPSS_DXCH_DIAG_BIST_STATUS_ENT  resultsStatus;
    CPSS_DXCH_DIAG_BIST_RESULT_STC  results;
    GT_U32                          resultsNum;
    PRV_CPSS_DFX_DIAG_DATA_INTEGRITY_MAP_DB_STC     *dbArrayPtr; /* pointer to data integrity DB */
    GT_U32                                          dbArrayEntryNum; /* size of data integrity DB */
    GT_U32                          counter;
#ifdef ASIC_SIMULATION
    CPSS_DXCH_DIAG_BIST_STATUS_ENT  expectedStatus;
#endif /* ASIC_SIMULATION */

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E);

    cpssOsBzero((GT_VOID*)&results, sizeof(results));

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* Mask data integrity events caused by BIST */
        st = cpssEventDeviceMaskSet(dev,
                                    CPSS_PP_DATA_INTEGRITY_ERROR_E,
                                    CPSS_EVENT_MASK_E);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* assign DB pointer and size */
        prvCpssDxChDiagDataIntegrityDbPointerSet(dev, &dbArrayPtr, &dbArrayEntryNum);
        if ((dbArrayPtr == NULL) || (dbArrayEntryNum == 0))
        {
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_FAIL, st, dev);
            continue;
        }

        PRV_UTF_LOG1_MAC("BIST Start for table size %d\n", dbArrayEntryNum);

        /* loop over all memories and inject errors */
        for( ii = 0 ; ii < dbArrayEntryNum ; ii++ )
        {
            pipe = dbArrayPtr[ii].key/4096;
            client = ((dbArrayPtr[ii].key)%4096)/128;

            if (prvCpssDxChDiagBistCheckSkipClient(dev, pipe, client))
                continue;

          /*  PRV_UTF_LOG1_MAC("BIST Start for memory %d\n", dbArrayPtr[ii].key); */

            ram = ((dbArrayPtr[ii].key)%4096)%128;

            st = prvCpssDfxMemoryErrorInjectionSet(dev, pipe, client, ram,
                 CPSS_DIAG_DATA_INTEGRITY_ERROR_INJECT_MODE_SINGLE_E ,GT_TRUE);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
                Do BIST.
                Expected: GT_OK.
            */
            st = cpssDxChDiagBistTriggerAllSet(dev);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

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

            resultsStatus = CPSS_DXCH_DIAG_BIST_STATUS_NOT_READY_E;

            while ((counter--) && (resultsStatus == CPSS_DXCH_DIAG_BIST_STATUS_NOT_READY_E))
            {
                cpssOsTimerWkAfter(1);

                resultsNum = 1;
                st = cpssDxChDiagBistResultsGet(dev, &resultsStatus, &results, &resultsNum);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
            }
#ifndef ASIC_SIMULATION
            UTF_VERIFY_EQUAL2_STRING_MAC(CPSS_DXCH_DIAG_BIST_STATUS_FAIL_E,
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
                expectedStatus = CPSS_DXCH_DIAG_BIST_STATUS_PASS_E;
            }
            else
            {
                expectedStatus = CPSS_DXCH_DIAG_BIST_STATUS_NOT_READY_E;
            }

            UTF_VERIFY_EQUAL1_STRING_MAC(expectedStatus, resultsStatus,
                                     "got other BIST status: %d", dev);
#endif /* ASIC_SIMULATION */

            st = prvCpssDfxMemoryErrorInjectionSet(dev, pipe, client, ram,
                 CPSS_DIAG_DATA_INTEGRITY_ERROR_INJECT_MODE_SINGLE_E ,GT_FALSE);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        }

        PRV_UTF_LOG0_MAC("BIST Done!\n");

        if(!prvUtfIsGmCompilation())
        {
            /* after messing with the memories ... synch shadow to HW state */
            prvCpssDxChDiagDataIntegrityShadowAndHwSynch(dev);
            PRV_UTF_LOG0_MAC("BIST Shadow synch Done!\n");
        }
    }
}

/* Skip this test for GM - performance reason */
/*    GM_SKIP_TEST_DUE_TO_POOR_PERFORMANCE_MAC;*/

/*
GT_STATUS    cpssDxChDiagExternalMemoriesBistRun
(
        IN  GT_U8                                              devNum,
    IN  GT_U32                                       extMemoBitmap,
    IN  GT_BOOL                                      testWholeMemory,
    IN  GT_U32                                       testedAreaOffset,
    IN  GT_U32                                       testedAreaLength,
    IN  CPSS_DXCH_DIAG_EXT_MEMORY_BIST_PATTERN_ENT   pattern,
    OUT GT_BOOL                                      *testStatusPtr,
    OUT CPSS_DXCH_DIAG_EXT_MEMORY_INF_BIST_ERROR_STC errorInfoArr[]
)
*/
UTF_TEST_CASE_MAC(cpssDxChDiagExternalMemoriesBistRun)
{

#ifndef INCLUDE_TM
    SKIP_TEST_MAC;
#else

#ifdef ASIC_SIMULATION
    SKIP_TEST_MAC;
#else
/*
    ITERATE_DEVICES (Bobcat2)
*/
    GT_STATUS                                    rc = GT_OK;
    GT_U8                                        dev;
    GT_BOOL                                      testStatus;
    CPSS_DXCH_DIAG_EXT_MEMORY_INF_BIST_ERROR_STC errorInfoArr[5];
    GT_U32                                       extMemoBitmap = 0x1F;
    GT_U32                                       i;
    /* function declaration */
    GT_STATUS appDemoBc2DramOrTmInit
    (
        IN  GT_U8       dev,
        IN  GT_U32      flags,
        OUT GT_U32     *dramIfBmpPtr
    );

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E | UTF_ALDRIN_E | UTF_AC3X_E);

    /* test stuck CPU - need to fix it in future */
    UTF_HW_CRASHED_TEST_BYPASS_MAC(UTF_CAELUM_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        if (PRV_CPSS_IS_DEV_EXISTS_MAC(dev) == 0)
        {
            continue;
        }

        if(PRV_CPSS_DXCH_PP_MAC(dev)->fineTuning.featureInfo.TmSupported == GT_FALSE)
        {
            /* device has TM disabled - skip test */
            SKIP_TEST_MAC;
        }

        if ((PRV_CPSS_DXCH_PP_MAC(dev)->extMemory.externalMemoryInitFlags
             & PRV_CPSS_DXCH_TM_GLUE_DRAM_INIT_DONE_TM_CNS) != 0)
        {
            /* already initialized for TM, incompatible with BIST */
            SKIP_TEST_MAC;
        }

        cpssOsMemSet(errorInfoArr, 0, sizeof(errorInfoArr));

        if ((PRV_CPSS_DXCH_PP_MAC(dev)->extMemory.externalMemoryInitFlags
             & PRV_CPSS_DXCH_TM_GLUE_DRAM_INIT_DONE_DDR_PHY_CNS) == 0)
        {
            rc = appDemoBc2DramOrTmInit(
                dev, CPSS_DXCH_TM_GLUE_DRAM_INIT_DDR_PHY_CNS, &extMemoBitmap);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoBc2DramOrTmInit");
        }

        PRV_UTF_LOG1_MAC("Active DRAM interfaces 0x%X\n", extMemoBitmap);

        rc = cpssDxChDiagExternalMemoriesBistRun(
            dev, extMemoBitmap, GT_TRUE/*testWholeMemory*/,
            0/*testedAreaOffset*/, 0/*testedAreaLength*/,
            CPSS_DXCH_DIAG_EXT_MEMORY_BIST_PATTERN_BASIC_E,
            &testStatus, errorInfoArr);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChDiagExternalMemoriesBistRun");
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_TRUE, testStatus, "Bist status");
        PRV_UTF_LOG1_MAC(
            "cpssDxChDiagExternalMemoriesBistRun testStatus = %d\n",
            testStatus);
        for (i = 0; (i < 5); i++)
        {
            if (((extMemoBitmap >> i) & 1) == 0)
            {
                continue;
            }
            PRV_UTF_LOG3_MAC(
                "#%d errCounter: 0x%8.8X, lastFailedAddr: 0x%8.8X\n",
                i, errorInfoArr[i].errCounter, errorInfoArr[i].lastFailedAddr);
        }
    }

#endif /* ASIC_SIMULATION */
#endif /*##ifndef INCLUDE_TM*/
}

/*----------------------------------------------------------------------------*/
/*
 * Configuration of cpssDxChDiagBist suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssDxChDiagBist)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChDiagBistResultsGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChDiagBistTriggerAllSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChDiagBistTriggerAllSetWithError)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChDiagExternalMemoriesBistRun)

UTF_SUIT_END_TESTS_MAC(cpssDxChDiagBist)
