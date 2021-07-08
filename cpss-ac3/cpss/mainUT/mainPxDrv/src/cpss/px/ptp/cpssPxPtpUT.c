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
* @file cpssPxPtpUT.c
*
* @brief Unit tests for cpssPxPtp, that provides
* CPSS Px Precision Time Protocol function implementations.
*
* @version   1
********************************************************************************
*/
/* includes */

#define UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC

#include <cpss/px/config/private/prvCpssPxInfo.h>
#include <cpss/px/ptp/cpssPxPtp.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPtpTaiTodCounterFunctionGet
(
    IN  GT_SW_DEV_NUM                     dev,
    OUT CPSS_PX_PTP_TAI_TOD_COUNTER_FUNC_ENT  *functionPtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPtpTaiTodCounterFunctionGet)
{
/*
    ITERATE_DEVICES(Pipe)
    1.1. Call with valid parameters,
    Expected: GT_OK.
    1.2. Call api with wrong functionPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS st = GT_OK;

    GT_U8                             dev;
    CPSS_PX_PTP_TAI_TOD_COUNTER_FUNC_ENT  function;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with valid parameters
            Expected: GT_OK.
        */
        st = cpssPxPtpTaiTodCounterFunctionGet(dev, &function);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.4. Call api with wrong functionPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssPxPtpTaiTodCounterFunctionGet(dev, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPtpTaiTodCounterFunctionGet(dev, &function);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPtpTaiTodCounterFunctionGet(dev, &function);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPtpTaiTodCounterFunctionSet
(
    IN  GT_SW_DEV_NUM                    dev,
    IN  CPSS_PX_PTP_TAI_TOD_COUNTER_FUNC_ENT function
)
*/
UTF_TEST_CASE_MAC(cpssPxPtpTaiTodCounterFunctionSet)
{
/*
    ITERATE_DEVICES(Lion and above)
    1.1. Call with function[CPSS_PX_PTP_TAI_TOD_COUNTER_FUNC_UPDATE_E /
                            CPSS_PX_PTP_TAI_TOD_COUNTER_FUNC_INCREMENT_E /
                            CPSS_PX_PTP_TAI_TOD_COUNTER_FUNC_CAPTURE_E /
                            CPSS_PX_PTP_TAI_TOD_COUNTER_FUNC_GENERATE_E],
    Expected: GT_OK.
    1.2. Call cpssPxPtpTaiTodCounterFunctionGet with the same parameters.
    Expected: GT_OK and the same values than was set.
    1.3. Call api with wrong function [wrong enum values].
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS st = GT_OK;

    GT_U8                              dev;
    CPSS_PX_PTP_TAI_TOD_COUNTER_FUNC_ENT function = CPSS_PX_PTP_TAI_TOD_COUNTER_FUNC_UPDATE_E;
    CPSS_PX_PTP_TAI_TOD_COUNTER_FUNC_ENT functionGet = CPSS_PX_PTP_TAI_TOD_COUNTER_FUNC_UPDATE_E;

    /* there is no TAI in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with function[CPSS_PX_PTP_TAI_TOD_COUNTER_FUNC_UPDATE_E /
                                    CPSS_PX_PTP_TAI_TOD_COUNTER_FUNC_INCREMENT_E /
                                    CPSS_PX_PTP_TAI_TOD_COUNTER_FUNC_CAPTURE_E /
                                    CPSS_PX_PTP_TAI_TOD_COUNTER_FUNC_UPDATE_E],
            Expected: GT_OK.
        */

        /* call with function[CPSS_PX_PTP_TAI_TOD_COUNTER_FUNC_UPDATE_E] */
        function = CPSS_PX_PTP_TAI_TOD_COUNTER_FUNC_UPDATE_E;
        st = cpssPxPtpTaiTodCounterFunctionSet(dev, function);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call cpssPxPtpTaiTodCounterFunctionGet with the same parameters.
            Expected: GT_OK and the same values than was set.
        */
        st = cpssPxPtpTaiTodCounterFunctionGet(dev, &functionGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                  "cpssPxPtpTaiTodCounterFunctionGet: %d ", dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(function, functionGet,
                       "got another function then was set: %d", dev);

        /* call with function[CPSS_PX_PTP_TAI_TOD_COUNTER_FUNC_INCREMENT_E] */
        function = CPSS_PX_PTP_TAI_TOD_COUNTER_FUNC_INCREMENT_E;
        st = cpssPxPtpTaiTodCounterFunctionSet(dev, function);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call cpssPxPtpTaiTodCounterFunctionGet with the same parameters.
            Expected: GT_OK and the same values than was set.
        */
        st = cpssPxPtpTaiTodCounterFunctionGet(dev, &functionGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                  "cpssPxPtpTaiTodCounterFunctionGet: %d ", dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(function, functionGet,
                       "got another function then was set: %d", dev);

        /* call with function[CPSS_PX_PTP_TAI_TOD_COUNTER_FUNC_CAPTURE_E] */
        function = CPSS_PX_PTP_TAI_TOD_COUNTER_FUNC_CAPTURE_E;

        st = cpssPxPtpTaiTodCounterFunctionSet(dev, function);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call cpssPxPtpTaiTodCounterFunctionGet with the same parameters.
            Expected: GT_OK and the same values than was set.
        */
        st = cpssPxPtpTaiTodCounterFunctionGet(dev, &functionGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                  "cpssPxPtpTaiTodCounterFunctionGet: %d ", dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(function, functionGet,
                       "got another function then was set: %d", dev);

        st = cpssPxPtpTaiTodCounterFunctionGet(dev, &functionGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                  "cpssPxPtpTaiTodCounterFunctionGet: %d ", dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(function, functionGet,
                       "got another function then was set: %d", dev);

        /* call with function[CPSS_PX_PTP_TAI_TOD_COUNTER_FUNC_UPDATE_E] */
        function = CPSS_PX_PTP_TAI_TOD_COUNTER_FUNC_UPDATE_E;
        st = cpssPxPtpTaiTodCounterFunctionSet(dev, function);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        function = CPSS_PX_PTP_TAI_TOD_COUNTER_FUNC_CAPTURE_E;
        st = cpssPxPtpTaiTodCounterFunctionSet(dev, function);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call cpssPxPtpTaiTodCounterFunctionGet with the same parameters.
            Expected: GT_OK and the same values than was set.
        */
        st = cpssPxPtpTaiTodCounterFunctionGet(dev, &functionGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                  "cpssPxPtpTaiTodCounterFunctionGet: %d ", dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(function, functionGet,
                       "got another function then was set: %d", dev);

        /*
            1.3. Call api with wrong function [wrong enum values].
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssPxPtpTaiTodCounterFunctionSet
                            (dev, function),
                            function);
    }

    /* restore correct values */
    function = CPSS_PX_PTP_TAI_TOD_COUNTER_FUNC_UPDATE_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPtpTaiTodCounterFunctionSet(dev, function);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPtpTaiTodCounterFunctionSet(dev, function);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPtpTaiTodCounterFunctionTriggerGet
(
    IN  GT_SW_DEV_NUM             dev,
    OUT GT_BOOL                  *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPtpTaiTodCounterFunctionTriggerGet)
{
/*
    ITERATE_DEVICES(Pipe)
    1.1. Call with valid parameters.
    Expected: GT_OK.
    1.2. Call api with wrong enablePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS st = GT_OK;
    GT_U8                    dev;
    GT_BOOL                  enable;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with valid parameters.
            Expected: GT_OK.
        */
        st = cpssPxPtpTaiTodCounterFunctionTriggerGet(dev, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call api with wrong enablePtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssPxPtpTaiTodCounterFunctionTriggerGet(dev, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPtpTaiTodCounterFunctionTriggerGet(dev, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPtpTaiTodCounterFunctionTriggerGet(dev, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPtpTaiTodCounterFunctionTriggerSet
(
    IN  GT_U8                   dev
)
*/
UTF_TEST_CASE_MAC(cpssPxPtpTaiTodCounterFunctionTriggerSet)
{
/*
    ITERATE_DEVICES(Pipe)
    1.1. Call with valid parameters.
    Expected: GT_OK.
    1.2. Call cpssPxPtpTaiTodCounterFunctionTriggerGet
           with the same parameters.
    Expected: GT_OK and the same values than was set.
    1.3. Call api with wrong direction [wrong enum values].
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS st = GT_OK;

    GT_U8                   dev;
    GT_BOOL                 enable = GT_FALSE;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with valid parameters.
            Expected: GT_OK.
        */
        st = cpssPxPtpTaiTodCounterFunctionTriggerSet(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call cpssPxPtpTaiTodCounterFunctionTriggerGet
                   with the same parameters.
            Expected: GT_OK and the same values than was set.
        */
        st = cpssPxPtpTaiTodCounterFunctionTriggerGet(dev, &enable);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                  "cpssPxPtpTaiTodCounterFunctionTriggerGet: %d ", dev);

        /* Verifying values */
        st = cpssPxPtpTaiTodCounterFunctionTriggerSet(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPtpTaiTodCounterFunctionTriggerSet(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPtpTaiTodCounterFunctionTriggerSet(dev);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
 GT_STATUS cpssPxPtpTaiCaptureOverwriteEnableGet
(
    IN  GT_SW_DEV_NUM                           devNum,
    OUT GT_BOOL                                 *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPtpTaiCaptureOverwriteEnableGet)
{
/*
    ITERATE_DEVICES(Pipe)
    1.1. Call with valid parameters
    Expected: GT_OK.
    1.2. Call with invalid enabledPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS                    st      =  GT_OK;
    GT_U8                        devNum  =  0;
    GT_BOOL                      enable  =  GT_FALSE;

    /* there is no TAI in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /*
            1.1. Call with valid parameters
            Expected: GT_OK.
        */
        st = cpssPxPtpTaiCaptureOverwriteEnableGet(devNum, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

        /*
           1.2 Call with invalid enabledPtr [NULL].
           Expected: GT_BAD_PTR.
        */        st = cpssPxPtpTaiCaptureOverwriteEnableGet(devNum, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, enabled = NULL",
                                     devNum);
    }

    /* restore valid values */
    enable  = GT_TRUE;

    /* 2. For not applicable devices check that function returns non GT_OK.*/
    /* prepare device iterator to go through all non-applicable devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_NONE_FAMILY_E);

    /* Go over all non-applicable devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssPxPtpTaiCaptureOverwriteEnableGet(devNum, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /*3. Call function with out of bound value for device id.*/
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPtpTaiCaptureOverwriteEnableGet(devNum, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPtpTaiCaptureOverwriteEnableSet
(
    IN  GT_SW_DEV_NUM                           devNum,
    IN  GT_BOOL                                 enable  
)
*/
UTF_TEST_CASE_MAC(cpssPxPtpTaiCaptureOverwriteEnableSet)
{
    /*
        ITERATE_DEVICES(Pipe)
        1.1. Call with valid parameters
        Expected: GT_OK.
        1.2. Call cpssPxPtpTaiCaptureOverwriteEnableGet().
        Expected: GT_OK.
    */
        GT_STATUS                    st      =  GT_OK;
        GT_U8                        devNum  =  0;
        GT_BOOL                      apEnable     =  GT_FALSE;
        GT_BOOL                      apEnableGet  =  GT_FALSE;

        /* there is no TAI in GM */
        GM_NOT_SUPPORT_THIS_TEST_MAC;

        /* prepare device iterator */
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_NONE_FAMILY_E);

        /* 1. Go over all active devices. */
        while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
        {
            /*
                1.1 Call with valid parameters
                Expected: GT_OK.
            */
            apEnable = GT_TRUE;

            st = cpssPxPtpTaiCaptureOverwriteEnableSet(devNum, apEnable);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

            /*
                1.2. Call cpssPxPtpTaiCaptureOverwriteEnableGet().
                Expected: GT_OK and the same parameters value as were set in 1.1
            */
            st = cpssPxPtpTaiCaptureOverwriteEnableGet(devNum, &apEnableGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);
            UTF_VERIFY_EQUAL1_PARAM_MAC(apEnable, apEnableGet, devNum);

            /*
                1.1 Call with valid parameters
                Expected: GT_OK.
            */
            apEnable = GT_FALSE;

            st = cpssPxPtpTaiCaptureOverwriteEnableSet(devNum, apEnable);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

            /*
                1.2. Call cpssPxPtpTaiCaptureOverwriteEnableGet().
                Expected: GT_OK and the same parameters value as were set in 1.1
            */
            st = cpssPxPtpTaiCaptureOverwriteEnableGet(devNum, &apEnableGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);
            UTF_VERIFY_EQUAL1_PARAM_MAC(apEnable, apEnableGet, devNum);
        }

        apEnable  = GT_TRUE;

        /* 2.For not applicable devices check that function returns non GT_OK.*/
        /* prepare device iterator to go through all non-applicable devices */
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_NONE_FAMILY_E);

        /* Go over all non-applicable devices. */
        while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
        {
            st = cpssPxPtpTaiCaptureOverwriteEnableSet(devNum, apEnable);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
        }

        /* 3. Call function with out of bound value for device id.*/
        devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;
        st = cpssPxPtpTaiCaptureOverwriteEnableSet(devNum, apEnable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPtpTaiClockCycleGet
(
    IN  GT_SW_DEV_NUM                         devNum,
    OUT GT_U32                                *secondsPtr,
    OUT GT_U32                                *nanoSecondsPtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPtpTaiClockCycleGet)
{
    /*
        ITERATE_DEVICES(Pipe)
        1.1 Call with valid parameters
        Expected: GT_OK.
        1.2 Call with invalid secondPtr [NULL].
        Expected: GT_BAD_PTR.
        1.3 Call with invalid nanoSecondPtr [NULL].
        Expected: GT_BAD_PTR.
    */

    GT_STATUS                    st      =  GT_OK;
    GT_U8                        devNum  =  0;
    OUT GT_U32                   seconds;
    OUT GT_U32                   nanoSeconds;

    /* there is no TAI in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /*
            1.1 Call with valid parameters
            Expected: GT_OK.
        */
        st = cpssPxPtpTaiClockCycleGet(devNum, &seconds, &nanoSeconds);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);


        /*
            1.2 Call with invalid secondPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssPxPtpTaiClockCycleGet(devNum, NULL, &nanoSeconds);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, enabled = NULL",
            devNum);

        /*
            1.3 Call with invalid nanoSeconsPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssPxPtpTaiClockCycleGet(devNum, &seconds, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, enabled = NULL",
            devNum);
    }

    /* restore valid values */
    /* 2. For not applicable devices check that function returns non GT_OK.*/
    /* prepare device iterator to go through all non-applicable devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_NONE_FAMILY_E);

    /* Go over all non-applicable devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssPxPtpTaiClockCycleGet(devNum, &seconds, &nanoSeconds);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3. Call function with out of bound value for device id. */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssPxPtpTaiClockCycleGet(devNum, &seconds, &nanoSeconds);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPtpTaiClockCycleSet
(
    IN  GT_SW_DEV_NUM                         devNum,
    IN  GT_U32                                seconds,
    IN  GT_U32                                nanoSeconds
)
*/
UTF_TEST_CASE_MAC(cpssPxPtpTaiClockCycleSet)
{
    /*
        ITERATE_DEVICES(Pipe)
        1.1 Call with valid parameters
        Expected: GT_OK.
        1.2. Call cpssPxPtpTaiClockCycleGet().
        Expected: GT_OK.
    */
        GT_STATUS                    st      =  GT_OK;
        GT_U8                        devNum  =  0;
        GT_U32                       apSecond = 0;
        GT_U32                       apSecondGet;
        GT_U32                       apNanoSecond = 0;
        GT_U32                       apNanoSecondGet;

        /* there is no TAI in GM */
        GM_NOT_SUPPORT_THIS_TEST_MAC;

        /* prepare device iterator */
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_NONE_FAMILY_E);

        /* 1. Go over all active devices. */
        while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
        {
            /*
                1.1 Call with valid parameters.
                Expected: GT_OK.
            */
            apSecond = 0;
            apNanoSecond = 0;
            st = cpssPxPtpTaiClockCycleSet(devNum, apSecond, apNanoSecond);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

            /*
                1.2. Call cpssPxPtpTaiClockCycleGet().
                Expected: GT_OK.
            */
            st = cpssPxPtpTaiClockCycleGet(devNum, &apSecondGet,
                &apNanoSecondGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);
            UTF_VERIFY_EQUAL1_PARAM_MAC(apSecond, apSecondGet, devNum);
            UTF_VERIFY_EQUAL1_PARAM_MAC(apNanoSecond, apNanoSecondGet, devNum);

        }

        /* 2.For not applicable devices check that function returns non GT_OK.*/
        /* prepare device iterator to go through all non-applicable devices */
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_NONE_FAMILY_E);

        /* Go over all non-applicable devices. */
        while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
        {
            st = cpssPxPtpTaiClockCycleSet(devNum, apSecond, apNanoSecond);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
        }

        /* 3. Call function with out of bound value for device id. */
        devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;
        st = cpssPxPtpTaiClockCycleSet(devNum, apSecond, apNanoSecond);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPtpTaiClockModeGet
(
    IN  GT_SW_DEV_NUM                         devNum,
    OUT CPSS_PX_PTP_TAI_CLOCK_MODE_ENT        *clockModePtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPtpTaiClockModeGet)
{
    /*
        ITERATE_DEVICES(Pipe)
        1.1 Call with valid parameters
        Expected: GT_OK.
        1.2 Call with invalid clockModePtr [NULL].
        Expected: GT_BAD_PTR.
    */

    GT_STATUS                            st      =  GT_OK;
    GT_U8                                devNum  =  0;
    CPSS_PX_PTP_TAI_CLOCK_MODE_ENT       clockMode;

    /* there is no TAI in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /*
            1.1 Call with valid parameters
            Expected: GT_OK.
        */
        st = cpssPxPtpTaiClockModeGet(devNum, &clockMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

        /*
           1.2 Call with invalid clockModePtr [NULL].
           Expected: GT_BAD_PTR.
        */
        st = cpssPxPtpTaiClockModeGet(devNum, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, enabled = NULL",
                                     devNum);
    }

    /* 2. For not applicable devices check that function returns non GT_OK.*/
    /* prepare device iterator to go through all non-applicable devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_NONE_FAMILY_E);

    /* Go over all non-applicable devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssPxPtpTaiClockModeGet(devNum, &clockMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3. Call function with out of bound value for device id. */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssPxPtpTaiClockModeGet(devNum, &clockMode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPtpTaiClockModeSet
(
    IN  GT_SW_DEV_NUM                           devNum,
    IN  CPSS_PX_PTP_TAI_CLOCK_MODE_ENT        clockMode
)
*/
UTF_TEST_CASE_MAC(cpssPxPtpTaiClockModeSet)
{
    /*
        ITERATE_DEVICES(Pipe)
        1.1. Call with
                clockMode [CPSS_PX_PTP_TAI_CLOCK_MODE_DISABLED_E /
                            CPSS_PX_PTP_TAI_CLOCK_MODE_INPUT_E   /
                            CPSS_PX_PTP_TAI_CLOCK_MODE_INPUT_ADJUST_E]
        Expected: GT_OK.
        1.2. Call cpssPxPtpTaiClockModeGet().
        Expected: GT_OK.
        1.3. Call with invalid clock mode.
        Expected: GT_BAD_PARAM.
    */
    GT_STATUS                        st      =  GT_OK;
    GT_U8                            devNum  =  0;
    CPSS_PX_PTP_TAI_CLOCK_MODE_ENT   clockMode;
    CPSS_PX_PTP_TAI_CLOCK_MODE_ENT   clockModeGet;

    /* there is no TAI in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        for(clockMode = CPSS_PX_PTP_TAI_CLOCK_MODE_DISABLED_E;
            clockMode <= CPSS_PX_PTP_TAI_CLOCK_MODE_INPUT_ADJUST_E; clockMode++)
        {
            /*
            1.1. Call with
                clockMode [CPSS_PX_PTP_TAI_CLOCK_MODE_DISABLED_E /
                            CPSS_PX_PTP_TAI_CLOCK_MODE_INPUT_E   /
                            CPSS_PX_PTP_TAI_CLOCK_MODE_INPUT_ADJUST_E]
            */
            st = cpssPxPtpTaiClockModeSet(devNum, clockMode);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

            /*
                1.2. Call cpssPxPtpTaiClockModeGet().
                Expected: GT_OK.
            */
            st = cpssPxPtpTaiClockModeGet(devNum, &clockModeGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);
            UTF_VERIFY_EQUAL1_PARAM_MAC(clockMode, clockModeGet, devNum);
        }

        /*
            1.3. Call with invalid clock mode.
            Expected: GT_BAD_PARAM.
        */
        st = cpssPxPtpTaiClockModeSet(devNum, clockMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
    }

    /* restore valid values */
    clockMode = CPSS_PX_PTP_TAI_CLOCK_MODE_DISABLED_E;

    /* 2.For not applicable devices check that function returns non GT_OK.*/
    /* prepare device iterator to go through all non-applicable devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_NONE_FAMILY_E);

    /* Go over all non-applicable devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssPxPtpTaiClockModeSet(devNum, clockMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3. Call function with out of bound value for device id. */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssPxPtpTaiClockModeSet(devNum, clockMode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPtpTaiExternalPulseWidthGet
(
    IN  GT_SW_DEV_NUM                           devNum,
    OUT GT_U32                                  *extPulseWidthPtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPtpTaiExternalPulseWidthGet)
{
   /*
        ITERATE_DEVICES(Pipe)
        1.1 Call with valid parameters.
        Expected: GT_OK.
        1.2 Call with invalid extPulseWidthPtr [NULL].
        Expected: GT_BAD_PTR.
    */

    GT_STATUS                    st      =  GT_OK;
    GT_U8                        devNum  =  0;
    OUT GT_U32                   extPulseWidth;

    /* there is no TAI in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /*
            1.1 Call with valid parameters.
            Expected: GT_OK.
        */
        st = cpssPxPtpTaiExternalPulseWidthGet(devNum, &extPulseWidth);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

        /*
           1.2 Call with invalid nanoSeconsPtr [NULL].
           Expected: GT_BAD_PTR.
        */
        st = cpssPxPtpTaiExternalPulseWidthGet(devNum, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, enabled = NULL",
                                     devNum);
    }

    /* 2. For not applicable devices check that function returns non GT_OK.*/
    /* prepare device iterator to go through all non-applicable devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_NONE_FAMILY_E);

    /* Go over all non-applicable devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssPxPtpTaiExternalPulseWidthGet(devNum, &extPulseWidth);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /*3. Call function with out of bound value for device id.*/
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssPxPtpTaiExternalPulseWidthGet(devNum, &extPulseWidth);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPtpTaiExternalPulseWidthSet
(
    IN  GT_SW_DEV_NUM                           devNum,
    IN  GT_U32                                  extPulseWidth
)
*/
UTF_TEST_CASE_MAC(cpssPxPtpTaiExternalPulseWidthSet)
{
    /*
        ITERATE_DEVICES(Pipe)
        1.1. Call with valid parameters.
        Expected: GT_OK.
        1.2. Call cpssPxPtpTaiExternalPulseWidthGet().
        Expected: GT_OK.
    */
        GT_STATUS                    st      =  GT_OK;
        GT_U8                        devNum  =  0;
        GT_U32                       extPulseWidth;
        GT_U32                       extPulseWidthGet;
        CPSS_PX_PTP_TAI_TOD_STEP_STC todStep;

        /* there is no TAI in GM */
        GM_NOT_SUPPORT_THIS_TEST_MAC;

        /* prepare device iterator */
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_NONE_FAMILY_E);

        /* 1. Go over all active devices. */
        while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
        {
            /*
                1.1. Call with valid parameters.
                Expected: GT_OK.
            */

            /* Set TOD step */
            todStep.nanoSeconds = 1000;
            todStep.fracNanoSeconds = 0;
            st = cpssPxPtpTaiTodStepSet(devNum, &todStep);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

            extPulseWidth = 0;
            st = cpssPxPtpTaiExternalPulseWidthSet(devNum, extPulseWidth);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

            /*
                1.2. Call cpssPxPtpTaiExternalPulseWidthGet().
                Expected: GT_OK.
            */
            st = cpssPxPtpTaiExternalPulseWidthGet(devNum, &extPulseWidthGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);
            UTF_VERIFY_EQUAL1_PARAM_MAC(extPulseWidth, extPulseWidthGet, devNum);
        }

        /* 2. For not applicable devices check that function returns non GT_OK.*/
        /* prepare device iterator to go through all non-applicable devices */
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_NONE_FAMILY_E);

        /* Go over all non-applicable devices. */
        while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
        {
            st = cpssPxPtpTaiExternalPulseWidthSet(devNum, extPulseWidth);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
        }

        /* 3. Call function with out of bound value for device id.*/
        devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;
        st = cpssPxPtpTaiExternalPulseWidthSet(devNum, extPulseWidth);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*
GT_STATUS cpssPxPtpTaiFractionalNanosecondDriftGet
(
    IN  GT_SW_DEV_NUM                         devNum,
    OUT GT_32                                 *fracNanoSecondPtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPtpTaiFractionalNanosecondDriftGet)
{
   /*
        ITERATE_DEVICES(Pipe)
        1.1 Call with valid parameters.
        Expected: GT_OK.
        1.2 Call with invalid fracNanoSecondPtr [NULL].
        Expected: GT_BAD_PTR.
    */

    GT_STATUS                st      =  GT_OK;
    GT_U8                    devNum  =  0;
    GT_32                    fracNanoSecond;

    /* there is no TAI in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /*
            1.1 Call with valid parameters.
            Expected: GT_OK.
        */
        st = cpssPxPtpTaiFractionalNanosecondDriftGet(devNum, &fracNanoSecond);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

        /*
            1.2 Call with invalid nanoSeconsPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssPxPtpTaiFractionalNanosecondDriftGet(devNum, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, enabled = NULL",
                                     devNum);
    }
    /* 2. For not applicable devices check that function returns non GT_OK.*/
    /* prepare device iterator to go through all non-applicable devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_NONE_FAMILY_E);

    /* Go over all non-applicable devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssPxPtpTaiFractionalNanosecondDriftGet(devNum, &fracNanoSecond);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /*3. Call function with out of bound value for device id.*/
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssPxPtpTaiFractionalNanosecondDriftGet(devNum, &fracNanoSecond);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPtpTaiFractionalNanosecondDriftSet
(
    IN  GT_SW_DEV_NUM                         devNum,
    IN  GT_32                                 fracNanoSecond
)
*/
UTF_TEST_CASE_MAC(cpssPxPtpTaiFractionalNanosecondDriftSet)
{
/*
    ITERATE_DEVICES(Pipe)
    1.1 Call with valid parameters.
    Expected: GT_OK.
    1.2. Call cpssPxPtpTaiFractionalNanosecondDriftGet().
    Expected: GT_OK.
*/
    GT_STATUS                    st      =  GT_OK;
    GT_U8                        devNum  =  0;
    GT_U32                       fracNanoSecond = 0;
    GT_32                        fracNanoSecondGet;

    /* there is no TAI in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /*
            1.1 Call with valid parameters.
            Expected: GT_OK.
        */
        fracNanoSecond = 0;
        st = cpssPxPtpTaiFractionalNanosecondDriftSet(devNum, fracNanoSecond);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

        /*
            1.2 Call cpssPxPtpTaiFractionalNanosecondDriftGet().
            Expected: GT_OK.
        */
        st = cpssPxPtpTaiFractionalNanosecondDriftGet(devNum,
            &fracNanoSecondGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(fracNanoSecond, fracNanoSecondGet, devNum);
    }

    /* 2.For not applicable devices check that function returns non GT_OK.*/
    /* prepare device iterator to go through all non-applicable devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_NONE_FAMILY_E);

    /* Go over all non-applicable devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssPxPtpTaiFractionalNanosecondDriftSet(devNum, fracNanoSecond);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3. Call function with out of bound value for device id. */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssPxPtpTaiFractionalNanosecondDriftSet(devNum, fracNanoSecond);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPtpTaiFrequencyDriftThesholdsGet
(
    IN  GT_SW_DEV_NUM                         devNum,
    OUT GT_U32                                *interruptThresholdPtr,
    OUT GT_U32                                *adjustThresholdPtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPtpTaiFrequencyDriftThesholdsGet)
{
   /*
        ITERATE_DEVICES(Pipe)
        1.1 Call with valid parameters
        Expected: GT_OK.
        1.2 Call with invalid interruptThresholdPtr [NULL].
        Expected: GT_BAD_PTR.
        1.3 Call with invalid adjustThresholdPtr [NULL].
        Expected: GT_BAD_PTR.
    */

    GT_STATUS                    st      =  GT_OK;
    GT_U8                        devNum  =  0;
    GT_U32                       interruptThreshold;
    GT_U32                       adjustThreshold;

    /* there is no TAI in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /*
            1.1 Call with valid parameters
            Expected: GT_OK.
        */
        st = cpssPxPtpTaiFrequencyDriftThesholdsGet(devNum, &interruptThreshold,
            &adjustThreshold);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

        /*
            1.2 Call with invalid interruptThresholdPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssPxPtpTaiFrequencyDriftThesholdsGet(devNum, NULL,
            &adjustThreshold);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, enabled = NULL",
                                     devNum);
        /*
           1.3 Call with invalid adjustThresholdPtr [NULL].
           Expected: GT_BAD_PTR.
        */
        st = cpssPxPtpTaiFrequencyDriftThesholdsGet(devNum, &interruptThreshold,
            NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, enabled = NULL",
                                     devNum);
    }

    /* 2. For not applicable devices check that function returns non GT_OK.*/
    /* prepare device iterator to go through all non-applicable devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_NONE_FAMILY_E);

    /* Go over all non-applicable devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssPxPtpTaiFrequencyDriftThesholdsGet(devNum, &interruptThreshold,
            &adjustThreshold);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3. Call function with out of bound value for device id. */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssPxPtpTaiFrequencyDriftThesholdsGet(devNum, &interruptThreshold,
        &adjustThreshold);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPtpTaiFrequencyDriftThesholdsSet
(
    IN  GT_SW_DEV_NUM                         devNum,
    IN  GT_U32                                interruptThreshold,
    IN  GT_U32                                adjustThreshold
)
*/
UTF_TEST_CASE_MAC(cpssPxPtpTaiFrequencyDriftThesholdsSet)
{
/*
    ITERATE_DEVICES(Pipe)
    1.1 Call with
            interruptThreshold  [0 / 4096 / 16777215]
            adjustThreshold     [0 / 127 / 255]
    Expected: GT_OK.
    1.2. Call cpssPxPtpTaiFrequencyDriftThesholdsGet().
    Expected: GT_OK.
*/
    GT_STATUS                    st      =  GT_OK;
    GT_U8                        devNum  =  0;
    GT_U32                       interruptThreshold;
    GT_U32                       interruptThresholdGet;
    GT_U32                       adjustThreshold;
    GT_U32                       adjustThresholdGet;

    /* there is no TAI in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /*
            1.1 Call with
            interruptThreshold  [0 / 4096 / 16777215]
            adjustThreshold     [0 / 127 / 255]
            Expected: GT_OK.
        */
        /*
            interruptThreshold  = 0
            adjustThreshold     = 0
        */
        interruptThreshold  = 0;
        adjustThreshold     = 0;

        st = cpssPxPtpTaiFrequencyDriftThesholdsSet(devNum, interruptThreshold,
            adjustThreshold);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

        /*
            1.2. Call cpssPxPtpTaiFrequencyDriftThesholdsGet().
            Expected: GT_OK.
        */
        st = cpssPxPtpTaiFrequencyDriftThesholdsGet(devNum,
            &interruptThresholdGet, &adjustThresholdGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(interruptThreshold, interruptThresholdGet,
                                    devNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(adjustThreshold, adjustThresholdGet,
                                    devNum);

        /*
            interruptThreshold  = 4096
            adjustThreshold     = 127
        */
        interruptThreshold  = 4096;
        adjustThreshold     = 127;
        st = cpssPxPtpTaiFrequencyDriftThesholdsSet(devNum, interruptThreshold,
            adjustThreshold);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

        /*
            1.2. Call cpssPxPtpTaiFrequencyDriftThesholdsGet().
            Expected: GT_OK.
        */
        st = cpssPxPtpTaiFrequencyDriftThesholdsGet(devNum,
            &interruptThresholdGet, &adjustThresholdGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(interruptThreshold, interruptThresholdGet,
                                    devNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(adjustThreshold, adjustThresholdGet,
                                    devNum);

        /*
            interruptThreshold  = 16777215
            adjustThreshold     = 255
        */
        interruptThreshold  = 16777215;
        adjustThreshold     = 255;

        st = cpssPxPtpTaiFrequencyDriftThesholdsSet(devNum, interruptThreshold,
            adjustThreshold);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

        /*
            1.2. Call cpssPxPtpTaiFrequencyDriftThesholdsGet().
            Expected: GT_OK.
        */
        st = cpssPxPtpTaiFrequencyDriftThesholdsGet(devNum,
            &interruptThresholdGet, &adjustThresholdGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(interruptThreshold, interruptThresholdGet,
                                    devNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(adjustThreshold, adjustThresholdGet,
                                    devNum);
    }

    /* restore valid values */
    interruptThreshold  = 0;
    adjustThreshold     = 0;

    /* 2.For not applicable devices check that function returns non GT_OK.*/
    /* prepare device iterator to go through all non-applicable devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_NONE_FAMILY_E);

    /* Go over all non-applicable devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssPxPtpTaiFrequencyDriftThesholdsSet(devNum, interruptThreshold,
            adjustThreshold);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3. Call function with out of bound value for device id. */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssPxPtpTaiFrequencyDriftThesholdsSet(devNum, interruptThreshold,
        adjustThreshold);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPtpTaiIncomingClockCounterEnableGet
(
    IN  GT_SW_DEV_NUM                         devNum,
    OUT GT_BOOL                               *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPtpTaiIncomingClockCounterEnableGet)
{
   /*
        ITERATE_DEVICES(Pipe)
        1.1 Call with valid parameters
        Expected: GT_OK.
        1.2 Call with invalid enablePtr [NULL].
        Expected: GT_BAD_PTR.
    */

    GT_STATUS                    st      =  GT_OK;
    GT_U8                        devNum  =  0;
    GT_BOOL                      enable;

    /* there is no TAI in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /*
            1.1 Call with valid parameters.
            Expected: GT_OK.
        */
        st = cpssPxPtpTaiIncomingClockCounterEnableGet(devNum, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

        /*
            1.2 Call with invalid enablePtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssPxPtpTaiIncomingClockCounterEnableGet(devNum, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, enabled = NULL",
            devNum);
    }

    /* 2. For not applicable devices check that function returns non GT_OK.*/
    /* prepare device iterator to go through all non-applicable devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_NONE_FAMILY_E);

    /* Go over all non-applicable devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssPxPtpTaiIncomingClockCounterEnableGet(devNum, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3. Call function with out of bound value for device id .*/
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssPxPtpTaiIncomingClockCounterEnableGet(devNum, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPtpTaiIncomingClockCounterEnableSet
(
    IN  GT_SW_DEV_NUM                         devNum,
    IN  GT_BOOL                               enable
)
*/
UTF_TEST_CASE_MAC(cpssPxPtpTaiIncomingClockCounterEnableSet)
{
    /*
        ITERATE_DEVICES(Pipe)
        1.1 Call with
                enable [GT_TRUE / GT_FALSE]
        Expected: GT_OK.
        1.2 Call cpssPxPtpTaiIncomingClockCounterEnableGet().
        Expected: GT_OK.
        1.3 Call with invalid enabledPtr [NULL].
        Expected: GT_BAD_PTR.
    */
        GT_STATUS                    st          =  GT_OK;
        GT_U8                        devNum      =  0;
        GT_BOOL                      apEnable    =  GT_FALSE;
        GT_BOOL                      apEnableGet =  GT_FALSE;

        /* there is no TAI in GM */
        GM_NOT_SUPPORT_THIS_TEST_MAC;

        /* prepare device iterator */
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_NONE_FAMILY_E);

        /* 1. Go over all active devices. */
        while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
        {
            /*
                1.1 Call with
                    enable [GT_TRUE / GT_FALSE]
                Expected: GT_OK.
            */
            /*
                apEnable [GT_TRUE]
            */
            apEnable = GT_TRUE;

            st = cpssPxPtpTaiIncomingClockCounterEnableSet(devNum, apEnable);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

            /*
                1.2 Call cpssPxPtpTaiIncomingClockCounterEnableGet().
                Expected: GT_OK.
            */
            st = cpssPxPtpTaiIncomingClockCounterEnableGet(devNum, &apEnableGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);
            UTF_VERIFY_EQUAL1_PARAM_MAC(apEnable, apEnableGet, devNum);

            /*
                apEnable [GT_FALSE]
            */
            apEnable = GT_FALSE;
            st = cpssPxPtpTaiIncomingClockCounterEnableSet(devNum, apEnable);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

            /*
                1.2 Call cpssPxPtpTaiIncomingClockCounterEnableGet().
                Expected: GT_OK.
            */
            st = cpssPxPtpTaiIncomingClockCounterEnableGet(devNum, &apEnableGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);
            UTF_VERIFY_EQUAL1_PARAM_MAC(apEnable, apEnableGet, devNum);
        }

        apEnable          = GT_TRUE;

        /* 2.For not applicable devices check that function returns non GT_OK.*/
        /* prepare device iterator to go through all non-applicable devices */
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_NONE_FAMILY_E);

        /* Go over all non-applicable devices. */
        while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
        {
            st = cpssPxPtpTaiIncomingClockCounterEnableSet(devNum, apEnable);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
        }

        /* 3. Call function with out of bound value for device id.*/
        devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;
        st = cpssPxPtpTaiIncomingClockCounterEnableSet(devNum, apEnable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPtpTaiIncomingClockCounterGet
(
    IN  GT_U8                                 devNum,
    OUT GT_U32                                *valuePtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPtpTaiIncomingClockCounterGet)
{
    /*
        ITERATE_DEVICES(Pipe)
        1.1 Call with valid parameters.
        Expected: GT_OK.
        1.2 Call with invalid valuePtr [NULL].
        Expected: GT_BAD_PTR.
    */

    GT_STATUS                    st      =  GT_OK;
    GT_U8                        devNum  =  0;
    GT_U32                       value;

    /* there is no TAI in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /*
            1.1 Call with valid parameters.
            Expected: GT_OK.
        */
        st = cpssPxPtpTaiIncomingClockCounterGet(devNum, &value);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

        /*
            1.2 Call with invalid valuePtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssPxPtpTaiIncomingClockCounterGet(devNum, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, value = NULL",
            devNum);
    }

    /* 2. For not applicable devices check that function returns non GT_OK.*/
    /* prepare device iterator to go through all non-applicable devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_NONE_FAMILY_E);

    /* Go over all non-applicable devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssPxPtpTaiIncomingClockCounterGet(devNum, &value);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3. Call function with out of bound value for device id .*/
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssPxPtpTaiIncomingClockCounterGet(devNum, &value);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPtpTaiIncomingTriggerCounterGet
(
    IN  GT_SW_DEV_NUM                         devNum,
    OUT GT_U32                                *valuePtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPtpTaiIncomingTriggerCounterGet)
{
   /*
        ITERATE_DEVICES(Pipe)
        1.1 Call with valid parameters.
        Expected: GT_OK.
        1.2 Call with invalid valuePtr [NULL].
        Expected: GT_BAD_PTR.
    */

    GT_STATUS                    st      =  GT_OK;
    GT_U8                        devNum  =  0;
    OUT GT_U32                   value;

    /* there is no TAI in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /*
            1.1 Call with valid parameters.
            Expected: GT_OK.
        */
        st = cpssPxPtpTaiIncomingTriggerCounterGet(devNum, &value);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

        /*
            1.2 Call with invalid valuePtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssPxPtpTaiIncomingTriggerCounterGet(devNum, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, enabled = NULL",
            devNum);
    }

    /* 2. For not applicable devices check that function returns non GT_OK.*/
    /* prepare device iterator to go through all non-applicable devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_NONE_FAMILY_E);

    /* Go over all non-applicable devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssPxPtpTaiIncomingTriggerCounterGet(devNum, &value);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3. Call function with out of bound value for device id. */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssPxPtpTaiIncomingTriggerCounterGet(devNum, &value);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPtpTaiInputTriggersCountEnableGet
(
    IN  GT_SW_DEV_NUM                           devNum,
    OUT GT_BOOL                                 *enablePtrs
)
*/
UTF_TEST_CASE_MAC(cpssPxPtpTaiInputTriggersCountEnableGet)
{
/*
    ITERATE_DEVICES(Pipe)
    1.1 Call with valid parameters.
    Expected: GT_OK.
    1.2 Call with invalid enabledPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS                    st      =  GT_OK;
    GT_U8                        devNum  =  0;
    GT_BOOL                      enable  =  GT_FALSE;

    /* there is no TAI in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /*
            1.1 Call with valid parameters.
            Expected: GT_OK.
        */
        st = cpssPxPtpTaiInputTriggersCountEnableGet(devNum, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

        /*
            1.2 Call with invalid enabledPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssPxPtpTaiInputTriggersCountEnableGet(devNum, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, enabled = NULL",
                                     devNum);
    }

    /* 2. For not applicable devices check that function returns non GT_OK.*/
    /* prepare device iterator to go through all non-applicable devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_NONE_FAMILY_E);

    /* Go over all non-applicable devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssPxPtpTaiInputTriggersCountEnableGet(devNum, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3. Call function with out of bound value for device id. */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssPxPtpTaiInputTriggersCountEnableGet(devNum, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPtpTaiInputTriggersCountEnableSet
(
    IN  GT_SW_DEV_NUM                           devNum,
    IN  GT_BOOL                                 enable
)
*/
UTF_TEST_CASE_MAC(cpssPxPtpTaiInputTriggersCountEnableSet)
{
    /*
        ITERATE_DEVICES(Pipe)
        1.1 Call with valid parameters.
        Expected: GT_OK.
        1.2 Call cpssPxPtpTaiInputTriggersCountEnableGet().
        Expected: GT_OK.
        1.3 Call with invalid enabledPtr [NULL].
        Expected: GT_BAD_PTR.
    */
        GT_STATUS                    st      =  GT_OK;
        GT_U8                        devNum  =  0;
        GT_BOOL                      apEnable     =  GT_FALSE;
        GT_BOOL                      apEnableGet  =  GT_FALSE;

        /* there is no TAI in GM */
        GM_NOT_SUPPORT_THIS_TEST_MAC;

        /* prepare device iterator */
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_NONE_FAMILY_E);

        /* 1. Go over all active devices. */
        while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
        {
            /*
                1.1 Call with valid parameters.
                Expected: GT_OK.
            */
            apEnable = GT_TRUE;
            st = cpssPxPtpTaiInputTriggersCountEnableSet(devNum, apEnable);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

            /*
                1.2 Call cpssPxPtpTaiInputTriggersCountEnableGet().
                Expected: GT_OK.
            */
            st = cpssPxPtpTaiInputTriggersCountEnableGet(devNum, &apEnableGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);
            UTF_VERIFY_EQUAL1_PARAM_MAC(apEnable, apEnableGet, devNum);

            apEnable = GT_FALSE;
            st = cpssPxPtpTaiInputTriggersCountEnableSet(devNum, apEnable);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

            /*
                1.2 Call cpssPxPtpTaiInputTriggersCountEnableGet().
                Expected: GT_OK.
            */
            st = cpssPxPtpTaiInputTriggersCountEnableGet(devNum, &apEnableGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);
            UTF_VERIFY_EQUAL1_PARAM_MAC(apEnable, apEnableGet, devNum);

        }

        /* restore valid values */
        apEnable  = GT_TRUE;

        /* 2.For not applicable devices check that function returns non GT_OK.*/
        /* prepare device iterator to go through all non-applicable devices */
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_NONE_FAMILY_E);

        /* Go over all non-applicable devices. */
        while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
        {
            st = cpssPxPtpTaiInputTriggersCountEnableSet(devNum, apEnable);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
        }

        /* 3. Call function with out of bound value for device id. */
        devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;
        st = cpssPxPtpTaiInputTriggersCountEnableSet(devNum, apEnable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPtpTaiOutputTriggerEnableGet
(
    IN  GT_SW_DEV_NUM                         devNum,
    OUT GT_BOOL                               *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPtpTaiOutputTriggerEnableGet)
{
/*
    ITERATE_DEVICES(Pipe)
    1.1 Call with valid parameters.
    Expected: GT_OK.
    1.2 Call with invalid enabledPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS                    st      =  GT_OK;
    GT_U8                        devNum  =  0;
    GT_BOOL                      enable  =  GT_FALSE;

    /* there is no TAI in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /*
            1.1 Call with valid parameters.
            Expected: GT_OK.
        */
        st = cpssPxPtpTaiOutputTriggerEnableGet(devNum, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

        /*
            1.2 Call with invalid enabledPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssPxPtpTaiOutputTriggerEnableGet(devNum, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, enabled = NULL",
                                     devNum);
    }

    /* restore valid values */
    enable  = GT_TRUE;

    /* 2. For not applicable devices check that function returns non GT_OK.*/
    /* prepare device iterator to go through all non-applicable devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_NONE_FAMILY_E);

    /* Go over all non-applicable devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssPxPtpTaiOutputTriggerEnableGet(devNum, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3. Call function with out of bound value for device id. */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPtpTaiOutputTriggerEnableGet(devNum, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPtpTaiOutputTriggerEnableSet
(
    IN  GT_SW_DEV_NUM                         devNum,
    IN  GT_BOOL                               enable
)
*/
UTF_TEST_CASE_MAC(cpssPxPtpTaiOutputTriggerEnableSet)
{
    /*
        ITERATE_DEVICES(Pipe)
        1.1 Call with valid parameters.
        Expected: GT_OK.
        1.2. Call cpssPxPtpTaiOutputTriggerEnableGet().
        Expected: GT_OK.
    */
        GT_STATUS                    st           =  GT_OK;
        GT_U8                        devNum       =  0;
        GT_BOOL                      apEnable     =  GT_FALSE;
        GT_BOOL                      apEnableGet  =  GT_FALSE;

        /* there is no TAI in GM */
        GM_NOT_SUPPORT_THIS_TEST_MAC;

        /* prepare device iterator */
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_NONE_FAMILY_E);

        /* 1. Go over all active devices. */
        while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
        {
            /*
                1.1 Call with valid parameters.
                Expected: GT_OK.
            */
            apEnable          = GT_TRUE;

            st = cpssPxPtpTaiOutputTriggerEnableSet(devNum, apEnable);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

            /*
                1.2. Call cpssPxPtpTaiOutputTriggerEnableGet().
                Expected: GT_OK.
            */
            st = cpssPxPtpTaiOutputTriggerEnableGet(devNum, &apEnableGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);
            UTF_VERIFY_EQUAL1_PARAM_MAC(apEnable, apEnableGet, devNum);

            apEnable          = GT_FALSE;

            st = cpssPxPtpTaiOutputTriggerEnableSet(devNum, apEnable);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

            /*
                1.2. Call cpssPxPtpTaiOutputTriggerEnableGet().
                Expected: GT_OK.
            */
            st = cpssPxPtpTaiOutputTriggerEnableGet(devNum, &apEnableGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);
            UTF_VERIFY_EQUAL1_PARAM_MAC(apEnable, apEnableGet, devNum);
        }

        /* restore valid values */
        apEnable          = GT_TRUE;

        /* 2.For not applicable devices check that function returns non GT_OK.*/
        /* prepare device iterator to go through all non-applicable devices */
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_NONE_FAMILY_E);

        /* Go over all non-applicable devices. */
        while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
        {
            st = cpssPxPtpTaiOutputTriggerEnableSet(devNum, apEnable);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
        }

        /* 3. Call function with out of bound value for device id. */
        devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;
        st = cpssPxPtpTaiOutputTriggerEnableSet(devNum, apEnable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPtpTaiPClockCycleGet
(
    IN  GT_SW_DEV_NUM                         devNum,
    OUT GT_U32                                *nanoSecondsPtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPtpTaiPClockCycleGet)
{
   /*
        ITERATE_DEVICES(Pipe)
        1.1 Call with valid parameters.
        Expected: GT_OK.
        1.2 Call with invalid nanoSecondsPtr [NULL].
        Expected: GT_BAD_PTR.
    */

    GT_STATUS                    st      =  GT_OK;
    GT_U8                        devNum  =  0;
    OUT GT_U32                   nanoSeconds;

    /* there is no TAI in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /*
            1.1 Call with valid parameters.
            Expected: GT_OK.
        */
        st = cpssPxPtpTaiPClockCycleGet(devNum, &nanoSeconds);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

        /*
            1.2 Call with invalid nanoSeconsPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssPxPtpTaiPClockCycleGet(devNum, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, enabled = NULL",
                                     devNum);
    }

    /* 2. For not applicable devices check that function returns non GT_OK.*/
    /* prepare device iterator to go through all non-applicable devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_NONE_FAMILY_E);

    /* Go over all non-applicable devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssPxPtpTaiPClockCycleGet(devNum, &nanoSeconds);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3. Call function with out of bound value for device id. */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssPxPtpTaiPClockCycleGet(devNum, &nanoSeconds);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPtpTaiPClockCycleSet
(
    IN  GT_SW_DEV_NUM                         devNum,
    IN  GT_U32                                nanoSeconds
)
*/
UTF_TEST_CASE_MAC(cpssPxPtpTaiPClockCycleSet)
{
    /*
        ITERATE_DEVICES(Pipe)
        1.1 Call with valid parameters.
        Expected: GT_OK.
        1.2 Call cpssPxPtpTaiPClockCycleGet().
        Expected: GT_OK.
    */
        GT_STATUS                    st      =  GT_OK;
        GT_U8                        devNum  =  0;
        GT_U32                       nanoSeconds = 0;
        GT_U32                       nanoSecondsGet;

        /* there is no TAI in GM */
        GM_NOT_SUPPORT_THIS_TEST_MAC;

        /* prepare device iterator */
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_NONE_FAMILY_E);

        /* 1. Go over all active devices. */
        while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
        {
            /*
                1.1 Call with valid parameters.
                Expected: GT_OK.
            */
            nanoSeconds       = 0;
            st = cpssPxPtpTaiPClockCycleSet(devNum, nanoSeconds);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

            /*
                1.2 Call cpssPxPtpTaiPClockCycleGet().
                Expected: GT_OK.
            */
            st = cpssPxPtpTaiPClockCycleGet(devNum, &nanoSecondsGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);
            UTF_VERIFY_EQUAL1_PARAM_MAC(nanoSeconds, nanoSecondsGet, devNum);
        }

        /* 2.For not applicable devices check that function returns non GT_OK.*/
        /* prepare device iterator to go through all non-applicable devices */
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_NONE_FAMILY_E);

        /* Go over all non-applicable devices. */
        while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
        {
            st = cpssPxPtpTaiPClockCycleSet(devNum, nanoSeconds);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
        }

        /* 3. Call function with out of bound value for device id. */
        devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;
        st = cpssPxPtpTaiPClockCycleSet(devNum, nanoSeconds);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPtpTaiPtpPClockDriftAdjustEnableGet
(
    IN  GT_SW_DEV_NUM                           devNum,
    OUT GT_BOOL                                 *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPtpTaiPtpPClockDriftAdjustEnableGet)
{
/*
    ITERATE_DEVICES(Pipe)
    1.1 Call with valid parameters.
    Expected: GT_OK.
    1.2 Call with invalid enabledPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS                    st      =  GT_OK;
    GT_U8                        devNum  =  0;
    GT_BOOL                      enable  =  GT_FALSE;

    /* there is no TAI in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /*
            1.1 Call with valid parameters.
            Expected: GT_OK.
        */
        st = cpssPxPtpTaiPtpPClockDriftAdjustEnableGet(devNum, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

        /*
            1.2 Call with invalid enabledPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssPxPtpTaiPtpPClockDriftAdjustEnableGet(devNum, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, enabled = NULL",
            devNum);
    }

    /* 2. For not applicable devices check that function returns non GT_OK.*/
    /* prepare device iterator to go through all non-applicable devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_NONE_FAMILY_E);

    /* Go over all non-applicable devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssPxPtpTaiPtpPClockDriftAdjustEnableGet(
                devNum,
                &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3. Call function with out of bound value for device id. */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssPxPtpTaiPtpPClockDriftAdjustEnableGet(devNum, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPtpTaiPtpPClockDriftAdjustEnableSet
(
    IN  GT_SW_DEV_NUM                           devNum,
    IN  GT_BOOL                                 enable
)
*/
UTF_TEST_CASE_MAC(cpssPxPtpTaiPtpPClockDriftAdjustEnableSet)
{
    /*
        ITERATE_DEVICES(Pipe)
        1.1 Call with valid parameters.
        Expected: GT_OK.
        1.2 Call cpssPxPtpTaiPtpPClockDriftAdjustEnableGet().
        Expected: GT_OK.
    */
        GT_STATUS                    st      =  GT_OK;
        GT_U8                        devNum  =  0;
        GT_BOOL                      apEnable     =  GT_FALSE;
        GT_BOOL                      apEnableGet  =  GT_FALSE;

        /* there is no TAI in GM */
        GM_NOT_SUPPORT_THIS_TEST_MAC;

        /* prepare device iterator */
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_NONE_FAMILY_E);

        /* 1. Go over all active devices. */
        while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
        {
            /*
                1.1 Call with valid parameters.
                Expected: GT_OK.
            */
            apEnable = GT_TRUE;

            st = cpssPxPtpTaiPtpPClockDriftAdjustEnableSet(devNum, apEnable);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

            /*
                1.2 Call cpssPxPtpTaiPtpPClockDriftAdjustEnableGet().
                Expected: GT_OK.
            */
            st = cpssPxPtpTaiPtpPClockDriftAdjustEnableGet(devNum,
                &apEnableGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);
            UTF_VERIFY_EQUAL1_PARAM_MAC(apEnable, apEnableGet, devNum);
        }
        /* 2.For not applicable devices check that function returns non GT_OK.*/
        /* prepare device iterator to go through all non-applicable devices */
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_NONE_FAMILY_E);

        /* Go over all non-applicable devices. */
        while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
        {
            st = cpssPxPtpTaiPtpPClockDriftAdjustEnableSet(devNum, apEnable);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
        }

        /* 3. Call function with out of bound value for device id. */
        devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;
        st = cpssPxPtpTaiPtpPClockDriftAdjustEnableSet(devNum, apEnable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPtpTaiTodCaptureStatusGet
(
    IN  GT_SW_DEV_NUM                         devNum,
    IN  GT_U32                                captureIndex,
    OUT GT_BOOL                               *validPtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPtpTaiTodCaptureStatusGet)
{
/*
    ITERATE_DEVICES(Pipe)
    1.1 Call with
            captureIndex [0 / 1]
    Expected: GT_OK.
    1.2 Call with invalid validPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS                    st      =  GT_OK;
    GT_U8                        devNum  =  0;
    GT_BOOL                      valid   =  GT_FALSE;
    GT_U32                       captureIndex;

    /* there is no TAI in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /*
            1.1 Call with
                    captureIndex [0 / 1]
            Expected: GT_OK.
        */
        captureIndex      = 0;
        st = cpssPxPtpTaiTodCaptureStatusGet(devNum, captureIndex, &valid);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

        captureIndex      = 1;
        st = cpssPxPtpTaiTodCaptureStatusGet(devNum, captureIndex, &valid);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

        /*
           1.2 Call with invalid validPtr [NULL].
           Expected: GT_BAD_PTR.
        */
        st = cpssPxPtpTaiTodCaptureStatusGet(devNum, captureIndex, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, valid = NULL",
                                     devNum);
    }

    /* restore valid values */
    valid             = GT_TRUE;
    captureIndex      = 0;

    /* 2. For not applicable devices check that function returns non GT_OK.*/
    /* prepare device iterator to go through all non-applicable devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_NONE_FAMILY_E);

    /* Go over all non-applicable devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssPxPtpTaiTodCaptureStatusGet(devNum, captureIndex, &valid);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3. Call function with out of bound value for device id. */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssPxPtpTaiTodCaptureStatusGet(devNum, captureIndex, &valid);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPtpTaiTodGet
(
    IN  GT_SW_DEV_NUM                       devNum,
    IN  CPSS_PX_PTP_TAI_TOD_TYPE_ENT        todValueType,
    OUT CPSS_PX_PTP_TAI_TOD_COUNT_STC       *todValuePtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPtpTaiTodGet)
{
/*
    ITERATE_DEVICES(Pipe)
    1.1 Call with valid parameters
    Expected: GT_OK.
    1.2 Call with invalid todValuePtr [NULL].
    Expected: GT_BAD_PTR.
    1.3 Call with invalid todValueType.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS                      st      =  GT_OK;
    GT_U8                          devNum  =  0;
    CPSS_PX_PTP_TAI_TOD_TYPE_ENT   todValueType;
    CPSS_PX_PTP_TAI_TOD_COUNT_STC  todValue;

    /* there is no TAI in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /*
            1.1 Call with
            todValueType [CPSS_PX_PTP_TAI_TOD_TYPE_TRIGGER_GENERATION_E /
                          CPSS_PX_PTP_TAI_TOD_TYPE_TRIGGER_GENERATION_MASK_E   /
                          CPSS_PX_PTP_TAI_TOD_TYPE_LOAD_VALUE_E]
            Expected: GT_OK.
        */
        todValueType      = CPSS_PX_PTP_TAI_TOD_TYPE_TRIGGER_GENERATION_E;

        st = cpssPxPtpTaiTodGet(devNum, todValueType, &todValue);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

        todValueType      = CPSS_PX_PTP_TAI_TOD_TYPE_LOAD_VALUE_E;
        st = cpssPxPtpTaiTodGet(devNum, todValueType, &todValue);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

        /*
            1.2 Call with invalid todValuePtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssPxPtpTaiTodGet(devNum, todValueType, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, devNum);

        /*
            1.3 Call with invalid todValueType.
            Expected: GT_BAD_PARAM.
        */
        st = cpssPxPtpTaiTodGet(devNum, 10, &todValue);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
    }

    /* restore valid values */
    todValueType      = CPSS_PX_PTP_TAI_TOD_TYPE_TRIGGER_GENERATION_E;

    /* 2. For not applicable devices check that function returns non GT_OK.*/
    /* prepare device iterator to go through all non-applicable devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_NONE_FAMILY_E);

    /* Go over all non-applicable devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssPxPtpTaiTodGet(devNum, todValueType, &todValue);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3. Call function with out of bound value for device id. */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssPxPtpTaiTodGet(devNum, todValueType, &todValue);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPtpTaiTodSet
(
    IN  GT_SW_DEV_NUM                         devNum,
    IN  CPSS_PX_PTP_TAI_TOD_TYPE_ENT          todValueType,
    IN  CPSS_PX_PTP_TAI_TOD_COUNT_STC         *todValuePtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPtpTaiTodSet)
{
    /*
        ITERATE_DEVICES(Pipe)
        1.1 Call with
                todValueType [CPSS_PX_PTP_TAI_TOD_TYPE_TRIGGER_GENERATION_E /
                              CPSS_PX_PTP_TAI_TOD_TYPE_TRIGGER_GENERATION_MASK_E         /
                              CPSS_PX_PTP_TAI_TOD_TYPE_LOAD_VALUE_E]
                todValuePtr {
                                nanoSeconds     [0 / 16383 / 268435455]
                                seconds         [0 / 16383 / 268435455]
                                fracNanoSeconds [0 / 16383 / 268435455]
                            }
        Expected: GT_OK.
        1.2 Call cpssPxPtpTaiTodGet().
        Expected: GT_OK.
        1.3 Call with invalid todValueType.
        Expected: GT_BAD_PARAM.
    */
        GT_STATUS                      st      =  GT_OK;
        GT_U8                          devNum  =  0;
        CPSS_PX_PTP_TAI_TOD_TYPE_ENT   todValueType;
        CPSS_PX_PTP_TAI_TOD_COUNT_STC  todValue;
        CPSS_PX_PTP_TAI_TOD_COUNT_STC  todValueGet;
        CPSS_PX_PTP_TAI_TOD_TYPE_ENT   todValueTypeArr[] = {
            CPSS_PX_PTP_TAI_TOD_TYPE_TRIGGER_GENERATION_E,
            CPSS_PX_PTP_TAI_TOD_TYPE_TRIGGER_GENERATION_MASK_E,
            CPSS_PX_PTP_TAI_TOD_TYPE_LOAD_VALUE_E
        };
        GT_U32   nanoSecondsArr[] = {0, 16383, 268435455};
        GT_U32  fracNanoSecondsArr[] = {0, 16383, 268435455};
        GT_U32  secondsArr[] = {0, 16383, 268435455};
        GT_U32  ii;

        /* there is no TAI in GM */
        GM_NOT_SUPPORT_THIS_TEST_MAC;

        /* prepare device iterator */
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_NONE_FAMILY_E);

        /* 1. Go over all active devices. */
        while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
        {
            for (ii = 0; ii < 3; ii++)
            {
                /*
                    1.1 Call with
                    todValueType [CPSS_PX_PTP_TAI_TOD_TYPE_TRIGGER_GENERATION_E/
                                CPSS_PX_PTP_TAI_TOD_TYPE_TRIGGER_GENERATION_MASK_E        /
                                CPSS_PX_PTP_TAI_TOD_TYPE_LOAD_VALUE_E]
                    todValuePtr {
                                    nanoSeconds     [0 / 16383 / 268435455]
                                    seconds         [0 / 16383 / 268435455]
                                    fracNanoSeconds [0 / 16383 / 268435455]
                                }
                    Expected: GT_OK.
                */
                todValueType             = todValueTypeArr[ii];
                todValue.nanoSeconds     = nanoSecondsArr[ii];
                todValue.seconds.l[0]    = secondsArr[ii];
                todValue.seconds.l[1]    = 0;
                todValue.fracNanoSeconds = fracNanoSecondsArr[ii];

                st = cpssPxPtpTaiTodSet(devNum, todValueType, &todValue);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

                /*
                    1.2 Call cpssPxPtpTaiTodGet().
                    Expected: GT_OK.
                */
                st = cpssPxPtpTaiTodGet(devNum, todValueType, &todValueGet);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);
                UTF_VERIFY_EQUAL1_PARAM_MAC(todValue.nanoSeconds,
                                            todValueGet.nanoSeconds,
                                            devNum);
                UTF_VERIFY_EQUAL1_PARAM_MAC(todValue.seconds.l[0],
                                            todValueGet.seconds.l[0],
                                            devNum);
                UTF_VERIFY_EQUAL1_PARAM_MAC(todValue.seconds.l[1],
                                            todValueGet.seconds.l[1],
                                            devNum);
                UTF_VERIFY_EQUAL1_PARAM_MAC(todValue.fracNanoSeconds,
                                            todValueGet.fracNanoSeconds,
                                            devNum);
            }
            /*
                1.3 Call with invalid todValueType.
                Expected: GT_BAD_PARAM.
            */
            st = cpssPxPtpTaiTodGet(devNum,
                CPSS_PX_PTP_TAI_TOD_TYPE_CAPTURE_VALUE1_E, &todValue);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);
        }

        /* restore valid values */
        todValueType = CPSS_PX_PTP_TAI_TOD_TYPE_TRIGGER_GENERATION_E;
        todValue.nanoSeconds     = 0;
        todValue.seconds.l[0]    = 0;
        todValue.seconds.l[1]    = 0;
        todValue.fracNanoSeconds = 0;

        /* 2.For not applicable devices check that function returns non GT_OK.*/
        /* prepare device iterator to go through all non-applicable devices */
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_NONE_FAMILY_E);

        /* Go over all non-applicable devices. */
        while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
        {
            st = cpssPxPtpTaiTodSet(devNum, todValueType, &todValue);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
        }

        /* 3. Call function with out of bound value for device id. */
        devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;
        st = cpssPxPtpTaiTodSet(devNum, todValueType, &todValue);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPtpTaiTodStepGet
(
    IN  GT_SW_DEV_NUM                     devNum,
    OUT CPSS_PX_PTP_TAI_TOD_STEP_STC    *todStepPtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPtpTaiTodStepGet)
{
   /*
        ITERATE_DEVICES(Pipe)
        1.1 Call with correct arguments.
        Expected: GT_OK.
        1.2 Call with invalid todStepPtr [NULL].
        Expected: GT_BAD_PTR.
    */

    GT_STATUS                        st      =  GT_OK;
    GT_U8                            devNum  =  0;
    CPSS_PX_PTP_TAI_TOD_STEP_STC     todStep;

    /* there is no TAI in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /*
            1.1 Call with correct arguments.
            Expected: GT_OK.
        */
        st = cpssPxPtpTaiTodStepGet(devNum, &todStep);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

        /*
            1.2 Call with invalid nanoSeconsPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssPxPtpTaiTodStepGet(devNum, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, enabled = NULL",
                                     devNum);
    }

    /* 2. For not applicable devices check that function returns non GT_OK.*/
    /* prepare device iterator to go through all non-applicable devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_NONE_FAMILY_E);

    /* Go over all non-applicable devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssPxPtpTaiTodStepGet(devNum, &todStep);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /*3. Call function with out of bound value for device id.*/
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssPxPtpTaiTodStepGet(devNum, &todStep);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPtpTaiTodStepSet
(
    IN  GT_SW_DEV_NUM                     devNum,
    IN  CPSS_PX_PTP_TAI_TOD_STEP_STC    *todStepPtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPtpTaiTodStepSet)
{
   /*
        ITERATE_DEVICES(Pipe)
        1.1 Call with
                todStep {
                            nanoSeconds     [0 / 65535 / 65535]
                            fracNanoSeconds [0 / 65535 / 2147483647]
                        }
        Expected: GT_OK.
        1.2 Call cpssPxPtpTaiTodStepGet().
        Expected: GT_OK.
    */
        GT_STATUS                      st      =  GT_OK;
        GT_U8                          devNum  =  0;
        CPSS_PX_PTP_TAI_TOD_STEP_STC   todStep;
        CPSS_PX_PTP_TAI_TOD_STEP_STC   todStepGet;
        CPSS_PX_PTP_TAI_TOD_STEP_STC   todStepArr[] = {
            {0, 0}, {65535, 65535}, {65535, 2147483647}
        };
        GT_U32                         ii;

        /* there is no TAI in GM */
        GM_NOT_SUPPORT_THIS_TEST_MAC;

        /* prepare device iterator */
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_NONE_FAMILY_E);

        /* 1. Go over all active devices. */
        while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
        {
            for (ii = 0; ii < 3; ii++)
            {
                /*
                    1.1 Call with
                            todStep {
                                        nanoSeconds     [0 / 65535 / 65535]
                                        fracNanoSeconds [0 / 65535 / 2147483647]
                                    }
                    Expected: GT_OK.
                */
                todStep.nanoSeconds = todStepArr[ii].nanoSeconds;
                todStep.fracNanoSeconds = todStepArr[ii].fracNanoSeconds;

                st = cpssPxPtpTaiTodStepSet(devNum, &todStep);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

                /*
                    1.2 Call cpssPxPtpTaiTodStepGet().
                    Expected: GT_OK.
                */
                st = cpssPxPtpTaiTodStepGet(devNum, &todStepGet);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);
                UTF_VERIFY_EQUAL1_PARAM_MAC(todStep.nanoSeconds,
                                            todStepGet.nanoSeconds,
                                            devNum);
                UTF_VERIFY_EQUAL1_PARAM_MAC(todStep.fracNanoSeconds,
                                            todStepGet.fracNanoSeconds,
                                            devNum);
            }
        }
        /* 2.For not applicable devices check that function returns non GT_OK.*/
        /* prepare device iterator to go through all non-applicable devices */
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_NONE_FAMILY_E);

        /* Go over all non-applicable devices. */
        while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
        {
            st = cpssPxPtpTaiTodStepSet(devNum, &todStep);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
        }

        /*3. Call function with out of bound value for device id.*/
        devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;
        st = cpssPxPtpTaiTodStepSet(devNum, &todStep);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*
GT_STATUS cpssPxPtpTaiGracefulStepSet
(
    IN  GT_SW_DEV_NUM                      devNum,
    IN  GT_U32                             gracefulStep
);
*/
UTF_TEST_CASE_MAC(cpssPxPtpTaiGracefulStepSet)
{
/*
    ITERATE_DEVICES(Pipe)
    1.1 Call with
            gracefulStep     [0 / 15 / 31]
    Expected: GT_OK.
    1.2 Call cpssPxPtpTaiGracefulStepGet().
    Expected: GT_OK.
*/
    GT_STATUS                    st      =  GT_OK;
    GT_U8                        devNum  =  0;
    GT_U32                       gracefulStep;
    GT_U32                       gracefulStepGet;
    GT_U32                       ii;
    GT_U32                       gracefulStepArr[] = {0, 15, 31};

    /* there is no TAI in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        for(ii = 0; ii < 3; ii++)
        {
            /*
                1.1 Call with
                        gracefulStep     [0 / 15 / 31]
            */
            gracefulStep = gracefulStepArr[ii];
            st = cpssPxPtpTaiGracefulStepSet(devNum, gracefulStep);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

            /*
                1.2 Call cpssPxPtpTaiGracefulStepGet().
                Expected: GT_OK.
            */
            st = cpssPxPtpTaiGracefulStepGet(devNum, &gracefulStepGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);
            UTF_VERIFY_EQUAL1_PARAM_MAC(gracefulStep, gracefulStepGet, devNum);
        }
    }

    /* restore valid values */
    gracefulStep     = 0;

    /* 2.For not applicable devices check that function returns non GT_OK.*/
    /* prepare device iterator to go through all non-applicable devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_NONE_FAMILY_E);

    /* Go over all non-applicable devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssPxPtpTaiGracefulStepSet(devNum, gracefulStep);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3. Call function with out of bound value for device id. */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssPxPtpTaiGracefulStepSet(devNum, gracefulStep);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPtpTaiGracefulStepGet
(
    IN  GT_SW_DEV_NUM                     devNum,
    OUT GT_U32                            *gracefulStepPtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPtpTaiGracefulStepGet)
{
   /*
        ITERATE_DEVICES(Pipe)
        1.1 Call with valid parameters.
        Expected: GT_OK.
        1.2 Call with invalid gracefulStepPtr [NULL].
        Expected: GT_BAD_PTR.
    */

    GT_STATUS                    st      =  GT_OK;
    GT_U8                        devNum  =  0;
    GT_U32                       gracefulStep;

    /* there is no TAI in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /*
            1.1 Call with valid parameters.
            Expected: GT_OK.
        */
        st = cpssPxPtpTaiGracefulStepGet(devNum, &gracefulStep);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

        /*
           1.2 Call with invalid gracefulStepPtr [NULL].
           Expected: GT_BAD_PTR.
        */
        st = cpssPxPtpTaiGracefulStepGet(devNum, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, enabled = NULL",
                                     devNum);
    }

    /* 2. For not applicable devices check that function returns non GT_OK.*/
    /* prepare device iterator to go through all non-applicable devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_NONE_FAMILY_E);

    /* Go over all non-applicable devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssPxPtpTaiGracefulStepGet(devNum, &gracefulStep);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /*3. Call function with out of bound gracefulStep for device id.*/
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssPxPtpTaiGracefulStepGet(devNum, &gracefulStep);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPtpTaiTodUpdateCounterGet
(
    IN  GT_SW_DEV_NUM                         devNum,
    OUT GT_U32                                *valuePtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPtpTaiTodUpdateCounterGet)
{
   /*
        ITERATE_DEVICES(Pipe)
        1.1 Call with valid params.
        Expected: GT_OK.
        1.2 Call with invalid valuePtr [NULL].
        Expected: GT_BAD_PTR.
    */

    GT_STATUS                    st     = GT_OK;
    GT_U8                        devNum = 0;
    GT_U32                       value;

    /* there is no TAI in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /*
            1.1 Call with valid params.
            Expected: GT_OK.
        */
        st = cpssPxPtpTaiTodUpdateCounterGet(devNum, &value);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

        /*
           1.2 Call with invalid nanoSeconsPtr [NULL].
           Expected: GT_BAD_PTR.
        */
        st = cpssPxPtpTaiTodUpdateCounterGet(devNum, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, enabled = NULL",
            devNum);
    }

    /* 2. For not applicable devices check that function returns non GT_OK.*/
    /* prepare device iterator to go through all non-applicable devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_NONE_FAMILY_E);

    /* Go over all non-applicable devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssPxPtpTaiTodUpdateCounterGet(devNum, &value);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /*3. Call function with out of bound value for device id.*/
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssPxPtpTaiTodUpdateCounterGet(devNum, &value);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPtpTsuControlGet
(
    IN  GT_SW_DEV_NUM                     devNum,
    IN  GT_PHYSICAL_PORT_NUM              portNum,
    OUT CPSS_PX_PTP_TSU_CONTROL_STC       *controlPtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPtpTsuControlGet)
{
/*
    ITERATE_DEVICES(Pipe)
    1.1 Call with non-null controlPrt.
    Expected: GT_OK
    1.2 Call with null controlPrt.
    Expected: GT_BAD_PTR
    1.3. For active device for out of bound value for port number.
    Expected: GT_BAD_PARAM
*/
    GT_STATUS                         st      =  GT_OK;
    GT_U8                             devNum  =  0;
    UTF_PHYSICAL_PORT_NUM             port;
    CPSS_PX_PTP_TSU_CONTROL_STC       control = {GT_TRUE};

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports.*/
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            /*
                1.1 Call with non-null controlPrt.
                Expected: GT_OK
            */
            st = cpssPxPtpTsuControlGet(devNum, port, &control);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

            /*
                1.2 Call with null controlPrt.
                Expected: GT_BAD_PTR
            */
            st = cpssPxPtpTsuControlGet(devNum, port, NULL);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, devNum);
        }

        /* 1.3. For active device for out of bound value for port number.*/
        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(devNum);
        st = cpssPxPtpTsuControlGet(devNum, port, &control);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, port);
    }

    /*set port valid value*/
    control.unitEnable          = GT_TRUE;
    port                        = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_NONE_FAMILY_E);

    /* 2. Go over all non-applicable devices.*/
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssPxPtpTsuControlGet(devNum, port, &control);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3. Call function with out of bound value for device id.*/
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssPxPtpTsuControlGet(devNum, port, &control);
    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, port);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPtpTsuControlSet
(
    IN  GT_SW_DEV_NUM                     devNum,
    IN  GT_PHYSICAL_PORT_NUM              portNum,
    IN  CPSS_PX_PTP_TSU_CONTROL_STC       *controlPtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPtpTsuControlSet)
{
/*
    ITERATE_DEVICES(Pipe)
    1.1 Call with controlPtr {
        control.unitEnable          [GT_TRUE / GT_FALSE]
    }
    Expected: GT_OK
    1.2 Call cpssPxPtpTsuControlGet().
    Expected: GT_OK
    1.3 Call with null controlPrt.
    Expected: GT_BAD_PTR
*/
    GT_STATUS                     st      =  GT_OK;
    GT_U8                         devNum  =  0;
    UTF_PHYSICAL_PORT_NUM         port;
    CPSS_PX_PTP_TSU_CONTROL_STC   control = {GT_TRUE};
    CPSS_PX_PTP_TSU_CONTROL_STC   controlGet;

    /* there is no TAI and PTP in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports.*/
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            /*
                1.1 Call with controlPtr {
                        control.unitEnable          [GT_TRUE]
                }
                Expected: GT_OK
            */
            control.unitEnable          = GT_TRUE;
            st = cpssPxPtpTsuControlSet(devNum, port, &control);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

            /*
                1.2 Call cpssPxPtpTsuControlGet().
                Expected: GT_OK.
            */
            st = cpssPxPtpTsuControlGet(devNum, port, &controlGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

            /* verifying values */
            UTF_VERIFY_EQUAL2_PARAM_MAC(control.unitEnable,
                                        controlGet.unitEnable, devNum, port);

            /*
                1.1 Call with controlPtr {
                        control.unitEnable          [GT_FALSE]
                }
                Expected: GT_OK
            */
            control.unitEnable          = GT_FALSE;
            st = cpssPxPtpTsuControlSet(devNum, port, &control);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

            /*
                1.2 Call cpssPxPtpTsuControlGet().
                Expected: GT_OK.
            */
            st = cpssPxPtpTsuControlGet(devNum, port, &controlGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

            /* verifying values */
            UTF_VERIFY_EQUAL2_PARAM_MAC(control.unitEnable,
                                        controlGet.unitEnable, devNum, port);

            /*
                1.3 Call with null controlPrt.
                Expected: GT_BAD_PARAM
            */
            st = cpssPxPtpTsuControlSet(devNum, port, NULL);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, devNum);
        }
    }

    /*set port valid value*/
    control.unitEnable          = GT_TRUE;
    port                         = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_NONE_FAMILY_E);

    /* 2. Go over all non-applicable devices.*/
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssPxPtpTsuControlSet(devNum, port, &control);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3. Call function with out of bound value for device id.*/
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssPxPtpTsuControlSet(devNum, port, &control);
    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, port);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPtpTsuCountersClear
(
    IN  GT_SW_DEV_NUM                     devNum,
    IN  GT_PHYSICAL_PORT_NUM              portNum
)
*/
UTF_TEST_CASE_MAC(cpssPxPtpTsuCountersClear)
{
/*
    ITERATE_DEVICES(Pipe)
    1.1. For all active devices go over all available physical ports.
    1.2. For active device for out of bound value for port number.
*/
    GT_STATUS              st      =  GT_OK;
    GT_U8                  devNum  =  0;
    UTF_PHYSICAL_PORT_NUM  port;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports.*/
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {

            st = cpssPxPtpTsuCountersClear(devNum, port);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);
        }

        /* 1.2. For active device for out of bound value for port number.*/
        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(devNum);

        st = cpssPxPtpTsuCountersClear(devNum, port);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, port);
    }

    /*set port valid value*/
    port = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_NONE_FAMILY_E);
    /* 2. Go over all non-applicable devices.*/
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssPxPtpTsuCountersClear(devNum, port);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3. Call function with out of bound value for device id.*/
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssPxPtpTsuCountersClear(devNum, port);
    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, port);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPtpTsuPacketCouterGet
(
    IN  GT_SW_DEV_NUM                             devNum,
    IN  GT_PHYSICAL_PORT_NUM                      portNum,
    IN  CPSS_PX_PTP_TSU_PACKET_COUNTER_TYPE_ENT   counterType,
    OUT GT_U32                                    *valuePtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPtpTsuPacketCouterGet)
{
/*
    ITERATE_DEVICES(Pipe)
    1.1.1 Call with counterType
        [CPSS_PX_PTP_TSU_PACKET_COUNTER_TYPE_TOTAL_E   /
        CPSS_PX_PTP_TSU_PACKET_COUNTER_TYPE_FORWARD_E /
        CPSS_PX_PTP_TSU_PACKET_COUNTER_TYPE_CAPTURE_INGRESS_TIME_E]
    Expected: GT_OK
    1.1.2 Call with NULL value
    Expected: GT_BAD_PTR
    1.2  Call with counterType out of range
         and other params from 1.1.1.
    Expected: GT_BAD_PARAM.
    1.3. For active device for out of bound value for port number.
*/
    GT_STATUS                                 st      =  GT_OK;
    GT_U8                                     devNum  =  0;
    UTF_PHYSICAL_PORT_NUM                     port;
    CPSS_PX_PTP_TSU_PACKET_COUNTER_TYPE_ENT   counterType;
    GT_U32                                    value;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports.*/
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1 Call with counterType
                [CPSS_PX_PTP_TSU_PACKET_COUNTER_TYPE_TOTAL_E]
                Expected: GT_OK
            */
            counterType = CPSS_PX_PTP_TSU_PACKET_COUNTER_TYPE_TOTAL_E;
            st = cpssPxPtpTsuPacketCouterGet(devNum, port, counterType, &value);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

            /*
                1.1.1 Call with counterType
                [CPSS_PX_PTP_TSU_PACKET_COUNTER_TYPE_PTP_V1_E]
                Expected: GT_OK
            */
            counterType = CPSS_PX_PTP_TSU_PACKET_COUNTER_TYPE_PTP_V1_E;
            st = cpssPxPtpTsuPacketCouterGet(devNum, port, counterType, &value);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

            /*
                1.1.1 Call with counterType
                [CPSS_PX_PTP_TSU_PACKET_COUNTER_TYPE_PTP_V2_E]
                Expected: GT_OK
            */
            counterType =
                   CPSS_PX_PTP_TSU_PACKET_COUNTER_TYPE_PTP_V2_E;
            st = cpssPxPtpTsuPacketCouterGet(devNum, port, counterType, &value);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

            /*
                1.1.2 Call with NULL value
                Expected: GT_BAD_PARAM
            */
            counterType = CPSS_PX_PTP_TSU_PACKET_COUNTER_TYPE_TOTAL_E;
            st = cpssPxPtpTsuPacketCouterGet(devNum, port, counterType, NULL);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, devNum);
        }

        /*
           1.2  Call with counterType out of range
                 and other params from 1.1.1.
           Expected: GT_BAD_PARAM.
        */
        /* set valid parameters*/
        port        = 0;
        counterType = CPSS_PX_PTP_TSU_PACKET_COUNTER_TYPE_TOTAL_E;
        UTF_ENUMS_CHECK_MAC(cpssPxPtpTsuPacketCouterGet
            (devNum, port, counterType, &value), counterType);

        /* 1.3. For active device for out of bound value for port number.*/
        counterType = CPSS_PX_PTP_TSU_PACKET_COUNTER_TYPE_TOTAL_E;
        port        = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(devNum);

        st = cpssPxPtpTsuPacketCouterGet(devNum, port, counterType, &value);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, port);
    }

    /*set port valid value*/
    port        = 0;
    counterType = CPSS_PX_PTP_TSU_PACKET_COUNTER_TYPE_TOTAL_E;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_NONE_FAMILY_E);
    /* 2. Go over all non-applicable devices.*/
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssPxPtpTsuPacketCouterGet(devNum, port, counterType, &value);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3. Call function with out of bound value for device id.*/
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssPxPtpTsuPacketCouterGet(devNum, port, counterType, &value);
    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, port);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPtpTaiInputClockSelectSet
(
    IN  GT_SW_DEV_NUM                     devNum,
    IN  CPSS_PX_PTP_TAI_CLOCK_SELECT_ENT  clockSelect
)
*/
UTF_TEST_CASE_MAC(cpssPxPtpTaiInputClockSelectSet)
{
/*
    ITERATE_DEVICES(Pipe)
    1.1. Call cpssPxPtpTaiInputClockSelectSet with
            clockSelect[CPSS_PX_PTP_TAI_CLOCK_SELECT_PTP_PLL_E /CPSS_PX_PTP_TAI_CLOCK_SELECT_CORE_PLL_E],
    Expected: GT_OK.
    1.2. Call cpssPxPtpTaiInputClockSelectGet
           with the same parameters.
    Expected: GT_OK.
    1.3. Call cpssPxPtpTaiInputClockSelectGet with invalid clockSelect.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS st = GT_OK;

    GT_U8                           dev;
    CPSS_PX_PTP_TAI_CLOCK_SELECT_ENT  clockSelect[2] =
        {CPSS_PX_PTP_TAI_CLOCK_SELECT_PTP_PLL_E, CPSS_PX_PTP_TAI_CLOCK_SELECT_CORE_PLL_E};
    CPSS_PX_PTP_TAI_CLOCK_SELECT_ENT  clockSelectGet[2] =
        {CPSS_PX_PTP_TAI_CLOCK_SELECT_PTP_PLL_E, CPSS_PX_PTP_TAI_CLOCK_SELECT_CORE_PLL_E};

    GT_U32 j;

    /* there is no TAI and PTP in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for(j = 0; j < CPSS_PX_PTP_TAI_CLOCK_SELECT_CORE_PLL_E + 1; j++)
        {
            /*
                1.1. Call with  clockSelect[CPSS_PX_PTP_TAI_CLOCK_SELECT_PTP_PLL_E /CPSS_PX_PTP_TAI_CLOCK_SELECT_CORE_PLL_E].
                Expected: GT_OK.
            */
            st = cpssPxPtpTaiInputClockSelectSet(dev, clockSelect[j]);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
                1.2. Call cpssPxPtpTaiInputClockSelectGet with the same parameters.
                Expected: GT_OK and the same values than was set.
            */
            st = cpssPxPtpTaiInputClockSelectGet(dev, &clockSelectGet[j]);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                        "cpssPxPtpTaiInputClockSelectGet: %d ", dev);

            /* Verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(clockSelect[j], clockSelectGet[j],
                            "got another clockSelect then was set: %d", dev);
        }
        /*
            1.3. Call cpssPxPtpTaiInputClockSelectGet with invalid clockSelect.
            Expected: GT_BAD_PARAM.
        */
        st = cpssPxPtpTaiInputClockSelectSet(dev, 10);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPtpTaiInputClockSelectGet
(
    IN  GT_SW_DEV_NUM                     devNum,
    OUT CPSS_PX_PTP_TAI_CLOCK_SELECT_ENT  *clockSelectPtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPtpTaiInputClockSelectGet)
{
/*
    ITERATE_DEVICES(Pipe)
    1.1.1 Call api with non-null clockSelectPtr
    Expected: GT_OK.
    1.1.2 Call api with wrong clockSelectPtr[NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS                             st = GT_OK;
    GT_U8                                 devNum = 0;
    OUT CPSS_PX_PTP_TAI_CLOCK_SELECT_ENT  clockSelect;

    /* there is no TAI and PTP in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /*
            1.1.1 Call with non-null clockInterfaceDirectionPtr
            Expected: GT_OK.
        */
        st = cpssPxPtpTaiInputClockSelectGet(devNum, &clockSelect);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

        /*
            1.1.2 Call api with wrong clockSelectPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssPxPtpTaiInputClockSelectGet(devNum, NULL);
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_NONE_FAMILY_E);

    /* 2. Go over all non-applicable devices.*/
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssPxPtpTaiInputClockSelectGet(devNum, &clockSelect);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3. Call function with out of bound value for device id.*/
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssPxPtpTaiInputClockSelectGet(devNum, &clockSelect);
    UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPtpPortTypeKeySet
(
    IN  GT_SW_DEV_NUM                        devNum,
    IN  GT_PHYSICAL_PORT_NUM                 portNum,
    IN  CPSS_PX_PTP_TYPE_KEY_STC *portKeyPtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPtpPortTypeKeySet)
{
    /*
        ITERATE_DEVICES(Pipe)
        1.1 ITERATE_PORTS
        1.1.1  Call with valid parameters and verify
        Expected: GT_OK.
        1.1.2  Call with bad pointer to portKeyPtr
        Expected: GT_BAD_PTR.
        1.1.3  Call with bad anchor type
        Expected: GT_BAD_PARAM.
        1.1.4  Call with bad offset
        Expected: GT_OUT_OF_RANGE.
        1.2  Call with invalid portNum
        Expected: GT_BAD_PARAM.
        1.3  Call with higher than max portNum
        Expected: GT_BAD_PARAM.
    */
    CPSS_PX_PTP_TYPE_KEY_STC portKey = {
        {
            {CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L2_E, 127},
            {CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_PTP_E, 0}
        },
        {
            {CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L2_E, 1},
            {CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L3_E, 2},
            {CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L4_E, 4},
            {CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_PTP_E, 8},
            {CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L2_E, 16},
            {CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L3_E, 127}
        }
    };
    CPSS_PX_PTP_TYPE_KEY_STC portKeyGet;
    GT_STATUS                            st      =  GT_OK;
    GT_U8                                devNum  =  0;
    UTF_PHYSICAL_PORT_NUM                port = 0;
    GT_U32                               ii;
    CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_ENT  anchor;
    GT_U32                               offset;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&port, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports.*/
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1  Call with valid parameters and verify
                Expected: GT_OK.
            */
            st = cpssPxPtpPortTypeKeySet(devNum, port, &portKey);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

            st = cpssPxPtpPortTypeKeyGet(devNum, port, &portKeyGet);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
            for (ii = 0; ii < CPSS_PX_PTP_TYPE_UDB_NUM_CNS; ii++)
            {
                UTF_VERIFY_EQUAL0_PARAM_MAC(portKey.portUdbArr[ii].udbAnchorType,
                                         portKeyGet.portUdbArr[ii].udbAnchorType);
                UTF_VERIFY_EQUAL0_PARAM_MAC(portKey.portUdbArr[ii].udbByteOffset,
                                         portKeyGet.portUdbArr[ii].udbByteOffset);
            }

            for (ii = 0; ii < CPSS_PX_PTP_TYPE_UDBP_NUM_CNS; ii++)
            {
                UTF_VERIFY_EQUAL0_PARAM_MAC(
                    portKey.portUdbPairArr[ii].udbAnchorType,
                    portKeyGet.portUdbPairArr[ii].udbAnchorType);
                UTF_VERIFY_EQUAL0_PARAM_MAC(
                    portKey.portUdbPairArr[ii].udbByteOffset,
                    portKeyGet.portUdbPairArr[ii].udbByteOffset);
            }

            /*
                1.1.2  Call with bad pointer to portKeyPtr
                Expected: GT_BAD_PTR.
            */
            st = cpssPxPtpPortTypeKeySet(devNum, port, NULL);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, st);

            /*
                1.1.3  Call with bad anchor type
                Expected: GT_BAD_PARAM.
            */
            for (ii = 0; ii < CPSS_PX_PTP_TYPE_UDB_NUM_CNS; ii++)
            {
                anchor = portKey.portUdbArr[ii].udbAnchorType;
                portKey.portUdbArr[ii].udbAnchorType =
                    CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_PTP_E + 1;
                st = cpssPxPtpPortTypeKeySet(devNum, port, &portKey);
                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PARAM, st);
                portKey.portUdbArr[ii].udbAnchorType = anchor;
            }

            for (ii = 0; ii < CPSS_PX_PTP_TYPE_UDBP_NUM_CNS; ii++)
            {
                anchor = portKey.portUdbPairArr[ii].udbAnchorType;
                portKey.portUdbPairArr[ii].udbAnchorType =
                    CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_PTP_E + 1;
                st = cpssPxPtpPortTypeKeySet(devNum, port, &portKey);
                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PARAM, st);
                portKey.portUdbPairArr[ii].udbAnchorType = anchor;
            }

            /*
                1.1.4  Call with bad offset
                Expected: GT_OUT_OF_RANGE.
            */
            for (ii = 0; ii < CPSS_PX_PTP_TYPE_UDB_NUM_CNS; ii++)
            {
                offset = portKey.portUdbArr[ii].udbByteOffset;
                portKey.portUdbArr[ii].udbByteOffset =
                    CPSS_PX_PTP_UDB_OFFSET_MAX_CNS + 100;
                st = cpssPxPtpPortTypeKeySet(devNum, port, &portKey);
                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OUT_OF_RANGE, st);
                portKey.portUdbArr[ii].udbByteOffset = offset;
            }

            for (ii = 0; ii < CPSS_PX_PTP_TYPE_UDBP_NUM_CNS; ii++)
            {
                offset = portKey.portUdbPairArr[ii].udbByteOffset;
                portKey.portUdbPairArr[ii].udbByteOffset =
                    CPSS_PX_PTP_UDB_OFFSET_MAX_CNS + 100;
                st = cpssPxPtpPortTypeKeySet(devNum, port, &portKey);
                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OUT_OF_RANGE, st);
                portKey.portUdbPairArr[ii].udbByteOffset = offset;
            }
        }

        /* 1.2. For all active devices go over all non available
           physical ports.
        */
        st = prvUtfNextPhyPortReset(&port, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            st = cpssPxPtpPortTypeKeySet(devNum, port, &portKey);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PARAM, st);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(devNum);
        st = cpssPxPtpPortTypeKeySet(devNum, port, &portKey);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PARAM, st);

    }
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_NONE_FAMILY_E);

    /* 2. Go over all non-applicable devices.*/
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssPxPtpPortTypeKeySet(devNum, port, &portKey);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PARAM, st);
    }

    /* 3. Call function with out of bound value for device id.*/
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssPxPtpPortTypeKeySet(devNum, port, &portKey);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PARAM, st);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPtpPortTypeKeyGet
(
    IN  GT_SW_DEV_NUM                        devNum,
    IN  GT_PHYSICAL_PORT_NUM                 portNum,
    OUT CPSS_PX_PTP_TYPE_KEY_STC *portKeyPtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPtpPortTypeKeyGet)
{
    /*
        ITERATE_DEVICES(Pipe)
        1.1 ITERATE_PORTS
        1.1.1  Call with valid parameters
        Expected: GT_OK.
        1.1.2  Call with bad pointer to portKeyPtr
        Expected: GT_BAD_PTR.
        1.2  Call with invalid portNum
        Expected: GT_BAD_PARAM.
        1.3  Call with higher than max portNum
        Expected: GT_BAD_PARAM.
    */
    CPSS_PX_PTP_TYPE_KEY_STC portKeyGet;
    GT_STATUS                            st      =  GT_OK;
    GT_U8                                devNum  =  0;
    UTF_PHYSICAL_PORT_NUM                port = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&port, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports.*/
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        { 
            /*
                1.1.1  Call with valid parameters and verify
                Expected: GT_OK.
            */
            st = cpssPxPtpPortTypeKeyGet(devNum, port, &portKeyGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, port);

            /*
                1.1.2  Call with bad pointer to portKeyPtr
                Expected: GT_BAD_PTR.
            */
            st = cpssPxPtpPortTypeKeyGet(devNum, port, NULL);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, devNum, port);
        }

        /* 1.2. For all active devices go over all non available
           physical ports.
        */
        st = prvUtfNextPhyPortReset(&port, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            st = cpssPxPtpPortTypeKeyGet(devNum, port, &portKeyGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(devNum);
        st = cpssPxPtpPortTypeKeyGet(devNum, port, &portKeyGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, port);;

    }
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_NONE_FAMILY_E);

    /* 2. Go over all non-applicable devices.*/
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssPxPtpPortTypeKeyGet(devNum, port, &portKeyGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, port);
    }

    /* 3. Call function with out of bound value for device id.*/
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssPxPtpPortTypeKeyGet(devNum, port, &portKeyGet);
    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, port);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPtpTypeKeyEntrySet
(
    IN  GT_SW_DEV_NUM                           devNum,
    IN  CPSS_PX_PTP_TYPE_INDEX                  ptpTypeIndex,
    IN  CPSS_PX_PTP_TYPE_KEY_FORMAT_STC         *keyDataPtr,
    IN  CPSS_PX_PTP_TYPE_KEY_FORMAT_STC         *keyMaskPtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPtpTypeKeyEntrySet)
{
    /*
        ITERATE_DEVICES(Pipe)
        1.1. Call with valid parameters
        Expected: GT_OK.
        1.2. Verify with cpssPxPtpTypeKeyEntryGet.
        Expected: GT_OK.
        1.3. Call with invalid srcPortProfile.
        Expected: GT_OUT_OF_RANGE.
        1.4. Call with invalid PTP type.
        Expected: GT_BAD_PARAM.
        1.5. Call with bad pointer to keyDataPtr.
        Expected: GT_BAD_PTR.
        1.6. Call with bad pointer to keyMaskPtr.
        Expected: GT_BAD_PTR.
    */
    GT_STATUS                               st      =  GT_OK;
    GT_U8                                   devNum  =  0;
    CPSS_PX_PTP_TYPE_KEY_FORMAT_STC         keyData =
        {GT_TRUE, 127, {{{255, 0}}, {{0, 255}}}, {0, 4, 8, 16, 32, 64}};
    CPSS_PX_PTP_TYPE_KEY_FORMAT_STC         keyMask =
        {GT_FALSE, 0, {{{254, 0}}, {{0, 254}}}, {255, 3, 7, 15, 31, 63}};
    CPSS_PX_PTP_TYPE_KEY_FORMAT_STC         keyDataGet;
    CPSS_PX_PTP_TYPE_KEY_FORMAT_STC         keyMaskGet;
    CPSS_PX_PTP_TYPE_INDEX                  ptpTypeIndex =
        CPSS_PX_PTP_TYPE_NUM_CNS - 1;
    GT_U32                                  ii;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /*
            1.1. Call with valid parameters
            Expected: GT_OK.
        */
        st = cpssPxPtpTypeKeyEntrySet(devNum, ptpTypeIndex,
            &keyData, &keyMask);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, ptpTypeIndex);

        /*  1.2. Verify with cpssPxPtpTypeKeyEntryGet.
            Expected: GT_OK.
        */
        st = cpssPxPtpTypeKeyEntryGet(devNum, ptpTypeIndex,
            &keyDataGet, &keyMaskGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, ptpTypeIndex);
        UTF_VERIFY_EQUAL0_PARAM_MAC(keyData.isUdp, keyDataGet.isUdp);
        UTF_VERIFY_EQUAL0_PARAM_MAC(keyData.srcPortProfile,
            keyDataGet.srcPortProfile);
        for (ii = 0; ii < CPSS_PX_PTP_TYPE_UDB_NUM_CNS; ii++)
        {
            UTF_VERIFY_EQUAL0_PARAM_MAC(keyData.udbArr[ii],
                                                keyDataGet.udbArr[ii]);
        }

        for (ii = 0; ii < CPSS_PX_PTP_TYPE_UDBP_NUM_CNS; ii++)
        {
            UTF_VERIFY_EQUAL0_PARAM_MAC(keyData.udbPairsArr[ii].udb[0],
                                     keyDataGet.udbPairsArr[ii].udb[0]);
            UTF_VERIFY_EQUAL0_PARAM_MAC(keyData.udbPairsArr[ii].udb[1],
                                     keyDataGet.udbPairsArr[ii].udb[1]);
        }

        /*  1.3. Call with invalid srcPortProfile.
            Expected: GT_OUT_OF_RANGE.
        */
        keyData.srcPortProfile = CPSS_PX_INGRESS_SOURCE_PORT_PROFILE_MAX_CNS;
        st = cpssPxPtpTypeKeyEntrySet(devNum, ptpTypeIndex,
            &keyData, &keyMask);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OUT_OF_RANGE, st, devNum, ptpTypeIndex);
        keyData.srcPortProfile = 127;

        /*
            1.4. Call with invalid PTP type.
            Expected: GT_BAD_PARAM.
        */
        ptpTypeIndex = CPSS_PX_PTP_TYPE_NUM_CNS;
        st = cpssPxPtpTypeKeyEntrySet(devNum, ptpTypeIndex,
            &keyData, &keyMask);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, ptpTypeIndex);
        ptpTypeIndex = CPSS_PX_PTP_TYPE_NUM_CNS - 1;

        /*
            1.5. Call with bad pointer to keyDataPtr.
            Expected: GT_BAD_PTR.
        */
        st = cpssPxPtpTypeKeyEntrySet(devNum, ptpTypeIndex,
            NULL, &keyMaskGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, devNum, ptpTypeIndex);

        /*  
            1.6. Call with bad pointer to keyMaskPtr.
            Expected: GT_BAD_PTR.
        */
         st = cpssPxPtpTypeKeyEntrySet(devNum, ptpTypeIndex,
            &keyDataGet, NULL);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, devNum, ptpTypeIndex);

    }

    /* 2. For not applicable devices check that function returns non GT_OK.*/
    /* prepare device iterator to go through all non-applicable devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_NONE_FAMILY_E);

    /* Go over all non-applicable devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssPxPtpTypeKeyEntrySet(devNum, ptpTypeIndex,
            &keyData, &keyMask);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum,
            ptpTypeIndex);
    }

    /*3. Call function with out of bound value for device id.*/
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPtpTypeKeyEntrySet(devNum, ptpTypeIndex,
            &keyData, &keyMask);
    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, ptpTypeIndex);

}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPtpTypeKeyEntryGet
(
    IN  GT_SW_DEV_NUM                           devNum,
    IN  CPSS_PX_PTP_TYPE_INDEX                  ptpTypeIndex,
    OUT CPSS_PX_PTP_TYPE_KEY_FORMAT_STC         *keyDataPtr,
    OUT CPSS_PX_PTP_TYPE_KEY_FORMAT_STC         *keyMaskPtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPtpTypeKeyEntryGet)
{
    /*
        ITERATE_DEVICES(Pipe)
        1.1. Call with valid parameters
        Expected: GT_OK.
        1.2. Call with invalid PTP type.
        Expected: GT_BAD_PARAM.
        1.3. Call with bad pointer to keyDataPtr.
        Expected: GT_BAD_PTR.
        1.4. Call with bad pointer to keyMaskPtr.
        Expected: GT_BAD_PTR.
    */
    GT_STATUS                               st      =  GT_OK;
    GT_U8                                   devNum  =  0;
    CPSS_PX_PTP_TYPE_KEY_FORMAT_STC         keyDataGet;
    CPSS_PX_PTP_TYPE_KEY_FORMAT_STC         keyMaskGet;
    CPSS_PX_PTP_TYPE_INDEX                  ptpTypeIndex =
        CPSS_PX_PTP_TYPE_NUM_CNS - 1;

        /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /*
            1.1. Call with valid parameters
            Expected: GT_OK.
        */
        st = cpssPxPtpTypeKeyEntryGet(devNum, ptpTypeIndex,
            &keyDataGet, &keyMaskGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, ptpTypeIndex);

        /*
            1.2. Call with invalid PTP type.
            Expected: GT_BAD_PARAM.
        */
        ptpTypeIndex = CPSS_PX_PTP_TYPE_NUM_CNS;
        st = cpssPxPtpTypeKeyEntryGet(devNum, ptpTypeIndex,
            &keyDataGet, &keyMaskGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, ptpTypeIndex);
        ptpTypeIndex = CPSS_PX_PTP_TYPE_NUM_CNS - 1;

        /*
            1.3. Call with bad pointer to keyDataPtr.
            Expected: GT_BAD_PTR.
        */
        st = cpssPxPtpTypeKeyEntryGet(devNum, ptpTypeIndex,
            NULL, &keyMaskGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, devNum, ptpTypeIndex);

        /*  
            1.4. Call with bad pointer to keyMaskPtr.
            Expected: GT_BAD_PTR.
        */
         st = cpssPxPtpTypeKeyEntryGet(devNum, ptpTypeIndex,
            &keyDataGet, NULL);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, devNum, ptpTypeIndex);
    }

    /* 2. For not applicable devices check that function returns non GT_OK.*/
    /* prepare device iterator to go through all non-applicable devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_NONE_FAMILY_E);

    /* Go over all non-applicable devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssPxPtpTypeKeyEntryGet(devNum, ptpTypeIndex,
            &keyDataGet, &keyMaskGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum,
            ptpTypeIndex);
    }

    /*3. Call function with out of bound value for device id.*/
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPtpTypeKeyEntryGet(devNum, ptpTypeIndex,
            &keyDataGet, &keyMaskGet);
    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, ptpTypeIndex);
}

