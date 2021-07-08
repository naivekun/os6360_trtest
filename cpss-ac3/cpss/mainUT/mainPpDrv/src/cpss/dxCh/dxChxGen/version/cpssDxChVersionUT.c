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
* @file cpssDxChVersionUT.c
*
* @brief Unit tests for cpssDxChVersionUT, that provides
* software CPSS DxCh version information.
*
* @version   3
********************************************************************************
*/
/* includes */

#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/dxCh/dxChxGen/version/cpssDxChVersion.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChVersionGet
(
    OUT CPSS_VERSION_INFO_STC   *versionPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChVersionGet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with non-NULL pointer.
    Expected: GT_OK.
    1.2. Call with versionPtr [NULL], other params same as in 1.1.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    CPSS_VERSION_INFO_STC   version;

    cpssOsBzero((GT_VOID*)&version, sizeof(version));

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with non-NULL pointers.
            Expected: GT_OK.
        */
        st = cpssDxChVersionGet(&version);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /*
            1.2. Call with versionPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChVersionGet(NULL);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, st);

    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChVersionGet(&version);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }
}

/*----------------------------------------------------------------------------*/
/*
 * Configuration of cpssDxChVersion suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssDxChVersion)

    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChVersionGet)

UTF_SUIT_END_TESTS_MAC(cpssDxChVersion)


