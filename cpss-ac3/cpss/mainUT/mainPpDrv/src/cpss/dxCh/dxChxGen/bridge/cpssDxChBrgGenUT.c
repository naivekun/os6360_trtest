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
* @file cpssDxChBrgGenUT.c
*
* @brief Unit tests for cpssDxChBrgGen, that provides
* CPSS DxCh implementation for Bridge Generic APIs.
*
* @version   58
********************************************************************************
*/
/* includes */
/* the define of UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC must come from C files that
   already fixed the types of ports from GT_U8 !

   NOTE: must come before ANY include to H files !!!!
*/
#define UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC

#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgGen.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>

/* Defines */

/* Default valid value for port id */
#define BRG_GEN_VALID_PHY_PORT_CNS  0

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgGenIgmpSnoopEnable
(
    IN GT_U8        dev,
    IN GT_U8        port,
    IN GT_BOOL      status
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgGenIgmpSnoopEnable)
{
/*
    ITERATE_DEVICES_PHY_PORTS (DxChx)
    1.1.1. Call with state [GT_FALSE and GT_TRUE].
    Expected: GT_OK.
    1.1.2. Call cpssDxChBrgGenIgmpSnoopEnableGet.
    Expected: GT_OK and the same value.
*/
    GT_STATUS   st       = GT_OK;

    GT_U8       dev;
    GT_PORT_NUM       port     = BRG_GEN_VALID_PHY_PORT_CNS;
    GT_BOOL     state    = GT_TRUE;
    GT_BOOL     stateGet = GT_FALSE;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextVirtPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call with state [GT_FALSE and GT_TRUE].
                Expected: GT_OK.
            */

            /* Call function with state [GT_FALSE] */
            state = GT_FALSE;

            st = cpssDxChBrgGenIgmpSnoopEnable(dev, port, state);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, state);

            /*
                1.1.2. Call cpssDxChBrgGenIgmpSnoopEnableGet.
                Expected: GT_OK and the same value.
            */

            st = cpssDxChBrgGenIgmpSnoopEnableGet(dev, port, &stateGet);

            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                         "cpssDxChBrgGenIgmpSnoopEnableGet: %d, %d", dev, stateGet);
            /* Verifying value */
            UTF_VERIFY_EQUAL1_STRING_MAC(state, stateGet,
                                         "get another state than was set: %d", dev);

            /* Call function with state [GT_TRUE] */
            state = GT_TRUE;

            st = cpssDxChBrgGenIgmpSnoopEnable(dev, port, state);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, state);

            /*
                1.1.2. Call cpssDxChBrgGenIgmpSnoopEnableGet.
                Expected: GT_OK and the same value.
            */

            st = cpssDxChBrgGenIgmpSnoopEnableGet(dev, port, &stateGet);

            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                         "cpssDxChBrgGenIgmpSnoopEnableGet: %d, %d", dev, stateGet);
            /* Verifying value */
            UTF_VERIFY_EQUAL1_STRING_MAC(state, stateGet,
                                         "get another state than was set: %d", dev);
        }
        state = GT_TRUE;

        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextVirtPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChBrgGenIgmpSnoopEnable(dev, port, state);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */

        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChBrgGenIgmpSnoopEnable(dev, port, state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                     */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgGenIgmpSnoopEnable(dev, port, state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    state = GT_TRUE;

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = BRG_GEN_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgGenIgmpSnoopEnable(dev, port, state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgGenIgmpSnoopEnable(dev, port, state);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgGenIgmpSnoopEnableGet
(
    IN  GT_U8        devNum,
    IN  GT_U8        port,
    OUT GT_BOOL      *statusPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgGenIgmpSnoopEnableGet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (DxChx)
    1.1.1. Call with valid statusPtr.
    Expected: GT_OK.
    1.1.2. Call wrong statusPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st   = GT_OK;

    GT_U8       dev;
    GT_PORT_NUM       port = BRG_GEN_VALID_PHY_PORT_CNS;
    GT_BOOL     state;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextVirtPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call with valid statusPtr.
                Expected: GT_OK.
            */
            st = cpssDxChBrgGenIgmpSnoopEnableGet(dev, port, &state);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, state);

            /*
                1.1.2. Call wrong statusPtr [NULL].
                Expected: GT_OK.
            */
            st = cpssDxChBrgGenIgmpSnoopEnableGet(dev, port, NULL);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PTR, st, dev, port, state);
        }

        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextVirtPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChBrgGenIgmpSnoopEnableGet(dev, port, &state);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */

        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChBrgGenIgmpSnoopEnableGet(dev, port, &state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                     */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgGenIgmpSnoopEnableGet(dev, port, &state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    state = GT_TRUE;

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = BRG_GEN_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgGenIgmpSnoopEnableGet(dev, port, &state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgGenIgmpSnoopEnableGet(dev, port, &state);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgGenDropIpMcEnable
(
    IN GT_U8    dev,
    IN GT_BOOL  state
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgGenDropIpMcEnable)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with state [GT_TRUE and GT_FALSE].
    Expected: GT_OK.
    1.2. Call cpssDxChBrgGenDropIpMcEnableGet
    Expected: GT_OK and same enable.
*/
    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_BOOL     state = GT_TRUE;
    GT_BOOL     stateGet = GT_TRUE;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*  1.1. */
        state = GT_FALSE;

        st = cpssDxChBrgGenDropIpMcEnable(dev, state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, state);

        /*  1.2. */
        st = cpssDxChBrgGenDropIpMcEnableGet(dev, &stateGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        UTF_VERIFY_EQUAL1_STRING_MAC(state, stateGet,
            "get another state than was set: %d", dev);

        /*  1.1. */
        state = GT_TRUE;

        st = cpssDxChBrgGenDropIpMcEnable(dev, state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, state);

        /*  1.2. */
        st = cpssDxChBrgGenDropIpMcEnableGet(dev, &stateGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        UTF_VERIFY_EQUAL1_STRING_MAC(state, stateGet,
            "get another state than was set: %d", dev);
    }

    state = GT_TRUE;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgGenDropIpMcEnable(dev, state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgGenDropIpMcEnable(dev, state);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChBrgGenDropIpMcEnableGet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with not NULL state.
    Expected: GT_OK.
    1.2. Call with NULL state.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_BOOL     state = GT_TRUE;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*  1.1. */
        st = cpssDxChBrgGenDropIpMcEnableGet(dev, &state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.2. */
        st = cpssDxChBrgGenDropIpMcEnableGet(dev, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }
    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgGenDropIpMcEnableGet(dev, &state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgGenDropIpMcEnableGet(dev, &state);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgGenDropNonIpMcEnable
(
    IN GT_U8    dev,
    IN GT_BOOL  state
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgGenDropNonIpMcEnable)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with state [GT_TRUE and GT_FALSE].
    Expected: GT_OK.
    1.2. Call cpssDxChBrgGenDropNonIpMcEnableGet
    Expected: GT_OK and same enable.
*/
    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_BOOL     state = GT_TRUE;
    GT_BOOL     stateGet = GT_TRUE;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*  1.1. */
        state = GT_FALSE;

        st = cpssDxChBrgGenDropNonIpMcEnable(dev, state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, state);

        /*  1.2. */
        st = cpssDxChBrgGenDropNonIpMcEnableGet(dev, &stateGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        UTF_VERIFY_EQUAL1_STRING_MAC(state, stateGet,
            "get another state than was set: %d", dev);

        /*  1.1. */
        state = GT_TRUE;

        st = cpssDxChBrgGenDropNonIpMcEnable(dev, state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, state);

        /*  1.2. */
        st = cpssDxChBrgGenDropNonIpMcEnableGet(dev, &stateGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        UTF_VERIFY_EQUAL1_STRING_MAC(state, stateGet,
            "get another state than was set: %d", dev);
    }

    state = GT_TRUE;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgGenDropNonIpMcEnable(dev, state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgGenDropNonIpMcEnable(dev, state);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChBrgGenDropNonIpMcEnableGet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with not NULL state.
    Expected: GT_OK.
    1.2. Call with NULL state.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_BOOL     state = GT_TRUE;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*  1.1. */
        st = cpssDxChBrgGenDropNonIpMcEnableGet(dev, &state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.2. */
        st = cpssDxChBrgGenDropNonIpMcEnableGet(dev, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }
    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgGenDropNonIpMcEnableGet(dev, &state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgGenDropNonIpMcEnableGet(dev, &state);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgGenDropInvalidSaEnable
(
    IN GT_U8    dev,
    IN GT_BOOL  enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgGenDropInvalidSaEnable)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with state [GT_TRUE and GT_FALSE].
    Expected: GT_OK.
    1.2. Call cpssDxChBrgGenDropInvalidSaEnableGet
    Expected: GT_OK and same enable.
*/
    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_BOOL     state = GT_TRUE;
    GT_BOOL     stateGet = GT_TRUE;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*  1.1. */
        state = GT_FALSE;

        st = cpssDxChBrgGenDropInvalidSaEnable(dev, state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, state);

        /*  1.2. */

        st = cpssDxChBrgGenDropInvalidSaEnableGet(dev, &stateGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        UTF_VERIFY_EQUAL1_STRING_MAC(state, stateGet,
            "get another state than was set: %d", dev);

        /*  1.1. */
        state = GT_TRUE;

        st = cpssDxChBrgGenDropInvalidSaEnable(dev, state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, state);

        /*  1.2. */
        st = cpssDxChBrgGenDropInvalidSaEnableGet(dev, &stateGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        UTF_VERIFY_EQUAL1_STRING_MAC(state, stateGet,
            "get another state than was set: %d", dev);
    }

    state = GT_TRUE;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgGenDropInvalidSaEnable(dev, state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgGenDropInvalidSaEnable(dev, state);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChBrgGenDropInvalidSaEnableGet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with not NULL state.
    Expected: GT_OK.
    1.2. Call with NULL state.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_BOOL     state = GT_TRUE;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*  1.1. */
        st = cpssDxChBrgGenDropInvalidSaEnableGet(dev, &state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.2. */
        st = cpssDxChBrgGenDropInvalidSaEnableGet(dev, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }
    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgGenDropInvalidSaEnableGet(dev, &state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgGenDropInvalidSaEnableGet(dev, &state);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgGenUcLocalSwitchingEnable
(
    IN GT_U8        dev,
    IN GT_U8        port,
    IN GT_BOOL      enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgGenUcLocalSwitchingEnable)
{
/*
    ITERATE_DEVICES_PHY_PORTS (DxChx)
    1.1.1. Call with enable [GT_FALSE and GT_TRUE].
    Expected: GT_OK.
    1.1.2. Call with cpssDxChBrgGenUcLocalSwitchingEnableGet
    Expected: GT_OK and same enablePtr.
*/
    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_PORT_NUM       port  = BRG_GEN_VALID_PHY_PORT_CNS;
    GT_BOOL     state = GT_TRUE;
    GT_BOOL     stateGet = GT_TRUE;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextVirtPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call with state [GT_FALSE and GT_TRUE].
                Expected: GT_OK.
            */

            /* Call function with state [GT_FALSE] */
            state = GT_FALSE;

            st = cpssDxChBrgGenUcLocalSwitchingEnable(dev, port, state);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, state);

            /* 1.2. */
            st = cpssDxChBrgGenUcLocalSwitchingEnableGet(dev, port, &stateGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            UTF_VERIFY_EQUAL1_STRING_MAC(state, stateGet,
                "get another state than was set: %d", dev);

            /* Call function with state [GT_TRUE] */
            state = GT_TRUE;

            st = cpssDxChBrgGenUcLocalSwitchingEnable(dev, port, state);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, state);

            /* 1.2. */
            st = cpssDxChBrgGenUcLocalSwitchingEnableGet(dev, port, &stateGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            UTF_VERIFY_EQUAL1_STRING_MAC(state, stateGet,
                "get another state than was set: %d", dev);
        }

        state = GT_TRUE;

        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextVirtPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChBrgGenUcLocalSwitchingEnable(dev, port, state);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */

        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChBrgGenUcLocalSwitchingEnable(dev, port, state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                     */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgGenUcLocalSwitchingEnable(dev, port, state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    state = GT_TRUE;

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = BRG_GEN_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgGenUcLocalSwitchingEnable(dev, port, state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgGenUcLocalSwitchingEnable(dev, port, state);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChBrgGenUcLocalSwitchingEnableGet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (DxChx)
    1.1.1. Call with not NUUL enable.
    Expected: GT_OK.
    1.1.2. Call with NUUL enable.
    Expected: GT_OK and same enable.
*/
    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_PORT_NUM       port  = BRG_GEN_VALID_PHY_PORT_CNS;
    GT_BOOL     state = GT_TRUE;


    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextVirtPortGet(&port, GT_TRUE))
        {
            /*  1.1.1.  */
            st = cpssDxChBrgGenUcLocalSwitchingEnableGet(dev, port, &state);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*  1.1.2.  */
            st = cpssDxChBrgGenUcLocalSwitchingEnableGet(dev, port, NULL);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, port);
        }

        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextVirtPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChBrgGenUcLocalSwitchingEnableGet(dev, port, &state);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */

        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChBrgGenUcLocalSwitchingEnableGet(dev, port, &state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                     */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgGenUcLocalSwitchingEnableGet(dev, port, &state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    state = GT_TRUE;

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = BRG_GEN_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgGenUcLocalSwitchingEnableGet(dev, port, &state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgGenUcLocalSwitchingEnableGet(dev, port, &state);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgGenIgmpSnoopModeSet
(
    IN GT_U8                        dev,
    IN CPSS_IGMP_SNOOP_MODE_ENT     mode
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgGenIgmpSnoopModeSet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call function with  mode [ CPSS_IGMP_ALL_TRAP_MODE_E,
                                    CPSS_IGMP_SNOOP_TRAP_MODE_E,
                                    CPSS_IGMP_ROUTER_MIRROR_MODE_E]
    Expected: GT_OK.
    1.2. Call cpssDxChBrgGenIgmpSnoopModeGet
    Expected: GT_OK and same mode.
    1.3.  Call function with invalid mode.
    Expected: NOT GT_OK.
    1.4. Call function with out-of-range mode.
    Expected: GT_BAD_PARAM
*/
    GT_STATUS                   st   = GT_OK;

    GT_U8                       dev;
    CPSS_IGMP_SNOOP_MODE_ENT    mode = CPSS_IGMP_ALL_TRAP_MODE_E;
    CPSS_IGMP_SNOOP_MODE_ENT    modeGet = CPSS_IGMP_ALL_TRAP_MODE_E;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with  mode [ CPSS_IGMP_ALL_TRAP_MODE_E /
                                            CPSS_IGMP_SNOOP_TRAP_MODE_E /
                                            CPSS_IGMP_ROUTER_MIRROR_MODE_E]
            Expected: GT_OK.
        */

        /* Call function with mode [CPSS_IGMP_ALL_TRAP_MODE_E] */
        mode = CPSS_IGMP_ALL_TRAP_MODE_E;

        st = cpssDxChBrgGenIgmpSnoopModeSet(dev, mode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, mode);

        /*  1.2.  */
        st = cpssDxChBrgGenIgmpSnoopModeGet(dev, &modeGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        UTF_VERIFY_EQUAL1_STRING_MAC(mode, modeGet,
            "get another mode than was set: %d", dev);

        /* Call function with mode [CPSS_IGMP_SNOOP_TRAP_MODE_E] */
        mode = CPSS_IGMP_SNOOP_TRAP_MODE_E;

        st = cpssDxChBrgGenIgmpSnoopModeSet(dev, mode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, mode);

        /*  1.2.  */
        st = cpssDxChBrgGenIgmpSnoopModeGet(dev, &modeGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        UTF_VERIFY_EQUAL1_STRING_MAC(mode, modeGet,
            "get another mode than was set: %d", dev);

        /* Call function with mode [CPSS_IGMP_ROUTER_MIRROR_MODE_E] */
        mode = CPSS_IGMP_ROUTER_MIRROR_MODE_E;

        st = cpssDxChBrgGenIgmpSnoopModeSet(dev, mode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, mode);

        /*  1.2.  */
        st = cpssDxChBrgGenIgmpSnoopModeGet(dev, &modeGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        UTF_VERIFY_EQUAL1_STRING_MAC(mode, modeGet,
            "get another mode than was set: %d", dev);

        /*
            1.3.  Call function with invalid mode.
            Expected: NOT GT_OK.
        */
        mode = CPSS_IGMP_ROUTER_MIRROR_MODE_E + 1;

        st = cpssDxChBrgGenIgmpSnoopModeSet(dev, mode);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, mode);

        /*
            1.4. Call function with wrong enum values mode.
            Expected: GT_BAD_PARAM
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChBrgGenIgmpSnoopModeSet
                            (dev, mode),
                            mode);
    }

    mode = CPSS_IGMP_ALL_TRAP_MODE_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgGenIgmpSnoopModeSet(dev, mode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgGenIgmpSnoopModeSet(dev, mode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChBrgGenIgmpSnoopModeGet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call function with  not NULL modePtr
    Expected: GT_OK.
    1.2. Call function with NULL modePtr
    Expected: GT_BAD_PTR.
*/
    GT_STATUS                   st   = GT_OK;

    GT_U8                       dev;
    CPSS_IGMP_SNOOP_MODE_ENT    mode = CPSS_IGMP_ALL_TRAP_MODE_E;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*  1.1.  */
        st = cpssDxChBrgGenIgmpSnoopModeGet(dev, &mode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.2.  */
        st = cpssDxChBrgGenIgmpSnoopModeGet(dev, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgGenIgmpSnoopModeGet(dev, &mode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgGenIgmpSnoopModeGet(dev, &mode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgGenArpBcastToCpuCmdSet
(
    IN GT_U8                            dev,
    IN CPSS_DXCH_ARP_BCAST_CMD_MODE_ENT cmdMode,
    IN CPSS_PACKET_CMD_ENT              cmd
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgGenArpBcastToCpuCmdSet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call function with cmd [CPSS_PACKET_CMD_FORWARD_E /
                                 CPSS_PACKET_CMD_MIRROR_TO_CPU_E /
                                 CPSS_PACKET_CMD_TRAP_TO_CPU_E]
    Expected: GT_OK.
    1.2. Call cpssDxChBrgGenArpBcastToCpuCmdGet
    Expected: GT_OK and same cmd.
    1.3. Call function with cmd [CPSS_PACKET_CMD_DROP_HARD_E /
                                 CPSS_PACKET_CMD_DROP_SOFT_E] (not supported).
    Expected: NOT GT_OK.
    1.4. Call function with out-of-range cmd.
    Expected: GT_BAD_PARAM
*/
    GT_STATUS           st  = GT_OK;

    GT_U8               dev;
    CPSS_DXCH_ARP_BCAST_CMD_MODE_ENT cmdMode;
    CPSS_PACKET_CMD_ENT cmd = CPSS_PACKET_CMD_FORWARD_E;
    CPSS_PACKET_CMD_ENT cmdGet = CPSS_PACKET_CMD_FORWARD_E;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for (cmdMode = CPSS_DXCH_ARP_BCAST_CMD_MODE_PORT_E;
             cmdMode <= CPSS_DXCH_ARP_BCAST_CMD_MODE_VLAN_E;
             cmdMode++)
        {
            /*
                1.1. Call function with cmd [CPSS_PACKET_CMD_FORWARD_E /
                                             CPSS_PACKET_CMD_MIRROR_TO_CPU_E /
                                             CPSS_PACKET_CMD_TRAP_TO_CPU_E]
                Expected: GT_OK.
            */

            /* Call function with cmd [CPSS_PACKET_CMD_FORWARD_E] */
            cmd = CPSS_PACKET_CMD_FORWARD_E;

            st = cpssDxChBrgGenArpBcastToCpuCmdSet(dev, cmdMode, cmd);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, cmd);

            /*  1.2.  */
            st = cpssDxChBrgGenArpBcastToCpuCmdGet(dev, cmdMode, &cmdGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* Verifying value */
            UTF_VERIFY_EQUAL1_STRING_MAC(cmd, cmdGet,
                "get another cmd than was set: %d", dev);

            /* Call function with cmd [CPSS_PACKET_CMD_MIRROR_TO_CPU_E] */
            cmd = CPSS_PACKET_CMD_MIRROR_TO_CPU_E;

            st = cpssDxChBrgGenArpBcastToCpuCmdSet(dev, cmdMode, cmd);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, cmd);

            /*  1.2.  */
            st = cpssDxChBrgGenArpBcastToCpuCmdGet(dev, cmdMode, &cmdGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* Verifying value */
            UTF_VERIFY_EQUAL1_STRING_MAC(cmd, cmdGet,
                "get another cmd than was set: %d", dev);

            /* Call function with cmd [CPSS_PACKET_CMD_TRAP_TO_CPU_E] */
            cmd = CPSS_PACKET_CMD_TRAP_TO_CPU_E;

            st = cpssDxChBrgGenArpBcastToCpuCmdSet(dev, cmdMode, cmd);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, cmd);

            /*  1.2.  */
            st = cpssDxChBrgGenArpBcastToCpuCmdGet(dev, cmdMode, &cmdGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* Verifying value */
            UTF_VERIFY_EQUAL1_STRING_MAC(cmd, cmdGet,
                "get another cmd than was set: %d", dev);
            /*
                1.3. Call function with cmd [CPSS_PACKET_CMD_DROP_HARD_E /
                                             CPSS_PACKET_CMD_DROP_SOFT_E] (not supported).
                Expected: NOT GT_OK.
            */

             if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev) == GT_FALSE)
             {
                /* Call function with cmd [CPSS_PACKET_CMD_MIRROR_TO_CPU_E] */
                cmd = CPSS_PACKET_CMD_DROP_HARD_E;

                st = cpssDxChBrgGenArpBcastToCpuCmdSet(dev, cmdMode, cmd);
                UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, cmd);

                /* Call function with cmd [CPSS_PACKET_CMD_DROP_SOFT_E] */
                cmd = CPSS_PACKET_CMD_DROP_SOFT_E;

                st = cpssDxChBrgGenArpBcastToCpuCmdSet(dev, cmdMode, cmd);
                UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, cmd);
             }

            /*
                1.4. Call function with wrong enum values cmd.
                Expected: GT_BAD_PARAM.
            */
            UTF_ENUMS_CHECK_MAC(cpssDxChBrgGenArpBcastToCpuCmdSet
                                (dev, cmdMode, cmd), cmd);
        }
    }

    cmd = CPSS_PACKET_CMD_FORWARD_E;
    cmdMode = CPSS_DXCH_ARP_BCAST_CMD_MODE_VLAN_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgGenArpBcastToCpuCmdSet(dev, cmdMode, cmd);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgGenArpBcastToCpuCmdSet(dev, cmdMode, cmd);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChBrgGenArpBcastToCpuCmdGet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call function with not NULL
    Expected: GT_OK.
    1.2. Call function with NULL
    Expected: GT_BAD_PTR.
*/
    GT_STATUS           st  = GT_OK;

    GT_U8               dev;
    CPSS_DXCH_ARP_BCAST_CMD_MODE_ENT cmdMode;
    CPSS_PACKET_CMD_ENT cmd = CPSS_PACKET_CMD_FORWARD_E;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for (cmdMode = CPSS_DXCH_ARP_BCAST_CMD_MODE_PORT_E;
             cmdMode <= CPSS_DXCH_ARP_BCAST_CMD_MODE_VLAN_E;
             cmdMode++)
        {
            /*  1.1.  */
            st = cpssDxChBrgGenArpBcastToCpuCmdGet(dev, cmdMode, &cmd);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*  1.2.  */
            st = cpssDxChBrgGenArpBcastToCpuCmdGet(dev, cmdMode, NULL);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
        }
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);
    cmdMode = CPSS_DXCH_ARP_BCAST_CMD_MODE_VLAN_E;

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgGenArpBcastToCpuCmdGet(dev, cmdMode, &cmd);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChBrgGenArpBcastToCpuCmdGet(dev, cmdMode, &cmd);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChBrgGenRipV1MirrorToCpuEnable)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with state [GT_TRUE and GT_FALSE].
    Expected: GT_OK.
    1.2. Call cpssDxChBrgGenDropIpMcEnableGet
    Expected: GT_OK and same enable.
*/
    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_BOOL     state = GT_TRUE;
    GT_BOOL     stateGet = GT_TRUE;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*  1.1. */
        state = GT_FALSE;

        st = cpssDxChBrgGenRipV1MirrorToCpuEnable(dev, state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, state);

        /*  1.2. */
        st = cpssDxChBrgGenRipV1MirrorToCpuEnableGet(dev, &stateGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        UTF_VERIFY_EQUAL1_STRING_MAC(state, stateGet,
            "get another state than was set: %d", dev);

        /*  1.1. */
        state = GT_TRUE;

        st = cpssDxChBrgGenRipV1MirrorToCpuEnable(dev, state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, state);

        /*  1.2. */
        st = cpssDxChBrgGenRipV1MirrorToCpuEnableGet(dev, &stateGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        UTF_VERIFY_EQUAL1_STRING_MAC(state, stateGet,
            "get another state than was set: %d", dev);
    }

    state = GT_TRUE;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgGenRipV1MirrorToCpuEnable(dev, state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgGenRipV1MirrorToCpuEnable(dev, state);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChBrgGenRipV1MirrorToCpuEnableGet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with not NULL state.
    Expected: GT_OK.
    1.2. Call with NULL state.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_BOOL     state = GT_TRUE;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*  1.1. */
        st = cpssDxChBrgGenRipV1MirrorToCpuEnableGet(dev, &state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.2. */
        st = cpssDxChBrgGenRipV1MirrorToCpuEnableGet(dev, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }
    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgGenRipV1MirrorToCpuEnableGet(dev, &state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgGenRipV1MirrorToCpuEnableGet(dev, &state);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChBrgGenIeeeReservedMcastTrapEnable)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with state [GT_TRUE and GT_FALSE].
    Expected: GT_OK.
    1.2. Call cpssDxChBrgGenDropIpMcEnableGet
    Expected: GT_OK and same enable.
*/
    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_BOOL     state = GT_TRUE;
    GT_BOOL     stateGet = GT_TRUE;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*  1.1. */
        state = GT_FALSE;

        st = cpssDxChBrgGenIeeeReservedMcastTrapEnable(dev, state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, state);

        /*  1.2. */
        st = cpssDxChBrgGenIeeeReservedMcastTrapEnableGet(dev, &stateGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        UTF_VERIFY_EQUAL1_STRING_MAC(state, stateGet,
            "get another state than was set: %d", dev);

        /*  1.1. */
        state = GT_TRUE;

        st = cpssDxChBrgGenIeeeReservedMcastTrapEnable(dev, state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, state);

        /*  1.2. */
        st = cpssDxChBrgGenIeeeReservedMcastTrapEnableGet(dev, &stateGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        UTF_VERIFY_EQUAL1_STRING_MAC(state, stateGet,
            "get another state than was set: %d", dev);
    }

    state = GT_TRUE;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgGenIeeeReservedMcastTrapEnable(dev, state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgGenIeeeReservedMcastTrapEnable(dev, state);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChBrgGenIeeeReservedMcastTrapEnableGet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with not NULL state.
    Expected: GT_OK.
    1.2. Call with NULL state.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_BOOL     state = GT_TRUE;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*  1.1. */
        st = cpssDxChBrgGenIeeeReservedMcastTrapEnableGet(dev, &state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.2. */
        st = cpssDxChBrgGenIeeeReservedMcastTrapEnableGet(dev, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }
    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgGenIeeeReservedMcastTrapEnableGet(dev, &state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgGenIeeeReservedMcastTrapEnableGet(dev, &state);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgGenIeeeReservedMcastProtCmdSet
(
    IN GT_U8                devNum,
    IN GT_U32               profileIndex,
    IN GT_U8                protocol,
    IN CPSS_PACKET_CMD_ENT  cmd
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgGenIeeeReservedMcastProtCmdSet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call function with protocol [0, 10, 100, 250],
                            cmd [CPSS_PACKET_CMD_FORWARD_E /
                                 CPSS_PACKET_CMD_MIRROR_TO_CPU_E /
                                 CPSS_PACKET_CMD_TRAP_TO_CPU_E /
                                 CPSS_PACKET_CMD_DROP_SOFT_E]
    Expected: GT_OK.
    1.2. Call cpssDxChBrgGenIeeeReservedMcastProtCmdGet with the same parameters.
    Expected: GT_OK and the same value.
    1.3.  Call  with wrong profileIndex [5] (relevant only for xCat and above)
    Expected: NOT GT_OK for xCat and above and GT_OK for other.
    1.4. Call function with mode [CPSS_PACKET_CMD_DROP_HARD_E] (not supported)
                            and protocol [0].
    Expected: NOT GT_OK.
    1.5. Call function with out-of-range cmd and protocol [0].
    Expected: GT_BAD_PARAM
*/
    GT_STATUS st    = GT_OK;

    GT_U8               dev;
    GT_U32              profileIndex = 0;
    GT_U8               protocol     = 0;
    CPSS_PACKET_CMD_ENT cmd          = CPSS_PACKET_CMD_FORWARD_E;
    CPSS_PACKET_CMD_ENT cmdGet;
    CPSS_PACKET_CMD_ENT  cmdArr[4]={CPSS_PACKET_CMD_FORWARD_E,
                                    CPSS_PACKET_CMD_MIRROR_TO_CPU_E,
                                    CPSS_PACKET_CMD_TRAP_TO_CPU_E,
                                    CPSS_PACKET_CMD_DROP_SOFT_E};

    GT_U32 cmdIndex;
    GT_U32 profileNum;/* number of profiles : xcat..Lion2 - 4 , Bobcat2, Caelum, Bobcat3 - 8 */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with protocol [0, 10, 100, 250],
                                    cmd [CPSS_PACKET_CMD_FORWARD_E /
                                         CPSS_PACKET_CMD_MIRROR_TO_CPU_E /
                                         CPSS_PACKET_CMD_TRAP_TO_CPU_E /
                                         CPSS_PACKET_CMD_DROP_SOFT_E]
            Expected: GT_OK.
        */


        /* Call function with profileIndex [0], protocol [0], cmd [CPSS_PACKET_CMD_FORWARD_E] */
        profileIndex = 0;
        protocol     = 0;
        cmd          = CPSS_PACKET_CMD_FORWARD_E;

        st = cpssDxChBrgGenIeeeReservedMcastProtCmdSet(dev, profileIndex, protocol, cmd);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, protocol, cmd);

        /*
            1.2. Call cpssDxChBrgGenIeeeReservedMcastProtCmdGet with the same parameters.
            Expected: GT_OK and the same value.
         */
        st = cpssDxChBrgGenIeeeReservedMcastProtCmdGet(dev, profileIndex, protocol, &cmdGet);

        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                          "cpssDxChBrgGenIeeeReservedMcastProtCmdGet: %d, %d", dev, cmdGet);
        /* Verifying value */
        UTF_VERIFY_EQUAL1_STRING_MAC(cmd, cmdGet, "get another cmd than was set: %d", dev);


        /* Call function with protocol [10], cmd [CPSS_PACKET_CMD_MIRROR_TO_CPU_E] */
        protocol = 10;
        cmd = CPSS_PACKET_CMD_MIRROR_TO_CPU_E;

        st = cpssDxChBrgGenIeeeReservedMcastProtCmdSet(dev, profileIndex, protocol, cmd);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, protocol, cmd);

        /*
            1.2. Call cpssDxChBrgGenIeeeReservedMcastProtCmdGet with the same parameters.
            Expected: GT_OK and the same value.
         */
        st = cpssDxChBrgGenIeeeReservedMcastProtCmdGet(dev, profileIndex, protocol, &cmdGet);

        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                          "cpssDxChBrgGenIeeeReservedMcastProtCmdGet: %d, %d", dev, cmdGet);
        /* Verifying value */
        UTF_VERIFY_EQUAL1_STRING_MAC(cmd, cmdGet, "get another cmd than was set: %d", dev);


        /* Call function with protocol [100], cmd [CPSS_PACKET_CMD_TRAP_TO_CPU_E] */
        protocol = 100;
        cmd = CPSS_PACKET_CMD_TRAP_TO_CPU_E;

        st = cpssDxChBrgGenIeeeReservedMcastProtCmdSet(dev, profileIndex, protocol, cmd);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, protocol, cmd);

        /*
            1.2. Call cpssDxChBrgGenIeeeReservedMcastProtCmdGet with the same parameters.
            Expected: GT_OK and the same value.
         */
        st = cpssDxChBrgGenIeeeReservedMcastProtCmdGet(dev, profileIndex, protocol, &cmdGet);

        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                          "cpssDxChBrgGenIeeeReservedMcastProtCmdGet: %d, %d", dev, cmdGet);
        /* Verifying value */
        UTF_VERIFY_EQUAL1_STRING_MAC(cmd, cmdGet, "get another cmd than was set: %d", dev);


        /* Call function with protocol [250], cmd [CPSS_PACKET_CMD_DROP_SOFT_E] */
        protocol = 250;
        cmd = CPSS_PACKET_CMD_DROP_SOFT_E;

        st = cpssDxChBrgGenIeeeReservedMcastProtCmdSet(dev, profileIndex, protocol, cmd);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, protocol, cmd);

        /*
            1.2. Call cpssDxChBrgGenIeeeReservedMcastProtCmdGet with the same parameters.
            Expected: GT_OK and the same value.
         */
        st = cpssDxChBrgGenIeeeReservedMcastProtCmdGet(dev, profileIndex, protocol, &cmdGet);

        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                          "cpssDxChBrgGenIeeeReservedMcastProtCmdGet: %d, %d", dev, cmdGet);
        /* Verifying value */
        UTF_VERIFY_EQUAL1_STRING_MAC(cmd, cmdGet, "get another cmd than was set: %d", dev);

        /*
            1.3.  Call  with wrong profileIndex [5] (relevant only for xCat and above)
            Expected: NOT GT_OK for xCat and above and GT_OK for other.
        */

        if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
        {
            profileNum = 8;
        }
        else
        {
            profileNum = 4;
        }
        profileIndex = profileNum;

        st = cpssDxChBrgGenIeeeReservedMcastProtCmdSet(dev, profileIndex, protocol, cmd);

        profileIndex = 0;

        if(PRV_CPSS_PP_MAC(dev)->devFamily >= CPSS_PP_FAMILY_DXCH_XCAT_E)
        {
            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, protocol, cmd);
        }
        else
        {
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, protocol, cmd);
        }

        /*
            1.4.  Call function with mode [CPSS_PACKET_CMD_DROP_HARD_E] (not supported)
                                     and protocol [0].
            Expected: NOT GT_OK.
        */
        protocol = 0;
        cmd = CPSS_PACKET_CMD_DROP_HARD_E;

        st = cpssDxChBrgGenIeeeReservedMcastProtCmdSet(dev, profileIndex, protocol, cmd);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, cmd = %d", dev, cmd);

        /*
            1.5. Call function with wrong enum values cmd and protocol [0].
            Expected: GT_BAD_PARAM
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChBrgGenIeeeReservedMcastProtCmdSet
                            (dev, profileIndex, protocol, cmd),
                            cmd);
    }

    protocol = 0;
    cmd = CPSS_PACKET_CMD_FORWARD_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgGenIeeeReservedMcastProtCmdSet(dev, profileIndex, protocol, cmd);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgGenIeeeReservedMcastProtCmdSet(dev, profileIndex, protocol, cmd);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);


    /* Test for xCat devices */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        if(GT_TRUE == prvUtfReduceLogSizeFlagGet())
        {
            /* we need not checks below for log tests */
            break;
        }

        /* Call function with profileIndex [2], protocol [0], cmd [CPSS_PACKET_CMD_TRAP_TO_CPU_E] */
        profileIndex = 2;
        protocol     = 0;
        cmd          = CPSS_PACKET_CMD_TRAP_TO_CPU_E;

        st = cpssDxChBrgGenIeeeReservedMcastProtCmdSet(dev, profileIndex, protocol, cmd);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, protocol, cmd);

        /*
            1.2. Call cpssDxChBrgGenIeeeReservedMcastProtCmdGet with the same parameters.
            Expected: GT_OK and the same value.
         */
        st = cpssDxChBrgGenIeeeReservedMcastProtCmdGet(dev, profileIndex, protocol, &cmdGet);

        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                          "cpssDxChBrgGenIeeeReservedMcastProtCmdGet: %d, %d", dev, cmdGet);
        /* Verifying value */
        UTF_VERIFY_EQUAL1_STRING_MAC(cmd, cmdGet, "get another cmd than was set: %d", dev);

        if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
        {
            profileNum = 8;
        }
        else
        {
            profileNum = 4;
        }

        for( profileIndex = 0; profileIndex < profileNum; profileIndex++)
        {
            for(protocol = 0; /* no break here */ ; protocol++)
            {
                cmdIndex = (profileIndex + protocol) % 4;
                cmd = cmdArr[cmdIndex];
                st = cpssDxChBrgGenIeeeReservedMcastProtCmdSet(dev, profileIndex, protocol, cmd);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, protocol, cmd);

                if(protocol == 255)
                {
                    break;
                }
            }
        }

        for( profileIndex = 0; profileIndex < profileNum; profileIndex++)
        {
            for(protocol = 0; /* no break here */; protocol++)
            {
                cmdIndex = (profileIndex + protocol) % 4;
                cmd = cmdArr[cmdIndex];

                st = cpssDxChBrgGenIeeeReservedMcastProtCmdGet(dev, profileIndex, protocol, &cmdGet);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                  "cpssDxChBrgGenIeeeReservedMcastProtCmdGet: %d, %d", dev, cmdGet);
                /* Verifying value */
                UTF_VERIFY_EQUAL1_STRING_MAC(cmd, cmdGet, "get another cmd than was set: %d", dev);
                if(protocol == 255)
                {
                    break;
                }
            }
       }

    }

}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgGenIeeeReservedMcastProtCmdGet
(
    IN GT_U8                devNum,
    IN GT_U32               profileIndex,
    IN GT_U8                protocol,
    OUT CPSS_PACKET_CMD_ENT *cmdPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgGenIeeeReservedMcastProtCmdGet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call function with protocol [0, 10, 100, 250].
    Expected: GT_OK.
    1.2.  Call  with wrong profileIndex [5] (relevant only for xCat and above)
    Expected: NOT GT_OK for xCat and above and GT_OK for other.
    1.3. Call with wrong cmdPtr [NULL] and protocol [0].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS           st           = GT_OK;

    GT_U8               dev;
    GT_U32              profileIndex = 0;
    GT_U8               protocol     = 0;
    CPSS_PACKET_CMD_ENT cmd;
    GT_U32 profileNum;/* number of profiles : xcat..Lion2 - 4 , Bobcat2, Caelum, Bobcat3 - 8 */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with protocol [0, 10, 100, 250].
            Expected: GT_OK.
        */

        /* Call function with profileIndex [0], protocol [0]*/
        profileIndex = 0;
        protocol     = 0;

        st = cpssDxChBrgGenIeeeReservedMcastProtCmdGet(dev, profileIndex, protocol, &cmd);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, protocol, &cmd);

        /* Call function with protocol [10]*/
        protocol = 10;

        st = cpssDxChBrgGenIeeeReservedMcastProtCmdGet(dev, profileIndex, protocol, &cmd);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, protocol, &cmd);

        /* Call function with protocol [100]*/
        protocol = 100;

        st = cpssDxChBrgGenIeeeReservedMcastProtCmdGet(dev, profileIndex, protocol, &cmd);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, protocol, &cmd);

        /* Call function with protocol [250]*/
        protocol = 250;

        st = cpssDxChBrgGenIeeeReservedMcastProtCmdGet(dev, profileIndex, protocol, &cmd);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, protocol, &cmd);

        /*
            1.2.  Call  with wrong profileIndex [5] (relevant only for xCat and above)
            Expected: NOT GT_OK for xCat and above and GT_OK for other.
        */

        if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
        {
            profileNum = 8;
        }
        else
        {
            profileNum = 4;
        }

        profileIndex = profileNum;

        st = cpssDxChBrgGenIeeeReservedMcastProtCmdGet(dev, profileIndex, protocol, &cmd);

        profileIndex = 0;

        if(PRV_CPSS_PP_MAC(dev)->devFamily >= CPSS_PP_FAMILY_DXCH_XCAT_E)
        {
            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, protocol, &cmd);
        }
        else
        {
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, protocol, &cmd);
        }

        /*
            1.3. Call with wrong cmdPtr [NULL] and protocol [0].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChBrgGenIeeeReservedMcastProtCmdGet(dev, profileIndex, protocol, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, cmdPtr NULL", dev);
    }

    protocol = 0;
    cmd = CPSS_PACKET_CMD_FORWARD_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgGenIeeeReservedMcastProtCmdGet(dev, profileIndex, protocol, &cmd);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgGenIeeeReservedMcastProtCmdGet(dev, profileIndex, protocol, &cmd);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgGenCiscoL2ProtCmdSet
(
    IN GT_U8                dev,
    IN CPSS_PACKET_CMD_ENT  cmd
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgGenCiscoL2ProtCmdSet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call function with cmd [CPSS_PACKET_CMD_FORWARD_E /
                                 CPSS_PACKET_CMD_MIRROR_TO_CPU_E /
                                 CPSS_PACKET_CMD_TRAP_TO_CPU_E]
    Expected: GT_OK.
    1.2. Call cpssDxChBrgGenCiscoL2ProtCmdGet
    Expected: GT_OK and same cmd.
    1.3.  Call function with cmd [CPSS_PACKET_CMD_DROP_HARD_E /
                                  CPSS_PACKET_CMD_DROP_SOFT_E] (not supported).
    Expected: NOT GT_OK.
    1.4. Call function with out-of-range cmd.
    Expected: GT_BAD_PARAM
*/
    GT_STATUS           st  = GT_OK;

    GT_U8               dev;
    CPSS_PACKET_CMD_ENT cmd = CPSS_PACKET_CMD_FORWARD_E;
    CPSS_PACKET_CMD_ENT cmdGet = CPSS_PACKET_CMD_FORWARD_E;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with cmd [CPSS_PACKET_CMD_FORWARD_E /
                                         CPSS_PACKET_CMD_MIRROR_TO_CPU_E /
                                         CPSS_PACKET_CMD_TRAP_TO_CPU_E]
            Expected: GT_OK.
        */

        /* Call function with cmd [CPSS_PACKET_CMD_FORWARD_E] */
        cmd = CPSS_PACKET_CMD_FORWARD_E;

        st = cpssDxChBrgGenCiscoL2ProtCmdSet(dev, cmd);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, cmd);

        /*  1.2.  */
        st = cpssDxChBrgGenCiscoL2ProtCmdGet(dev, &cmdGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        UTF_VERIFY_EQUAL1_STRING_MAC(cmd, cmdGet,
            "get another cmd than was set: %d", dev);

        /* Call function with cmd [CPSS_PACKET_CMD_MIRROR_TO_CPU_E] */
        cmd = CPSS_PACKET_CMD_MIRROR_TO_CPU_E;

        st = cpssDxChBrgGenCiscoL2ProtCmdSet(dev, cmd);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, cmd);

        /*  1.2.  */
        st = cpssDxChBrgGenCiscoL2ProtCmdGet(dev, &cmdGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        UTF_VERIFY_EQUAL1_STRING_MAC(cmd, cmdGet,
            "get another cmd than was set: %d", dev);

        /* Call function with cmd [CPSS_PACKET_CMD_TRAP_TO_CPU_E] */
        cmd = CPSS_PACKET_CMD_TRAP_TO_CPU_E;

        st = cpssDxChBrgGenCiscoL2ProtCmdSet(dev, cmd);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, cmd);

        /*  1.2.  */
        st = cpssDxChBrgGenCiscoL2ProtCmdGet(dev, &cmdGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        UTF_VERIFY_EQUAL1_STRING_MAC(cmd, cmdGet,
            "get another cmd than was set: %d", dev);

        /*
            1.3. Call function with cmd [CPSS_PACKET_CMD_DROP_HARD_E /
                                         CPSS_PACKET_CMD_DROP_SOFT_E] (not supported).
            Expected: NOT GT_OK.
        */

        /* Call function with cmd [CPSS_PACKET_CMD_MIRROR_TO_CPU_E] */
        cmd = CPSS_PACKET_CMD_DROP_HARD_E;

        st = cpssDxChBrgGenCiscoL2ProtCmdSet(dev, cmd);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, cmd);

        /* Call function with cmd [CPSS_PACKET_CMD_DROP_SOFT_E] */
        cmd = CPSS_PACKET_CMD_DROP_SOFT_E;

        st = cpssDxChBrgGenCiscoL2ProtCmdSet(dev, cmd);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, cmd);

        /*
            1.4. Call function with wrong enum values cmd.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChBrgGenCiscoL2ProtCmdSet
                            (dev, cmd),
                            cmd);
    }

    cmd = CPSS_PACKET_CMD_FORWARD_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgGenCiscoL2ProtCmdSet(dev, cmd);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgGenCiscoL2ProtCmdSet(dev, cmd);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChBrgGenCiscoL2ProtCmdGet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call function with nopt NULL cmd
    Expected: GT_OK.
    1.2.  Call function with NULL cmd
    Expected:GT_BAD_PTR.
*/
    GT_STATUS           st  = GT_OK;

    GT_U8               dev;
    CPSS_PACKET_CMD_ENT cmd = CPSS_PACKET_CMD_FORWARD_E;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*  1.1. */
        st = cpssDxChBrgGenCiscoL2ProtCmdGet(dev, &cmd);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.2. */
        st = cpssDxChBrgGenCiscoL2ProtCmdGet(dev, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgGenCiscoL2ProtCmdGet(dev, &cmd);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgGenCiscoL2ProtCmdGet(dev, &cmd);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgGenIpV6IcmpTrapEnable
(
    IN GT_U8      dev,
    IN GT_U8      port,
    IN GT_BOOL    enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgGenIpV6IcmpTrapEnable)
{
/*
    ITERATE_DEVICES_PHY_PORTS (DxChx)
    1.1.1. Call with state [GT_FALSE and GT_TRUE].
    Expected: GT_OK.
    1.1.2. Call cpssDxChBrgGenIpV6IcmpTrapEnableGet.
    Expected: GT_OK and the same value.
*/
    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_PORT_NUM       port  = BRG_GEN_VALID_PHY_PORT_CNS;
    GT_BOOL     state = GT_TRUE;
    GT_BOOL     stateGet;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_E_ARCH_CNS);/*SIP5 not support*/

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextVirtPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call with state [GT_FALSE and GT_TRUE].
                Expected: GT_OK.
            */

            /* Call function with state [GT_FALSE] */
            state = GT_FALSE;

            st = cpssDxChBrgGenIpV6IcmpTrapEnable(dev, port, state);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, state);

            /*
                1.1.2. Call cpssDxChBrgGenIpV6IcmpTrapEnableGet.
                Expected: GT_OK and the same value.
            */

            st = cpssDxChBrgGenIpV6IcmpTrapEnableGet(dev, port, &stateGet);

            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                         "cpssDxChBrgGenIpV6IcmpTrapEnableGet: %d, %d", dev, stateGet);
            /* Verifying value */
            UTF_VERIFY_EQUAL1_STRING_MAC(state, stateGet,
                                         "get another state than was set: %d", dev);


            /* Call function with state [GT_TRUE] */
            state = GT_TRUE;

            st = cpssDxChBrgGenIpV6IcmpTrapEnable(dev, port, state);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, state);

            /*
                1.1.2. Call cpssDxChBrgGenIpV6IcmpTrapEnableGet.
                Expected: GT_OK and the same value.
            */

            st = cpssDxChBrgGenIpV6IcmpTrapEnableGet(dev, port, &stateGet);

            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                         "cpssDxChBrgGenIpV6IcmpTrapEnableGet: %d, %d", dev, stateGet);
            /* Verifying value */
            UTF_VERIFY_EQUAL1_STRING_MAC(state, stateGet,
                                         "get another state than was set: %d", dev);
        }

        state = GT_TRUE;

        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextVirtPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChBrgGenIpV6IcmpTrapEnable(dev, port, state);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */

        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChBrgGenIpV6IcmpTrapEnable(dev, port, state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                     */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgGenIpV6IcmpTrapEnable(dev, port, state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    state = GT_TRUE;

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = BRG_GEN_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_E_ARCH_CNS);/*SIP5 not support*/

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgGenIpV6IcmpTrapEnable(dev, port, state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgGenIpV6IcmpTrapEnable(dev, port, state);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgGenIpV6IcmpTrapEnableGet
(
    IN  GT_U8        devNum,
    IN  GT_U8        port,
    OUT GT_BOOL      *statusPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgGenIpV6IcmpTrapEnableGet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (DxChx)
    1.1.1. Call with valid statusPtr.
    Expected: GT_OK.
    1.1.2. Call wrong statusPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st   = GT_OK;

    GT_U8       dev;
    GT_PORT_NUM       port = BRG_GEN_VALID_PHY_PORT_CNS;
    GT_BOOL     state;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_E_ARCH_CNS);/*SIP5 not support*/

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextVirtPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call with valid statusPtr.
                Expected: GT_OK.
            */

            st = cpssDxChBrgGenIpV6IcmpTrapEnableGet(dev, port, &state);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, state);

            /*
                1.1.2. Call wrong statusPtr [NULL].
                Expected: GT_OK.
            */

            st = cpssDxChBrgGenIpV6IcmpTrapEnableGet(dev, port, NULL);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PTR, st, dev, port, state);
        }

        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextVirtPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChBrgGenIpV6IcmpTrapEnableGet(dev, port, &state);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */

        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChBrgGenIpV6IcmpTrapEnableGet(dev, port, &state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                     */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgGenIpV6IcmpTrapEnableGet(dev, port, &state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    state = GT_TRUE;

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = BRG_GEN_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_E_ARCH_CNS);/*SIP5 not support*/

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgGenIpV6IcmpTrapEnableGet(dev, port, &state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgGenIpV6IcmpTrapEnableGet(dev, port, &state);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgGenIcmpv6MsgTypeSet
(
    IN GT_U8                dev,
    IN GT_U32               index,
    IN GT_U8                msgType,
    IN CPSS_PACKET_CMD_ENT  cmd
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgGenIcmpv6MsgTypeSet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call function with valid index [0, 1, 6],
                            valid msgType [0, 3, 250],
                            valid cmd [CPSS_PACKET_CMD_FORWARD_E /
                                       CPSS_PACKET_CMD_MIRROR_TO_CPU_E /
                                       CPSS_PACKET_CMD_TRAP_TO_CPU_E].
    Expected: GT_OK.
    1.2. Call cpssDxChBrgGenIcmpv6MsgTypeGet
    Expected: GT_OK and same msgType, cmd.
    1.3. Call function with out of range index [8]
                            and index [0],
                            msgType [0].
    Expected:GT_BAD_PARAM.
    1.4. Call function with cmd [CPSS_PACKET_CMD_DROP_HARD_E /
                                 CPSS_PACKET_CMD_DROP_SOFT_E] (not supported)
                            and other parameters from 1.2.
    Expected: NOT GT_OK.
    1.5. Call function with out of range cmd
                            and other parameters from 1.2.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS           st       = GT_OK;

    GT_U8               dev;
    GT_U32              index      = 0;
    GT_U8               msgType    = 0;
    CPSS_PACKET_CMD_ENT cmd        = CPSS_PACKET_CMD_FORWARD_E;
    GT_U8               msgTypeGet = 0;
    CPSS_PACKET_CMD_ENT cmdGet     = CPSS_PACKET_CMD_FORWARD_E;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with valid index [0, 1, 6],
                                    valid msgType [0, 3, 250],
                                    valid cmd [CPSS_PACKET_CMD_FORWARD_E /
                                               CPSS_PACKET_CMD_MIRROR_TO_CPU_E /
                                               CPSS_PACKET_CMD_TRAP_TO_CPU_E].
            Expected: GT_OK.
        */

        /* Call function with index [0], msgType [0], cmd [CPSS_PACKET_CMD_FORWARD_E] */
        index = 0;
        msgType = 0;
        cmd = CPSS_PACKET_CMD_FORWARD_E;

        st = cpssDxChBrgGenIcmpv6MsgTypeSet(dev, index, msgType, cmd);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, index, msgType, cmd);

        /*  1.2.  */
        st = cpssDxChBrgGenIcmpv6MsgTypeGet(dev, index, &msgTypeGet, &cmdGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, index);

        UTF_VERIFY_EQUAL1_STRING_MAC(msgType, msgTypeGet,
            "get another msgType than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(cmd, cmdGet,
            "get another cmd than was set: %d", dev);

        /* Call function with index [0], msgType [0], cmd [CPSS_PACKET_CMD_MIRROR_TO_CPU_E] */
        index = 1;
        msgType = 3;
        cmd = CPSS_PACKET_CMD_MIRROR_TO_CPU_E;

        st = cpssDxChBrgGenIcmpv6MsgTypeSet(dev, index, msgType, cmd);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, index, msgType, cmd);

        /*  1.2.  */
        st = cpssDxChBrgGenIcmpv6MsgTypeGet(dev, index, &msgTypeGet, &cmdGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, index);

        UTF_VERIFY_EQUAL1_STRING_MAC(msgType, msgTypeGet,
            "get another msgType than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(cmd, cmdGet,
            "get another cmd than was set: %d", dev);

        /* Call function with index [0], msgType [0], cmd [CPSS_PACKET_CMD_TRAP_TO_CPU_E] */
        index = 6;
        msgType = 250;
        cmd = CPSS_PACKET_CMD_TRAP_TO_CPU_E;

        st = cpssDxChBrgGenIcmpv6MsgTypeSet(dev, index, msgType, cmd);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, index, msgType, cmd);

        /*  1.2.  */
        st = cpssDxChBrgGenIcmpv6MsgTypeGet(dev, index, &msgTypeGet, &cmdGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, index);

        UTF_VERIFY_EQUAL1_STRING_MAC(msgType, msgTypeGet,
            "get another msgType than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(cmd, cmdGet,
            "get another cmd than was set: %d", dev);

        /*
            1.3. Call with out of range index [8]
                           msgType [0].
            Expected:GT_BAD_PARAM.
        */
        index = 8;
        msgType = 0;
        cmd = CPSS_PACKET_CMD_FORWARD_E;

        st = cpssDxChBrgGenIcmpv6MsgTypeSet(dev, index, msgType, cmd);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, index);

        /*
            1.4. Call function with cmd [CPSS_PACKET_CMD_DROP_HARD_E /
                                         CPSS_PACKET_CMD_DROP_SOFT_E] (not supported) and other parameters from 1.1.
            Expected: NOT GT_OK.
        */

        /* Call function with index [0], msgType [0], cmd [CPSS_PACKET_CMD_DROP_HARD_E] */
        index = 0;
        cmd = CPSS_PACKET_CMD_DROP_HARD_E;

        st = cpssDxChBrgGenIcmpv6MsgTypeSet(dev, index, msgType, cmd);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, cmd = %d", dev, cmd);

        /* Call function with index [0], msgType [0], cmd [CPSS_PACKET_CMD_DROP_SOFT_E] */
        cmd = CPSS_PACKET_CMD_DROP_SOFT_E;

        st = cpssDxChBrgGenIcmpv6MsgTypeSet(dev, index, msgType, cmd);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, cmd = %d", dev, cmd);

        /*
            1.5. Call function with wrong enum values cmd and other parameters from 1.1.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChBrgGenIcmpv6MsgTypeSet
                            (dev, index, msgType, cmd),
                            cmd);
    }

    index = 0;
    msgType = 0;
    cmd = CPSS_PACKET_CMD_FORWARD_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgGenIcmpv6MsgTypeSet(dev, index, msgType, cmd);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgGenIcmpv6MsgTypeSet(dev, index, msgType, cmd);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChBrgGenIcmpv6MsgTypeGet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call function with valid index [0, 1, 6],
                            and not NULL msgType, cmd.
    Expected: GT_OK.
    1.2. Call function with out of range index [8].
    Expected:GT_BAD_PARAM.
    1.3. Call function with NULL msgTypePtr
    Expected: GT_BAD_PTR.
    1.4. Call function with NULL cmdPtr
    Expected: GT_BAD_PTR.
*/
    GT_STATUS           st       = GT_OK;

    GT_U8               dev;
    GT_U32              index   = 0;
    GT_U8               msgType = 0;
    CPSS_PACKET_CMD_ENT cmd     = CPSS_PACKET_CMD_FORWARD_E;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*  1.1.  */
        index = 0;

        st = cpssDxChBrgGenIcmpv6MsgTypeGet(dev, index, &msgType, &cmd);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, index);

        /*  1.1.  */
        index = 1;

        st = cpssDxChBrgGenIcmpv6MsgTypeGet(dev, index, &msgType, &cmd);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, index);

        /*  1.1.  */
        index = 6;

        st = cpssDxChBrgGenIcmpv6MsgTypeGet(dev, index, &msgType, &cmd);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, index);

        /*  1.2.  */
        index = 8;

        st = cpssDxChBrgGenIcmpv6MsgTypeGet(dev, index, &msgType, &cmd);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, index);

        index = 0;

        /*  1.3. */
        st = cpssDxChBrgGenIcmpv6MsgTypeGet(dev, index, NULL, &cmd);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, index);

        /*  1.4. */
        st = cpssDxChBrgGenIcmpv6MsgTypeGet(dev, index, &msgType, NULL);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, index);
    }

    index = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgGenIcmpv6MsgTypeGet(dev, index, &msgType, &cmd);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgGenIcmpv6MsgTypeGet(dev, index, &msgType, &cmd);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgGenIpLinkLocalMirrorToCpuEnable