/*----------------------------------------------------------------------------*/
/*
 GT_STATUS cpssPxPtpTypeKeyEntryEnableGet
(
    IN  GT_SW_DEV_NUM       devNum,
    IN  CPSS_PX_PTP_TYPE_INDEX ptpTypeIndex,
    OUT GT_BOOL             *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPtpTypeKeyEntryEnableGet)
{
/*
    ITERATE_DEVICES(Pipe)
    1.1. Call with valid parameters
    Expected: GT_OK.
    1.2. Call with invalid enabledPtr [NULL].
    Expected: GT_BAD_PTR.
    1.3. Call with invalid PTP type.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS                    st      =  GT_OK;
    GT_U8                        devNum  =  0;
    GT_BOOL                      enable  =  GT_FALSE;
    CPSS_PX_PTP_TYPE_INDEX       ptpTypeIndex = CPSS_PX_PTP_TYPE_NUM_CNS - 1;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /*
            1.1. Call with valid parameters
            Expected: GT_OK.
        */
        st = cpssPxPtpTypeKeyEntryEnableGet(devNum, ptpTypeIndex, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, ptpTypeIndex);

        /*
           1.2 Call with invalid enabledPtr [NULL].
           Expected: GT_BAD_PTR.
        */
        st = cpssPxPtpTypeKeyEntryEnableGet(devNum, ptpTypeIndex, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, enabled = NULL",
                                     devNum);

        /*
           1.3. Call with invalid PTP type.
           Expected: GT_BAD_PARAM.
        */
        ptpTypeIndex = CPSS_PX_PTP_TYPE_NUM_CNS;
        st = cpssPxPtpTypeKeyEntryEnableGet(devNum, ptpTypeIndex, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, ptpTypeIndex);
        ptpTypeIndex = CPSS_PX_PTP_TYPE_NUM_CNS - 1;
    }

    /* restore valid values */
    enable  = GT_TRUE;

    /* 2. For not applicable devices check that function returns non GT_OK.*/
    /* prepare device iterator to go through all non-applicable devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_NONE_FAMILY_E);

    /* Go over all non-applicable devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssPxPtpTypeKeyEntryEnableGet(devNum, ptpTypeIndex, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum,
            ptpTypeIndex);
    }

    /*3. Call function with out of bound value for device id.*/
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPtpTypeKeyEntryEnableGet(devNum, ptpTypeIndex, &enable);
    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, ptpTypeIndex);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPtpTypeKeyEntryEnableSet
(
    IN  GT_SW_DEV_NUM       devNum,
    IN  CPSS_PX_PTP_TYPE_INDEX ptpTypeIndex,
    IN  GT_BOOL             enable
)
*/
UTF_TEST_CASE_MAC(cpssPxPtpTypeKeyEntryEnableSet)
{
    /*
        ITERATE_DEVICES(Pipe)
        1.1. Call with valid parameters
        Expected: GT_OK.
        1.2. Call cpssPxPtpTypeKeyEntryEnableGet() to verify.
        Expected: GT_OK
        1.3. Call with invalid PTP type.
        Expected: GT_BAD_PARAM.

    */
        GT_STATUS                    st      =  GT_OK;
        GT_U8                        devNum  =  0;
        GT_BOOL                      apEnable     =  GT_FALSE;
        GT_BOOL                      apEnableGet  =  GT_FALSE;
        CPSS_PX_PTP_TYPE_INDEX       ptpTypeIndex = CPSS_PX_PTP_TYPE_NUM_CNS - 1;

        /* prepare device iterator */
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_NONE_FAMILY_E);

        /* 1. Go over all active devices. */
        while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
        {
            /*
                1.1 Call with valid parameters
                Expected: GT_OK.
            */
            apEnable = GT_TRUE;
            ptpTypeIndex = CPSS_PX_PTP_TYPE_NUM_CNS - 1;

            st = cpssPxPtpTypeKeyEntryEnableSet(devNum, ptpTypeIndex,
                apEnable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, ptpTypeIndex);

            /*
                1.2. Call cpssPxPtpTypeKeyEntryEnableGet().
                Expected: GT_OK and the same parameters value as were set in 1.1
            */
            st = cpssPxPtpTypeKeyEntryEnableGet(devNum, ptpTypeIndex,
                &apEnableGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, ptpTypeIndex);
            UTF_VERIFY_EQUAL1_PARAM_MAC(apEnable, apEnableGet, devNum);

            /*
                1.3. Call with invalid PTP type.
                Expected: GT_BAD_PARAM.
            */
            ptpTypeIndex = CPSS_PX_PTP_TYPE_NUM_CNS;
            st = cpssPxPtpTypeKeyEntryEnableSet(devNum, ptpTypeIndex,
                apEnable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, ptpTypeIndex);
            ptpTypeIndex = 0;

            /*
                1.1 Call with valid parameters
                Expected: GT_OK.
            */
            apEnable = GT_FALSE;

            st = cpssPxPtpTypeKeyEntryEnableSet(devNum, ptpTypeIndex,
                apEnable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, ptpTypeIndex);

            /*
                1.2. Call cpssPxPtpTypeKeyEntryEnableGet().
                Expected: GT_OK and the same parameters value as were set in 1.1
            */
            st = cpssPxPtpTypeKeyEntryEnableGet(devNum, ptpTypeIndex,
                &apEnableGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, ptpTypeIndex);
            UTF_VERIFY_EQUAL1_PARAM_MAC(apEnable, apEnableGet, devNum);

            /*
                1.3. Call with invalid PTP type.
                Expected: GT_BAD_PARAM.
            */
            ptpTypeIndex = CPSS_PX_PTP_TYPE_NUM_CNS;
            st = cpssPxPtpTypeKeyEntryEnableSet(devNum, ptpTypeIndex,
                apEnable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, ptpTypeIndex);
            ptpTypeIndex = CPSS_PX_PTP_TYPE_NUM_CNS - 1;
        }

        apEnable  = GT_TRUE;

        /* 2.For not applicable devices check that function returns non GT_OK.*/
        /* prepare device iterator to go through all non-applicable devices */
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_NONE_FAMILY_E);

        /* Go over all non-applicable devices. */
        while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
        {
            st = cpssPxPtpTypeKeyEntryEnableSet(devNum, ptpTypeIndex,
                apEnable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum,
                ptpTypeIndex);
        }

        /* 3. Call function with out of bound value for device id.*/
        devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;
        st = cpssPxPtpTypeKeyEntryEnableSet(devNum, ptpTypeIndex,
            apEnable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, ptpTypeIndex);
}

