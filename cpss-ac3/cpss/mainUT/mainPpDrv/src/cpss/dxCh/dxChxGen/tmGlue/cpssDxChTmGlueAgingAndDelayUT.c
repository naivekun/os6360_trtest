/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* cpssDxChTmGlueAgingAndDelay.c
*
* DESCRIPTION:
*       Unit tests for CPSS DXCH Aging and Delay Measurements APIs
*
* DEPENDENCIES:
*
* FILE REVISION NUMBER:
*       $Revision: 4 $
*******************************************************************************/


#include <cpss/dxCh/dxChxGen/cpssDxChTypes.h>
#include <cpss/dxCh/dxChxGen/tmGlue/cpssDxChTmGlueAgingAndDelay.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <common/tgfCommon.h>

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTmGlueAgingAndDelayDropAgedPacketEnableSet
(
    IN GT_U8   devNum,
    IN GT_BOOL enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChTmGlueAgingAndDelayDropAgedPacketEnableSet)
{
/*
    ITERATE_DEVICES (Bobcat2)
    1.1. Call with enable[GT_FALSE, GT_TRUE].
    Expected: GT_OK.
    1.2. Call cpssDxChTmGlueAgingAndDelayDropAgePacketEnableGet.
    Expected: GT_OK and the same values as was set.
*/
    GT_STATUS                               st          = GT_OK;
    GT_U8                                   dev         = 0;
    GT_BOOL                                 enable      = GT_FALSE;
    GT_BOOL                                 enableRet   = GT_FALSE;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E |
                                           UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E | UTF_AC3X_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with enable[GT_FALSE, GT_TRUE].
            Expected: GT_OK.
        */

        /* call with enable[GT_FALSE] */
        enable = GT_FALSE;

        st = cpssDxChTmGlueAgingAndDelayDropAgedPacketEnableSet(dev, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call cpssDxChTmGlueAgingAndDelayDropAgedPacketEnableGet.
                Expected: GT_OK and the same values as was set.
        */
        st = cpssDxChTmGlueAgingAndDelayDropAgedPacketEnableGet(dev, &enableRet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        if (GT_OK == st)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableRet,
                                         "got another enable: %d", enableRet);
        }

        /* call with enable[GT_TRUE] */
        enable = GT_TRUE;

        st = cpssDxChTmGlueAgingAndDelayDropAgedPacketEnableSet(dev, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call cpssDxChTmGlueAgingAndDelayDropAgedPacketEnableGet.
                Expected: GT_OK and the same values as was set.
        */
        st = cpssDxChTmGlueAgingAndDelayDropAgedPacketEnableGet(dev, &enableRet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        if (GT_OK == st)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableRet,
                                         "got another enable: %d", enableRet);
        }
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E |
                                           UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E | UTF_AC3X_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTmGlueAgingAndDelayDropAgedPacketEnableSet(dev, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTmGlueAgingAndDelayDropAgedPacketEnableSet(dev, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTmGlueAgingAndDelayDropAgedPacketEnableGet
(
    IN GT_U8    devNum,
    OUT GT_BOOL *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChTmGlueAgingAndDelayDropAgedPacketEnableGet)
{
/*
    ITERATE_DEVICES (Bobcat2)
    1.1. Call with non-null enablePtr.
    Expected: GT_OK.
    1.2. Call with out of range enablePtr[NULL] and other valid params.
    Expected: NON GT_OK.
*/
    GT_STATUS                               st              = GT_OK;
    GT_U8                                   dev             = 0;
    GT_BOOL                                 enable          = GT_FALSE;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E |
                                           UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E | UTF_AC3X_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with non-null enablePtr.
            Expected: GT_OK.
        */
        st = cpssDxChTmGlueAgingAndDelayDropAgedPacketEnableGet(dev, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
        1.2. Call with out of range enablePtr[NULL] and other valid params.
        Expected: NON GT_OK.
        */
        st = cpssDxChTmGlueAgingAndDelayDropAgedPacketEnableGet(dev, NULL);
        UTF_VERIFY_NOT_EQUAL1_STRING_MAC(GT_OK, st,
                                         "cpssDxChTmGlueAgingAndDelayDropAgedPacketEnableGet: %d", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E |
                                           UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E | UTF_AC3X_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTmGlueAgingAndDelayDropAgedPacketEnableGet(dev, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTmGlueAgingAndDelayDropAgedPacketEnableGet(dev, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTmGlueAgingAndDelayAgedPacketsCountingEnableSet
(
    IN GT_U8   devNum,
    IN GT_BOOL enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChTmGlueAgingAndDelayAgedPacketsCountingEnableSet)
{
/*
    ITERATE_DEVICES (Bobcat2)
    1.1. Call with enable[GT_FALSE, GT_TRUE].
    Expected: GT_OK.
    1.2. Call cpssDxChTmGlueAgingAndDelayDroppedPacketsCountingEnableGet.
    Expected: GT_OK and the same values as was set.
*/
    GT_STATUS                               st          = GT_OK;
    GT_U8                                   dev         = 0;
    GT_BOOL                                 enable      = GT_FALSE;
    GT_BOOL                                 enableRet   = GT_FALSE;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E |
                                           UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E | UTF_AC3X_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with enable[GT_FALSE, GT_TRUE].
            Expected: GT_OK.
        */

        /* call with enable[GT_FALSE] */
        enable = GT_FALSE;

        st = cpssDxChTmGlueAgingAndDelayAgedPacketsCountingEnableSet(dev, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call cpssDxChTmGlueAgingAndDelayDroppedPacketsCountingEnableGet.
                Expected: GT_OK and the same values as was set.
        */
        st = cpssDxChTmGlueAgingAndDelayAgedPacketsCountingEnableGet(dev, &enableRet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        if (GT_OK == st)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableRet,
                                         "got another enable: %d", enableRet);
        }

        /* call with enable[GT_TRUE] */
        enable = GT_TRUE;

        st = cpssDxChTmGlueAgingAndDelayAgedPacketsCountingEnableSet(dev, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call cpssDxChTmGlueAgingAndDelayDroppedPacketsCountingEnableGet.
                Expected: GT_OK and the same values as was set.
        */
        st = cpssDxChTmGlueAgingAndDelayAgedPacketsCountingEnableGet(dev, &enableRet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        if (GT_OK == st)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableRet,
                                         "got another enable: %d", enableRet);
        }
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E |
                                           UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E | UTF_AC3X_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTmGlueAgingAndDelayAgedPacketsCountingEnableSet(dev, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTmGlueAgingAndDelayAgedPacketsCountingEnableSet(dev, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTmGlueAgingAndDelayAgedPacketsCountingEnableGet
(
    IN GT_U8    devNum,
    OUT GT_BOOL *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChTmGlueAgingAndDelayAgedPacketsCountingEnableGet)
{
/*
    ITERATE_DEVICES (Bobcat2)
    1.1. Call with non-null enablePtr.
    Expected: GT_OK.
    1.2. Call with out of range enablePtr[NULL] and other valid params.
    Expected: NON GT_OK.
*/
    GT_STATUS                               st              = GT_OK;
    GT_U8                                   dev             = 0;
    GT_BOOL                                 enable          = GT_FALSE;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E |
                                           UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E | UTF_AC3X_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with non-null enablePtr.
            Expected: GT_OK.
        */
        st = cpssDxChTmGlueAgingAndDelayAgedPacketsCountingEnableGet(dev, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
        1.2. Call with out of range enablePtr[NULL] and other valid params.
        Expected: NON GT_OK.
        */
        st = cpssDxChTmGlueAgingAndDelayAgedPacketsCountingEnableGet(dev, NULL);
        UTF_VERIFY_NOT_EQUAL1_STRING_MAC(GT_OK, st,
                                         "cpssDxChTmGlueAgingAndDelayDroppedPacketsCountingEnableGet: %d", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E |
                                           UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E | UTF_AC3X_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTmGlueAgingAndDelayAgedPacketsCountingEnableGet(dev, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTmGlueAgingAndDelayAgedPacketsCountingEnableGet(dev, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTmGlueGeneralPerPortDropOutgoingPacketEnableSet
(
    IN  GT_U8            devNum,
    IN  GT_U32           tmPort,
    IN  GT_BOOL          enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChTmGlueGeneralPerPortDropOutgoingPacketEnableSet)
{
/*
    ITERATE_DEVICES (Bobcat2)
    1.1. Call with enable[GT_FALSE, GT_TRUE].
    Expected: GT_OK.
    1.2. Call cpssDxChTmGlueAgingAndDelayPerPortDropOutgoingPacketEnableGet.
    Expected: GT_OK and the same values as was set.
    1.3. Call with tmPort[192].
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS                               st          = GT_OK;
    GT_U8                                   dev         = 0;
    GT_BOOL                                 enable      = GT_FALSE;
    GT_BOOL                                 enableRet   = GT_FALSE;
    GT_U32                                  tmPort      = 191;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E |
                                           UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E | UTF_AC3X_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        tmPort      = 191;
        /*
            1.1. Call with enable[GT_FALSE, GT_TRUE].
            Expected: GT_OK.
        */

        /* call with enable[GT_FALSE] */
        enable = GT_FALSE;

        st = cpssDxChTmGlueGeneralPerPortDropOutgoingPacketEnableSet(dev,tmPort,enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call cpssDxChTmGlueAgingAndDelayPerPortDropOutgoingPacketEnableGet.
                Expected: GT_OK and the same values as was set.
        */
        st = cpssDxChTmGlueGeneralPerPortDropOutgoingPacketEnableGet(dev,tmPort,&enableRet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        if (GT_OK == st)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableRet,
                                         "got another enable: %d", enableRet);
        }

        /* call with enable[GT_TRUE] */
        enable = GT_TRUE;

        st = cpssDxChTmGlueGeneralPerPortDropOutgoingPacketEnableSet(dev,tmPort, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call cpssDxChTmGlueAgingAndDelayPerPortDropOutgoingPacketEnableGet.
                Expected: GT_OK and the same values as was set.
        */
        st = cpssDxChTmGlueGeneralPerPortDropOutgoingPacketEnableGet(dev,tmPort, &enableRet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        if (GT_OK == st)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableRet,
                                         "got another enable: %d", enableRet);
        }

        /* 1.3. Call with tmPort[192].
            Expected: GT_BAD_PARAM.
        */
        tmPort=192;
        st = cpssDxChTmGlueGeneralPerPortDropOutgoingPacketEnableSet(dev,tmPort, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    tmPort=191;
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E |
                                           UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E | UTF_AC3X_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTmGlueGeneralPerPortDropOutgoingPacketEnableSet(dev,tmPort, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTmGlueGeneralPerPortDropOutgoingPacketEnableSet(dev,tmPort, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTmGlueGeneralPerPortDropOutgoingPacketEnableGet
(
    IN  GT_U8    devNum,
    IN  GT_U32  tmPort,
    OUT GT_BOOL *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChTmGlueGeneralPerPortDropOutgoingPacketEnableGet)
{
/*
    ITERATE_DEVICES (Bobcat2)
    1.1. Call with non-null enablePtr.
    Expected: GT_OK.
    1.2. Call with out of range enablePtr[NULL] and other valid params.
    Expected: NON GT_OK.
    1.3. Call with tmPort[192].
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS                               st              = GT_OK;
    GT_U8                                   dev             = 0;
    GT_BOOL                                 enable          = GT_FALSE;
    GT_U32                                  tmPort          = 191;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E |
                                           UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E | UTF_AC3X_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        tmPort          = 191;
        /*
            1.1. Call with non-null enablePtr.
            Expected: GT_OK.
        */
        st = cpssDxChTmGlueGeneralPerPortDropOutgoingPacketEnableGet(dev,tmPort, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
        1.2. Call with out of range enablePtr[NULL] and other valid params.
        Expected: NON GT_OK.
        */
        st = cpssDxChTmGlueGeneralPerPortDropOutgoingPacketEnableGet(dev,tmPort, NULL);
        UTF_VERIFY_NOT_EQUAL1_STRING_MAC(GT_OK, st,
                                         "cpssDxChTmGlueAgingAndDelayPerPortDropOutgoingPacketEnableGet: %d", dev);


        /* 1.3. Call with tmPort[192].
            Expected: GT_BAD_PARAM.
        */
        tmPort=192;
        st = cpssDxChTmGlueGeneralPerPortDropOutgoingPacketEnableGet(dev,tmPort, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    tmPort=191;
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E |
                                           UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E | UTF_AC3X_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTmGlueGeneralPerPortDropOutgoingPacketEnableGet(dev,tmPort, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTmGlueGeneralPerPortDropOutgoingPacketEnableGet(dev,tmPort, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTmGlueAgingAndDelayAgedPacketCounterConfigSet
(
    IN  GT_U8            devNum,
    IN  GT_U32           agedPacketCouterQueueMask,
    IN  GT_U32           agedPacketCouterQueue
*/
UTF_TEST_CASE_MAC(cpssDxChTmGlueAgingAndDelayAgedPacketCounterConfigSet)
{
/*
    ITERATE_DEVICES (Bobcat2)
    1.1. Call with agedPacketCouterQueueMask[5],agedPacketCouterQueue[5]
    Expected: GT_OK
    1.2. Call cpssDxChTmGlueAgingAndDelayAgedPacketCounterConfigGet.
    Expected: GT_OK and the same value as was set.
    1.3. Call with agedPacketCouterQueueMask[16383],agedPacketCouterQueue[16383]
    Expected: GT_OK
    1.4. Call cpssDxChTmGlueAgingAndDelayAgedPacketCounterConfigGet.
    Expected: GT_OK and the same value as was set.
    1.5. Call with agedPacketCouterQueueMask[16383+1],agedPacketCouterQueue[16383+1]
    Expected: GT_OUT_OF_RANGE

*/
    GT_STATUS                               st                = GT_OK;
    GT_U8                                   dev               = 0;
    GT_U32                                  agedPacketCouterQueueMask=0;
    GT_U32                                  agedPacketCouterQueue=0;
    GT_U32                                  agedPacketCouterQueueMaskRet=0;
    GT_U32                                  agedPacketCouterQueueRet=0;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E |
                                           UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E | UTF_AC3X_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with agedPacketCouterQueueMask[5],agedPacketCouterQueue[5]
            Expected: GT_OK
        */

        agedPacketCouterQueueMask=5;
        agedPacketCouterQueue=5;

        st = cpssDxChTmGlueAgingAndDelayAgedPacketCounterConfigSet(dev,
                                                                   agedPacketCouterQueueMask,
                                                                   agedPacketCouterQueue);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call cpssDxChTmGlueAgingAndDelayAgedPacketCounterConfigGet.
                Expected: GT_OK and the same value as was set.
        */
        st = cpssDxChTmGlueAgingAndDelayAgedPacketCounterConfigGet(dev,
                                                                  &agedPacketCouterQueueMaskRet,
                                                                  &agedPacketCouterQueueRet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        if (GT_OK == st)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(agedPacketCouterQueueMask, agedPacketCouterQueueMaskRet,
                                         "got another agedPacketCouterQueueMaskRet: %d", agedPacketCouterQueueMaskRet);

            UTF_VERIFY_EQUAL1_STRING_MAC(agedPacketCouterQueue, agedPacketCouterQueueRet,
                                         "got another agedPacketCouterQueueRet: %d", agedPacketCouterQueueRet);
        }

        /*
            1.3. Call with agedPacketCouterQueueMask[16383],agedPacketCouterQueue[16383]
            Expected: GT_OK
        */

        agedPacketCouterQueueMask=16383;
        agedPacketCouterQueue=16383;


        st = cpssDxChTmGlueAgingAndDelayAgedPacketCounterConfigSet(dev,
                                                                   agedPacketCouterQueueMask,
                                                                   agedPacketCouterQueue);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        /*
            1.4. Call cpssDxChTmGlueAgingAndDelayAgedPacketCounterConfigGet.
                Expected: GT_OK and the same value as was set.
        */
        st = cpssDxChTmGlueAgingAndDelayAgedPacketCounterConfigGet(dev,
                                                                  &agedPacketCouterQueueMaskRet,
                                                                  &agedPacketCouterQueueRet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        if (GT_OK == st)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(agedPacketCouterQueueMask, agedPacketCouterQueueMaskRet,
                                         "got another agedPacketCouterQueueMaskRet: %d", agedPacketCouterQueueMaskRet);

            UTF_VERIFY_EQUAL1_STRING_MAC(agedPacketCouterQueue, agedPacketCouterQueueRet,
                                         "got another agedPacketCouterQueueRet: %d", agedPacketCouterQueueRet);
        }

        /*
            1.5. Call with agedPacketCouterQueueMask[16383+1],agedPacketCouterQueue[16383+1]
            Expected: GT_OK
        */

        agedPacketCouterQueueMask=16384;
        agedPacketCouterQueue=16384;

        st = cpssDxChTmGlueAgingAndDelayAgedPacketCounterConfigSet(dev,
                                                                   agedPacketCouterQueueMask,
                                                                   agedPacketCouterQueue);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, dev);

    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    agedPacketCouterQueueMask=0;
    agedPacketCouterQueue=0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E |
                                           UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E | UTF_AC3X_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTmGlueAgingAndDelayAgedPacketCounterConfigSet(dev,
                                                                   agedPacketCouterQueueMask,
                                                                   agedPacketCouterQueue);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTmGlueAgingAndDelayAgedPacketCounterConfigSet(dev,
                                                               agedPacketCouterQueueMask,
                                                               agedPacketCouterQueue);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTmGlueAgingAndDelayAgedPacketCounterConfigGet
(
    IN  GT_U8            devNum,
    OUT GT_U32           *agedPacketCouterQueueMaskPtr,
    OUT GT_U32           *agedPacketCouterQueuePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChTmGlueAgingAndDelayAgedPacketCounterConfigGet)
{
/*
    ITERATE_DEVICES (Bobcat2)
    1.1. Call with non-null agedPacketCouterQueueMaskPtr and agedPacketCouterQueuePtr.
    Expected: GT_OK.
    1.2. Call with out of range agedPacketCouterQueueMaskPtr[NULL] and other valid params.
    Expected: NON GT_OK.
    1.3. Call with out of range agedPacketCouterQueuePtr[NULL] and other valid params.
    Expected: NON GT_OK.
*/
    GT_STATUS                               st              = GT_OK;
    GT_U8                                   dev             = 0;
    GT_U32                                  agedPacketCouterQueueMask;
    GT_U32                                  agedPacketCouterQueue;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E |
                                           UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E | UTF_AC3X_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with non-null enablePtr.
            Expected: GT_OK.
        */
        st = cpssDxChTmGlueAgingAndDelayAgedPacketCounterConfigGet(dev,
                                                                  &agedPacketCouterQueueMask,
                                                                  &agedPacketCouterQueue);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
        1.2. Call with out of range agedPacketCouterQueueMask[NULL] and other valid params.
        Expected: NON GT_OK.
        */
        st = cpssDxChTmGlueAgingAndDelayAgedPacketCounterConfigGet(dev,
                                                                  NULL,
                                                                  &agedPacketCouterQueue);
        UTF_VERIFY_NOT_EQUAL1_STRING_MAC(GT_OK, st,
                                         "cpssDxChTmGlueAgingAndDelayAgedPacketCounterConfigGet: %d", dev);

        /*
        1.3. Call with out of range agedPacketCouterQueue[NULL] and other valid params.
        Expected: NON GT_OK.
        */
        st = cpssDxChTmGlueAgingAndDelayAgedPacketCounterConfigGet(dev,
                                                                  &agedPacketCouterQueueMask,
                                                                  NULL);
        UTF_VERIFY_NOT_EQUAL1_STRING_MAC(GT_OK, st,
                                         "cpssDxChTmGlueAgingAndDelayAgedPacketCounterConfigGet: %d", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E |
                                           UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E | UTF_AC3X_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTmGlueAgingAndDelayAgedPacketCounterConfigGet(dev,
                                                                  &agedPacketCouterQueueMask,
                                                                  &agedPacketCouterQueue);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTmGlueAgingAndDelayAgedPacketCounterConfigGet(dev,
                                                              &agedPacketCouterQueueMask,
                                                              &agedPacketCouterQueue);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTmGlueAgingAndDelayAgedPacketCounterGet
(
    IN  GT_U8            devNum,
    OUT GT_U32           *agedPacketCounterPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChTmGlueAgingAndDelayAgedPacketCounterGet)
{
/*
    ITERATE_DEVICES (Bobcat2)
    1.1. Call with agedPacketCounterPtr and other valid params.
    Expected: GT_OK.
    1.2. Call with out of range agedPacketCounterPtr and other valid params.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS       st          = GT_OK;
    GT_U8           dev         = 0;
    GT_U32          agedPacketCounter = 0;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E |
                                           UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E | UTF_AC3X_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with agedPacketCounterPtr and other valid params.
            Expected: GT_OK.
        */

        st = cpssDxChTmGlueAgingAndDelayAgedPacketCounterGet(dev, &agedPacketCounter);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with out of range agedPacketCounterPtr and other valid params.
                Expected: GT_BAD_PTR.
        */
        st = cpssDxChTmGlueAgingAndDelayAgedPacketCounterGet(dev, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E |
                                           UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E | UTF_AC3X_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTmGlueAgingAndDelayAgedPacketCounterGet(dev, &agedPacketCounter);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChTmGlueAgingAndDelayAgedPacketCounterGet(dev, &agedPacketCounter);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTmGlueAgingAndDelayTimerResolutionSet
(
    IN GT_U8                    devNum,
    IN GT_U32                   timerResolution
*/
UTF_TEST_CASE_MAC(cpssDxChTmGlueAgingAndDelayTimerResolutionSet)
{
/*
    ITERATE_DEVICES (Bobcat2)
    1.1. Call with timerResolution[73000]
         Expected: GT_OK
    1.2. Call cpssDxChTmGlueAgingAndDelayTimerResolutionGet.
         Expected: GT_OK and the same value as was set.
    1.3. Call with timerResolution[maxTimerResolution]
         Expected: GT_OK
    1.4. Call cpssDxChTmGlueAgingAndDelayTimerResolutionGet.
         Expected: GT_OK and the same value as was set.
    1.5. Call with timerResolution[maxTimerResolution+coreClockPeriod]
         Expected: GT_OUT_OF_RANGE
*/
    GT_STATUS st                    = GT_OK;
    GT_U8     dev                   = 0;
    GT_U32    timerResolutionOrig    = 0;
    GT_U32    timerResolution       = 0;
    GT_U32    timerResolutionRet    = 0;
    GT_U32    maxInitialValue;     /* max units resolution in coreClock */
    GT_U32    maxTimerResolution;  /* max units resolution in nanosec */
    double    fCoreClockPeriod;     /* core Clock Period in nanosec */
        int       cond;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E |
                                           UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E | UTF_AC3X_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with timerResolution[73000]
            Expected: GT_OK
        */

        timerResolution=73000;
        timerResolutionOrig=timerResolution;

        st = cpssDxChTmGlueAgingAndDelayTimerResolutionSet(dev,&timerResolution);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        if (GT_OK == st)
        {
                        cond = (timerResolution==timerResolutionOrig) || (timerResolution - timerResolutionOrig == 1)  || (timerResolutionOrig - timerResolution == 1)  ;
            UTF_VERIFY_EQUAL1_STRING_MAC(cond , 1, "wrong  timerResolution calculation:  %d", timerResolution);
        }

        /*
            1.2. Call cpssDxChTmGlueAgingAndDelayTimerResolutionGet.
                Expected: GT_OK and the same value as was set.
        */
        st = cpssDxChTmGlueAgingAndDelayTimerResolutionGet(dev , &timerResolutionRet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        if (GT_OK == st)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(timerResolution, timerResolutionRet, "got another timerResolutionRet: %d", timerResolutionRet);
        }

        /*
            1.3. Call with timerResolution[maxTimerResolution]
            Expected: GT_OK
        */

        maxInitialValue = 0x1FFFF;

        /* "calculate" Core Clock period in nanoseconds */
        fCoreClockPeriod = 1000.0 / (PRV_CPSS_PP_MAC(dev)->coreClock);

        /* calculate timerResolution */
        maxTimerResolution =(GT_U32)( maxInitialValue * fCoreClockPeriod);

        timerResolution=maxTimerResolution;
        timerResolutionOrig=timerResolution;

        st = cpssDxChTmGlueAgingAndDelayTimerResolutionSet(dev , &timerResolution);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        /*
            1.4. Call cpssDxChTmGlueAgingAndDelayTimerResolutionGet.
                Expected: GT_OK and the same value as was set.
        */
        if (GT_OK == st)
        {
                        cond = (timerResolution==timerResolutionOrig) || (timerResolution - timerResolutionOrig == 1)  || (timerResolutionOrig - timerResolution == 1)  ;
            UTF_VERIFY_EQUAL1_STRING_MAC(cond , 1, "wrong  timerResolution calculation:  %d", timerResolution);
        }


                st = cpssDxChTmGlueAgingAndDelayTimerResolutionGet(dev,&timerResolutionRet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

                if (GT_OK == st)
        {

            UTF_VERIFY_EQUAL1_STRING_MAC(timerResolution, timerResolutionRet,
                                         "got another timerResolutionRet: %d", timerResolutionRet);
        }

        /*
            1.5. Call with timerResolution maxTimerResolution + one clock more
            Expected: GT_OUT_OF_RANGE
        */

        timerResolution = timerResolution+(GT_U32)(fCoreClockPeriod+0.5);
        timerResolutionOrig=timerResolution;

        st = cpssDxChTmGlueAgingAndDelayTimerResolutionSet(dev,&timerResolution);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, dev);

    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    timerResolution=0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E |
                                           UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E | UTF_AC3X_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTmGlueAgingAndDelayTimerResolutionSet(dev,&timerResolution);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTmGlueAgingAndDelayTimerResolutionSet(dev,&timerResolution);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTmGlueAgingAndDelayTimerResolutionGet
(
    IN  GT_U8                    devNum,
    OUT GT_U32                   *timerResolutionPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChTmGlueAgingAndDelayTimerResolutionGet)
{
/*
    ITERATE_DEVICES (Bobcat2)
    1.1. Call with non-null timerResolutionPtr
    Expected: GT_OK.
    1.2. Call with out of range timerResolutionPtr[NULL] and other valid params.
    Expected: NON GT_OK.
*/
    GT_STATUS                               st              = GT_OK;
    GT_U8                                   dev             = 0;
    GT_U32                                  timerResolution = 0;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E |
                                           UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E | UTF_AC3X_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with non-null enablePtr.
            Expected: GT_OK.
        */
        st = cpssDxChTmGlueAgingAndDelayTimerResolutionGet(dev,&timerResolution);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
        1.2. Call with out of range timerResolution[NULL] and other valid params.
        Expected: NON GT_OK.
        */
        st = cpssDxChTmGlueAgingAndDelayTimerResolutionGet(dev,NULL);
        UTF_VERIFY_NOT_EQUAL1_STRING_MAC(GT_OK, st,
                                         "cpssDxChTmGlueAgingAndDelayTimerResolutionGet: %d", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E |
                                           UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E | UTF_AC3X_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTmGlueAgingAndDelayTimerResolutionGet(dev,&timerResolution);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTmGlueAgingAndDelayTimerResolutionGet(dev,&timerResolution);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTmGlueAgingAndDelayTimerGet
(
    IN  GT_U8                    devNum,
    OUT GT_U32                   *timerPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChTmGlueAgingAndDelayTimerGet)
{
/*
    ITERATE_DEVICES (Bobcat2)
    1.1. Call with non-null timerPtr
    Expected: GT_OK.
    1.2. Call with out of range timerPtr[NULL] and other valid params.
    Expected: NON GT_OK.
*/
    GT_STATUS                               st          = GT_OK;
    GT_U8                                   dev         = 0;
    GT_U32                                  timer       = 0;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E |
                                           UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E | UTF_AC3X_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with non-null timer.
            Expected: GT_OK.
        */
        st = cpssDxChTmGlueAgingAndDelayTimerGet(dev,&timer);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
        1.2. Call with out of range timer[NULL] and other valid params.
        Expected: NON GT_OK.
        */
        st = cpssDxChTmGlueAgingAndDelayTimerGet(dev,NULL);
        UTF_VERIFY_NOT_EQUAL1_STRING_MAC(GT_OK, st,
                                         "cpssDxChTmGlueAgingAndDelayTimerGet: %d", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E |
                                           UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E | UTF_AC3X_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTmGlueAgingAndDelayTimerGet(dev,&timer);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTmGlueAgingAndDelayTimerGet(dev,&timer);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTmGlueAgingAndDelayAgingProfileThresholdsSet
(
    IN GT_U8                                                devNum,
    IN GT_U32                                               profileId,
    IN CPSS_DXCH_TM_AGING_AND_DELAY_PROFILE_THRESHOLDS_STC  *thresholdsPtr
*/
UTF_TEST_CASE_MAC(cpssDxChTmGlueAgingAndDelayAgingProfileThresholdsSet)
{
/*
    ITERATE_DEVICES (Bobcat2)
    1.1. Call with profileId[4],threshold0 = 10, threshold1=20, threshold2=30
    Expected: GT_OK
    1.2. Call cpssDxChTmGlueAgingAndDelayAgingProfileThresholdsGet.
    Expected: GT_OK and the same value as was set.
    1.3. Call with profileId[15],threshold0 = 0xFFFFF, threshold1=0xFFFFF, threshold2=0xFFFFF
    Expected: GT_OK
    1.4. Call cpssDxChTmGlueAgingAndDelayAgingProfileThresholdsGet.
    Expected: GT_OK and the same value as was set.
    1.5. Call with profileId[15+1],threshold0 = 0xFFFFF, threshold1=0xFFFFF, threshold2=0xFFFFF
    Expected: GT_OUT_OF_RANGE
    1.6. Call with profileId[15],threshold0 = 0xFFFFF+1, threshold1=0xFFFFF, threshold2=0xFFFFF
    Expected: GT_OUT_OF_RANGE
    1.7. Call with profileId[15],threshold0 = 0xFFFFF, threshold1=0xFFFFF+1, threshold2=0xFFFFF
    Expected: GT_OUT_OF_RANGE
    1.8. Call with profileId[15],threshold0 = 0xFFFFF, threshold1=0xFFFFF, threshold2=0xFFFFF+1
    Expected: GT_OUT_OF_RANGE
    1.9. Call with profileId[2],thresholdsPtr[NULL]
    Expected: GT_BAD_PTR

*/
    GT_STATUS                               st        = GT_OK;
    GT_U8                                   dev       = 0;
    GT_U32                                  profileId = 0;
    CPSS_DXCH_TM_AGING_AND_DELAY_PROFILE_THRESHOLDS_STC  thresholds;
    CPSS_DXCH_TM_AGING_AND_DELAY_PROFILE_THRESHOLDS_STC  thresholdsRet;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E |
                                           UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E | UTF_AC3X_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with profileId[4],threshold0 = 10, threshold1=20, threshold2=30
            Expected: GT_OK
        */

        profileId = 4;
        thresholds.threshold0=10;
        thresholds.threshold1=20;
        thresholds.threshold2=30;


        st = cpssDxChTmGlueAgingAndDelayAgingProfileThresholdsSet(dev,profileId,&thresholds);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call cpssDxChTmGlueAgingAndDelayAgingProfileThresholdsGet.
                Expected: GT_OK and the same value as was set.
        */
        st = cpssDxChTmGlueAgingAndDelayAgingProfileThresholdsGet(dev,profileId,&thresholdsRet);

        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        if (GT_OK == st)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(thresholds.threshold0, thresholdsRet.threshold0,
                                         "got another thresholdsRet.threshold0: %d", thresholdsRet.threshold0);

            UTF_VERIFY_EQUAL1_STRING_MAC(thresholds.threshold1, thresholdsRet.threshold1,
                                         "got another thresholdsRet.threshold1: %d", thresholdsRet.threshold1);

            UTF_VERIFY_EQUAL1_STRING_MAC(thresholds.threshold2, thresholdsRet.threshold2,
                                         "got another thresholdsRet.threshold2: %d", thresholdsRet.threshold2);
        }

        /*
            1.3. Call with profileId[15],threshold0 = 0xFFFFF, threshold1=0xFFFFF, threshold2=0xFFFFF
            Expected: GT_OK
        */

        profileId = 15;
        thresholds.threshold0=0xFFFFF;
        thresholds.threshold1=0xFFFFF;
        thresholds.threshold2=0xFFFFF;

        st = cpssDxChTmGlueAgingAndDelayAgingProfileThresholdsSet(dev,profileId,&thresholds);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.4. Call cpssDxChTmGlueAgingAndDelayAgingProfileThresholdsGet.
            Expected: GT_OK and the same value as was set.
        */
        st = cpssDxChTmGlueAgingAndDelayAgingProfileThresholdsGet(dev,profileId,&thresholdsRet);

        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        if (GT_OK == st)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(thresholds.threshold0, thresholdsRet.threshold0,
                                         "got another thresholdsRet.threshold0: %d", thresholdsRet.threshold0);

            UTF_VERIFY_EQUAL1_STRING_MAC(thresholds.threshold1, thresholdsRet.threshold1,
                                         "got another thresholdsRet.threshold1: %d", thresholdsRet.threshold1);

            UTF_VERIFY_EQUAL1_STRING_MAC(thresholds.threshold2, thresholdsRet.threshold2,
                                         "got another thresholdsRet.threshold2: %d", thresholdsRet.threshold2);
        }

        /*
             1.5. Call with profileId[15+1],threshold0 = 0xFFFFF, threshold1=0xFFFFF, threshold2=0xFFFFF
            Expected: GT_OUT_OF_RANGE
        */

        profileId = 15+1;
        thresholds.threshold0=0xFFFFF;
        thresholds.threshold1=0xFFFFF;
        thresholds.threshold2=0xFFFFF;

        st = cpssDxChTmGlueAgingAndDelayAgingProfileThresholdsSet(dev,profileId,&thresholds);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, dev);

         /*
            1.6. Call with profileId[15],threshold0 = 0xFFFFF+1, threshold1=0xFFFFF, threshold2=0xFFFFF
            Expected: GT_OUT_OF_RANGE
        */

        profileId = 15;
        thresholds.threshold0=0xFFFFF+1;
        thresholds.threshold1=0xFFFFF;
        thresholds.threshold2=0xFFFFF;

        st = cpssDxChTmGlueAgingAndDelayAgingProfileThresholdsSet(dev,profileId,&thresholds);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, dev);

        /*
            1.7. Call with profileId[15],threshold0 = 0xFFFFF, threshold1=0xFFFFF+1, threshold2=0xFFFFF
            Expected: GT_OUT_OF_RANGE
        */

        profileId = 15;
        thresholds.threshold0=0xFFFFF;
        thresholds.threshold1=0xFFFFF+1;
        thresholds.threshold2=0xFFFFF;

        st = cpssDxChTmGlueAgingAndDelayAgingProfileThresholdsSet(dev,profileId,&thresholds);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, dev);

        /*
             1.8. Call with profileId[15],threshold0 = 0xFFFFF, threshold1=0xFFFFF, threshold2=0xFFFFF+1
            Expected: GT_OUT_OF_RANGE
        */

        profileId = 15;
        thresholds.threshold0=0xFFFFF;
        thresholds.threshold1=0xFFFFF;
        thresholds.threshold2=0xFFFFF+1;

        st = cpssDxChTmGlueAgingAndDelayAgingProfileThresholdsSet(dev,profileId,&thresholds);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, dev);

        /*
            1.9. Call with profileId[2],thresholdsPtr[NULL]
            Expected: GT_BAD_PTR
        */

        st = cpssDxChTmGlueAgingAndDelayAgingProfileThresholdsSet(dev,profileId,NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    profileId = 4;
    thresholds.threshold0=10;
    thresholds.threshold1=20;
    thresholds.threshold2=30;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E |
                                           UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E | UTF_AC3X_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTmGlueAgingAndDelayAgingProfileThresholdsSet(dev,profileId,&thresholds);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTmGlueAgingAndDelayAgingProfileThresholdsSet(dev,profileId,&thresholds);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTmGlueAgingAndDelayAgingProfileThresholdsGet
(
    IN  GT_U8                                               devNum,
    IN  GT_U32                                              profileId,
    OUT CPSS_DXCH_TM_AGING_AND_DELAY_PROFILE_THRESHOLDS_STC *thresholdsPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChTmGlueAgingAndDelayAgingProfileThresholdsGet)
{
/*
    ITERATE_DEVICES (Bobcat2)
    1.1. Call with profileId[7] and non-null thresholdsPtr
    Expected: GT_OK.
    1.2. Call with out of range thresholdsPtr[NULL] and other valid params.
    Expected: NON GT_OK.
    1.3. Call with out of range profileId[16] and other valid params.
    Expected: NON GT_OK.
*/
    GT_STATUS                               st              = GT_OK;
    GT_U8                                   dev             = 0;
    GT_U32                                                  profileId=0;
    CPSS_DXCH_TM_AGING_AND_DELAY_PROFILE_THRESHOLDS_STC     thresholds;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E |
                                           UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E | UTF_AC3X_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with profileId[7] and non-null thresholdsPtr
            Expected: GT_OK.
        */
        profileId=7;
        st = cpssDxChTmGlueAgingAndDelayAgingProfileThresholdsGet(dev,profileId,&thresholds);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
        1.2. Call with out of range dropAgedPacketCouterQueueMask[NULL] and other valid params.
        Expected: NON GT_OK.
        */
        st = cpssDxChTmGlueAgingAndDelayAgingProfileThresholdsGet(dev,profileId,NULL);
        UTF_VERIFY_NOT_EQUAL1_STRING_MAC(GT_OK, st,
                                         "cpssDxChTmGlueAgingAndDelayAgingProfileThresholdsGet: %d", dev);

        /*
            1.3. Call with out of range profileId[16] and other valid params.
            Expected: NON GT_OK.
        */
        profileId=16;
        st = cpssDxChTmGlueAgingAndDelayAgingProfileThresholdsGet(dev,profileId,&thresholds);
        UTF_VERIFY_NOT_EQUAL1_STRING_MAC(GT_OK, st,
                                         "cpssDxChTmGlueAgingAndDelayAgedPacketCounterConfigGet: %d", dev);
    }

    profileId=8;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E |
                                           UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E | UTF_AC3X_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTmGlueAgingAndDelayAgingProfileThresholdsGet(dev,profileId,&thresholds);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTmGlueAgingAndDelayAgingProfileThresholdsGet(dev,profileId,&thresholds);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTmGlueAgingAndDelayQueueAgingProfileIdSet
(
    IN GT_U8        devNum,
    IN GT_U32       queueId,
    IN GT_U32       profileId
*/
UTF_TEST_CASE_MAC(cpssDxChTmGlueAgingAndDelayQueueAgingProfileIdSet)
{
/*
    ITERATE_DEVICES (Bobcat2)
    1.1. Call with profileId[15],queueId[16383]
    Expected: GT_OK
    1.2. Call cpssDxChTmGlueAgingAndDelayQueueAgingProfileIdGet.
    Expected: GT_OK and the same value as was set.
    1.3. Call with profileId[16],queueId[1222]
    Expected: GT_OUT_OF_RANGE
    1.4. Call with profileId[5],queueId[16384]
    Expected: GT_OUT_OF_RANGE

*/
    GT_STATUS    st             = GT_OK;
    GT_U8        dev            = 0;
    GT_U32       queueId        = 0;
    GT_U32       profileId      = 0;
    GT_U32       profileIdRet   = 0;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E |
                                           UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E | UTF_AC3X_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with profileId[15],queueId[16383]
            Expected: GT_OK
        */

        queueId = 16383;
        profileId = 15;


        st = cpssDxChTmGlueAgingAndDelayQueueAgingProfileIdSet(dev,queueId,profileId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call cpssDxChTmGlueAgingAndDelayQueueAgingProfileIdGet.
                Expected: GT_OK and the same value as was set.
        */
        st = cpssDxChTmGlueAgingAndDelayQueueAgingProfileIdGet(dev,queueId,&profileIdRet);

        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        if (GT_OK == st)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(profileId, profileIdRet,
                                         "got another profileIdRet: %d", profileIdRet);
        }

        /*
            1.3. Call with profileId[16],queueId[1222]
            Expected: GT_OUT_OF_RANGE
        */

        queueId = 1222;
        profileId = 16;


        st = cpssDxChTmGlueAgingAndDelayQueueAgingProfileIdSet(dev,queueId,profileId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, dev);

          /*
            1.3. Call with profileId[5],queueId[16384]
            Expected: GT_OUT_OF_RANGE
        */

        queueId = 16384;
        profileId = 15;

        st = cpssDxChTmGlueAgingAndDelayQueueAgingProfileIdSet(dev,queueId,profileId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, dev);

    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    queueId = 5;
    profileId = 2;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E |
                                           UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E | UTF_AC3X_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTmGlueAgingAndDelayQueueAgingProfileIdSet(dev,queueId,profileId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTmGlueAgingAndDelayQueueAgingProfileIdSet(dev,queueId,profileId);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTmGlueAgingAndDelayQueueAgingProfileIdGet
(
    IN  GT_U8        devNum,
    IN  GT_U32       queueId,
    OUT GT_U32       *profileIdPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChTmGlueAgingAndDelayQueueAgingProfileIdGet)
{
/*
    ITERATE_DEVICES (Bobcat2)
    1.1. Call with queueId[16383] and non-null profileIdPtr
    Expected: GT_OK.
    1.2. Call with out of range profileIdPtr[NULL] and other valid params.
    Expected: NON GT_OK.
    1.3. Call with out of range queueId[16384] and other valid params.
    Expected: NON GT_OK.
*/
    GT_STATUS    st             = GT_OK;
    GT_U8        dev            = 0;
    GT_U32       queueId        = 0;
    GT_U32       profileId      = 0;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E |
                                           UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E | UTF_AC3X_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with queueId[16383] and non-null profileIdPtr
            Expected: GT_OK.
        */
        queueId = 16383;
        st = cpssDxChTmGlueAgingAndDelayQueueAgingProfileIdGet(dev,queueId,&profileId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with out of range profileIdPtr[NULL] and other valid params.
            Expected: NON GT_OK.
        */
        st = cpssDxChTmGlueAgingAndDelayQueueAgingProfileIdGet(dev,queueId,NULL);
        UTF_VERIFY_NOT_EQUAL1_STRING_MAC(GT_OK, st,
                                         "cpssDxChTmGlueAgingAndDelayQueueAgingProfileIdGet: %d", dev);

        /*
            1.3. Call with out of range queueId[16384] and other valid params.
            Expected: NON GT_OK.
        */
        queueId=16384;
        st = cpssDxChTmGlueAgingAndDelayQueueAgingProfileIdGet(dev,queueId,&profileId);
        UTF_VERIFY_NOT_EQUAL1_STRING_MAC(GT_OK, st,
                                         "cpssDxChTmGlueAgingAndDelayQueueAgingProfileIdGet: %d", dev);
    }

    queueId=10;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E |
                                           UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E | UTF_AC3X_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTmGlueAgingAndDelayQueueAgingProfileIdGet(dev,queueId,&profileId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTmGlueAgingAndDelayQueueAgingProfileIdGet(dev,queueId,&profileId);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTmGlueAgingAndDelayCounterQueueIdSet
(
    IN GT_U8                    devNum,
    IN GT_U32                   statisticalAgingSet,
    IN GT_U32                   queueId
*/
UTF_TEST_CASE_MAC(cpssDxChTmGlueAgingAndDelayCounterQueueIdSet)
{
/*
    ITERATE_DEVICES (Bobcat2)
    1.1. Call with statisticalAgingSet[3],queueId[5]
    Expected: GT_OK
    1.2. Call cpssDxChTmGlueAgingAndDelayStatisticalAgingQueueIdGet.
    Expected: GT_OK and the same value as was set.
    1.3. Call with statisticalAgingSet[99],queueId[16383]
    Expected: GT_OK
    1.4. Call cpssDxChTmGlueAgingAndDelayStatisticalAgingQueueIdGet.
    Expected: GT_OK and the same value as was set.
    1.5. Call with statisticalAgingSet[100],queueId[5]
    Expected: GT_BAD_PARAM
    1.6. Call statisticalAgingSet[3],queueId[16384]
    Expected: GT_OUT_OF_RANGE

*/
    GT_STATUS    st                  = GT_OK;
    GT_U8        dev                 = 0;
    GT_U32       statisticalAgingSet = 0;
    GT_U32       queueId             = 0;
    GT_U32       queueIdRet          = 0;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E |
                                           UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E | UTF_AC3X_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with statisticalAgingSet[3],queueId[5]
            Expected: GT_OK
        */

        statisticalAgingSet = 3;
        queueId             = 5;


        st = cpssDxChTmGlueAgingAndDelayCounterQueueIdSet(dev,statisticalAgingSet,queueId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call cpssDxChTmGlueAgingAndDelayStatisticalAgingQueueIdGet.
                Expected: GT_OK and the same value as was set.
        */
        st = cpssDxChTmGlueAgingAndDelayCounterQueueIdGet(dev,statisticalAgingSet,&queueIdRet);

        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        if (GT_OK == st)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(queueId, queueIdRet,
                                         "got another queueIdRet: %d", queueIdRet);
        }

        /*
            1.3. Call with statisticalAgingSet[99],queueId[16383]
            Expected: GT_OK
        */

        statisticalAgingSet = 99;
        queueId             = 16383;


        st = cpssDxChTmGlueAgingAndDelayCounterQueueIdSet(dev,statisticalAgingSet,queueId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.4. Call cpssDxChTmGlueAgingAndDelayStatisticalAgingQueueIdGet.
                Expected: GT_OK and the same value as was set.
        */
        st = cpssDxChTmGlueAgingAndDelayCounterQueueIdGet(dev,statisticalAgingSet,&queueIdRet);

        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        if (GT_OK == st)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(queueId, queueIdRet,
                                         "got another queueIdRet: %d", queueIdRet);
        }

        /*
            1.4. Call with statisticalAgingSet[100],queueId[5]
            Expected: GT_OUT_OF_RANGE
        */

        statisticalAgingSet = 100;
        queueId             = 5;

        st = cpssDxChTmGlueAgingAndDelayCounterQueueIdSet(dev,statisticalAgingSet,queueId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

         /*
            1.4. Call with statisticalAgingSet[3],queueId[16384]
            Expected: GT_OUT_OF_RANGE
        */

        statisticalAgingSet = 3;
        queueId             = 16384;

        st = cpssDxChTmGlueAgingAndDelayCounterQueueIdSet(dev,statisticalAgingSet,queueId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, dev);

    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    statisticalAgingSet = 3;
    queueId             = 5;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E |
                                           UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E | UTF_AC3X_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTmGlueAgingAndDelayCounterQueueIdSet(dev,statisticalAgingSet,queueId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTmGlueAgingAndDelayCounterQueueIdSet(dev,statisticalAgingSet,queueId);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTmGlueAgingAndDelayCounterQueueIdGet
(
    IN  GT_U8                    devNum,
    IN  GT_U32                   counterSetIndex,
    OUT GT_U32                   *queueIdPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChTmGlueAgingAndDelayCounterQueueIdGet)
{
/*
    ITERATE_DEVICES (Bobcat2)
    1.1. Call with counterSetIndex[8] and non-null queueIdPtr
    Expected: GT_OK.
    1.2. Call with out of range queueIdPtr[NULL] and other valid params.
    Expected: NON GT_OK.
    1.3. Call with out of range counterSetIndex[100] and other valid params.
    Expected: NON GT_OK.
*/
    GT_STATUS    st                  = GT_OK;
    GT_U8        dev                 = 0;
    GT_U32       counterSetIndex     = 0;
    GT_U32       queueId             = 0;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E |
                                           UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E | UTF_AC3X_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with queueId[16383] and non-null queueIdPtr
            Expected: GT_OK.
        */
        counterSetIndex = 8;

        st = cpssDxChTmGlueAgingAndDelayCounterQueueIdGet(dev,counterSetIndex,&queueId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with out of range queueIdPtr[NULL] and other valid params.
            Expected: NON GT_OK.
        */
        st = cpssDxChTmGlueAgingAndDelayCounterQueueIdGet(dev,counterSetIndex,NULL);
        UTF_VERIFY_NOT_EQUAL1_STRING_MAC(GT_OK, st,
                                         "cpssDxChTmGlueAgingAndDelayCounterQueueIdGet: %d", dev);

        /*
            1.3. Call with out of range counterSetIndex[100] and other valid params.
            Expected: NON GT_OK.
        */
        counterSetIndex=100;

        st = cpssDxChTmGlueAgingAndDelayCounterQueueIdGet(dev,counterSetIndex,&queueId);
        UTF_VERIFY_NOT_EQUAL1_STRING_MAC(GT_OK, st,
                                         "cpssDxChTmGlueAgingAndDelayCounterQueueIdGet: %d", dev);
    }

    counterSetIndex=10;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E |
                                           UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E | UTF_AC3X_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTmGlueAgingAndDelayCounterQueueIdGet(dev,counterSetIndex,&queueId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTmGlueAgingAndDelayCounterQueueIdGet(dev,counterSetIndex,&queueId);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTmGlueAgingAndDelayCountersGet
(
    IN  GT_U8                                       devNum,
    IN  GT_U32                                      counterSetIndex,
    OUT CPSS_DXCH_TM_AGING_AND_DELAY_STATISTICS_STC *agingCountersPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChTmGlueAgingAndDelayCountersGet)
{
/*
    ITERATE_DEVICES (Bobcat2)
    1.1. Call with counterSetIndex[18] and non-null agingCountersPtr
    Expected: GT_OK.
    1.2. Call with out of range agingCountersPtr and other valid params.
    Expected: GT_BAD_PTR.
    1.3. Call with counterSetIndex[100] and non-null agingCountersPtr
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS       st          = GT_OK;
    GT_U8           dev         = 0;
    GT_U32                                      counterSetIndex;
    CPSS_DXCH_TM_AGING_AND_DELAY_STATISTICS_STC agingCounters;


    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E |
                                           UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E | UTF_AC3X_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with counterSetIndex[18] and non-null agingCountersPtr
            Expected: GT_OK.
        */
        counterSetIndex=18;

        st = cpssDxChTmGlueAgingAndDelayCountersGet(dev,
                                                  counterSetIndex,
                                                  &agingCounters);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with out of range agingCountersPtr and other valid params.
            Expected: GT_BAD_PTR.
        */

        st = cpssDxChTmGlueAgingAndDelayCountersGet(dev,
                                                  counterSetIndex,
                                                  NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

         /*
            1.3. Call with counterSetIndex[100] and non-null agingCountersPtr
            Expected: GT_BAD_PARAM.
        */
        counterSetIndex=100;

        st = cpssDxChTmGlueAgingAndDelayCountersGet(dev,
                                                  counterSetIndex,
                                                  &agingCounters);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    counterSetIndex=20;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E |
                                           UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E | UTF_AC3X_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTmGlueAgingAndDelayCountersGet(dev,
                                                  counterSetIndex,
                                                  &agingCounters);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChTmGlueAgingAndDelayCountersGet(dev,
                                              counterSetIndex,
                                              &agingCounters);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTmGlueAgingAndDelayPerPortAgedPacketCounterGet
(
    IN  GT_U8            devNum,
    IN  GT_U32           tmPort,
    OUT GT_U32           *agedOutPacketCounterPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChTmGlueAgingAndDelayPerPortAgedPacketCounterGet)
{
/*
    ITERATE_DEVICES (Bobcat2)
    1.1. Call with tmPort[18] and non-null agedOutPacketCounterPtr
    Expected: GT_OK.
    1.2. Call with out of range agedOutPacketCounterPtr and other valid params.
    Expected: GT_BAD_PTR.
    1.3. Call with tmPort[192] and non-null agedOutPacketCounterPtr
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS       st          = GT_OK;
    GT_U8           dev         = 0;
    GT_U32          tmPort;
    GT_U32          agedOutPacketCounter;


    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E |
                                           UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E | UTF_AC3X_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with tmPort[18] and non-null agedOutPacketCounterPtr
            Expected: GT_OK.
        */
        tmPort=18;

        st = cpssDxChTmGlueAgingAndDelayPerPortAgedPacketCounterGet(dev,
                                                                       tmPort,
                                                                       &agedOutPacketCounter);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with out of range agedOutPacketCounterPtr and other valid params.
            Expected: GT_BAD_PTR.
        */

        st = cpssDxChTmGlueAgingAndDelayPerPortAgedPacketCounterGet(dev,
                                                                       tmPort,
                                                                       NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

         /*
            1.3. Call with tmPort[192] and non-null agedOutPacketCounterPtr
            Expected: GT_BAD_PARAM.
        */
        tmPort=192;

        st = cpssDxChTmGlueAgingAndDelayPerPortAgedPacketCounterGet(dev,
                                                                       tmPort,
                                                                       &agedOutPacketCounter);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    tmPort=20;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E |
                                           UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E | UTF_AC3X_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTmGlueAgingAndDelayPerPortAgedPacketCounterGet(dev,
                                                                       tmPort,
                                                                       &agedOutPacketCounter);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChTmGlueAgingAndDelayPerPortAgedPacketCounterGet(dev,
                                                                   tmPort,
                                                                   &agedOutPacketCounter);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
 * Configuration of cpssDxChTmGlueAgingAndDelay suit
 */

UTF_SUIT_BEGIN_TESTS_MAC(cpssDxChTmGlueAgingAndDelay)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTmGlueAgingAndDelayDropAgedPacketEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTmGlueAgingAndDelayDropAgedPacketEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTmGlueAgingAndDelayAgedPacketsCountingEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTmGlueAgingAndDelayAgedPacketsCountingEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTmGlueGeneralPerPortDropOutgoingPacketEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTmGlueGeneralPerPortDropOutgoingPacketEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTmGlueAgingAndDelayAgedPacketCounterConfigSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTmGlueAgingAndDelayAgedPacketCounterConfigGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTmGlueAgingAndDelayAgedPacketCounterGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTmGlueAgingAndDelayTimerResolutionSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTmGlueAgingAndDelayTimerResolutionGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTmGlueAgingAndDelayTimerGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTmGlueAgingAndDelayAgingProfileThresholdsSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTmGlueAgingAndDelayAgingProfileThresholdsGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTmGlueAgingAndDelayQueueAgingProfileIdSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTmGlueAgingAndDelayQueueAgingProfileIdGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTmGlueAgingAndDelayCounterQueueIdSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTmGlueAgingAndDelayCounterQueueIdGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTmGlueAgingAndDelayCountersGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTmGlueAgingAndDelayPerPortAgedPacketCounterGet)

UTF_SUIT_END_TESTS_MAC(cpssDxChTmGlueAgingAndDelay)