(
    IN GT_U8                        dev,
    IN CPSS_IP_PROTOCOL_STACK_ENT   protocolStack,
    IN GT_BOOL                      enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgGenIpLinkLocalMirrorToCpuEnable)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call function with valid protocolStack [CPSS_IP_PROTOCOL_IPV4_E,
                                                 CPSS_IP_PROTOCOL_IPV6_E]
                            and state [GT_FALSE and GT_TRUE].
    Expected: GT_OK.
    1.2. Call cpssDxChBrgGenIpLinkLocalMirrorToCpuEnableGet with the same params.
    Expected: GT_OK and the same values.
    1.3. Call function with wrong protocolStack [CPSS_IP_PROTOCOL_IPV4V6_E].
         and other parameters from 1.1.
    Expected: NOT GT_OK.
    1.4. Call function with wrong enum values protocolStack
         and other parameters from 1.1.
    Expected: GT_BAD_PARAM
*/
    GT_STATUS                  st       = GT_OK;

    GT_U8                      dev;
    CPSS_IP_PROTOCOL_STACK_ENT protocol = CPSS_IP_PROTOCOL_IPV4_E;
    GT_BOOL                    state    = GT_FALSE;
    GT_BOOL                    stateGet = GT_FALSE;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with valid protocolStack [CPSS_IP_PROTOCOL_IPV4_E,
                                                         CPSS_IP_PROTOCOL_IPV6_E]
                                    and state [GT_FALSE and GT_TRUE].
            Expected: GT_OK.
        */

        /* Call function with protocol [CPSS_IP_PROTOCOL_IPV4_E], state [GT_FALSE] */
        protocol = CPSS_IP_PROTOCOL_IPV4_E;
        state = GT_FALSE;

        st = cpssDxChBrgGenIpLinkLocalMirrorToCpuEnable(dev, protocol, state);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, protocol, state);

        /*
            1.2. Call cpssDxChBrgGenIpLinkLocalMirrorToCpuEnableGet with the same params.
            Expected: GT_OK and the same values.
        */
        st = cpssDxChBrgGenIpLinkLocalMirrorToCpuEnableGet(dev, protocol, &stateGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
            "cpssDxChBrgGenIpLinkLocalMirrorToCpuEnableGet: %d, %d", dev, protocol);

        /* Verifying values */
        UTF_VERIFY_EQUAL2_STRING_MAC(state, stateGet,
            "get another enable value than was set: %d, %d", dev, protocol);

        /* Call function with protocol [CPSS_IP_PROTOCOL_IPV4_E], state [GT_TRUE] */
        state = GT_TRUE;

        st = cpssDxChBrgGenIpLinkLocalMirrorToCpuEnable(dev, protocol, state);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, protocol, state);

        /* 1.2. Call cpssDxChBrgGenIpLinkLocalMirrorToCpuEnableGet */
        st = cpssDxChBrgGenIpLinkLocalMirrorToCpuEnableGet(dev, protocol, &stateGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
            "cpssDxChBrgGenIpLinkLocalMirrorToCpuEnableGet: %d, %d", dev, protocol);

        /* Verifying values */
        UTF_VERIFY_EQUAL2_STRING_MAC(state, stateGet,
            "get another enable value than was set: %d, %d", dev, protocol);

        /* Call function with protocol [CPSS_IP_PROTOCOL_IPV6_E], state [GT_FALSE] */
        protocol = CPSS_IP_PROTOCOL_IPV6_E;
        state = GT_FALSE;

        st = cpssDxChBrgGenIpLinkLocalMirrorToCpuEnable(dev, protocol, state);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, protocol, state);

        /* 1.2. Call cpssDxChBrgGenIpLinkLocalMirrorToCpuEnableGet */
        st = cpssDxChBrgGenIpLinkLocalMirrorToCpuEnableGet(dev, protocol, &stateGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
            "cpssDxChBrgGenIpLinkLocalMirrorToCpuEnableGet: %d, %d", dev, protocol);

        /* Verifying values */
        UTF_VERIFY_EQUAL2_STRING_MAC(state, stateGet,
            "get another enable value than was set: %d, %d", dev, protocol);

        /* Call function with protocol [CPSS_IP_PROTOCOL_IPV6_E], state [GT_TRUE] */
        state = GT_TRUE;

        st = cpssDxChBrgGenIpLinkLocalMirrorToCpuEnable(dev, protocol, state);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, protocol, state);

        /* 1.2. Call cpssDxChBrgGenIpLinkLocalMirrorToCpuEnableGet */
        st = cpssDxChBrgGenIpLinkLocalMirrorToCpuEnableGet(dev, protocol, &stateGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
            "cpssDxChBrgGenIpLinkLocalMirrorToCpuEnableGet: %d, %d", dev, protocol);

        /* Verifying values */
        UTF_VERIFY_EQUAL2_STRING_MAC(state, stateGet,
            "get another enable value than was set: %d, %d", dev, protocol);


        /*
            1.3. Call function with wrong protocolStack [CPSS_IP_PROTOCOL_IPV4V6_E].
                 and other parameters from 1.1.
            Expected: NOT GT_OK.
        */

        /* Call function with protocol [CPSS_IP_PROTOCOL_IPV4V6_E], state [GT_FALSE] */
        protocol = CPSS_IP_PROTOCOL_IPV4V6_E;
        state = GT_FALSE;

        st = cpssDxChBrgGenIpLinkLocalMirrorToCpuEnable(dev, protocol, state);
        UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, protocol, state);

        /* Call function with protocol [CPSS_IP_PROTOCOL_IPV4V6_E], state [GT_TRUE] */
        state = GT_TRUE;

        st = cpssDxChBrgGenIpLinkLocalMirrorToCpuEnable(dev, protocol, state);
        UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, protocol, state);

        /*
            1.4. Call function with wrong enum values protocolStack
                 and other parameters from 1.1.
            Expected: GT_BAD_PARAM
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChBrgGenIpLinkLocalMirrorToCpuEnable
                            (dev, protocol, state),
                            protocol);
    }

    protocol = CPSS_IP_PROTOCOL_IPV4_E;
    state = GT_TRUE;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgGenIpLinkLocalMirrorToCpuEnable(dev, protocol, state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgGenIpLinkLocalMirrorToCpuEnable(dev, protocol, state);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgGenIpLinkLocalMirrorToCpuEnableGet
(
    IN  GT_U8                        devNum,
    IN  CPSS_IP_PROTOCOL_STACK_ENT   protocolStack,
    OUT GT_BOOL                      *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgGenIpLinkLocalMirrorToCpuEnableGet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call function with valid protocolStack [CPSS_IP_PROTOCOL_IPV4_E,
                                                 CPSS_IP_PROTOCOL_IPV6_E]
    Expected: GT_OK.
    1.2. Call with wrong enum values protocolStack
         and other parameters from 1.1.
    Expected: GT_BAD_PARAM.
    1.3. Call with wrong enablePtr[NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS                  st       = GT_OK;

    GT_U8                      dev;
    CPSS_IP_PROTOCOL_STACK_ENT protocol = CPSS_IP_PROTOCOL_IPV4_E;
    GT_BOOL                    state    = GT_FALSE;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with valid protocolStack [CPSS_IP_PROTOCOL_IPV4_E,
                                                         CPSS_IP_PROTOCOL_IPV6_E].
            Expected: GT_OK.
        */

        /* Call function with protocol [CPSS_IP_PROTOCOL_IPV4_E]*/
        protocol = CPSS_IP_PROTOCOL_IPV4_E;

        st = cpssDxChBrgGenIpLinkLocalMirrorToCpuEnableGet(dev, protocol, &state);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, protocol, state);

        /* Call function with protocol [CPSS_IP_PROTOCOL_IPV4_E]*/
        st = cpssDxChBrgGenIpLinkLocalMirrorToCpuEnableGet(dev, protocol, &state);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, protocol, state);

        /*
            1.2. Call function with wrong enum values protocolStack
                 and other parameters from 1.1.
            Expected: GT_BAD_PARAM
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChBrgGenIpLinkLocalMirrorToCpuEnableGet
                            (dev, protocol, &state),
                            protocol);

        /*
            1.3. Call with wrong enablePtr[NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChBrgGenIpLinkLocalMirrorToCpuEnableGet(dev, protocol, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, enablePtr = NULL", dev);
    }

    protocol = CPSS_IP_PROTOCOL_IPV4_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgGenIpLinkLocalMirrorToCpuEnableGet(dev, protocol, &state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgGenIpLinkLocalMirrorToCpuEnableGet(dev, protocol, &state);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgGenIpLinkLocalProtMirrorToCpuEnable
(
    IN GT_U8                        dev,
    IN CPSS_IP_PROTOCOL_STACK_ENT   protocolStack,
    IN GT_U8                        protocol,
    IN GT_BOOL                      enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgGenIpLinkLocalProtMirrorToCpuEnable)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call function with valid protocolStack [CPSS_IP_PROTOCOL_IPV4_E,
                                                 CPSS_IP_PROTOCOL_IPV6_E],
                                  protocol [0, 250]
                                  and enable [GT_FALSE and GT_TRUE].
    Expected: GT_OK.
    1.2. Call cpssDxChBrgGenIpLinkLocalProtMirrorToCpuEnableGet
    Expected: GT_OK and same enablePtr.
    1.3. Call function with out of range protocolStack and other parameters from 1.1.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS                  st       = GT_OK;

    GT_U8                      dev;
    CPSS_IP_PROTOCOL_STACK_ENT prStack  = CPSS_IP_PROTOCOL_IPV4_E;
    GT_U8                      protocol = 0;
    GT_BOOL                    state    = GT_FALSE;
    GT_BOOL                    stateGet    = GT_FALSE;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with valid protocolStack [CPSS_IP_PROTOCOL_IPV4_E /
                                                         CPSS_IP_PROTOCOL_IPV6_E],
                                    protocol [0, 250]
                                    and enable [GT_FALSE and GT_TRUE].
            Expected: GT_OK for all DxCh devices and NON GT_OK for others.
        */

        /* Call function with protocolStack [CPSS_IP_PROTOCOL_IPV4_E] */
        /* protocol [0] state [GT_FALSE]                              */
        prStack = CPSS_IP_PROTOCOL_IPV4_E;
        protocol = 0;
        state = GT_FALSE;

        st = cpssDxChBrgGenIpLinkLocalProtMirrorToCpuEnable(dev, prStack, protocol, state);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, prStack, protocol, state);

        /*  1.2.  */
        st = cpssDxChBrgGenIpLinkLocalProtMirrorToCpuEnableGet(dev, prStack, protocol, &stateGet);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, prStack, protocol);

        UTF_VERIFY_EQUAL1_STRING_MAC(state, stateGet,
            "get another state than was set: %d", dev);

        /* Call function with protocolStack [CPSS_IP_PROTOCOL_IPV4_E] */
        /* protocol [0] state [GT_TRUE]                              */
        state = GT_TRUE;

        st = cpssDxChBrgGenIpLinkLocalProtMirrorToCpuEnable(dev, prStack, protocol, state);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, prStack, protocol, state);

        /*  1.2.  */
        st = cpssDxChBrgGenIpLinkLocalProtMirrorToCpuEnableGet(dev, prStack, protocol, &stateGet);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, prStack, protocol);

        UTF_VERIFY_EQUAL1_STRING_MAC(state, stateGet,
            "get another state than was set: %d", dev);


        /* Call function with protocolStack [CPSS_IP_PROTOCOL_IPV6_E] */
        /* protocol [250] state [GT_FALSE]                            */
        prStack = CPSS_IP_PROTOCOL_IPV6_E;
        protocol = 250;
        state = GT_FALSE;

        st = cpssDxChBrgGenIpLinkLocalProtMirrorToCpuEnable(dev, prStack, protocol, state);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, prStack, protocol, state);

        /*  1.2.  */
        st = cpssDxChBrgGenIpLinkLocalProtMirrorToCpuEnableGet(dev, prStack, protocol, &stateGet);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, prStack, protocol);

        UTF_VERIFY_EQUAL1_STRING_MAC(state, stateGet,
            "get another state than was set: %d", dev);

        /* Call function with protocolStack [CPSS_IP_PROTOCOL_IPV6_E] */
        /* protocol [250] state [GT_TRUE]                             */
        state = GT_TRUE;

        st = cpssDxChBrgGenIpLinkLocalProtMirrorToCpuEnable(dev, prStack, protocol, state);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, prStack, protocol, state);

        /*  1.2.  */
        st = cpssDxChBrgGenIpLinkLocalProtMirrorToCpuEnableGet(dev, prStack, protocol, &stateGet);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, prStack, protocol);

        UTF_VERIFY_EQUAL1_STRING_MAC(state, stateGet,
            "get another state than was set: %d", dev);

        /*
            1.3. Call function with wrong enum values protocolStack
                 and other parameters from 1.1.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChBrgGenIpLinkLocalProtMirrorToCpuEnable
                            (dev, prStack, protocol, state),
                            prStack);
    }

    prStack = CPSS_IP_PROTOCOL_IPV4_E;
    protocol = 0;
    state = GT_FALSE;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgGenIpLinkLocalProtMirrorToCpuEnable(dev, prStack, protocol, state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgGenIpLinkLocalProtMirrorToCpuEnable(dev, prStack, protocol, state);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChBrgGenIpLinkLocalProtMirrorToCpuEnableGet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call function with valid protocolStack [CPSS_IP_PROTOCOL_IPV4_E,
                                                 CPSS_IP_PROTOCOL_IPV6_E],
                                  protocol [0, 250] and not NULL enablePtr.
    Expected: GT_OK.
    1.2. Call function with out of range protocolStack and other parameters from 1.1.
    Expected: GT_BAD_PARAM.
    1.3. Call function with NULL enablePtr.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS                  st       = GT_OK;

    GT_U8                      dev;
    CPSS_IP_PROTOCOL_STACK_ENT prStack  = CPSS_IP_PROTOCOL_IPV4_E;
    GT_U8                      protocol = 0;
    GT_BOOL                    state    = GT_FALSE;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*  1.1. */
        prStack = CPSS_IP_PROTOCOL_IPV4_E;
        protocol = 0;

        st = cpssDxChBrgGenIpLinkLocalProtMirrorToCpuEnableGet(dev, prStack, protocol, &state);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, prStack, protocol);

        /*  1.1. */
        prStack = CPSS_IP_PROTOCOL_IPV6_E;
        protocol = 250;

        st = cpssDxChBrgGenIpLinkLocalProtMirrorToCpuEnableGet(dev, prStack, protocol, &state);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, prStack, protocol);

        /*  1.2.  */
        UTF_ENUMS_CHECK_MAC(cpssDxChBrgGenIpLinkLocalProtMirrorToCpuEnableGet
                            (dev, prStack, protocol, &state),
                            prStack);

        /*  1.3. */
        st = cpssDxChBrgGenIpLinkLocalProtMirrorToCpuEnableGet(dev, prStack, protocol, NULL);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PTR, st, dev, prStack, protocol);
    }

    prStack = CPSS_IP_PROTOCOL_IPV4_E;
    protocol = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgGenIpLinkLocalProtMirrorToCpuEnableGet(dev, prStack, protocol, &state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgGenIpLinkLocalProtMirrorToCpuEnableGet(dev, prStack, protocol, &state);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgGenIpV6SolicitedCmdSet
(
    IN GT_U8                dev,
    IN CPSS_PACKET_CMD_ENT  cmd
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgGenIpV6SolicitedCmdSet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call function with cmd [CPSS_PACKET_CMD_FORWARD_E /
                                 CPSS_PACKET_CMD_MIRROR_TO_CPU_E /
                                 CPSS_PACKET_CMD_TRAP_TO_CPU_E]
    Expected: GT_OK.
    1.2. Call cpssDxChBrgGenIpV6SolicitedCmdGet
    Expected: GT_OK and same cmdPtr.
    1.3. Call function with cmd [CPSS_PACKET_CMD_DROP_HARD_E] (not supported).
    Expected: NOT GT_OK.
    1.4. For Cheetah call function with out-of-range cmd.
    Expected: GT_BAD_PARAM
*/
    GT_STATUS           st  = GT_OK;

    GT_U8               dev;
    CPSS_PACKET_CMD_ENT cmd = CPSS_PACKET_CMD_FORWARD_E;
    CPSS_PACKET_CMD_ENT cmdGet = CPSS_PACKET_CMD_FORWARD_E;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with cmd [CPSS_PACKET_CMD_FORWARD_E /
                                         CPSS_PACKET_CMD_MIRROR_TO_CPU_E /
                                         CPSS_PACKET_CMD_TRAP_TO_CPU_E /
                                         CPSS_PACKET_CMD_DROP_SOFT_E]
            Expected: GT_OK.
        */

        /* Call function with cmd [CPSS_PACKET_CMD_FORWARD_E] */
        cmd = CPSS_PACKET_CMD_FORWARD_E;

        st = cpssDxChBrgGenIpV6SolicitedCmdSet(dev, cmd);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, cmd);

        /*  1.2.  */
        st = cpssDxChBrgGenIpV6SolicitedCmdGet(dev, &cmdGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        UTF_VERIFY_EQUAL1_STRING_MAC(cmd, cmdGet,
            "get another cmd than was set: %d", dev);

        /* Call function with cmd [CPSS_PACKET_CMD_MIRROR_TO_CPU_E] */
        cmd = CPSS_PACKET_CMD_MIRROR_TO_CPU_E;

        st = cpssDxChBrgGenIpV6SolicitedCmdSet(dev, cmd);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, cmd);

        /*  1.2.  */
        st = cpssDxChBrgGenIpV6SolicitedCmdGet(dev, &cmdGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        UTF_VERIFY_EQUAL1_STRING_MAC(cmd, cmdGet,
            "get another cmd than was set: %d", dev);

        /* Call function with cmd [CPSS_PACKET_CMD_TRAP_TO_CPU_E] */
        cmd = CPSS_PACKET_CMD_TRAP_TO_CPU_E;

        st = cpssDxChBrgGenIpV6SolicitedCmdSet(dev, cmd);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, cmd);

        /*  1.2.  */
        st = cpssDxChBrgGenIpV6SolicitedCmdGet(dev, &cmdGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        UTF_VERIFY_EQUAL1_STRING_MAC(cmd, cmdGet,
            "get another cmd than was set: %d", dev);

        /*
            1.3. Call function with cmd [CPSS_PACKET_CMD_DROP_HARD_E] (not supported).
            Expected: NOT GT_OK.
        */
        cmd = CPSS_PACKET_CMD_DROP_HARD_E;

        st = cpssDxChBrgGenIpV6SolicitedCmdSet(dev, cmd);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, cmd);

        /*
            1.4. Call function with wrong enum values cmd.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChBrgGenIpV6SolicitedCmdSet
                            (dev, cmd),
                            cmd);
    }

    cmd = CPSS_PACKET_CMD_FORWARD_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgGenIpV6SolicitedCmdSet(dev, cmd);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgGenIpV6SolicitedCmdSet(dev, cmd);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChBrgGenIpV6SolicitedCmdGet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call function with not NULL cmd
    Expected: GT_OK.
    1.2. Call function with NULL cmd
    Expected: GT_BAD_PTR.
*/
    GT_STATUS           st  = GT_OK;

    GT_U8               dev;
    CPSS_PACKET_CMD_ENT cmd = CPSS_PACKET_CMD_FORWARD_E;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*  1.1. */
        st = cpssDxChBrgGenIpV6SolicitedCmdGet(dev, &cmd);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.2. */
        st = cpssDxChBrgGenIpV6SolicitedCmdGet(dev, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgGenIpV6SolicitedCmdGet(dev, &cmd);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgGenIpV6SolicitedCmdGet(dev, &cmd);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgGenIpV6SolicitedNodeMcastPrefixSet
(
    IN GT_U8        dev,
    IN GT_IPV6ADDR  *addressPtr,
    IN GT_IPV6ADDR  *maskPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgGenIpV6SolicitedNodeMcastPrefixSet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call function with addressPtr[0,...,0,254,12], *maskPtr[0,…,0,255,255]
    Expected: GT_OK
    1.2. Call cpssDxChBrgGenIpV6SolicitedNodeMcastPrefixGet
    Expected: GT_OK and same addressPtr, maskPtr.
    1.3.  Call function with maskPtr[NULL] and other parameters from 1.1.
    Expected: GT_BAD_PTR
    1.4.  Call function with addressPtr[NULL] and other parameters from 1.1.
    Expected: GT_BAD_PTR
*/
    GT_STATUS    st  = GT_OK;

    GT_U8        dev;
    GT_IPV6ADDR  addr;
    GT_IPV6ADDR  mask;
    GT_IPV6ADDR  addrGet = {{0}};
    GT_IPV6ADDR  maskGet = {{0}};

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*  1.1. */
        addr.u32Ip[0] = 10;
        addr.u32Ip[1] = 0;
        addr.u32Ip[2] = 254;
        addr.u32Ip[3] = 12;

        mask.u32Ip[0] = 255;
        mask.u32Ip[1] = 255;
        mask.u32Ip[2] = 0;
        mask.u32Ip[3] = 0;

        st = cpssDxChBrgGenIpV6SolicitedNodeMcastPrefixSet(dev, &addr, &mask);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.2.   */
        st = cpssDxChBrgGenIpV6SolicitedNodeMcastPrefixGet(dev, &addrGet, &maskGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        UTF_VERIFY_EQUAL1_STRING_MAC(addr.u32Ip[0], addrGet.u32Ip[0],
            "get another addr.u32Ip[0] than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(addr.u32Ip[1], addrGet.u32Ip[1],
            "get another addr.u32Ip[1] than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(addr.u32Ip[2], addrGet.u32Ip[2],
            "get another addr.u32Ip[2] than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(addr.u32Ip[3], addrGet.u32Ip[3],
            "get another addr.u32Ip[3] than was set: %d", dev);

        UTF_VERIFY_EQUAL1_STRING_MAC(mask.u32Ip[0], maskGet.u32Ip[0],
            "get another mask.u32Ip[0] than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(mask.u32Ip[1], maskGet.u32Ip[1],
            "get another mask.u32Ip[0] than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(mask.u32Ip[2], maskGet.u32Ip[2],
            "get another mask.u32Ip[0] than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(mask.u32Ip[3], maskGet.u32Ip[3],
            "get another mask.u32Ip[0] than was set: %d", dev);

        /*  1.1. */
        addr.u32Ip[0] = 0xABABABAB;
        addr.u32Ip[1] = 0x1C1C1C1C;
        addr.u32Ip[2] = 0xABABABAB;
        addr.u32Ip[3] = 0x1C1C1C1C;

        mask.u32Ip[0] = 0xFFFFFFFF;
        mask.u32Ip[1] = 0xFFFFFFFF;
        mask.u32Ip[2] = 0xFFFF0000;
        mask.u32Ip[3] = 0;

        st = cpssDxChBrgGenIpV6SolicitedNodeMcastPrefixSet(dev, &addr, &mask);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.2.   */
        st = cpssDxChBrgGenIpV6SolicitedNodeMcastPrefixGet(dev, &addrGet, &maskGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        UTF_VERIFY_EQUAL1_STRING_MAC(addr.u32Ip[0], addrGet.u32Ip[0],
            "get another addr.u32Ip[0] than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(addr.u32Ip[1], addrGet.u32Ip[1],
            "get another addr.u32Ip[1] than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(addr.u32Ip[2], addrGet.u32Ip[2],
            "get another addr.u32Ip[2] than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(addr.u32Ip[3], addrGet.u32Ip[3],
            "get another addr.u32Ip[3] than was set: %d", dev);

        UTF_VERIFY_EQUAL1_STRING_MAC(mask.u32Ip[0], maskGet.u32Ip[0],
            "get another mask.u32Ip[0] than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(mask.u32Ip[1], maskGet.u32Ip[1],
            "get another mask.u32Ip[0] than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(mask.u32Ip[2], maskGet.u32Ip[2],
            "get another mask.u32Ip[0] than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(mask.u32Ip[3], maskGet.u32Ip[3],
            "get another mask.u32Ip[0] than was set: %d", dev);

        /*  1.1. */
        addr.u32Ip[0] = 0xFFFFFFFF;
        addr.u32Ip[1] = 0xFFFFFFFF;
        addr.u32Ip[2] = 0xFFFFFFFF;
        addr.u32Ip[3] = 0xFFFFFFFF;

        mask.u32Ip[0] = 0xFFFFFFFF;
        mask.u32Ip[1] = 0xFFFFFFFF;
        mask.u32Ip[2] = 0xFFFFFFFF;
        mask.u32Ip[3] = 0xFFFFFFFF;

        st = cpssDxChBrgGenIpV6SolicitedNodeMcastPrefixSet(dev, &addr, &mask);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.2.   */
        st = cpssDxChBrgGenIpV6SolicitedNodeMcastPrefixGet(dev, &addrGet, &maskGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        UTF_VERIFY_EQUAL1_STRING_MAC(addr.u32Ip[0], addrGet.u32Ip[0],
            "get another addr.u32Ip[0] than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(addr.u32Ip[1], addrGet.u32Ip[1],
            "get another addr.u32Ip[1] than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(addr.u32Ip[2], addrGet.u32Ip[2],
            "get another addr.u32Ip[2] than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(addr.u32Ip[3], addrGet.u32Ip[3],
            "get another addr.u32Ip[3] than was set: %d", dev);

        UTF_VERIFY_EQUAL1_STRING_MAC(mask.u32Ip[0], maskGet.u32Ip[0],
            "get another mask.u32Ip[0] than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(mask.u32Ip[1], maskGet.u32Ip[1],
            "get another mask.u32Ip[0] than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(mask.u32Ip[2], maskGet.u32Ip[2],
            "get another mask.u32Ip[0] than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(mask.u32Ip[3], maskGet.u32Ip[3],
            "get another mask.u32Ip[0] than was set: %d", dev);

        /*
            1.3.  Call function with maskPtr[NULL] and other parameters from 1.1.
            Expected: GT_BAD_PTR
        */
        st = cpssDxChBrgGenIpV6SolicitedNodeMcastPrefixSet(dev, &addr, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, maskPtr = NULL", dev);

        /*
            1.4.  Call function with addressPtr[NULL] and other parameters from 1.1.
            Expected: GT_BAD_PTR
        */
        st = cpssDxChBrgGenIpV6SolicitedNodeMcastPrefixSet(dev, NULL, &mask);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, addressPtr = NULL", dev);
    }

    addr.u32Ip[0] = 10;
    addr.u32Ip[1] = 0;
    addr.u32Ip[2] = 254;
    addr.u32Ip[3] = 12;

    mask.u32Ip[0] = 255;
    mask.u32Ip[1] = 255;
    mask.u32Ip[2] = 0;
    mask.u32Ip[3] = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgGenIpV6SolicitedNodeMcastPrefixSet(dev, &addr, &mask);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgGenIpV6SolicitedNodeMcastPrefixSet(dev, &addr, &mask);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChBrgGenIpV6SolicitedNodeMcastPrefixGet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call function with not NULL addressPtr and maskPtr
    Expected: GT_OK
    1.2.  Call function with maskPtr[NULL] and other parameters from 1.1.
    Expected: GT_BAD_PTR
    1.3.  Call function with addressPtr[NULL] and other parameters from 1.1.
    Expected: GT_BAD_PTR
*/
    GT_STATUS    st  = GT_OK;

    GT_U8        dev;
    GT_IPV6ADDR  addr;
    GT_IPV6ADDR  mask;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*  1.1.  */
        addr.u32Ip[0] = 10;
        addr.u32Ip[1] = 0;
        addr.u32Ip[2] = 254;
        addr.u32Ip[3] = 12;

        mask.u32Ip[0] = 255;
        mask.u32Ip[1] = 255;
        mask.u32Ip[2] = 0;
        mask.u32Ip[3] = 0;

        st = cpssDxChBrgGenIpV6SolicitedNodeMcastPrefixGet(dev, &addr, &mask);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.2.  */
        st = cpssDxChBrgGenIpV6SolicitedNodeMcastPrefixGet(dev, &addr, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, maskPtr = NULL", dev);

        /*  1.3.  */
        st = cpssDxChBrgGenIpV6SolicitedNodeMcastPrefixGet(dev, NULL, &mask);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, addressPtr = NULL", dev);
    }

    addr.u32Ip[0] = 10;
    addr.u32Ip[1] = 0;
    addr.u32Ip[2] = 254;
    addr.u32Ip[3] = 12;

    mask.u32Ip[0] = 255;
    mask.u32Ip[1] = 255;
    mask.u32Ip[2] = 0;
    mask.u32Ip[3] = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgGenIpV6SolicitedNodeMcastPrefixGet(dev, &addr, &mask);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgGenIpV6SolicitedNodeMcastPrefixGet(dev, &addr, &mask);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

















/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgGenRateLimitGlobalCfgSet
(
    IN GT_U8                                dev,
    IN CPSS_DXCH_BRG_GEN_RATE_LIMIT_STC *   brgRateLimitPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgGenRateLimitGlobalCfgSet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call function with brgRateLimitPtr[CPSS_DROP_MODE_SOFT_E,
                                            CPSS_RATE_LIMIT_PCKT_BASED_E,
                            0, 1*granularity, 2*granularity, 3*granularity],
                            where:
                            for DxCh  granularity = 256 * 200 / coreClock(dev).
                            for DxCh2 granularity = 256 * 220 / coreClock(dev).
                            for DxCh3 granularity = 256.
    Expected: GT_OK.
    1.2. Call function with brgRateLimitPtr[CPSS_DROP_MODE_HARD_E,
                                            CPSS_RATE_LIMIT_BYTE_BASED_E,
                            0, 1*granularity, 2*granularity, 3*granularity ]
    Expected: GT_OK.
    1.3. Call cpssDxChBrgGenRateLimitGlobalCfgGet.
    Expected: GT_OK and the same brgRateLimitPtr.
    1.4. Call with out of range brgRateLimitPtr-> dropMode:
         brgRateLimitPtr[3, CPSS_RATE_LIMIT_PCKT_BASED_E, 0,
                         1*granularity, 2*granularity, 3*granularity]
    Expected: GT_BAD_PARAM.
    1.5. Call with out of range  brgRateLimitPtr-> rMode:
         brgRateLimitPtr[brgRateLimitPtr[CPSS_DROP_MODE_SOFT_E, 3, 0, 256, 512, 768]
    Expected: GT_BAD_PARAM.
    1.6. Call with brgRateLimitPtr[brgRateLimitPtr->win1000Mbps
         = (16128+256) *200/ coreClock(dev) = ((16128 / 256) + 1) * granularity.
    Expected: NON GT_OK.
    1.7. Call with brgRateLimitPtr[brgRateLimitPtr->win100Mbps
                                                = (130816+256)*200 / coreClock(dev).
    Expected: NON GT_OK.
    1.8. Call with brgRateLimitPtr[brgRateLimitPtr->win10Mbps=
                                                    (1048320+256) *200/ coreClock(dev).
    Expected:. NON GT_OK.
    1.9. Call with brgRateLimitPtr[brgRateLimitPtr->win10Gbps =
                                            (104832.0+25.6) *200/ (coreClock(dev) * 10).
    Expected: NON GT_OK.
    1.10. Call with brgRateLimitPtr[brgRateLimitPtr->win1000Mbps =
                               16128 *200/ coreClock(dev) = (16128 / 256) * granularity.
    Expected: GT_OK.
    1.11. Call with brgRateLimitPtr[brgRateLimitPtr->win100Mbps = 130816 *200/ coreClock(dev).
    Expected: GT_OK.
    1.12. Call with brgRateLimitPtr[brgRateLimitPtr->win10Mbps=  1048320 *200/ coreClock(dev).
    Expected: GT_OK.
    1.13. Call function with brgRateLimitPtr[NULL]
    Expected: GT_BAD_PTR
*/
    GT_STATUS  st    = GT_OK;

    GT_U8                               dev;
    CPSS_DXCH_BRG_GEN_RATE_LIMIT_STC    rateLimit;
    CPSS_DXCH_BRG_GEN_RATE_LIMIT_STC    rateGet;
    GT_U32                              coreClock     = 0;
    GT_U32                              granularity   = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_LION_E | UTF_LION2_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* Calculate device granularity */
        /* The granularity doesn't depend from PP core clock for DxCh3 and above */
        if(PRV_CPSS_PP_MAC(dev)->devFamily >= CPSS_PP_FAMILY_CHEETAH3_E)
        {
            granularity = 256;
        }
        else
        {
            coreClock = PRV_CPSS_PP_MAC(dev)->baseCoreClock;

            granularity = (256 * coreClock) / PRV_CPSS_PP_MAC(dev)->coreClock;
        }

        /*
            1.1. Call function with brgRateLimitPtr[CPSS_DROP_MODE_SOFT_E,
            CPSS_RATE_LIMIT_PCKT_BASED_E, 0, 1*granularity, 2*granularity, 3*granularity],
            where granularity = 256 * 200 / coreClock(dev).
            Expected: GT_OK.
        */
        rateLimit.dropMode    = CPSS_DROP_MODE_SOFT_E;
        rateLimit.rMode       = CPSS_RATE_LIMIT_PCKT_BASED_E;
        rateLimit.win10Mbps   = 0;
        rateLimit.win100Mbps  = 1 * granularity;
        rateLimit.win1000Mbps = 2 * granularity;
        rateLimit.win10Gbps   = 3 * granularity;

        st = cpssDxChBrgGenRateLimitGlobalCfgSet(dev, &rateLimit);
        UTF_VERIFY_EQUAL7_STRING_MAC(GT_OK, st, "%d, brgRateLimitPtr: dropMode = %d"
                                     "rMode = %d"
                                     "win10Mbps = %d"
                                     "win100Mbps = %d"
                                     "win1000Mbps = %d"
                                     "win10Gbps = %d", dev, rateLimit.dropMode,
                                     rateLimit.rMode, rateLimit.win10Mbps, rateLimit.win100Mbps,
                                     rateLimit.win1000Mbps, rateLimit.win10Gbps);

        /*
            1.2. Call function with brgRateLimitPtr[CPSS_DROP_MODE_HARD_E,
                                                    CPSS_RATE_LIMIT_BYTE_BASED_E,
                                                    0, 1, 2, 3]
            Expected: GT_OK.
        */
        rateLimit.dropMode = CPSS_DROP_MODE_HARD_E;
        rateLimit.rMode    = CPSS_RATE_LIMIT_BYTE_BASED_E;

        st = cpssDxChBrgGenRateLimitGlobalCfgSet(dev, &rateLimit);
        UTF_VERIFY_EQUAL7_STRING_MAC(GT_OK, st, "%d, brgRateLimitPtr: dropMode = %d"
                                     "rMode = %d"
                                     "win10Mbps = %d"
                                     "win100Mbps = %d"
                                     "win1000Mbps = %d"
                                     "win10Gbps = %d", dev, rateLimit.dropMode,
                                     rateLimit.rMode, rateLimit.win10Mbps, rateLimit.win100Mbps,
                                     rateLimit.win1000Mbps, rateLimit.win10Gbps);

        /*
            1.3. Call cpssDxChBrgGenRateLimitGlobalCfgGet.
            Expected: GT_OK and the same brgRateLimitPtr.
        */

        st = cpssDxChBrgGenRateLimitGlobalCfgGet(dev, &rateGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                     "cpssDxChBrgGenRateLimitGlobalCfgGet: %d", dev);

        /* Verifying brgRateLimitPtr */
        UTF_VERIFY_EQUAL1_STRING_MAC(rateLimit.dropMode , rateGet.dropMode,
                                     "get another brgRateLimitPtr->dropMode than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(rateLimit.rMode, rateGet.rMode,
                                     "get another brgRateLimitPtr->rMode than was set: %d", dev);

        /*
            1.4. Call with wrong enum values brgRateLimitPtr->dropMode:
                                        brgRateLimitPtr[3, CPSS_RATE_LIMIT_PCKT_BASED_E,
                                                        0, 1, 2, 3]
            Expected: GT_BAD_PARAM.
        */
        rateLimit.rMode = CPSS_RATE_LIMIT_PCKT_BASED_E;

        UTF_ENUMS_CHECK_MAC(cpssDxChBrgGenRateLimitGlobalCfgSet
                            (dev, &rateLimit),
                            rateLimit.dropMode);

        rateLimit.dropMode = CPSS_DROP_MODE_SOFT_E;

        /*
            1.5. Call with wrong enum values brgRateLimitPtr->rMode:
                                        brgRateLimitPtr[brgRateLimitPtr[CPSS_DROP_MODE_SOFT_E,
                                                                        3, 0, 1, 2, 3]
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChBrgGenRateLimitGlobalCfgSet
                            (dev, &rateLimit),
                            rateLimit.rMode);

        rateLimit.rMode = CPSS_RATE_LIMIT_PCKT_BASED_E;

        /*
            1.6. Call with brgRateLimitPtr[brgRateLimitPtr[CPSS_DROP_MODE_SOFT_E,
                                                           CPSS_RATE_LIMIT_PCKT_BASED_E,
                                                           2, 1, 16384 / coreClock(dev), 3]
            Expected: NON GT_OK.
        */
        rateLimit.win1000Mbps = ((16128 / 256) + 2) * granularity;

        st = cpssDxChBrgGenRateLimitGlobalCfgSet(dev, &rateLimit);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, rateLimit.win1000Mbps);

        rateLimit.win1000Mbps = 2 * granularity; /* restore */

        /*
            1.7. Call with brgRateLimitPtr[CPSS_DROP_MODE_SOFT_E,
                                           CPSS_RATE_LIMIT_PCKT_BASED_E,
                                           0, 131072 / coreClock(dev), 2, 3]
            Expected: NON GT_OK.
        */
        rateLimit.win100Mbps = ((130816 / 256) + 2) * granularity;

        st = cpssDxChBrgGenRateLimitGlobalCfgSet(dev, &rateLimit);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st,
                "%d, brgRateLimitPtr->win100Mbps = %d", dev, rateLimit.win100Mbps);

        rateLimit.win100Mbps = 1 * granularity; /* restore */

        /*
            1.8. Call with brgRateLimitPtr[brgRateLimitPtr[CPSS_DROP_MODE_SOFT_E,
                                                           CPSS_RATE_LIMIT_PCKT_BASED_E,
                                                           1048577 / coreClock(dev), 1,  2, 3]
            Expected:. NON GT_OK.
        */
        rateLimit.win10Mbps  = ((1048320 / 256) + 2) * granularity;

        st = cpssDxChBrgGenRateLimitGlobalCfgSet(dev, &rateLimit);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st,
                "%d, brgRateLimitPtr->win10Mbps = %d", dev, rateLimit.win10Mbps);

        rateLimit.win10Mbps = 0; /* restore */

        /*
            1.9. Call with brgRateLimitPtr[CPSS_DROP_MODE_SOFT_E,
                                           CPSS_RATE_LIMIT_PCKT_BASED_E,
                                           0, 1, 2, 104858 / coreClock(dev)]
            Expected: NON GT_OK.
        */
        rateLimit.win10Gbps = ((1048320 / 256) + 2) * granularity / 10 + 1;

        st = cpssDxChBrgGenRateLimitGlobalCfgSet(dev, &rateLimit);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, brgRateLimitPtr->win10Gbps = %d", dev, rateLimit.win10Gbps);

        rateLimit.win10Gbps = 3 * granularity; /* restore */

        /*
            1.10. Call with brgRateLimitPtr[CPSS_DROP_MODE_SOFT_E,
                                            CPSS_RATE_LIMIT_PCKT_BASED_E,
                                            0, 1, 2560 / coreClock(dev), 3]
            Expected: GT_OK
        */
        if(PRV_CPSS_PP_MAC(dev)->devFamily > CPSS_PP_FAMILY_CHEETAH_E)
        {
            rateLimit.win1000Mbps = (16128 / 256) * granularity;
        }
        else
        {
            rateLimit.win1000Mbps = (7936 / 256) * granularity;
        }

        st = cpssDxChBrgGenRateLimitGlobalCfgSet(dev, &rateLimit);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                "%d, brgRateLimitPtr->win1000Mbps = %d", dev, rateLimit.win1000Mbps);

        rateLimit.win1000Mbps = 2 * granularity; /* restore */

        /*
            1.11. Call with brgRateLimitPtr[brgRateLimitPtr[CPSS_DROP_MODE_SOFT_E,
                                                            CPSS_RATE_LIMIT_PCKT_BASED_E,
                                                            0, 130817 / coreClock(dev), 2, 3]
            Expected: GT_OK
        */
        rateLimit.win100Mbps = (130816 / 256) * granularity;

        st = cpssDxChBrgGenRateLimitGlobalCfgSet(dev, &rateLimit);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                "%d, brgRateLimitPtr->win100Mbps = %d", dev, rateLimit.win100Mbps);

        rateLimit.win100Mbps = 1 * granularity; /* restore */

        /*
            1.12. Call with brgRateLimitPtr[CPSS_DROP_MODE_SOFT_E,
                                            CPSS_RATE_LIMIT_PCKT_BASED_E,
                                            1048576 / coreClock(dev), 1, 2, 3]
            Expected: GT_OK
        */
        rateLimit.win10Mbps = (1048320 / 256) * granularity;

        st = cpssDxChBrgGenRateLimitGlobalCfgSet(dev, &rateLimit);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                "%d, brgRateLimitPtr->win10Mbps = %d", dev, rateLimit.win10Mbps);

        rateLimit.win10Mbps = 0;

        /*
            1.13. Call function with brgRateLimitPtr[NULL]
            Expected: GT_BAD_PTR
        */
        st = cpssDxChBrgGenRateLimitGlobalCfgSet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, NULL", dev);
    }

    rateLimit.dropMode    = CPSS_DROP_MODE_SOFT_E;
    rateLimit.rMode       = CPSS_RATE_LIMIT_PCKT_BASED_E;
    rateLimit.win10Mbps   = 0;
    rateLimit.win100Mbps  = 256;
    rateLimit.win1000Mbps = 512;
    rateLimit.win10Gbps   = 768;

    /* 2. For not-active devices  */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_LION_E | UTF_LION2_E);
    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgGenRateLimitGlobalCfgSet(dev, &rateLimit);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/

    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgGenRateLimitGlobalCfgSet(dev, &rateLimit);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgGenRateLimitGlobalCfgGet
(
    IN GT_U8                                dev,
    OUT CPSS_DXCH_BRG_GEN_RATE_LIMIT_STC *  brgRateLimitPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgGenRateLimitGlobalCfgGet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with non-null brgRateLimitPtr.
    Expected: GT_OK.
    1.2. Call with brgRateLimitPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS                         st = GT_OK;

    GT_U8                             dev;
    CPSS_DXCH_BRG_GEN_RATE_LIMIT_STC  rateLimit;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_LION_E | UTF_LION2_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with non-null brgRateLimitPtr.
            Expected: GT_OK.
        */
        st = cpssDxChBrgGenRateLimitGlobalCfgGet(dev, &rateLimit);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with brgRateLimitPtr [NULL].
            Expected: GT_BAD_PTR.
        */

        st = cpssDxChBrgGenRateLimitGlobalCfgGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_LION_E | UTF_LION2_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgGenRateLimitGlobalCfgGet(dev, &rateLimit);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/

    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgGenRateLimitGlobalCfgGet(dev, &rateLimit);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgGenPortRateLimitSet
(
    IN GT_U8                                    dev,
    IN GT_U8                                    port,
    IN CPSS_DXCH_BRG_GEN_RATE_LIMIT_PORT_STC *  portGfgPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgGenPortRateLimitSet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (DxChx)
    1.1.1. Call with valid portGfgPtr [GT_TRUE, GT_TRUE, GT_TRUE, GT_TRUE, 0].
    Expected: GT_OK.
    1.1.2. Call cpssDxChBrgGenPortRateLimitGet.
    Expected: GT_OK and the same portGfgPtr.
    1.1.3. Call with valid portGfgPtr [GT_FALSE, GT_FALSE, GT_FALSE, GT_TRUE, 65535].
    Expected: GT_OK.
    1.1.4. Call cpssDxChBrgGenPortRateLimitGet.
    Expected: GT_OK and the same portGfgPtr.
    1.1.5. Call with valid portGfgPtr [GT_FALSE, GT_TRUE, GT_FALSE, GT_FALSE, 65535].
    Expected: GT_OK.
    1.1.6. Call cpssDxChBrgGenPortRateLimitGet.
    Expected: GT_OK and the same portGfgPtr.
    1.1.7. Call with portGfgPtr [NULL]
    Expected: GT_BAD_PTR.
    1.1.8. Call with portCfg [GT_TRUE, GT_TRUE, GT_TRUE, GT_TRUE, 65536]
    Expected: NOT GT_OK.
*/
    GT_STATUS                             st    = GT_OK;
    GT_U8                                 dev;
    GT_PHYSICAL_PORT_NUM                  port  = BRG_GEN_VALID_PHY_PORT_CNS;
    CPSS_DXCH_BRG_GEN_RATE_LIMIT_PORT_STC portCfg;
    CPSS_DXCH_BRG_GEN_RATE_LIMIT_PORT_STC portCfgGet;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_LION_E | UTF_LION2_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call with valid portGfgPtr [GT_TRUE, GT_TRUE, GT_TRUE, GT_TRUE, 0].
                Expected: GT_OK.
            */

            portCfg.enableBc      = GT_TRUE;
            portCfg.enableMc      = GT_TRUE;
            portCfg.enableUcKnown = GT_TRUE;
            portCfg.enableUcUnk   = GT_TRUE;
            portCfg.enableMcReg   = GT_TRUE;
            portCfg.rateLimit     = 0;

            st = cpssDxChBrgGenPortRateLimitSet(dev, port, &portCfg);
            if(port >= 256)
            {
                /* feature is limited to 256 ports */
                UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                break;
            }
            if(GT_FALSE == PRV_CPSS_DXCH_ERRATA_GET_MAC(dev,
                PRV_CPSS_DXCH_ALDRIN2_BRIDGE_RATE_LIMIT_UC_WA_E))
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, port);
            }

            /*
                1.1.2. Call cpssDxChBrgGenPortRateLimitGet.
                Expected: GT_OK and the same portGfgPtr.
            */
            st = cpssDxChBrgGenPortRateLimitGet(dev, port, &portCfgGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                             "cpssDxChBrgGenPortRateLimitGet: %d, %d", dev, port);

            /* Verifying struct fields */
            if(GT_FALSE == PRV_CPSS_DXCH_ERRATA_GET_MAC(dev,
                PRV_CPSS_DXCH_ALDRIN2_BRIDGE_RATE_LIMIT_UC_WA_E))
            {
                UTF_VERIFY_EQUAL2_STRING_MAC(portCfg.enableBc, portCfgGet.enableBc,
                    "get another portGfgPtr->qosProfileId than was set: %d, %d", dev, port);
                UTF_VERIFY_EQUAL2_STRING_MAC(portCfg.enableMc, portCfgGet.enableMc,
                    "get another portGfgPtr->enableMc than was set: %d, %d", dev, port);
                UTF_VERIFY_EQUAL2_STRING_MAC(portCfg.enableUcKnown, portCfgGet.enableUcKnown,
                    "get another portGfgPtr->enableUcKnown than was set: %d, %d", dev, port);
                UTF_VERIFY_EQUAL2_STRING_MAC(portCfg.enableUcUnk, portCfgGet.enableUcUnk,
                    "get another portGfgPtr->enableUcUnk than was set: %d, %d", dev, port);
                UTF_VERIFY_EQUAL2_STRING_MAC(portCfg.rateLimit, portCfgGet.rateLimit,
                    "get another portGfgPtr->rateLimit than was set: %d, %d", dev, port);

                if (PRV_CPSS_DXCH_XCAT2_FAMILY_CHECK_MAC(dev))
                {
                    UTF_VERIFY_EQUAL2_STRING_MAC(portCfg.enableMcReg, portCfgGet.enableMcReg,
                        "get another portGfgPtr->enableMcReg than was set: %d, %d", dev, port);
                }
            }

            /*
                1.1.3. Call with valid portGfgPtr [GT_FALSE, GT_FALSE,
                                                   GT_FALSE, GT_TRUE, 32767].
                Expected: GT_OK.
            */
            portCfg.enableBc      = GT_FALSE;
            portCfg.enableMc      = GT_FALSE;
            portCfg.enableUcKnown = GT_FALSE;
            portCfg.enableUcUnk   = GT_FALSE;
            portCfg.enableMcReg   = GT_TRUE;
            portCfg.rateLimit     = 65535;

            st = cpssDxChBrgGenPortRateLimitSet(dev, port, &portCfg);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);


            /*
                1.1.4. Call cpssDxChBrgGenPortRateLimitGet.
                Expected: GT_OK and the same portGfgPtr.
            */
            st = cpssDxChBrgGenPortRateLimitGet(dev, port, &portCfgGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                             "cpssDxChBrgGenPortRateLimitGet: %d, %d", dev, port);

            /* Verifying struct fields */
            UTF_VERIFY_EQUAL2_STRING_MAC(portCfg.enableBc, portCfgGet.enableBc,
                 "get another portGfgPtr->qosProfileId than was set: %d, %d", dev, port);
            UTF_VERIFY_EQUAL2_STRING_MAC(portCfg.enableMc, portCfgGet.enableMc,
                 "get another portGfgPtr->enableMc than was set: %d, %d", dev, port);
            if(GT_FALSE == PRV_CPSS_DXCH_ERRATA_GET_MAC(dev,
                PRV_CPSS_DXCH_ALDRIN2_BRIDGE_RATE_LIMIT_UC_WA_E))
            {
                UTF_VERIFY_EQUAL2_STRING_MAC(portCfg.enableUcKnown, portCfgGet.enableUcKnown,
                    "get another portGfgPtr->enableUcKnown than was set: %d, %d", dev, port);
                UTF_VERIFY_EQUAL2_STRING_MAC(portCfg.enableUcUnk, portCfgGet.enableUcUnk,
                    "get another portGfgPtr->enableUcUnk than was set: %d, %d", dev, port);
            }
            UTF_VERIFY_EQUAL2_STRING_MAC(portCfg.rateLimit, portCfgGet.rateLimit,
                 "get another portGfgPtr->rateLimit than was set: %d, %d", dev, port);

            if (PRV_CPSS_DXCH_XCAT2_FAMILY_CHECK_MAC(dev))
            {
                UTF_VERIFY_EQUAL2_STRING_MAC(portCfg.enableMcReg, portCfgGet.enableMcReg,
                     "get another portGfgPtr->enableMcReg than was set: %d, %d", dev, port);
            }

            /*
                1.1.5. Call with valid portGfgPtr [GT_FALSE, GT_TRUE,
                                                   GT_FALSE, GT_FALSE, 32767].
                Expected: GT_OK.
            */
            portCfg.enableBc      = GT_FALSE;
            portCfg.enableMc      = GT_TRUE;
            portCfg.enableUcKnown = GT_FALSE;
            portCfg.enableUcUnk   = GT_FALSE;
            portCfg.enableMcReg   = GT_TRUE;
            portCfg.rateLimit     = 32767;

            st = cpssDxChBrgGenPortRateLimitSet(dev, port, &portCfg);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*
                1.1.6. Call cpssDxChBrgGenPortRateLimitGet.
                Expected: GT_OK and the same portGfgPtr.
            */
            st = cpssDxChBrgGenPortRateLimitGet(dev, port, &portCfgGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                             "cpssDxChBrgGenPortRateLimitGet: %d, %d", dev, port);

            /* Verifying struct fields */
            UTF_VERIFY_EQUAL2_STRING_MAC(portCfg.enableBc, portCfgGet.enableBc,
                 "get another portGfgPtr->qosProfileId than was set: %d, %d", dev, port);
            UTF_VERIFY_EQUAL2_STRING_MAC(portCfg.enableMc, portCfgGet.enableMc,
                 "get another portGfgPtr->enableMc than was set: %d, %d", dev, port);
            if(GT_FALSE == PRV_CPSS_DXCH_ERRATA_GET_MAC(dev,
                PRV_CPSS_DXCH_ALDRIN2_BRIDGE_RATE_LIMIT_UC_WA_E))
            {
                UTF_VERIFY_EQUAL2_STRING_MAC(portCfg.enableUcKnown, portCfgGet.enableUcKnown,
                    "get another portGfgPtr->enableUcKnown than was set: %d, %d", dev, port);
                UTF_VERIFY_EQUAL2_STRING_MAC(portCfg.enableUcUnk, portCfgGet.enableUcUnk,
                    "get another portGfgPtr->enableUcUnk than was set: %d, %d", dev, port);
            }
            UTF_VERIFY_EQUAL2_STRING_MAC(portCfg.rateLimit, portCfgGet.rateLimit,
                 "get another portGfgPtr->rateLimit than was set: %d, %d", dev, port);

            if (PRV_CPSS_DXCH_XCAT2_FAMILY_CHECK_MAC(dev))
            {
                UTF_VERIFY_EQUAL2_STRING_MAC(portCfg.enableMcReg, portCfgGet.enableMcReg,
                     "get another portGfgPtr->enableMcReg than was set: %d, %d", dev, port);
            }

            /*
                1.1.7. Call with portGfgPtr [NULL]
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChBrgGenPortRateLimitGet(dev, port, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, NULL", dev, port);

            /*
                1.1.8. Call with portCfg [GT_TRUE, GT_TRUE, GT_TRUE, GT_TRUE, 65536]
                Expected: NOT GT_OK.
            */
            if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
            {
                portCfg.rateLimit = BIT_22;
            }
            else
            {
                portCfg.rateLimit = BIT_16;
            }

            st = cpssDxChBrgGenPortRateLimitSet(dev, port, &portCfg);
            UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st,
                 "%d, %d, portGfgPtr->rateLimit = %d", dev, port, portCfg.rateLimit);
        }

        portCfg.enableBc      = GT_TRUE;
        portCfg.enableMc      = GT_TRUE;
        portCfg.enableUcKnown = GT_TRUE;
        portCfg.enableUcUnk   = GT_TRUE;
        portCfg.enableMcReg   = GT_TRUE;
        portCfg.rateLimit     = 0;

        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChBrgGenPortRateLimitGet(dev, port, &portCfg);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChBrgGenPortRateLimitGet(dev, port, &portCfg);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgGenPortRateLimitGet(dev, port, &portCfg);
        if (UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }
    }

    portCfg.enableBc      = GT_TRUE;
    portCfg.enableMc      = GT_TRUE;
    portCfg.enableUcKnown = GT_TRUE;
    portCfg.enableUcUnk   = GT_TRUE;
    portCfg.enableMcReg   = GT_TRUE;
    portCfg.rateLimit     = 0;

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = BRG_GEN_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_LION_E | UTF_LION2_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgGenPortRateLimitGet(dev, port, &portCfg);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgGenPortRateLimitGet(dev, port, &portCfg);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgGenPortRateLimitGet
(
    IN GT_U8                                    dev,
    IN GT_U8                                    port,
    OUT CPSS_DXCH_BRG_GEN_RATE_LIMIT_PORT_STC * portGfgPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgGenPortRateLimitGet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (DxChx)
    1.1.1. Call with non-null portGfgPtr.
    Expected: GT_OK.
    1.1.2. Call with portGfgPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS                             st      = GT_OK;

    GT_U8                                 dev;
    GT_PHYSICAL_PORT_NUM                  port    = BRG_GEN_VALID_PHY_PORT_CNS;
    CPSS_DXCH_BRG_GEN_RATE_LIMIT_PORT_STC portCfg;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_LION_E | UTF_LION2_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call with non-null portGfgPtr.
                Expected: GT_OK.
            */
            st = cpssDxChBrgGenPortRateLimitGet(dev, port, &portCfg);
            if(port >= 256)
            {
                /* feature is limited to 256 ports */
                UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                break;
            }
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*
                1.1.2. Call with portCfgPtr [NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChBrgGenPortRateLimitGet(dev, port, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, NULL", dev, port);
        }

        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available
           physical ports.
        */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_FALSE))
        {
            st = cpssDxChBrgGenPortRateLimitGet(dev, port, &portCfg);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChBrgGenPortRateLimitGet(dev, port, &portCfg);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgGenPortRateLimitGet(dev, port, &portCfg);
        if (UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    port = BRG_GEN_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_LION_E | UTF_LION2_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgGenPortRateLimitGet(dev, port, &portCfg);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgGenPortRateLimitGet(dev, port, &portCfg);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgGenPortRateLimitTcpSynSet
(
    IN GT_U8      dev,
    IN GT_U8      port,
    IN GT_BOOL    enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgGenPortRateLimitTcpSynSet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (DxCh2 and above)
    1.1.1. Call with enable [GT_FALSE and GT_TRUE].
    Expected: GT_OK.
*/
    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM port  = BRG_GEN_VALID_PHY_PORT_CNS;
    GT_BOOL     state = GT_TRUE;
    GT_BOOL     stateGet;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_LION_E | UTF_LION2_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call with enable [GT_FALSE and GT_TRUE].
                Expected: GT_OK.
            */

            /* Call function with status [GT_FALSE] */
            state = GT_FALSE;

            st = cpssDxChBrgGenPortRateLimitTcpSynSet(dev, port, state);
            if(port >= 256)
            {
                /* feature is limited to 256 ports */
                UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                break;
            }
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, state);

            st = cpssDxChBrgGenPortRateLimitTcpSynGet(dev, port, &stateGet);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, stateGet);

            /* Verifying struct fields */
            UTF_VERIFY_EQUAL2_STRING_MAC(state, stateGet,
                 "get another state than was set: %d, %d", dev, port);

            /* Call function with status [GT_TRUE] */
            state = GT_TRUE;

            st = cpssDxChBrgGenPortRateLimitTcpSynSet(dev, port, state);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, state);

            st = cpssDxChBrgGenPortRateLimitTcpSynGet(dev, port, &stateGet);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, stateGet);

            /* Verifying struct fields */
            UTF_VERIFY_EQUAL2_STRING_MAC(state, stateGet,
                 "get another state than was set: %d, %d", dev, port);
        }

        state = GT_TRUE;

        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChBrgGenPortRateLimitTcpSynSet(dev, port, state);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */

        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChBrgGenPortRateLimitTcpSynSet(dev, port, state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgGenPortRateLimitTcpSynSet(dev, port, state);
        if (UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }
    }

    state = GT_TRUE;

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = BRG_GEN_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_LION_E | UTF_LION2_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgGenPortRateLimitTcpSynSet(dev, port, state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgGenPortRateLimitTcpSynSet(dev, port, state);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgGenPortRateLimitTcpSynGet
(
    IN GT_U8      dev,
    IN GT_U8      port,
    IN GT_BOOL    enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgGenPortRateLimitTcpSynGet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (DxChx)
    1.1.1. Call with non-null statePtr.
    Expected: GT_OK.
    1.1.2. Call with statePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st      = GT_OK;

    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM port    = BRG_GEN_VALID_PHY_PORT_CNS;
    GT_BOOL     state;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_LION_E | UTF_LION2_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call with non-null statePtr.
                Expected: GT_OK.
            */
            st = cpssDxChBrgGenPortRateLimitTcpSynGet(dev, port, &state);
            if(port >= 256)
            {
                /* feature is limited to 256 ports */
                UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                break;
            }
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, state);

            /*
                1.1.2. Call with statePtr [NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChBrgGenPortRateLimitTcpSynGet(dev, port, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, NULL", dev, port);
        }

        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available
           physical ports.
        */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_FALSE))
        {
            st = cpssDxChBrgGenPortRateLimitTcpSynGet(dev, port, &state);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChBrgGenPortRateLimitTcpSynGet(dev, port, &state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgGenPortRateLimitTcpSynGet(dev, port, &state);
        if (UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    port = BRG_GEN_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_LION_E | UTF_LION2_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgGenPortRateLimitTcpSynGet(dev, port, &state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgGenPortRateLimitTcpSynGet(dev, port, &state);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgGenUdpBcDestPortCfgSet
(
   IN GT_U8                     dev,
   IN GT_U32                    entryIndex,
   IN GT_U16                    udpPortNum,
   IN CPSS_NET_RX_CPU_CODE_ENT  cpuCode,
   IN CPSS_PACKET_CMD_ENT       cmd
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgGenUdpBcDestPortCfgSet)
{
/*
    ITERATE_DEVICES (DxCh2 and above)
    1.1. Call with  entryIndex[0],
                    udpPortNum[0],
                    cpuCode [CPSS_NET_UDP_BC_MIRROR_TRAP0_E /
                             CPSS_NET_UDP_BC_MIRROR_TRAP1_E /
                             CPSS_NET_UDP_BC_MIRROR_TRAP2_E /
                             CPSS_NET_UDP_BC_MIRROR_TRAP3_E],
                    cmd [CPSS_PACKET_CMD_MIRROR_TO_CPU_E /
                         CPSS_PACKET_CMD_TRAP_TO_CPU_E]
    Expected: GT_OK.
    1.2. Call with invalid entryIndex[12]
                   and other parameters from 1.1.
    Expected: NON GT_OK.
    1.3. Call function with cpuCode [CPSS_NET_IPV6_UNREGISTERED_MULTICAST_E /
                                     CPSS_NET_UNREG_MC_E] (not supported)
                            and other parameters from 1.1.
    Expected: NOT GT_OK.
    1.4. Call function with cmd [CPSS_PACKET_CMD_DROP_HARD_E /
                                 CPSS_PACKET_CMD_FORWARD_E] (not supported)
                            and other parameters from 1.1.
    Expected: NOT GT_OK.
    1.5. Call function with wrong enum values cpuCode
                            and other parameters from 1.1.
    Expected: GT_BAD_PARAM.
    1.6. Call function with wrong enum values cmd
                            and other parameters from 1.1.
    Expected: GT_BAD_PARAM.
    1.7. Call cpssDxChBrgGenUdpBcDestPortCfgInvalidate
         with entryindex [0] to invalidate all changes.
*/
    GT_STATUS                st      = GT_OK;

    GT_U8                    dev;
    GT_U32                   index1   = 0;
    GT_U16                   udpPort    = 0;
    GT_U16                   udpPortGet = 1;
    CPSS_NET_RX_CPU_CODE_ENT cpuCode    = CPSS_NET_CONTROL_E;
    CPSS_NET_RX_CPU_CODE_ENT cpuCodeGet = CPSS_NET_CONTROL_E;
    CPSS_PACKET_CMD_ENT      cmd     = CPSS_PACKET_CMD_FORWARD_E;
    CPSS_PACKET_CMD_ENT      cmdGet  = CPSS_PACKET_CMD_FORWARD_E;
    GT_BOOL                  validGet;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with entryIndex[0],
                           udpPortNum[0],
                           cpuCode [CPSS_NET_UDP_BC_MIRROR_TRAP0_E,
                                    CPSS_NET_UDP_BC_MIRROR_TRAP1_E,
                                    CPSS_NET_UDP_BC_MIRROR_TRAP2_E,
                                    CPSS_NET_UDP_BC_MIRROR_TRAP3_E],
                            cmd [CPSS_PACKET_CMD_MIRROR_TO_CPU_E,
                                 CPSS_PACKET_CMD_TRAP_TO_CPU_E]
            Expected: GT_OK.
        */

        /* Call function with entryIndex [0], udpPortNum [0] */
        /* cpuCode [CPSS_NET_UDP_BC_MIRROR_TRAP0_E]          */
        /* cmd [CPSS_PACKET_CMD_MIRROR_TO_CPU_E]             */
        index1   = 0;
        udpPort = 0;
        cpuCode = CPSS_NET_UDP_BC_MIRROR_TRAP0_E;
        cmd = CPSS_PACKET_CMD_MIRROR_TO_CPU_E;

        st = cpssDxChBrgGenUdpBcDestPortCfgSet(dev, index1, udpPort, cpuCode, cmd);
        UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, dev, index1, udpPort, cpuCode, cmd);

        if(GT_OK == st)
        {
            /*
                1.2. Call cpssDxChBrgGenUdpBcDestPortCfgGet with the same params.
                Expected: GT_OK and the same values.
            */
            st = cpssDxChBrgGenUdpBcDestPortCfgGet(dev, index1, &validGet,
                                                    &udpPortGet, &cpuCodeGet, &cmdGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            UTF_VERIFY_EQUAL1_STRING_MAC(udpPort, udpPortGet,
                            "get another udpPort than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(cpuCode, cpuCodeGet,
                            "get another cpuCode than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(cmd, cmdGet,
                            "get another cmd than was set: %d", dev);
        }

        /* Call function with entryIndex [0], udpPortNum [0] */
        /* cpuCode [CPSS_NET_UDP_BC_MIRROR_TRAP1_E]          */
        /* cmd [CPSS_PACKET_CMD_MIRROR_TO_CPU_E]             */
        cpuCode = CPSS_NET_UDP_BC_MIRROR_TRAP1_E;

        st = cpssDxChBrgGenUdpBcDestPortCfgSet(dev, index1, udpPort, cpuCode, cmd);
        UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, dev, index1, udpPort, cpuCode, cmd);

        if(GT_OK == st)
        {
            /*
                1.2. Call cpssDxChBrgGenUdpBcDestPortCfgGet with the same params.
                Expected: GT_OK and the same values.
            */
            st = cpssDxChBrgGenUdpBcDestPortCfgGet(dev, index1, &validGet,
                                                    &udpPortGet, &cpuCodeGet, &cmdGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            UTF_VERIFY_EQUAL1_STRING_MAC(udpPort, udpPortGet,
                            "get another udpPort than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(cpuCode, cpuCodeGet,
                            "get another cpuCode than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(cmd, cmdGet,
                            "get another cmd than was set: %d", dev);
        }
        /* Call function with entryIndex [0], udpPortNum [0] */
        /* cpuCode [CPSS_NET_UDP_BC_MIRROR_TRAP2_E]          */
        /* cmd [CPSS_PACKET_CMD_TRAP_TO_CPU_E]               */
        cpuCode = CPSS_NET_UDP_BC_MIRROR_TRAP2_E;
        cmd = CPSS_PACKET_CMD_TRAP_TO_CPU_E;

        st = cpssDxChBrgGenUdpBcDestPortCfgSet(dev, index1, udpPort, cpuCode, cmd);
        UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, dev, index1, udpPort, cpuCode, cmd);

        if(GT_OK == st)
        {
            /*
                1.2. Call cpssDxChBrgGenUdpBcDestPortCfgGet with the same params.
                Expected: GT_OK and the same values.
            */
            st = cpssDxChBrgGenUdpBcDestPortCfgGet(dev, index1, &validGet,
                                                    &udpPortGet, &cpuCodeGet, &cmdGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            UTF_VERIFY_EQUAL1_STRING_MAC(udpPort, udpPortGet,
                            "get another udpPort than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(cpuCode, cpuCodeGet,
                            "get another cpuCode than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(cmd, cmdGet,
                            "get another cmd than was set: %d", dev);
        }
        /* Call function with entryIndex [0], udpPortNum [0] */
        /* cpuCode [CPSS_NET_UDP_BC_MIRROR_TRAP3_E]          */
        /* cmd [CPSS_PACKET_CMD_TRAP_TO_CPU_E]               */
        cpuCode = CPSS_NET_UDP_BC_MIRROR_TRAP3_E;

        st = cpssDxChBrgGenUdpBcDestPortCfgSet(dev, index1, udpPort, cpuCode, cmd);
        UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, dev, index1, udpPort, cpuCode, cmd);

        if(GT_OK == st)
        {
        /*
                1.2. Call cpssDxChBrgGenUdpBcDestPortCfgGet with the same params.
                Expected: GT_OK and the same values.
            */
            st = cpssDxChBrgGenUdpBcDestPortCfgGet(dev, index1, &validGet,
                                                    &udpPortGet, &cpuCodeGet, &cmdGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            UTF_VERIFY_EQUAL1_STRING_MAC(udpPort, udpPortGet,
                            "get another udpPort than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(cpuCode, cpuCodeGet,
                            "get another cpuCode than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(cmd, cmdGet,
                            "get another cmd than was set: %d", dev);
        }

        /*
            1.3. Call with invalid entryIndex[12] and other parameters from 1.1.
            Expected: NON GT_OK.
        */
        index1 = 12;

        st = cpssDxChBrgGenUdpBcDestPortCfgSet(dev, index1, udpPort, cpuCode, cmd);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, index1);

        /*
            1.3. Call function with cpuCode [CPSS_NET_IPV6_UNREGISTERED_MULTICAST_E,
                                             CPSS_NET_UNREG_MC_E] (not supported)
                                    and other parameters from 1.1.
            Expected: NOT GT_OK.
        */

        /* Call function with entryIndex [0], udpPortNum [0] */
        /* cpuCode [CPSS_NET_IPV6_UNREGISTERED_MULTICAST_E]  */
        /* cmd [CPSS_PACKET_CMD_MIRROR_TO_CPU_E]             */
        index1   = 0;
        cpuCode = CPSS_NET_IPV6_UNREGISTERED_MULTICAST_E;
        cmd = CPSS_PACKET_CMD_MIRROR_TO_CPU_E;

        st = cpssDxChBrgGenUdpBcDestPortCfgSet(dev, index1, udpPort, cpuCode, cmd);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, cpuCode = %d", dev, cpuCode);

        /* Call function with entryIndex [0], udpPortNum [0] */
        /* cpuCode [CPSS_NET_UNREG_MC_E]                     */
        /* cmd [CPSS_PACKET_CMD_MIRROR_TO_CPU_E]             */
        cpuCode = CPSS_NET_UNREG_MC_E;

        st = cpssDxChBrgGenUdpBcDestPortCfgSet(dev, index1, udpPort, cpuCode, cmd);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, cpuCode = %d", dev, cpuCode);

        /*
            1.4. Call function with cmd [CPSS_PACKET_CMD_DROP_HARD_E,
                                         CPSS_PACKET_CMD_FORWARD_E] (not supported)
                            and other parameters from 1.1.
            Expected: NOT GT_OK.
        */

        /* Call function with entryIndex [0], udpPortNum [0] */
        /* cpuCode [CPSS_NET_UDP_BC_MIRROR_TRAP0_E]          */
        /* cmd [CPSS_PACKET_CMD_DROP_HARD_E]                 */
        cpuCode = CPSS_NET_UDP_BC_MIRROR_TRAP0_E;
        cmd = CPSS_PACKET_CMD_DROP_HARD_E;

        st = cpssDxChBrgGenUdpBcDestPortCfgSet(dev, index1, udpPort, cpuCode, cmd);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, cmd = %d", dev, cmd);

        /* Call function with entryIndex [0], udpPortNum [0] */
        /* cpuCode [CPSS_NET_UDP_BC_MIRROR_TRAP0_E]          */
        /* cmd [CPSS_PACKET_CMD_FORWARD_E]                   */
        cmd = CPSS_PACKET_CMD_FORWARD_E;

        st = cpssDxChBrgGenUdpBcDestPortCfgSet(dev, index1, udpPort, cpuCode, cmd);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, cmd = %d", dev, cmd);

        /*
            1.5. Call function with wrong enum values cpuCode
                                    and other parameters from 1.1.
            Expected: GT_BAD_PARAM.
        */
        cmd = CPSS_PACKET_CMD_MIRROR_TO_CPU_E;

        UTF_ENUMS_CHECK_MAC(cpssDxChBrgGenUdpBcDestPortCfgSet
                            (dev, index1, udpPort, cpuCode, cmd),
                            cpuCode);

        cpuCode = CPSS_NET_UDP_BC_MIRROR_TRAP0_E;

        /*
            1.6. Call function with wrong enum values cmd
                                    and other parameters from 1.1.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChBrgGenUdpBcDestPortCfgSet
                            (dev, index1, udpPort, cpuCode, cmd),
                            cmd);

        cmd = CPSS_PACKET_CMD_MIRROR_TO_CPU_E;

        /*
            1.7. Call cpssDxChBrgGenUdpBcDestPortCfgInvalidate with entryindex [0]
            to invalidate all changes.
        */
        index1 = 0;

        st = cpssDxChBrgGenUdpBcDestPortCfgInvalidate(dev, index1);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                    "cpssDxChBrgGenUdpBcDestPortCfgInvalidate: %d", dev);
    }

    index1   = 0;
    udpPort = 0;
    cpuCode = CPSS_NET_UDP_BC_MIRROR_TRAP0_E;
    cmd = CPSS_PACKET_CMD_MIRROR_TO_CPU_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgGenUdpBcDestPortCfgSet(dev, index1, udpPort, cpuCode, cmd);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgGenUdpBcDestPortCfgSet(dev, index1, udpPort, cpuCode, cmd);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgGenUdpBcDestPortCfgGet
(
   IN GT_U8                     devNum,
   IN GT_U32                    entryIndex,
   OUT GT_BOOL                  *validPtr,
   OUT GT_U16                   *udpPortNumPtr,
   OUT CPSS_NET_RX_CPU_CODE_ENT *cpuCodePtr,
   OUT CPSS_PACKET_CMD_ENT      *cmdPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgGenUdpBcDestPortCfgGet)
{
    /*
        ITERATE_DEVICES (DxCh2 and above)
        1.1. Call with  entryIndex[0 - 11], and not NULL pointers.
        Expected: GT_OK.
        1.2. Call with invalid entryIndex[12]
                       and other parameters from 1.1.
        Expected: NON GT_OK.
        1.3. Call with invalid validPtr [NULL].
        Expected: GT_BAD_PTR.
        1.4. Call with invalid udpPortNumPtr [NULL].
        Expected: GT_BAD_PTR.
        1.5. Call with invalid cpuCodePtr [NULL].
        Expected: GT_BAD_PTR.
        1.6. Call with invalid cmdPtr [NULL].
        Expected: GT_BAD_PTR.
    */
    GT_STATUS                st      = GT_OK;

    GT_U8                    dev;
    GT_U32                   index   = 0;
    GT_BOOL                  valid;
    GT_U16                   udpPort;
    CPSS_NET_RX_CPU_CODE_ENT cpuCode;
    CPSS_PACKET_CMD_ENT      cmd;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with  entryIndex[0 - 11], and not NULL pointers.
            Expected: GT_OK.
        */
        for(index = 0; index < 12; index++)
        {
            st = cpssDxChBrgGenUdpBcDestPortCfgGet(dev, index, &valid,
                                                    &udpPort, &cpuCode, &cmd);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        }

        /*
            1.2. Call with invalid entryIndex[12] and other parameters from 1.1.
            Expected: NON GT_OK.
        */
        index = 12;

        st = cpssDxChBrgGenUdpBcDestPortCfgGet(dev, index, &valid,
                                                &udpPort, &cpuCode, &cmd);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, index);

        index = 0;

        /*
            1.3. Call with invalid validPtr[NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChBrgGenUdpBcDestPortCfgGet(dev, index, NULL,
                                                &udpPort, &cpuCode, &cmd);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, index);

        /*
            1.3. Call with invalid udpPortPtr[NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChBrgGenUdpBcDestPortCfgGet(dev, index, &valid,
                                                NULL, &cpuCode, &cmd);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, index);
        /*
            1.3. Call with invalid cpuCodePtr[NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChBrgGenUdpBcDestPortCfgGet(dev, index, &valid,
                                                &udpPort, NULL, &cmd);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, index);
        /*
            1.3. Call with invalid cmdPtr[NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChBrgGenUdpBcDestPortCfgGet(dev, index, &valid,
                                                &udpPort, &cpuCode, NULL);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, index);
    }

    index   = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgGenUdpBcDestPortCfgGet(dev, index, &valid,
                                                &udpPort, &cpuCode, &cmd);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgGenUdpBcDestPortCfgGet(dev, index, &valid,
                                            &udpPort, &cpuCode, &cmd);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgGenUdpBcDestPortCfgInvalidate
(
   IN GT_U8                                     dev,
   IN GT_U32                                    entryIndex
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgGenUdpBcDestPortCfgInvalidate)
{
/*
    ITERATE_DEVICES (DxCh2 and above)
    1.1. Call with entryIndex [0].
    Expected: GT_OK.
    1.2. Call with entryIndex [12].
    Expected: NON GT_OK.
*/
    GT_STATUS  st    = GT_OK;

    GT_U8      dev;
    GT_U32     index = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with entryIndex [0].
            Expected: GT_OK.
        */
        index = 0;

        st = cpssDxChBrgGenUdpBcDestPortCfgInvalidate(dev, index);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, index);

        /*
            1.2. Call with entryIndex [12].
            Expected: NON GT_OK.
        */
        index = 12;

        st = cpssDxChBrgGenUdpBcDestPortCfgInvalidate(dev, index);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, index);
    }

    index = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    /* go over all non active or non-applicable devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgGenUdpBcDestPortCfgInvalidate(dev, index);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgGenUdpBcDestPortCfgInvalidate(dev, index);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgGenBpduTrapEnableSet
(
    IN GT_U8   dev,
    IN GT_BOOL enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgGenBpduTrapEnableSet)
{
/*
    ITERATE_DEVICES (DxCh2 and above)
    1.1. Call with enable [GT_FALSE and GT_TRUE].
    Expected: GT_OK.
    1.2. Call cpssDxChBrgGenBpduTrapEnableGet
    Expected: GT_OK and same enable.
*/
    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_BOOL     state = GT_TRUE;
    GT_BOOL     stateGet = GT_TRUE;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*  1.1  */
        state = GT_FALSE;

        st = cpssDxChBrgGenBpduTrapEnableSet(dev, state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, state);

        /*  1.2  */
        st = cpssDxChBrgGenBpduTrapEnableGet(dev, &stateGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        UTF_VERIFY_EQUAL1_STRING_MAC(state, stateGet,
            "get another state than was set: %d", dev);

        /*  1.1  */
        state = GT_TRUE;

        st = cpssDxChBrgGenBpduTrapEnableSet(dev, state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, state);

        /*  1.2  */
        st = cpssDxChBrgGenBpduTrapEnableGet(dev, &stateGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        UTF_VERIFY_EQUAL1_STRING_MAC(state, stateGet,
            "get another state than was set: %d", dev);
    }

    state = GT_TRUE;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgGenBpduTrapEnableSet(dev, state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgGenBpduTrapEnableSet(dev, state);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChBrgGenBpduTrapEnableGet)
{
/*
    ITERATE_DEVICES (DxCh2 and above)
    1.1. Call with not NULL enable
    Expected: GT_OK.
    1.2. Call with NULL enable
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_BOOL     state = GT_TRUE;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*  1.1  */
        st = cpssDxChBrgGenBpduTrapEnableGet(dev, &state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.2  */
        st = cpssDxChBrgGenBpduTrapEnableGet(dev, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgGenBpduTrapEnableGet(dev, &state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgGenBpduTrapEnableGet(dev, &state);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgGenArpTrapEnable
(
    IN GT_U8        dev,
    IN GT_U8        port,
    IN GT_BOOL      enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgGenArpTrapEnable)
{
/*
    ITERATE_DEVICES_PHY_PORTS (DxChx)
    1.1.1. Call with enable [GT_FALSE and GT_TRUE].
    Expected: GT_OK.
    1.1.2. Call get func with the same params.
    Expected: GT_OK and the same enable.
*/
    GT_STATUS   st    = GT_OK;

    GT_U8       dev;
    GT_PORT_NUM       port  = BRG_GEN_VALID_PHY_PORT_CNS;
    GT_BOOL     state = GT_TRUE;
    GT_BOOL     stateGet = GT_FALSE;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextVirtPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call with state [GT_FALSE and GT_TRUE].
                Expected: GT_OK.
            */

            /* Call function with state [GT_FALSE] */
            state = GT_FALSE;

            st = cpssDxChBrgGenArpTrapEnable(dev, port, state);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, state);

            /*
                1.1.2. Call get func with the same params.
                Expected: GT_OK.
            */

            st = cpssDxChBrgGenArpTrapEnableGet(dev, port, &stateGet);

            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, state);
            UTF_VERIFY_EQUAL1_STRING_MAC(state, stateGet,
                            "get another state than was set: %d", dev);


            /* Call function with state [GT_TRUE] */
            state = GT_TRUE;

            st = cpssDxChBrgGenArpTrapEnable(dev, port, state);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, state);

            /*
                1.1.2. Call get func with the same params.
                Expected: GT_OK.
            */

            st = cpssDxChBrgGenArpTrapEnableGet(dev, port, &stateGet);

            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, state);
            UTF_VERIFY_EQUAL1_STRING_MAC(state, stateGet,
                            "get another state than was set: %d", dev);
        }

        state = GT_TRUE;

        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextVirtPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChBrgGenArpTrapEnable(dev, port, state);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */

        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChBrgGenArpTrapEnable(dev, port, state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                     */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgGenArpTrapEnable(dev, port, state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    state = GT_TRUE;

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = BRG_GEN_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgGenArpTrapEnable(dev, port, state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgGenArpTrapEnable(dev, port, state);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgGenArpTrapEnableGet
(
    IN GT_U8        dev,
    IN GT_U8        port,
    OUT GT_BOOL    *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgGenArpTrapEnableGet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (DxChx)
    1.1.1. Call with not null enable.
    Expected: GT_OK.
    1.1.2. Call with wrong enable [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st    = GT_OK;

    GT_U8       dev;
    GT_PORT_NUM       port  = BRG_GEN_VALID_PHY_PORT_CNS;
    GT_BOOL     state = GT_TRUE;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextVirtPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call with not null enable.
                Expected: GT_OK.
            */
            st = cpssDxChBrgGenArpTrapEnableGet(dev, port, &state);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, state);

            /*
                1.1.2. Call with wrong enable [NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChBrgGenArpTrapEnableGet(dev, port, NULL);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PTR, st, dev, port, state);
        }

        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextVirtPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChBrgGenArpTrapEnableGet(dev, port, &state);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */

        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChBrgGenArpTrapEnableGet(dev, port, &state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                     */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgGenArpTrapEnableGet(dev, port, &state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    state = GT_TRUE;

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = BRG_GEN_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgGenArpTrapEnableGet(dev, port, &state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgGenArpTrapEnableGet(dev, port, &state);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgGenDropIpMcModeSet
(
    IN GT_U8                    dev,
    IN CPSS_DROP_MODE_TYPE_ENT  mode
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgGenDropIpMcModeSet)
{
/*
    ITERATE_DEVICE (DXCHX)
    1.1. Call cpssDxChBrgGenDropIpMcEnable with state [GT_TRUE]
              to enable Drop IP multicast.
    Expected: GT_OK.
    1.2. Call with mode [CPSS_DROP_MODE_SOFT_E / CPSS_DROP_MODE_HARD_E].
    Expected: GT_OK.
    1.3. Call cpssDxChBrgGenDropIpMcModeGet with not NULL modePtr.
    Expected: GT_OK and the same modePtr as was set.
    1.4. Call with mode.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;

    CPSS_DROP_MODE_TYPE_ENT  mode    = CPSS_DROP_MODE_SOFT_E;
    CPSS_DROP_MODE_TYPE_ENT  modeGet = CPSS_DROP_MODE_SOFT_E;
    GT_BOOL                  state   = GT_FALSE;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call cpssDxChBrgGenDropIpMcEnable with state [GT_TRUE]
                      to enable Drop IP multicast.
            Expected: GT_OK.
        */
        state = GT_TRUE;

        st = cpssDxChBrgGenDropIpMcEnable(dev, state);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                   "cpssDxChBrgGenDropIpMcEnable: %d, %d", dev, state);

        /*
            1.2. Call with mode [CPSS_DROP_MODE_SOFT_E / CPSS_DROP_MODE_HARD_E]
            Expected: GT_OK.
        */
        /* iterate with mode = CPSS_DROP_MODE_SOFT_E */
        mode = CPSS_DROP_MODE_SOFT_E;

        st = cpssDxChBrgGenDropIpMcModeSet(dev, mode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, mode);

        /*
            1.3. Call cpssDxChBrgGenDropIpMcModeGet with not NULL modePtr
            Expected: GT_OK and the same modePtr as was set.
        */
        st = cpssDxChBrgGenDropIpMcModeGet(dev, &modeGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChBrgGenDropIpMcModeGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(mode, modeGet, "get another mode than was set: %d", dev);

        /* iterate with mode = CPSS_DROP_MODE_HARD_E */
        mode = CPSS_DROP_MODE_HARD_E;

        st = cpssDxChBrgGenDropIpMcModeSet(dev, mode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, mode);

        /*
            1.3. Call cpssDxChBrgGenDropIpMcModeGet with not NULL modePtr
            Expected: GT_OK and the same modePtr as was set.
        */
        st = cpssDxChBrgGenDropIpMcModeGet(dev, &modeGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChBrgGenDropIpMcModeGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(mode, modeGet, "get another mode than was set: %d", dev);

        /*
            1.4. Call with wrong enum values mode.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChBrgGenDropIpMcModeSet
                            (dev, mode),
                            mode);
    }

    mode = CPSS_DROP_MODE_SOFT_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgGenDropIpMcModeSet(dev, mode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgGenDropIpMcModeSet(dev, mode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgGenDropIpMcModeGet
(
    IN  GT_U8                    dev,
    OUT CPSS_DROP_MODE_TYPE_ENT  *modePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgGenDropIpMcModeGet)
{
/*
    ITERATE_DEVICE (DXCHX)
    1.1. Call cpssDxChBrgGenDropIpMcEnable with state [GT_TRUE]
              to enable Drop IP multicast.
    Expected: GT_OK.
    1.2. Call with not NULL modePtr.
    Expected: GT_OK
    1.3. Call with modePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    CPSS_DROP_MODE_TYPE_ENT  mode  = CPSS_DROP_MODE_SOFT_E;
    GT_BOOL                  state = GT_FALSE;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call cpssDxChBrgGenDropIpMcEnable with state [GT_TRUE]
                      to enable Drop IP multicast.
            Expected: GT_OK.
        */
        state = GT_TRUE;

        st = cpssDxChBrgGenDropIpMcEnable(dev, state);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                   "cpssDxChBrgGenDropIpMcEnable: %d, %d", dev, state);

        /*
            1.2. Call with not NULL modePtr.
            Expected: GT_OK
        */
        st = cpssDxChBrgGenDropIpMcModeGet(dev, &mode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.3. Call with modePtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChBrgGenDropIpMcModeGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, modePtr = NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgGenDropIpMcModeGet(dev, &mode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgGenDropIpMcModeGet(dev, &mode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgGenDropNonIpMcModeSet
(
    IN GT_U8                    dev,
    IN CPSS_DROP_MODE_TYPE_ENT  mode
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgGenDropNonIpMcModeSet)
{
/*
    ITERATE_DEVICE (DXCHX)
    1.1. Call cpssDxChBrgGenDropNonIpMcEnable with state [GT_TRUE]
              to enable Drop IP multicast.
    Expected: GT_OK.
    1.2. Call with mode [CPSS_DROP_MODE_SOFT_E / CPSS_DROP_MODE_HARD_E].
    Expected: GT_OK.
    1.3. Call cpssDxChBrgGenDropNonIpMcModeGet with not NULL modePtr.
    Expected: GT_OK and the same modePtr as was set.
    1.4. Call with mode.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    CPSS_DROP_MODE_TYPE_ENT  mode    = CPSS_DROP_MODE_SOFT_E;
    CPSS_DROP_MODE_TYPE_ENT  modeGet = CPSS_DROP_MODE_SOFT_E;
    GT_BOOL                  state   = GT_FALSE;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call cpssDxChBrgGenDropNonIpMcEnable with state [GT_TRUE]
                      to enable Drop IP multicast.
            Expected: GT_OK.
        */
        state = GT_TRUE;

        st = cpssDxChBrgGenDropNonIpMcEnable(dev, state);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                   "cpssDxChBrgGenDropNonIpMcEnable: %d, %d", dev, state);

        /*
            1.2. Call with mode [CPSS_DROP_MODE_SOFT_E / CPSS_DROP_MODE_HARD_E]
            Expected: GT_OK.
        */
        /* iterate with mode = CPSS_DROP_MODE_SOFT_E */
        mode = CPSS_DROP_MODE_SOFT_E;

        st = cpssDxChBrgGenDropNonIpMcModeSet(dev, mode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, mode);

        /*
            1.3. Call cpssDxChBrgGenDropNonIpMcModeGet with not NULL modePtr
            Expected: GT_OK and the same modePtr as was set.
        */
        st = cpssDxChBrgGenDropNonIpMcModeGet(dev, &modeGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChBrgGenDropNonIpMcModeGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(mode, modeGet, "get another mode than was set: %d", dev);

        /* iterate with mode = CPSS_DROP_MODE_HARD_E */
        mode = CPSS_DROP_MODE_HARD_E;

        st = cpssDxChBrgGenDropNonIpMcModeSet(dev, mode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, mode);

        /*
            1.3. Call cpssDxChBrgGenDropNonIpMcModeGet with not NULL modePtr
            Expected: GT_OK and the same modePtr as was set.
        */
        st = cpssDxChBrgGenDropNonIpMcModeGet(dev, &modeGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChBrgGenDropNonIpMcModeGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(mode, modeGet, "get another mode than was set: %d", dev);

        /*
            1.4. Call with wrong enum values mode.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChBrgGenDropNonIpMcModeSet
                            (dev, mode),
                            mode);
    }

    mode = CPSS_DROP_MODE_SOFT_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgGenDropNonIpMcModeSet(dev, mode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgGenDropNonIpMcModeSet(dev, mode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgGenDropNonIpMcModeGet
(
    IN  GT_U8                    dev,
    OUT CPSS_DROP_MODE_TYPE_ENT  *modePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgGenDropNonIpMcModeGet)
{
/*
    ITERATE_DEVICE (DXCHX)
    1.1. Call cpssDxChBrgGenDropNonIpMcEnable with state [GT_TRUE]
              to enable Drop IP multicast.
    Expected: GT_OK.
    1.2. Call with not NULL modePtr.
    Expected: GT_OK
    1.3. Call with modePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    CPSS_DROP_MODE_TYPE_ENT  mode  = CPSS_DROP_MODE_SOFT_E;
    GT_BOOL                  state = GT_FALSE;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call cpssDxChBrgGenDropNonIpMcEnable with state [GT_TRUE]
                      to enable Drop IP multicast.
            Expected: GT_OK.
        */
        state = GT_TRUE;

        st = cpssDxChBrgGenDropNonIpMcEnable(dev, state);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                   "cpssDxChBrgGenDropNonIpMcEnable: %d, %d", dev, state);

        /*
            1.2. Call with not NULL modePtr.
            Expected: GT_OK
        */
        st = cpssDxChBrgGenDropNonIpMcModeGet(dev, &mode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.3. Call with modePtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChBrgGenDropNonIpMcModeGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, modePtr = NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgGenDropNonIpMcModeGet(dev, &mode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgGenDropNonIpMcModeGet(dev, &mode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgGenCfiRelayEnableGet
(
    IN  GT_U8   dev,
    OUT GT_BOOL *enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgGenCfiRelayEnableGet)
{
/*
    ITERATE_DEVICES (dxCh3 and above)
    1.1. Call with correct parameters.
    Expected: GT_OK.
    1.2. Call with incorrect enable pointer (NULL).
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st     = GT_OK;
    GT_U8       dev;
    GT_BOOL     state;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with correct parameters.
            Expected: GT_OK.
        */
        st = cpssDxChBrgGenCfiRelayEnableGet(dev, &state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, state);

        /*
            1.2. Call with incorrect enable pointer (NULL).
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChBrgGenCfiRelayEnableGet(dev, NULL);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, state);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgGenCfiRelayEnableGet(dev, &state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgGenCfiRelayEnableGet(dev, &state);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgGenCfiRelayEnableSet
(
    IN  GT_U8   dev,
    IN  GT_BOOL enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgGenCfiRelayEnableSet)
{
/*
    ITERATE_DEVICES (dxCh3 and above)
    1.1. Call with correct parameters.
    Expected: GT_OK.
    1.2. Call cpssDxChBrgGenCfiRelayEnableGet.
    Expected: GT_OK and the same enable than was set.
*/
    GT_STATUS   st       = GT_OK;
    GT_U8       dev;
    GT_BOOL     state    = GT_TRUE;
    GT_BOOL     stateGet = GT_FALSE;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with correct parameters.
            Expected: GT_OK.
        */
        st = cpssDxChBrgGenCfiRelayEnableSet(dev, state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, state);

        /*
            1.2. Call cpssDxChBrgGenCfiRelayEnableGet.
            Expected: GT_OK and the same enable than was set.
        */

        st = cpssDxChBrgGenCfiRelayEnableGet(dev, &stateGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                               "cpssDxChBrgGenCfiRelayEnableGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(state, stateGet,
                               "get another state than was set: %d", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgGenCfiRelayEnableSet(dev, state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgGenCfiRelayEnableSet(dev, state);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgGenPortIeeeReservedMcastProfileIndexSet
(
    IN GT_U8      dev,
    IN GT_U8      port,
    IN GT_U32     profileIndex
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgGenPortIeeeReservedMcastProfileIndexSet)
{
/*
    ITERATE_DEVICES_PHY_CPU_PORTS (xCat and above)
    1.1.1. Call with profileIndex[0 / 3].
    Expected: GT_OK.
    1.1.2. Call cpssDxChBrgGenPortIeeeReservedMcastProfileIndexGet.
    Expected: GT_OK and the same value.
    1.1.3. Call with wrong profileIndex [5].
    Expected: NOT GT_OK.
*/
    GT_STATUS   st = GT_OK;

    GT_U8       dev;
    GT_PORT_NUM       port  = BRG_GEN_VALID_PHY_PORT_CNS;
    GT_U32      profileIndex = 0;
    GT_U32      profileIndexGet;
    GT_U32      profileNum;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextVirtPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call with profileIndex[0 / 3].
                Expected: GT_OK.
            */

            /* Call function with profileIndex [0] */
            profileIndex = 0;

            st = cpssDxChBrgGenPortIeeeReservedMcastProfileIndexSet(dev, port, profileIndex);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, profileIndex);

            /*
                1.1.2. Call cpssDxChBrgGenPortIeeeReservedMcastProfileIndexGet.
                Expected: GT_OK and the same value.
            */

            st = cpssDxChBrgGenPortIeeeReservedMcastProfileIndexGet(dev, port, &profileIndexGet);

            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
             "cpssDxChBrgGenPortIeeeReservedMcastProfileIndexGet: %d, %d", dev, profileIndexGet);
            /* Verifying value */
            UTF_VERIFY_EQUAL1_STRING_MAC(profileIndex, profileIndexGet,
                                         "get another profileIndex than was set: %d", dev);


            /* Call function with profileIndex [3] */
            profileIndex = 3;

            st = cpssDxChBrgGenPortIeeeReservedMcastProfileIndexSet(dev, port, profileIndex);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, profileIndex);

            /*
                1.1.2. Call cpssDxChBrgGenPortIeeeReservedMcastProfileIndexGet.
                Expected: GT_OK and the same value.
            */

            st = cpssDxChBrgGenPortIeeeReservedMcastProfileIndexGet(dev, port, &profileIndexGet);

            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
              "cpssDxChBrgGenPortIeeeReservedMcastProfileIndexGet: %d, %d", dev, profileIndexGet);
            /* Verifying value */
            UTF_VERIFY_EQUAL1_STRING_MAC(profileIndex, profileIndexGet,
                                         "get another profileIndex than was set: %d", dev);

            /*
                1.1.3. Call with wrong profileIndex [5].
                Expected: NOT GT_OK.
            */

            /* Call function with profileIndex [5] */
            if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
            {
                profileNum = 8;
            }
            else
            {
                profileNum = 4;
            }
            profileIndex = profileNum;

            st = cpssDxChBrgGenPortIeeeReservedMcastProfileIndexSet(dev, port, profileIndex);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OUT_OF_RANGE, st, dev, port, profileIndex);

            profileIndex = profileNum - 1;
        }

        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextVirtPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChBrgGenPortIeeeReservedMcastProfileIndexSet(dev, port, profileIndex);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */

        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChBrgGenPortIeeeReservedMcastProfileIndexSet(dev, port, profileIndex);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                     */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgGenPortIeeeReservedMcastProfileIndexSet(dev, port, profileIndex);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = BRG_GEN_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgGenPortIeeeReservedMcastProfileIndexSet(dev, port, profileIndex);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgGenPortIeeeReservedMcastProfileIndexSet(dev, port, profileIndex);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/* GT_STATUS cpssDxChBrgGenPortIeeeReservedMcastProfileIndexGet
(
    IN  GT_U8        devNum,
    IN  GT_U8        port,
    OUT GT_U32       *profileIndexPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgGenPortIeeeReservedMcastProfileIndexGet)
{
/*
    ITERATE_DEVICES_PHY_CPU_PORTS (xCat and above)
    1.1.1. Call with valid profileIndexPtr.
    Expected: GT_OK.
    1.1.2. Call wrong profileIndexPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;

    GT_U8       dev;
    GT_PORT_NUM       port = BRG_GEN_VALID_PHY_PORT_CNS;
    GT_U32      profileIndex;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextVirtPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call with valid profileIndexPtr.
                Expected: GT_OK.
            */
            st = cpssDxChBrgGenPortIeeeReservedMcastProfileIndexGet(dev, port, &profileIndex);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, profileIndex);

            /*
                1.1.2. Call wrong profileIndexPtr [NULL].
                Expected: GT_OK.
            */
            st = cpssDxChBrgGenPortIeeeReservedMcastProfileIndexGet(dev, port, NULL);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PTR, st, dev, port, profileIndex);
        }

        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextVirtPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChBrgGenPortIeeeReservedMcastProfileIndexGet(dev, port, &profileIndex);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */

        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChBrgGenPortIeeeReservedMcastProfileIndexGet(dev, port, &profileIndex);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                     */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgGenPortIeeeReservedMcastProfileIndexGet(dev, port, &profileIndex);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = BRG_GEN_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgGenPortIeeeReservedMcastProfileIndexGet(dev, port, &profileIndex);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgGenPortIeeeReservedMcastProfileIndexGet(dev, port, &profileIndex);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgGenPortOversizeUntaggedPacketsFilterEnableSet
(
    IN GT_U8      dev,
    IN GT_U8      port,
    IN GT_BOOL    enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgGenPortOversizeUntaggedPacketsFilterEnableSet)
{
/*
    ITERATE_DEVICES_PHY_CPU_PORTS (DxCh3 and above)
    1.1.1. Call with state [GT_FALSE and GT_TRUE].
    Expected: GT_OK.
    1.1.2. Call cpssDxChBrgGenPortOversizeUntaggedPacketsFilterEnableGet.
    Expected: GT_OK and the same value.
*/
    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_PORT_NUM       port  = BRG_GEN_VALID_PHY_PORT_CNS;
    GT_BOOL     state = GT_TRUE;
    GT_BOOL     stateGet;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextDefaultEPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextDefaultEPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call with state [GT_FALSE and GT_TRUE].
                Expected: GT_OK.
            */
            /* Call function with state [GT_FALSE] */
            state = GT_FALSE;

            st = cpssDxChBrgGenPortOversizeUntaggedPacketsFilterEnableSet(dev, port, state);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, state);

            /*
                1.1.2. Call cpssDxChBrgGenPortOversizeUntaggedPacketsFilterEnableGet.
                Expected: GT_OK and the same value.
            */
            st = cpssDxChBrgGenPortOversizeUntaggedPacketsFilterEnableGet(dev, port, &stateGet);

            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
             "cpssDxChBrgGenPortOversizeUntaggedPacketsFilterEnableGet: %d, %d", dev, stateGet);
            /* Verifying value */
            UTF_VERIFY_EQUAL1_STRING_MAC(state, stateGet,
                                         "get another state than was set: %d", dev);

            /* Call function with state [GT_TRUE] */
            state = GT_TRUE;

            st = cpssDxChBrgGenPortOversizeUntaggedPacketsFilterEnableSet(dev, port, state);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, state);

            /*
                1.1.2. Call cpssDxChBrgGenPortOversizeUntaggedPacketsFilterEnableGet.
                Expected: GT_OK and the same value.
            */
            st = cpssDxChBrgGenPortOversizeUntaggedPacketsFilterEnableGet(dev, port, &stateGet);

            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
             "cpssDxChBrgGenPortOversizeUntaggedPacketsFilterEnableGet: %d, %d", dev, stateGet);

            /* Verifying value */
            UTF_VERIFY_EQUAL1_STRING_MAC(state, stateGet,
                                         "get another state than was set: %d", dev);
        }
        state = GT_TRUE;

        st = prvUtfNextDefaultEPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextDefaultEPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChBrgGenPortOversizeUntaggedPacketsFilterEnableSet(dev, port, state);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */

        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChBrgGenPortOversizeUntaggedPacketsFilterEnableSet(dev, port, state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                     */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgGenPortOversizeUntaggedPacketsFilterEnableSet(dev, port, state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    state = GT_TRUE;

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = BRG_GEN_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgGenPortOversizeUntaggedPacketsFilterEnableSet(dev, port, state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgGenPortOversizeUntaggedPacketsFilterEnableSet(dev, port, state);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgGenPortOversizeUntaggedPacketsFilterEnableGet
(
    IN  GT_U8        devNum,
    IN  GT_U8        port,
    OUT GT_BOOL      *statusPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgGenPortOversizeUntaggedPacketsFilterEnableGet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (DxCh3 and above)
    1.1.1. Call with valid statusPtr.
    Expected: GT_OK.
    1.1.2. Call wrong statusPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st   = GT_OK;

    GT_U8       dev;
    GT_PORT_NUM       port = BRG_GEN_VALID_PHY_PORT_CNS;
    GT_BOOL     state;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextDefaultEPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextDefaultEPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call with valid statusPtr.
                Expected: GT_OK.
            */
            st = cpssDxChBrgGenPortOversizeUntaggedPacketsFilterEnableGet(dev, port, &state);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, state);

            /*
                1.1.2. Call wrong statusPtr [NULL].
                Expected: GT_OK.
            */
            st = cpssDxChBrgGenPortOversizeUntaggedPacketsFilterEnableGet(dev, port, NULL);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PTR, st, dev, port, state);
        }

        st = prvUtfNextDefaultEPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextDefaultEPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChBrgGenPortOversizeUntaggedPacketsFilterEnableGet(dev, port, &state);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */

        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChBrgGenPortOversizeUntaggedPacketsFilterEnableGet(dev, port, &state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                     */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgGenPortOversizeUntaggedPacketsFilterEnableGet(dev, port, &state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    state = GT_TRUE;

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = BRG_GEN_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgGenPortOversizeUntaggedPacketsFilterEnableGet(dev, port, &state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgGenPortOversizeUntaggedPacketsFilterEnableGet(dev, port, &state);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgGenPortRateLimitSpeedGranularitySet
(
    IN  GT_U8                devNum,
    IN  GT_U8                portNum,
    IN  CPSS_PORT_SPEED_ENT  speed
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgGenPortRateLimitSpeedGranularitySet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (xCat and above exclude Lion)
    1.1.1. Call with speed [CPSS_PORT_SPEED_10_E,
                            CPSS_PORT_SPEED_100_E,
                            CPSS_PORT_SPEED_1000_E,].
    Expected: GT_OK for non flexLink ports.
    1.1.2. Call cpssDxChBrgGenPortRateLimitSpeedGranularityGet with the same params.
    Expected: GT_OK and the same values.
    1.1.3. Call with wrong enum values speed.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS              st       = GT_OK;

    GT_U8                  dev;
    GT_PHYSICAL_PORT_NUM                  port     = BRG_GEN_VALID_PHY_PORT_CNS;
    CPSS_PORT_SPEED_ENT    speed    = CPSS_PORT_SPEED_10_E;
    CPSS_PORT_SPEED_ENT    speedGet = CPSS_PORT_SPEED_10_E;
    GT_BOOL                isFlexLink;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_LION_E | UTF_LION2_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            if(!UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
            {
                st = prvUtfPortIsFlexLinkGet(dev,port,&isFlexLink);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            }
            else
            {
                isFlexLink = GT_FALSE;
            }

            /*
                1.1.1. Call with speed [CPSS_PORT_SPEED_10_E,
                                        CPSS_PORT_SPEED_100_E,
                                        CPSS_PORT_SPEED_1000_E,].
                Expected: GT_OK for non flexLink ports.
            */

            /* speed = CPSS_PORT_SPEED_10_E */
            speed = CPSS_PORT_SPEED_10_E;

            st = cpssDxChBrgGenPortRateLimitSpeedGranularitySet(dev, port, speed);
            if(port >= 256)
            {
                /* feature is limited to 256 ports */
                UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                break;
            }

            if((!isFlexLink) ||
               (PRV_CPSS_PP_MAC(dev)->devFamily >= CPSS_PP_FAMILY_DXCH_XCAT2_E))
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, speed);

                if (GT_OK == st)
                {
                    /*
                        1.1.2. Call cpssDxChBrgGenPortRateLimitSpeedGranularityGet
                               with the same params.
                        Expected: GT_OK and the same values.
                    */

                    st = cpssDxChBrgGenPortRateLimitSpeedGranularityGet(dev, port, &speedGet);

                    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                     "cpssDxChBrgGenPortRateLimitSpeedGranularityGet: %d, %d", dev, port);

                    /* Verifying value */
                    UTF_VERIFY_EQUAL1_STRING_MAC(speed, speedGet,
                                               "get another speed than was set: %d", dev);
                }
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
            }

            /* speed = CPSS_PORT_SPEED_100_E */
            speed = CPSS_PORT_SPEED_100_E;

            st = cpssDxChBrgGenPortRateLimitSpeedGranularitySet(dev, port, speed);

            if((!isFlexLink) ||
               (PRV_CPSS_PP_MAC(dev)->devFamily >= CPSS_PP_FAMILY_DXCH_XCAT2_E))
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, speed);

                if (GT_OK == st)
                {
                    /*
                        1.1.2. Call cpssDxChBrgGenPortRateLimitSpeedGranularityGet
                               with the same params.
                        Expected: GT_OK and the same values.
                    */

                    st = cpssDxChBrgGenPortRateLimitSpeedGranularityGet(dev, port, &speedGet);

                    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                     "cpssDxChBrgGenPortRateLimitSpeedGranularityGet: %d, %d", dev, port);

                    /* Verifying value */
                    UTF_VERIFY_EQUAL1_STRING_MAC(speed, speedGet,
                                               "get another speed than was set: %d", dev);
                }
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
            }

            /* speed = CPSS_PORT_SPEED_1000_E */
            speed = CPSS_PORT_SPEED_1000_E;

            st = cpssDxChBrgGenPortRateLimitSpeedGranularitySet(dev, port, speed);

            if((!isFlexLink) ||
               (PRV_CPSS_PP_MAC(dev)->devFamily >= CPSS_PP_FAMILY_DXCH_XCAT2_E))
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, speed);

                if (GT_OK == st)
                {
                    /*
                        1.1.2. Call cpssDxChBrgGenPortRateLimitSpeedGranularityGet
                               with the same params.
                        Expected: GT_OK and the same values.
                    */

                    st = cpssDxChBrgGenPortRateLimitSpeedGranularityGet(dev, port, &speedGet);

                    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                     "cpssDxChBrgGenPortRateLimitSpeedGranularityGet: %d, %d", dev, port);

                    /* Verifying value */
                    UTF_VERIFY_EQUAL1_STRING_MAC(speed, speedGet,
                                               "get another speed than was set: %d", dev);
                }
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
            }

            /*
               1.1.3. Call with wrong enum values speed.
               Expected: GT_BAD_PARAM.
            */
            UTF_ENUMS_CHECK_MAC(cpssDxChBrgGenPortRateLimitSpeedGranularitySet
                                (dev, port, speed),
                                speed);
        }

        /*
           1.2. For all active devices go over all non available
           physical ports.
        */
        speed = CPSS_PORT_SPEED_1000_E;

        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            /* speed = CPSS_PORT_SPEED_1000_E                */
            st = cpssDxChBrgGenPortRateLimitSpeedGranularitySet(dev, port, speed);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        /* speed == CPSS_PORT_SPEED_1000_E */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChBrgGenPortRateLimitSpeedGranularitySet(dev, port, speed);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgGenPortRateLimitSpeedGranularitySet(dev, port, speed);
        if (UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
        {
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, speed);
        }
        else
        {
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, port, speed);
        }
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    speed = CPSS_PORT_SPEED_1000_E;
    port = BRG_GEN_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_LION_E | UTF_LION2_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgGenPortRateLimitSpeedGranularitySet(dev, port, speed);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    /* port == 0, speed == CPSS_PORT_SPEED_1000_E */

    st = cpssDxChBrgGenPortRateLimitSpeedGranularitySet(dev, port, speed);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgGenPortRateLimitSpeedGranularityGet
(
    IN  GT_U8                 devNum,
    IN  GT_U8                 portNum,
    OUT CPSS_PORT_SPEED_ENT   *speedPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgGenPortRateLimitSpeedGranularityGet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (xCat and above exclude Lion)
    1.1.1. Call with non-null speedPtr.
    Expected: GT_OK.
    1.1.2. Call with speedPtr [NULL].
    Expected: GT_BAD_PTR.
*/

    GT_STATUS           st    = GT_OK;
    GT_U8               dev;
    GT_PHYSICAL_PORT_NUM               port  = BRG_GEN_VALID_PHY_PORT_CNS;
    CPSS_PORT_SPEED_ENT speed;
    GT_BOOL                isFlexLink;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_LION_E | UTF_LION2_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            if(!UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
            {
                st = prvUtfPortIsFlexLinkGet(dev,port,&isFlexLink);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            }
            else
            {
                isFlexLink = GT_FALSE;
            }

            /*
               1.1.1. Call with non-null speedPtr.
               Expected: GT_OK.
            */
            st = cpssDxChBrgGenPortRateLimitSpeedGranularityGet(dev, port, &speed);
            if(port >= 256)
            {
                /* feature is limited to 256 ports */
                UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                break;
            }

            if((!isFlexLink) ||
               (PRV_CPSS_PP_MAC(dev)->devFamily >= CPSS_PP_FAMILY_DXCH_XCAT2_E))
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
            }

            /*
               1.1.2. Call with speedPtr [NULL].
               Expected: GT_BAD_PTR.
            */
            st = cpssDxChBrgGenPortRateLimitSpeedGranularityGet(dev, port, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, NULL", dev, port);
        }

        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /*
           1.2. For all active devices go over all non available
           physical ports.
        */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_FALSE))
        {
            st = cpssDxChBrgGenPortRateLimitSpeedGranularityGet(dev, port, &speed);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChBrgGenPortRateLimitSpeedGranularityGet(dev, port, &speed);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgGenPortRateLimitSpeedGranularityGet(dev, port, &speed);
        if (UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    port = BRG_GEN_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_LION_E | UTF_LION2_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgGenPortRateLimitSpeedGranularityGet(dev, port, &speed);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgGenPortRateLimitSpeedGranularityGet(dev, port, &speed);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgGenRateLimitSpeedCfgEnableSet
(
    IN  GT_U8   dev,
    IN  GT_BOOL enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgGenRateLimitSpeedCfgEnableSet)
{
/*
    ITERATE_DEVICES (xCat and above exclude Lion)
    1.1. Call with correct parameters.
    Expected: GT_OK.
    1.2. Call cpssDxChBrgGenRateLimitSpeedCfgEnableGet.
    Expected: GT_OK and the same enable than was set.
*/
    GT_STATUS   st       = GT_OK;

    GT_U8       dev;
    GT_BOOL     state    = GT_TRUE;
    GT_BOOL     stateGet = GT_FALSE;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_LION_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with correct parameters.
            Expected: GT_OK.
        */
        st = cpssDxChBrgGenRateLimitSpeedCfgEnableSet(dev, state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, state);

        /*
            1.2. Call cpssDxChBrgGenRateLimitSpeedCfgEnableGet.
            Expected: GT_OK and the same enable than was set.
        */

        st = cpssDxChBrgGenRateLimitSpeedCfgEnableGet(dev, &stateGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                 "cpssDxChBrgGenRateLimitSpeedCfgEnableGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(state, stateGet,
                 "get another state than was set: %d", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_LION_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgGenRateLimitSpeedCfgEnableSet(dev, state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgGenRateLimitSpeedCfgEnableSet(dev, state);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgGenRateLimitSpeedCfgEnableGet
(
    IN  GT_U8   dev,
    OUT GT_BOOL *enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgGenRateLimitSpeedCfgEnableGet)
{
/*
    ITERATE_DEVICES (xCat and above exclude Lion)
    1.1. Call with correct parameters.
    Expected: GT_OK.
    1.2. Call with incorrect enable pointer (NULL).
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st     = GT_OK;
    GT_U8       dev;
    GT_BOOL     state;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_LION_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with correct parameters.
            Expected: GT_OK.
        */
        st = cpssDxChBrgGenRateLimitSpeedCfgEnableGet(dev, &state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, state);

        /*
            1.2. Call with incorrect enable pointer (NULL).
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChBrgGenRateLimitSpeedCfgEnableGet(dev, NULL);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, state);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_LION_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgGenRateLimitSpeedCfgEnableGet(dev, &state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgGenRateLimitSpeedCfgEnableGet(dev, &state);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgGenExcludeLinkLocalMcFromUnregMcFilterEnableSet
(
    IN  GT_U8                      dev,
    IN  CPSS_IP_PROTOCOL_STACK_ENT protocolStack,
    IN  GT_BOOL                    enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgGenExcludeLinkLocalMcFromUnregMcFilterEnableSet)
{
/*
    ITERATE_DEVICES(DxChXcat and above)
    1.1. Call with protocolStack[CPSS_IP_PROTOCOL_IPV4_E /
                                 CPSS_IP_PROTOCOL_IPV6_E],
                   enable[GT_TRUE / GT_FALSE],
    Expected: GT_OK.
    1.2. Call cpssDxChBrgGenExcludeLinkLocalMcFromUnregMcFilterEnableGet
           with the same parameters.
    Expected: GT_OK and the same values than was set.
    1.3. Call with wrong protocolStack[CPSS_IP_PROTOCOL_IPV4V6_E].
    Expected: GT_BAD_PARAM.
    1.4. Call api with wrong protocolStack [wrong enum values].
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS st = GT_OK;

    GT_U8                      dev;
    CPSS_IP_PROTOCOL_STACK_ENT protocolStack = CPSS_IP_PROTOCOL_IPV4_E;
    GT_BOOL                    enable = GT_FALSE;
    GT_BOOL                    enableGet = GT_FALSE;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with protocolStack[CPSS_IP_PROTOCOL_IPV4_E /
                                         CPSS_IP_PROTOCOL_IPV6_E],
                           enable[GT_TRUE / GT_FALSE],
            Expected: GT_OK.
        */

        /* call with protocolStack[CPSS_IP_PROTOCOL_IPV4_E],
                     enable[GT_TRUE] */
        protocolStack = CPSS_IP_PROTOCOL_IPV4_E;
        enable = GT_TRUE;

        st = cpssDxChBrgGenExcludeLinkLocalMcFromUnregMcFilterEnableSet(dev,
                                     protocolStack, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call cpssDxChBrgGenExcludeLinkLocalMcFromUnregMcFilterEnableGet
                   with the same parameters.
            Expected: GT_OK and the same values than was set.
        */
        st = cpssDxChBrgGenExcludeLinkLocalMcFromUnregMcFilterEnableGet(dev,
                                     protocolStack, &enableGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                  "cpssDxChBrgGenExcludeLinkLocalMcFromUnregMcFilterEnableGet: %d ", dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                       "got another enable then was set: %d", dev);

        /* call with protocolStack[CPSS_IP_PROTOCOL_IPV6_E],
                     enable[GT_FALSE] */
        protocolStack = CPSS_IP_PROTOCOL_IPV6_E;
        enable = GT_FALSE;

        st = cpssDxChBrgGenExcludeLinkLocalMcFromUnregMcFilterEnableSet(dev,
                                     protocolStack, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* call get func again, 1.2. */
        st = cpssDxChBrgGenExcludeLinkLocalMcFromUnregMcFilterEnableGet(dev,
                                     protocolStack, &enableGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                  "cpssDxChBrgGenExcludeLinkLocalMcFromUnregMcFilterEnableGet: %d ", dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                       "got another enable then was set: %d", dev);

        /*
            1.3. Call with wrong protocolStack[CPSS_IP_PROTOCOL_IPV4V6_E].
            Expected: GT_BAD_PARAM.
        */
        protocolStack = CPSS_IP_PROTOCOL_IPV4V6_E;
        enable = GT_TRUE;

        st = cpssDxChBrgGenExcludeLinkLocalMcFromUnregMcFilterEnableSet(dev,
                                     protocolStack, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        protocolStack = CPSS_IP_PROTOCOL_IPV4_E;

        /*
            1.4. Call api with wrong protocolStack [wrong enum values].
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChBrgGenExcludeLinkLocalMcFromUnregMcFilterEnableSet
                            (dev, protocolStack, enable),
                            protocolStack);
    }

    /* restore correct values */
    protocolStack = CPSS_IP_PROTOCOL_IPV4_E;
    enable = GT_TRUE;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgGenExcludeLinkLocalMcFromUnregMcFilterEnableSet(dev,
                                     protocolStack, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgGenExcludeLinkLocalMcFromUnregMcFilterEnableSet(dev,
                                     protocolStack, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgGenExcludeLinkLocalMcFromUnregMcFilterEnableGet
(
    IN  GT_U8                       dev,
    IN  CPSS_IP_PROTOCOL_STACK_ENT  protocolStack,
    OUT GT_BOOL                     *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgGenExcludeLinkLocalMcFromUnregMcFilterEnableGet)
{
/*
    ITERATE_DEVICES(DxChXcat and above)
    1.1. Call with protocolStack[CPSS_IP_PROTOCOL_IPV4_E /
                                 CPSS_IP_PROTOCOL_IPV6_E].
    Expected: GT_OK.
    1.2. Call with wrong protocolStack[CPSS_IP_PROTOCOL_IPV4V6_E].
    Expected: GT_BAD_PARAM.
    1.3. Call api with wrong protocolStack [wrong enum values].
    Expected: GT_BAD_PARAM.
    1.4. Call api with wrong enablePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS st = GT_OK;

    GT_U8                       dev;
    CPSS_IP_PROTOCOL_STACK_ENT  protocolStack = CPSS_IP_PROTOCOL_IPV4_E;
    GT_BOOL                     enable;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with protocolStack[CPSS_IP_PROTOCOL_IPV4_E /
                                         CPSS_IP_PROTOCOL_IPV6_E].
            Expected: GT_OK.
        */

        /* call with protocolStack[CPSS_IP_PROTOCOL_IPV4_E] */
        protocolStack = CPSS_IP_PROTOCOL_IPV4_E;

        st = cpssDxChBrgGenExcludeLinkLocalMcFromUnregMcFilterEnableGet(dev,
                                     protocolStack, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* call with protocolStack[CPSS_IP_PROTOCOL_IPV6_E] */
        protocolStack = CPSS_IP_PROTOCOL_IPV6_E;

        st = cpssDxChBrgGenExcludeLinkLocalMcFromUnregMcFilterEnableGet(dev,
                                     protocolStack, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with wrong protocolStack[CPSS_IP_PROTOCOL_IPV4V6_E].
            Expected: GT_BAD_PARAM.
        */
        protocolStack = CPSS_IP_PROTOCOL_IPV4V6_E;
        enable = GT_TRUE;

        st = cpssDxChBrgGenExcludeLinkLocalMcFromUnregMcFilterEnableSet(dev,
                                     protocolStack, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        protocolStack = CPSS_IP_PROTOCOL_IPV4_E;

        /*
            1.3. Call api with wrong protocolStack [wrong enum values].
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChBrgGenExcludeLinkLocalMcFromUnregMcFilterEnableGet
                            (dev, protocolStack, &enable),
                            protocolStack);

        /*
            1.4. Call api with wrong enablePtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChBrgGenExcludeLinkLocalMcFromUnregMcFilterEnableGet(dev,
                                     protocolStack, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,
                                   "%d, enablePtr = NULL", dev);
    }

    /* restore correct values */
    protocolStack = CPSS_IP_PROTOCOL_IPV4_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgGenExcludeLinkLocalMcFromUnregMcFilterEnableGet(dev,
                                     protocolStack, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgGenExcludeLinkLocalMcFromUnregMcFilterEnableGet(dev,
                                     protocolStack, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgGenPortIeeeReservedMcastLearningEnableSet
(
    IN GT_U8    devNum,
    IN GT_U8    portNum,
    IN GT_BOOL  enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgGenPortIeeeReservedMcastLearningEnableSet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (xCat2 and above)
    1.1.1. Call with enable [GT_FALSE and GT_TRUE].
    Expected: GT_OK.
*/
    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_PORT_NUM       port  = BRG_GEN_VALID_PHY_PORT_CNS;
    GT_BOOL     state = GT_TRUE;
    GT_BOOL     stateGet;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                     UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextVirtPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call with enable [GT_FALSE and GT_TRUE].
                Expected: GT_OK.
            */

            /* Call function with status [GT_FALSE] */
            state = GT_FALSE;

            st = cpssDxChBrgGenPortIeeeReservedMcastLearningEnableSet(dev, port, state);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, state);

            st = cpssDxChBrgGenPortIeeeReservedMcastLearningEnableGet(dev, port, &stateGet);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, stateGet);

            /* Verifying struct fields */
            UTF_VERIFY_EQUAL2_STRING_MAC(state, stateGet,
                 "get another state than was set: %d, %d", dev, port);

            /* Call function with status [GT_TRUE] */
            state = GT_TRUE;

            st = cpssDxChBrgGenPortIeeeReservedMcastLearningEnableSet(dev, port, state);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, state);

            st = cpssDxChBrgGenPortIeeeReservedMcastLearningEnableGet(dev, port, &stateGet);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, stateGet);

            /* Verifying struct fields */
            UTF_VERIFY_EQUAL2_STRING_MAC(state, stateGet,
                 "get another state than was set: %d, %d", dev, port);
        }

        state = GT_TRUE;

        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextVirtPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChBrgGenPortIeeeReservedMcastLearningEnableSet(dev, port, state);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */

        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChBrgGenPortIeeeReservedMcastLearningEnableSet(dev, port, state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                     */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgGenPortIeeeReservedMcastLearningEnableSet(dev, port, state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    state = GT_TRUE;

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = BRG_GEN_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_LION_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgGenPortIeeeReservedMcastLearningEnableSet(dev, port, state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgGenPortIeeeReservedMcastLearningEnableSet(dev, port, state);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgGenPortIeeeReservedMcastLearningEnableGet
(
    IN  GT_U8    devNum,
    IN  GT_U8    portNum,
    OUT GT_BOOL  *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgGenPortIeeeReservedMcastLearningEnableGet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (xCat2 and above)
    1.1.1. Call with non-null statePtr.
    Expected: GT_OK.
    1.1.2. Call with statePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st      = GT_OK;

    GT_U8       dev;
    GT_PORT_NUM       port    = BRG_GEN_VALID_PHY_PORT_CNS;
    GT_BOOL     state;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                     UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextVirtPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call with non-null statePtr.
                Expected: GT_OK.
            */
            st = cpssDxChBrgGenPortIeeeReservedMcastLearningEnableGet(dev, port, &state);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, state);

            /*
                1.1.2. Call with statePtr [NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChBrgGenPortIeeeReservedMcastLearningEnableGet(dev, port, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, NULL", dev, port);
        }

        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available
           physical ports.
        */
        while(GT_OK == prvUtfNextVirtPortGet(&port, GT_FALSE))
        {
            st = cpssDxChBrgGenPortIeeeReservedMcastLearningEnableGet(dev, port, &state);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChBrgGenPortIeeeReservedMcastLearningEnableGet(dev, port, &state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgGenPortIeeeReservedMcastLearningEnableGet(dev, port, &state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    port = BRG_GEN_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                     UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgGenPortIeeeReservedMcastLearningEnableGet(dev, port, &state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgGenPortIeeeReservedMcastLearningEnableGet(dev, port, &state);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgGenPortRateLimitDropCntrEnableSet
(
    IN GT_U8    devNum,
    IN GT_U8    portNum,
    IN GT_BOOL  enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgGenPortRateLimitDropCntrEnableSet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2)
    1.1.1. Call with enable [GT_FALSE / GT_TRUE].
    Expected: GT_OK.
    1.1.2. Call cpssDxChBrgGenPortRateLimitDropCntrEnableGet
    Expected: GT_OK and the same state.
*/
    GT_STATUS   st   = GT_OK;
    GT_U8       dev  = 0;
    GT_PHYSICAL_PORT_NUM    port = BRG_GEN_VALID_PHY_PORT_CNS;

    GT_BOOL     enable    = GT_FALSE;
    GT_BOOL     enableGet = GT_FALSE;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_LION_E | UTF_LION2_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call with enable [GT_FALSE and GT_TRUE].
                Expected: GT_OK.
            */

            /* Call function with status [GT_FALSE] */
            enable = GT_FALSE;

            st = cpssDxChBrgGenPortRateLimitDropCntrEnableSet(dev, port, enable);
            if(port >= 256)
            {
                /* feature is limited to 256 ports */
                UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                break;
            }
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);

            /*
                1.1.2. Call cpssDxChBrgGenPortRateLimitDropCntrEnableGet
                Expected: GT_OK and the same enable.
            */

            st = cpssDxChBrgGenPortRateLimitDropCntrEnableGet(dev, port, &enableGet);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enableGet);

            /* Verifying struct fields */
            UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                 "get another state than was set: %d, %d", dev, port);

            /*
                1.1.1. Call with enable [GT_FALSE and GT_TRUE].
                Expected: GT_OK.
            */

            /* Call function with  [GT_TRUE] */
            enable = GT_TRUE;

            st = cpssDxChBrgGenPortRateLimitDropCntrEnableSet(dev, port, enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);

            /*
                1.1.2. Call cpssDxChBrgGenPortRateLimitDropCntrEnableGet
                Expected: GT_OK and the same enable.
            */

            st = cpssDxChBrgGenPortRateLimitDropCntrEnableGet(dev, port, &enableGet);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enableGet);

            /* Verifying struct fields */
            UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                 "get another state than was set: %d, %d", dev, port);
        }

        enable = GT_TRUE;

        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChBrgGenPortRateLimitDropCntrEnableSet(dev, port, enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChBrgGenPortRateLimitDropCntrEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgGenPortRateLimitDropCntrEnableSet(dev, port, enable);
        if (UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }
    }

    enable = GT_TRUE;

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = BRG_GEN_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_LION_E | UTF_LION2_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgGenPortRateLimitDropCntrEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgGenPortRateLimitDropCntrEnableSet(dev, port, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgGenPortRateLimitDropCntrEnableGet
(
    IN  GT_U8        devNum,
    IN  GT_U8        portNum,
    OUT GT_BOOL     *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgGenPortRateLimitDropCntrEnableGet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2)
    1.1.1. Call function with not NULL enablePtr.
    Expected: GT_OK.
    1.1.2. Call function with enablePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st   = GT_OK;
    GT_U8       dev  = 0;
    GT_PHYSICAL_PORT_NUM       port = BRG_GEN_VALID_PHY_PORT_CNS;

    GT_BOOL     enable = GT_FALSE;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_LION_E | UTF_LION2_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call function with not NULL enablePtr.
                Expected: GT_OK.
            */
            st =cpssDxChBrgGenPortRateLimitDropCntrEnableGet(dev, port, &enable);
            if(port >= 256)
            {
                /* feature is limited to 256 ports */
                UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                break;
            }
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*
                1.1.2. Call function with enablePtr [NULL].
                Expected: GT_BAD_PTR.
            */
            st =cpssDxChBrgGenPortRateLimitDropCntrEnableGet(dev, port, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, enablePtr = NULL", dev, port);
        }

        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st =cpssDxChBrgGenPortRateLimitDropCntrEnableGet(dev, port, &enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st =cpssDxChBrgGenPortRateLimitDropCntrEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st =cpssDxChBrgGenPortRateLimitDropCntrEnableGet(dev, port, &enable);
        if (UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }
    }

    /* 2. For not-active devices and devices from non-applicable family */
    port =BRG_GEN_VALID_PHY_PORT_CNS;

    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_LION_E | UTF_LION2_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st =cpssDxChBrgGenPortRateLimitDropCntrEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st =cpssDxChBrgGenPortRateLimitDropCntrEnableGet(dev, port, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgGenRateLimitDropCntrSet
(
    IN  GT_U8   devNum,
    IN  GT_U64  value
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgGenRateLimitDropCntrSet)
{
/*
    ITERATE_DEVICES (DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2)
    1.1. Call with the value [0 / 100 / 0xFFFFFFFFFF].
    Expected: GT_OK.
    1.2. Call cpssDxChBrgGenRateLimitDropCntrGet with non-NULL valuePtr.
    Expected: GT_OK and the same value.
    1.3. Call with out of range value [0x100FFFFFFFF] .
    Expected: NOT GT_OK.
*/
    GT_STATUS   st   = GT_OK;
    GT_U8       dev  = 0 ;

    GT_U64      value    = {{0,0}};
    GT_U64      valueGet = {{0,0}};
    GT_BOOL     isEqual  = GT_FALSE;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_LION_E | UTF_LION2_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with the value [0 / 100 / 0xFFFFFFFFFF].
            Expected: GT_OK.
        */

        /* call with value = 0 */
        value.l[0] = 0;
        value.l[1] = 0;

        st = cpssDxChBrgGenRateLimitDropCntrSet(dev, value);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "%0.8X%0.8X, %d", value.l[1], value.l[0], dev);

        /*
            1.2. Call cpssDxChBrgGenRateLimitDropCntrGet with non-NULL valuePtr.
            Expected: GT_OK and the same value.
        */

        st = cpssDxChBrgGenRateLimitDropCntrGet(dev, &valueGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                       "cpssDxChBrgGenRateLimitDropCntrGet: %d", dev);

        /* Verifying values */
        isEqual = (0 == cpssOsMemCmp((GT_VOID*) &value,
                  (GT_VOID*) &valueGet, sizeof(GT_U64))) ? GT_TRUE : GT_FALSE;
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_TRUE, isEqual,
                    "get another value %0.8X%0.8X than was set: %d",value.l[1], value.l[0], dev);

        /* call with value = 100 */
        value.l[0] = 100;
        value.l[1] = 0;

        st = cpssDxChBrgGenRateLimitDropCntrSet(dev, value);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "%0.8X%0.8X, %d", value.l[1], value.l[0], dev);

        /*
            1.2. Call cpssDxChBrgGenRateLimitDropCntGet with non-NULL valuePtr.
            Expected: GT_OK and the same value.
        */
        st = cpssDxChBrgGenRateLimitDropCntrGet(dev, &valueGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                       "cpssDxChBrgGenRateLimitDropCntrGet: %d", dev);

        /* Verifying values */
        isEqual = (0 == cpssOsMemCmp((GT_VOID*) &value,
                  (GT_VOID*) &valueGet, sizeof(GT_U64))) ? GT_TRUE : GT_FALSE;
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_TRUE, isEqual,
                    "get another value %0.8X%0.8X than was set: %d",value.l[1], value.l[0], dev);

        /* call with value = 0xFFFFFFFFFF */
        value.l[0] = 0xFFFFFFFF;
        value.l[1] = 0xFF;

        st = cpssDxChBrgGenRateLimitDropCntrSet(dev, value);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "%0.8X%0.8X, %d", value.l[1], value.l[0], dev);

        /*
            1.2. Call cpssDxChBrgGenRateLimitDropCntrGet with non-NULL valuePtr.
            Expected: GT_OK and the same value.
        */

        st = cpssDxChBrgGenRateLimitDropCntrGet(dev, &valueGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                       "cpssDxChBrgGenRateLimitDropCntrGet: %d", dev);

        /* Verifying values */
        isEqual = (0 == cpssOsMemCmp((GT_VOID*) &value,
                  (GT_VOID*) &valueGet,sizeof(GT_U64))) ? GT_TRUE : GT_FALSE;
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_TRUE, isEqual,
                    "get another value %0.8X%0.8X than was set: %d",value.l[1], value.l[0], dev);

        /*
            1.3. Call with out of range value [0x100FFFFFFFF] .
            Expected: NOT GT_OK.
        */
        value.l[0] = 0xFFFFFFFF;
        value.l[1] = 0x100;

        {
            GT_U16 portGroupNum = 1;
            /* calculate number of port groups */
            st = prvCpssPortGroupsNumActivePortGroupsInBmpGet(dev,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,PRV_CPSS_DXCH_UNIT_L2I_E,&portGroupNum);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                       "prvCpssPortGroupsNumActivePortGroupsInBmpGet: %d", dev);

            /* the multi core device can split the value between the port groups !!! */
            value.l[1] *= portGroupNum;
        }


        st = cpssDxChBrgGenRateLimitDropCntrSet(dev, value);
        UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st,
              "Value %0.8X%0.8X is another as GT_OUT_OF_RANGE for %d", value.l[1], value.l[0], dev);

        value.l[1] -= 1;
        st = cpssDxChBrgGenRateLimitDropCntrSet(dev, value);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,
              "Value Should Fit into the 40Bits counter in the device (per core , per pipe)");
    }

    /*set defaul value*/
    value.l[0] = 0;
    value.l[1] = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_LION_E | UTF_LION2_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgGenRateLimitDropCntrSet(dev, value);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgGenRateLimitDropCntrSet(dev, value);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgGenRateLimitDropCntrGet
(
    IN  GT_U8   devNum,
    OUT GT_U64  *valuePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgGenRateLimitDropCntrGet)
{
/*
    ITERATE_DEVICES (DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2)
    1.1. Call with non-null valuePtr.
    Expected: GT_OK.
    1.2. Call with valuePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev = 0;

    GT_U64      value = {{0}};


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_LION_E | UTF_LION2_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with non-null valuePtr.
            Expected: GT_OK.
        */
        st = cpssDxChBrgGenRateLimitDropCntrGet(dev, &value);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with null valuePtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChBrgGenRateLimitDropCntrGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, valuePtr = NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_LION_E | UTF_LION2_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgGenRateLimitDropCntrGet(dev, &value);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgGenRateLimitDropCntrGet(dev, &value);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}
/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgGenIngressPortBcFilterDaCommandSet
(
    IN GT_U8                devNum,
    IN GT_PORT_NUM          portNum,
    IN CPSS_PACKET_CMD_ENT  cmd
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgGenIngressPortBcFilterDaCommandSet)
{
/*
    ITERATE_DEVICES_VIRT_PORTS (SIP5)
    1.1.1. Call with cmd [CPSS_PACKET_CMD_FORWARD_E /
                          CPSS_PACKET_CMD_MIRROR_TO_CPU_E /
                          CPSS_PACKET_CMD_TRAP_TO_CPU_E /
                          CPSS_PACKET_CMD_DROP_HARD_E /
                          CPSS_PACKET_CMD_DROP_SOFT_E].
    Expected: GT_OK.
    1.1.2. Call cpssDxChBrgGenIngressPortBcFilterDaCommandGet with non NULL cmdPtr.
    Expected: GT_OK and the same cmd.
    1.1.3. Call with not supported cmd [CPSS_PACKET_CMD_ROUTE_E /
                                        CPSS_PACKET_CMD_ROUTE_AND_MIRROR_E /
                                        CPSS_PACKET_CMD_BRIDGE_AND_MIRROR_E /
                                        CPSS_PACKET_CMD_BRIDGE_E /
                                        CPSS_PACKET_CMD_NONE_E],
    Expected: NON GT_OK.
    1.1.4. Call with cmd [wrong enum values].
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st   = GT_OK;
    GT_U8       dev  = 0;
    GT_PORT_NUM port = 0;

    CPSS_PACKET_CMD_ENT  cmd    = CPSS_PACKET_CMD_FORWARD_E;
    CPSS_PACKET_CMD_ENT  cmdGet = CPSS_PACKET_CMD_FORWARD_E;

    GT_U32      notAppFamilyBmp;
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
                1.1.1. Call with cmd [CPSS_PACKET_CMD_FORWARD_E /
                                      CPSS_PACKET_CMD_MIRROR_TO_CPU_E /
                                      CPSS_PACKET_CMD_TRAP_TO_CPU_E /
                                      CPSS_PACKET_CMD_DROP_HARD_E /
                                      CPSS_PACKET_CMD_DROP_SOFT_E].
                Expected: GT_OK.
            */
            /* call with cmd = CPSS_PACKET_CMD_FORWARD_E */
            cmd = CPSS_PACKET_CMD_FORWARD_E;

            st = cpssDxChBrgGenIngressPortBcFilterDaCommandSet(dev, port, cmd);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, cmd);

            /*
                1.1.2. Call cpssDxChBrgGenIngressPortBcFilterDaCommandGet with non NULL cmdPtr.
                Expected: GT_OK and the same cmd.
            */
            st = cpssDxChBrgGenIngressPortBcFilterDaCommandGet(dev, port, &cmdGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                       "cpssDxChBrgGenIngressPortBcFilterDaCommandGet: %d, %d", dev, port);

            /* verifying values */
            UTF_VERIFY_EQUAL2_STRING_MAC(cmd, cmdGet,
                       "get another cmd than was set: %d, %d", dev, port);

            /* call with cmd = CPSS_PACKET_CMD_MIRROR_TO_CPU_E */
            cmd = CPSS_PACKET_CMD_MIRROR_TO_CPU_E;

            st = cpssDxChBrgGenIngressPortBcFilterDaCommandSet(dev, port, cmd);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, cmd);

            /*
                1.1.2. Call cpssDxChBrgGenIngressPortBcFilterDaCommandGet with non NULL cmdPtr.
                Expected: GT_OK and the same cmd.
            */
            st = cpssDxChBrgGenIngressPortBcFilterDaCommandGet(dev, port, &cmdGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                       "cpssDxChBrgGenIngressPortBcFilterDaCommandGet: %d, %d", dev, port);

            /* verifying values */
            UTF_VERIFY_EQUAL2_STRING_MAC(cmd, cmdGet,
                       "get another cmd than was set: %d, %d", dev, port);

            /* call with cmd = CPSS_PACKET_CMD_TRAP_TO_CPU_E */
            cmd = CPSS_PACKET_CMD_TRAP_TO_CPU_E;

            st = cpssDxChBrgGenIngressPortBcFilterDaCommandSet(dev, port, cmd);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, cmd);

            /*
                1.1.2. Call cpssDxChBrgGenIngressPortBcFilterDaCommandGet with non NULL cmdPtr.
                Expected: GT_OK and the same cmd.
            */
            st = cpssDxChBrgGenIngressPortBcFilterDaCommandGet(dev, port, &cmdGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                       "cpssDxChBrgGenIngressPortBcFilterDaCommandGet: %d, %d", dev, port);

            /* verifying values */
            UTF_VERIFY_EQUAL2_STRING_MAC(cmd, cmdGet,
                       "get another cmd than was set: %d, %d", dev, port);

            /* call with cmd = CPSS_PACKET_CMD_DROP_HARD_E */
            cmd = CPSS_PACKET_CMD_DROP_HARD_E;

            st = cpssDxChBrgGenIngressPortBcFilterDaCommandSet(dev, port, cmd);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, cmd);

            /*
                1.1.2. Call cpssDxChBrgGenIngressPortBcFilterDaCommandGet with non NULL cmdPtr.
                Expected: GT_OK and the same cmd.
            */
            st = cpssDxChBrgGenIngressPortBcFilterDaCommandGet(dev, port, &cmdGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                       "cpssDxChBrgGenIngressPortBcFilterDaCommandGet: %d, %d", dev, port);

            /* verifying values */
            UTF_VERIFY_EQUAL2_STRING_MAC(cmd, cmdGet,
                       "get another cmd than was set: %d, %d", dev, port);

            /* call with cmd = CPSS_PACKET_CMD_DROP_SOFT_E */
            cmd = CPSS_PACKET_CMD_DROP_SOFT_E;

            st = cpssDxChBrgGenIngressPortBcFilterDaCommandSet(dev, port, cmd);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, cmd);

            /*
                1.1.2. Call cpssDxChBrgGenIngressPortBcFilterDaCommandGet with non NULL cmdPtr.
                Expected: GT_OK and the same cmd.
            */
            st = cpssDxChBrgGenIngressPortBcFilterDaCommandGet(dev, port, &cmdGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                       "cpssDxChBrgGenIngressPortBcFilterDaCommandGet: %d, %d", dev, port);

            /* verifying values */
            UTF_VERIFY_EQUAL2_STRING_MAC(cmd, cmdGet,
                       "get another cmd than was set: %d, %d", dev, port);

            /*
                1.1.3. Call with not supported cmd [CPSS_PACKET_CMD_ROUTE_E /
                                                    CPSS_PACKET_CMD_ROUTE_AND_MIRROR_E /
                                                    CPSS_PACKET_CMD_BRIDGE_AND_MIRROR_E /
                                                    CPSS_PACKET_CMD_BRIDGE_E /
                                                    CPSS_PACKET_CMD_NONE_E],
                Expected: NON GT_OK.
            */
            /* call with cmd = CPSS_PACKET_CMD_ROUTE_E */
            cmd = CPSS_PACKET_CMD_ROUTE_E;

            st = cpssDxChBrgGenIngressPortBcFilterDaCommandSet(dev, port, cmd);
            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, cmd);

            /* call with cmd = CPSS_PACKET_CMD_ROUTE_AND_MIRROR_E */
            cmd = CPSS_PACKET_CMD_ROUTE_AND_MIRROR_E;

            st = cpssDxChBrgGenIngressPortBcFilterDaCommandSet(dev, port, cmd);
            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, cmd);

            /* call with cmd = CPSS_PACKET_CMD_BRIDGE_AND_MIRROR_E */
            cmd = CPSS_PACKET_CMD_BRIDGE_AND_MIRROR_E;

            st = cpssDxChBrgGenIngressPortBcFilterDaCommandSet(dev, port, cmd);
            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, cmd);

            /* call with cmd = CPSS_PACKET_CMD_BRIDGE_E */
            cmd = CPSS_PACKET_CMD_BRIDGE_E;

            st = cpssDxChBrgGenIngressPortBcFilterDaCommandSet(dev, port, cmd);
            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, cmd);

            /* call with cmd = CPSS_PACKET_CMD_NONE_E */
            cmd = CPSS_PACKET_CMD_NONE_E;

            st = cpssDxChBrgGenIngressPortBcFilterDaCommandSet(dev, port, cmd);
            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, cmd);

            /* restore value */
            cmd = CPSS_PACKET_CMD_FORWARD_E;

            /*
                1.1.4. Call with cmd [wrong enum values].
                Expected: GT_BAD_PARAM.
            */
            UTF_ENUMS_CHECK_MAC(cpssDxChBrgGenIngressPortBcFilterDaCommandSet
                                (dev, port, cmd),
                                cmd);
        }

        cmd = CPSS_PACKET_CMD_FORWARD_E;

        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChBrgGenIngressPortBcFilterDaCommandSet(dev, port, cmd);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChBrgGenIngressPortBcFilterDaCommandSet(dev, port, cmd);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgGenIngressPortBcFilterDaCommandSet(dev, port, cmd);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    cmd = CPSS_PACKET_CMD_FORWARD_E;
    port = 0;

    /* 2. For not active devices check that function returns non GT_OK.*/

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgGenIngressPortBcFilterDaCommandSet(dev, port, cmd);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgGenIngressPortBcFilterDaCommandSet(dev, port, cmd);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgGenIngressPortBcFilterDaCommandGet
(
    IN  GT_U8                   devNum,
    IN  GT_PORT_NUM             portNum,
    OUT CPSS_PACKET_CMD_ENT     *cmdPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgGenIngressPortBcFilterDaCommandGet)
{
/*
    ITERATE_DEVICES_VIRT_PORTS (SIP5)
    1.1.1. Call with not NULL cmdPtr.
    Expected: GT_OK.
    1.1.2. Call with cmdPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st   = GT_OK;
    GT_U8       dev  = 0;
    GT_PORT_NUM port = 0;

    CPSS_PACKET_CMD_ENT cmd = CPSS_PACKET_CMD_FORWARD_E;
    GT_U32      notAppFamilyBmp;
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
                1.1.1. Call with not NULL cmdPtr.
                Expected: GT_OK.
            */
            st = cpssDxChBrgGenIngressPortBcFilterDaCommandGet(dev, port, &cmd);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*
                1.1.2. Call with cmdPtr [NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChBrgGenIngressPortBcFilterDaCommandGet(dev, port, NULL);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, cmdPtr = NULL", dev);
        }

        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChBrgGenIngressPortBcFilterDaCommandGet(dev, port, &cmd);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChBrgGenIngressPortBcFilterDaCommandGet(dev, port, &cmd);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                     */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgGenIngressPortBcFilterDaCommandGet(dev, port, &cmd);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    port = 0;

    /* 2. For not active devices check that function returns non GT_OK.*/

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgGenIngressPortBcFilterDaCommandGet(dev, port, &cmd);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgGenIngressPortBcFilterDaCommandGet(dev, port, &cmd);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgGenIngressPortUnknownUcFilterDaCommandSet
(
    IN GT_U8                devNum,
    IN GT_PORT_NUM          portNum,
    IN CPSS_PACKET_CMD_ENT  cmd
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgGenIngressPortUnknownUcFilterDaCommandSet)
{
/*
    ITERATE_DEVICES_VIRT_PORTS (SIP5)
    1.1.1. Call with cmd [CPSS_PACKET_CMD_FORWARD_E /
                          CPSS_PACKET_CMD_MIRROR_TO_CPU_E /
                          CPSS_PACKET_CMD_TRAP_TO_CPU_E /
                          CPSS_PACKET_CMD_DROP_HARD_E /
                          CPSS_PACKET_CMD_DROP_SOFT_E].
    Expected: GT_OK.
    1.1.2. Call cpssDxChBrgGenIngressPortUnknownUcFilterDaCommandGet with non NULL cmdPtr.
    Expected: GT_OK and the same cmd.
    1.1.3. Call with not supported cmd [CPSS_PACKET_CMD_ROUTE_E /
                                        CPSS_PACKET_CMD_ROUTE_AND_MIRROR_E /
                                        CPSS_PACKET_CMD_BRIDGE_AND_MIRROR_E /
                                        CPSS_PACKET_CMD_BRIDGE_E /
                                        CPSS_PACKET_CMD_NONE_E],
    Expected: NON GT_OK.
    1.1.4. Call with cmd [wrong enum values].
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st   = GT_OK;
    GT_U8       dev  = 0;
    GT_PORT_NUM port = 0;

    CPSS_PACKET_CMD_ENT  cmd    = CPSS_PACKET_CMD_FORWARD_E;
    CPSS_PACKET_CMD_ENT  cmdGet = CPSS_PACKET_CMD_FORWARD_E;
    GT_U32      notAppFamilyBmp;
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
                1.1.1. Call with cmd [CPSS_PACKET_CMD_FORWARD_E /
                                      CPSS_PACKET_CMD_MIRROR_TO_CPU_E /
                                      CPSS_PACKET_CMD_TRAP_TO_CPU_E /
                                      CPSS_PACKET_CMD_DROP_HARD_E /
                                      CPSS_PACKET_CMD_DROP_SOFT_E].
                Expected: GT_OK.
            */
            /* call with cmd = CPSS_PACKET_CMD_FORWARD_E */
            cmd = CPSS_PACKET_CMD_FORWARD_E;

            st = cpssDxChBrgGenIngressPortUnknownUcFilterDaCommandSet(dev, port, cmd);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, cmd);

            /*
                1.1.2. Call cpssDxChBrgGenIngressPortUnknownUcFilterDaCommandGet with non NULL cmdPtr.
                Expected: GT_OK and the same cmd.
            */
            st = cpssDxChBrgGenIngressPortUnknownUcFilterDaCommandGet(dev, port, &cmdGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                       "cpssDxChBrgGenIngressPortUnknownUcFilterDaCommandGet: %d, %d", dev, port);

            /* verifying values */
            UTF_VERIFY_EQUAL2_STRING_MAC(cmd, cmdGet,
                       "get another cmd than was set: %d, %d", dev, port);

            /* call with cmd = CPSS_PACKET_CMD_MIRROR_TO_CPU_E */
            cmd = CPSS_PACKET_CMD_MIRROR_TO_CPU_E;

            st = cpssDxChBrgGenIngressPortUnknownUcFilterDaCommandSet(dev, port, cmd);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, cmd);

            /*
                1.1.2. Call cpssDxChBrgGenIngressPortUnknownUcFilterDaCommandGet with non NULL cmdPtr.
                Expected: GT_OK and the same cmd.
            */
            st = cpssDxChBrgGenIngressPortUnknownUcFilterDaCommandGet(dev, port, &cmdGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                       "cpssDxChBrgGenIngressPortUnknownUcFilterDaCommandGet: %d, %d", dev, port);

            /* verifying values */
            UTF_VERIFY_EQUAL2_STRING_MAC(cmd, cmdGet,
                       "get another cmd than was set: %d, %d", dev, port);

            /* call with cmd = CPSS_PACKET_CMD_TRAP_TO_CPU_E */
            cmd = CPSS_PACKET_CMD_TRAP_TO_CPU_E;

            st = cpssDxChBrgGenIngressPortUnknownUcFilterDaCommandSet(dev, port, cmd);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, cmd);

            /*
                1.1.2. Call cpssDxChBrgGenIngressPortUnknownUcFilterDaCommandGet with non NULL cmdPtr.
                Expected: GT_OK and the same cmd.
            */
            st = cpssDxChBrgGenIngressPortUnknownUcFilterDaCommandGet(dev, port, &cmdGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                       "cpssDxChBrgGenIngressPortUnknownUcFilterDaCommandGet: %d, %d", dev, port);

            /* verifying values */
            UTF_VERIFY_EQUAL2_STRING_MAC(cmd, cmdGet,
                       "get another cmd than was set: %d, %d", dev, port);

            /* call with cmd = CPSS_PACKET_CMD_DROP_HARD_E */
            cmd = CPSS_PACKET_CMD_DROP_HARD_E;

            st = cpssDxChBrgGenIngressPortUnknownUcFilterDaCommandSet(dev, port, cmd);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, cmd);

            /*
                1.1.2. Call cpssDxChBrgGenIngressPortUnknownUcFilterDaCommandGet with non NULL cmdPtr.
                Expected: GT_OK and the same cmd.
            */
            st = cpssDxChBrgGenIngressPortUnknownUcFilterDaCommandGet(dev, port, &cmdGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                       "cpssDxChBrgGenIngressPortUnknownUcFilterDaCommandGet: %d, %d", dev, port);

            /* verifying values */
            UTF_VERIFY_EQUAL2_STRING_MAC(cmd, cmdGet,
                       "get another cmd than was set: %d, %d", dev, port);

            /* call with cmd = CPSS_PACKET_CMD_DROP_SOFT_E */
            cmd = CPSS_PACKET_CMD_DROP_SOFT_E;

            st = cpssDxChBrgGenIngressPortUnknownUcFilterDaCommandSet(dev, port, cmd);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, cmd);

            /*
                1.1.2. Call cpssDxChBrgGenIngressPortUnknownUcFilterDaCommandGet with non NULL cmdPtr.
                Expected: GT_OK and the same cmd.
            */
            st = cpssDxChBrgGenIngressPortUnknownUcFilterDaCommandGet(dev, port, &cmdGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                       "cpssDxChBrgGenIngressPortUnknownUcFilterDaCommandGet: %d, %d", dev, port);

            /* verifying values */
            UTF_VERIFY_EQUAL2_STRING_MAC(cmd, cmdGet,
                       "get another cmd than was set: %d, %d", dev, port);

            /*
                1.1.3. Call with not supported cmd [CPSS_PACKET_CMD_ROUTE_E /
                                                    CPSS_PACKET_CMD_ROUTE_AND_MIRROR_E /
                                                    CPSS_PACKET_CMD_BRIDGE_AND_MIRROR_E /
                                                    CPSS_PACKET_CMD_BRIDGE_E /
                                                    CPSS_PACKET_CMD_NONE_E],
                Expected: NON GT_OK.
            */
            /* call with cmd = CPSS_PACKET_CMD_ROUTE_E */
            cmd = CPSS_PACKET_CMD_ROUTE_E;

            st = cpssDxChBrgGenIngressPortUnknownUcFilterDaCommandSet(dev, port, cmd);
            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, cmd);

            /* call with cmd = CPSS_PACKET_CMD_ROUTE_AND_MIRROR_E */
            cmd = CPSS_PACKET_CMD_ROUTE_AND_MIRROR_E;

            st = cpssDxChBrgGenIngressPortUnknownUcFilterDaCommandSet(dev, port, cmd);
            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, cmd);

            /* call with cmd = CPSS_PACKET_CMD_BRIDGE_AND_MIRROR_E */
            cmd = CPSS_PACKET_CMD_BRIDGE_AND_MIRROR_E;

            st = cpssDxChBrgGenIngressPortUnknownUcFilterDaCommandSet(dev, port, cmd);
            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, cmd);

            /* call with cmd = CPSS_PACKET_CMD_BRIDGE_E */
            cmd = CPSS_PACKET_CMD_BRIDGE_E;

            st = cpssDxChBrgGenIngressPortUnknownUcFilterDaCommandSet(dev, port, cmd);
            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, cmd);

            /* call with cmd = CPSS_PACKET_CMD_NONE_E */
            cmd = CPSS_PACKET_CMD_NONE_E;

            st = cpssDxChBrgGenIngressPortUnknownUcFilterDaCommandSet(dev, port, cmd);
            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, cmd);

            /* restore value */
            cmd = CPSS_PACKET_CMD_FORWARD_E;

            /*
                1.1.4. Call with cmd [wrong enum values].
                Expected: GT_BAD_PARAM.
            */
            UTF_ENUMS_CHECK_MAC(cpssDxChBrgGenIngressPortUnknownUcFilterDaCommandSet
                                (dev, port, cmd),
                                cmd);
        }

        cmd = CPSS_PACKET_CMD_FORWARD_E;

        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChBrgGenIngressPortUnknownUcFilterDaCommandSet(dev, port, cmd);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChBrgGenIngressPortUnknownUcFilterDaCommandSet(dev, port, cmd);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgGenIngressPortUnknownUcFilterDaCommandSet(dev, port, cmd);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    cmd = CPSS_PACKET_CMD_FORWARD_E;
    port = 0;

    /* 2. For not active devices check that function returns non GT_OK.*/

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgGenIngressPortUnknownUcFilterDaCommandSet(dev, port, cmd);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgGenIngressPortUnknownUcFilterDaCommandSet(dev, port, cmd);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgGenIngressPortUnknownUcFilterDaCommandGet
(
    IN  GT_U8                   devNum,
    IN  GT_PORT_NUM             portNum,
    OUT CPSS_PACKET_CMD_ENT     *cmdPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgGenIngressPortUnknownUcFilterDaCommandGet)
{
/*
    ITERATE_DEVICES_VIRT_PORTS (SIP5)
    1.1.1. Call with not NULL cmdPtr.
    Expected: GT_OK.
    1.1.2. Call with cmdPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st   = GT_OK;
    GT_U8       dev  = 0;
    GT_PORT_NUM port = 0;

    CPSS_PACKET_CMD_ENT cmd = CPSS_PACKET_CMD_FORWARD_E;
    GT_U32      notAppFamilyBmp;
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
                1.1.1. Call with not NULL cmdPtr.
                Expected: GT_OK.
            */
            st = cpssDxChBrgGenIngressPortUnknownUcFilterDaCommandGet(dev, port, &cmd);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*
                1.1.2. Call with cmdPtr [NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChBrgGenIngressPortUnknownUcFilterDaCommandGet(dev, port, NULL);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, cmdPtr = NULL", dev);
        }

        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChBrgGenIngressPortUnknownUcFilterDaCommandGet(dev, port, &cmd);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChBrgGenIngressPortUnknownUcFilterDaCommandGet(dev, port, &cmd);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                     */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgGenIngressPortUnknownUcFilterDaCommandGet(dev, port, &cmd);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    port = 0;

    /* 2. For not active devices check that function returns non GT_OK.*/

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgGenIngressPortUnknownUcFilterDaCommandGet(dev, port, &cmd);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgGenIngressPortUnknownUcFilterDaCommandGet(dev, port, &cmd);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgGenIngressPortUnregisteredMcFilterDaCommandSet
(
    IN GT_U8                devNum,
    IN GT_PORT_NUM          portNum,
    IN CPSS_PACKET_CMD_ENT  cmd
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgGenIngressPortUnregisteredMcFilterDaCommandSet)
{
/*
    ITERATE_DEVICES_VIRT_PORTS (SIP5)
    1.1.1. Call with cmd [CPSS_PACKET_CMD_FORWARD_E /
                          CPSS_PACKET_CMD_MIRROR_TO_CPU_E /
                          CPSS_PACKET_CMD_TRAP_TO_CPU_E /
                          CPSS_PACKET_CMD_DROP_HARD_E /
                          CPSS_PACKET_CMD_DROP_SOFT_E].
    Expected: GT_OK.
    1.1.2. Call cpssDxChBrgGenIngressPortUnregisteredMcFilterDaCommandGet with non NULL cmdPtr.
    Expected: GT_OK and the same cmd.
    1.1.3. Call with not supported cmd [CPSS_PACKET_CMD_ROUTE_E /
                                        CPSS_PACKET_CMD_ROUTE_AND_MIRROR_E /
                                        CPSS_PACKET_CMD_BRIDGE_AND_MIRROR_E /
                                        CPSS_PACKET_CMD_BRIDGE_E /
                                        CPSS_PACKET_CMD_NONE_E],
    Expected: NON GT_OK.
    1.1.4. Call with cmd [wrong enum values].
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st   = GT_OK;
    GT_U8       dev  = 0;
    GT_PORT_NUM port = 0;

    CPSS_PACKET_CMD_ENT  cmd    = CPSS_PACKET_CMD_FORWARD_E;
    CPSS_PACKET_CMD_ENT  cmdGet = CPSS_PACKET_CMD_FORWARD_E;
    GT_U32      notAppFamilyBmp;
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
                1.1.1. Call with cmd [CPSS_PACKET_CMD_FORWARD_E /
                                      CPSS_PACKET_CMD_MIRROR_TO_CPU_E /
                                      CPSS_PACKET_CMD_TRAP_TO_CPU_E /
                                      CPSS_PACKET_CMD_DROP_HARD_E /
                                      CPSS_PACKET_CMD_DROP_SOFT_E].
                Expected: GT_OK.
            */
            /* call with cmd = CPSS_PACKET_CMD_FORWARD_E */
            cmd = CPSS_PACKET_CMD_FORWARD_E;

            st = cpssDxChBrgGenIngressPortUnregisteredMcFilterDaCommandSet(dev, port, cmd);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, cmd);

            /*
                1.1.2. Call cpssDxChBrgGenIngressPortUnregisteredMcFilterDaCommandGet with non NULL cmdPtr.
                Expected: GT_OK and the same cmd.
            */
            st = cpssDxChBrgGenIngressPortUnregisteredMcFilterDaCommandGet(dev, port, &cmdGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                       "cpssDxChBrgGenIngressPortUnregisteredMcFilterDaCommandGet: %d, %d", dev, port);

            /* verifying values */
            UTF_VERIFY_EQUAL2_STRING_MAC(cmd, cmdGet,
                       "get another cmd than was set: %d, %d", dev, port);

            /* call with cmd = CPSS_PACKET_CMD_MIRROR_TO_CPU_E */
            cmd = CPSS_PACKET_CMD_MIRROR_TO_CPU_E;

            st = cpssDxChBrgGenIngressPortUnregisteredMcFilterDaCommandSet(dev, port, cmd);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, cmd);

            /*
                1.1.2. Call cpssDxChBrgGenIngressPortUnregisteredMcFilterDaCommandGet with non NULL cmdPtr.
                Expected: GT_OK and the same cmd.
            */
            st = cpssDxChBrgGenIngressPortUnregisteredMcFilterDaCommandGet(dev, port, &cmdGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                       "cpssDxChBrgGenIngressPortUnregisteredMcFilterDaCommandGet: %d, %d", dev, port);

            /* verifying values */
            UTF_VERIFY_EQUAL2_STRING_MAC(cmd, cmdGet,
                       "get another cmd than was set: %d, %d", dev, port);

            /* call with cmd = CPSS_PACKET_CMD_TRAP_TO_CPU_E */
            cmd = CPSS_PACKET_CMD_TRAP_TO_CPU_E;

            st = cpssDxChBrgGenIngressPortUnregisteredMcFilterDaCommandSet(dev, port, cmd);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, cmd);

            /*
                1.1.2. Call cpssDxChBrgGenIngressPortUnregisteredMcFilterDaCommandGet with non NULL cmdPtr.
                Expected: GT_OK and the same cmd.
            */
            st = cpssDxChBrgGenIngressPortUnregisteredMcFilterDaCommandGet(dev, port, &cmdGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                       "cpssDxChBrgGenIngressPortUnregisteredMcFilterDaCommandGet: %d, %d", dev, port);

            /* verifying values */
            UTF_VERIFY_EQUAL2_STRING_MAC(cmd, cmdGet,
                       "get another cmd than was set: %d, %d", dev, port);

            /* call with cmd = CPSS_PACKET_CMD_DROP_HARD_E */
            cmd = CPSS_PACKET_CMD_DROP_HARD_E;

            st = cpssDxChBrgGenIngressPortUnregisteredMcFilterDaCommandSet(dev, port, cmd);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, cmd);

            /*
                1.1.2. Call cpssDxChBrgGenIngressPortUnregisteredMcFilterDaCommandGet with non NULL cmdPtr.
                Expected: GT_OK and the same cmd.
            */
            st = cpssDxChBrgGenIngressPortUnregisteredMcFilterDaCommandGet(dev, port, &cmdGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                       "cpssDxChBrgGenIngressPortUnregisteredMcFilterDaCommandGet: %d, %d", dev, port);

            /* verifying values */
            UTF_VERIFY_EQUAL2_STRING_MAC(cmd, cmdGet,
                       "get another cmd than was set: %d, %d", dev, port);

            /* call with cmd = CPSS_PACKET_CMD_DROP_SOFT_E */
            cmd = CPSS_PACKET_CMD_DROP_SOFT_E;

            st = cpssDxChBrgGenIngressPortUnregisteredMcFilterDaCommandSet(dev, port, cmd);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, cmd);

            /*
                1.1.2. Call cpssDxChBrgGenIngressPortUnregisteredMcFilterDaCommandGet with non NULL cmdPtr.
                Expected: GT_OK and the same cmd.
            */
            st = cpssDxChBrgGenIngressPortUnregisteredMcFilterDaCommandGet(dev, port, &cmdGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                       "cpssDxChBrgGenIngressPortUnregisteredMcFilterDaCommandGet: %d, %d", dev, port);

            /* verifying values */
            UTF_VERIFY_EQUAL2_STRING_MAC(cmd, cmdGet,
                       "get another cmd than was set: %d, %d", dev, port);

            /*
                1.1.3. Call with not supported cmd [CPSS_PACKET_CMD_ROUTE_E /
                                                    CPSS_PACKET_CMD_ROUTE_AND_MIRROR_E /
                                                    CPSS_PACKET_CMD_BRIDGE_AND_MIRROR_E /
                                                    CPSS_PACKET_CMD_BRIDGE_E /
                                                    CPSS_PACKET_CMD_NONE_E],
                Expected: NON GT_OK.
            */
            /* call with cmd = CPSS_PACKET_CMD_ROUTE_E */
            cmd = CPSS_PACKET_CMD_ROUTE_E;

            st = cpssDxChBrgGenIngressPortUnregisteredMcFilterDaCommandSet(dev, port, cmd);
            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, cmd);

            /* call with cmd = CPSS_PACKET_CMD_ROUTE_AND_MIRROR_E */
            cmd = CPSS_PACKET_CMD_ROUTE_AND_MIRROR_E;

            st = cpssDxChBrgGenIngressPortUnregisteredMcFilterDaCommandSet(dev, port, cmd);
            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, cmd);

            /* call with cmd = CPSS_PACKET_CMD_BRIDGE_AND_MIRROR_E */
            cmd = CPSS_PACKET_CMD_BRIDGE_AND_MIRROR_E;

            st = cpssDxChBrgGenIngressPortUnregisteredMcFilterDaCommandSet(dev, port, cmd);
            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, cmd);

            /* call with cmd = CPSS_PACKET_CMD_BRIDGE_E */
            cmd = CPSS_PACKET_CMD_BRIDGE_E;

            st = cpssDxChBrgGenIngressPortUnregisteredMcFilterDaCommandSet(dev, port, cmd);
            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, cmd);

            /* call with cmd = CPSS_PACKET_CMD_NONE_E */
            cmd = CPSS_PACKET_CMD_NONE_E;

            st = cpssDxChBrgGenIngressPortUnregisteredMcFilterDaCommandSet(dev, port, cmd);
            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, cmd);

            /* restore value */
            cmd = CPSS_PACKET_CMD_FORWARD_E;

            /*
                1.1.4. Call with cmd [wrong enum values].
                Expected: GT_BAD_PARAM.
            */
            UTF_ENUMS_CHECK_MAC(cpssDxChBrgGenIngressPortUnregisteredMcFilterDaCommandSet
                                (dev, port, cmd),
                                cmd);
        }

        cmd = CPSS_PACKET_CMD_FORWARD_E;

        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChBrgGenIngressPortUnregisteredMcFilterDaCommandSet(dev, port, cmd);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChBrgGenIngressPortUnregisteredMcFilterDaCommandSet(dev, port, cmd);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgGenIngressPortUnregisteredMcFilterDaCommandSet(dev, port, cmd);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    cmd = CPSS_PACKET_CMD_FORWARD_E;
    port = 0;

    /* 2. For not active devices check that function returns non GT_OK.*/

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgGenIngressPortUnregisteredMcFilterDaCommandSet(dev, port, cmd);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgGenIngressPortUnregisteredMcFilterDaCommandSet(dev, port, cmd);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgGenIngressPortUnregisteredMcFilterDaCommandGet
(
    IN  GT_U8                   devNum,
    IN  GT_PORT_NUM             portNum,
    OUT CPSS_PACKET_CMD_ENT     *cmdPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgGenIngressPortUnregisteredMcFilterDaCommandGet)
{
/*
    ITERATE_DEVICES_VIRT_PORTS (SIP5)
    1.1.1. Call with not NULL cmdPtr.
    Expected: GT_OK.
    1.1.2. Call with cmdPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st   = GT_OK;
    GT_U8       dev  = 0;
    GT_PORT_NUM port = 0;

    CPSS_PACKET_CMD_ENT cmd = CPSS_PACKET_CMD_FORWARD_E;
    GT_U32      notAppFamilyBmp;
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
                1.1.1. Call with not NULL cmdPtr.
                Expected: GT_OK.
            */
            st = cpssDxChBrgGenIngressPortUnregisteredMcFilterDaCommandGet(dev, port, &cmd);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*
                1.1.2. Call with cmdPtr [NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChBrgGenIngressPortUnregisteredMcFilterDaCommandGet(dev, port, NULL);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, cmdPtr = NULL", dev);
        }

        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChBrgGenIngressPortUnregisteredMcFilterDaCommandGet(dev, port, &cmd);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChBrgGenIngressPortUnregisteredMcFilterDaCommandGet(dev, port, &cmd);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                     */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgGenIngressPortUnregisteredMcFilterDaCommandGet(dev, port, &cmd);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    port = 0;

    /* 2. For not active devices check that function returns non GT_OK.*/

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgGenIngressPortUnregisteredMcFilterDaCommandGet(dev, port, &cmd);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgGenIngressPortUnregisteredMcFilterDaCommandGet(dev, port, &cmd);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgGenPortArpMacSaMismatchDropEnable
(
    IN GT_U8        devNum,
    IN GT_PORT_NUM  portNum,
    IN GT_BOOL      enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgGenPortArpMacSaMismatchDropEnable)
{
/*
    ITERATE_DEVICES_VIRT_PORTS (SIP5)
    1.1.1. Call with enable [GT_TRUE / GT_FALSE].
    Expected: GT_OK.
    1.1.2. Call cpssDxChBrgGenPortArpMacSaMismatchDropEnableGet with non NULL enablePtr.
    Expected: GT_OK and the same enable.
*/
    GT_STATUS   st   = GT_OK;
    GT_U8       dev  = 0;
    GT_PORT_NUM port = 0;

    GT_BOOL     enable    = GT_FALSE;
    GT_BOOL     enableGet = GT_FALSE;
    GT_U32      notAppFamilyBmp;
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
                1.1.1. Call with enable [GT_TRUE / GT_FALSE].
                Expected: GT_OK.
            */
            /* call with enable = GT_TRUE */
            enable = GT_TRUE;

            st = cpssDxChBrgGenPortArpMacSaMismatchDropEnable(dev, port, enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);

            /*
                1.1.2. Call cpssDxChBrgGenPortArpMacSaMismatchDropEnableGet with non NULL enablePtr.
                Expected: GT_OK and the same enable.
            */
            st = cpssDxChBrgGenPortArpMacSaMismatchDropEnableGet(dev, port, &enableGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                       "cpssDxChBrgGenPortArpMacSaMismatchDropEnableGet: %d, %d", dev, port);

            /* verifying values */
            UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                       "get another enable than was set: %d, %d", dev, port);

            /* call with enable = GT_FALSE */
            enable = GT_FALSE;

            st = cpssDxChBrgGenPortArpMacSaMismatchDropEnable(dev, port, enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);

            /*
                1.1.2. Call cpssDxChBrgGenPortArpMacSaMismatchDropEnableGet with non NULL enablePtr.
                Expected: GT_OK and the same enable.
            */
            st = cpssDxChBrgGenPortArpMacSaMismatchDropEnableGet(dev, port, &enableGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                       "cpssDxChBrgGenPortArpMacSaMismatchDropEnableGet: %d, %d", dev, port);

            /* verifying values */
            UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                       "get another enable than was set: %d, %d", dev, port);
        }

        enable = GT_FALSE;

        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChBrgGenPortArpMacSaMismatchDropEnable(dev, port, enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChBrgGenPortArpMacSaMismatchDropEnable(dev, port, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                     */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgGenPortArpMacSaMismatchDropEnable(dev, port, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    enable = GT_FALSE;
    port = 0;

    /* 2. For not active devices check that function returns non GT_OK.*/

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgGenPortArpMacSaMismatchDropEnable(dev, port, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgGenPortArpMacSaMismatchDropEnable(dev, port, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgGenPortArpMacSaMismatchDropEnableGet
(
    IN  GT_U8       devNum,
    IN  GT_PORT_NUM portNum,
    OUT GT_BOOL     *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgGenPortArpMacSaMismatchDropEnableGet)
{
/*
    ITERATE_DEVICES_VIRT_PORTS (SIP5)
    1.1.1. Call with not NULL enablePtr.
    Expected: GT_OK.
    1.1.2. Call with enablePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st   = GT_OK;
    GT_U8       dev  = 0;
    GT_PORT_NUM port = 0;

    GT_BOOL     enable = 0;
    GT_U32      notAppFamilyBmp;
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
            st = cpssDxChBrgGenPortArpMacSaMismatchDropEnableGet(dev, port, &enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*
                1.1.2. Call with enablePtr [NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChBrgGenPortArpMacSaMismatchDropEnableGet(dev, port, NULL);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, enablePtr = NULL", dev);
        }

        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChBrgGenPortArpMacSaMismatchDropEnableGet(dev, port, &enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChBrgGenPortArpMacSaMismatchDropEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                     */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgGenPortArpMacSaMismatchDropEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    port = 0;

    /* 2. For not active devices check that function returns non GT_OK.*/

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgGenPortArpMacSaMismatchDropEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgGenPortArpMacSaMismatchDropEnableGet(dev, port, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgGenPortBcUdpTrapMirrorEnableSet
(
    IN GT_U8            devNum,
    IN GT_PORT_NUM      portNum,
    IN GT_BOOL          enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgGenPortBcUdpTrapMirrorEnableSet)
{
/*
    ITERATE_DEVICES_VIRT_PORTS (SIP5)
    1.1.1. Call with enable [GT_TRUE / GT_FALSE].
    Expected: GT_OK.
    1.1.2. Call cpssDxChBrgGenPortBcUdpTrapMirrorEnableGet with non NULL enablePtr.
    Expected: GT_OK and the same enable.
*/
    GT_STATUS   st   = GT_OK;
    GT_U8       dev  = 0;
    GT_PORT_NUM port = 0;

    GT_BOOL     enable    = GT_FALSE;
    GT_BOOL     enableGet = GT_FALSE;
    GT_U32      notAppFamilyBmp;
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
                1.1.1. Call with enable [GT_TRUE / GT_FALSE].
                Expected: GT_OK.
            */
            /* call with enable = GT_TRUE */
            enable = GT_TRUE;

            st = cpssDxChBrgGenPortBcUdpTrapMirrorEnableSet(dev, port, enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);

            /*
                1.1.2. Call cpssDxChBrgGenPortBcUdpTrapMirrorEnableGet with non NULL enablePtr.
                Expected: GT_OK and the same enable.
            */
            st = cpssDxChBrgGenPortBcUdpTrapMirrorEnableGet(dev, port, &enableGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                       "cpssDxChBrgGenPortBcUdpTrapMirrorEnableGet: %d, %d", dev, port);

            /* verifying values */
            UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                       "get another enable than was set: %d, %d", dev, port);

            /* call with enable = GT_FALSE */
            enable = GT_FALSE;

            st = cpssDxChBrgGenPortBcUdpTrapMirrorEnableSet(dev, port, enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);

            /*
                1.1.2. Call cpssDxChBrgGenPortBcUdpTrapMirrorEnableGet with non NULL enablePtr.
                Expected: GT_OK and the same enable.
            */
            st = cpssDxChBrgGenPortBcUdpTrapMirrorEnableGet(dev, port, &enableGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                       "cpssDxChBrgGenPortBcUdpTrapMirrorEnableGet: %d, %d", dev, port);

            /* verifying values */
            UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                       "get another enable than was set: %d, %d", dev, port);
        }

        enable = GT_FALSE;

        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChBrgGenPortBcUdpTrapMirrorEnableSet(dev, port, enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChBrgGenPortBcUdpTrapMirrorEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                     */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgGenPortBcUdpTrapMirrorEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    enable = GT_FALSE;
    port = 0;

    /* 2. For not active devices check that function returns non GT_OK.*/

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgGenPortBcUdpTrapMirrorEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgGenPortBcUdpTrapMirrorEnableSet(dev, port, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgGenPortBcUdpTrapMirrorEnableGet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    OUT GT_BOOL         *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgGenPortBcUdpTrapMirrorEnableGet)
{
/*
    ITERATE_DEVICES_VIRT_PORTS (SIP5)
    1.1.1. Call with not NULL enablePtr.
    Expected: GT_OK.
    1.1.2. Call with enablePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st   = GT_OK;
    GT_U8       dev  = 0;
    GT_PORT_NUM port = 0;

    GT_BOOL     enable = 0;
    GT_U32      notAppFamilyBmp;
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
            st = cpssDxChBrgGenPortBcUdpTrapMirrorEnableGet(dev, port, &enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*
                1.1.2. Call with enablePtr [NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChBrgGenPortBcUdpTrapMirrorEnableGet(dev, port, NULL);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, enablePtr = NULL", dev);
        }

        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChBrgGenPortBcUdpTrapMirrorEnableGet(dev, port, &enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChBrgGenPortBcUdpTrapMirrorEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                     */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgGenPortBcUdpTrapMirrorEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    port = 0;

    /* 2. For not active devices check that function returns non GT_OK.*/

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgGenPortBcUdpTrapMirrorEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgGenPortBcUdpTrapMirrorEnableGet(dev, port, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgGenPortIpControlTrapEnableSet
(
    IN GT_U8                                devNum,
    IN GT_PORT_NUM                          portNum,
    IN CPSS_DXCH_BRG_IP_CTRL_TYPE_ENT       ipCntrlType
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgGenPortIpControlTrapEnableSet)
{
/*
    ITERATE_DEVICES_VIRT_PORTS (SIP5)
    1.1.1. Call with ipCntrlType [CPSS_DXCH_BRG_IP_CTRL_NONE_E /
                                  CPSS_DXCH_BRG_IP_CTRL_IPV4_E /
                                  CPSS_DXCH_BRG_IP_CTRL_IPV6_E /
                                  CPSS_DXCH_BRG_IP_CTRL_IPV4_IPV6_E].
    Expected: GT_OK.
    1.1.2. Call cpssDxChBrgGenPortIpControlTrapEnableGet with non NULL ipCntrlTypePtr.
    Expected: GT_OK and the same ipCntrlType.
    1.1.3. Call with ipCntrlType [wrong enum values].
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st   = GT_OK;
    GT_U8       dev  = 0;
    GT_PORT_NUM port = 0;

    CPSS_DXCH_BRG_IP_CTRL_TYPE_ENT  ipCntrlType    = CPSS_DXCH_BRG_IP_CTRL_NONE_E;
    CPSS_DXCH_BRG_IP_CTRL_TYPE_ENT  ipCntrlTypeGet = CPSS_DXCH_BRG_IP_CTRL_NONE_E;
    GT_U32      notAppFamilyBmp;
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
                1.1.1. Call with ipCntrlType [CPSS_DXCH_BRG_IP_CTRL_NONE_E /
                                              CPSS_DXCH_BRG_IP_CTRL_IPV4_E /
                                              CPSS_DXCH_BRG_IP_CTRL_IPV6_E /
                                              CPSS_DXCH_BRG_IP_CTRL_IPV4_IPV6_E].
                Expected: GT_OK.
            */
            /* call with ipCntrlType = CPSS_DXCH_BRG_IP_CTRL_NONE_E */
            ipCntrlType = CPSS_DXCH_BRG_IP_CTRL_NONE_E;

            st = cpssDxChBrgGenPortIpControlTrapEnableSet(dev, port, ipCntrlType);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, ipCntrlType);

            /*
                1.1.2. Call cpssDxChBrgGenPortIpControlTrapEnableGet with non NULL ipCntrlTypePtr.
                Expected: GT_OK and the same ipCntrlType.
            */
            st = cpssDxChBrgGenPortIpControlTrapEnableGet(dev, port, &ipCntrlTypeGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                       "cpssDxChBrgGenPortIpControlTrapEnableGet: %d, %d", dev, port);

            /* verifying values */
            UTF_VERIFY_EQUAL2_STRING_MAC(ipCntrlType, ipCntrlTypeGet,
                       "get another ipCntrlType than was set: %d, %d", dev, port);

            /* call with ipCntrlType = CPSS_DXCH_BRG_IP_CTRL_IPV4_E */
            ipCntrlType = CPSS_DXCH_BRG_IP_CTRL_IPV4_E;

            st = cpssDxChBrgGenPortIpControlTrapEnableSet(dev, port, ipCntrlType);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, ipCntrlType);

            /*
                1.1.2. Call cpssDxChBrgGenPortIpControlTrapEnableGet with non NULL ipCntrlTypePtr.
                Expected: GT_OK and the same ipCntrlType.
            */
            st = cpssDxChBrgGenPortIpControlTrapEnableGet(dev, port, &ipCntrlTypeGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                       "cpssDxChBrgGenPortIpControlTrapEnableGet: %d, %d", dev, port);

            /* verifying values */
            UTF_VERIFY_EQUAL2_STRING_MAC(ipCntrlType, ipCntrlTypeGet,
                       "get another ipCntrlType than was set: %d, %d", dev, port);

            /* call with ipCntrlType = CPSS_DXCH_BRG_IP_CTRL_IPV6_E */
            ipCntrlType = CPSS_DXCH_BRG_IP_CTRL_IPV6_E;

            st = cpssDxChBrgGenPortIpControlTrapEnableSet(dev, port, ipCntrlType);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, ipCntrlType);

            /*
                1.1.2. Call cpssDxChBrgGenPortIpControlTrapEnableGet with non NULL ipCntrlTypePtr.
                Expected: GT_OK and the same ipCntrlType.
            */
            st = cpssDxChBrgGenPortIpControlTrapEnableGet(dev, port, &ipCntrlTypeGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                       "cpssDxChBrgGenPortIpControlTrapEnableGet: %d, %d", dev, port);

            /* verifying values */
            UTF_VERIFY_EQUAL2_STRING_MAC(ipCntrlType, ipCntrlTypeGet,
                       "get another ipCntrlType than was set: %d, %d", dev, port);

            /* call with ipCntrlType = CPSS_DXCH_BRG_IP_CTRL_IPV4_IPV6_E */
            ipCntrlType = CPSS_DXCH_BRG_IP_CTRL_IPV4_IPV6_E;

            st = cpssDxChBrgGenPortIpControlTrapEnableSet(dev, port, ipCntrlType);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, ipCntrlType);

            /*
                1.1.2. Call cpssDxChBrgGenPortIpControlTrapEnableGet with non NULL ipCntrlTypePtr.
                Expected: GT_OK and the same ipCntrlType.
            */
            st = cpssDxChBrgGenPortIpControlTrapEnableGet(dev, port, &ipCntrlTypeGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                       "cpssDxChBrgGenPortIpControlTrapEnableGet: %d, %d", dev, port);

            /* verifying values */
            UTF_VERIFY_EQUAL2_STRING_MAC(ipCntrlType, ipCntrlTypeGet,
                       "get another ipCntrlType than was set: %d, %d", dev, port);

            /*
                1.1.3. Call with ipCntrlType [wrong enum values].
                Expected: GT_BAD_PARAM.
            */
            UTF_ENUMS_CHECK_MAC(cpssDxChBrgGenPortIpControlTrapEnableSet
                                (dev, port, ipCntrlType),
                                ipCntrlType);
        }

        ipCntrlType = CPSS_DXCH_BRG_IP_CTRL_NONE_E;

        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChBrgGenPortIpControlTrapEnableSet(dev, port, ipCntrlType);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChBrgGenPortIpControlTrapEnableSet(dev, port, ipCntrlType);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                     */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgGenPortIpControlTrapEnableSet(dev, port, ipCntrlType);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    ipCntrlType = CPSS_DXCH_BRG_IP_CTRL_NONE_E;
    port = 0;

    /* 2. For not active devices check that function returns non GT_OK.*/

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgGenPortIpControlTrapEnableSet(dev, port, ipCntrlType);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgGenPortIpControlTrapEnableSet(dev, port, ipCntrlType);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgGenPortIpControlTrapEnableGet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_NUM                         portNum,
    OUT CPSS_DXCH_BRG_IP_CTRL_TYPE_ENT      *ipCntrlTypePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgGenPortIpControlTrapEnableGet)
{
/*
    ITERATE_DEVICES_VIRT_PORTS (SIP5)
    1.1.1. Call with not NULL ipCntrlTypePtr.
    Expected: GT_OK.
    1.1.2. Call with ipCntrlTypePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st   = GT_OK;
    GT_U8       dev  = 0;
    GT_PORT_NUM port = 0;

    CPSS_DXCH_BRG_IP_CTRL_TYPE_ENT  ipCntrlType = CPSS_DXCH_BRG_IP_CTRL_NONE_E;
    GT_U32      notAppFamilyBmp;
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
                1.1.1. Call with not NULL ipCntrlTypePtr.
                Expected: GT_OK.
            */
            st = cpssDxChBrgGenPortIpControlTrapEnableGet(dev, port, &ipCntrlType);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*
                1.1.2. Call with ipCntrlTypePtr [NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChBrgGenPortIpControlTrapEnableGet(dev, port, NULL);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, ipCntrlTypePtr = NULL", dev);
        }

        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChBrgGenPortIpControlTrapEnableGet(dev, port, &ipCntrlType);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChBrgGenPortIpControlTrapEnableGet(dev, port, &ipCntrlType);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                     */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgGenPortIpControlTrapEnableGet(dev, port, &ipCntrlType);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    port = 0;

    /* 2. For not active devices check that function returns non GT_OK.*/

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgGenPortIpControlTrapEnableGet(dev, port, &ipCntrlType);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgGenPortIpControlTrapEnableGet(dev, port, &ipCntrlType);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChBrgGenPortOversizeUntaggedPacketsFilterCmdSet)
{
/*
    ITERATE_DEVICES
    1.1. Call with the relevant values
         command[CPSS_PACKET_CMD_FORWARD_E/
                 CPSS_PACKET_CMD_MIRROR_TO_CPU_E/
                 CPSS_PACKET_CMD_TRAP_TO_CPU_E/
                 CPSS_PACKET_CMD_DROP_HARD_E/
                 CPSS_PACKET_CMD_DROP_SOFT_E].
    Expected: GT_OK.
    1.2. Call cpssDxChBrgGenPortOversizeUntaggedPacketsFilterCmdGet.
    Expected: GT_OK and the same command.
    1.3. Call with the not relevant values
         command[CPSS_PACKET_CMD_ROUTE_E/
                 CPSS_PACKET_CMD_ROUTE_AND_MIRROR_E/
                 CPSS_PACKET_CMD_NONE_E].
    Expected: NOT GT_OK.
    1.4. Call with wrong enum values command.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st   = GT_OK;
    GT_U8       dev  = 0 ;

    CPSS_PACKET_CMD_ENT      command = CPSS_PACKET_CMD_FORWARD_E;
    CPSS_PACKET_CMD_ENT      commandGet = CPSS_PACKET_CMD_FORWARD_E;


    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*  1.1.  */
        command = CPSS_PACKET_CMD_FORWARD_E;

        st = cpssDxChBrgGenPortOversizeUntaggedPacketsFilterCmdSet(dev, command);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.2.  */
        st = cpssDxChBrgGenPortOversizeUntaggedPacketsFilterCmdGet(dev, &commandGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(command, commandGet,
                   "get another command than was set: %d", dev);

        /*  1.1.  */
        command = CPSS_PACKET_CMD_MIRROR_TO_CPU_E;

        st = cpssDxChBrgGenPortOversizeUntaggedPacketsFilterCmdSet(dev, command);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.2.  */
        st = cpssDxChBrgGenPortOversizeUntaggedPacketsFilterCmdGet(dev, &commandGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(command, commandGet,
                   "get another command than was set: %d", dev);

        /*  1.1.  */
        command = CPSS_PACKET_CMD_TRAP_TO_CPU_E;

        st = cpssDxChBrgGenPortOversizeUntaggedPacketsFilterCmdSet(dev, command);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.2.  */
        st = cpssDxChBrgGenPortOversizeUntaggedPacketsFilterCmdGet(dev, &commandGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(command, commandGet,
                   "get another command than was set: %d", dev);

        /*  1.1.  */
        command = CPSS_PACKET_CMD_DROP_HARD_E;

        st = cpssDxChBrgGenPortOversizeUntaggedPacketsFilterCmdSet(dev, command);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.2.  */
        st = cpssDxChBrgGenPortOversizeUntaggedPacketsFilterCmdGet(dev, &commandGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(command, commandGet,
                   "get another command than was set: %d", dev);

        /*  1.1.  */
        command = CPSS_PACKET_CMD_DROP_SOFT_E;

        st = cpssDxChBrgGenPortOversizeUntaggedPacketsFilterCmdSet(dev, command);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.2.  */
        st = cpssDxChBrgGenPortOversizeUntaggedPacketsFilterCmdGet(dev, &commandGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(command, commandGet,
                   "get another command than was set: %d", dev);

        /*  1.3.  */
        command = CPSS_PACKET_CMD_ROUTE_E;

        st = cpssDxChBrgGenPortOversizeUntaggedPacketsFilterCmdSet(dev, command);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.3.  */
        command = CPSS_PACKET_CMD_ROUTE_AND_MIRROR_E;

        st = cpssDxChBrgGenPortOversizeUntaggedPacketsFilterCmdSet(dev, command);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.3.  */
        command = CPSS_PACKET_CMD_NONE_E;

        st = cpssDxChBrgGenPortOversizeUntaggedPacketsFilterCmdSet(dev, command);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.4.  */
        UTF_ENUMS_CHECK_MAC(cpssDxChBrgGenPortOversizeUntaggedPacketsFilterCmdSet(dev, command),
                            command);
    }

    command = CPSS_PACKET_CMD_FORWARD_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgGenPortOversizeUntaggedPacketsFilterCmdSet(dev, command);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgGenPortOversizeUntaggedPacketsFilterCmdSet(dev, command);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChBrgGenPortOversizeUntaggedPacketsFilterCmdGet)
{
/*
    ITERATE_DEVICES
    1.1. Call with not NULL command.
    Expected: GT_OK.
    1.2. Call with NULL command.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st   = GT_OK;
    GT_U8       dev  = 0 ;

    CPSS_PACKET_CMD_ENT      command = CPSS_PACKET_CMD_FORWARD_E;

    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*  1.1.  */
        st = cpssDxChBrgGenPortOversizeUntaggedPacketsFilterCmdGet(dev, &command);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.2.  */
        st = cpssDxChBrgGenPortOversizeUntaggedPacketsFilterCmdGet(dev, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgGenPortOversizeUntaggedPacketsFilterCmdGet(dev, &command);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgGenPortOversizeUntaggedPacketsFilterCmdGet(dev, &command);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*******************/
/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgGenBypassModeSet
(
    IN GT_U8                                devNum,
    IN CPSS_DXCH_BRG_GEN_BYPASS_MODE_ENT    mode
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgGenBypassModeSet)
{
/*
    ITERATE_DEVICES (Bobcat2, Caelum, Bobcat3)
    1.1. Call with mode [CPSS_DXCH_BRG_GEN_BYPASS_MODE_ALL_EXCEPT_SA_LEARNING_E /
                         CPSS_DXCH_BRG_GEN_BYPASS_MODE_ONLY_FORWARDING_DECISION_E].
    Expected: GT_OK.
    1.2. Call cpssDxChBrgGenBypassModeGet with not NULL modePtr.
    Expected: GT_OK and the same modePtr as was set.
    1.3. Call with mode.
    Expected: GT_BAD_PARAM.
Call with wrong enum values mode
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;
    GT_U32      notAppFamilyBmp;

    CPSS_DXCH_BRG_GEN_BYPASS_MODE_ENT  mode    = CPSS_DXCH_BRG_GEN_BYPASS_MODE_ALL_EXCEPT_SA_LEARNING_E;
    CPSS_DXCH_BRG_GEN_BYPASS_MODE_ENT  modeGet = CPSS_DXCH_BRG_GEN_BYPASS_MODE_ALL_EXCEPT_SA_LEARNING_E;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with mode [CPSS_DXCH_BRG_GEN_BYPASS_MODE_ALL_EXCEPT_SA_LEARNING_E /
                                 CPSS_DXCH_BRG_GEN_BYPASS_MODE_ONLY_FORWARDING_DECISION_E]
            Expected: GT_OK.
        */
        /* iterate with mode = CPSS_DXCH_BRG_GEN_BYPASS_MODE_ALL_EXCEPT_SA_LEARNING_E */
        mode = CPSS_DXCH_BRG_GEN_BYPASS_MODE_ALL_EXCEPT_SA_LEARNING_E;

        st = cpssDxChBrgGenBypassModeSet(dev, mode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, mode);

        /*
            1.2. Call cpssDxChBrgGenBypassModeGet with not NULL modePtr
            Expected: GT_OK and the same modePtr as was set.
        */
        st = cpssDxChBrgGenBypassModeGet(dev, &modeGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChBrgGenBypassModeSet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(mode, modeGet, "get another mode than was set: %d", dev);

        /* iterate with mode = CPSS_DROP_MODE_HARD_E */
        mode = CPSS_DXCH_BRG_GEN_BYPASS_MODE_ONLY_FORWARDING_DECISION_E;

        st = cpssDxChBrgGenBypassModeSet(dev, mode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, mode);

        /*
            1.3. Call cpssDxChBrgGenBypassModeGet with not NULL modePtr
            Expected: GT_OK and the same modePtr as was set.
        */
        st = cpssDxChBrgGenBypassModeGet(dev, &modeGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChBrgGenBypassModeGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(mode, modeGet, "get another mode than was set: %d", dev);

        /*
            1.4. Call with wrong enum values mode.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChBrgGenBypassModeSet
                            (dev, mode),
                            mode);
    }

    mode = CPSS_DXCH_BRG_GEN_BYPASS_MODE_ALL_EXCEPT_SA_LEARNING_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgGenBypassModeSet(dev, mode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgGenBypassModeSet(dev, mode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgGenBypassModeGet
(
    IN  GT_U8                               devNum,
    OUT CPSS_DXCH_BRG_GEN_BYPASS_MODE_ENT   *modePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgGenBypassModeGet)
{
/*
    ITERATE_DEVICES (Bobcat2, Caelum, Bobcat3)
    1.1. Call with not NULL modePtr.
    Expected: GT_OK
    1.2. Call with modePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;
    GT_U32      notAppFamilyBmp;

    CPSS_DXCH_BRG_GEN_BYPASS_MODE_ENT  mode  = CPSS_DXCH_BRG_GEN_BYPASS_MODE_ALL_EXCEPT_SA_LEARNING_E;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with not NULL modePtr.
            Expected: GT_OK
        */
        st = cpssDxChBrgGenBypassModeGet(dev, &mode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with modePtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChBrgGenBypassModeGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, modePtr = NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgGenBypassModeGet(dev, &mode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgGenBypassModeGet(dev, &mode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgGenMtuConfigSet
(
    IN GT_U8                                        devNum,
    IN GT_BOOL                                      mtuCheckEnable,
    IN CPSS_DXCH_BRG_GEN_BYTE_COUNT_MODE_MTU_ENT    byteCountMode,
    IN CPSS_PACKET_CMD_ENT                          exceptionCommand,
    IN CPSS_NET_RX_CPU_CODE_ENT                     exceptionCpuCode
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgGenMtuConfigSet)
{
/*
    ITERATE_DEVICES (Bobcat2, Caelum, Bobcat3)
    1.1. Call with  mtuCheckEnable [GT_TRUE] = feature enabled
                    byteCountMode  [CPSS_DXCH_BRG_GEN_BYTE_COUNT_MODE_MTU_L3_E]
                    exceptionCommand [CPSS_PACKET_CMD_FORWARD_E]
                    exceptionCpuCode [CPSS_NET_CONTROL_E]
    Expected: GT_OK.
    1.2. Call with  mtuCheckEnable [GT_TRUE] = feature enabled
                    byteCountMode  [CPSS_DXCH_BRG_GEN_BYTE_COUNT_MODE_MTU_L3_E]
                    exceptionCommand [CPSS_PACKET_CMD_MIRROR_TO_CPU_E]
                    exceptionCpuCode [CPSS_NET_IP_MTU_EXCEED_E]
    Expected: GT_OK.
    1.3. Call with  mtuCheckEnable [GT_TRUE] = feature enabled
                    byteCountMode  [CPSS_DXCH_BRG_GEN_BYTE_COUNT_MODE_MTU_L2_E]
                    exceptionCommand [CPSS_PACKET_CMD_TRAP_TO_CPU_E]
                    exceptionCpuCode [CPSS_NET_CLASS_KEY_MIRROR_E]
    Expected: GT_OK and the same modePtr as was set.
    1.4. Call with  mtuCheckEnable [GT_TRUE] = feature enabled
                    byteCountMode  [CPSS_DXCH_BRG_GEN_BYTE_COUNT_MODE_MTU_L2_E]
                    exceptionCommand [CPSS_PACKET_CMD_DROP_HARD_E]
                    exceptionCpuCode [CPSS_NET_CONTROL_E]
    Expected: GT_OK.
    1.5. Call with  mtuCheckEnable [GT_TRUE] = feature enabled
                    byteCountMode  [CPSS_DXCH_BRG_GEN_BYTE_COUNT_MODE_MTU_L3_E]
                    exceptionCommand [CPSS_PACKET_CMD_DROP_SOFT_E]
                    exceptionCpuCode [CPSS_NET_CONTROL_E]
    Expected: GT_OK.
    1.6. Call with  mtuCheckEnable [GT_TRUE] = feature enabled
                    byteCountMode  [CPSS_DXCH_BRG_GEN_BYTE_COUNT_MODE_MTU_L3_E]
                    exceptionCommand [CPSS_PACKET_CMD_BRIDGE_AND_MIRROR_E]
                    exceptionCpuCode [CPSS_NET_CONTROL_E]
    Expected: GT_BAD_PARAM due to exceptionCommand.
    1.7. Call with  mtuCheckEnable [GT_TRUE] = feature enabled
                    byteCountMode  [CPSS_DXCH_BRG_GEN_BYTE_COUNT_MODE_MTU_L2_E]
                    exceptionCommand [CPSS_PACKET_CMD_NONE_E]
                    exceptionCpuCode [CPSS_NET_CONTROL_E]
    Expected: GT_BAD_PARAM due to exceptionCommand.
    1.8. Call with  mtuCheckEnable [GT_FALSE] = feature disabled
                    byteCountMode  [CPSS_DXCH_BRG_GEN_BYTE_COUNT_MODE_MTU_L3_E]
                    exceptionCommand [CPSS_PACKET_CMD_FORWARD_E]
                    exceptionCpuCode [CPSS_NET_CONTROL_E]
    Expected: GT_OK.
    1.9. Call with  mtuCheckEnable [GT_FALSE] = feature disabled
                    byteCountMode  [CPSS_DXCH_BRG_GEN_BYTE_COUNT_MODE_MTU_L2_E]
                    exceptionCommand [CPSS_PACKET_CMD_NONE_E]
                    exceptionCpuCode [CPSS_NET_CONTROL_E]
    Expected: GT_BAD_PARAM due to exceptionCommand.

    1.10 Call with wrong enum values mode
*/
    GT_STATUS   st  = GT_OK;
    GT_U32      notAppFamilyBmp;
    GT_U8       dev;
    IN GT_BOOL                                      mtuCheckEnable, mtuCheckEnableGet;
    IN CPSS_DXCH_BRG_GEN_BYTE_COUNT_MODE_MTU_ENT    byteCountMode, byteCountModeGet;
    IN CPSS_PACKET_CMD_ENT                          exceptionCommand, exceptionCommandGet;
    IN CPSS_NET_RX_CPU_CODE_ENT                     exceptionCpuCode, exceptionCpuCodeGet;

    mtuCheckEnableGet = GT_TRUE;
    byteCountModeGet  = CPSS_DXCH_BRG_GEN_BYTE_COUNT_MODE_MTU_L3_E;
    exceptionCommandGet = CPSS_PACKET_CMD_FORWARD_E;
    exceptionCpuCodeGet = CPSS_NET_CONTROL_E;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
        1.1. Call cpssDxChBrgGenMtuConfigSet
                  with  mtuCheckEnable [GT_TRUE] = feature enabled
                        byteCountMode  [CPSS_DXCH_BRG_GEN_BYTE_COUNT_MODE_MTU_L3_E]
                        exceptionCommand [CPSS_PACKET_CMD_FORWARD_E]
                        exceptionCpuCode [CPSS_NET_CONTROL_E]
        Expected: GT_OK.
        */
        mtuCheckEnable = GT_TRUE;
        byteCountMode  = CPSS_DXCH_BRG_GEN_BYTE_COUNT_MODE_MTU_L3_E;
        exceptionCommand = CPSS_PACKET_CMD_FORWARD_E;
        exceptionCpuCode = CPSS_NET_CONTROL_E;
        st = cpssDxChBrgGenMtuConfigSet(dev, mtuCheckEnable,
                                        byteCountMode, exceptionCommand,
                                        exceptionCpuCode);
        UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, dev, mtuCheckEnable, byteCountMode,
                                    exceptionCommand, exceptionCpuCode);

        /*
        1.1.2. Call cpssDxChBrgGenMtuConfigGet with not NULL modePtr
        Expected: GT_OK and the same modePtr as was set.
        */
        st = cpssDxChBrgGenMtuConfigGet(dev, &mtuCheckEnableGet,
                                        &byteCountModeGet, &exceptionCommandGet,
                                        &exceptionCpuCodeGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChBrgGenMtuConfigSet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(mtuCheckEnable, mtuCheckEnableGet,
                                     "get mtuCheckEnable than was set: %d", mtuCheckEnable);
        UTF_VERIFY_EQUAL1_STRING_MAC(byteCountMode, byteCountModeGet,
                                     "get byteCountMode than was set: %d", byteCountMode);
        UTF_VERIFY_EQUAL1_STRING_MAC(exceptionCommand, exceptionCommandGet,
                                     "get exceptionCommand than was set: %d", exceptionCommand);
        UTF_VERIFY_EQUAL1_STRING_MAC(exceptionCpuCode, exceptionCpuCodeGet,
                                     "get exceptionCpuCode than was set: %d", exceptionCpuCode);

        /*
        1.2. Call with  mtuCheckEnable [GT_TRUE] = feature enabled
                byteCountMode  [CPSS_DXCH_BRG_GEN_BYTE_COUNT_MODE_MTU_L3_E]
                exceptionCommand [CPSS_PACKET_CMD_MIRROR_TO_CPU_E]
                exceptionCpuCode [CPSS_NET_IP_MTU_EXCEED_E]
        Expected: GT_OK.
        */
        mtuCheckEnable = GT_TRUE;
        byteCountMode  = CPSS_DXCH_BRG_GEN_BYTE_COUNT_MODE_MTU_L3_E;
        exceptionCommand = CPSS_PACKET_CMD_MIRROR_TO_CPU_E;
        exceptionCpuCode = CPSS_NET_IP_MTU_EXCEED_E;
        st = cpssDxChBrgGenMtuConfigSet(dev, mtuCheckEnable,
                                        byteCountMode, exceptionCommand,
                                        exceptionCpuCode);
        UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, dev, mtuCheckEnable, byteCountMode,
                                    exceptionCommand, exceptionCpuCode);

        /*
        1.2.2. Call cpssDxChBrgGenMtuConfigGet with not NULL modePtr
        Expected: GT_OK and the same modePtr as was set.
        */
        st = cpssDxChBrgGenMtuConfigGet(dev, &mtuCheckEnableGet,
                                        &byteCountModeGet, &exceptionCommandGet,
                                        &exceptionCpuCodeGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChBrgGenMtuConfigSet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(mtuCheckEnable, mtuCheckEnableGet,
                                     "get mtuCheckEnable than was set: %d", mtuCheckEnable);
        UTF_VERIFY_EQUAL1_STRING_MAC(byteCountMode, byteCountModeGet,
                                     "get byteCountMode than was set: %d", byteCountMode);
        UTF_VERIFY_EQUAL1_STRING_MAC(exceptionCommand, exceptionCommandGet,
                                     "get exceptionCommand than was set: %d", exceptionCommand);
        UTF_VERIFY_EQUAL1_STRING_MAC(exceptionCpuCode, exceptionCpuCodeGet,
                                     "get exceptionCpuCode than was set: %d", exceptionCpuCode);

        /*
        1.3. Call with  mtuCheckEnable [GT_TRUE] = feature enabled
                        byteCountMode  [CPSS_DXCH_BRG_GEN_BYTE_COUNT_MODE_MTU_L2_E]
                        exceptionCommand [CPSS_PACKET_CMD_TRAP_TO_CPU_E]
                        exceptionCpuCode [CPSS_NET_CLASS_KEY_MIRROR_E]
        Expected: GT_OK and the same modePtr as was set.
        */
        mtuCheckEnable = GT_TRUE;
        byteCountMode  = CPSS_DXCH_BRG_GEN_BYTE_COUNT_MODE_MTU_L2_E;
        exceptionCommand = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
        exceptionCpuCode = CPSS_NET_CLASS_KEY_MIRROR_E;
        st = cpssDxChBrgGenMtuConfigSet(dev, mtuCheckEnable,
                                        byteCountMode, exceptionCommand,
                                        exceptionCpuCode);
        UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, dev, mtuCheckEnable, byteCountMode,
                                    exceptionCommand, exceptionCpuCode);

        /*
        1.3.2. Call cpssDxChBrgGenMtuConfigGet with not NULL modePtr
        Expected: GT_OK and the same modePtr as was set.
        */
        st = cpssDxChBrgGenMtuConfigGet(dev, &mtuCheckEnableGet,
                                        &byteCountModeGet, &exceptionCommandGet,
                                        &exceptionCpuCodeGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChBrgGenMtuConfigSet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(mtuCheckEnable, mtuCheckEnableGet,
                                     "get mtuCheckEnable than was set: %d", mtuCheckEnable);
        UTF_VERIFY_EQUAL1_STRING_MAC(byteCountMode, byteCountModeGet,
                                     "get byteCountMode than was set: %d", byteCountMode);
        UTF_VERIFY_EQUAL1_STRING_MAC(exceptionCommand, exceptionCommandGet,
                                     "get exceptionCommand than was set: %d", exceptionCommand);
        UTF_VERIFY_EQUAL1_STRING_MAC(exceptionCpuCode, exceptionCpuCodeGet,
                                     "get exceptionCpuCode than was set: %d", exceptionCpuCode);

        /*
        1.4. Call with  mtuCheckEnable [GT_TRUE] = feature enabled
                        byteCountMode  [CPSS_DXCH_BRG_GEN_BYTE_COUNT_MODE_MTU_L2_E]
                        exceptionCommand [CPSS_PACKET_CMD_DROP_HARD_E]
                        exceptionCpuCode [CPSS_NET_CONTROL_E]
        Expected: GT_OK.
        */
        mtuCheckEnable = GT_TRUE;
        byteCountMode  = CPSS_DXCH_BRG_GEN_BYTE_COUNT_MODE_MTU_L2_E;
        exceptionCommand = CPSS_PACKET_CMD_DROP_HARD_E;
        exceptionCpuCode = CPSS_NET_CONTROL_E;
        st = cpssDxChBrgGenMtuConfigSet(dev, mtuCheckEnable,
                                        byteCountMode, exceptionCommand,
                                        exceptionCpuCode);
        UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, dev, mtuCheckEnable, byteCountMode,
                                    exceptionCommand, exceptionCpuCode);

        /*
        1.4.2. Call cpssDxChBrgGenMtuConfigGet with not NULL modePtr
        Expected: GT_OK and the same modePtr as was set.
        */
        st = cpssDxChBrgGenMtuConfigGet(dev, &mtuCheckEnableGet,
                                        &byteCountModeGet, &exceptionCommandGet,
                                        &exceptionCpuCodeGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChBrgGenMtuConfigSet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(mtuCheckEnable, mtuCheckEnableGet,
                                     "get mtuCheckEnable than was set: %d", mtuCheckEnable);
        UTF_VERIFY_EQUAL1_STRING_MAC(byteCountMode, byteCountModeGet,
                                     "get byteCountMode than was set: %d", byteCountMode);
        UTF_VERIFY_EQUAL1_STRING_MAC(exceptionCommand, exceptionCommandGet,
                                     "get exceptionCommand than was set: %d", exceptionCommand);
        UTF_VERIFY_EQUAL1_STRING_MAC(exceptionCpuCode, exceptionCpuCodeGet,
                                     "get exceptionCpuCode than was set: %d", exceptionCpuCode);

        /*
        1.5. Call with  mtuCheckEnable [GT_TRUE] = feature enabled
                        byteCountMode  [CPSS_DXCH_BRG_GEN_BYTE_COUNT_MODE_MTU_L3_E]
                        exceptionCommand [CPSS_PACKET_CMD_DROP_SOFT_E]
                        exceptionCpuCode [CPSS_NET_CONTROL_E]
        Expected: GT_OK.
        */
        mtuCheckEnable = GT_TRUE;
        byteCountMode  = CPSS_DXCH_BRG_GEN_BYTE_COUNT_MODE_MTU_L3_E;
        exceptionCommand = CPSS_PACKET_CMD_DROP_SOFT_E;
        exceptionCpuCode = CPSS_NET_CONTROL_E;
        st = cpssDxChBrgGenMtuConfigSet(dev, mtuCheckEnable,
                                        byteCountMode, exceptionCommand,
                                        exceptionCpuCode);
        UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, dev, mtuCheckEnable, byteCountMode,
                                    exceptionCommand, exceptionCpuCode);

        /*
        1.5.2. Call cpssDxChBrgGenMtuConfigGet with not NULL modePtr
        Expected: GT_OK and the same modePtr as was set.
        */
        st = cpssDxChBrgGenMtuConfigGet(dev, &mtuCheckEnableGet,
                                        &byteCountModeGet, &exceptionCommandGet,
                                        &exceptionCpuCodeGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChBrgGenMtuConfigSet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(mtuCheckEnable, mtuCheckEnableGet,
                                     "get mtuCheckEnable than was set: %d", mtuCheckEnable);
        UTF_VERIFY_EQUAL1_STRING_MAC(byteCountMode, byteCountModeGet,
                                     "get byteCountMode than was set: %d", byteCountMode);
        UTF_VERIFY_EQUAL1_STRING_MAC(exceptionCommand, exceptionCommandGet,
                                     "get exceptionCommand than was set: %d", exceptionCommand);
        UTF_VERIFY_EQUAL1_STRING_MAC(exceptionCpuCode, exceptionCpuCodeGet,
                                     "get exceptionCpuCode than was set: %d", exceptionCpuCode);

        /*
        1.6. Call with  mtuCheckEnable [GT_TRUE] = feature enabled
                        byteCountMode  [CPSS_DXCH_BRG_GEN_BYTE_COUNT_MODE_MTU_L3_E]
                        exceptionCommand [CPSS_PACKET_CMD_BRIDGE_AND_MIRROR_E]
                        exceptionCpuCode [CPSS_NET_CONTROL_E]
        Expected: GT_BAD_PARAM due to exceptionCommand.
        */
        mtuCheckEnable = GT_TRUE;
        byteCountMode  = CPSS_DXCH_BRG_GEN_BYTE_COUNT_MODE_MTU_L3_E;
        exceptionCommand = CPSS_PACKET_CMD_BRIDGE_AND_MIRROR_E;
        exceptionCpuCode = CPSS_NET_CONTROL_E;
        st = cpssDxChBrgGenMtuConfigSet(dev, mtuCheckEnable,
                                        byteCountMode, exceptionCommand,
                                        exceptionCpuCode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        /*
        1.7. Call with  mtuCheckEnable [GT_TRUE] = feature enabled
                        byteCountMode  [CPSS_DXCH_BRG_GEN_BYTE_COUNT_MODE_MTU_L2_E]
                        exceptionCommand [CPSS_PACKET_CMD_NONE_E]
                        exceptionCpuCode [CPSS_NET_CONTROL_E]
        Expected: GT_BAD_PARAM due to exceptionCommand.
        */
        mtuCheckEnable = GT_TRUE;
        byteCountMode  = CPSS_DXCH_BRG_GEN_BYTE_COUNT_MODE_MTU_L2_E;
        exceptionCommand = CPSS_PACKET_CMD_NONE_E;
        exceptionCpuCode = CPSS_NET_CONTROL_E;
        st = cpssDxChBrgGenMtuConfigSet(dev, mtuCheckEnable,
                                        byteCountMode, exceptionCommand,
                                        exceptionCpuCode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        /*
        1.8. Call with  mtuCheckEnable [GT_FALSE] = feature disabled
                        byteCountMode  [CPSS_DXCH_BRG_GEN_BYTE_COUNT_MODE_MTU_L3_E]
                        exceptionCommand [CPSS_PACKET_CMD_FORWARD_E]
                        exceptionCpuCode [CPSS_NET_CONTROL_E]
        Expected: GT_OK.
        */
        mtuCheckEnable = GT_FALSE;
        byteCountMode  = CPSS_DXCH_BRG_GEN_BYTE_COUNT_MODE_MTU_L3_E;
        exceptionCommand = CPSS_PACKET_CMD_FORWARD_E;
        exceptionCpuCode = CPSS_NET_CONTROL_E;
        st = cpssDxChBrgGenMtuConfigSet(dev, mtuCheckEnable,
                                        byteCountMode, exceptionCommand,
                                        exceptionCpuCode);
        UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, dev, mtuCheckEnable, byteCountMode,
                                    exceptionCommand, exceptionCpuCode);

        /*
        1.8.2. Call cpssDxChBrgGenMtuConfigGet with not NULL modePtr
        Expected: GT_OK and the same modePtr as was set.
        */
        st = cpssDxChBrgGenMtuConfigGet(dev, &mtuCheckEnableGet,
                                        &byteCountModeGet, &exceptionCommandGet,
                                        &exceptionCpuCodeGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChBrgGenMtuConfigSet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(mtuCheckEnable, mtuCheckEnableGet,
                                     "get mtuCheckEnable than was set: %d", mtuCheckEnable);
        UTF_VERIFY_EQUAL1_STRING_MAC(byteCountMode, byteCountModeGet,
                                     "get byteCountMode than was set: %d", byteCountMode);
        UTF_VERIFY_EQUAL1_STRING_MAC(exceptionCommand, exceptionCommandGet,
                                     "get exceptionCommand than was set: %d", exceptionCommand);
        UTF_VERIFY_EQUAL1_STRING_MAC(exceptionCpuCode, exceptionCpuCodeGet,
                                     "get exceptionCpuCode than was set: %d", exceptionCpuCode);

        /*
        1.9. Call with  mtuCheckEnable [GT_FALSE] = feature disabled
                        byteCountMode  [CPSS_DXCH_BRG_GEN_BYTE_COUNT_MODE_MTU_L2_E]
                        exceptionCommand [CPSS_PACKET_CMD_NONE_E]
                        exceptionCpuCode [CPSS_NET_CONTROL_E]
        Expected: GT_BAD_PARAM due to exceptionCommand.
        */
        mtuCheckEnable = GT_FALSE;
        byteCountMode  = CPSS_DXCH_BRG_GEN_BYTE_COUNT_MODE_MTU_L2_E;
        exceptionCommand = CPSS_PACKET_CMD_NONE_E;
        exceptionCpuCode = CPSS_NET_CONTROL_E;
        st = cpssDxChBrgGenMtuConfigSet(dev, mtuCheckEnable,
                                        byteCountMode, exceptionCommand,
                                        exceptionCpuCode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        /*
            1.10. Call with wrong enum values.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChBrgGenMtuConfigSet
                            (dev, mtuCheckEnable, byteCountMode,
                             exceptionCommand, exceptionCpuCode),
                            byteCountMode);
        UTF_ENUMS_CHECK_MAC(cpssDxChBrgGenMtuConfigSet
                            (dev, mtuCheckEnable, byteCountMode,
                             exceptionCommand, exceptionCpuCode),
                            exceptionCommand);
        UTF_ENUMS_CHECK_MAC(cpssDxChBrgGenMtuConfigSet
                            (dev, mtuCheckEnable, byteCountMode,
                             exceptionCommand, exceptionCpuCode),
                            exceptionCpuCode);
    }

    mtuCheckEnable = GT_TRUE;
    byteCountMode  = CPSS_DXCH_BRG_GEN_BYTE_COUNT_MODE_MTU_L2_E;
    exceptionCommand = CPSS_PACKET_CMD_DROP_HARD_E;
    exceptionCpuCode = CPSS_NET_CONTROL_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgGenMtuConfigSet(dev, mtuCheckEnable,
                                        byteCountMode, exceptionCommand,
                                        exceptionCpuCode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgGenMtuConfigSet(dev, mtuCheckEnable,
                                    byteCountMode, exceptionCommand,
                                    exceptionCpuCode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgGenMtuConfigGet
(
    IN  GT_U8                                         devNum,
    OUT GT_BOOL                                      *mtuCheckEnablePtr,
    OUT CPSS_DXCH_BRG_GEN_BYTE_COUNT_MODE_MTU_ENT    *byteCountModePtr,
    OUT CPSS_PACKET_CMD_ENT                          *exceptionCommandPtr,
    OUT CPSS_NET_RX_CPU_CODE_ENT                     *exceptionCpuCodePtr
)*/
UTF_TEST_CASE_MAC(cpssDxChBrgGenMtuConfigGet)
{
/*
    ITERATE_DEVICES (Bobcat2, Caelum, Bobcat3)
    1.1. Call with not NULL modePtr.
    Expected: GT_OK
    1.2. Call with modePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS                                    st = GT_OK;
    GT_U8                                        dev;
    GT_U32                                       notAppFamilyBmp;
    GT_BOOL                                      mtuCheckEnable;
    CPSS_DXCH_BRG_GEN_BYTE_COUNT_MODE_MTU_ENT    byteCountMode;
    CPSS_PACKET_CMD_ENT                          exceptionCommand;
    CPSS_NET_RX_CPU_CODE_ENT                     exceptionCpuCode;

    mtuCheckEnable = GT_TRUE;
    byteCountMode  = CPSS_DXCH_BRG_GEN_BYTE_COUNT_MODE_MTU_L2_E;
    exceptionCommand = CPSS_PACKET_CMD_DROP_HARD_E;
    exceptionCpuCode = CPSS_NET_CONTROL_E;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with not NULL modePtr.
            Expected: GT_OK
        */
        st = cpssDxChBrgGenMtuConfigGet(dev, &mtuCheckEnable,
                                    &byteCountMode, &exceptionCommand,
                                    &exceptionCpuCode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with modePtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChBrgGenMtuConfigGet(dev, NULL,
                                    &byteCountMode, &exceptionCommand,
                                    &exceptionCpuCode);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, mtuCheckEnable = NULL", dev);

        st = cpssDxChBrgGenMtuConfigGet(dev, &mtuCheckEnable,
                                    NULL, &exceptionCommand,
                                    &exceptionCpuCode);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, byteCountMode = NULL", dev);

        st = cpssDxChBrgGenMtuConfigGet(dev, &mtuCheckEnable,
                                    &byteCountMode, NULL,
                                    &exceptionCpuCode);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, exceptionCommand = NULL", dev);

        st = cpssDxChBrgGenMtuConfigGet(dev, &mtuCheckEnable,
                                    &byteCountMode, &exceptionCommand,
                                    NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, exceptionCpuCode = NULL", dev);

    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgGenMtuConfigGet(dev, &mtuCheckEnable,
                                        &byteCountMode, &exceptionCommand,
                                        &exceptionCpuCode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgGenMtuConfigGet(dev, &mtuCheckEnable,
                                        &byteCountMode, &exceptionCommand,
                                        &exceptionCpuCode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgGenMtuPortProfileIdxSet
(
    IN GT_U8         devNum,
    IN GT_PORT_NUM   portNum,
    IN GT_U32        mtuProfileIdx
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgGenMtuPortProfileIdxSet)
{
/*
    ITERATE_DEVICES (Bobcat2, Caelum, Bobcat3)
    1. for all active devices
      1.1 for all active virtual ports
        1.1.1 Call with mtuProfileIdx  { legal value = 0 }
        Expected: GT_OK.
        1.1.2. Call with mtuProfileIdx  { legal value = 3 }
        Expected: GT_OK.
        1.1.3. Call with mtuProfileIdx out of range valud   { value = 4 }
        Expected: GT_BAD_PARAM.
      1.2 for all non-active virtual ports
        1.2.1
*/

    GT_STATUS       st = GT_OK;

    GT_U8           devNum;
    GT_PORT_NUM     portNum = 0;
    GT_U32          notAppFamilyBmp;
    GT_U32          mtuProfileIdx, mtuProfileIdxGet;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        st = prvUtfNextVirtPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&portNum, GT_TRUE))
        {
            /*
                1.1.1. Call with mtuProfileIdx  { legal value = 0 }
                Expected: GT_OK.
            */
            mtuProfileIdx = 0;

            st = cpssDxChBrgGenMtuPortProfileIdxSet(devNum,
                                                  portNum,
                                                  mtuProfileIdx);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portNum, mtuProfileIdx);

            /*
                1.1.2. Call cpssDxChBrgEportMtuProfileIdxGet.
                Expected: GT_OK and the same mtuProfileIdx.
            */
            st = cpssDxChBrgGenMtuPortProfileIdxGet(devNum,
                                                  portNum,
                                                 &mtuProfileIdxGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                "cpssDxChBrgGenMtuPortProfileIdxGet: %d, %d", devNum, portNum);
            UTF_VERIFY_EQUAL2_STRING_MAC(mtuProfileIdx, mtuProfileIdxGet,
                "get another mtuProfileIdxGet than was set: %d, %d", mtuProfileIdx, mtuProfileIdxGet);

            /*
                1.2.1. Call with mtuProfileIdx  { legal value = 3 }
                Expected: GT_OK.
            */
            mtuProfileIdx = 3;

            st = cpssDxChBrgGenMtuPortProfileIdxSet(devNum,
                                                  portNum,
                                                  mtuProfileIdx);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portNum, mtuProfileIdx);

            /*
                1.2.2. Call cpssDxChBrgEportMtuProfileIdxGet.
                Expected: GT_OK and the same mtuProfileIdx.
            */
            st = cpssDxChBrgGenMtuPortProfileIdxGet(devNum,
                                                  portNum,
                                                 &mtuProfileIdxGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                "cpssDxChBrgGenMtuPortProfileIdxGet: %d, %d", devNum, portNum);
            UTF_VERIFY_EQUAL2_STRING_MAC(mtuProfileIdx, mtuProfileIdxGet,
                "get another mtuProfileIdxGet than was set: %d, %d", mtuProfileIdx, mtuProfileIdxGet);
            /*
                1.3.1. Call with out of range mtuProfileIdx  { 4 }
                Expected: GT_BAD_PARAM
            */
            mtuProfileIdx = 4;

            st = cpssDxChBrgGenMtuPortProfileIdxSet(devNum,
                                                  portNum,
                                                  mtuProfileIdx);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum, mtuProfileIdx);
        }

        mtuProfileIdx = 1;

        st = prvUtfNextVirtPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&portNum, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChBrgGenMtuPortProfileIdxSet(devNum,
                                                  portNum,
                                                  mtuProfileIdx);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        portNum = UTF_CPSS_PP_MAX_PORT_NUM_CNS(devNum);

        st = cpssDxChBrgGenMtuPortProfileIdxSet(devNum,
                                              portNum,
                                              mtuProfileIdx);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                     */
        portNum = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgGenMtuPortProfileIdxSet(devNum,
                                              portNum,
                                              mtuProfileIdx);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    mtuProfileIdx  = 2;
    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChBrgGenMtuPortProfileIdxSet(devNum,
                                              portNum,
                                              mtuProfileIdx);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3. Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgGenMtuPortProfileIdxSet(devNum,
                                          portNum,
                                          mtuProfileIdx);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgGenMtuPortProfileIdxGet
(
    IN  GT_U8          devNum,
    IN  GT_PORT_NUM    portNum,
    OUT GT_U32         *mtuProfileIdxPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgGenMtuPortProfileIdxGet)
{
/*
    ITERATE_DEVICES (Bobcat2, Caelum, Bobcat3)
    1.1.1. Call function with not NULL mtuProfileIdxPtr.
    Expected: GT_OK.
    1.1.2. Call function with NULL mtuProfileIdxPtr.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS                               st = GT_OK;

    GT_U8                                   devNum;
    GT_PORT_NUM                             portNum = 0;
    GT_U32                                  mtuProfileIdx;
    GT_U32                                  notAppFamilyBmp;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);


    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* 1.1. For all active devices go over all available physical ports. */
        st = prvUtfNextVirtPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while(GT_OK == prvUtfNextVirtPortGet(&portNum, GT_TRUE))
        {
            /*
                1.1.1. Call function with not NULL mtuProfileIdxPtr.
                Expected: GT_OK.
            */
            st = cpssDxChBrgGenMtuPortProfileIdxGet(devNum, portNum, &mtuProfileIdx);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portNum, &mtuProfileIdx);

            /*
                1.1.2. Call function with NULL mtuProfileIdxPtr.
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChBrgGenMtuPortProfileIdxGet(devNum, portNum, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st,
                "%d, %d, physicalInfoPtr = NULL", devNum, portNum);
        }

        st = prvUtfNextVirtPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available virtual ports. */
        while(GT_OK == prvUtfNextVirtPortGet(&portNum, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active portNum */
            st = cpssDxChBrgGenMtuPortProfileIdxGet(devNum, portNum, &mtuProfileIdx);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        portNum = UTF_CPSS_PP_MAX_PORT_NUM_CNS(devNum) ;

        st = cpssDxChBrgGenMtuPortProfileIdxGet(devNum, portNum, &mtuProfileIdx);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU portNum number.                                  */
        portNum = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgGenMtuPortProfileIdxGet(devNum, portNum, &mtuProfileIdx);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    portNum = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChBrgGenMtuPortProfileIdxGet(devNum, portNum, &mtuProfileIdx);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3. Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgGenMtuPortProfileIdxGet(devNum, portNum, &mtuProfileIdx);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);

}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgGenMtuExceedCntrGet
(
    IN  GT_U8        devNum,
    OUT GT_U32       *valuePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgGenMtuExceedCntrGet)
{
/*
    ITERATE_DEVICES (Bobcat2, Caelum, Bobcat3)
    1.1.1. Call function with not NULL valuePtr.
    Expected: GT_OK.
    1.1.2. Call function with NULL valuePtr.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS                               st = GT_OK;

    GT_U8                                   devNum;
    GT_U32                                  value;
    GT_U32                                  notAppFamilyBmp;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);


    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /*
            1.1.1. Call function with not NULL valuePtr.
            Expected: GT_OK.
        */
        st = cpssDxChBrgGenMtuExceedCntrGet(devNum, &value);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, &value);

        /*
            1.1.2. Call function with NULL valuePtr.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChBrgGenMtuExceedCntrGet(devNum, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,
                                     "%d, valuePtr = NULL", devNum);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChBrgGenMtuExceedCntrGet(devNum, &value);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3. Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgGenMtuExceedCntrGet(devNum, &value);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgGenMtuProfileSet
(
    IN GT_U8    devNum,
    IN GT_U32   profile,
    IN GT_U32   mtuSize
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgGenMtuProfileSet)
{
/*
    ITERATE_DEVICES
    ITERATE_ALL_PROFILES
    1.1. Call cpssDxChBrgGenMtuProfileSet with relevant values
         mtuSize[0/0x1FFF/0x3FFF].
    Expected: GT_OK.
    1.2. Call cpssDxChBrgGenMtuProfileSet.
    Expected: GT_OK and the same mtuSize.
    1.3. Call with out of range profile[4].
    Expected: NOT GT_OK.
    1.4. Call with out of range mtuSize[0x4000].
    Expected: NOT GT_OK.
*/
    GT_STATUS   st   = GT_OK;
    GT_U8       dev  = 0 ;

    GT_U32   profile;
    GT_U32   mtuSize;
    GT_U32   mtuSizeGet = 0;

    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    mtuSize = 0x100;

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* ITERATE_ALL_PROFILES */
        for (profile = 0; profile < 4; profile++) {
            /*  1.1.  */
            mtuSize = 0x3FFF;

            st = cpssDxChBrgGenMtuProfileSet(dev, profile, mtuSize);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*  1.2.  */
            st = cpssDxChBrgGenMtuProfileGet(dev, profile, &mtuSizeGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* Verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(mtuSize, mtuSizeGet,
                       "get another mtuSize than was set: %d", dev);

            /*  1.1.  */
            mtuSize = 0x1FFF;
            st = cpssDxChBrgGenMtuProfileSet(dev, profile, mtuSize);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*  1.2.  */
            st = cpssDxChBrgGenMtuProfileGet(dev, profile, &mtuSizeGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* Verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(mtuSize, mtuSizeGet,
                       "get another mtuSize than was set: %d", dev);

            /*  1.4.  */
            mtuSize = 0x4000;
            st = cpssDxChBrgGenMtuProfileSet(dev, profile, mtuSize);
            UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);
            mtuSize = 0;
        }

        /*  1.3.  */
        profile = 4;
        st = cpssDxChBrgGenMtuProfileSet(dev, profile, mtuSize);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        profile = 0;

    }
    profile = 0;
    mtuSize = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgGenMtuProfileSet(dev, profile, mtuSize);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgGenMtuProfileSet(dev, profile, mtuSize);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgGenMtuProfileGet
(
    IN  GT_U8   devNum,
    IN  GT_U32  profile,
    OUT GT_U32  *mtuSizePtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChBrgGenMtuProfileGet)
{
/*
    ITERATE_DEVICES
    1.1. Call cpssDxChBrgGenMtuProfileGet with relevant values
         profile[0/3],
         and not NULL mtuSizePtr.
    Expected: GT_OK.
    1.2. Call with out of range profile[4].
    Expected: NOT GT_OK.
    1.3. Call with NULL mtuSizePtr.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st   = GT_OK;
    GT_U8       dev  = 0 ;

    GT_U32   profile = 0;
    GT_U32   mtuSize = 0;

    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*  1.1.  */
        profile = 0;

        st = cpssDxChBrgGenMtuProfileGet(dev, profile, &mtuSize);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.1.  */
        profile = 3;

        st = cpssDxChBrgGenMtuProfileGet(dev, profile, &mtuSize);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.2.  */
        profile = 4;

        st = cpssDxChBrgGenMtuProfileGet(dev, profile, &mtuSize);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        profile = 0;

        /*  1.3.  */
        st = cpssDxChBrgGenMtuProfileGet(dev, profile, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }
    profile = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgGenMtuProfileGet(dev, profile, &mtuSize);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgGenMtuProfileGet(dev, profile, &mtuSize);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgGenIpmClassificationModeSet
(
    IN  GT_U8                                           devNum,
    IN  CPSS_DXCH_BRG_GEN_IPM_CLASSIFICATION_MODE_ENT   mode
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgGenIpmClassificationModeSet)
{
/*
    ITERATE_DEVICES (xCat3)
    1.1. Call with mode [CPSS_DXCH_BRG_GEN_IPM_CLASSIFICATION_MODE_MAC_DA_E/
                         CPSS_DXCH_BRG_GEN_IPM_CLASSIFICATION_MODE_MAC_DA_AND_ETHERTYPE_E].
    Expected: GT_OK.
    1.2. Call cpssDxChBrgGenIpmClassificationModeGet with not NULL modePtr.
    Expected: GT_OK and the same modePtr as was set.
    1.3. Call with mode.
    Expected: GT_BAD_PARAM.
    Call with wrong enum values mode
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;

    CPSS_DXCH_BRG_GEN_IPM_CLASSIFICATION_MODE_ENT  mode    = CPSS_DXCH_BRG_GEN_IPM_CLASSIFICATION_MODE_MAC_DA_E;
    CPSS_DXCH_BRG_GEN_IPM_CLASSIFICATION_MODE_ENT  modeGet = CPSS_DXCH_BRG_GEN_IPM_CLASSIFICATION_MODE_MAC_DA_E;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_LION_E |
                                           UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with mode [CPSS_DXCH_BRG_GEN_IPM_CLASSIFICATION_MODE_MAC_DA_E /
                                 CPSS_DXCH_BRG_GEN_IPM_CLASSIFICATION_MODE_MAC_DA_AND_ETHERTYPE_E]
            Expected: GT_OK.
        */
        /* iterate with mode = CPSS_DXCH_BRG_GEN_IPM_CLASSIFICATION_MODE_MAC_DA_E */
        mode = CPSS_DXCH_BRG_GEN_IPM_CLASSIFICATION_MODE_MAC_DA_E;

        st = cpssDxChBrgGenIpmClassificationModeSet(dev, mode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, mode);

        /*
            1.2. Call cpssDxChBrgGenIpmClassificationModeGet with not NULL modePtr
            Expected: GT_OK and the same modePtr as was set.
        */
        st = cpssDxChBrgGenIpmClassificationModeGet(dev, &modeGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChBrgGenIpmClassificationModeGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(mode, modeGet, "get another mode than was set: %d", dev);

        /* iterate with mode = CPSS_DXCH_BRG_GEN_IPM_CLASSIFICATION_MODE_MAC_DA_AND_ETHERTYPE_E */
        mode = CPSS_DXCH_BRG_GEN_IPM_CLASSIFICATION_MODE_MAC_DA_AND_ETHERTYPE_E;

        st = cpssDxChBrgGenIpmClassificationModeSet(dev, mode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, mode);

        /*
            1.3. Call cpssDxChBrgGenIpmClassificationModeGet with not NULL modePtr
            Expected: GT_OK and the same modePtr as was set.
        */
        st = cpssDxChBrgGenIpmClassificationModeGet(dev, &modeGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChBrgGenIpmClassificationModeGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(mode, modeGet, "get another mode than was set: %d", dev);

        /*
            1.4. Call with wrong enum values mode.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChBrgGenIpmClassificationModeSet
                            (dev, mode),
                            mode);
    }

    mode = CPSS_DXCH_BRG_GEN_IPM_CLASSIFICATION_MODE_MAC_DA_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_LION_E |
                                           UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgGenIpmClassificationModeSet(dev, mode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgGenIpmClassificationModeSet(dev, mode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgGenIpmClassificationModeGet
(
    IN  GT_U8                                           devNum,
    OUT CPSS_DXCH_BRG_GEN_IPM_CLASSIFICATION_MODE_ENT   *modePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgGenIpmClassificationModeGet)
{
/*
    ITERATE_DEVICES (xCat3)
    1.1. Call with not NULL modePtr.
    Expected: GT_OK
    1.2. Call with modePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    CPSS_DXCH_BRG_GEN_IPM_CLASSIFICATION_MODE_ENT  mode    = CPSS_DXCH_BRG_GEN_IPM_CLASSIFICATION_MODE_MAC_DA_E;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_LION_E |
                                           UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with not NULL modePtr.
            Expected: GT_OK
        */
        st = cpssDxChBrgGenIpmClassificationModeGet(dev, &mode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with modePtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChBrgGenIpmClassificationModeGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, modePtr = NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_LION_E |
                                           UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgGenIpmClassificationModeGet(dev, &mode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgGenIpmClassificationModeGet(dev, &mode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
 * Configuration of cpssDxChBrgGen suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssDxChBrgGen)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgGenIgmpSnoopEnable)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgGenIgmpSnoopEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgGenDropIpMcEnable)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgGenDropIpMcEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgGenDropNonIpMcEnable)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgGenDropNonIpMcEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgGenDropInvalidSaEnable)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgGenDropInvalidSaEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgGenUcLocalSwitchingEnable)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgGenUcLocalSwitchingEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgGenIgmpSnoopModeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgGenIgmpSnoopModeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgGenArpBcastToCpuCmdSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgGenArpBcastToCpuCmdGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgGenRipV1MirrorToCpuEnable)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgGenRipV1MirrorToCpuEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgGenIeeeReservedMcastTrapEnable)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgGenIeeeReservedMcastTrapEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgGenIeeeReservedMcastProtCmdSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgGenIeeeReservedMcastProtCmdGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgGenCiscoL2ProtCmdSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgGenCiscoL2ProtCmdGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgGenIpV6IcmpTrapEnable)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgGenIpV6IcmpTrapEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgGenIcmpv6MsgTypeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgGenIcmpv6MsgTypeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgGenIpLinkLocalMirrorToCpuEnable)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgGenIpLinkLocalMirrorToCpuEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgGenIpLinkLocalProtMirrorToCpuEnable)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgGenIpLinkLocalProtMirrorToCpuEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgGenIpV6SolicitedCmdSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgGenIpV6SolicitedCmdGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgGenIpV6SolicitedNodeMcastPrefixSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgGenIpV6SolicitedNodeMcastPrefixGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgGenRateLimitGlobalCfgSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgGenRateLimitGlobalCfgGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgGenPortRateLimitSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgGenPortRateLimitGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgGenPortRateLimitTcpSynSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgGenPortRateLimitTcpSynGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgGenUdpBcDestPortCfgSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgGenUdpBcDestPortCfgInvalidate)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgGenBpduTrapEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgGenBpduTrapEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgGenArpTrapEnable)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgGenArpTrapEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgGenDropIpMcModeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgGenDropIpMcModeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgGenDropNonIpMcModeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgGenDropNonIpMcModeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgGenCfiRelayEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgGenCfiRelayEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgGenPortIeeeReservedMcastProfileIndexSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgGenPortIeeeReservedMcastProfileIndexGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgGenPortOversizeUntaggedPacketsFilterEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgGenPortOversizeUntaggedPacketsFilterEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgGenUdpBcDestPortCfgGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgGenPortRateLimitSpeedGranularitySet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgGenPortRateLimitSpeedGranularityGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgGenRateLimitSpeedCfgEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgGenRateLimitSpeedCfgEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgGenExcludeLinkLocalMcFromUnregMcFilterEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgGenExcludeLinkLocalMcFromUnregMcFilterEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgGenPortIeeeReservedMcastLearningEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgGenPortIeeeReservedMcastLearningEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgGenIngressPortBcFilterDaCommandSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgGenIngressPortBcFilterDaCommandGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgGenIngressPortUnknownUcFilterDaCommandSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgGenIngressPortUnknownUcFilterDaCommandGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgGenIngressPortUnregisteredMcFilterDaCommandSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgGenIngressPortUnregisteredMcFilterDaCommandGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgGenPortArpMacSaMismatchDropEnable)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgGenPortArpMacSaMismatchDropEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgGenPortBcUdpTrapMirrorEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgGenPortBcUdpTrapMirrorEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgGenPortIpControlTrapEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgGenPortIpControlTrapEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgGenRateLimitDropCntrSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgGenRateLimitDropCntrGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgGenPortRateLimitDropCntrEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgGenPortRateLimitDropCntrEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgGenPortOversizeUntaggedPacketsFilterCmdSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgGenPortOversizeUntaggedPacketsFilterCmdGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgGenBypassModeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgGenBypassModeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgGenMtuConfigSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgGenMtuConfigGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgGenMtuPortProfileIdxSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgGenMtuPortProfileIdxGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgGenMtuExceedCntrGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgGenMtuProfileSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgGenMtuProfileGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgGenIpmClassificationModeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgGenIpmClassificationModeGet)
UTF_SUIT_END_TESTS_MAC(cpssDxChBrgGen)