/*----------------------------------------------------------------------------*/
/*
 GT_STATUS cpssPxPtpTaiPClockOutputEnableGet
(
    IN  GT_SW_DEV_NUM                           devNum,
    OUT GT_BOOL                                 *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPtpTaiPClockOutputEnableGet)
{
/*
    ITERATE_DEVICES(Pipe)
    1.1. Call with valid parameters
    Expected: GT_OK.
    1.2. Call with invalid enabledPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS                    st      =  GT_OK;
    GT_U8                        devNum  =  0;
    GT_BOOL                      enable  =  GT_FALSE;

    /* there is no TAI in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /*
            1.1. Call with valid parameters
            Expected: GT_OK.
        */
        st = cpssPxPtpTaiPClockOutputEnableGet(devNum, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

        /*
           1.2 Call with invalid enabledPtr [NULL].
           Expected: GT_BAD_PTR.
        */        st = cpssPxPtpTaiPClockOutputEnableGet(devNum, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, enabled = NULL",
                                     devNum);
    }

    /* restore valid values */
    enable  = GT_TRUE;

    /* 2. For not applicable devices check that function returns non GT_OK.*/
    /* prepare device iterator to go through all non-applicable devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_NONE_FAMILY_E);

    /* Go over all non-applicable devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssPxPtpTaiPClockOutputEnableGet(devNum, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /*3. Call function with out of bound value for device id.*/
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPtpTaiPClockOutputEnableGet(devNum, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPtpTaiPClockOutputEnableSet
(
    IN  GT_SW_DEV_NUM                           devNum,
    IN  GT_BOOL                                 enable  
)
*/
UTF_TEST_CASE_MAC(cpssPxPtpTaiPClockOutputEnableSet)
{
    /*
        ITERATE_DEVICES(Pipe)
        1.1. Call with valid parameters
        Expected: GT_OK.
        1.2. Call cpssPxPtpTaiPClockOutputEnableGet().
        Expected: GT_OK.
    */
        GT_STATUS                    st      =  GT_OK;
        GT_U8                        devNum  =  0;
        GT_BOOL                      apEnable     =  GT_FALSE;
        GT_BOOL                      apEnableGet  =  GT_FALSE;

        /* there is no TAI in GM */
        GM_NOT_SUPPORT_THIS_TEST_MAC;

        /* prepare device iterator */
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_NONE_FAMILY_E);

        /* 1. Go over all active devices. */
        while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
        {
            /*
                1.1 Call with valid parameters
                Expected: GT_OK.
            */
            apEnable = GT_TRUE;

            st = cpssPxPtpTaiPClockOutputEnableSet(devNum, apEnable);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

            /*
                1.2. Call cpssPxPtpTaiPClockOutputEnableGet().
                Expected: GT_OK and the same parameters value as were set in 1.1
            */
            st = cpssPxPtpTaiPClockOutputEnableGet(devNum, &apEnableGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);
            UTF_VERIFY_EQUAL1_PARAM_MAC(apEnable, apEnableGet, devNum);

            /*
                1.1 Call with valid parameters
                Expected: GT_OK.
            */
            apEnable = GT_FALSE;

            st = cpssPxPtpTaiPClockOutputEnableSet(devNum, apEnable);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

            /*
                1.2. Call cpssPxPtpTaiPClockOutputEnableGet().
                Expected: GT_OK and the same parameters value as were set in 1.1
            */
            st = cpssPxPtpTaiPClockOutputEnableGet(devNum, &apEnableGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);
            UTF_VERIFY_EQUAL1_PARAM_MAC(apEnable, apEnableGet, devNum);
        }

        apEnable  = GT_TRUE;

        /* 2.For not applicable devices check that function returns non GT_OK.*/
        /* prepare device iterator to go through all non-applicable devices */
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_NONE_FAMILY_E);

        /* Go over all non-applicable devices. */
        while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
        {
            st = cpssPxPtpTaiPClockOutputEnableSet(devNum, apEnable);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
        }

        /* 3. Call function with out of bound value for device id.*/
        devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;
        st =cpssPxPtpTaiPClockOutputEnableSet(devNum, apEnable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
 GT_STATUS cpssPxPtpTaiPhaseUpdateEnableGet
(
    IN  GT_SW_DEV_NUM                           devNum,
    OUT GT_BOOL                                 *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPtpTaiPhaseUpdateEnableGet)
{
/*
    ITERATE_DEVICES(Pipe)
    1.1. Call with valid parameters
    Expected: GT_OK.
    1.2. Call with invalid enabledPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS                    st      =  GT_OK;
    GT_U8                        devNum  =  0;
    GT_BOOL                      enable  =  GT_FALSE;

    /* there is no TAI in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /*
            1.1. Call with valid parameters
            Expected: GT_OK.
        */
        st = cpssPxPtpTaiPhaseUpdateEnableGet(devNum, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

        /*
           1.2 Call with invalid enabledPtr [NULL].
           Expected: GT_BAD_PTR.
        */        st = cpssPxPtpTaiPhaseUpdateEnableGet(devNum, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, enabled = NULL",
                                     devNum);
    }

    /* restore valid values */
    enable  = GT_TRUE;

    /* 2. For not applicable devices check that function returns non GT_OK.*/
    /* prepare device iterator to go through all non-applicable devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_NONE_FAMILY_E);

    /* Go over all non-applicable devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssPxPtpTaiPhaseUpdateEnableGet(devNum, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /*3. Call function with out of bound value for device id.*/
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPtpTaiPhaseUpdateEnableGet(devNum, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPtpTaiPhaseUpdateEnableSet
(
    IN  GT_SW_DEV_NUM                           devNum,
    IN  GT_BOOL                                 enable  
)
*/
UTF_TEST_CASE_MAC(cpssPxPtpTaiPhaseUpdateEnableSet)
{
    /*
        ITERATE_DEVICES(Pipe)
        1.1. Call with valid parameters
        Expected: GT_OK.
        1.2. Call cpssPxPtpTaiPhaseUpdateEnableGet().
        Expected: GT_OK.
    */
        GT_STATUS                    st      =  GT_OK;
        GT_U8                        devNum  =  0;
        GT_BOOL                      apEnable     =  GT_FALSE;
        GT_BOOL                      apEnableGet  =  GT_FALSE;

        /* there is no TAI in GM */
        GM_NOT_SUPPORT_THIS_TEST_MAC;

        /* prepare device iterator */
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_NONE_FAMILY_E);

        /* 1. Go over all active devices. */
        while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
        {
            /*
                1.1 Call with valid parameters
                Expected: GT_OK.
            */
            apEnable = GT_TRUE;

            st = cpssPxPtpTaiPhaseUpdateEnableSet(devNum, apEnable);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

            /*
                1.2. Call cpssPxPtpTaiPhaseUpdateEnableGet().
                Expected: GT_OK and the same parameters value as were set in 1.1
            */
            st = cpssPxPtpTaiPhaseUpdateEnableGet(devNum, &apEnableGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);
            UTF_VERIFY_EQUAL1_PARAM_MAC(apEnable, apEnableGet, devNum);

            /*
                1.1 Call with valid parameters
                Expected: GT_OK.
            */
            apEnable = GT_FALSE;

            st = cpssPxPtpTaiPhaseUpdateEnableSet(devNum, apEnable);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

            /*
                1.2. Call cpssPxPtpTaiPhaseUpdateEnableGet().
                Expected: GT_OK and the same parameters value as were set in 1.1
            */
            st = cpssPxPtpTaiPhaseUpdateEnableGet(devNum, &apEnableGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);
            UTF_VERIFY_EQUAL1_PARAM_MAC(apEnable, apEnableGet, devNum);
        }

        apEnable  = GT_TRUE;

        /* 2.For not applicable devices check that function returns non GT_OK.*/
        /* prepare device iterator to go through all non-applicable devices */
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_NONE_FAMILY_E);

        /* Go over all non-applicable devices. */
        while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
        {
            st = cpssPxPtpTaiPhaseUpdateEnableSet(devNum, apEnable);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
        }

        /* 3. Call function with out of bound value for device id.*/
        devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;
        st =cpssPxPtpTaiPhaseUpdateEnableSet(devNum, apEnable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPtpTaiPulseInModeSet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DXCH_PTP_TAI_PULSE_IN_MODE_ENT     pulseMode
);
*/
UTF_TEST_CASE_MAC(cpssPxPtpTaiPulseInModeSet)
{
/*
    1.1. Call cpssPxPtpTaiPulseInModeSet with
                        pulseMode[CPSS_PX_PTP_TAI_PULSE_IN_DISABLED_E /
                        CPSS_PX_PTP_TAI_PULSE_IN_IS_PTP_PULSE_IN_E /
                        CPSS_PX_PTP_TAI_PULSE_IN_IS_PTP_CLOCK_IN_E]
    Expected: GT_OK & PulseInMode according to the set command.
    1.2. Call cpssPxPtpTaiPulseInModeSet with invalid pulseMode.
    Expected: GT_BAD_PARAM.
    1.3. Call cpssPxPtpTaiPulseInModeSet with invalid device number.
    Expected: GT_BAD_PARAM.
*/

    GT_STATUS st = GT_OK;
    GT_U8     dev = 0;
    CPSS_PX_PTP_TAI_PULSE_IN_MODE_ENT pulseMode[3] =
        {CPSS_PX_PTP_TAI_PULSE_IN_DISABLED_E, CPSS_PX_PTP_TAI_PULSE_IN_IS_PTP_CLOCK_IN_E, CPSS_PX_PTP_TAI_PULSE_IN_IS_PTP_PULSE_IN_E};
    CPSS_PX_PTP_TAI_PULSE_IN_MODE_ENT pulseModeGet[3] =
        {CPSS_PX_PTP_TAI_PULSE_IN_DISABLED_E, CPSS_PX_PTP_TAI_PULSE_IN_IS_PTP_CLOCK_IN_E, CPSS_PX_PTP_TAI_PULSE_IN_IS_PTP_PULSE_IN_E};

    GT_U32 j;

    /* there is no TAI and PTP in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for(j = 0; j < 3; j++)
        {
            st = cpssPxPtpTaiPulseInModeSet(dev, pulseMode[j]);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            st = cpssPxPtpTaiPulseInModeGet(dev, &pulseModeGet[j]);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                      "cpssPxPtpTaiPulseInModeGet: %d ", dev);

            /* Verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(pulseModeGet[j], pulseMode[j],
                           "got another pulseMode then was set: %d", dev);
        }

        /*
           1.2. Call api with wrong pulseMode[wrong enum values].
                Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssPxPtpTaiPulseInModeSet(dev, pulseMode[0]), pulseMode[0]);
    }

    /*
       1.3. Call function with out of bound value for device id
            Expected: GT_BAD_PARAM.
    */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssPxPtpTaiPulseInModeSet(dev, pulseMode[0]);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPtpTaiPulseInModeGet
(
    IN  GT_U8                                   devNum,
    OUT CPSS_PX_PTP_TAI_PULSE_IN_MODE_ENT     *pulseModePtr
);
*/
UTF_TEST_CASE_MAC(cpssPxPtpTaiPulseInModeGet)
{
/*
    1.1. Call cpssPxPtpTaiPulseInModeGet without any set operations
    Expected: GT_OK & PulseInMode as CPSS_PX_PTP_TAI_PULSE_IN_IS_PTP_PULSE_IN_E
    1.2. Call cpssPxPtpTaiPulseInModeGet with invalid pulseMode.
    Expected: GT_BAD_PARAM.
    1.3. Call cpssPxPtpTaiPulseInModeGet with invalid device number.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS st = GT_OK;
    GT_U8     dev = 0;
    CPSS_PX_PTP_TAI_PULSE_IN_MODE_ENT pulseMode[3] =
        {CPSS_PX_PTP_TAI_PULSE_IN_IS_PTP_PULSE_IN_E, CPSS_PX_PTP_TAI_PULSE_IN_IS_PTP_CLOCK_IN_E, CPSS_PX_PTP_TAI_PULSE_IN_DISABLED_E};
    CPSS_PX_PTP_TAI_PULSE_IN_MODE_ENT pulseModeGet[3] =
        {CPSS_PX_PTP_TAI_PULSE_IN_DISABLED_E, CPSS_PX_PTP_TAI_PULSE_IN_IS_PTP_CLOCK_IN_E, CPSS_PX_PTP_TAI_PULSE_IN_IS_PTP_PULSE_IN_E};

    /* there is no TAI and PTP in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
         * 1.1. Call cpssPxPtpTaiPulseInModeGet without any set operations
         *           Expected: GT_OK & PulseInMode as CPSS_PX_PTP_TAI_PULSE_IN_IS_PTP_PULSE_IN_E
         */

        st = cpssPxPtpTaiPulseInModeGet(dev, &pulseModeGet[0]);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                "cpssPxPtpTaiPulseInModeGet: %d ", dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(pulseMode[0], pulseModeGet[0],
                "got another pulseMode then default: %d", dev);

        /*
           1.2. Call api with NULL PulseMode pointer.
                Expected: GT_BAD_PARAM.
        */
        st = cpssPxPtpTaiPulseInModeGet(dev, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /*
       1.4. Call function with out of bound value for device id
            Expected: GT_BAD_PARAM.
    */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssPxPtpTaiPulseInModeGet(dev, &pulseModeGet[0]);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
 * Configuration of cpssPxPtp suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssPxPtp)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPtpTaiTodCounterFunctionGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPtpTaiTodCounterFunctionSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPtpTaiTodCounterFunctionTriggerGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPtpTaiTodCounterFunctionTriggerSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPtpTaiCaptureOverwriteEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPtpTaiCaptureOverwriteEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPtpTaiClockCycleGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPtpTaiClockCycleSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPtpTaiClockModeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPtpTaiClockModeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPtpTaiExternalPulseWidthGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPtpTaiExternalPulseWidthSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPtpTaiFractionalNanosecondDriftGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPtpTaiFractionalNanosecondDriftSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPtpTaiFrequencyDriftThesholdsGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPtpTaiFrequencyDriftThesholdsSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPtpTaiIncomingClockCounterEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPtpTaiIncomingClockCounterEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPtpTaiIncomingClockCounterGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPtpTaiIncomingTriggerCounterGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPtpTaiInputTriggersCountEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPtpTaiInputTriggersCountEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPtpTaiOutputTriggerEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPtpTaiOutputTriggerEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPtpTaiPClockCycleGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPtpTaiPClockCycleSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPtpTaiPtpPClockDriftAdjustEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPtpTaiPtpPClockDriftAdjustEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPtpTaiTodCaptureStatusGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPtpTaiTodGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPtpTaiTodSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPtpTaiTodStepGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPtpTaiTodStepSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPtpTaiTodUpdateCounterGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPtpTaiGracefulStepSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPtpTaiGracefulStepGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPtpTsuControlGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPtpTsuControlSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPtpTsuCountersClear)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPtpTsuPacketCouterGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPtpTaiInputClockSelectSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPtpTaiInputClockSelectGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPtpPortTypeKeySet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPtpPortTypeKeyGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPtpTypeKeyEntrySet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPtpTypeKeyEntryGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPtpTypeKeyEntryEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPtpTypeKeyEntryEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPtpTaiPClockOutputEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPtpTaiPClockOutputEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPtpTaiPhaseUpdateEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPtpTaiPhaseUpdateEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPtpTaiPulseInModeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPtpTaiPulseInModeGet)
UTF_SUIT_END_TESTS_MAC(cpssPxPtp)


