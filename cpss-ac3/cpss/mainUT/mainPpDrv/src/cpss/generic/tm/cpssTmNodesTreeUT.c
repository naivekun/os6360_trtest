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
* @file cpssTmNodesTreeUT.c
*
* @brief Unit tests for cpssTmTree.
*
* @version   2
********************************************************************************
*/

#include <cpss/generic/tm/cpssTmCtl.h>
#include <cpss/generic/tm/cpssTmDrop.h>
#include <cpss/generic/tm/cpssTmSched.h>
#include <cpss/generic/tm/cpssTmShaping.h>
#include <cpss/generic/tm/cpssTmNodesCreate.h>
#include <cpss/generic/tm/cpssTmNodesTree.h>
#include <cpssCommon/private/prvCpssMath.h>
#include <cpss/generic/tm/cpssTmUtDefs.h>
#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>


/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
GT_STATUS cpssTmTreeChangeStatus
(
    IN GT_U8    devNum,
    IN GT_BOOL  status
)
GT_STATUS cpssTmTreeStatusGet
(
    IN GT_U8    devNum,
    IN GT_BOOL  * status
)
*/
UTF_TEST_CASE_MAC(cpssTmTreeChangeStatus)
{
    GT_STATUS               st = GT_OK;

    GT_U8                   dev;
    GT_BOOL                 status;
    CPSS_PP_FAMILY_TYPE_ENT devFamily;
    GT_BOOL                 status_out;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev,
                                 UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E |
                                     UTF_PUMA2_E | UTF_PUMA3_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E | UTF_AC3X_E);
    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* get device family */
        st = prvUtfDeviceFamilyGet(dev, &devFamily);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfDeviceFamilyGet: %d", dev);

        /* TM initialization */

        st = cpssTmTestTmCtlLibInit(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);


        /*1.1 Check with legal parameters.
              Call with status [GT_TRUE]
                Expected: GT_OK */

        status= GT_TRUE;

        st = cpssTmTreeChangeStatus(dev, status);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        st = cpssTmTreeStatusGet(dev, &status_out);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        UTF_VERIFY_EQUAL1_PARAM_MAC(status, status_out, dev);

         /*1.2 Check with legal parameters.
              Call with status [GT_FALSE]
                Expected: GT_OK */

        status= GT_FALSE;

        st = cpssTmTreeChangeStatus(dev, status);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        st = cpssTmTreeStatusGet(dev, &status_out);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        UTF_VERIFY_EQUAL1_PARAM_MAC(status, status_out, dev);


        st = cpssTmClose(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        status= GT_TRUE;
    }


    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    status= GT_TRUE;

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssTmTreeChangeStatus(dev, status);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
        st = cpssTmTreeStatusGet(dev, &status_out);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssTmTreeChangeStatus(dev, status);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
    st = cpssTmTreeStatusGet(dev, &status_out);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}
/*----------------------------------------------------------------------------
GT_STATUS cpssTmTreeDwrrPrioSet
(
    IN GT_U8    devNum,
    IN GT_BOOL  prios[8]
);
GT_STATUS cpssTmTreeDwrrPrioGet
(
    IN GT_U8    devNum,
    IN GT_BOOL  prios[8]
);
*/
UTF_TEST_CASE_MAC(cpssTmTreeDwrrPrioSet)
{
    GT_STATUS               st = GT_OK;

    GT_U8                   dev;
    GT_BOOL                 prios[8];
    GT_BOOL                 priosOut[8];
    CPSS_PP_FAMILY_TYPE_ENT devFamily;
    GT_U16                  ii;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev,
                                 UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E |
                                     UTF_PUMA2_E | UTF_PUMA3_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E | UTF_AC3X_E);
    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* get device family */
        st = prvUtfDeviceFamilyGet(dev, &devFamily);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfDeviceFamilyGet: %d", dev);

        /* TM initialization */

        st = cpssTmTestTmCtlLibInit(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*1.1 Check with legal parameters.
              Call with prios [GT_TRUE]
                Expected: GT_OK */

        for (ii=0;ii<8;ii++) {
            prios[ii]= GT_TRUE;
        }

        st = cpssTmTreeDwrrPrioSet(dev, prios);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        st = cpssTmTreeDwrrPrioGet(dev, priosOut);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        for (ii=0;ii<8;ii++) {
            UTF_VERIFY_EQUAL1_PARAM_MAC(prios[ii], priosOut[ii], dev);
        }

         /*1.2 Check with legal parameters.
              Call with prios [GT_FALSE]
                Expected: GT_OK */

        for (ii=0;ii<8;ii++) {
            prios[ii]= GT_FALSE;
        }

        st = cpssTmTreeDwrrPrioSet(dev, prios);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        st = cpssTmTreeDwrrPrioGet(dev, priosOut);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        for (ii=0;ii<8;ii++) {
            UTF_VERIFY_EQUAL1_PARAM_MAC(prios[ii],priosOut[ii], dev);
        }

          /*
            1.3 Call with prios [NULL], other parameters same as in 1.1.
            Expected: GT_BAD_PTR.
        */
        for (ii=0;ii<8;ii++) {
            prios[ii]= GT_TRUE;
        }

        st = cpssTmTreeDwrrPrioSet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, prios = NULL", dev);
        st = cpssTmTreeDwrrPrioGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, prios = NULL", dev);

        st = cpssTmClose(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }


    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssTmTreeDwrrPrioSet(dev, prios);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
        st = cpssTmTreeDwrrPrioGet(dev, priosOut);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssTmTreeDwrrPrioSet(dev, prios);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
    st = cpssTmTreeDwrrPrioGet(dev, priosOut);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
 * Configuration of cpssTmTree suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssTmNodesTree)
    UTF_SUIT_DECLARE_TEST_MAC(cpssTmTreeChangeStatus)
    UTF_SUIT_DECLARE_TEST_MAC(cpssTmTreeDwrrPrioSet)
UTF_SUIT_END_TESTS_MAC(cpssTmNodesTree)

