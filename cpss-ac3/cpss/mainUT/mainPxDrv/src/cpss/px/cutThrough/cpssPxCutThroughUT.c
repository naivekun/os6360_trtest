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
* @file cpssPxCutThroughUT.c
*
* @brief Unit tests for cpssPxCutThrough.
*
* @version   1
********************************************************************************
*/
/* includes */
/* the define of UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC must come from C files that
   already fixed the types of ports from GT_U8 !

   NOTE: must come before ANY include to H files !!!!
*/
#define UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>

#include <cpss/px/cutThrough/cpssPxCutThrough.h>
#include <cpss/px/config/private/prvCpssPxInfo.h>
#include <cpss/px/config/cpssPxCfgInit.h>


/* defines */

/* Default valid value for port id */
#define PORT_CUT_THROUGH_VALID_PHY_PORT_CNS  0

UTF_TEST_CASE_MAC(cpssPxCutThroughPortEnableSet)
{
    GT_STATUS               st     = GT_OK;
    GT_U8                   dev;
    GT_PHYSICAL_PORT_NUM    port   = PORT_CUT_THROUGH_VALID_PHY_PORT_CNS;
    GT_BOOL                 enable;
    GT_BOOL                 untaggedEnable;
    GT_BOOL                 enableGet;
    GT_BOOL                 untaggedEnableGet;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. For all active devices go over all available physical ports. */
        st = prvUtfNextRxDmaPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        while(GT_OK == prvUtfNextRxDmaPortGet(&port, GT_TRUE))
        {
            enable           = ((port & 2) ? GT_TRUE : GT_FALSE);
            untaggedEnable   = ((port & 1) ? GT_TRUE : GT_FALSE);

            st = cpssPxCutThroughPortEnableSet(
                dev, port, enable, untaggedEnable);
            UTF_VERIFY_EQUAL2_STRING_MAC(
                GT_OK, st, "[cpssPxCutThroughPortEnableSet]: %d, %d", dev, port);

            st = cpssPxCutThroughPortEnableGet(
                dev, port, &enableGet, &untaggedEnableGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(
                GT_OK, st, "[cpssPxCutThroughPortEnableGet]: %d, %d", dev, port);

            UTF_VERIFY_EQUAL2_STRING_MAC(
                enable, enableGet,
                "enable: get another value than was set: %d, %d", dev, port);
            UTF_VERIFY_EQUAL2_STRING_MAC(
                untaggedEnable, untaggedEnableGet,
                "untaggedEnable: get another value than was set: %d, %d", dev, port);
        }

        /* 1.2. For all active devices go over all non available
           physical ports.
        */
        enable           = GT_FALSE;
        untaggedEnable   = GT_FALSE;
        st = prvUtfNextRxDmaPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        while(GT_OK == prvUtfNextRxDmaPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssPxCutThroughPortEnableSet(
                dev, port, enable, untaggedEnable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);
        enable           = GT_FALSE;
        untaggedEnable   = GT_FALSE;
        st = cpssPxCutThroughPortEnableSet(
            dev, port, enable, untaggedEnable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    port = PORT_CUT_THROUGH_VALID_PHY_PORT_CNS;
    enable           = GT_FALSE;
    untaggedEnable   = GT_FALSE;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxCutThroughPortEnableSet(
            dev, port, enable, untaggedEnable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    /* port == 0, enable == GT_TRUE */

    st = cpssPxCutThroughPortEnableSet(
        dev, port, enable, untaggedEnable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

UTF_TEST_CASE_MAC(cpssPxCutThroughPortEnableGet)
{
    GT_STATUS               st     = GT_OK;
    GT_U8                   dev;
    GT_PHYSICAL_PORT_NUM    port   = PORT_CUT_THROUGH_VALID_PHY_PORT_CNS;
    GT_BOOL                 enableGet;
    GT_BOOL                 untaggedEnableGet;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. For all active devices go over all available physical ports. */
        st = prvUtfNextRxDmaPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        while(GT_OK == prvUtfNextRxDmaPortGet(&port, GT_TRUE))
        {
            st = cpssPxCutThroughPortEnableGet(
                dev, port, &enableGet, &untaggedEnableGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(
                GT_OK, st, "[cpssPxCutThroughPortEnableGet]: %d, %d", dev, port);
        }

        /* 1.2. For all active devices go over all non available
           physical ports.
        */
        st = prvUtfNextRxDmaPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        while(GT_OK == prvUtfNextRxDmaPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssPxCutThroughPortEnableGet(
                dev, port, &enableGet, &untaggedEnableGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);
        st = cpssPxCutThroughPortEnableGet(
            dev, port, &enableGet, &untaggedEnableGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. check null-pointer parameters using first valid port*/
        st = prvUtfNextRxDmaPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        while(GT_OK == prvUtfNextRxDmaPortGet(&port, GT_TRUE))
        {
            st = cpssPxCutThroughPortEnableGet(
                dev, port, NULL, &untaggedEnableGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

            st = cpssPxCutThroughPortEnableGet(
                dev, port, &enableGet, NULL);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
            break;
        }
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    port = PORT_CUT_THROUGH_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxCutThroughPortEnableGet(
            dev, port, &enableGet, &untaggedEnableGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    /* port == 0, enable == GT_TRUE */

    st = cpssPxCutThroughPortEnableGet(
        dev, port, &enableGet, &untaggedEnableGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

UTF_TEST_CASE_MAC(cpssPxCutThroughUpEnableSet)
{
    GT_STATUS               st     = GT_OK;
    GT_U8                   dev;
    GT_U32                  up;
    GT_BOOL                 enable;
    GT_BOOL                 enableGet;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. For all active devices go over all valid up values. */
        for (up = 0; (up < 8); up++)
        {
            enable = ((up & 1) ? GT_TRUE : GT_FALSE);

            st = cpssPxCutThroughUpEnableSet(
                dev, up, enable);
            UTF_VERIFY_EQUAL2_STRING_MAC(
                GT_OK, st, "[cpssPxCutThroughUpEnableSet]: %d, %d", dev, up);

            st = cpssPxCutThroughUpEnableGet(
                dev, up, &enableGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(
                GT_OK, st, "[cpssPxCutThroughUpEnableGet]: %d, %d", dev, up);

            UTF_VERIFY_EQUAL2_STRING_MAC(
                enable, enableGet,
                "enable: get another value than was set: %d, %d", dev, up);
        }

        /* 1.2. For all active devices call with wrong up.
        */
        up     = 8;
        enable = GT_FALSE;
        st = cpssPxCutThroughUpEnableSet(
            dev, up, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, up);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    up     = 0;
    enable = GT_FALSE;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxCutThroughUpEnableSet(
            dev, up, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    /* up == 0, enable == GT_TRUE */

    st = cpssPxCutThroughUpEnableSet(
        dev, up, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

UTF_TEST_CASE_MAC(cpssPxCutThroughUpEnableGet)
{
    GT_STATUS               st     = GT_OK;
    GT_U8                   dev;
    GT_U32                  up;
    GT_BOOL                 enableGet;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. For all active devices go over all valid up values. */
        for (up = 0; (up < 8); up++)
        {
            st = cpssPxCutThroughUpEnableGet(
                dev, up, &enableGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(
                GT_OK, st, "[cpssPxCutThroughUpEnableGet]: %d, %d", dev, up);
        }

        /* 1.2. For all active devices call with wrong up.
        */
        up     = 8;
        st = cpssPxCutThroughUpEnableGet(
            dev, up, &enableGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, up);

        /* 1.2. For all active devices call NULL pointer parameter.
        */
        up     = 0;
        st = cpssPxCutThroughUpEnableGet(
            dev, up, NULL);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, up);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    up     = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxCutThroughUpEnableGet(
            dev, up, &enableGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    /* up == 0, enable == GT_TRUE */

    st = cpssPxCutThroughUpEnableGet(
        dev, up, &enableGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

UTF_TEST_CASE_MAC(cpssPxCutThroughVlanEthertypeSet)
{
    GT_STATUS               st     = GT_OK;
    GT_U8                   dev;
    GT_U32                  etherType0;
    GT_U32                  etherType1;
    GT_U32                  etherType0Get;
    GT_U32                  etherType1Get;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. For all active devices call with valid parameter values. */
        etherType0 = 0x1234;
        etherType1 = 0x5678;
        st = cpssPxCutThroughVlanEthertypeSet(
            dev, etherType0, etherType1);
        UTF_VERIFY_EQUAL1_STRING_MAC(
            GT_OK, st, "[cpssPxCutThroughVlanEthertypeSet]: %d", dev);

        st = cpssPxCutThroughVlanEthertypeGet(
            dev, &etherType0Get, &etherType1Get);
        UTF_VERIFY_EQUAL1_STRING_MAC(
            GT_OK, st, "[cpssPxCutThroughVlanEthertypeGet]: %d", dev);

        UTF_VERIFY_EQUAL1_STRING_MAC(
            etherType0, etherType0Get,
            "etherType0: get another value than was set: %d", dev);

        UTF_VERIFY_EQUAL1_STRING_MAC(
            etherType1, etherType1Get,
            "etherType1: get another value than was set: %d", dev);

        /* 1.2. For all active devices call with wrong etherType0.
        */
        etherType0 = BIT_16;
        etherType1 = 0x8888;
        st = cpssPxCutThroughVlanEthertypeSet(
            dev, etherType0, etherType1);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, dev);

        /* 1.3. For all active devices call with wrong etherType1.
        */
        etherType0 = 0x7777;
        etherType1 = BIT_16;
        st = cpssPxCutThroughVlanEthertypeSet(
            dev, etherType0, etherType1);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, dev);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);
    etherType0 = 0x1234;
    etherType1 = 0x5678;

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxCutThroughVlanEthertypeSet(
            dev, etherType0, etherType1);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    etherType0 = 0x1234;
    etherType1 = 0x5678;

    st = cpssPxCutThroughVlanEthertypeSet(
        dev, etherType0, etherType1);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

UTF_TEST_CASE_MAC(cpssPxCutThroughVlanEthertypeGet)
{
    GT_STATUS               st     = GT_OK;
    GT_U8                   dev;
    GT_U32                  etherType0Get;
    GT_U32                  etherType1Get;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. For all active devices call with valid parameter values. */
        st = cpssPxCutThroughVlanEthertypeGet(
            dev, &etherType0Get, &etherType1Get);
        UTF_VERIFY_EQUAL1_STRING_MAC(
            GT_OK, st, "[cpssPxCutThroughVlanEthertypeGet]: %d", dev);


        /* 1.2. For all active devices call with NULL etherType0Ptr.
        */
        st = cpssPxCutThroughVlanEthertypeGet(
            dev, NULL, &etherType1Get);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /* 1.3. For all active devices call with NULL etherType1Ptr.
        */
        st = cpssPxCutThroughVlanEthertypeGet(
            dev, &etherType0Get, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxCutThroughVlanEthertypeGet(
            dev, &etherType0Get, &etherType1Get);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxCutThroughVlanEthertypeGet(
        dev, &etherType0Get, &etherType1Get);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

UTF_TEST_CASE_MAC(cpssPxCutThroughMaxBuffersLimitSet)
{
    GT_STATUS               st     = GT_OK;
    GT_U8                   dev;
    GT_PHYSICAL_PORT_NUM    port   = PORT_CUT_THROUGH_VALID_PHY_PORT_CNS;
    GT_U32                  buffersLimit;
    GT_U32                  buffersLimitGet;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. For all active devices go over all available physical ports. */
        st = prvUtfNextRxDmaPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        while(GT_OK == prvUtfNextRxDmaPortGet(&port, GT_TRUE))
        {
            buffersLimit = port;

            st = cpssPxCutThroughMaxBuffersLimitSet(
                dev, port, buffersLimit);
            UTF_VERIFY_EQUAL2_STRING_MAC(
                GT_OK, st, "[cpssPxCutThroughMaxBuffersLimitSet]: %d, %d", dev, port);

            st = cpssPxCutThroughMaxBuffersLimitGet(
                dev, port, &buffersLimitGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(
                GT_OK, st, "[cpssPxCutThroughMaxBuffersLimitGet]: %d, %d", dev, port);

            UTF_VERIFY_EQUAL2_STRING_MAC(
                buffersLimit, buffersLimitGet,
                "buffersLimit: get another value than was set: %d, %d", dev, port);
        }

        /* 1.2. For all active devices go over all non available
           physical ports.
        */
        buffersLimit = 100;
        st = prvUtfNextRxDmaPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        while(GT_OK == prvUtfNextRxDmaPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssPxCutThroughMaxBuffersLimitSet(
                dev, port, buffersLimit);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);
        buffersLimit = 100;
        st = cpssPxCutThroughMaxBuffersLimitSet(
            dev, port, buffersLimit);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. too big size - check on one port only */
        port = PORT_CUT_THROUGH_VALID_PHY_PORT_CNS;
        buffersLimit = BIT_16;
        st = cpssPxCutThroughMaxBuffersLimitSet(
            dev, port, buffersLimit);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OUT_OF_RANGE, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    port = PORT_CUT_THROUGH_VALID_PHY_PORT_CNS;
    buffersLimit = 100;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxCutThroughMaxBuffersLimitSet(
            dev, port, buffersLimit);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxCutThroughMaxBuffersLimitSet(
        dev, port, buffersLimit);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

UTF_TEST_CASE_MAC(cpssPxCutThroughMaxBuffersLimitGet)
{
    GT_STATUS               st     = GT_OK;
    GT_U8                   dev;
    GT_PHYSICAL_PORT_NUM    port   = PORT_CUT_THROUGH_VALID_PHY_PORT_CNS;
    GT_U32                  buffersLimitGet;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. For all active devices go over all available physical ports. */
        st = prvUtfNextRxDmaPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        while(GT_OK == prvUtfNextRxDmaPortGet(&port, GT_TRUE))
        {
            st = cpssPxCutThroughMaxBuffersLimitGet(
                dev, port, &buffersLimitGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(
                GT_OK, st, "[cpssPxCutThroughMaxBuffersLimitGet]: %d, %d", dev, port);
        }

        /* 1.2. For all active devices go over all non available
           physical ports.
        */
        st = prvUtfNextRxDmaPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        while(GT_OK == prvUtfNextRxDmaPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssPxCutThroughMaxBuffersLimitGet(
                dev, port, &buffersLimitGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);
        st = cpssPxCutThroughMaxBuffersLimitGet(
            dev, port, &buffersLimitGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. Null pointer check*/
        port = PORT_CUT_THROUGH_VALID_PHY_PORT_CNS;
        st = cpssPxCutThroughMaxBuffersLimitGet(
            dev, port, NULL);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    port = PORT_CUT_THROUGH_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxCutThroughMaxBuffersLimitGet(
            dev, port, &buffersLimitGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxCutThroughMaxBuffersLimitGet(
        dev, port, &buffersLimitGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
 * Configuration of cpssPxCutThrough suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssPxCutThrough)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxCutThroughPortEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxCutThroughPortEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxCutThroughUpEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxCutThroughUpEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxCutThroughVlanEthertypeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxCutThroughVlanEthertypeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxCutThroughMaxBuffersLimitSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxCutThroughMaxBuffersLimitGet)
UTF_SUIT_END_TESTS_MAC(cpssPxCutThrough)



