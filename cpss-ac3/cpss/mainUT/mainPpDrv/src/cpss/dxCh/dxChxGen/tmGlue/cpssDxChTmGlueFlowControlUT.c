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
* @file cpssDxChTmGlueFlowControlUT.c
*
* @brief Traffic Manager Glue - Flow Control unit tests.
*
* @version   3
********************************************************************************
*/

/* the define of UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC must come from C files that
   already fixed the types of ports from GT_U8 !

   NOTE: must come before ANY include to H files !!!!
*/
#define UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC

#include <cpss/dxCh/dxChxGen/cpssDxChTypes.h>
#include <cpss/dxCh/dxChxGen/tmGlue/cpssDxChTmGlueFlowControl.h>
#include <cpss/dxCh/dxChxGen/tmGlue/cpssDxChTmGluePfc.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <common/tgfCommon.h>

/* Invalid xoffThreshold */
#define PORT_PFC_INVALID_XOFFTHRESHOLD_CNS  0x3FF+1

/* Invalid xonThreshold */
#define PORT_PFC_INVALID_XONTHRESHOLD_CNS  0x3FF+1


/* Invalid tmPortInd */
#define PORT_PFC_INVALID_TM_PORT_IND_CNS        192

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTmGlueFlowControlEnableGet
(
    IN GT_U8                    devNum,
    IN GT_BOOL                  *egressEnable,
    IN GT_BOOL                  *ingressEnable
)
*/
UTF_TEST_CASE_MAC(cpssDxChTmGlueFlowControlEnableSet)
{
/*
    ITERATE_DEVICES (Bobcat2)
    1.1. Call with enable[GT_TRUE, GT_FALSE].
    Expected: GT_OK.
    1.2. Call cpssDxChTmGlueFlowControlEnableGet.
    Expected: GT_OK and the same values as was set.
*/
    GT_STATUS                               st          = GT_OK;
    GT_U8                                   dev         = 0;
    GT_BOOL                                 egressEnable    = GT_TRUE;
    GT_BOOL                                 egressEnableRet = GT_TRUE;
    GT_BOOL                                 ingressEnable   = GT_TRUE;
    GT_BOOL                                 ingressEnableRet = GT_TRUE;


    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E |
                                           UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E | UTF_AC3X_E );

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with enable[GT_FALSE, GT_TRUE].
            Expected: GT_OK.
        */

        /* call with egressEnable, ingressEnable[GT_FALSE] */

        st = cpssDxChTmGlueFlowControlEnableSet(dev, egressEnable, ingressEnable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call cpssDxChTmGlueFlowControlEnableSet(dev, egressEnable, ingressEnable).
                Expected: GT_OK and the same values as was set.
        */
        st = cpssDxChTmGlueFlowControlEnableGet(dev, &egressEnableRet, &ingressEnableRet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        if (GT_OK == st)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(egressEnable, egressEnableRet,
                                         "got another egress enable: %d", egressEnableRet);
        }

        /* for bobk verify ingressEnable */
        if (PRV_CPSS_SIP_5_15_CHECK_MAC(dev)) /* IS_BOBK_DEV_MAC(dev) */
        {
            if (GT_OK == st)
            {
                UTF_VERIFY_EQUAL1_STRING_MAC(ingressEnable, ingressEnableRet,
                                             "got another ingress enable: %d", ingressEnableRet);
            }
        }

        /*
            1.3. Verify get correct value pfcResponseMode CPSS_DXCH_TM_GLUE_PFC_RESPONSE_MODE_TM_E
            Expected: GT_OK and the same values as was set.
        */
        st = cpssDxChTmGluePfcResponseModeSet(dev, 0, CPSS_DXCH_TM_GLUE_PFC_RESPONSE_MODE_TM_E);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssDxChTmGlueFlowControlEnableGet(dev, &egressEnableRet, &ingressEnableRet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        if (GT_OK == st)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(egressEnable, egressEnableRet,
                                         "got another egress enable: %d", egressEnableRet);
        }

        /* for bobk verify ingressEnable */
        if (PRV_CPSS_SIP_5_15_CHECK_MAC(dev)) /* IS_BOBK_DEV_MAC(dev) */
        {
            if (GT_OK == st)
            {
                UTF_VERIFY_EQUAL1_STRING_MAC(ingressEnable, ingressEnableRet,
                                             "got another ingress enable: %d", ingressEnableRet);
            }
        }

        /* call with egressEnable, ingressEnable[GT_TRUE] */
        egressEnable = GT_FALSE;
        ingressEnable = GT_FALSE;

        st = cpssDxChTmGlueFlowControlEnableSet(dev, egressEnable, ingressEnable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.4. Call cpssDxChTmGlueFlowControlEnableGet.
                Expected: GT_OK and the same values as was set.
        */
        st = cpssDxChTmGlueFlowControlEnableGet(dev, &egressEnableRet, &ingressEnableRet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        if (GT_OK == st)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(egressEnable, egressEnableRet,
                                         "got another egress enable: %d", egressEnableRet);
        }

        /* for bobk verify ingressEnable */
        if (PRV_CPSS_SIP_5_15_CHECK_MAC(dev)) /* IS_BOBK_DEV_MAC(dev) */
        {
            if (GT_OK == st)
            {
                UTF_VERIFY_EQUAL1_STRING_MAC(ingressEnable, ingressEnableRet,
                                             "got another ingress enable: %d", ingressEnableRet);
            }
        }

        /*
            1.5. Verify get correct value pfcResponseMode CPSS_DXCH_TM_GLUE_PFC_RESPONSE_MODE_TXQ_E
            Expected: GT_OK and the same values as was set.
        */
        st = cpssDxChTmGluePfcResponseModeSet(dev, 0, CPSS_DXCH_TM_GLUE_PFC_RESPONSE_MODE_TXQ_E);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssDxChTmGlueFlowControlEnableGet(dev, &egressEnableRet, &ingressEnableRet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        if (GT_OK == st)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(egressEnable, egressEnableRet,
                                         "got another egress enable: %d", egressEnableRet);
        }

        /* for bobk verify ingressEnable */
        if (PRV_CPSS_SIP_5_15_CHECK_MAC(dev)) /* IS_BOBK_DEV_MAC(dev) */
        {
            if (GT_OK == st)
            {
                UTF_VERIFY_EQUAL1_STRING_MAC(ingressEnable, ingressEnableRet,
                                             "got another ingress enable: %d", ingressEnableRet);
            }
        }
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E |
                                           UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E | UTF_AC3X_E );

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTmGlueFlowControlEnableSet(dev, egressEnable, ingressEnable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTmGlueFlowControlEnableSet(dev, egressEnable, ingressEnable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTmGlueFlowControlEnableGet
(
    IN GT_U8                    devNum,
    OUT GT_BOOL                *egressEnablePtr,
    OUT GT_BOOL                *ingressEnablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChTmGlueFlowControlEnableGet)
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
    GT_BOOL                                 egressEnable    = GT_FALSE;
    GT_BOOL                                 ingressEnable   = GT_FALSE;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E |
                                           UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E | UTF_AC3X_E );

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with non-null enablePtr.
            Expected: GT_OK.
        */
        st = cpssDxChTmGlueFlowControlEnableGet(dev, &egressEnable, &ingressEnable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
        1.2. Call with out of range enablePtr[NULL] and other valid params.
        Expected: NON GT_OK.
        */
        st = cpssDxChTmGlueFlowControlEnableGet(dev, NULL, &ingressEnable);
        UTF_VERIFY_NOT_EQUAL1_STRING_MAC(GT_OK, st,
                                         "cpssDxChTmGlueFlowControlEnableGet: %d", dev);

        /* for bobk verify ingressEnable */
        if (PRV_CPSS_SIP_5_15_CHECK_MAC(dev)) /* IS_BOBK_DEV_MAC(dev) */
        {
            st = cpssDxChTmGlueFlowControlEnableGet(dev, &egressEnable, NULL);
            UTF_VERIFY_NOT_EQUAL1_STRING_MAC(GT_OK, st,
                                             "cpssDxChTmGlueFlowControlEnableGet: %d", dev);
        }
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E |
                                           UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E | UTF_AC3X_E );

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTmGlueFlowControlEnableGet(dev, &egressEnable, &ingressEnable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTmGlueFlowControlEnableGet(dev, &egressEnable, &ingressEnable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTmGlueFlowControlEgressCounterSet
(
    IN  GT_U8     devNum,
    IN  GT_U32    tmPortInd,
    IN  GT_U32    value
)
*/
UTF_TEST_CASE_MAC(cpssDxChTmGlueFlowControlEgressCounterSet)
{
/*
    ITERATE_DEVICES (Bobcat2)
    1.1. Call function with tmPortInd [0, 20, 100, 191],
                            value [0..1023]
    Expected: GT_OK.
    1.2. Call cpssDxChTmGlueFlowControlEgressCounterGet with the same parameters.
    Expected: GT_OK and the same value.
    1.3.  Call  with bad parameter tmPortInd [200]
    Expected: GT_BAD_PARAM.
    1.4.  Call  with wrong(out-of-range) value [1024]
    Expected: GT_OUT_OF_RANGE.
*/
    GT_STATUS st    = GT_OK;

    GT_U8     dev;
    GT_U32    tmPortInd = 0;
    GT_U32    value = 0;
    GT_U32    valueGet;

  /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E |
                                           UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E | UTF_AC3X_E );

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with tmPortInd [0, 20, 100, 191],
                            value [0..1023]
            Expected: GT_OK.
            1.2. Call cpssDxChTmGlueFlowControlEgressCounterGet with the same parameters.
            Expected: GT_OK and the same values read.
        */
        /* Call function with tmPortInd [0], value [500] */
        tmPortInd = 0;
        value = 500;
        st = cpssDxChTmGlueFlowControlEgressCounterSet(dev, tmPortInd, value);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssDxChTmGlueFlowControlEgressCounterGet(dev, tmPortInd, &valueGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                          "cpssDxChTmGlueFlowControlEgressCounterGet: %d, %d", dev, valueGet);
        /* Verifying value */
        UTF_VERIFY_EQUAL1_STRING_MAC(value, valueGet, "get another value than was set: %d", dev);

        /* Call function with tmPortInd [20], value [1000] */
        tmPortInd = 20;
        value = 1000;
        st = cpssDxChTmGlueFlowControlEgressCounterSet(dev, tmPortInd, value);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssDxChTmGlueFlowControlEgressCounterGet(dev, tmPortInd, &valueGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                          "cpssDxChTmGlueFlowControlEgressCounterGet: %d, %d", dev, valueGet);
        /* Verifying value */
        UTF_VERIFY_EQUAL1_STRING_MAC(value, valueGet, "get another value than was set: %d", dev);


        /* Call function with tmPortInd [100], value [10] */
        tmPortInd = 100;
        value = 10;
        st = cpssDxChTmGlueFlowControlEgressCounterSet(dev, tmPortInd, value);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssDxChTmGlueFlowControlEgressCounterGet(dev, tmPortInd, &valueGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                          "cpssDxChTmGlueFlowControlEgressCounterGet: %d, %d", dev, valueGet);
        /* Verifying value */
        UTF_VERIFY_EQUAL1_STRING_MAC(value, valueGet, "get another value than was set: %d", dev);

        /* Call function with tmPortInd [191], value [200] */
        tmPortInd = 191;
        value = 200;
        st = cpssDxChTmGlueFlowControlEgressCounterSet(dev, tmPortInd, value);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssDxChTmGlueFlowControlEgressCounterGet(dev, tmPortInd, &valueGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                          "cpssDxChTmGlueFlowControlEgressCounterGet: %d, %d", dev, valueGet);
        /* Verifying value */
        UTF_VERIFY_EQUAL1_STRING_MAC(value, valueGet, "get another value than was set: %d", dev);

        /*
            1.3.  Call  with wrong tmPortInd [192]
            Expected: GT_BAD_PARAM.
        */

        tmPortInd = 192;
        value = 100;
        st = cpssDxChTmGlueFlowControlEgressCounterSet(dev, tmPortInd, value);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, tmPortInd, value);

        /*
            1.4.  Call  with wrong(out-of-range) value [1024]
            Expected: GT_OUT_OF_RANGE.
        */
        tmPortInd = 8;
        value = 1024;
        st = cpssDxChTmGlueFlowControlEgressCounterSet(dev, tmPortInd, value);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OUT_OF_RANGE, st, dev, tmPortInd, value);
    }

    tmPortInd = 0;
    value = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E |
                                           UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E | UTF_AC3X_E );

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTmGlueFlowControlEgressCounterSet(dev, tmPortInd, value);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTmGlueFlowControlEgressCounterSet(dev, tmPortInd, value);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTmGlueDropQueueProfileIdGet
(
    IN  GT_U8     devNum,
    IN  GT_U32    tmPortInd,
    OUT GT_U32   *valuePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChTmGlueFlowControlEgressCounterGet)
{
/*
    ITERATE_DEVICES (Bobcat2)
    1.1. Call function with tmPortInd [0, 20, 100, 191].
    Expected: GT_OK.
    1.2.  Call  with wrong tmPortInd [200]
    Expected: GT_BAD_PARAM
    1.3. Call with wrong valuePtr [NULL]
    Expected: GT_OUT_OF_RANGE.
*/

    GT_STATUS   st   = GT_OK;
    GT_U8       dev  = 0 ;

    GT_U32      tmPortInd = 0;
    GT_U32      value = 0;

   /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E |
                                           UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E | UTF_AC3X_E );

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*  1.1.  */
        tmPortInd = 0;
        st = cpssDxChTmGlueFlowControlEgressCounterGet(dev, tmPortInd, &value);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, tmPortInd);

        /*  1.1.  */
        tmPortInd = 20;
        st = cpssDxChTmGlueFlowControlEgressCounterGet(dev, tmPortInd, &value);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, tmPortInd);

        /*  1.1.  */
        tmPortInd = 100;
        st = cpssDxChTmGlueFlowControlEgressCounterGet(dev, tmPortInd, &value);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, tmPortInd);

        /*  1.1.  */
        tmPortInd = 191;
        st = cpssDxChTmGlueFlowControlEgressCounterGet(dev, tmPortInd, &value);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, tmPortInd);

        /*  1.2.  */
        tmPortInd = 200;
        st = cpssDxChTmGlueFlowControlEgressCounterGet(dev, tmPortInd, &value);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, tmPortInd);

        /*  1.3.  */
        tmPortInd = 0;
        st = cpssDxChTmGlueFlowControlEgressCounterGet(dev, tmPortInd, NULL);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, tmPortInd);
    }
    tmPortInd = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E |
                                           UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E | UTF_AC3X_E );

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTmGlueFlowControlEgressCounterGet(dev, tmPortInd, &value);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTmGlueFlowControlEgressCounterGet(dev, tmPortInd, &value);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTmGlueFlowControlEgressThresholdsSet
(
    IN GT_U8 devNum,
    IN GT_U32 tmPortInd,
    IN GT_U32 xOffThreshold,
    IN GT_U32 xOnThreshold
)
*/
UTF_TEST_CASE_MAC(cpssDxChTmGlueFlowControlEgressThresholdsSet)
{
/*
    ITERATE_DEVICES (Bobcat2)
    1.1. Call with tmPortInd [0 - 191],
                   xoffThreshold [0 - 0x7FF],
                   xonThreshold [0 - 0x7FF].
    Expected: GT_OK.
    1.2. Call cpssDxChTmGlueFlowControlEgressThresholdsGet with not-NULL pointers.
    Expected: GT_OK and the same values as was set.
    1.3. Call with wrong tmPortInd [PORT_PFC_INVALID_TM_PORT_IND_CNS].
    Expected: NOT GT_OK.
    1.4. Call with wrong xoffThreshold [PORT_PFC_INVALID_XOFFTHRESHOLD_CNS].
    Expected: NOT GT_OK.
    1.5. Call with wrong xonThreshold [PORT_PFC_INVALID_XONTHRESHOLD_CNS].
    Expected: NOT GT_OK.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;

    GT_U8   tmPortInd = 0;
    GT_U32  xoffThreshold = 0;
    GT_U32  xoffThresholdGet = 1;
    GT_U32  xonThreshold = 0;
    GT_U32  xonThresholdGet = 1;
    GT_U32  xoffStep;
    GT_U32  dropStep;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E | UTF_LION_E | UTF_LION2_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E | UTF_AC3X_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with tmPortInd [0 - 191], xoffThreshold [0 - 0x3FF], xonThreshold [0 - 0x3FF].
            Expected: GT_OK.
        */

        xoffStep = 300;
        dropStep = 500;

        for(tmPortInd = 0; tmPortInd < PORT_PFC_INVALID_TM_PORT_IND_CNS; tmPortInd++)
            for(xoffThreshold = 0;
                xoffThreshold < PORT_PFC_INVALID_XOFFTHRESHOLD_CNS;
                xoffThreshold += xoffStep)
                for(xonThreshold = 0;
                    xonThreshold < PORT_PFC_INVALID_XONTHRESHOLD_CNS;
                    xonThreshold += dropStep)
                {
                    st = cpssDxChTmGlueFlowControlEgressThresholdsSet(dev, tmPortInd,
                                                             xoffThreshold, xonThreshold);
                    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

                    if(GT_OK == st)
                    {
                        /*
                            1.2. Call cpssDxChTmGlueFlowControlEgressThresholdsGet with not-NULL pointers.
                            Expected: GT_OK and the same xoffThreshold and xonThreshold as was set.
                        */
                        st = cpssDxChTmGlueFlowControlEgressThresholdsGet(dev, tmPortInd,
                                                                 &xoffThresholdGet, &xonThresholdGet);
                        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                    "cpssDxChTmGlueFlowControlEgressThresholdsGet: %d", dev);
                        UTF_VERIFY_EQUAL1_STRING_MAC(xoffThreshold, xoffThresholdGet,
                                                     "got another xoffThreshold then was set: %d", dev);
                        UTF_VERIFY_EQUAL1_STRING_MAC(xonThreshold, xonThresholdGet,
                                                     "got another xonThreshold then was set: %d", dev);
                    }
                }

        /*
            1.3. Call with wrong tmPortInd [PORT_PFC_INVALID_TM_PORT_IND_CNS].
            Expected: NOT GT_OK.
        */
        tmPortInd = PORT_PFC_INVALID_TM_PORT_IND_CNS;

        st = cpssDxChTmGlueFlowControlEgressThresholdsSet(dev, tmPortInd, xoffThreshold, xonThreshold);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        tmPortInd = 0;

        /*
            1.4. Call with wrong xoffThreshold [PORT_PFC_INVALID_XOFFTHRESHOLD_CNS].
            Expected: NOT GT_OK.
        */
        xoffThreshold = PORT_PFC_INVALID_XOFFTHRESHOLD_CNS;

        st = cpssDxChTmGlueFlowControlEgressThresholdsSet(dev, tmPortInd, xoffThreshold, xonThreshold);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        xoffThreshold = 0;

        /*
            1.5. Call with wrong xonThreshold [PORT_PFC_INVALID_XONTHRESHOLD_MAC].
            Expected: NOT GT_OK.
        */
        xonThreshold = PORT_PFC_INVALID_XONTHRESHOLD_CNS;

        st = cpssDxChTmGlueFlowControlEgressThresholdsSet(dev, tmPortInd, xoffThreshold, xonThreshold);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        xonThreshold = 0;
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E | UTF_LION_E | UTF_LION2_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E | UTF_AC3X_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTmGlueFlowControlEgressThresholdsSet(dev, tmPortInd, xoffThreshold, xonThreshold);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTmGlueFlowControlEgressThresholdsSet(dev, tmPortInd, xoffThreshold, xonThreshold);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTmGlueFlowControlEgressThresholdsGet
(
    IN  GT_U8    devNum,
    IN  GT_U8    tmPortInd,
    OUT GT_U32   *xoffThresholdPtr,
    OUT GT_U32   *xonThresholdPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChTmGlueFlowControlEgressThresholdsGet)
{
/*
    ITERATE_DEVICES (Bobcat2)
    1.1. Call with tmPortInd [0 - 191] and  not-NULL pointers.
    Expected: GT_OK.
    1.2. Call with wrong tmPortInd [PORT_PFC_INVALID_TM_PORT_IND_CNS].
    Expected: NOT GT_OK.
    1.3. Call with wrong  xoffThresholdPtr [NULL].
    Expected: GT_BAD_PTR.
    1.4. Call with wrong  xonThresholdPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;

    GT_U8       tmPortInd = 0;
    GT_U32      xoffThreshold = 0;
    GT_U32      xonThreshold = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E | UTF_LION_E | UTF_LION2_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E | UTF_AC3X_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with not-NULL tmPortInd.
            Expected: GT_OK.
        */
        for(tmPortInd = 0; tmPortInd < PORT_PFC_INVALID_TM_PORT_IND_CNS; tmPortInd++)
        {
            st = cpssDxChTmGlueFlowControlEgressThresholdsGet(dev, tmPortInd, &xoffThreshold, &xonThreshold);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        }

        /*
            1.2. Call with wrong tmPortInd [PORT_PFC_INVALID_TCQUEUE_CNS].
            Expected: NOT GT_OK.
        */
        tmPortInd = PORT_PFC_INVALID_TM_PORT_IND_CNS;

        st = cpssDxChTmGlueFlowControlEgressThresholdsGet(dev, tmPortInd, &xoffThreshold, &xonThreshold);
        UTF_VERIFY_NOT_EQUAL1_STRING_MAC(GT_OK, st, "%d, tmPortInd = NULL", dev);

        tmPortInd = 0;

        /*
            1.3. Call with wrong xoffThresholdPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChTmGlueFlowControlEgressThresholdsGet(dev, tmPortInd, NULL, &xonThreshold);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, xoffThresholdPtr = NULL", dev);

        /*
            1.4. Call with wrong xonThresholdPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChTmGlueFlowControlEgressThresholdsGet(dev, tmPortInd, &xoffThreshold, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, xonThresholdPtr = NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E | UTF_LION_E | UTF_LION2_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E | UTF_AC3X_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTmGlueFlowControlEgressThresholdsGet(dev, tmPortInd, &xoffThreshold, &xonThreshold);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTmGlueFlowControlEgressThresholdsGet(dev, tmPortInd, &xoffThreshold, &xonThreshold);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*
GT_STATUS cpssDxChTmGlueFlowControlPortSpeedSet
(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM                portNum,
    IN CPSS_PORT_SPEED_ENT             speed
)
*/
UTF_TEST_CASE_MAC(cpssDxChTmGlueFlowControlPortSpeedSet)
{
/*
    ITERATE_DEVICES (Bobcat2)
    1.1. Call function with tmPortInd [0, 20, 63]
         if > B0 call also with 100, 191],

         speed values: CPSS_PORT_SPEED_100_E,
                       CPSS_PORT_SPEED_1000_E
                       CPSS_PORT_SPEED_10000_E
                       CPSS_PORT_SPEED_20000_E
                       CPSS_PORT_SPEED_40000_E
    Expected: GT_OK.
    1.3.  Call  with bad parameter tmPortInd [300]
    Expected: GT_BAD_PARAM.
    1.4.  Call  with wrong(out-of-range) value [1024]
    Expected: GT_OUT_OF_RANGE.
*/
    GT_STATUS               st = GT_OK;
    GT_U8                   dev;
    UTF_PHYSICAL_PORT_NUM   physicalPort = 0;
    GT_U32                  value = 0;

  /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E |
                                           UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E | UTF_AC3X_E );

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
             1.1. Call function with tmPortInd [0, 20, 63]
                 if > B0 call also with 100, 255],

                 speed values: CPSS_PORT_SPEED_100_E, CPSS_PORT_SPEED_1000_E, CPSS_PORT_SPEED_10000_E
                               CPSS_PORT_SPEED_20000_E, CPSS_PORT_SPEED_40000_E
            Expected: GT_OK.
        */

        /* 1.1. For all active devices go over all available physical ports < 64. */
        while (GT_OK == prvUtfNextPhyPortGet(&physicalPort, GT_TRUE))
        {
            if(physicalPort > 63 && !PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
            {
                break;
            }

            value = CPSS_PORT_SPEED_100_E;
            st = cpssDxChTmGlueFlowControlPortSpeedSet(dev, physicalPort, value);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            value = CPSS_PORT_SPEED_1000_E;
            st = cpssDxChTmGlueFlowControlPortSpeedSet(dev, physicalPort, value);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            value = CPSS_PORT_SPEED_10000_E;
            st = cpssDxChTmGlueFlowControlPortSpeedSet(dev, physicalPort, value);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            value = CPSS_PORT_SPEED_20000_E;
            st = cpssDxChTmGlueFlowControlPortSpeedSet(dev, physicalPort, value);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            value = CPSS_PORT_SPEED_40000_E;
            st = cpssDxChTmGlueFlowControlPortSpeedSet(dev, physicalPort, value);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        }

        physicalPort = 0;

        /* 1.2. For all active devices go over all non available physical ports. */
        while (GT_OK == prvUtfNextPhyPortGet(&physicalPort, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            value = CPSS_PORT_SPEED_1000_E;
            st = cpssDxChTmGlueFlowControlPortSpeedSet(dev, physicalPort, value);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, physicalPort);
        }

        /* 1.3. For active device check that function returns GT_OK for CPU port */
        physicalPort = CPSS_CPU_PORT_NUM_CNS;
        value = CPSS_PORT_SPEED_10000_E;
        st = cpssDxChTmGlueFlowControlPortSpeedSet(dev, physicalPort, value);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.4.  Call with wrong physicalPort
            Expected: GT_BAD_PARAM.
        */

        if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
        {
            physicalPort = 300;
        }
        else
        {
            physicalPort = 64;
        }

        value = CPSS_PORT_SPEED_40000_E;
        st = cpssDxChTmGlueFlowControlPortSpeedSet(dev, physicalPort, value);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, physicalPort, value);

        /*
            1.4.  Call  with wrong speed value [CPSS_PORT_SPEED_NA_E + 10]
            Expected: GT_BAD_PARAM
        */
        physicalPort = 8;
        value = CPSS_PORT_SPEED_NA_E + 10;
        st = cpssDxChTmGlueFlowControlPortSpeedSet(dev, physicalPort, value);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, physicalPort, value);
    }

    physicalPort = 0;
    value = CPSS_PORT_SPEED_1000_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E |
                                           UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E | UTF_AC3X_E );

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTmGlueFlowControlPortSpeedSet(dev, physicalPort, value);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTmGlueFlowControlPortSpeedSet(dev, physicalPort, value);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
 * Configuration of cpssDxChTmGlueFlowControl suit
 */

UTF_SUIT_BEGIN_TESTS_MAC(cpssDxChTmGlueFlowControl)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTmGlueFlowControlEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTmGlueFlowControlEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTmGlueFlowControlEgressCounterSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTmGlueFlowControlEgressCounterGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTmGlueFlowControlEgressThresholdsSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTmGlueFlowControlEgressThresholdsGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTmGlueFlowControlPortSpeedSet)
UTF_SUIT_END_TESTS_MAC(cpssDxChTmGlueFlowControl)
