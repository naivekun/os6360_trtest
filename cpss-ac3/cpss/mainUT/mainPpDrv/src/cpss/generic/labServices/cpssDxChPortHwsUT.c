/******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELLSEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE       *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.    *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,      *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.  *
*******************************************************************************
*/
/**
********************************************************************************
* @file cpssDxChPortHwsUT.c
*
* @brief Unit tests for HW Services library.
*
* @version   1
********************************************************************************
*/
/* the define of UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC
 * must come from C files that
    already fixed the types of ports from GT_U8 !

    NOTE: must come before ANY include to H files !!!!
*/
#define UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC

#include <cpss/common/labServices/port/gop/port/private/mvHwsDiagnostic.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <common/tgfCommon.h>


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
/*
GT_STATUS mvHwsDiagDeviceDbCheck(GT_U8 devNum, MV_HWS_DEV_TYPE devType)
*/
UTF_TEST_CASE_MAC(mvHwsDiagDeviceDbCheck)
{
/*
    ITERATE_DEVICE (all)
    1.1. Call with valid nextDevNumPtr.
    Expected: GT_OK.
    1.2. Call with out of range nextDevNumPtr[NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS               st          = GT_OK;
    GT_U8                   dev         = 0;
    MV_HWS_DEV_TYPE         devTypeHws = 0;
    CPSS_PP_FAMILY_TYPE_ENT devFamily;
    CPSS_PP_DEVICE_TYPE     devTypeCpss;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                        UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E |
                                        UTF_LION_E | UTF_XCAT2_E);

    PRV_TGF_SKIP_SIMULATION_FAILED_TEST_MAC(UTF_BOBCAT2_E, CPSS-6065);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfDeviceFamilyGet(dev, &devFamily);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        switch (devFamily)
        {
            case CPSS_PP_FAMILY_DXCH_XCAT3_E:
                devTypeHws = Alleycat3A0;
                break;

            case CPSS_PP_FAMILY_DXCH_LION2_E:
                st = prvUtfDeviceTypeGet(dev, &devTypeCpss);
                switch(devTypeCpss)
                {
                    case CPSS_LION2_HOOPER_PORT_GROUPS_0123_DEVICES_CASES_MAC:
                        devTypeHws = HooperA0;
                        break;
                    default:
                        devTypeHws = Lion2B0;
                        break;
                }
                break;

            case CPSS_PP_FAMILY_DXCH_BOBCAT2_E:
                st = prvUtfDeviceTypeGet(dev, &devTypeCpss);
                switch(devTypeCpss)
                {
                    case CPSS_BOBK_ALL_DEVICES_CASES_MAC:
                        continue;
                    default:
                        devTypeHws = BobcatA0;
                        break;
                }
                break;

            default:
                continue;
        }
        st = mvHwsDiagDeviceDbCheck(dev, devTypeHws);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, devTypeHws);

        /* 1.1 Call function with out of bound value for device id.*/
        st = mvHwsDiagDeviceDbCheck(dev, LAST_SIL_TYPE);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, LAST_SIL_TYPE);
    }

    /* 2. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = mvHwsDiagDeviceDbCheck(dev, devTypeHws);
    UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, devTypeHws);
}

/*---------------------------------------------------------------------------*/
/*
 * Configuration of mvHws suit
 */

UTF_SUIT_BEGIN_TESTS_MAC(mvHws)
    UTF_SUIT_DECLARE_TEST_MAC(mvHwsDiagDeviceDbCheck)
UTF_SUIT_END_TESTS_MAC(mvHws)


