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
* @file cpssDxChBrgEgrFltUT.c
*
* @brief Unit tests for cpssDxChBrgEgrFlt, that provides
* egress filtering facility DxCh cpss implementation.
*
* @version   31
********************************************************************************
*/

/* the define of UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC must come from C files that
   already fixed the types of ports from GT_U8 !

   NOTE: must come before ANY include to H files !!!!
*/
#define UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC

/* includes */
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgEgrFlt.h>
/* get the device info and common info */
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>

/* defines */

/* Default valid value for port id */
#define BRG_EGR_FLT_VALID_PHY_PORT_CNS  0

/* macro to check that device supports 'range' of physical ports rather then only 'existing physical ports' */
#define IS_E_ARCH_AND_PHYSICAL_PORT_IN_RANGE_MAC(_dev,_port)   \
    (UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(_dev) && ((_port) <= (UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(_dev)-1)))

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgPortEgrFltUnkEnable
(
    IN GT_U8        dev,
    IN GT_U8        port,
    IN GT_BOOL      enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgPortEgrFltUnkEnable)
{
/*
    ITERATE_DEVICES_PHY_CPU_PORT (DxChx)
    1.1.1. Call function with enable [GT_FALSE and GT_TRUE].
    Expected: GT_OK for Cheetah devices and GT_BAD_PARAM for others.
    1.1.2. Call cpssDxChBrgPortEgrFltUnkEnableGet.
    Expected: GT_OK and the same value.
*/
    GT_STATUS   st        = GT_OK;
    GT_U8       dev;
    GT_PORT_NUM  port      = BRG_EGR_FLT_VALID_PHY_PORT_CNS;
    GT_BOOL     enable    = GT_TRUE;
    GT_BOOL     enableGet = GT_FALSE;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            /* 1.1.1. Call function for with enable = GT_FALSE and GT_TRUE.     */
            /* Expected: GT_OK for Cheetah devices and GT_BAD_PARAM for others. */

            /* Call function with [enable==GT_FALSE] */
            enable    = GT_FALSE;

            st = cpssDxChBrgPortEgrFltUnkEnable(dev, port, enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);

            /*1.1.2. Call cpssDxChBrgPortEgrFltUnkEnableGet. */
            /*Expected: GT_OK and the same value.*/
            enableGet    = GT_TRUE;

            st = cpssDxChBrgPortEgrFltUnkEnableGet(dev, port, &enableGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                         "cpssDxChBrgPortEgrFltUnkEnableGet: %d, %d", dev, port);
            UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                     "get another enable state than was set: %d, %d", dev, port);

            /* Call function with [enable==GT_TRUE] */
            enable    = GT_TRUE;

            st = cpssDxChBrgPortEgrFltUnkEnable(dev, port, enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);

            /*1.1.2. Call cpssDxChBrgPortEgrFltUnkEnableGet. */
            /*Expected: GT_OK and the same value.*/
            enableGet = GT_FALSE;

            st = cpssDxChBrgPortEgrFltUnkEnableGet(dev, port, &enableGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                         "cpssDxChBrgPortEgrFltUnkEnableGet: %d, %d", dev, port);
            UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                     "get another enable state than was set: %d, %d", dev, port);
        }

        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while (GT_OK == prvUtfNextPhyPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port    */
            /* enable == GT_TRUE                                */
            st = cpssDxChBrgPortEgrFltUnkEnable(dev, port, enable);

            if(IS_E_ARCH_AND_PHYSICAL_PORT_IN_RANGE_MAC(dev,port))
            {
                /* the range is '256 physical ports' */
                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
            }
            else
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        /* enable == GT_TRUE                                               */
        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChBrgPortEgrFltUnkEnable(dev, port, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                     */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgPortEgrFltUnkEnable(dev, port, enable);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    port = BRG_EGR_FLT_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgPortEgrFltUnkEnable(dev, port, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    /* port == 0            */
    /* enable == GT_TRUE    */

    st = cpssDxChBrgPortEgrFltUnkEnable(dev, port, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgPortEgrFltUnkEnableGet
(
    IN  GT_U8   dev,
    IN  GT_U8   port,
    OUT GT_BOOL *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgPortEgrFltUnkEnableGet)
{
/*
    ITERATE_DEVICES_PHY_CPU_PORT (DxChx)
    1.1.1. Call function with correct parameters.
    1.1.2. Call function with wrong enablePtr[NULL].
    Expected: GT_OK.
*/
    GT_STATUS   st     = GT_OK;
    GT_U8       dev;
    GT_PORT_NUM  port   = BRG_EGR_FLT_VALID_PHY_PORT_CNS;
    GT_BOOL     enable;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            /* 1.1.1. Call function for with enable.  */
            /* Expected: GT_OK. */
            st = cpssDxChBrgPortEgrFltUnkEnableGet(dev, port, &enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);

            /* 1.1.2. For active device check that function returns GT_BAD_PTR */
            st = cpssDxChBrgPortEgrFltUnkEnableGet(dev, port, NULL);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, port);
        }
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while (GT_OK == prvUtfNextPhyPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChBrgPortEgrFltUnkEnableGet(dev, port, &enable);
            if(IS_E_ARCH_AND_PHYSICAL_PORT_IN_RANGE_MAC(dev,port))
            {
                /* the range is '256 physical ports' */
                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
            }
            else
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChBrgPortEgrFltUnkEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                     */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgPortEgrFltUnkEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    port = BRG_EGR_FLT_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgPortEgrFltUnkEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgPortEgrFltUnkEnableGet(dev, port, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgPortEgrFltUregMcastEnable
(
    IN GT_U8        dev,
    IN GT_U8        port,
    IN GT_BOOL      enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgPortEgrFltUregMcastEnable)
{
/*
    ITERATE_DEVICES_PHY_CPU_PORT (DxChx)
    1.1.1. Call function with enable [GT_FALSE and GT_TRUE].
    Expected: GT_OK for Cheetah devices and GT_BAD_PARAM for others.
*/
    GT_STATUS   st     = GT_OK;
    GT_U8       dev;
    GT_PORT_NUM       port   = BRG_EGR_FLT_VALID_PHY_PORT_CNS;
    GT_BOOL     enable = GT_TRUE;
    GT_BOOL     enableGet;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            /* 1.1.1. Call function for with enable = GT_TRUE and GT_FALSE.     */
            /* Expected: GT_OK for Cheetah devices and GT_BAD_PARAM for others. */

            /* Call function with [enable==GT_FALSE] */
            enable = GT_FALSE;

            st = cpssDxChBrgPortEgrFltUregMcastEnable(dev, port, enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);

            enableGet = GT_TRUE;

            st = cpssDxChBrgPortEgrFltUregMcastEnableGet(dev, port, &enableGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                         "cpssDxChBrgPortEgrFltUnkEnableGet: %d, %d", dev, port);
            UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                     "get another enable state than was set: %d, %d", dev, port);

            /* Call function with [enable==GT_TRUE] */
            enable = GT_TRUE;

            st = cpssDxChBrgPortEgrFltUregMcastEnable(dev, port, enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);

            enableGet = GT_FALSE;

            st = cpssDxChBrgPortEgrFltUregMcastEnableGet(dev, port, &enableGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                         "cpssDxChBrgPortEgrFltUnkEnableGet: %d, %d", dev, port);
            UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                     "get another enable state than was set: %d, %d", dev, port);
        }

        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while (GT_OK == prvUtfNextPhyPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port    */
            /* enable == GT_TRUE                                */
            st = cpssDxChBrgPortEgrFltUregMcastEnable(dev, port, enable);
            if(IS_E_ARCH_AND_PHYSICAL_PORT_IN_RANGE_MAC(dev,port))
            {
                /* the range is '256 physical ports' */
                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
            }
            else
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChBrgPortEgrFltUregMcastEnable(dev, port, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                     */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgPortEgrFltUregMcastEnable(dev, port, enable);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    port = BRG_EGR_FLT_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgPortEgrFltUregMcastEnable(dev, port, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    /* enable == GT_TRUE */
    /* port == 0         */

    st = cpssDxChBrgPortEgrFltUregMcastEnable(dev, port, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgPortEgrFltUregMcastEnableGet
(
    IN  GT_U8   dev,
    IN  GT_U8   port,
    OUT GT_BOOL *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgPortEgrFltUregMcastEnableGet)
{
/*
    ITERATE_DEVICES_PHY_CPU_PORT (DxChx)
    1.1.1. Call function with correct parameters.
    Expected: GT_OK.
    1.1.1. Call function with wrong enablePtr[NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st     = GT_OK;
    GT_U8       dev;
    GT_PORT_NUM       port   = BRG_EGR_FLT_VALID_PHY_PORT_CNS;
    GT_BOOL     enable;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            /* 1.1.1. Call function for with enable.  */
            /* Expected: GT_OK. */
            st = cpssDxChBrgPortEgrFltUregMcastEnableGet(dev, port, &enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);

            /* 1.1.2. For active device check that function returns GT_BAD_PTR */
            st = cpssDxChBrgPortEgrFltUregMcastEnableGet(dev, port, NULL);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, port);
        }
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while (GT_OK == prvUtfNextPhyPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChBrgPortEgrFltUregMcastEnableGet(dev, port, &enable);
            if(IS_E_ARCH_AND_PHYSICAL_PORT_IN_RANGE_MAC(dev,port))
            {
                /* the range is '256 physical ports' */
                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
            }
            else
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChBrgPortEgrFltUregMcastEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                     */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgPortEgrFltUregMcastEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    port = BRG_EGR_FLT_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgPortEgrFltUregMcastEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgPortEgrFltUregMcastEnableGet(dev, port, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgVlanEgressFilteringEnable
(
    IN GT_U8        dev,
    IN GT_BOOL      enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanEgressFilteringEnable)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call function with enable [GT_FALSE and GT_TRUE].
    Expected: GT_OK for Cheetah devices and GT_BAD_PARAM for others.
*/
    GT_STATUS   st     = GT_OK;
    GT_U8       dev;
    GT_BOOL     enable = GT_TRUE;
    GT_BOOL     enableGet;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Call function for with enable = GT_TRUE and GT_FALSE.       */
        /* Expected: GT_OK for Cheetah devices and GT_BAD_PARAM for others. */

        /* Call function with [enable==GT_FALSE] */
        enable = GT_FALSE;

        st = cpssDxChBrgVlanEgressFilteringEnable(dev, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, enable);

        enableGet = GT_TRUE;

        st = cpssDxChBrgVlanEgressFilteringEnableGet(dev, &enableGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
              "cpssDxChBrgVlanEgressFilteringEnableGet: %d, %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                "get another enable state than was set: %d, %d", dev);

        /* Call function with [enable==GT_TRUE] */
        enable = GT_TRUE;

        st = cpssDxChBrgVlanEgressFilteringEnable(dev, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, enable);

        enableGet = GT_FALSE;

        st = cpssDxChBrgVlanEgressFilteringEnableGet(dev, &enableGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
              "cpssDxChBrgVlanEgressFilteringEnableGet: %d, %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                "get another enable state than was set: %d, %d", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices   */
    /* enable == GT_TRUE                */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanEgressFilteringEnable(dev, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/

    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    /* enable == GT_TRUE */

    st = cpssDxChBrgVlanEgressFilteringEnable(dev, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgVlanEgressFilteringEnableGet
(
    IN  GT_U8   dev,
    OUT GT_BOOL *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanEgressFilteringEnableGet)
{
/*
    ITERATE_DEVICES_PHY_PORT (DxChx)
    1.1. Call function with correct parameters.
    Expected: GT_OK.
    1.2. For active device check that function returns GT_BAD_PTR.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st     = GT_OK;
    GT_U8       dev;
    GT_BOOL     enable;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Call function for with enable.  */
        /* Expected: GT_OK. */
        st = cpssDxChBrgVlanEgressFilteringEnableGet(dev, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, enable);

        /* 1.2. For active device check that function returns GT_BAD_PTR */
        /* Expected: GT_BAD_PTR. */
        st = cpssDxChBrgVlanEgressFilteringEnableGet(dev, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanEgressFilteringEnableGet(dev, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgVlanEgressFilteringEnableGet(dev, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}
/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgRoutedUnicastEgressFilteringEnable
(
    IN GT_U8        dev,
    IN GT_BOOL      enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgRoutedUnicastEgressFilteringEnable)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call function with enable [GT_FALSE and GT_TRUE].
    Expected: GT_OK for Cheetah devices and GT_BAD_PARAM for others.
*/
    GT_STATUS   st     = GT_OK;
    GT_U8       dev;
    GT_BOOL     enable = GT_TRUE;
    GT_BOOL     enableGet;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Call function for with enable = GT_TRUE and GT_FALSE.       */
        /* Expected: GT_OK for Cheetah devices and GT_BAD_PARAM for others. */

        /* Call function with [enable==GT_FALSE] */
        enable = GT_FALSE;

        st = cpssDxChBrgRoutedUnicastEgressFilteringEnable(dev, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, enable);

        enableGet = GT_TRUE;

        st = cpssDxChBrgRoutedUnicastEgressFilteringEnableGet(dev, &enableGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
              "cpssDxChBrgRoutedUnicastEgressFilteringEnableGet: %d, %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                "get another enable state than was set: %d, %d", dev);

        /* Call function with [enable==GT_TRUE] */
        enable = GT_TRUE;

        st = cpssDxChBrgRoutedUnicastEgressFilteringEnable(dev, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, enable);

        enableGet = GT_FALSE;

        st = cpssDxChBrgRoutedUnicastEgressFilteringEnableGet(dev, &enableGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
              "cpssDxChBrgRoutedUnicastEgressFilteringEnableGet: %d, %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                "get another enable state than was set: %d, %d", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices   */
    /* enable == GT_TRUE                */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgRoutedUnicastEgressFilteringEnable(dev, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/

    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    /* enable == GT_TRUE */

    st = cpssDxChBrgRoutedUnicastEgressFilteringEnable(dev, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgRoutedUnicastEgressFilteringEnableGet
(
    IN  GT_U8   dev,
    OUT GT_BOOL *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgRoutedUnicastEgressFilteringEnableGet)
{
/*
    ITERATE_DEVICES_PHY_PORT (DxChx)
    1.1. Call function with correct parameters.
    Expected: GT_OK.
    1.2. For active device check that function returns GT_BAD_PTR.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st     = GT_OK;
    GT_U8       dev;
    GT_BOOL     enable;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Call function for with enable.  */
        /* Expected: GT_OK. */
        st = cpssDxChBrgRoutedUnicastEgressFilteringEnableGet(dev, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, enable);

        /* 1.2. For active device check that function returns GT_BAD_PTR */
        /* Expected: GT_BAD_PTR. */
        st = cpssDxChBrgRoutedUnicastEgressFilteringEnableGet(dev, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgRoutedUnicastEgressFilteringEnableGet(dev, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgRoutedUnicastEgressFilteringEnableGet(dev, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgRoutedSpanEgressFilteringEnable
(
    IN GT_U8        dev,
    IN GT_BOOL      enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgRoutedSpanEgressFilteringEnable)
{
/*
    ITERATE_DEVICES
    1.1. Call function with enable [GT_FALSE and GT_TRUE].
    Expected: GT_OK for Cheetah devices and GT_BAD_PARAM for others.
*/
    GT_STATUS   st     = GT_OK;
    GT_U8       dev;
    GT_BOOL     enable = GT_TRUE;
    GT_BOOL     enableGet;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Call function for with enable = GT_TRUE and GT_FALSE.       */
        /* Expected: GT_OK for Cheetah devices and GT_BAD_PARAM for others. */

        /* Call function with [enable==GT_FALSE] */
        enable = GT_FALSE;

        st = cpssDxChBrgRoutedSpanEgressFilteringEnable(dev, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, enable);

        enableGet = GT_TRUE;

        st = cpssDxChBrgRoutedSpanEgressFilteringEnableGet(dev, &enableGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
              "cpssDxChBrgRoutedSpanEgressFilteringEnableGet: %d, %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                      "get another enable state than was set: %d, %d", dev);

        /* Call function with [enable==GT_TRUE] */
        enable = GT_TRUE;

        st = cpssDxChBrgRoutedSpanEgressFilteringEnable(dev, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, enable);

        enableGet = GT_FALSE;

        st = cpssDxChBrgRoutedSpanEgressFilteringEnableGet(dev, &enableGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
              "cpssDxChBrgRoutedSpanEgressFilteringEnableGet: %d, %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                      "get another enable state than was set: %d, %d", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices   */
    /* enable == GT_TRUE                */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgRoutedSpanEgressFilteringEnable(dev, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/

    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    /* enable == GT_TRUE */

    st = cpssDxChBrgRoutedSpanEgressFilteringEnable(dev, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgRoutedSpanEgressFilteringEnableGet
(
    IN  GT_U8   dev,
    OUT GT_BOOL *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgRoutedSpanEgressFilteringEnableGet)
{
/*
    ITERATE_DEVICES_PHY_PORT (DxChx)
    1.1. Call function with correct parameters.
    Expected: GT_OK.
    1.2. For active device check that function returns GT_BAD_PTR.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st     = GT_OK;
    GT_U8       dev;
    GT_BOOL     enable;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Call function for with enable.  */
        /* Expected: GT_OK. */
        st = cpssDxChBrgRoutedSpanEgressFilteringEnableGet(dev, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, enable);

        /* 1.2. For active device check that function returns GT_BAD_PTR */
        /* Expected: GT_BAD_PTR. */

        st = cpssDxChBrgRoutedSpanEgressFilteringEnableGet(dev, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgRoutedSpanEgressFilteringEnableGet(dev, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgRoutedSpanEgressFilteringEnableGet(dev, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgPortEgrFltUregBcEnable
(
    IN GT_U8        dev,
    IN GT_U8        port,
    IN GT_BOOL      enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgPortEgrFltUregBcEnable)
{
/*
    ITERATE_DEVICES_PHY_PORT (DxCh2 and above)
    1.1.1. Call function with enable [GT_FALSE and GT_TRUE].
    Expected: GT_OK for Cheetah2 devices and GT_BAD_PARAM for others.
*/
    GT_STATUS   st     = GT_OK;
    GT_U8       dev;
    GT_PORT_NUM port = BRG_EGR_FLT_VALID_PHY_PORT_CNS;
    GT_BOOL     enable = GT_TRUE;
    GT_BOOL     enableGet;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            /* 1.1.1. Call function for with enable = GT_TRUE and GT_FALSE.     */
            /* Expected: GT_OK for Cheetah devices and GT_BAD_PARAM for others. */

            /* Call function with [enable==GT_FALSE] */
            enable = GT_FALSE;

            st = cpssDxChBrgPortEgrFltUregBcEnable(dev, port, enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);

            enableGet = GT_TRUE;

            st = cpssDxChBrgPortEgrFltUregBcEnableGet(dev, port, &enableGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                  "cpssDxChBrgPortEgrFltUregBcEnableGet: %d, %d", dev, port);
            UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                 "get another enable state than was set: %d, %d", dev, port);

            /* Call function with [enable==GT_TRUE] */
            enable = GT_TRUE;

            st = cpssDxChBrgPortEgrFltUregBcEnable(dev, port, enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);

            enableGet = GT_FALSE;

            st = cpssDxChBrgPortEgrFltUregBcEnableGet(dev, port, &enableGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                  "cpssDxChBrgPortEgrFltUregBcEnableGet: %d, %d", dev, port);
            UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                 "get another enable state than was set: %d, %d", dev, port);
        }

        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while (GT_OK == prvUtfNextPhyPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port    */
            /* enable == GT_TRUE                                */
            st = cpssDxChBrgPortEgrFltUregBcEnable(dev, port, enable);
            if(IS_E_ARCH_AND_PHYSICAL_PORT_IN_RANGE_MAC(dev,port))
            {
                /* the range is '256 physical ports' */
                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
            }
            else
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChBrgPortEgrFltUregBcEnable(dev, port, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                     */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgPortEgrFltUregBcEnable(dev, port, enable);
        if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);
        else
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, port, enable);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    port = BRG_EGR_FLT_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    /* Go over all non active devices.  */
    /* enable == GT_TRUE                */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgPortEgrFltUregBcEnable(dev, port, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    /* port == 0            */
    /* enable == GT_TRUE    */

    st = cpssDxChBrgPortEgrFltUregBcEnable(dev, port, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgPortEgrFltUregBcEnableGet
(
    IN  GT_U8   dev,
    IN  GT_U8   port,
    OUT GT_BOOL *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgPortEgrFltUregBcEnableGet)
{
/*
    ITERATE_DEVICES_PHY_PORT (DxChx)
    1.1.1. Call function with correct parameters.
    Expected: GT_OK.
*/
    GT_STATUS   st     = GT_OK;
    GT_U8       dev;
    GT_PORT_NUM       port   = BRG_EGR_FLT_VALID_PHY_PORT_CNS;
    GT_BOOL     enable;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            /* 1.1.1. Call function for with enable.  */
            /* Expected: GT_OK. */
            st = cpssDxChBrgPortEgrFltUregBcEnableGet(dev, port, &enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);

            /* 1.1.2. For active device check that function returns GT_BAD_PTR */
            st = cpssDxChBrgPortEgrFltUregBcEnableGet(dev, port, NULL);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, port);
        }
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while (GT_OK == prvUtfNextPhyPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChBrgPortEgrFltUregBcEnableGet(dev, port, &enable);
            if(IS_E_ARCH_AND_PHYSICAL_PORT_IN_RANGE_MAC(dev,port))
            {
                /* the range is '256 physical ports' */
                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
            }
            else
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChBrgPortEgrFltUregBcEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                     */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgPortEgrFltUregBcEnableGet(dev, port, &enable);
        if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);
        else
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, port, enable);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    port = BRG_EGR_FLT_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgPortEgrFltUregBcEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgPortEgrFltUregBcEnableGet(dev, port, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgPortEgressMcastLocalEnable
(
    IN GT_U8        dev,
    IN GT_U8        port,
    IN GT_BOOL      enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgPortEgressMcastLocalEnable)
{
/*
    ITERATE_DEVICES_PHY_PORT (DxChx)
    1.1.1. Call function with enable [GT_FALSE and GT_TRUE].
    Expected: GT_OK for Cheetah devices and GT_BAD_PARAM for others.
*/
    GT_STATUS   st     = GT_OK;
    GT_U8       dev;
    GT_PORT_NUM       port   = BRG_EGR_FLT_VALID_PHY_PORT_CNS;
    GT_BOOL     enable = GT_TRUE;
    GT_BOOL     enableGet;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            /* 1.1.1. Call function for with enable = GT_TRUE and GT_FALSE.     */
            /* Expected: GT_OK for Cheetah devices and GT_BAD_PARAM for others. */

            /* Call function with [enable==GT_FALSE] */
            enable = GT_FALSE;

            st = cpssDxChBrgPortEgressMcastLocalEnable(dev, port, enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);

            enableGet = GT_TRUE;

            st = cpssDxChBrgPortEgressMcastLocalEnableGet(dev, port, &enableGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                  "cpssDxChBrgPortEgressMcastLocalEnableGet: %d, %d", dev, port);
            UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                     "get another enable state than was set: %d, %d", dev, port);

            /* Call function with [enable==GT_TRUE] */
            enable = GT_TRUE;

            st = cpssDxChBrgPortEgressMcastLocalEnable(dev, port, enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);

            enableGet = GT_FALSE;

            st = cpssDxChBrgPortEgressMcastLocalEnableGet(dev, port, &enableGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                  "cpssDxChBrgPortEgressMcastLocalEnableGet: %d, %d", dev, port);
            UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                     "get another enable state than was set: %d, %d", dev, port);
        }

        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while (GT_OK == prvUtfNextPhyPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            /* enable == GT_TRUE                             */
            st = cpssDxChBrgPortEgressMcastLocalEnable(dev, port, enable);
            if(IS_E_ARCH_AND_PHYSICAL_PORT_IN_RANGE_MAC(dev,port))
            {
                /* the range is '256 physical ports' */
                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
            }
            else
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChBrgPortEgressMcastLocalEnable(dev, port, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                     */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgPortEgressMcastLocalEnable(dev, port, enable);
        if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);
        else
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, port, enable);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    port = BRG_EGR_FLT_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    /* enable == GT_TRUE               */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgPortEgressMcastLocalEnable(dev, port, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    /* port == 0         */
    /* enable == GT_TRUE */

    st = cpssDxChBrgPortEgressMcastLocalEnable(dev, port, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgPortEgressMcastLocalEnableGet
(
    IN  GT_U8    dev,
    IN  GT_U8    port,
    OUT GT_BOOL *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgPortEgressMcastLocalEnableGet)
{
/*
    ITERATE_DEVICES_PHY_PORT (DxChx)
    1.1.1. Call function with correct parameters.
    Expected: GT_OK.
*/
    GT_STATUS   st     = GT_OK;
    GT_U8       dev;
    GT_PORT_NUM       port   = BRG_EGR_FLT_VALID_PHY_PORT_CNS;
    GT_BOOL     enable = GT_TRUE;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            /* 1.1.1. Call function for with enable. */
            /* Expected: GT_OK. */
            st = cpssDxChBrgPortEgressMcastLocalEnableGet(dev, port, &enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);

            /* 1.1.2. For active device check that function returns GT_BAD_PTR */
            /* Expected: GT_BAD_PTR. */
            st = cpssDxChBrgPortEgressMcastLocalEnableGet(dev, port, NULL);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, port);
        }

        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while (GT_OK == prvUtfNextPhyPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChBrgPortEgressMcastLocalEnableGet(dev, port, &enable);
            if(IS_E_ARCH_AND_PHYSICAL_PORT_IN_RANGE_MAC(dev,port))
            {
                /* the range is '256 physical ports' */
                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
            }
            else
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChBrgPortEgressMcastLocalEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                     */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgPortEgressMcastLocalEnableGet(dev, port, &enable);
        if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);
        else
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, port, enable);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    port = BRG_EGR_FLT_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgPortEgressMcastLocalEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgPortEgressMcastLocalEnableGet(dev, port, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgPortEgrFltIpMcRoutedEnable
(
    IN GT_U8    dev,
    IN GT_U8    port,
    IN GT_BOOL  enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgPortEgrFltIpMcRoutedEnable)
{
/*
    ITERATE_DEVICES_PHY_PORT (DxChx)
    1.1.1. Call function with enable [GT_FALSE and GT_TRUE].
    Expected: GT_OK.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;
    GT_PORT_NUM       port   = BRG_EGR_FLT_VALID_PHY_PORT_CNS;
    GT_BOOL     enable = GT_TRUE;
    GT_BOOL     enableGet;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E |UTF_CH1_DIAMOND_E | UTF_XCAT2_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&port, GT_TRUE))
        {
            /* 1.1.1. Call function for with enable [GT_TRUE and GT_FALSE].     */
            /* Expected: GT_OK. */

            /* Call function with [enable==GT_FALSE] */
            enable = GT_FALSE;

            st = cpssDxChBrgPortEgrFltIpMcRoutedEnable(dev, port, enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);

            enableGet = GT_TRUE;

            st = cpssDxChBrgPortEgrFltIpMcRoutedEnableGet(dev, port, &enableGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                  "cpssDxChBrgPortEgrFltIpMcRoutedEnableGet: %d, %d", dev, port);
            UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                     "get another enable state than was set: %d, %d", dev, port);

            /* Call function with [enable==GT_TRUE] */
            enable = GT_TRUE;

            st = cpssDxChBrgPortEgrFltIpMcRoutedEnable(dev, port, enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);

            enableGet = GT_FALSE;

            st = cpssDxChBrgPortEgrFltIpMcRoutedEnableGet(dev, port, &enableGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                  "cpssDxChBrgPortEgrFltIpMcRoutedEnableGet: %d, %d", dev, port);
            UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                     "get another enable state than was set: %d, %d", dev, port);
        }

        enable = GT_FALSE;

        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            /* enable == GT_TRUE                             */
            st = cpssDxChBrgPortEgrFltIpMcRoutedEnable(dev, port, enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChBrgPortEgrFltIpMcRoutedEnable(dev, port, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                     */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgPortEgrFltIpMcRoutedEnable(dev, port, enable);
        if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);
        else
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, port, enable);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    port = BRG_EGR_FLT_VALID_PHY_PORT_CNS;
    enable = GT_FALSE;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    /* enable == GT_TRUE               */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgPortEgrFltIpMcRoutedEnable(dev, port, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    /* port == 0         */
    /* enable == GT_TRUE */

    st = cpssDxChBrgPortEgrFltIpMcRoutedEnable(dev, port, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgPortEgrFltIpMcRoutedEnableGet
(
    IN  GT_U8   dev,
    IN  GT_U8   port,
    OUT GT_BOOL *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgPortEgrFltIpMcRoutedEnableGet)
{
/*
    ITERATE_DEVICES_PHY_PORT (DxChx)
    1.1.1. Call function.
    Expected: GT_OK.
*/
    GT_STATUS   st     = GT_OK;
    GT_U8       dev;
    GT_PORT_NUM       port   = BRG_EGR_FLT_VALID_PHY_PORT_CNS;
    GT_BOOL     enable;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E |UTF_CH1_DIAMOND_E | UTF_XCAT2_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&port, GT_TRUE))
        {
            /* 1.1.1. Call function for with enable.  */
            /* Expected: GT_OK. */
            st = cpssDxChBrgPortEgrFltIpMcRoutedEnableGet(dev, port, &enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);

            /* 1.1.2. For active device check that function returns GT_BAD_PTR */
            st = cpssDxChBrgPortEgrFltIpMcRoutedEnableGet(dev, port, NULL);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, port);
        }
        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChBrgPortEgrFltIpMcRoutedEnableGet(dev, port, &enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChBrgPortEgrFltIpMcRoutedEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                     */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgPortEgrFltIpMcRoutedEnableGet(dev, port, &enable);
        if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);
        else
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, port, enable);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    port = BRG_EGR_FLT_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgPortEgrFltIpMcRoutedEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgPortEgrFltIpMcRoutedEnableGet(dev, port, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgEgrFltPortLinkEnableSet
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    IN  CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_ENT portLinkStatusState
);
*/
UTF_TEST_CASE_MAC(cpssDxChBrgEgrFltPortLinkEnableSet)
{
/*
    ITERATE_DEVICES_VIRT_PORTS (Bobcat2; Caelum, Aldrin; AC3X; Bobcat3.)
    1.1.1 Call cpssDxChBrgEgrFltPortLinkEnableSet with
         portLinkStatusState [ CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_FORCE_LINK_UP_E /
                               CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_FORCE_LINK_DOWN_E
                               CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_LINK_STATE_BASED_E ]
    Expected: GT_OK.
    1.1.2 Call cpssDxChBrgEgrFltPortLinkEnableGet with
         non-NULL portLinkStatusState.
    Expected: GT_OK and the same portLinkStatusState.

    1.1.3 Call function with out of range portLinkStatusState
    Expected: GT_BAD_PARAM.
*/
    #define        BRG_EGR_FLT_PORT_LINK_STATUS_MAX_STC    3

    GT_STATUS      st  = GT_OK;
    GT_U8          dev;
    GT_PORT_NUM    port;
    GT_U32         notAppFamilyBmp = 0;
    GT_U32         statusIdx;
    GT_STATUS       expectedSt;

    CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_ENT portLinkStatusState;
    CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_ENT portLinkStatusStateGet;


    CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_ENT portLinkStatusStateArr[] = {
        CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_FORCE_LINK_UP_E,
        CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_FORCE_LINK_DOWN_E,
        CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_LINK_STATE_BASED_E
    };

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1. For all active devices go over all available virtual ports. */
        while (GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            for (statusIdx = 0; statusIdx < BRG_EGR_FLT_PORT_LINK_STATUS_MAX_STC; statusIdx++)
            {
                portLinkStatusState = portLinkStatusStateArr[statusIdx];

                /*
                  1.1.1 Call cpssDxChBrgEgrFltPortLinkEnableSet with
                    portLinkStatusState [  CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_FORCE_LINK_UP_E /
                                           CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_FORCE_LINK_DOWN_E
                                                Expected: GT_OK - Bobcat2, Caelum, Aldrin, AC3X, Bobcat3.
                                           CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_LINK_STATE_BASED_E ]
                                                Expected: GT_OK - Bobcat2, Caelum, Aldrin, AC3X.
                                                Expected: GT_BAD_PARAM -  Bobcat3

                */
                expectedSt = GT_OK;
                st = cpssDxChBrgEgrFltPortLinkEnableSet(dev, port, portLinkStatusState);
                if (PRV_CPSS_SIP_5_20_CHECK_MAC(dev))
                {
                    if (portLinkStatusState == CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_LINK_STATE_BASED_E )
                    {
                        /* value not supported */
                        expectedSt = GT_BAD_PARAM;
                    }
                }
                else
                {
                    if((portLinkStatusState == CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_LINK_STATE_BASED_E ) &&
                        (PRV_CPSS_DXCH_PP_MAC(dev)->port.portsMapInfoShadowArr[port].valid == GT_FALSE ||
                         PRV_CPSS_DXCH_PP_MAC(dev)->port.portsMapInfoShadowArr[port].portMap.mappingType ==
                            CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E))
                    {
                        /* no mapping to MAC */
                        expectedSt = GT_BAD_PARAM;
                    }
                }

                UTF_VERIFY_EQUAL3_PARAM_MAC(expectedSt, st, dev, port, portLinkStatusState);

                /*
                  1.1.2 Call cpssDxChBrgEgrFltPortLinkEnableGet with
                    non-NULL portLinkStatusState.
                */
                st = cpssDxChBrgEgrFltPortLinkEnableGet(dev, port, &portLinkStatusStateGet);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                    "cpssDxChBrgEgrFltPortLinkEnableGet: %d, %d",
                    dev, port);
                if(expectedSt == GT_OK)
                {
                    UTF_VERIFY_EQUAL2_STRING_MAC(portLinkStatusState, portLinkStatusStateGet,
                              "got another port link status than was set: %d, %d", dev, port);
                }
            }

            portLinkStatusState = CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_FORCE_LINK_DOWN_E;
            /*
              1.2 Call function with out of range portLinkStatusState
                Expected: GT_BAD_PARAM.
            */

            UTF_ENUMS_CHECK_MAC(cpssDxChBrgEgrFltPortLinkEnableSet
                                (dev, port, portLinkStatusState), portLinkStatusState);

        }

        portLinkStatusState = CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_FORCE_LINK_DOWN_E;

        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /*
            1.3.1 For all active devices go over all non available physical ports
        */
        while (GT_OK == prvUtfNextPhyPortGet(&port, GT_FALSE))
        {
            /* 1.3.2. Call function for each non-active port */
            st = cpssDxChBrgEgrFltPortLinkEnableSet(dev, port, portLinkStatusState);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /*
            1.4. For active device check that function returns GT_BAD_PARAM
                 for out of bound value for port number.
        */
        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChBrgEgrFltPortLinkEnableSet(dev, port, portLinkStatusState);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /*
            1.5. For active device check that function returns GT_OK
                 for CPU port number.
        */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgEgrFltPortLinkEnableSet(dev, port, portLinkStatusState);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

        portLinkStatusState = CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_FORCE_LINK_UP_E;
        st = cpssDxChBrgEgrFltPortLinkEnableSet(dev, port, portLinkStatusState);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    /*
        2. For not-active devices and devices from non-applicable family
           check that function returns GT_BAD_PARAM.
    */

    port = 0;
    portLinkStatusState = CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_FORCE_LINK_UP_E;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgEgrFltPortLinkEnableSet(dev, port, portLinkStatusState);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgEgrFltPortLinkEnableSet(dev, port, portLinkStatusState);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgEgrFltPortLinkEnableGet
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    OUT CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_ENT * portLinkStatusStatePtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChBrgEgrFltPortLinkEnableGet)
{
/*
    ITERATE_DEVICES (Bobcat2, Caelum, Bobcat3)
    1.1.1 Call function with non-NULL portLinkStatusStatePtr.
    Expected: GT_OK.
    1.1.2. Call function with portLinkStatusStatePtr[NULL].
    Expected: GT_BAD_PTR.

*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;
    GT_PORT_NUM port;
    GT_U32      notAppFamilyBmp = 0;
    CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_ENT portLinkStatusState =
                              CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_FORCE_LINK_UP_E;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* For all active devices go over all available physical ports */
        while (GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            /* 1.1.1. Call function for with enable.  */
            /* Expected: GT_OK. */
            st = cpssDxChBrgEgrFltPortLinkEnableGet(dev, port, &portLinkStatusState);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, portLinkStatusState);

            /* 1.1.2. For active device check that function returns GT_BAD_PTR */
            st = cpssDxChBrgEgrFltPortLinkEnableGet(dev, port, NULL);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, port);
        }
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while (GT_OK == prvUtfNextPhyPortGet(&port, GT_FALSE))
        {
            /* 1.2.1 Call function for each non-active port */
            st = cpssDxChBrgEgrFltPortLinkEnableGet(dev, port, &portLinkStatusState);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChBrgEgrFltPortLinkEnableGet(dev, port, &portLinkStatusState);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portLinkStatusState);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                     */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgEgrFltPortLinkEnableGet(dev, port, &portLinkStatusState);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, portLinkStatusState);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    port = BRG_EGR_FLT_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgEgrFltPortLinkEnableGet(dev, port, &portLinkStatusState);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgEgrFltPortLinkEnableGet(dev, port, &portLinkStatusState);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgEgrFltVlanPortFilteringEnableSet
(
    IN GT_U8          devNum,
    IN GT_PORT_NUM    port,
    IN GT_BOOL        enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgEgrFltVlanPortFilteringEnableSet)
{
/*
    ITERATE_DEVICES_VIRT_PORTS (Bobcat2, Caelum, Bobcat3)
    1.1.1. Call with enable [GT_FALSE / GT_TRUE].
    Expected: GT_OK.
    1.1.2. Call cpssDxChBrgEgrFltVlanPortFilteringEnableGet with
           not NULL enablePtr
    Expected: GT_OK
*/
    GT_STATUS      st  = GT_OK;
    GT_U8          dev;
    GT_PORT_NUM    port;
    GT_U32         notAppFamilyBmp = 0;
    GT_BOOL        enable    = GT_FALSE;
    GT_BOOL        enableGet = GT_FALSE;


    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call with enable [GT_FALSE / GT_TRUE].
                Expected: GT_OK.
            */
            /* iterate with enable = GT_FALSE */
            enable = GT_FALSE;

            st = cpssDxChBrgEgrFltVlanPortFilteringEnableSet(dev, port, enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);

            /*
               1.1.2. Call cpssDxChBrgEgrFltVlanPortFilteringEnableGet
                      with not NULL enablePtr
               Expected: GT_OK
            */
            st = cpssDxChBrgEgrFltVlanPortFilteringEnableGet(dev, port,
                                                             &enableGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                "cpssDxChBrgEgrFltVlanPortFilteringEnableGet: %d, %d",
                dev, port);
            UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                          "got another enable than was set: %d, %d", dev, port);

            /* iterate with enable = GT_TRUE */
            enable = GT_TRUE;

            st = cpssDxChBrgEgrFltVlanPortFilteringEnableSet(dev, port, enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);

            /*
               1.1.2. Call cpssDxChBrgEgrFltVlanPortFilteringEnableGet
                      with not NULL enablePtr
               Expected: GT_OK
            */
            st = cpssDxChBrgEgrFltVlanPortFilteringEnableGet(dev, port,
                                                             &enableGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                "cpssDxChBrgEgrFltVlanPortFilteringEnableGet: %d, %d",
                dev, port);
            UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                          "got another enable than was set: %d, %d", dev, port);
        }

        /* restore correct values*/
        enable = GT_FALSE;

        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /*
            1.2. For all active devices go over all non available physical ports
        */
        while (GT_OK == prvUtfNextVirtPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChBrgEgrFltVlanPortFilteringEnableSet(dev, port, enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /*
            1.3. For active device check that function returns GT_BAD_PARAM
                 for out of bound value for port number.
        */
        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChBrgEgrFltVlanPortFilteringEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /*
            1.4. For active device check that function returns GT_OK
                 for CPU port number.
        */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgEgrFltVlanPortFilteringEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    enable = GT_FALSE;
    port   = 0;

    /*
        2. For not-active devices and devices from non-applicable family
           check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgEgrFltVlanPortFilteringEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgEgrFltVlanPortFilteringEnableSet(dev, port, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgEgrFltVlanPortFilteringEnableGet
(
    IN  GT_U8          devNum,
    IN  GT_PORT_NUM    port,
    OUT GT_BOOL        *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgEgrFltVlanPortFilteringEnableGet)
{
/*
    ITERATE_DEVICES_VIRT_PORTS (Bobcat2, Caelum, Bobcat3)
    1.1.1. Call with not NULL enablePtr.
    Expected: GT_OK.
    1.1.2. Call with enablePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS      st  = GT_OK;
    GT_U8          dev;
    GT_PORT_NUM    port;
    GT_U32         notAppFamilyBmp = 0;
    GT_BOOL        enable = GT_FALSE;


    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call with not NULL enablePtr.
                Expected: GT_OK.
            */
            st = cpssDxChBrgEgrFltVlanPortFilteringEnableGet(dev, port,&enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*
                1.1.2. Call with enablePtr [NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChBrgEgrFltVlanPortFilteringEnableGet(dev, port, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st,
                                         "%d, %d, enablePtr = NULL", dev, port);
        }

        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /*
            1.2. For all active devices go over all non available physical ports
        */
        while (GT_OK == prvUtfNextVirtPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChBrgEgrFltVlanPortFilteringEnableGet(dev, port,&enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /*
            1.3. For active device check that function returns GT_BAD_PARAM
                 for out of bound value for port number.
        */
        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChBrgEgrFltVlanPortFilteringEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /*
            1.4. For active device check that function returns GT_OK
                 for CPU port number.
        */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgEgrFltVlanPortFilteringEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    port = 0;

    /*
        2. For not-active devices and devices from non-applicable family
           check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgEgrFltVlanPortFilteringEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgEgrFltVlanPortFilteringEnableGet(dev, port, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgEgrFltVlanPortMemberSet
(
    IN  GT_U8            devNum,
    IN  GT_U32           vidIndex,
    IN  GT_PORT_NUM      portNum,
    IN  GT_BOOL          isMember
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgEgrFltVlanPortMemberSet)
{
/*
    ITERATE_DEVICES_VIRT_PORTS (Bobcat2, Caelum, Bobcat3)
    1.1.1. Call function with vidIndex [0 / 0xA5A / BIT_12-1] and
                              isMember [GT_FALSE / GT_TRUE].
    Expected: GT_OK.
    1.1.2. Call cpssDxChBrgEgrFltVlanPortMemberGet with not NULL isMemberPtr
    Expected: GT_OK
    Expected: NOT GT_OK.
*/
    GT_STATUS      st  = GT_OK;
    GT_STATUS      rc;
    GT_U8          dev;
    GT_PORT_NUM    port;
    GT_U32         notAppFamilyBmp = 0;
    GT_U32         vidIndex = 0;
    GT_BOOL        isMember    = GT_FALSE;
    GT_BOOL        isMemberGet = GT_FALSE;
    CPSS_DXCH_BRG_EGR_FLT_VLAN_PORT_ACCESS_MODE_ENT fltTabAccessMode;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* sip6 not supported -- add it to 'not applicable' */
    UTF_SIP6_ADD_TO_FAMILY_BMP_MAC(notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call function with vidIndex [0 / 0xA5A / BIT_12-1] and
                                          isMember [GT_FALSE / GT_TRUE].
                Expected: GT_OK.
            */

            st = cpssDxChBrgEgrFltVlanPortAccessModeGet(dev, &fltTabAccessMode);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

            if( port < (GT_U32)(1 << (8 + fltTabAccessMode)) )
            {
                rc = GT_OK;
            }
            else
            {
                rc = GT_OUT_OF_RANGE;
            }

            /* Call with vidIndex [0] and isMember[GT_FALSE] */
            vidIndex = 0;
            isMember = GT_FALSE;
            st = cpssDxChBrgEgrFltVlanPortMemberSet(dev, vidIndex, port,
                                                    isMember);
            UTF_VERIFY_EQUAL3_PARAM_MAC(rc, st, dev, port, vidIndex);

            /* verify values */
            st = cpssDxChBrgEgrFltVlanPortMemberGet(dev, vidIndex, port,
                                                    &isMemberGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(rc, st,
                       "cpssDxChBrgEgrFltVlanPortMemberGet: %d, %d", dev, port);
            if( GT_OK == st )
            {
                UTF_VERIFY_EQUAL2_STRING_MAC(isMember, isMemberGet,
                        "got another isMember than was set: %d, %d", dev, port);
            }

            /* Call with vidIndex [0xA5A] and isMember[GT_TRUE] */
            vidIndex = 0xA5A;
            isMember = GT_TRUE;
            st = cpssDxChBrgEgrFltVlanPortMemberSet(dev, vidIndex, port,
                                                    isMember);
            UTF_VERIFY_EQUAL3_PARAM_MAC(rc, st, dev, port, vidIndex);

            /* verify values */
            st = cpssDxChBrgEgrFltVlanPortMemberGet(dev, vidIndex, port,
                                                    &isMemberGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(rc, st,
                       "cpssDxChBrgEgrFltVlanPortMemberGet: %d, %d", dev, port);
            if( GT_OK == st )
            {
                UTF_VERIFY_EQUAL2_STRING_MAC(isMember, isMemberGet,
                        "got another isMember than was set: %d, %d", dev, port);
            }

            /*
                Call with vidIndex [BIT_12-1] and isMember[GT_FALSE]
            */
            vidIndex = BIT_12-1;
            isMember = GT_FALSE;
            st = cpssDxChBrgEgrFltVlanPortMemberSet(dev, vidIndex, port,
                                                    isMember);
            UTF_VERIFY_EQUAL3_PARAM_MAC(rc, st, dev, port, vidIndex);

            /*
               1.1.2. Call cpssDxChBrgEgrFltVlanPortMemberGet with
                      not NULL isMemberPtr
               Expected: GT_OK
            */
            st = cpssDxChBrgEgrFltVlanPortMemberGet(dev, vidIndex, port,
                                                    &isMemberGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(rc, st,
                       "cpssDxChBrgEgrFltVlanPortMemberGet: %d, %d", dev, port);
            if( GT_OK == st )
            {
                UTF_VERIFY_EQUAL2_STRING_MAC(isMember, isMemberGet,
                        "got another isMember than was set: %d, %d", dev, port);
            }
        }

        /* restore correct values*/
        isMember = GT_FALSE;
        vidIndex = 0;

        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /*
            1.2. For all active devices go over all non available physical ports
        */
        while (GT_OK == prvUtfNextVirtPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChBrgEgrFltVlanPortMemberSet(dev, vidIndex, port, isMember);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /*
            1.3. For active device check that function returns GT_BAD_PARAM
                 for out of bound value for port number.
        */
        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChBrgEgrFltVlanPortMemberSet(dev, vidIndex, port, isMember);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /*
            1.4. For active device check that function returns GT_OK
                 for CPU port number.
        */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgEgrFltVlanPortMemberSet(dev, vidIndex, port, isMember);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    /* restore correct values*/
    isMember = GT_FALSE;
    vidIndex = 0;
    port   = 0;

    /*
        2. For not-active devices and devices from non-applicable family
           check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgEgrFltVlanPortMemberSet(dev, vidIndex, port, isMember);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgEgrFltVlanPortMemberSet(dev, vidIndex, port, isMember);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgEgrFltVlanPortMemberGet
(
    IN  GT_U8            devNum,
    IN  GT_U32           vidIndex,
    IN  GT_PORT_NUM      portNum,
    OUT GT_BOOL          *isMemberPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgEgrFltVlanPortMemberGet)
{
/*
    ITERATE_DEVICES_VIRT_PORTS (Bobcat2, Caelum, Bobcat3)
    1.1.1. Call function with vidIndex [0 / 0xA5A / BIT_12-1] and
                              not NULL isMemberPtr.
    Expected: GT_OK.
    1.1.2. Call with isMemberPtr [NULL] and vidIndex [0].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS      st  = GT_OK;
    GT_STATUS      rc;
    GT_U8          dev;
    GT_PORT_NUM    port;
    GT_U32         notAppFamilyBmp = 0;
    GT_U32         vidIndex = 0;
    GT_BOOL        isMember = GT_FALSE;
    CPSS_DXCH_BRG_EGR_FLT_VLAN_PORT_ACCESS_MODE_ENT fltTabAccessMode;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* sip6 not supported -- add it to 'not applicable' */
    UTF_SIP6_ADD_TO_FAMILY_BMP_MAC(notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call function with vidIndex [0 / 0xA5A / BIT_12-1] and
                                          not NULL isMemberPtr.
                Expected: GT_OK.
            */

            st = cpssDxChBrgEgrFltVlanPortAccessModeGet(dev, &fltTabAccessMode);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

            if( port < (GT_U32)(1 << (8 + fltTabAccessMode)) )
            {
                rc = GT_OK;
            }
            else
            {
                rc = GT_OUT_OF_RANGE;
            }

            /* Call with vidIndex [0] */
            vidIndex = 0;
            st = cpssDxChBrgEgrFltVlanPortMemberGet(dev, vidIndex, port,
                                                    &isMember);
            UTF_VERIFY_EQUAL3_PARAM_MAC(rc, st, dev, port, vidIndex);

            /* Call with vidIndex [0xA5A] */
            vidIndex = 0xA5A;
            st = cpssDxChBrgEgrFltVlanPortMemberGet(dev, vidIndex, port,
                                                    &isMember);
            UTF_VERIFY_EQUAL3_PARAM_MAC(rc, st, dev, port, vidIndex);

            /* Call with vidIndex [BIT_12-1] */
            vidIndex = BIT_12-1;
            st = cpssDxChBrgEgrFltVlanPortMemberGet(dev, vidIndex, port,
                                                    &isMember);
            UTF_VERIFY_EQUAL3_PARAM_MAC(rc, st, dev, port, vidIndex);

            /*
                1.1.2. Call with isMemberPtr [NULL] and vidIndex [0].
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChBrgEgrFltVlanPortMemberGet(dev, vidIndex, port, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st,
                                       "%d, %d, isMemberPtr = NULL", dev, port);
        }

        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /*
            1.2. For all active devices go over all non available physical ports
        */
        while (GT_OK == prvUtfNextVirtPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChBrgEgrFltVlanPortMemberGet(dev, vidIndex, port,
                                                    &isMember);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /*
            1.3. For active device check that function returns GT_BAD_PARAM
                 for out of bound value for port number.
        */
        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChBrgEgrFltVlanPortMemberGet(dev, vidIndex, port, &isMember);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /*
            1.4. For active device check that function returns GT_OK
                 for CPU port number.
        */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgEgrFltVlanPortMemberGet(dev, vidIndex, port, &isMember);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }
    /* restore correct values*/
    port = 0;

    /*
        2. For not-active devices and devices from non-applicable family
           check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgEgrFltVlanPortMemberGet(dev, vidIndex, port, &isMember);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgEgrFltVlanPortMemberGet(dev, vidIndex, port, &isMember);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgEgrFltVlanPortVidSelectModeSet
(
    IN  GT_U8                                                  devNum,
    IN  CPSS_DXCH_BRG_EGR_FLT_VLAN_PORT_VID_SELECT_MODE_ENT    vidSelectMode
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgEgrFltVlanPortVidSelectModeSet)
{
/*
    ITERATE_DEVICES (Bobcat2, Caelum, Bobcat3)
    1.1. Call function with
     vidSelectMode [CPSS_DXCH_BRG_EGR_FLT_VLAN_PORT_VID_SELECT_MODE_TAG1_E /
                    CPSS_DXCH_BRG_EGR_FLT_VLAN_PORT_VID_SELECT_MODE_EVLAN_E/
                    CPSS_DXCH_BRG_EGR_FLT_VLAN_PORT_VID_SELECT_MODE_ORIG_VLAN_E]
    Expected: GT_OK.
    1.2. Call cpssDxChBrgEgrFltVlanPortVidSelectModeGet with
         non-NULL vidSelectMode.
    Expected: GT_OK and the same vidSelectMode.
    1.3. Call function with out of range vidSelectMode
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;
    GT_U32      notAppFamilyBmp = 0;
    CPSS_DXCH_BRG_EGR_FLT_VLAN_PORT_VID_SELECT_MODE_ENT vidSelectMode =
                         CPSS_DXCH_BRG_EGR_FLT_VLAN_PORT_VID_SELECT_MODE_TAG1_E;
    CPSS_DXCH_BRG_EGR_FLT_VLAN_PORT_VID_SELECT_MODE_ENT vidSelectModeGet =
                         CPSS_DXCH_BRG_EGR_FLT_VLAN_PORT_VID_SELECT_MODE_TAG1_E;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with vidSelectMode
                   [CPSS_DXCH_BRG_EGR_FLT_VLAN_PORT_VID_SELECT_MODE_TAG1_E /
                    CPSS_DXCH_BRG_EGR_FLT_VLAN_PORT_VID_SELECT_MODE_EVLAN_E/
                    CPSS_DXCH_BRG_EGR_FLT_VLAN_PORT_VID_SELECT_MODE_ORIG_VLAN_E]
            Expected: GT_OK.
        */

        /*
            Call with vidSelectMode
                        [CPSS_DXCH_BRG_EGR_FLT_VLAN_PORT_VID_SELECT_MODE_TAG1_E]
        */
        vidSelectMode = CPSS_DXCH_BRG_EGR_FLT_VLAN_PORT_VID_SELECT_MODE_TAG1_E;

        st = cpssDxChBrgEgrFltVlanPortVidSelectModeSet(dev, vidSelectMode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, vidSelectMode);

        /* Verifying values */
        st = cpssDxChBrgEgrFltVlanPortVidSelectModeGet(dev, &vidSelectModeGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                   "cpssDxChBrgEgrFltVlanPortVidSelectModeGet: %d", dev);

        UTF_VERIFY_EQUAL1_STRING_MAC(vidSelectMode, vidSelectModeGet,
                   "get another vidSelectMode than was set: %d", dev);

        /*
            Call with vidSelectMode
                       [CPSS_DXCH_BRG_EGR_FLT_VLAN_PORT_VID_SELECT_MODE_EVLAN_E]
        */
        vidSelectMode = CPSS_DXCH_BRG_EGR_FLT_VLAN_PORT_VID_SELECT_MODE_EVLAN_E;

        st = cpssDxChBrgEgrFltVlanPortVidSelectModeSet(dev, vidSelectMode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, vidSelectMode);

        /* Verifying values */
        st = cpssDxChBrgEgrFltVlanPortVidSelectModeGet(dev, &vidSelectModeGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                   "cpssDxChBrgEgrFltVlanPortVidSelectModeGet: %d", dev);

        UTF_VERIFY_EQUAL1_STRING_MAC(vidSelectMode, vidSelectModeGet,
                   "get another vidSelectMode than was set: %d", dev);

        /*
            Call with vidSelectMode
                   [CPSS_DXCH_BRG_EGR_FLT_VLAN_PORT_VID_SELECT_MODE_ORIG_VLAN_E]
        */
        vidSelectMode =
                    CPSS_DXCH_BRG_EGR_FLT_VLAN_PORT_VID_SELECT_MODE_ORIG_VLAN_E;

        st = cpssDxChBrgEgrFltVlanPortVidSelectModeSet(dev, vidSelectMode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, vidSelectMode);

        /*
            1.2. Call cpssDxChBrgEgrFltVlanPortVidSelectModeGet with
                 non-NULL vidSelectMode.
            Expected: GT_OK and the same vidSelectMode.
        */
        st = cpssDxChBrgEgrFltVlanPortVidSelectModeGet(dev, &vidSelectModeGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                          "cpssDxChBrgEgrFltVlanPortVidSelectModeGet: %d", dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(vidSelectMode, vidSelectModeGet,
                             "get another vidSelectMode than was set: %d", dev);

        /*
            1.3. Call function with out of range vidSelectMode
            Expected: GT_BAD_PARAM.
        */

        UTF_ENUMS_CHECK_MAC(cpssDxChBrgEgrFltVlanPortVidSelectModeSet
                            (dev, vidSelectMode),
                            vidSelectMode);
    }

    /* restore correct values*/
    vidSelectMode = CPSS_DXCH_BRG_EGR_FLT_VLAN_PORT_VID_SELECT_MODE_TAG1_E;

    /*
      2. For not-active devices and devices from non-applicable family
         check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgEgrFltVlanPortVidSelectModeSet(dev, vidSelectMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgEgrFltVlanPortVidSelectModeSet(dev, vidSelectMode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgEgrFltVlanPortVidSelectModeGet
(
    IN  GT_U8                                                                  devNum,
    OUT CPSS_DXCH_BRG_EGR_FLT_VLAN_PORT_VID_SELECT_MODE_ENT    *vidSelectModePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgEgrFltVlanPortVidSelectModeGet)
{
/*
    ITERATE_DEVICES (Bobcat2, Caelum, Bobcat3)
    1.1. Call function with non-NULL vidSelectModePtr.
    Expected: GT_OK.
    1.2. Call function with vidSelectModePtr [NULL].
    Expected: GT_BAD_PTR.

*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;
    GT_U32      notAppFamilyBmp = 0;
    CPSS_DXCH_BRG_EGR_FLT_VLAN_PORT_VID_SELECT_MODE_ENT vidSelectModePtr =
                         CPSS_DXCH_BRG_EGR_FLT_VLAN_PORT_VID_SELECT_MODE_TAG1_E;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with non-NULL vidSelectModePtr.
            Expected: GT_OK.
        */

        st = cpssDxChBrgEgrFltVlanPortVidSelectModeGet(dev, &vidSelectModePtr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, vidSelectModePtr);

        /*
            1.2. Call function with vidSelectModePtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChBrgEgrFltVlanPortVidSelectModeGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,
                                     "%d, vidSelectModePtr = NULL", dev);
    }

    /*
        2. For not-active devices and devices from non-applicable family
           check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgEgrFltVlanPortVidSelectModeGet(dev, &vidSelectModePtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgEgrFltVlanPortVidSelectModeGet(dev, &vidSelectModePtr);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgEgrFltVlanPortAccessModeSet
(
    IN  GT_U8                                              devNum,
    IN  CPSS_DXCH_BRG_EGR_FLT_VLAN_PORT_ACCESS_MODE_ENT    fltTabAccessMode
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgEgrFltVlanPortAccessModeSet)
{
/*
    ITERATE_DEVICES (Bobcat2, Caelum, Bobcat3)
    1.1. Call function with
          fltTabAccessMode [ CPSS_DXCH_BRG_EGR_FLT_VLAN_PORT_ACCESS_MODE_256_E /
                             CPSS_DXCH_BRG_EGR_FLT_VLAN_PORT_ACCESS_MODE_512_E /
                             CPSS_DXCH_BRG_EGR_FLT_VLAN_PORT_ACCESS_MODE_1K_E /
                             CPSS_DXCH_BRG_EGR_FLT_VLAN_PORT_ACCESS_MODE_2K_E /
                             CPSS_DXCH_BRG_EGR_FLT_VLAN_PORT_ACCESS_MODE_4K_E /
                             CPSS_DXCH_BRG_EGR_FLT_VLAN_PORT_ACCESS_MODE_8K_E /
                             CPSS_DXCH_BRG_EGR_FLT_VLAN_PORT_ACCESS_MODE_16K_E /
                             CPSS_DXCH_BRG_EGR_FLT_VLAN_PORT_ACCESS_MODE_32K_E]
    Expected: GT_OK.
    1.2. Call cpssDxChBrgEgrFltVlanPortAccessModeGet with
         non-NULL fltTabAccessMode.
    Expected: GT_OK and the same fltTabAccessMode.
    1.3. Call function with out of range fltTabAccessMode
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;
    GT_U32      notAppFamilyBmp = 0;
    CPSS_DXCH_BRG_EGR_FLT_VLAN_PORT_ACCESS_MODE_ENT fltTabAccessMode =
                              CPSS_DXCH_BRG_EGR_FLT_VLAN_PORT_ACCESS_MODE_256_E;
    CPSS_DXCH_BRG_EGR_FLT_VLAN_PORT_ACCESS_MODE_ENT fltTabAccessModeGet =
                              CPSS_DXCH_BRG_EGR_FLT_VLAN_PORT_ACCESS_MODE_256_E;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with fltTabAccessMode
                           [ CPSS_DXCH_BRG_EGR_FLT_VLAN_PORT_ACCESS_MODE_256_E /
                             CPSS_DXCH_BRG_EGR_FLT_VLAN_PORT_ACCESS_MODE_512_E /
                             CPSS_DXCH_BRG_EGR_FLT_VLAN_PORT_ACCESS_MODE_1K_E /
                             CPSS_DXCH_BRG_EGR_FLT_VLAN_PORT_ACCESS_MODE_2K_E /
                             CPSS_DXCH_BRG_EGR_FLT_VLAN_PORT_ACCESS_MODE_4K_E /
                             CPSS_DXCH_BRG_EGR_FLT_VLAN_PORT_ACCESS_MODE_8K_E /
                             CPSS_DXCH_BRG_EGR_FLT_VLAN_PORT_ACCESS_MODE_16K_E /
                             CPSS_DXCH_BRG_EGR_FLT_VLAN_PORT_ACCESS_MODE_32K_E]
            Expected: GT_OK.
        */

        /*
            Call with fltTabAccessMode
                             [CPSS_DXCH_BRG_EGR_FLT_VLAN_PORT_ACCESS_MODE_256_E]
        */
        fltTabAccessMode = CPSS_DXCH_BRG_EGR_FLT_VLAN_PORT_ACCESS_MODE_256_E;

        st = cpssDxChBrgEgrFltVlanPortAccessModeSet(dev, fltTabAccessMode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, fltTabAccessMode);

        /* Verifying values */
        st = cpssDxChBrgEgrFltVlanPortAccessModeGet(dev, &fltTabAccessModeGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                   "cpssDxChBrgEgrFltVlanPortAccessModeGet: %d", dev);

        UTF_VERIFY_EQUAL1_STRING_MAC(fltTabAccessMode, fltTabAccessModeGet,
                   "get another fltTabAccessMode than was set: %d", dev);

        /*
            Call with fltTabAccessMode
                             [CPSS_DXCH_BRG_EGR_FLT_VLAN_PORT_ACCESS_MODE_512_E]
        */
        fltTabAccessMode = CPSS_DXCH_BRG_EGR_FLT_VLAN_PORT_ACCESS_MODE_512_E;

        st = cpssDxChBrgEgrFltVlanPortAccessModeSet(dev, fltTabAccessMode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, fltTabAccessMode);

        /* Verifying values */
        st = cpssDxChBrgEgrFltVlanPortAccessModeGet(dev, &fltTabAccessModeGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                   "cpssDxChBrgEgrFltVlanPortAccessModeGet: %d", dev);

        UTF_VERIFY_EQUAL1_STRING_MAC(fltTabAccessMode, fltTabAccessModeGet,
                   "get another fltTabAccessMode than was set: %d", dev);

        /*
            Call with fltTabAccessMode
                              [CPSS_DXCH_BRG_EGR_FLT_VLAN_PORT_ACCESS_MODE_1K_E]
        */
        fltTabAccessMode = CPSS_DXCH_BRG_EGR_FLT_VLAN_PORT_ACCESS_MODE_1K_E;

        st = cpssDxChBrgEgrFltVlanPortAccessModeSet(dev, fltTabAccessMode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, fltTabAccessMode);


        /* Verifying values */
        st = cpssDxChBrgEgrFltVlanPortAccessModeGet(dev, &fltTabAccessModeGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                   "cpssDxChBrgEgrFltVlanPortAccessModeGet: %d", dev);

        UTF_VERIFY_EQUAL1_STRING_MAC(fltTabAccessMode, fltTabAccessModeGet,
                   "get another fltTabAccessMode than was set: %d", dev);

        /*
            Call with fltTabAccessMode
                              [CPSS_DXCH_BRG_EGR_FLT_VLAN_PORT_ACCESS_MODE_2K_E]
        */
        fltTabAccessMode = CPSS_DXCH_BRG_EGR_FLT_VLAN_PORT_ACCESS_MODE_2K_E;

        st = cpssDxChBrgEgrFltVlanPortAccessModeSet(dev, fltTabAccessMode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, fltTabAccessMode);


        /* Verifying values */
        st = cpssDxChBrgEgrFltVlanPortAccessModeGet(dev, &fltTabAccessModeGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                   "cpssDxChBrgEgrFltVlanPortAccessModeGet: %d", dev);

        UTF_VERIFY_EQUAL1_STRING_MAC(fltTabAccessMode, fltTabAccessModeGet,
                   "get another fltTabAccessMode than was set: %d", dev);

        /*
            Call with fltTabAccessMode
                              [CPSS_DXCH_BRG_EGR_FLT_VLAN_PORT_ACCESS_MODE_4K_E]
        */
        fltTabAccessMode = CPSS_DXCH_BRG_EGR_FLT_VLAN_PORT_ACCESS_MODE_4K_E;

        st = cpssDxChBrgEgrFltVlanPortAccessModeSet(dev, fltTabAccessMode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, fltTabAccessMode);


        /* Verifying values */
        st = cpssDxChBrgEgrFltVlanPortAccessModeGet(dev, &fltTabAccessModeGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                   "cpssDxChBrgEgrFltVlanPortAccessModeGet: %d", dev);

        UTF_VERIFY_EQUAL1_STRING_MAC(fltTabAccessMode, fltTabAccessModeGet,
                   "get another fltTabAccessMode than was set: %d", dev);

        /*
            Call with fltTabAccessMode
                              [CPSS_DXCH_BRG_EGR_FLT_VLAN_PORT_ACCESS_MODE_8K_E]
        */
        fltTabAccessMode = CPSS_DXCH_BRG_EGR_FLT_VLAN_PORT_ACCESS_MODE_8K_E;

        st = cpssDxChBrgEgrFltVlanPortAccessModeSet(dev, fltTabAccessMode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, fltTabAccessMode);


        /* Verifying values */
        st = cpssDxChBrgEgrFltVlanPortAccessModeGet(dev, &fltTabAccessModeGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                   "cpssDxChBrgEgrFltVlanPortAccessModeGet: %d", dev);

        UTF_VERIFY_EQUAL1_STRING_MAC(fltTabAccessMode, fltTabAccessModeGet,
                   "get another fltTabAccessMode than was set: %d", dev);

        /*
            Call with fltTabAccessMode
                             [CPSS_DXCH_BRG_EGR_FLT_VLAN_PORT_ACCESS_MODE_16K_E]
        */
        fltTabAccessMode = CPSS_DXCH_BRG_EGR_FLT_VLAN_PORT_ACCESS_MODE_16K_E;

        st = cpssDxChBrgEgrFltVlanPortAccessModeSet(dev, fltTabAccessMode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, fltTabAccessMode);


        /* Verifying values */
        st = cpssDxChBrgEgrFltVlanPortAccessModeGet(dev, &fltTabAccessModeGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                   "cpssDxChBrgEgrFltVlanPortAccessModeGet: %d", dev);

        UTF_VERIFY_EQUAL1_STRING_MAC(fltTabAccessMode, fltTabAccessModeGet,
                   "get another fltTabAccessMode than was set: %d", dev);

        /*
            Call with fltTabAccessMode
                             [CPSS_DXCH_BRG_EGR_FLT_VLAN_PORT_ACCESS_MODE_32K_E]
        */
        fltTabAccessMode = CPSS_DXCH_BRG_EGR_FLT_VLAN_PORT_ACCESS_MODE_32K_E;

        st = cpssDxChBrgEgrFltVlanPortAccessModeSet(dev, fltTabAccessMode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, fltTabAccessMode);

        /*
            1.2. Call cpssDxChBrgEgrFltVlanPortAccessModeGet with
                 non-NULL fltTabAccessMode.
            Expected: GT_OK and the same fltTabAccessMode.
        */
        st = cpssDxChBrgEgrFltVlanPortAccessModeGet(dev, &fltTabAccessModeGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                          "cpssDxChBrgEgrFltVlanPortAccessModeGet: %d", dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(fltTabAccessMode, fltTabAccessModeGet,
                          "get another fltTabAccessMode than was set: %d", dev);

        /*
            1.3. Call function with out of range fltTabAccessMode
            Expected: GT_BAD_PARAM.
        */

        UTF_ENUMS_CHECK_MAC(cpssDxChBrgEgrFltVlanPortAccessModeSet
                            (dev, fltTabAccessMode),
                            fltTabAccessMode);
    }

    /* restore correct values*/
    fltTabAccessMode = CPSS_DXCH_BRG_EGR_FLT_VLAN_PORT_ACCESS_MODE_256_E;

    /*
      2. For not-active devices and devices from non-applicable family
         check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgEgrFltVlanPortAccessModeSet(dev, fltTabAccessMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgEgrFltVlanPortAccessModeSet(dev, fltTabAccessMode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgEgrFltVlanPortAccessModeGet
(
    IN  GT_U8                                                              devNum,
    OUT CPSS_DXCH_BRG_EGR_FLT_VLAN_PORT_ACCESS_MODE_ENT    *fltTabAccessModePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgEgrFltVlanPortAccessModeGet)
{
/*
    ITERATE_DEVICES (Bobcat2, Caelum, Bobcat3)
    1.1. Call function with non-NULL fltTabAccessModePtr.
    Expected: GT_OK.
    1.2. Call function with fltTabAccessModePtr [NULL].
    Expected: GT_BAD_PTR.

*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;
    GT_U32      notAppFamilyBmp = 0;
    CPSS_DXCH_BRG_EGR_FLT_VLAN_PORT_ACCESS_MODE_ENT fltTabAccessModePtr =
                              CPSS_DXCH_BRG_EGR_FLT_VLAN_PORT_ACCESS_MODE_256_E;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with non-NULL fltTabAccessModePtr.
            Expected: GT_OK.
        */

        st = cpssDxChBrgEgrFltVlanPortAccessModeGet(dev, &fltTabAccessModePtr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, fltTabAccessModePtr);

        /*
            1.2. Call function with fltTabAccessModePtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChBrgEgrFltVlanPortAccessModeGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,
                                     "%d, fltTabAccessModePtr = NULL", dev);
    }

    /*
        2. For not-active devices and devices from non-applicable family
           check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgEgrFltVlanPortAccessModeGet(dev, &fltTabAccessModePtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgEgrFltVlanPortAccessModeGet(dev, &fltTabAccessModePtr);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgEgrFltVlanPortVidMappingSet
(
    IN  GT_U8            devNum,
    IN  GT_U32           vid,
    IN  GT_U32           vidIndex
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgEgrFltVlanPortVidMappingSet)
{
/*
    ITERATE_DEVICES (Bobcat2, Caelum, Bobcat3)
    1.1. Call function with
                       vid [0 / 0xA5A / UTF_CPSS_PP_MAX_VLAN_NUM_CNS(dev)-1] and
                       vidIndex [0 / 0xA5A / BIT_12-1].
    Expected: GT_OK.
    1.2. Call cpssDxChBrgEgrFltVlanPortVidMappingGet with not NULL vidIndexPtr
    Expected: GT_OK and same vidIndex
    1.3. Call function with out of range
                       vid [UTF_CPSS_PP_MAX_VLAN_NUM_CNS(dev)] and vidIndex [0].
    Expected: NOT GT_OK.
    1.4. Call function with out of range vid [0] and vidIndex [BIT_12].
    Expected: NOT GT_OK.
*/
    GT_STATUS      st  = GT_OK;
    GT_U8          dev;
    GT_U32         notAppFamilyBmp = 0;
    GT_U32         vid = 0;
    GT_U32         vidIndex    = 0;
    GT_U32         vidIndexGet = 0;


    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with
                       vid [0 / 0xA5A / UTF_CPSS_PP_MAX_VLAN_NUM_CNS(dev)-1] and
                       vidIndex [0 / 0xA5A / BIT_12-1].
            Expected: GT_OK.
        */

        /* Call with vid [0] and vidIndex [0] */
        vid      = 0;
        vidIndex = 0;
        st = cpssDxChBrgEgrFltVlanPortVidMappingSet(dev, vid, vidIndex);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, vid);

        /* verify values */
        st = cpssDxChBrgEgrFltVlanPortVidMappingGet(dev, vid, &vidIndexGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                         "cpssDxChBrgEgrFltVlanPortVidMappingGet: %d, %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(vidIndex, vidIndexGet,
                              "got another vidIndex than was set: %d, %d", dev);

        /* Call with vid [0xA5A] and vidIndex [0xA5A] */
        vid      = 0xA5A;
        vidIndex = 0xA5A;
        st = cpssDxChBrgEgrFltVlanPortVidMappingSet(dev, vid, vidIndex);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, vid);

        /* verify values */
        st = cpssDxChBrgEgrFltVlanPortVidMappingGet(dev, vid, &vidIndexGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                         "cpssDxChBrgEgrFltVlanPortVidMappingGet: %d, %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(vidIndex, vidIndexGet,
                              "got another vidIndex than was set: %d, %d", dev);

        /* Call with vid [BIT_12-1] and
                     vidIndex [UTF_CPSS_PP_MAX_VLAN_NUM_CNS(dev)-1] */
        vid      = UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev)-1;
        vidIndex = BIT_12-1;
        st = cpssDxChBrgEgrFltVlanPortVidMappingSet(dev, vid, vidIndex);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, vid);

        /*
           1.2. Call cpssDxChBrgEgrFltVlanPortVidMappingGet with
                  not NULL vidIndexPtr
           Expected: GT_OK
        */
        st = cpssDxChBrgEgrFltVlanPortVidMappingGet(dev, vid, &vidIndexGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                         "cpssDxChBrgEgrFltVlanPortVidMappingGet: %d, %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(vidIndex, vidIndexGet,
                              "got another vidIndex than was set: %d, %d", dev);

        /*
            1.3. Call function with out of range
                                     vid [UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev)] and
                                     vidIndex [0].
            Expected: NOT GT_OK.
        */

        vid      = UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev);
        vidIndex = 0;
        st = cpssDxChBrgEgrFltVlanPortVidMappingSet(dev, vid, vidIndex);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, vid);

        vid = 0;

        /*
            1.4. Call function with out of range vid [0] and
                                                 vidIndex [BIT_12].
            Expected: NOT GT_OK.
        */

        vid      = 0;
        vidIndex = BIT_12;
        st = cpssDxChBrgEgrFltVlanPortVidMappingSet(dev, vid, vidIndex);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, vid);

        vidIndex = 0;
    }

    /* restore correct values*/
    vidIndex = 0;
    vid      = 0;

    /*
        2. For not-active devices and devices from non-applicable family
           check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgEgrFltVlanPortVidMappingSet(dev, vid, vidIndex);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgEgrFltVlanPortVidMappingSet(dev, vid, vidIndex);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgEgrFltVlanPortVidMappingGet
(
    IN  GT_U8            devNum,
    IN  GT_U32           vid,
    OUT GT_U32           *vidIndexPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgEgrFltVlanPortVidMappingGet)
{
/*
    ITERATE_DEVICES (Bobcat2, Caelum, Bobcat3)
    1.1. Call function with
                       vid [0 / 0xA5A / UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev)-1] and
                       not NULL vidIndexPtr.
    Expected: GT_OK.
    1.2. Call function with out of range
                                     vid [UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev)] and
                                     not NULL vidIndexPtr.
    Expected: NOT GT_OK.
    1.3. Call with vidIndexPtr [NULL] and vid [0].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS      st  = GT_OK;
    GT_U8          dev;
    GT_U32         notAppFamilyBmp = 0;
    GT_U32         vid = 0;
    GT_U32         vidIndex = 0;


    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with
                       vid [0 / 0xA5A / UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev)-1] and
                       not NULL vidIndexPtr.
            Expected: GT_OK.
        */

        /* Call with vid [0] */
        vid = 0;
        st = cpssDxChBrgEgrFltVlanPortVidMappingGet(dev, vid, &vidIndex);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, vid);

        /* Call with vid [0xA5A] */
        vid = 0xA5A;
        st = cpssDxChBrgEgrFltVlanPortVidMappingGet(dev, vid, &vidIndex);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, vid);

        /* Call with vid [UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev)-1] */
        vid = UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev)-1;
        st = cpssDxChBrgEgrFltVlanPortVidMappingGet(dev, vid, &vidIndex);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, vid);

        /*
            1.2. Call function with out of range
                                     vid [UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev)] and
                                     not NULL vidIndexPtr.
            Expected: NOT GT_OK.
        */

        vid = UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev);
        st = cpssDxChBrgEgrFltVlanPortVidMappingGet(dev, vid, &vidIndex);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, vid);

        vid = 0;

        /*
            1.3. Call with vidIndexPtr [NULL] and vid [0].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChBrgEgrFltVlanPortVidMappingGet(dev, vid, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,
                                             "%d, %d, vidIndexPtr = NULL", dev);
    }
    /* restore correct values*/
    vid = 0;

    /*
        2. For not-active devices and devices from non-applicable family
           check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgEgrFltVlanPortVidMappingGet(dev, vid, &vidIndex);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgEgrFltVlanPortVidMappingGet(dev, vid, &vidIndex);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
(
    GT_U8                    devNum,
    GT_BOOL                  enable,
    GT_U32                   meshIdOffset,
    GT_U32                   meshIdSize
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgEgrMeshIdConfigurationSet)
{
/*
    ITERATE_DEVICES (Bobcat2, Caelum, Bobcat3)
    1.1. Call function with valid values of enable[GT_TRUE/GT_TRUE/GT_TRUE],
                                            meshIdOffset [0 / 5 / 11],
                                            meshIdSize [1 / 2 / 4]
    Expected: GT_OK.
    1.2. Call function with out of range meshIdOffset [12] and other parameters
         same as in 1.1
    Expected: GT_BAD_PARAM.
    1.3. Call function with out of range meshIdSize [5] and other parameters
         same as in 1.1
    Expected: GT_BAD_PARAM.
*/
    GT_BOOL  st              = GT_FALSE;
    GT_U8    devNum          = 0;
    GT_BOOL  enable          = GT_FALSE;
    GT_U32   meshIdOffset    = 0;
    GT_U32   meshIdSize      = 0;
    GT_BOOL  enableGet       = GT_FALSE;
    GT_U32   meshIdOffsetGet = 0;
    GT_U32   meshIdSizeGet   = 0;

    GT_U32   notAppFamilyBmp;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /*
            1.1. Call function with valid values of
                 enable[GT_TRUE/GT_TRUE/GT_TRUE],
                 meshIdOffset [0 / 5 / 11],
                 eshIdSize [1 / 2 / 4]
            Expected: GT_OK.
        */

        /*
            1.1.1 Call with enable[GT_TRUE], meshIdOffset [0], meshIdSize [1]
            Expected: GT_OK
        */
        enable       = GT_TRUE;
        meshIdOffset = 0;
        meshIdSize   = 1;
        st = cpssDxChBrgEgrMeshIdConfigurationSet(devNum, enable, meshIdOffset,
                                                  meshIdSize);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

        /*
            1.1.1 Call cpssDxChBrgEgrMeshIdConfigurationGet
            Expected: GT_OK and values the same it was set
        */
        st = cpssDxChBrgEgrMeshIdConfigurationGet(devNum, &enableGet,
                                                  &meshIdOffsetGet,
                                                  &meshIdSizeGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                                    "cpssDxChBrgEgrMeshIdConfigurationGet: %d",
                                    devNum);
        UTF_VERIFY_EQUAL1_STRING_MAC(meshIdOffset, meshIdOffsetGet,
                                    "cpssDxChBrgEgrMeshIdConfigurationGet: %d",
                                    devNum);
        UTF_VERIFY_EQUAL1_STRING_MAC(meshIdSize, meshIdSizeGet,
                                    "cpssDxChBrgEgrMeshIdConfigurationGet: %d",
                                    devNum);

        /*
            1.1.2 Call with enable[GT_TRUE], meshIdOffset [5], meshIdSize [2]
            Expected: GT_OK
        */
        meshIdOffset = 5;
        meshIdSize   = 2;
        st = cpssDxChBrgEgrMeshIdConfigurationSet(devNum, enable, meshIdOffset,
                                                  meshIdSize);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

        /*
            1.1.2 Call cpssDxChBrgEgrMeshIdConfigurationGet
            Expected: GT_OK and values the same it was set
        */
        st = cpssDxChBrgEgrMeshIdConfigurationGet(devNum, &enableGet,
                                                  &meshIdOffsetGet,
                                                  &meshIdSizeGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                                    "cpssDxChBrgEgrMeshIdConfigurationGet: %d",
                                    devNum);
        UTF_VERIFY_EQUAL1_STRING_MAC(meshIdOffset, meshIdOffsetGet,
                                    "cpssDxChBrgEgrMeshIdConfigurationGet: %d",
                                    devNum);
        UTF_VERIFY_EQUAL1_STRING_MAC(meshIdSize, meshIdSizeGet,
                                    "cpssDxChBrgEgrMeshIdConfigurationGet: %d",
                                    devNum);

        /*
            1.1.3 Call with enable[GT_TRUE], meshIdOffset [11], meshIdSize [4]
            Expected: GT_OK
        */
        meshIdOffset = 11;
        meshIdSize   = 4;
        st = cpssDxChBrgEgrMeshIdConfigurationSet(devNum, enable, meshIdOffset,
                                                  meshIdSize);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

        /*
            1.1.3 Call cpssDxChBrgEgrMeshIdConfigurationGet
            Expected: GT_OK and values the same it was set
        */
        st = cpssDxChBrgEgrMeshIdConfigurationGet(devNum, &enableGet,
                                                  &meshIdOffsetGet,
                                                  &meshIdSizeGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                                    "cpssDxChBrgEgrMeshIdConfigurationGet: %d",
                                    devNum);
        UTF_VERIFY_EQUAL1_STRING_MAC(meshIdOffset, meshIdOffsetGet,
                                    "cpssDxChBrgEgrMeshIdConfigurationGet: %d",
                                    devNum);
        UTF_VERIFY_EQUAL1_STRING_MAC(meshIdSize, meshIdSizeGet,
                                    "cpssDxChBrgEgrMeshIdConfigurationGet: %d",
                                    devNum);

        /* restore correct values */
        enable       = GT_TRUE;
        meshIdOffset = 0;
        meshIdSize   = 1;

        /*
            1.2. Call function with out of range meshIdOffset [12] and other
                 parameters same as in 1.1
            Expected: GT_OUT_OF_RANGE
        */
        meshIdOffset = 12;
        st = cpssDxChBrgEgrMeshIdConfigurationSet(devNum, enable, meshIdOffset,
                                                  meshIdSize);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, devNum);

        /*
            1.3. Call function with out of range meshIdSize [5] and other
                 parameters same as in 1.1
            Expected: GT_OUT_OF_RANGE
        */
        meshIdOffset = 0;
        meshIdSize   = 5;
        st = cpssDxChBrgEgrMeshIdConfigurationSet(devNum, enable, meshIdOffset,
                                                  meshIdSize);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, devNum);
    }

    /* restore correct values*/
    enable       = GT_TRUE;
    meshIdOffset = 0;
    meshIdSize   = 1;

    /*
        2. For not-active devices and devices from non-applicable family
           check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChBrgEgrMeshIdConfigurationSet(devNum, enable, meshIdOffset,
                                                  meshIdSize);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3. Call function with out of bound value for device id. */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgEgrMeshIdConfigurationSet(devNum, enable, meshIdOffset,
                                              meshIdSize);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgEgrMeshIdConfigurationGet
(
    IN  GT_U8                    devNum,
    OUT GT_BOOL                  *enablePtr,
    OUT GT_U32                   *meshIdOffsetPtr,
    OUT GT_U32                   *meshIdSizePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgEgrMeshIdConfigurationGet)
{
/*
    ITERATE_DEVICES (Bobcat2, Caelum, Bobcat3)
    1.1. Call function with valid values of enablePtr [non-NULL]
                                            meshIdOffsetPtr [non-NULL]
                                            meshIdSizePtr [non-NULL]
    Expected: GT_OK.
    1.2. Call function with invalid enablePtr [NULL] and other
         parameters same as in 1.1
    Expected: GT_BAD_PTR.
    1.3. Call function with invalid meshIdOffset [NULL] and other
         parameters same as in 1.1
    Expected: GT_BAD_PTR.
    1.4. Call function with invalid meshIdSize [NULL] and other
         parameters same as in 1.1
    Expected: GT_BAD_PTR.
*/
    GT_U8   devNum          = 0;
    GT_BOOL enable          = GT_FALSE;
    GT_U32  meshIdOffset    = 0;
    GT_U32  meshIdSize      = 0;
    GT_BOOL st              = GT_OK;
    GT_BOOL notAppFamilyBmp;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /*
            1.1. Call function with valid values of enablePtr [non-NULL]
                                                    meshIdOffsetPtr [non-NULL]
                                                    meshIdSizePtr [non-NULL]
            Expected: GT_OK.
        */
        enable       = GT_FALSE;
        meshIdOffset = 0;
        meshIdSize   = 1;
        st = cpssDxChBrgEgrMeshIdConfigurationGet(devNum, &enable,
                                                  &meshIdOffset, &meshIdSize);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

        /*
            1.2. Call function with invalid enablePtr [NULL] and other
                 parameters same as in 1.1
           Expected: GT_BAD_PTR
        */
        st = cpssDxChBrgEgrMeshIdConfigurationGet(devNum, NULL, &meshIdOffset,
                                                  &meshIdSize);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, devNum);

        /*
           1.3. Call function with invalid meshIdOffsetPtr [NULL] and other
                parameters same as in 1.1
           Expected: GT_BAD_PTR
        */
        st = cpssDxChBrgEgrMeshIdConfigurationGet(devNum, &enable, NULL,
                                                  &meshIdSize);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, devNum);

        /*
           1.4. Call function with invalid meshIdSizePtr [NULL] and other
                parameters same as in 1.1
           Expected: GT_BAD_PTR
        */
        st = cpssDxChBrgEgrMeshIdConfigurationGet(devNum, &enable,
                                                  &meshIdOffset, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, devNum);
    }

    /*
        2. For not-active devices and devices from non-applicable family
           check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChBrgEgrMeshIdConfigurationGet(devNum, &enable,
                                                  &meshIdOffset, &meshIdSize);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3. Call function with out of bound value for device id. */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgEgrMeshIdConfigurationGet(devNum, &enable, &meshIdOffset,
                                              &meshIdSize);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
(
    GT_U8           devNum,
    GT_PORT_NUM     portNum,
    GT_U32          meshId
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgEgrPortMeshIdSet)
{
/*
    ITERATE_DEVICES (Bobcat2, Caelum, Bobcat3)
    1.1. Call function with valid values of meshId [0 / 7 / 15]
    Expected: GT_OK and values the same it was set.
    1.2.  Call function with out of range meshId [16]
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS       st              = GT_OK;
    GT_U8           devNum          = 0;
    GT_PORT_NUM     portNum         = 0;
    GT_U32          meshId          = 0;
    GT_U32          meshIdGet       = 1;
    GT_BOOL         notAppFamilyBmp = 0;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        st = prvUtfNextVirtPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&portNum, GT_TRUE))
        {
            /*
                1.1 Call function with valid values of meshId [0 / 7 / 15]
                Expected: GT_OK and values the same it was set.
            */

            /*
                1.1.1 Call function with meshId [0]
                Expected: GT_OK.
            */
            meshId = 0;
            st = cpssDxChBrgEgrPortMeshIdSet(devNum, portNum, meshId);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portNum, meshId);

            /*
                1.1.1. Call cpssDxChBrgEgrPortMeshIdGet
                Expected: GT_OK and the same meshId as it was set
            */
            st = cpssDxChBrgEgrPortMeshIdGet(devNum, portNum, &meshIdGet);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portNum, meshIdGet);

            UTF_VERIFY_EQUAL2_STRING_MAC(meshId, meshIdGet,
                                         "cpssDxChBrgEgrPortMeshIdGet: %d, %d",
                                         devNum, portNum);

            /*
                1.1.2 Call function with meshId [7]
                Expected: GT_OK.
            */
            meshId = 7;
            st = cpssDxChBrgEgrPortMeshIdSet(devNum, portNum, meshId);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portNum, meshId);
            /*
                1.1.2. Call cpssDxChBrgEgrPortMeshIdGet
                Expected: GT_OK and the same meshId as it was set
            */
            st = cpssDxChBrgEgrPortMeshIdGet(devNum, portNum, &meshIdGet);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portNum, meshIdGet);

            UTF_VERIFY_EQUAL2_STRING_MAC(meshId, meshIdGet,
                                         "cpssDxChBrgEgrPortMeshIdGet: %d, %d",
                                         devNum, portNum);

            /*
                1.1.3 Call function with meshId [15]
                Expected: GT_OK.
            */
            meshId = 15;
            st = cpssDxChBrgEgrPortMeshIdSet(devNum, portNum, meshId);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portNum, meshId);
            /*
                1.1.3. Call cpssDxChBrgEgrPortMeshIdGet
                Expected: GT_OK and the same meshId as it was set
            */
            st = cpssDxChBrgEgrPortMeshIdGet(devNum, portNum, &meshIdGet);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portNum, meshIdGet);

            UTF_VERIFY_EQUAL2_STRING_MAC(meshId, meshIdGet,
                                         "cpssDxChBrgEgrPortMeshIdGet: %d, %d",
                                         devNum, portNum);

            /*
                1.2 Call function with out of range meshId [16]
                Expected: GT_OUT_OF_RANGE.
            */
            meshId = 16;
            st = cpssDxChBrgEgrPortMeshIdSet(devNum, portNum, meshId);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OUT_OF_RANGE, st, devNum, portNum,
                                        meshId);
        }
        st = prvUtfNextVirtPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* restore correct values */
        meshId = 0;

        /*
           1.3. For all active devices go over all non available physical ports.
        */
        while (GT_OK == prvUtfNextVirtPortGet(&portNum, GT_FALSE))
        {
            /* 1.3.1. Call function for each non-active port */
            st = cpssDxChBrgEgrPortMeshIdSet(devNum, portNum, meshId);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum,
                                        meshId);
        }

        /*
           1.4. For active device check that function returns GT_BAD_PARAM
                for out of bound value for port number.
        */
        portNum = UTF_CPSS_PP_MAX_PORT_NUM_CNS(devNum);

        st = cpssDxChBrgEgrPortMeshIdSet(devNum, portNum, meshId);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum, meshId);
    }

    /* 2. For not active devices check that function returns non GT_OK. */
    portNum = BRG_EGR_FLT_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChBrgEgrPortMeshIdSet(devNum, portNum, meshId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3.Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgEgrPortMeshIdSet(devNum, portNum, meshId);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
(
    GT_U8           devNum,
    GT_PORT_NUM     portNum,
    GT_U32         *meshIdPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgEgrPortMeshIdGet)
{
/*
    ITERATE_DEVICES (Bobcat2, Caelum, Bobcat3)
    1.1.1. Call function with valid value of meshId [non-NULL]
    Expected: GT_OK.
    1.1.2. Call function with invalid value of meshId [NULL]
    Expected: GT_BAD_PTR.
*/
    GT_STATUS       st              = GT_OK;
    GT_U8           devNum          = 0;
    GT_PORT_NUM     portNum         = 0;
    GT_U32          meshId          = 0;
    GT_BOOL         notAppFamilyBmp = 0;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        st = prvUtfNextVirtPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&portNum, GT_TRUE))
        {
            /*
                1.1.1. Call function with valid value of meshIdPtr [non-NULL]
                Expected: GT_OK.
            */
            st = cpssDxChBrgEgrPortMeshIdGet(devNum, portNum, &meshId);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portNum, meshId);
            /*
                1.1.2. Call function with invalid value of meshIdPtr [NULL]
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChBrgEgrPortMeshIdGet(devNum, portNum, NULL);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, devNum, portNum);
        }
        st = prvUtfNextVirtPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /*
           1.2. For all active devices go over all non available physical ports.
        */
        while (GT_OK == prvUtfNextVirtPortGet(&portNum, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChBrgEgrPortMeshIdGet(devNum, portNum, &meshId);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum,
                                        meshId);
        }

        /*
           1.3. For active device check that function returns GT_BAD_PARAM
                for out of bound value for port number.
        */
        portNum = UTF_CPSS_PP_MAX_PORT_NUM_CNS(devNum);
        st = cpssDxChBrgEgrPortMeshIdGet(devNum, portNum, &meshId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);

        /*
            1.4. For active device check that function returns GT_BAD_PARAM
                 for CPU port number.
        */
        portNum = CPSS_CPU_PORT_NUM_CNS;
        st = cpssDxChBrgEgrPortMeshIdGet(devNum, portNum, &meshId);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portNum, meshId);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    portNum = BRG_EGR_FLT_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChBrgEgrPortMeshIdGet(devNum, portNum, &meshId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3.Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgEgrPortMeshIdGet(devNum, portNum, &meshId);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgEgrFltPortVid1FilteringEnableSet
(
    IN  GT_U8            devNum,
    IN  GT_PORT_NUM      portNum,
    IN  GT_BOOL          enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgEgrFltPortVid1FilteringEnableSet)
{
/*
    ITERATE_DEVICES (Bobcat2, Caelum, Bobcat3)
    1. Call with enable [GT_FALSE / GT_TRUE] and check an assigned valued
       with cpssDxChBrgEgrFltPortVid1FilteringEnableGet.
       Expected: GT_OK.
    2. Call with non available port number.       Expected: GT_BAD_PARAM.
    3. Call with device number out of applicable range.
       Expected: GT_NOT_APPLICABLE_DEVICE
*/
    GT_U8          dev;
    GT_PORT_NUM    port;
    GT_U32         notAppFamilyBmp = 0;
    GT_BOOL        enable          = GT_FALSE;
    GT_BOOL        enableGet       = GT_FALSE;
    GT_32          i;
    GT_STATUS      rc;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        rc = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

        if (UTF_CPSS_SIP5_0_ONLY_CHECK_MAC(dev))
        {
            /* not supported in  Bobcat2 a0 */
            rc = cpssDxChBrgEgrFltPortVid1FilteringEnableSet(dev, port, GT_FALSE);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, rc, dev);
            continue;
        }

        /* 1. go over all available virtual ports and call with
           enable [GT_TRUE / GT_FALSE]. Expected: GT_OK */
        while (GT_OK == prvUtfNextVirtPortGet(&port, GT_TRUE))
        {
            for (i = 0; i < 2; i++) {
                /* GT_TRUE - on 1st iteration, GT_FALSE - on 2nd */
                enable = (0==i ? GT_TRUE : GT_FALSE);

                /* assign value*/
                rc = cpssDxChBrgEgrFltPortVid1FilteringEnableSet(dev, port, enable);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, rc, dev, port, enable);

                /* check the assigned value*/
                rc = cpssDxChBrgEgrFltPortVid1FilteringEnableGet(dev, port, &enableGet);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                          "cpssDxChBrgEgrFltPortVid1FilteringEnableGet: %d, %d",
                          dev, port);
                UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                          "got another enable than was set: %d, %d", dev, port);
            }
        }

        enable = GT_FALSE;

        rc = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

        /* 2.1. For all active devices go over all non available ports */
        while (GT_OK == prvUtfNextVirtPortGet(&port, GT_FALSE))
        {
            rc = cpssDxChBrgEgrFltPortVid1FilteringEnableSet(dev, port, enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, rc, dev, port);
        }

        /* 2.2. For active device check that function returns GT_BAD_PARAM
               for out of bound value for port number. */
        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        rc = cpssDxChBrgEgrFltPortVid1FilteringEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, rc, dev, port);

        /* 2.3. For active device check that function
           returns GT_OK for CPU port number. */
        port = CPSS_CPU_PORT_NUM_CNS;

        rc = cpssDxChBrgEgrFltPortVid1FilteringEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, rc, dev, port);
    }

    enable = GT_FALSE;
    port   = 0;

    /* 3. For not-active devices and devices from non-applicable family
       check that function returns GT_NOT_APPLICABLE_DEVICE */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 3.1. go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        rc = cpssDxChBrgEgrFltPortVid1FilteringEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, rc, dev);
    }

    /* 3.2. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    rc = cpssDxChBrgEgrFltPortVid1FilteringEnableSet(dev, port, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, rc, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgEgrFltPortVid1FilteringEnableGet
(
    IN  GT_U8          devNum,
    IN  GT_PORT_NUM    port,
    OUT GT_BOOL        *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgEgrFltPortVid1FilteringEnableGet)
{
/*
    ITERATE_DEVICES_VIRT_PORTS (Bobcat2, Caelum, Bobcat3)
    1.1. Call with not NULL enablePtr.     Expected: GT_OK.
    1.2. Call with enablePtr [NULL].       Expected: GT_BAD_PTR.
    2. Call for non available port number. Expected: GT_BAD_PTR.
    3.1. Call with non active device id.   Expected: GT_NOT_APPLICABLE_DEVICE
    3.2. Call with device id out of range. Expected: GT_BAD_PARAM
*/
    GT_U8          dev;
    GT_PORT_NUM    port;
    GT_U32         notAppFamilyBmp = 0;
    GT_BOOL        enable          = GT_FALSE;
    GT_STATUS      rc              = GT_OK;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        rc = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

        if (UTF_CPSS_SIP5_0_ONLY_CHECK_MAC(dev))
        {
            /* not supported in  Bobcat2 a0 */
            rc = cpssDxChBrgEgrFltPortVid1FilteringEnableGet(dev, port, &enable);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, rc, dev);
            continue;
        }

        /* 1. For all active devices go over all available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&port, GT_TRUE))
        {
            /* 1.1. Call with not NULL enablePtr. Expected: GT_OK. */
            rc = cpssDxChBrgEgrFltPortVid1FilteringEnableGet(dev, port,&enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, rc, dev, port);

            /* 1.2. Call with enablePtr [NULL]. Expected: GT_BAD_PTR. */
            rc = cpssDxChBrgEgrFltPortVid1FilteringEnableGet(dev, port, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, rc,
                                         "%d, %d, enablePtr = NULL", dev, port);
        }

        rc = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

        /* 2. For all active devices go over all non available ports */
        while (GT_OK == prvUtfNextVirtPortGet(&port, GT_FALSE))
        {
            /* 2.1. call function for each non-active port */
            rc = cpssDxChBrgEgrFltPortVid1FilteringEnableGet(dev, port,&enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, rc, dev, port);
        }

        /* 2.2. check that function returns GT_BAD_PARAM for out of bound value for
           port number */
        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        rc = cpssDxChBrgEgrFltPortVid1FilteringEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, rc, dev, port);

        /* 2.3. For active device check that function returns GT_OK for
           CPU port number */
        port = CPSS_CPU_PORT_NUM_CNS;

        rc = cpssDxChBrgEgrFltPortVid1FilteringEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, rc, dev, port);
    }

    port = 0;

    /* 3.1. For not-active devices and devices from non-applicable family
       check that function returns GT_NOT_APPLICABLE_DEVICE */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        rc = cpssDxChBrgEgrFltPortVid1FilteringEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, rc, dev);
    }

    /* 3.2. Call function with the device id value out of range */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    rc = cpssDxChBrgEgrFltPortVid1FilteringEnableGet(dev, port, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, rc, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgEgrFltPortVid1Set
(
    IN  GT_U8            devNum,
    IN  GT_PORT_NUM      portNum,
    IN  GT_U16           vid1
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgEgrFltPortVid1Set)
{
/*
    ITERATE_DEVICES (Bobcat2, Caelum, Bobcat3)
    1.1. Call function with valid values of vid1 (1, _2K, _4K-1)
    Expected: GT_OK and values the same it was set.
    1.2. Call function with out of range vid1 (_4K)
    Expected: GT_OUT_OF_RANGE.
    2. Call for non available port number. Expected: GT_BAD_PTR.
    3.1. Call with non active device id.   Expected: GT_NOT_APPLICABLE_DEVICE
    3.2. Call with device id out of range. Expected: GT_BAD_PARAM
*/
    GT_U8           devNum          = 0;
    GT_PORT_NUM     portNum         = 0;
    GT_BOOL         notAppFamilyBmp = 0;
    GT_U16          vid1Arr[3]      = {_2K, _4K-1, 0};
    GT_U16          vid1Get         = 0;
    GT_U16          vid1;
    GT_U32          i =0;
    GT_STATUS       rc =GT_OK;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        rc = prvUtfNextVirtPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

        if (UTF_CPSS_SIP5_0_ONLY_CHECK_MAC(devNum))
        {
            /* not supported in  Bobcat2 a0 */
            rc = cpssDxChBrgEgrFltPortVid1Set(devNum, portNum, 0);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, rc, devNum);
            continue;
        }

        /* 1. For all active devices go over all available ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&portNum, GT_TRUE))
        {
            /* 1.1. Call function with valid values of vid1 (1, _2K, _4K-1)
                Expected: GT_OK and values the same it was set. */
            for (i = 0; i < sizeof (vid1Arr)/sizeof(vid1Arr[0]); i++) {
                vid1 = vid1Arr[i];
                rc = cpssDxChBrgEgrFltPortVid1Set(devNum, portNum, vid1);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, rc, devNum, portNum, vid1);

                rc = cpssDxChBrgEgrFltPortVid1Get(devNum, portNum, &vid1Get);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, rc, devNum, portNum, vid1Get);

                UTF_VERIFY_EQUAL2_STRING_MAC(vid1, vid1Get,
                                         "cpssDxChBrgEgrFltPortVid1Get: %d, %d",
                                         devNum, portNum);
            }

            /* 1.2. Call function with out of range vid1 (_4K)
                Expected: GT_OUT_OF_RANGE. */
            vid1 = _4K;
            rc = cpssDxChBrgEgrFltPortVid1Set(devNum, portNum, vid1);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OUT_OF_RANGE, rc,
                                        devNum, portNum, vid1);
        }

        rc = prvUtfNextVirtPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

        vid1 = 0;

        /* 2. for all active devices go over all non available ports */
        while (GT_OK == prvUtfNextVirtPortGet(&portNum, GT_FALSE))
        {
            /* 2.1. Call function for each non-active port */
            rc = cpssDxChBrgEgrFltPortVid1Set(devNum, portNum, vid1);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, rc,
                                        devNum, portNum, vid1);
        }

        /* 2.2. For active device check that function returns GT_BAD_PARAM
           for out of bound value for port number */
        portNum = UTF_CPSS_PP_MAX_PORT_NUM_CNS(devNum);

        rc = cpssDxChBrgEgrFltPortVid1Set(devNum, portNum, vid1);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, rc, devNum, portNum, vid1);
    }

    /* 3. For not active devices check that function returns non GT_OK. */
    portNum = BRG_EGR_FLT_VALID_PHY_PORT_CNS;
    vid1    = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        rc = cpssDxChBrgEgrFltPortVid1Set(devNum, portNum, vid1);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, rc, devNum);
    }

    /* 3.Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    rc = cpssDxChBrgEgrFltPortVid1Set(devNum, portNum, vid1);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, rc, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgEgrFltPortVid1Get
(
    IN  GT_U8            devNum,
    IN  GT_PORT_NUM      portNum,
    OUT GT_U16           *vid1Ptr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgEgrFltPortVid1Get)
{
/*
    ITERATE_DEVICES (Bobcat2, Caelum, Bobcat3)
    1. Call function with not null vid1Ptr. Expected: GT_OK.
    2. Call function with null  vid1Ptr.    Expected: GT_BAD_PTR.
    3. Call for non available port number.  Expected: GT_BAD_PTR.
    4.1. Call with non active device id.    Expected: GT_NOT_APPLICABLE_DEVICE
    4.2. Call with device id out of range.  Expected: GT_BAD_PARAM

*/
    GT_U8           devNum          = 0;
    GT_PORT_NUM     portNum         = 0;
    GT_U16          vid1            = 0;
    GT_BOOL         notAppFamilyBmp = 0;
    GT_STATUS       rc              = GT_OK;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        rc = prvUtfNextVirtPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

        if (UTF_CPSS_SIP5_0_ONLY_CHECK_MAC(devNum))
        {
            /* not supported in  Bobcat2 a0 */
            rc = cpssDxChBrgEgrFltPortVid1Get(devNum, portNum, &vid1);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, rc, devNum);
            continue;
        }

        /* for all active devices go over all available ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&portNum, GT_TRUE))
        {
            /* 1. Call function with not null value of vid1Ptr
                Expected: GT_OK */
            rc = cpssDxChBrgEgrFltPortVid1Get(devNum, portNum, &vid1);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, rc, devNum, portNum, vid1);

            /* 2. Call function with null value of vid1Ptr
               Expected: GT_BAD_PTR */
            rc = cpssDxChBrgEgrFltPortVid1Get(devNum, portNum, NULL);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, rc, devNum, portNum);
        }

        rc = prvUtfNextVirtPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

        /* 3. For all active devices go over all non available ports */
        while (GT_OK == prvUtfNextVirtPortGet(&portNum, GT_FALSE))
        {
            /* 3.1. call function for each non-active port */
            rc = cpssDxChBrgEgrFltPortVid1Get(devNum, portNum, &vid1);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, rc, devNum, portNum);
        }

        /* 3.2. For active device check that function returns GT_BAD_PARAM
           for out of bound value for port number. */
        portNum = UTF_CPSS_PP_MAX_PORT_NUM_CNS(devNum);
        rc = cpssDxChBrgEgrFltPortVid1Get(devNum, portNum, &vid1);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, rc, devNum, portNum);

        /* 3.3. For active device check that function returns GT_OK
           for CPU port number. */
        portNum = CPSS_CPU_PORT_NUM_CNS;
        rc = cpssDxChBrgEgrFltPortVid1Get(devNum, portNum, &vid1);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, rc, devNum, portNum, vid1);
    }

    /* 4. For not active devices check that function returns non GT_OK.*/

    portNum = BRG_EGR_FLT_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* 4.1. Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        rc = cpssDxChBrgEgrFltPortVid1Get(devNum, portNum, &vid1);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, rc, devNum);
    }

    /* 4.2.Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    rc = cpssDxChBrgEgrFltPortVid1Get(devNum, portNum, &vid1);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, rc, devNum);
}


/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChBrgEgrFltVlanEPortFilteringEnableSet)
{

    GT_STATUS      st  = GT_OK;
    GT_U8          dev;
    GT_PORT_NUM    port;
    GT_U32         notAppFamilyBmp = 0;
    GT_BOOL        enable    = GT_FALSE;
    GT_BOOL        enableGet = GT_FALSE;




    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);


    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {

        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);


        /* 1.1. For all active devices go over all available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&port, GT_TRUE))
        {


            /*
                         1.1.1. Call with enable [GT_FALSE / GT_TRUE].
                                   Expected: GT_OK.
                      */

            /* iterate with enable = GT_FALSE */
            enable = GT_FALSE;

            st = cpssDxChBrgEgrFltVlanEPortFilteringEnableSet(dev, port, enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);

            /*
                        1.1.2. Call cpssDxChBrgEgrFltVlanEPortFilteringEnableGet
                                 with not NULL enablePtr
                                 Expected: GT_OK
                     */
            st = cpssDxChBrgEgrFltVlanEPortFilteringEnableGet(dev, port,
                                                             &enableGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                "cpssDxChBrgEgrFltVlanEPortFilteringEnableGet: %d, %d",
                dev, port);
            UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                          "got another enable than was set: %d, %d", dev, port);

            /* iterate with enable = GT_TRUE */
            enable = GT_TRUE;

            st = cpssDxChBrgEgrFltVlanEPortFilteringEnableSet(dev, port, enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);


            st = cpssDxChBrgEgrFltVlanEPortFilteringEnableGet(dev, port,
                                                             &enableGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                "cpssDxChBrgEgrFltVlanEPortFilteringEnableGet: %d, %d",
                dev, port);
            UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                          "got another enable than was set: %d, %d", dev, port);
        }

        /* restore correct values*/
        enable = GT_FALSE;

        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);


        /*
                  1.2. For all active devices go over all non available ports
             */
        while (GT_OK == prvUtfNextVirtPortGet(&port, GT_FALSE))
        {

            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChBrgEgrFltVlanEPortFilteringEnableSet(dev, port, enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /*
                   1.3. For active device check that function returns GT_BAD_PARAM
                   for out of bound value for port number.
              */
        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);


        st = cpssDxChBrgEgrFltVlanEPortFilteringEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /*
                 1.4. For active device check that function returns GT_OK
                 for CPU port number.
              */
        port = CPSS_CPU_PORT_NUM_CNS;


        st = cpssDxChBrgEgrFltVlanEPortFilteringEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    enable = GT_FALSE;
    port   = 0;

    /*
        2. For not-active devices and devices from non-applicable family
           check that function returns GT_BAD_PARAM.
      */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);


    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgEgrFltVlanEPortFilteringEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgEgrFltVlanEPortFilteringEnableSet(dev, port, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

UTF_TEST_CASE_MAC(cpssDxChBrgEgrFltVlanEPortFilteringEnableGet)
{

    GT_STATUS      st  = GT_OK;
    GT_U8          dev;
    GT_PORT_NUM    port;
    GT_U32         notAppFamilyBmp = 0;
    GT_BOOL        enable = GT_FALSE;


    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&port, GT_TRUE))
        {
            /*
                         1.1.1. Call with not NULL enablePtr.
                         Expected: GT_OK.
                      */
            st = cpssDxChBrgEgrFltVlanEPortFilteringEnableGet(dev, port,&enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*
                           1.1.2. Call with enablePtr [NULL].
                           Expected: GT_BAD_PTR.
                      */
            st = cpssDxChBrgEgrFltVlanEPortFilteringEnableGet(dev, port, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st,
                                         "%d, %d, enablePtr = NULL", dev, port);
        }

        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /*
                     1.2. For all active devices go over all non available ports
                */
        while (GT_OK == prvUtfNextVirtPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChBrgEgrFltVlanEPortFilteringEnableGet(dev, port,&enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /*
                      1.3. For active device check that function returns GT_BAD_PARAM
                      for out of bound value for port number.
               */
        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChBrgEgrFltVlanEPortFilteringEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /*
                   1.4. For active device check that function returns GT_OK
                    for CPU port number.
               */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgEgrFltVlanEPortFilteringEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    port = 0;

    /*
        2. For not-active devices and devices from non-applicable family
           check that function returns GT_BAD_PARAM.
       */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgEgrFltVlanEPortFilteringEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgEgrFltVlanEPortFilteringEnableGet(dev, port, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}



/*----------------------------------------------------------------------------*/

/*
 * Configuration of cpssDxChBrgEgrFlt suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssDxChBrgEgrFlt)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgPortEgrFltUnkEnable)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgPortEgrFltUnkEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgPortEgrFltUregMcastEnable)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgPortEgrFltUregMcastEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanEgressFilteringEnable)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanEgressFilteringEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgRoutedUnicastEgressFilteringEnable)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgRoutedUnicastEgressFilteringEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgRoutedSpanEgressFilteringEnable)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgRoutedSpanEgressFilteringEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgPortEgrFltUregBcEnable)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgPortEgrFltUregBcEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgPortEgressMcastLocalEnable)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgPortEgressMcastLocalEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgPortEgrFltIpMcRoutedEnable)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgPortEgrFltIpMcRoutedEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgEgrFltPortLinkEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgEgrFltPortLinkEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgEgrFltVlanPortFilteringEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgEgrFltVlanPortFilteringEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgEgrFltVlanPortVidSelectModeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgEgrFltVlanPortVidSelectModeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgEgrFltVlanPortVidMappingSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgEgrFltVlanPortVidMappingGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgEgrFltVlanPortAccessModeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgEgrFltVlanPortAccessModeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgEgrFltVlanPortMemberSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgEgrFltVlanPortMemberGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgEgrMeshIdConfigurationSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgEgrMeshIdConfigurationGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgEgrPortMeshIdSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgEgrPortMeshIdGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgEgrFltPortVid1FilteringEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgEgrFltPortVid1FilteringEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgEgrFltPortVid1Set)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgEgrFltPortVid1Get)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgEgrFltVlanEPortFilteringEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgEgrFltVlanEPortFilteringEnableGet)
UTF_SUIT_END_TESTS_MAC(cpssDxChBrgEgrFlt)


