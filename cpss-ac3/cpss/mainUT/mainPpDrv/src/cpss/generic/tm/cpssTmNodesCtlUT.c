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
* @file cpssTmNodesCtlUT.c
*
* @brief Unit tests for cpssTmNodesCtl.
*
* @version   1.3
********************************************************************************
*/

#include <cpss/generic/tm/cpssTmCtl.h>
#include <cpss/generic/tm/cpssTmDrop.h>
#include <cpss/generic/tm/cpssTmSched.h>
#include <cpss/generic/tm/cpssTmShaping.h>
#include <cpss/generic/tm/cpssTmNodesCreate.h>
#include <cpss/generic/tm/cpssTmNodesUpdate.h>
#include <cpss/generic/tm/cpssTmNodesTree.h>
#include <cpss/generic/tm/cpssTmNodesReorder.h>
#include <cpss/generic/tm/cpssTmNodesCtl.h>
#include <cpss/generic/tm/cpssTmNodesRead.h>
#include <cpssCommon/private/prvCpssMath.h>
#include <cpss/generic/tm/cpssTmUtDefs.h>
#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>


/*******************************************************************************
GT_STATUS cpssTmNodesCtlQueueInstall
(
    IN GT_U8    devNum,
    IN GT_U32   queueInd
)
GT_STATUS cpssTmNodesCtlQueueInstallStatusGet
(
    IN GT_U8    devNum,
    IN GT_U32   queueInd,
    OUT GT_U8   *status
)

*/
UTF_TEST_CASE_MAC(cpssTmNodesCtlQueueInstall)
{
    GT_STATUS               st = GT_OK;

    GT_U8                           dev;
    GT_U32                          queueIndPtr = 1;
    GT_U32                          aNodeIndPtr;
    GT_U32                          bNodeIndPtr;
    GT_U32                          cNodeIndPtr;
    CPSS_PP_FAMILY_TYPE_ENT devFamily;
    GT_U32                           status;

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

        st = cpssTmTestPeriodicSchemeConfiguration(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssTmTestTreeBuild(dev, &queueIndPtr, &aNodeIndPtr, &bNodeIndPtr, &cNodeIndPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.1 Check with legal parameters.
                Expected: GT_OK */


        st = cpssTmNodesCtlQueueInstall(dev, queueIndPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        st = cpssTmNodesCtlQueueInstallStatusGet(dev, queueIndPtr, &status);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(1, status, dev);

        /*
            1.2 check out of range queueIndPtr .
            Call with queueIndPtr [queueIndPtr+20], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        queueIndPtr = queueIndPtr + 20;

        st = cpssTmNodesCtlQueueInstall(dev, queueIndPtr);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, queueIndPtr);
        st = cpssTmNodesCtlQueueInstallStatusGet(dev, queueIndPtr, &status);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, queueIndPtr);

        queueIndPtr = queueIndPtr - 20;

        st = cpssTmTestTmClose(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }


    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssTmNodesCtlQueueInstall(dev, queueIndPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
        st = cpssTmNodesCtlQueueInstallStatusGet(dev, queueIndPtr, &status);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssTmNodesCtlQueueInstall(dev, queueIndPtr);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
    st = cpssTmNodesCtlQueueInstallStatusGet(dev, queueIndPtr, &status);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*******************************************************************************
GT_STATUS cpssTmNodesCtlQueueUninstall
(
    IN GT_U8    devNum,
    IN GT_U32   queueInd
)
GT_STATUS cpssTmNodesCtlQueueInstallStatusGet
(
    IN GT_U8    devNum,
    IN GT_U32   queueInd,
    OUT GT_U8   *status
)
*/
UTF_TEST_CASE_MAC(cpssTmNodesCtlQueueUninstall)
{
    GT_STATUS               st = GT_OK;

    GT_U8                           dev;
    GT_U32                          queueIndPtr = 1;
    GT_U32                          aNodeIndPtr;
    GT_U32                          bNodeIndPtr;
    GT_U32                          cNodeIndPtr;
    CPSS_PP_FAMILY_TYPE_ENT devFamily;
    GT_U32                          status;

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

        st = cpssTmTestPeriodicSchemeConfiguration(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssTmTestTreeBuild(dev, &queueIndPtr, &aNodeIndPtr, &bNodeIndPtr, &cNodeIndPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssTmNodesCtlQueueInstall(dev, queueIndPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        st = cpssTmNodesCtlQueueInstallStatusGet(dev, queueIndPtr, &status);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(1, status, dev);

        /*
            1.1 check out of range queueIndPtr .
            Call with queueIndPtr [queueIndPtr+20], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        queueIndPtr = queueIndPtr + 20;

        st = cpssTmNodesCtlQueueUninstall(dev, queueIndPtr);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, queueIndPtr);

        queueIndPtr = queueIndPtr - 20;


        /*  1.1 Check with legal parameters.
                Expected: GT_OK */

        st = cpssTmNodesCtlQueueUninstall(dev, queueIndPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        st = cpssTmNodesCtlQueueInstallStatusGet(dev, queueIndPtr, &status);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(0, status, dev);

        st = cpssTmTestTmClose(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }


    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssTmNodesCtlQueueUninstall(dev, queueIndPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssTmNodesCtlQueueUninstall(dev, queueIndPtr);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*******************************************************************************
GT_STATUS cpssTmQueueFlush
(
    IN GT_U8    devNum,
    IN GT_U32   index
)
*/
UTF_TEST_CASE_MAC(cpssTmQueueFlush)
{
    GT_STATUS               st = GT_OK;

    GT_U8                           dev;
    GT_U32                          queueIndPtr = 1;
    GT_U32                          aNodeIndPtr;
    GT_U32                          bNodeIndPtr;
    GT_U32                          cNodeIndPtr;
    CPSS_PP_FAMILY_TYPE_ENT devFamily;

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

        st = cpssTmTestPeriodicSchemeConfiguration(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssTmTestTreeBuild(dev, &queueIndPtr, &aNodeIndPtr, &bNodeIndPtr, &cNodeIndPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.1 check uninstalled Q.
            Expected: NON GT_OK.
        */
        st = cpssTmQueueFlush(dev, queueIndPtr);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, queueIndPtr);

        st = cpssTmNodesCtlQueueInstall(dev, queueIndPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

         /*
            1.2 check out of range queueIndPtr.
            Call with queueIndPtr [queueIndPtr+20], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        queueIndPtr = queueIndPtr + 20;

        st = cpssTmQueueFlush(dev, queueIndPtr);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, queueIndPtr);

        queueIndPtr = queueIndPtr - 20;

        /*
           1.3 Check with legal parameters.
           Expected: GT_OK */

        /* activate flush only when full TM init done to avoid TM misbehaviour. */
        if (prvUtfIsTrafficManagerUsed())
        {
            st = cpssTmQueueFlush(dev, queueIndPtr);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        }

        st = cpssTmTestTmClose(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }


    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssTmQueueFlush(dev, queueIndPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssTmQueueFlush(dev, queueIndPtr);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*******************************************************************************
GT_STATUS cpssTmQueueFlushTriggerActionStart
(
    IN GT_U8    devNum,
    IN GT_U32   index
)
*/
UTF_TEST_CASE_MAC(cpssTmQueueFlushTriggerActionStart)
{
    GT_STATUS               st = GT_OK;

    GT_U8                           dev;
    GT_U32                          queueIndPtr = 1;
    GT_U32                          aNodeIndPtr;
    GT_U32                          bNodeIndPtr;
    GT_U32                          cNodeIndPtr;
    CPSS_PP_FAMILY_TYPE_ENT devFamily;

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

        st = cpssTmTestPeriodicSchemeConfiguration(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssTmTestTreeBuild(dev, &queueIndPtr, &aNodeIndPtr, &bNodeIndPtr, &cNodeIndPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.1 check uninstalled Q.
            Expected: NON GT_OK.
        */
        st = cpssTmQueueFlushTriggerActionStart(dev, queueIndPtr);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, queueIndPtr);

        st = cpssTmNodesCtlQueueInstall(dev, queueIndPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

         /*
            1.2 check out of range queueIndPtr.
            Call with queueIndPtr [queueIndPtr+20], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        queueIndPtr = queueIndPtr + 20;

        st = cpssTmQueueFlushTriggerActionStart(dev, queueIndPtr);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, queueIndPtr);

        queueIndPtr = queueIndPtr - 20;

        /*
           1.3 Check with legal parameters.
           Expected: GT_OK */

        /* activate flush only when full TM init done to avoid TM misbehaviour. */
        if (prvUtfIsTrafficManagerUsed())
        {
            st = cpssTmQueueFlushTriggerActionStart(dev, queueIndPtr);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        }

        st = cpssTmTestTmClose(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }


    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssTmQueueFlushTriggerActionStart(dev, queueIndPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssTmQueueFlushTriggerActionStart(dev, queueIndPtr);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}



/*******************************************************************************
GT_STATUS cpssTmQueueFlushTriggerActionStatusGet
(
    IN GT_U8                    devNum,
    IN GT_U32                   index,
    OUT GT_BOOL                 *actFinishedPtr
)
*/
UTF_TEST_CASE_MAC(cpssTmQueueFlushTriggerActionStatusGet)
{
    GT_STATUS               st = GT_OK;

    GT_U8                           dev;
    GT_U32                          queueIndPtr = 1;
    GT_U32                          aNodeIndPtr;
    GT_U32                          bNodeIndPtr;
    GT_U32                          cNodeIndPtr;
    GT_BOOL                         actFinished;
    CPSS_PP_FAMILY_TYPE_ENT devFamily;

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

        st = cpssTmTestPeriodicSchemeConfiguration(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssTmTestTreeBuild(dev, &queueIndPtr, &aNodeIndPtr, &bNodeIndPtr, &cNodeIndPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.1 check uninstalled Q.
            Expected: NON GT_OK.
        */
        st = cpssTmQueueFlushTriggerActionStatusGet(dev, queueIndPtr, &actFinished);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, queueIndPtr);

        st = cpssTmNodesCtlQueueInstall(dev, queueIndPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

         /*
            1.2 check out of range queueIndPtr.
            Call with queueIndPtr [queueIndPtr+20], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        queueIndPtr = queueIndPtr + 20;

        st = cpssTmQueueFlushTriggerActionStatusGet(dev, queueIndPtr, &actFinished);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, queueIndPtr);

        queueIndPtr = queueIndPtr - 20;

        /*
           1.3 Check with legal parameters.
           Expected: GT_OK */
        st = cpssTmQueueFlushTriggerActionStatusGet(dev, queueIndPtr, &actFinished);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
#ifndef ASIC_SIMULATION
        /* activation of flush done by prev test only when full TM init done. */
        if (prvUtfIsTrafficManagerUsed())
        {
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_TRUE, actFinished, dev);
        }
#endif

        st = cpssTmTestTmClose(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }


    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssTmQueueFlushTriggerActionStatusGet(dev, queueIndPtr, &actFinished);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssTmQueueFlushTriggerActionStatusGet(dev, queueIndPtr, &actFinished);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*******************************************************************************
GT_STATUS cpssTmPortFlush
(
    IN GT_U8    devNum,
    IN GT_U32   index
)
*/
UTF_TEST_CASE_MAC(cpssTmPortFlush)
{
    GT_STATUS                       st = GT_OK;
    GT_U8                           dev;
    GT_U32                          portInd = 1;
    GT_U32                          queueIndPtr = 1;
    GT_U32                          aNodeIndPtr;
    GT_U32                          bNodeIndPtr;
    GT_U32                          cNodeIndPtr;
    CPSS_PP_FAMILY_TYPE_ENT         devFamily;

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

        st = cpssTmTestPeriodicSchemeConfiguration(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssTmTestTreeBuild(dev, &queueIndPtr, &aNodeIndPtr, &bNodeIndPtr, &cNodeIndPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.1 check out of range portInd .
            Call with portInd [portInd+20], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        st = cpssTmNodesCtlQueueInstall(dev, queueIndPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        portInd = portInd + 20;

        st = cpssTmPortFlush(dev, portInd);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, portInd);

        portInd = portInd - 20;


        /* 1.2 Check with legal parameters.
                Expected: GT_OK */

        st = cpssTmPortFlush(dev, portInd);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssTmTestTmClose(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }


    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssTmPortFlush(dev, portInd);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssTmPortFlush(dev, portInd);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*******************************************************************************
GT_STATUS cpssTmPortFlushTriggerActionStart
(
    IN GT_U8    devNum,
    IN GT_U32   index
)
*/
UTF_TEST_CASE_MAC(cpssTmPortFlushTriggerActionStart)
{
    GT_STATUS                       st = GT_OK;
    GT_U8                           dev;
    GT_U32                          portInd = 1;
    GT_U32                          queueIndPtr = 1;
    GT_U32                          aNodeIndPtr;
    GT_U32                          bNodeIndPtr;
    GT_U32                          cNodeIndPtr;
    CPSS_PP_FAMILY_TYPE_ENT         devFamily;

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

        st = cpssTmTestPeriodicSchemeConfiguration(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssTmTestTreeBuild(dev, &queueIndPtr, &aNodeIndPtr, &bNodeIndPtr, &cNodeIndPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.1 check out of range portInd .
            Call with portInd [portInd+20], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        st = cpssTmNodesCtlQueueInstall(dev, queueIndPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        portInd = portInd + 20;

        st = cpssTmPortFlushTriggerActionStart(dev, portInd);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, portInd);

        portInd = portInd - 20;


        /* 1.2 Check with legal parameters.
                Expected: GT_OK */

        st = cpssTmPortFlushTriggerActionStart(dev, portInd);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssTmTestTmClose(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }


    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssTmPortFlushTriggerActionStart(dev, portInd);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssTmPortFlushTriggerActionStart(dev, portInd);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*******************************************************************************
GT_STATUS cpssTmPortFlushTriggerActionStatusGet
(
    IN GT_U8                    devNum,
    IN GT_U32                   index,
    OUT GT_BOOL                 *actFinishedPtr
)
*/
UTF_TEST_CASE_MAC(cpssTmPortFlushTriggerActionStatusGet)
{
    GT_STATUS                       st = GT_OK;
    GT_U8                           dev;
    GT_U32                          portInd = 1;
    GT_U32                          queueIndPtr = 1;
    GT_U32                          aNodeIndPtr;
    GT_U32                          bNodeIndPtr;
    GT_U32                          cNodeIndPtr;
    GT_BOOL                         actFinished;
    CPSS_PP_FAMILY_TYPE_ENT         devFamily;

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

        st = cpssTmTestPeriodicSchemeConfiguration(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssTmTestTreeBuild(dev, &queueIndPtr, &aNodeIndPtr, &bNodeIndPtr, &cNodeIndPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.1 check out of range portInd .
            Call with portInd [portInd+20], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        st = cpssTmNodesCtlQueueInstall(dev, queueIndPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        portInd = portInd + 20;

        st = cpssTmPortFlushTriggerActionStatusGet(dev, portInd, &actFinished);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, portInd);

        portInd = portInd - 20;


        /* 1.2 Check with legal parameters.
                Expected: GT_OK */

        st = cpssTmPortFlushTriggerActionStatusGet(dev, portInd, &actFinished);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
#ifndef ASIC_SIMULATION
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_TRUE, actFinished, dev);
#endif

        st = cpssTmTestTmClose(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }


    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssTmPortFlushTriggerActionStatusGet(dev, portInd, &actFinished);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssTmPortFlushTriggerActionStatusGet(dev, portInd, &actFinished);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*******************************************************************************
GT_STATUS cpssTmNodesCtlNodeDelete
(
    IN GT_U8                    devNum,
    IN CPSS_TM_LEVEL_ENT        level,
    IN GT_U32                   index
)
*/
UTF_TEST_CASE_MAC(cpssTmNodesCtlNodeDelete)
{
    GT_STATUS               st = GT_OK;

    GT_U8                           dev;
    GT_U32                          queueIndPtr = 1;
    GT_U32                          aNodeIndPtr;
    GT_U32                          bNodeIndPtr;
    GT_U32                          cNodeIndPtr;
    CPSS_TM_LEVEL_ENT               level;
    CPSS_PP_FAMILY_TYPE_ENT devFamily;

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

        st = cpssTmTestPeriodicSchemeConfiguration(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssTmTestTreeBuild(dev, &queueIndPtr, &aNodeIndPtr, &bNodeIndPtr, &cNodeIndPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

         /*
            1.1 check out of range queueIndPtr .
            Call with queueIndPtr [queueIndPtr+20], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        queueIndPtr = queueIndPtr + 20;
        level = CPSS_TM_LEVEL_Q_E;

        st = cpssTmNodesCtlNodeDelete(dev, level, queueIndPtr);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, queueIndPtr);

        queueIndPtr = queueIndPtr - 20;

        /*
            1.2 Check for wrong enum values level and other params same as in 1.1
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssTmNodesCtlNodeDelete
                            (dev, level, queueIndPtr), level);

        /*  1.3 Check with legal parameters.
                Expected: GT_OK */
        st = cpssTmNodesCtlNodeDelete(dev, level, queueIndPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssTmTestTmClose(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }


    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    level = CPSS_TM_LEVEL_Q_E;

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssTmNodesCtlNodeDelete(dev, level, queueIndPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssTmNodesCtlNodeDelete(dev, level, queueIndPtr);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*******************************************************************************
GT_STATUS cpssTmNodesCtlTransPortDelete
(
    IN GT_U8    devNum,
    IN GT_U32   index
)
*/
UTF_TEST_CASE_MAC(cpssTmNodesCtlTransPortDelete)
{
    GT_STATUS               st = GT_OK;

    GT_U8                           dev;
    GT_U32                          portInd =1;
    CPSS_TM_PORT_PARAMS_STC      paramsPtr;
    GT_U32                       cNodesNum;
    GT_U32                       bNodesNum;
    GT_U32                       aNodesNum;
    GT_U32                       queuesNum;
    CPSS_PP_FAMILY_TYPE_ENT devFamily;
    GT_U32                       ii;

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

        st = cpssTmTestPeriodicSchemeConfiguration(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Configure parameters for port */
        paramsPtr.cirBw = 1000000;
        paramsPtr.eirBw = 0;
        paramsPtr.cbs = 8; /*KBytes*/
        paramsPtr.ebs = 0x1FFFF;
        paramsPtr.eligiblePrioFuncId = CPSS_TM_ELIG_N_MIN_SHP_E;
        for(ii=0; ii<8; ii++){
            paramsPtr.schdModeArr[ii] = CPSS_TM_SCHD_MODE_RR_E;
            paramsPtr.quantumArr[ii]= UT_TM_MIN_PORT_QUANTUM_CNS;
        }
        paramsPtr.dropProfileInd = CPSS_TM_NO_DROP_PROFILE_CNS;

        portInd=1;
        cNodesNum=1;
        bNodesNum=1;
        aNodesNum=1;
        queuesNum=1;

        st = cpssTmPortCreate(dev, portInd, &paramsPtr, cNodesNum, bNodesNum, aNodesNum,queuesNum );
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

         /*
            1.1 check out of range portInd .
            Call with portInd [1240], other parameters same as in 1.1.
            Expected: NON GT_OK.
        */

        portInd= 1240;

        st = cpssTmNodesCtlTransPortDelete(dev,portInd);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, portInd);

        portInd=1;


        /*1.2 Check with legal parameters.
                Expected: GT_OK */

        st = cpssTmNodesCtlTransPortDelete(dev,portInd);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);


        st = cpssTmTestTmClose(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }


    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssTmNodesCtlTransPortDelete(dev,portInd);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssTmNodesCtlTransPortDelete(dev,portInd);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*******************************************************************************
GT_STATUS cpssTmNodesCtlReadNextChange
(
    IN GT_U8                        devNum,
    OUT CPSS_TM_TREE_CHANGE_STC     *changePtr
)*/
UTF_TEST_CASE_MAC(cpssTmNodesCtlReadNextChange)
{
    GT_STATUS               st = GT_OK;
    GT_U8                           dev;
    GT_U32                          portInd;
    CPSS_TM_A_NODE_PARAMS_STC       aParamsPtr;
    CPSS_TM_B_NODE_PARAMS_STC       bParamsPtr;
    CPSS_TM_C_NODE_PARAMS_STC       cParamsPtr;
    GT_U32                          bNodeIndPtr1;
    GT_U32                          bNodeIndPtr2;
    GT_U32                          cNodeIndPtr1;
    GT_U32                          aNodeIndPtr1;
    GT_U32                          aNodeIndPtr2;
    GT_U32                          aNodeIndPtr3;
    GT_U32                          aNodeIndPtr4;
    GT_U32                          queueIndPtr1;
    GT_U32                          queueIndPtr2;
    GT_U32                          queueIndPtr3;
    CPSS_TM_QUEUE_PARAMS_STC        qParamsPtr;
    CPSS_TM_PORT_PARAMS_STC      paramsPtr;
/*    GT_U32                       cNodesNum;
    GT_U32                       bNodesNum;
    GT_U32                       aNodesNum;
    GT_U32                       queuesNum;*/
    CPSS_PP_FAMILY_TYPE_ENT devFamily;
    GT_U32                       ii;
    CPSS_TM_TREE_CHANGE_STC     changePtr;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev,
                                 UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E |
                                     UTF_PUMA2_E | UTF_PUMA3_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E | UTF_AC3X_E);

    cpssOsBzero((GT_VOID*)&aParamsPtr, sizeof(aParamsPtr));
    cpssOsBzero((GT_VOID*)&bParamsPtr, sizeof(bParamsPtr));
    cpssOsBzero((GT_VOID*)&cParamsPtr, sizeof(cParamsPtr));
    cpssOsBzero((GT_VOID*)&qParamsPtr, sizeof(qParamsPtr));
    cpssOsBzero((GT_VOID*)&paramsPtr, sizeof(paramsPtr));
    cpssOsBzero((GT_VOID*)&changePtr, sizeof(changePtr));


    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* get device family */
        st = prvUtfDeviceFamilyGet(dev, &devFamily);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfDeviceFamilyGet: %d", dev);

        /* TM initialization */

        st = cpssTmTestTmCtlLibInit(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssTmTestPeriodicSchemeConfiguration(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Configure parameters for node A */
        aParamsPtr.shapingProfilePtr = 0;
        aParamsPtr.quantum = 0x40;
        for (ii=0; ii<8; ii++)
            aParamsPtr.schdModeArr[ii] = CPSS_TM_SCHD_MODE_RR_E; /* No DWRR configured*/
        aParamsPtr.dropProfileInd = 0;
        aParamsPtr.eligiblePrioFuncId = 8;
        aParamsPtr.numOfChildren = 1;

        /* Configure parameters for node B */
        bParamsPtr.shapingProfilePtr = 0;
        bParamsPtr.quantum = 0x40;
        for (ii=0; ii<8; ii++)
            bParamsPtr.schdModeArr[ii] = CPSS_TM_SCHD_MODE_RR_E; /* No DWRR configured*/
        bParamsPtr.dropProfileInd = 0;
        bParamsPtr.eligiblePrioFuncId = 9;
        bParamsPtr.numOfChildren = 1;

        /* Configure parameters for node C */
        cParamsPtr.shapingProfilePtr = 0;
        cParamsPtr.quantum = 0x40;
        cParamsPtr.dropCosMap = 1;
        cParamsPtr.eligiblePrioFuncId = 9;
        for (ii=0; ii<8; ii++){
            cParamsPtr.schdModeArr[ii] = CPSS_TM_SCHD_MODE_RR_E;
            cParamsPtr.dropProfileIndArr[ii] = 0;
        }
        cParamsPtr.numOfChildren = 2;

        /* profile for Q level */
        qParamsPtr.shapingProfilePtr = 0;
        qParamsPtr.quantum = 0x40;
        qParamsPtr.dropProfileInd = 0;
        qParamsPtr.eligiblePrioFuncId = 1;

        /* Configure parameters for port */
        paramsPtr.cirBw = 1000000;
        paramsPtr.eirBw = 0;
        paramsPtr.cbs = 8; /*KBytes*/
        paramsPtr.ebs = 0x1FFFF;
        paramsPtr.eligiblePrioFuncId = CPSS_TM_ELIG_N_MIN_SHP_E;
        for(ii=0; ii<8; ii++){
            paramsPtr.schdModeArr[ii] = CPSS_TM_SCHD_MODE_RR_E;
            paramsPtr.quantumArr[ii] = UT_TM_MIN_PORT_QUANTUM_CNS;
        }
        paramsPtr.dropProfileInd = CPSS_TM_NO_DROP_PROFILE_CNS;
        paramsPtr.numOfChildren = 1;

        portInd=1;
/*
        cNodesNum=1;
        bNodesNum=2;
        aNodesNum=2;
        queuesNum=2;
*/
        /*creat port*/
        st = cpssTmAsymPortCreate(dev, portInd, &paramsPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* creat 1 c nodes*/
        st = cpssTmCnodeToPortCreate(dev, portInd, &cParamsPtr, &cNodeIndPtr1);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*creat 2 childrens of c1*/
         st = cpssTmBnodeToCnodeCreate(dev,cNodeIndPtr1, &bParamsPtr, &bNodeIndPtr1);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssTmBnodeToCnodeCreate(dev,cNodeIndPtr1, &bParamsPtr, &bNodeIndPtr2);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*creat children of b1*/
        st = cpssTmAnodeToBnodeCreate(dev, bNodeIndPtr1, &aParamsPtr ,&aNodeIndPtr1);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*creat children of b2*/
        st = cpssTmAnodeToBnodeCreate(dev, bNodeIndPtr2, &aParamsPtr ,&aNodeIndPtr2);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*creat children of a1*/
        st = cpssTmQueueToAnodeCreate(dev, aNodeIndPtr1, &qParamsPtr, &queueIndPtr1);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*creat children of a2*/
        st = cpssTmQueueToAnodeCreate(dev, aNodeIndPtr2, &qParamsPtr, &queueIndPtr2);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssTmNodesCtlQueueInstall(dev,queueIndPtr1);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssTmNodesCtlQueueInstall(dev,queueIndPtr2);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*creat children of b1*/
        st = cpssTmAnodeToBnodeCreate(dev, bNodeIndPtr1, &aParamsPtr ,&aNodeIndPtr3);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*creat children of a3*/
        st = cpssTmQueueToAnodeCreate(dev, aNodeIndPtr3, &qParamsPtr, &queueIndPtr3);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssTmNodesCtlQueueInstall(dev,queueIndPtr3);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*1.1 Check with legal parameters.
                Expected: GT_OK */

        st = cpssTmNodesCtlReadNextChange(dev,&changePtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st=GT_OK;
        if (changePtr.index != 0
            || changePtr.oldIndex !=0 || changePtr.newIndex !=2) {
            st=GT_FAIL;
        }

        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*1.2 Check with legal parameters.
                Expected: GT_OK */

         /*creat children of b1*/
        st = cpssTmAnodeToBnodeCreate(dev, bNodeIndPtr1, &aParamsPtr ,&aNodeIndPtr4);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssTmNodesCtlCleanList(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssTmNodesCtlReadNextChange(dev,&changePtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NO_RESOURCE, st, dev);

        /*
            1.3 Call with changePtr [NULL], other parameters same as in 1.1.
            Expected: GT_BAD_PTR.
        */

        st = cpssTmNodesCtlReadNextChange(dev,NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, changePtr = NULL", dev);

        st = cpssTmTestTmClose(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        }


    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssTmNodesCtlReadNextChange(dev,&changePtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssTmNodesCtlReadNextChange(dev,&changePtr);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
 * Configuration of cpssTmDrop suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssTmNodesCtl)
    UTF_SUIT_DECLARE_TEST_MAC(cpssTmNodesCtlQueueInstall)
    UTF_SUIT_DECLARE_TEST_MAC(cpssTmNodesCtlQueueUninstall)
    UTF_SUIT_DECLARE_TEST_MAC(cpssTmQueueFlush)
    UTF_SUIT_DECLARE_TEST_MAC(cpssTmQueueFlushTriggerActionStart)
    UTF_SUIT_DECLARE_TEST_MAC(cpssTmQueueFlushTriggerActionStatusGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssTmPortFlush)
    UTF_SUIT_DECLARE_TEST_MAC(cpssTmPortFlushTriggerActionStart)
    UTF_SUIT_DECLARE_TEST_MAC(cpssTmPortFlushTriggerActionStatusGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssTmNodesCtlNodeDelete)
    UTF_SUIT_DECLARE_TEST_MAC(cpssTmNodesCtlTransPortDelete)
    UTF_SUIT_DECLARE_TEST_MAC(cpssTmNodesCtlReadNextChange)
UTF_SUIT_END_TESTS_MAC(cpssTmNodesCtl)

