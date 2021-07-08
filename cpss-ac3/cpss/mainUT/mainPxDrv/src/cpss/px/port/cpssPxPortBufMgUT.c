/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* cpssPxPortStatUT.c
*
* DESCRIPTION:
*       Unit tests for cpssPxPortBuf, that provides
*       CPSS implementation for Buffers management.
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*******************************************************************************/
/* includes */
/* the define of UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC must come from C files that
   already fixed the types of ports from GT_U8 !

   NOTE: must come before ANY include to H files !!!!
*/
#define UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>

#include <cpss/px/port/cpssPxPortBufMg.h>
#include <cpss/px/config/private/prvCpssPxInfo.h>
#include <cpss/px/config/cpssPxCfgInit.h>


/* defines */

/* Default valid value for port id */
#define PORT_BUF_MG_VALID_PHY_PORT_CNS  0

UTF_TEST_CASE_MAC(cpssPxPortBufMgGlobalXonLimitSet)
{
    GT_STATUS   st     = GT_OK;

    GT_U8            dev;
    GT_U32           xonLimit;
    GT_U32           xonLimitGet;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* correct, checked by Get after Set */
        xonLimit = 1;
        st = cpssPxPortBufMgGlobalXonLimitSet(dev, xonLimit);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, xonLimit);
        st = cpssPxPortBufMgGlobalXonLimitGet(dev, &xonLimitGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, xonLimitGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(xonLimit, xonLimitGet, dev);

        /* Out of range */
        xonLimit = 0x10000;
        st = cpssPxPortBufMgGlobalXonLimitSet(dev, xonLimit);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OUT_OF_RANGE, st, dev, xonLimit);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        xonLimit = 1;
        st = cpssPxPortBufMgGlobalXonLimitSet(dev, xonLimit);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, xonLimit);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    /* port == 0, enable == GT_TRUE */

    xonLimit = 1;
    st = cpssPxPortBufMgGlobalXonLimitSet(dev, xonLimit);
    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, xonLimit);
}

UTF_TEST_CASE_MAC(cpssPxPortBufMgGlobalXonLimitGet)
{
    GT_STATUS   st     = GT_OK;

    GT_U8            dev;
    GT_U32           xonLimitGet;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* correct */
        st = cpssPxPortBufMgGlobalXonLimitGet(dev, &xonLimitGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, xonLimitGet);

        /* NULL pointer */
        st = cpssPxPortBufMgGlobalXonLimitGet(dev, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortBufMgGlobalXonLimitGet(dev, &xonLimitGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    /* port == 0, enable == GT_TRUE */

    st = cpssPxPortBufMgGlobalXonLimitGet(dev, &xonLimitGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

UTF_TEST_CASE_MAC(cpssPxPortBufMgGlobalXoffLimitSet)
{
    GT_STATUS   st     = GT_OK;

    GT_U8            dev;
    GT_U32           xoffLimit;
    GT_U32           xoffLimitGet;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* correct, checked by Get after Set */
        xoffLimit = 1;
        st = cpssPxPortBufMgGlobalXoffLimitSet(dev, xoffLimit);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, xoffLimit);
        st = cpssPxPortBufMgGlobalXoffLimitGet(dev, &xoffLimitGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, xoffLimitGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(xoffLimit, xoffLimitGet, dev);

        /* Out of range */
        xoffLimit = 0x10000;
        st = cpssPxPortBufMgGlobalXoffLimitSet(dev, xoffLimit);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OUT_OF_RANGE, st, dev, xoffLimit);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        xoffLimit = 1;
        st = cpssPxPortBufMgGlobalXoffLimitSet(dev, xoffLimit);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, xoffLimit);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    /* port == 0, enable == GT_TRUE */

    xoffLimit = 1;
    st = cpssPxPortBufMgGlobalXoffLimitSet(dev, xoffLimit);
    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, xoffLimit);
}

UTF_TEST_CASE_MAC(cpssPxPortBufMgGlobalXoffLimitGet)
{
    GT_STATUS   st     = GT_OK;

    GT_U8            dev;
    GT_U32           xoffLimitGet;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* correct */
        st = cpssPxPortBufMgGlobalXoffLimitGet(dev, &xoffLimitGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, xoffLimitGet);

        /* NULL pointer */
        st = cpssPxPortBufMgGlobalXoffLimitGet(dev, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortBufMgGlobalXoffLimitGet(dev, &xoffLimitGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    /* port == 0, enable == GT_TRUE */

    st = cpssPxPortBufMgGlobalXoffLimitGet(dev, &xoffLimitGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

UTF_TEST_CASE_MAC(cpssPxPortBufMgRxProfileSet)
{
    GT_STATUS   st     = GT_OK;
    GT_U8                            dev;
    GT_PHYSICAL_PORT_NUM             port   = PORT_BUF_MG_VALID_PHY_PORT_CNS;
    CPSS_PORT_RX_FC_PROFILE_SET_ENT  profileSet;
    CPSS_PORT_RX_FC_PROFILE_SET_ENT  profileSetGet;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. For all active devices go over all available physical ports. */
        /* Call with CPSS_PORT_RX_FC_PROFILE_2_E and check all ports*/
        st = prvUtfNextRxDmaPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        profileSet = CPSS_PORT_RX_FC_PROFILE_2_E;
        while(GT_OK == prvUtfNextRxDmaPortGet(&port, GT_TRUE))
        {
            st = cpssPxPortBufMgRxProfileSet(dev, port, profileSet);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, profileSet);

            st = cpssPxPortBufMgRxProfileGet(dev, port, &profileSetGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(
                GT_OK, st, "[cpssPxPortBufMgRxProfileGet]: %d, %d", dev, port);
            UTF_VERIFY_EQUAL2_STRING_MAC(
                profileSet, profileSetGet,
                "get another profileSet value than was set: %d, %d", dev, port);
        }

        /* Call with CPSS_PORT_RX_FC_PROFILE_1_E - _8_E and check */
        /* check on first port only                             */
        st = prvUtfNextRxDmaPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        while(GT_OK == prvUtfNextRxDmaPortGet(&port, GT_TRUE))
        {
            for (profileSet = CPSS_PORT_RX_FC_PROFILE_1_E;
                  (profileSet <= CPSS_PORT_RX_FC_PROFILE_8_E);
                  profileSet ++)
            {
                st = cpssPxPortBufMgRxProfileSet(dev, port, profileSet);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, profileSet);

                st = cpssPxPortBufMgRxProfileGet(dev, port, &profileSetGet);
                UTF_VERIFY_EQUAL2_STRING_MAC(
                    GT_OK, st, "[cpssPxPortBufMgRxProfileGet]: %d, %d", dev, port);
                UTF_VERIFY_EQUAL2_STRING_MAC(
                    profileSet, profileSetGet,
                    "get another profileSet value than was set: %d, %d", dev, port);
            }
            break;
        }

        /* Call with CPSS_PORT_RX_FC_PROFILE__8_E + 1 and check */
        /* check on first port only                             */
        profileSet = CPSS_PORT_RX_FC_PROFILE_8_E + 1;
        st = prvUtfNextRxDmaPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        while(GT_OK == prvUtfNextRxDmaPortGet(&port, GT_TRUE))
        {
            st = cpssPxPortBufMgRxProfileSet(dev, port, profileSet);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, port, profileSet);
            break;
        }

        /* 1.2. For all active devices go over all non available
           physical ports.
        */
        profileSet = CPSS_PORT_RX_FC_PROFILE_1_E;
        st = prvUtfNextRxDmaPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        while(GT_OK == prvUtfNextRxDmaPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssPxPortBufMgRxProfileSet(dev, port, profileSet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        profileSet = CPSS_PORT_RX_FC_PROFILE_1_E;
        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);
        st = cpssPxPortBufMgRxProfileSet(dev, port, profileSet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    profileSet = CPSS_PORT_RX_FC_PROFILE_1_E;
    port = PORT_BUF_MG_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortBufMgRxProfileSet(dev, port, profileSet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    /* port == 0, enable == GT_TRUE */

    st = cpssPxPortBufMgRxProfileSet(dev, port, profileSet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

UTF_TEST_CASE_MAC(cpssPxPortBufMgRxProfileGet)
{
    GT_STATUS   st     = GT_OK;
    GT_U8                            dev;
    GT_PHYSICAL_PORT_NUM             port   = PORT_BUF_MG_VALID_PHY_PORT_CNS;
    CPSS_PORT_RX_FC_PROFILE_SET_ENT  profileSetGet;

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
            st = cpssPxPortBufMgRxProfileGet(dev, port, &profileSetGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(
                GT_OK, st, "[cpssPxPortBufMgRxProfileGet]: %d, %d", dev, port);
        }

        /* For first port check NULL-pointer */
        st = prvUtfNextRxDmaPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        while(GT_OK == prvUtfNextRxDmaPortGet(&port, GT_TRUE))
        {
            st = cpssPxPortBufMgRxProfileGet(dev, port, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(
                GT_BAD_PTR, st, "[cpssPxPortBufMgRxProfileGet]: %d, %d", dev, port);
            break;
        }

        /* 1.2. For all active devices go over all non available
           physical ports.
        */
        st = prvUtfNextRxDmaPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        while(GT_OK == prvUtfNextRxDmaPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssPxPortBufMgRxProfileGet(dev, port, &profileSetGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(
                GT_BAD_PARAM, st, "[cpssPxPortBufMgRxProfileGet]: %d, %d", dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);
        st = cpssPxPortBufMgRxProfileGet(dev, port, &profileSetGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(
            GT_BAD_PARAM, st, "[cpssPxPortBufMgRxProfileGet]: %d, %d", dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    port = PORT_BUF_MG_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortBufMgRxProfileGet(dev, port, &profileSetGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(
            GT_NOT_APPLICABLE_DEVICE, st, "[cpssPxPortBufMgRxProfileGet]: %d, %d", dev, port);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    /* port == 0, enable == GT_TRUE */

    st = cpssPxPortBufMgRxProfileGet(dev, port, &profileSetGet);
    UTF_VERIFY_EQUAL2_STRING_MAC(
        GT_BAD_PARAM, st, "[cpssPxPortBufMgRxProfileGet]: %d, %d", dev, port);
}

UTF_TEST_CASE_MAC(cpssPxPortBufMgProfileXonLimitSet)
{
    GT_STATUS   st     = GT_OK;

    GT_U8                            dev;
    CPSS_PORT_RX_FC_PROFILE_SET_ENT  profileSet;
    GT_U32                           xonLimit;
    GT_U32                           xonLimitGet;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for (profileSet = CPSS_PORT_RX_FC_PROFILE_1_E;
              (profileSet <= CPSS_PORT_RX_FC_PROFILE_8_E);
              profileSet ++)
        {
            /* correct, checked by Get after Set */
            xonLimit = 10 + profileSet;
            st = cpssPxPortBufMgProfileXonLimitSet(dev, profileSet, xonLimit);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, profileSet, xonLimit);
            st = cpssPxPortBufMgProfileXonLimitGet(dev, profileSet, &xonLimitGet);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, profileSet, xonLimitGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(xonLimit, xonLimitGet, dev, profileSet);
        }

        /* Bad profileSet */
        profileSet = CPSS_PORT_RX_FC_PROFILE_8_E + 1;
        xonLimit = 1;
        st = cpssPxPortBufMgProfileXonLimitSet(dev, profileSet, xonLimit);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, profileSet, xonLimit);

        /* Out of range */
        profileSet = CPSS_PORT_RX_FC_PROFILE_2_E;
        xonLimit = 0x10000;
        st = cpssPxPortBufMgProfileXonLimitSet(dev, profileSet, xonLimit);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OUT_OF_RANGE, st, dev, profileSet, xonLimit);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        profileSet = CPSS_PORT_RX_FC_PROFILE_2_E;
        xonLimit = 1;
        st = cpssPxPortBufMgProfileXonLimitSet(dev, profileSet, xonLimit);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, profileSet, xonLimit);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    /* port == 0, enable == GT_TRUE */

    profileSet = CPSS_PORT_RX_FC_PROFILE_2_E;
    xonLimit = 1;
    st = cpssPxPortBufMgProfileXonLimitSet(dev, profileSet, xonLimit);
    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, profileSet, xonLimit);
}

UTF_TEST_CASE_MAC(cpssPxPortBufMgProfileXonLimitGet)
{
    GT_STATUS   st     = GT_OK;

    GT_U8                            dev;
    CPSS_PORT_RX_FC_PROFILE_SET_ENT  profileSet;
    GT_U32                           xonLimitGet;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for (profileSet = CPSS_PORT_RX_FC_PROFILE_1_E;
              (profileSet <= CPSS_PORT_RX_FC_PROFILE_8_E);
              profileSet ++)
        {
            /* correct, checked by Get after Set */
            st = cpssPxPortBufMgProfileXonLimitGet(dev, profileSet, &xonLimitGet);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, profileSet, xonLimitGet);
        }

        /* Bad profileSet */
        profileSet = CPSS_PORT_RX_FC_PROFILE_8_E + 1;
        st = cpssPxPortBufMgProfileXonLimitGet(dev, profileSet, &xonLimitGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, profileSet);

        /* Bad pointer */
        profileSet = CPSS_PORT_RX_FC_PROFILE_2_E;
        st = cpssPxPortBufMgProfileXonLimitGet(dev, profileSet, NULL);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, profileSet);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        profileSet = CPSS_PORT_RX_FC_PROFILE_2_E;
        st = cpssPxPortBufMgProfileXonLimitGet(dev, profileSet, &xonLimitGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, profileSet);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    /* port == 0, enable == GT_TRUE */

    profileSet = CPSS_PORT_RX_FC_PROFILE_2_E;
    st = cpssPxPortBufMgProfileXonLimitGet(dev, profileSet, &xonLimitGet);
    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, profileSet);
}

UTF_TEST_CASE_MAC(cpssPxPortBufMgProfileXoffLimitSet)
{
    GT_STATUS   st     = GT_OK;

    GT_U8                            dev;
    CPSS_PORT_RX_FC_PROFILE_SET_ENT  profileSet;
    GT_U32                           xoffLimit;
    GT_U32                           xoffLimitGet;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for (profileSet = CPSS_PORT_RX_FC_PROFILE_1_E;
              (profileSet <= CPSS_PORT_RX_FC_PROFILE_8_E);
              profileSet ++)
        {
            /* correct, checked by Get after Set */
            xoffLimit = 10 + profileSet;
            st = cpssPxPortBufMgProfileXoffLimitSet(dev, profileSet, xoffLimit);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, profileSet, xoffLimit);
            st = cpssPxPortBufMgProfileXoffLimitGet(dev, profileSet, &xoffLimitGet);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, profileSet, xoffLimitGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(xoffLimit, xoffLimitGet, dev, profileSet);
        }

        /* Bad profileSet */
        profileSet = CPSS_PORT_RX_FC_PROFILE_8_E + 1;
        xoffLimit = 1;
        st = cpssPxPortBufMgProfileXoffLimitSet(dev, profileSet, xoffLimit);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, profileSet, xoffLimit);

        /* Out of range */
        profileSet = CPSS_PORT_RX_FC_PROFILE_2_E;
        xoffLimit = 0x10000;
        st = cpssPxPortBufMgProfileXoffLimitSet(dev, profileSet, xoffLimit);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OUT_OF_RANGE, st, dev, profileSet, xoffLimit);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        profileSet = CPSS_PORT_RX_FC_PROFILE_2_E;
        xoffLimit = 1;
        st = cpssPxPortBufMgProfileXoffLimitSet(dev, profileSet, xoffLimit);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, profileSet, xoffLimit);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    /* port == 0, enable == GT_TRUE */

    profileSet = CPSS_PORT_RX_FC_PROFILE_2_E;
    xoffLimit = 1;
    st = cpssPxPortBufMgProfileXoffLimitSet(dev, profileSet, xoffLimit);
    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, profileSet, xoffLimit);
}

UTF_TEST_CASE_MAC(cpssPxPortBufMgProfileXoffLimitGet)
{
    GT_STATUS   st     = GT_OK;

    GT_U8                            dev;
    CPSS_PORT_RX_FC_PROFILE_SET_ENT  profileSet;
    GT_U32                           xoffLimitGet;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for (profileSet = CPSS_PORT_RX_FC_PROFILE_1_E;
              (profileSet <= CPSS_PORT_RX_FC_PROFILE_8_E);
              profileSet ++)
        {
            /* correct, checked by Get after Set */
            st = cpssPxPortBufMgProfileXoffLimitGet(dev, profileSet, &xoffLimitGet);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, profileSet, xoffLimitGet);
        }

        /* Bad profileSet */
        profileSet = CPSS_PORT_RX_FC_PROFILE_8_E + 1;
        st = cpssPxPortBufMgProfileXoffLimitGet(dev, profileSet, &xoffLimitGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, profileSet);

        /* Bad pointer */
        profileSet = CPSS_PORT_RX_FC_PROFILE_2_E;
        st = cpssPxPortBufMgProfileXoffLimitGet(dev, profileSet, NULL);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, profileSet);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        profileSet = CPSS_PORT_RX_FC_PROFILE_2_E;
        st = cpssPxPortBufMgProfileXoffLimitGet(dev, profileSet, &xoffLimitGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, profileSet);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    /* port == 0, enable == GT_TRUE */

    profileSet = CPSS_PORT_RX_FC_PROFILE_2_E;
    st = cpssPxPortBufMgProfileXoffLimitGet(dev, profileSet, &xoffLimitGet);
    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, profileSet);
}

UTF_TEST_CASE_MAC(cpssPxPortBufMgProfileRxBufLimitSet)
{
    GT_STATUS   st     = GT_OK;

    GT_U8                            dev;
    CPSS_PORT_RX_FC_PROFILE_SET_ENT  profileSet;
    GT_U32                           rxBufLimit;
    GT_U32                           rxBufLimitGet;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for (profileSet = CPSS_PORT_RX_FC_PROFILE_1_E;
              (profileSet <= CPSS_PORT_RX_FC_PROFILE_8_E);
              profileSet ++)
        {
            /* correct, checked by Get after Set */
            rxBufLimit = 10 + profileSet;
            st = cpssPxPortBufMgProfileRxBufLimitSet(dev, profileSet, rxBufLimit);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, profileSet, rxBufLimit);
            st = cpssPxPortBufMgProfileRxBufLimitGet(dev, profileSet, &rxBufLimitGet);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, profileSet, rxBufLimitGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(rxBufLimit, rxBufLimitGet, dev, profileSet);
        }

        /* Bad profileSet */
        profileSet = CPSS_PORT_RX_FC_PROFILE_8_E + 1;
        rxBufLimit = 1;
        st = cpssPxPortBufMgProfileRxBufLimitSet(dev, profileSet, rxBufLimit);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, profileSet, rxBufLimit);

        /* Out of range */
        profileSet = CPSS_PORT_RX_FC_PROFILE_2_E;
        rxBufLimit = 0x10000;
        st = cpssPxPortBufMgProfileRxBufLimitSet(dev, profileSet, rxBufLimit);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OUT_OF_RANGE, st, dev, profileSet, rxBufLimit);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        profileSet = CPSS_PORT_RX_FC_PROFILE_2_E;
        rxBufLimit = 1;
        st = cpssPxPortBufMgProfileRxBufLimitSet(dev, profileSet, rxBufLimit);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, profileSet, rxBufLimit);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    /* port == 0, enable == GT_TRUE */

    profileSet = CPSS_PORT_RX_FC_PROFILE_2_E;
    rxBufLimit = 1;
    st = cpssPxPortBufMgProfileRxBufLimitSet(dev, profileSet, rxBufLimit);
    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, profileSet, rxBufLimit);
}

UTF_TEST_CASE_MAC(cpssPxPortBufMgProfileRxBufLimitGet)
{
    GT_STATUS   st     = GT_OK;

    GT_U8                            dev;
    CPSS_PORT_RX_FC_PROFILE_SET_ENT  profileSet;
    GT_U32                           rxBufLimitGet;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for (profileSet = CPSS_PORT_RX_FC_PROFILE_1_E;
              (profileSet <= CPSS_PORT_RX_FC_PROFILE_8_E);
              profileSet ++)
        {
            /* correct, checked by Get after Set */
            st = cpssPxPortBufMgProfileRxBufLimitGet(dev, profileSet, &rxBufLimitGet);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, profileSet, rxBufLimitGet);
        }

        /* Bad profileSet */
        profileSet = CPSS_PORT_RX_FC_PROFILE_8_E + 1;
        st = cpssPxPortBufMgProfileRxBufLimitGet(dev, profileSet, &rxBufLimitGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, profileSet);

        /* Bad pointer */
        profileSet = CPSS_PORT_RX_FC_PROFILE_2_E;
        st = cpssPxPortBufMgProfileRxBufLimitGet(dev, profileSet, NULL);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, profileSet);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        profileSet = CPSS_PORT_RX_FC_PROFILE_2_E;
        st = cpssPxPortBufMgProfileRxBufLimitGet(dev, profileSet, &rxBufLimitGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, profileSet);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    /* port == 0, enable == GT_TRUE */

    profileSet = CPSS_PORT_RX_FC_PROFILE_2_E;
    st = cpssPxPortBufMgProfileRxBufLimitGet(dev, profileSet, &rxBufLimitGet);
    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, profileSet);
}

UTF_TEST_CASE_MAC(cpssPxPortBufMgGlobalRxBufNumberGet)
{
    GT_STATUS   st     = GT_OK;

    GT_U8            dev;
    GT_U32           numOfBuffersGet;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* correct */
        st = cpssPxPortBufMgGlobalRxBufNumberGet(dev, &numOfBuffersGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, numOfBuffersGet);

        /* NULL pointer */
        st = cpssPxPortBufMgGlobalRxBufNumberGet(dev, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortBufMgGlobalRxBufNumberGet(dev, &numOfBuffersGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    /* port == 0, enable == GT_TRUE */

    st = cpssPxPortBufMgGlobalRxBufNumberGet(dev, &numOfBuffersGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

UTF_TEST_CASE_MAC(cpssPxPortBufMgRxBufNumberGet)
{
    GT_STATUS   st     = GT_OK;
    GT_U8                    dev;
    GT_PHYSICAL_PORT_NUM     port   = PORT_BUF_MG_VALID_PHY_PORT_CNS;
    GT_U32                   numOfBuffersGet;

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
            st = cpssPxPortBufMgRxBufNumberGet(dev, port, &numOfBuffersGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(
                GT_OK, st, "[cpssPxPortBufMgRxBufNumberGet]: %d, %d", dev, port);
        }

        /* For first port check NULL-pointer */
        st = prvUtfNextRxDmaPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        while(GT_OK == prvUtfNextRxDmaPortGet(&port, GT_TRUE))
        {
            st = cpssPxPortBufMgRxBufNumberGet(dev, port, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(
                GT_BAD_PTR, st, "[cpssPxPortBufMgRxBufNumberGet]: %d, %d", dev, port);
            break;
        }

        /* 1.2. For all active devices go over all non available
           physical ports.
        */
        st = prvUtfNextRxDmaPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        while(GT_OK == prvUtfNextRxDmaPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssPxPortBufMgRxBufNumberGet(dev, port, &numOfBuffersGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(
                GT_BAD_PARAM, st, "[cpssPxPortBufMgRxBufNumberGet]: %d, %d", dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);
        st = cpssPxPortBufMgRxBufNumberGet(dev, port, &numOfBuffersGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(
            GT_BAD_PARAM, st, "[cpssPxPortBufMgRxBufNumberGet]: %d, %d", dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    port = PORT_BUF_MG_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortBufMgRxBufNumberGet(dev, port, &numOfBuffersGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(
            GT_NOT_APPLICABLE_DEVICE, st, "[cpssPxPortBufMgRxBufNumberGet]: %d, %d", dev, port);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    /* port == 0, enable == GT_TRUE */

    st = cpssPxPortBufMgRxBufNumberGet(dev, port, &numOfBuffersGet);
    UTF_VERIFY_EQUAL2_STRING_MAC(
        GT_BAD_PARAM, st, "[cpssPxPortBufMgRxBufNumberGet]: %d, %d", dev, port);
}

UTF_TEST_CASE_MAC(cpssPxPortBufMgGlobalPacketNumberGet)
{
    GT_STATUS   st     = GT_OK;

    GT_U8            dev;
    GT_U32           numOfPacketsGet;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* correct */
        st = cpssPxPortBufMgGlobalPacketNumberGet(dev, &numOfPacketsGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, numOfPacketsGet);

        /* NULL pointer */
        st = cpssPxPortBufMgGlobalPacketNumberGet(dev, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortBufMgGlobalPacketNumberGet(dev, &numOfPacketsGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    /* port == 0, enable == GT_TRUE */

    st = cpssPxPortBufMgGlobalPacketNumberGet(dev, &numOfPacketsGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

UTF_TEST_CASE_MAC(cpssPxPortBufMgRxMcCntrGet)
{
    GT_STATUS   st     = GT_OK;
    GT_U8      dev;
    GT_U32     cntrIdx;
    GT_U32     cntrIdxBound;
    GT_U32     mcCounter;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = cpssPxCfgTableNumEntriesGet(
            dev, CPSS_PX_TABLE_BMA_MULTICAST_COUNTERS_E, &cntrIdxBound);
        UTF_VERIFY_EQUAL1_STRING_MAC(
            GT_OK, st, "[cpssPxCfgTableNumEntriesGet]: %d", dev);
        /* 1.1. For all active devices go over all available physical ports. */
        for (cntrIdx = 0; (cntrIdx < cntrIdxBound); cntrIdx++)
        {
            st = cpssPxPortBufMgRxMcCntrGet(dev, cntrIdx, &mcCounter);
            UTF_VERIFY_EQUAL2_STRING_MAC(
                GT_OK, st, "[cpssPxPortBufMgRxMcCntrGet]: %d, %d", dev, cntrIdx);
        }

        /* wrong cntrIdx */
        cntrIdx = cntrIdxBound;
        st = cpssPxPortBufMgRxMcCntrGet(dev, cntrIdx, &mcCounter);
        UTF_VERIFY_EQUAL2_STRING_MAC(
            GT_BAD_PARAM, st, "[cpssPxPortBufMgRxMcCntrGet]: %d, %d", dev, cntrIdx);

        /* check NULL-pointer */
        cntrIdx = 0;
        st = cpssPxPortBufMgRxMcCntrGet(dev, cntrIdx, NULL);
        UTF_VERIFY_EQUAL2_STRING_MAC(
            GT_BAD_PTR, st, "[cpssPxPortBufMgRxMcCntrGet]: %d, %d", dev, cntrIdx);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        cntrIdx = 0;
        st = cpssPxPortBufMgRxMcCntrGet(dev, cntrIdx, &mcCounter);
        UTF_VERIFY_EQUAL2_STRING_MAC(
            GT_NOT_APPLICABLE_DEVICE, st, "[cpssPxPortBufMgRxMcCntrGet]: %d, %d", dev, cntrIdx);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    cntrIdx = 0;
    st = cpssPxPortBufMgRxMcCntrGet(dev, cntrIdx, &mcCounter);
    UTF_VERIFY_EQUAL2_STRING_MAC(
        GT_BAD_PARAM, st, "[cpssPxPortBufMgRxMcCntrGet]: %d, %d", dev, cntrIdx);
}

UTF_TEST_CASE_MAC(cpssPxPortBufMgTxDmaBurstLimitEnableSet)
{
    GT_STATUS   st     = GT_OK;
    GT_U8                            dev;
    GT_PHYSICAL_PORT_NUM             port   = PORT_BUF_MG_VALID_PHY_PORT_CNS;
    GT_BOOL                          enable;
    GT_BOOL                          enableGet;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. For all active devices go over all available physical ports. */
        st = prvUtfNextTxDmaPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        enable = GT_TRUE;
        while(GT_OK == prvUtfNextTxDmaPortGet(&port, GT_TRUE))
        {
            st = cpssPxPortBufMgTxDmaBurstLimitEnableSet(dev, port, enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);

            st = cpssPxPortBufMgTxDmaBurstLimitEnableGet(dev, port, &enableGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(
                GT_OK, st, "[cpssPxPortBufMgTxDmaBurstLimitEnableGet]: %d, %d", dev, port);
            UTF_VERIFY_EQUAL2_STRING_MAC(
                enable, enableGet,
                "get another enable value than was set: %d, %d", dev, port);
        }

        /* 1.2. For all active devices go over all non available
           physical ports.
        */
        enable = GT_TRUE;
        st = prvUtfNextTxDmaPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        while(GT_OK == prvUtfNextTxDmaPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssPxPortBufMgTxDmaBurstLimitEnableSet(dev, port, enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        enable = GT_TRUE;
        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);
        st = cpssPxPortBufMgTxDmaBurstLimitEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    enable = GT_FALSE;
    port = PORT_BUF_MG_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortBufMgTxDmaBurstLimitEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    /* port == 0, enable == GT_TRUE */

    st = cpssPxPortBufMgTxDmaBurstLimitEnableSet(dev, port, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

UTF_TEST_CASE_MAC(cpssPxPortBufMgTxDmaBurstLimitEnableGet)
{
    GT_STATUS   st     = GT_OK;
    GT_U8                            dev;
    GT_PHYSICAL_PORT_NUM             port   = PORT_BUF_MG_VALID_PHY_PORT_CNS;
    GT_BOOL                          enableGet;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. For all active devices go over all available physical ports. */
        st = prvUtfNextTxDmaPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        while(GT_OK == prvUtfNextTxDmaPortGet(&port, GT_TRUE))
        {
            st = cpssPxPortBufMgTxDmaBurstLimitEnableGet(dev, port, &enableGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(
                GT_OK, st, "[cpssPxPortBufMgTxDmaBurstLimitEnableGet]: %d, %d", dev, port);
        }

        /* 1.2. For all active devices go over all non available
           physical ports.
        */
        st = prvUtfNextTxDmaPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        while(GT_OK == prvUtfNextTxDmaPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssPxPortBufMgTxDmaBurstLimitEnableGet(dev, port, &enableGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);
        st = cpssPxPortBufMgTxDmaBurstLimitEnableGet(dev, port, &enableGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    port = PORT_BUF_MG_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortBufMgTxDmaBurstLimitEnableGet(dev, port, &enableGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    /* port == 0, enable == GT_TRUE */

    st = cpssPxPortBufMgTxDmaBurstLimitEnableGet(dev, port, &enableGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

UTF_TEST_CASE_MAC(cpssPxPortBufMgTxDmaBurstLimitThresholdsSet)
{
    GT_STATUS   st     = GT_OK;
    GT_U8                            dev;
    GT_PHYSICAL_PORT_NUM             port   = PORT_BUF_MG_VALID_PHY_PORT_CNS;
    GT_U32                           almostFullThreshold;
    GT_U32                           fullThreshold;
    GT_U32                           almostFullThresholdGet;
    GT_U32                           fullThresholdGet;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. For all active devices go over all available physical ports. */
        st = prvUtfNextTxDmaPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        while(GT_OK == prvUtfNextTxDmaPortGet(&port, GT_TRUE))
        {
            almostFullThreshold = 256 + (port * 128);
            fullThreshold       = 512 + (port * 128);
            st = cpssPxPortBufMgTxDmaBurstLimitThresholdsSet(
                dev, port, almostFullThreshold, fullThreshold);
            UTF_VERIFY_EQUAL4_PARAM_MAC(
                GT_OK, st, dev, port, almostFullThreshold, fullThreshold);

            st = cpssPxPortBufMgTxDmaBurstLimitThresholdsGet(
                dev, port, &almostFullThresholdGet, &fullThresholdGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(
                GT_OK, st, "[cpssPxPortBufMgTxDmaBurstLimitThresholdsGet]: %d, %d", dev, port);
            UTF_VERIFY_EQUAL2_STRING_MAC(
                almostFullThreshold, almostFullThresholdGet,
                "get another almostFullThreshold value than was set: %d, %d", dev, port);
            UTF_VERIFY_EQUAL2_STRING_MAC(
                fullThreshold, fullThresholdGet,
                "get another fullThreshold value than was set: %d, %d", dev, port);
        }

        /* 1.2. For all active devices go over all non available
           physical ports.
        */
        st = prvUtfNextTxDmaPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        while(GT_OK == prvUtfNextTxDmaPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            almostFullThreshold = 256 + (port * 128);
            fullThreshold       = 512 + (port * 128);
            st = cpssPxPortBufMgTxDmaBurstLimitThresholdsSet(
                dev, port, almostFullThreshold, fullThreshold);
            UTF_VERIFY_EQUAL4_PARAM_MAC(
                GT_BAD_PARAM, st, dev, port, almostFullThreshold, fullThreshold);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);
        almostFullThreshold = 256 + (port * 128);
        fullThreshold       = 512 + (port * 128);
        st = cpssPxPortBufMgTxDmaBurstLimitThresholdsSet(
            dev, port, almostFullThreshold, fullThreshold);
        UTF_VERIFY_EQUAL4_PARAM_MAC(
            GT_BAD_PARAM, st, dev, port, almostFullThreshold, fullThreshold);

        /* 1.5 - bad almostFullThreshold, fullThreshold */
        st = prvUtfNextTxDmaPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        while(GT_OK == prvUtfNextTxDmaPortGet(&port, GT_TRUE))
        {
            /* bad almostFullThreshold */
            almostFullThreshold = 0x800000;
            fullThreshold       = 0;
            st = cpssPxPortBufMgTxDmaBurstLimitThresholdsSet(
                dev, port, almostFullThreshold, fullThreshold);
            UTF_VERIFY_EQUAL4_PARAM_MAC(
                GT_BAD_PARAM, st, dev, port, almostFullThreshold, fullThreshold);

            /* bad fullThreshold */
            almostFullThreshold = 0;
            fullThreshold       = 0x800000;
            st = cpssPxPortBufMgTxDmaBurstLimitThresholdsSet(
                dev, port, almostFullThreshold, fullThreshold);
            UTF_VERIFY_EQUAL4_PARAM_MAC(
                GT_BAD_PARAM, st, dev, port, almostFullThreshold, fullThreshold);
            /* for first port only */
            break;
        }
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    port = PORT_BUF_MG_VALID_PHY_PORT_CNS;
    almostFullThreshold = 0;
    fullThreshold       = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortBufMgTxDmaBurstLimitThresholdsSet(
            dev, port, almostFullThreshold, fullThreshold);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    /* port == 0, enable == GT_TRUE */

    st = cpssPxPortBufMgTxDmaBurstLimitThresholdsSet(
        dev, port, almostFullThreshold, fullThreshold);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

UTF_TEST_CASE_MAC(cpssPxPortBufMgTxDmaBurstLimitThresholdsGet)
{
    GT_STATUS   st     = GT_OK;
    GT_U8                            dev;
    GT_PHYSICAL_PORT_NUM             port   = PORT_BUF_MG_VALID_PHY_PORT_CNS;
    GT_U32                           almostFullThresholdGet;
    GT_U32                           fullThresholdGet;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. For all active devices go over all available physical ports. */
        st = prvUtfNextTxDmaPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        while(GT_OK == prvUtfNextTxDmaPortGet(&port, GT_TRUE))
        {
            st = cpssPxPortBufMgTxDmaBurstLimitThresholdsGet(
                dev, port, &almostFullThresholdGet, &fullThresholdGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(
                GT_OK, st, "[cpssPxPortBufMgTxDmaBurstLimitThresholdsGet]: %d, %d", dev, port);
        }

        /* 1.2. For all active devices go over all non available
           physical ports.
        */
        st = prvUtfNextTxDmaPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        while(GT_OK == prvUtfNextTxDmaPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssPxPortBufMgTxDmaBurstLimitThresholdsGet(
                dev, port, &almostFullThresholdGet, &fullThresholdGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);
        st = cpssPxPortBufMgTxDmaBurstLimitThresholdsGet(
            dev, port, &almostFullThresholdGet, &fullThresholdGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.5 - NULL pointers */
        st = prvUtfNextTxDmaPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        while(GT_OK == prvUtfNextTxDmaPortGet(&port, GT_TRUE))
        {
            /* bad almostFullThreshold */
            st = cpssPxPortBufMgTxDmaBurstLimitThresholdsGet(
                dev, port, NULL, &fullThresholdGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, port);

            /* bad fullThreshold */
            st = cpssPxPortBufMgTxDmaBurstLimitThresholdsGet(
                dev, port, &almostFullThresholdGet, NULL);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, port);
            /* for first port only */
            break;
        }
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    port = PORT_BUF_MG_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortBufMgTxDmaBurstLimitThresholdsGet(
            dev, port, &almostFullThresholdGet, &fullThresholdGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    /* port == 0, enable == GT_TRUE */

    st = cpssPxPortBufMgTxDmaBurstLimitThresholdsGet(
        dev, port, &almostFullThresholdGet, &fullThresholdGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
 * Configuration of cpssPxPortStat suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssPxPortBufMg)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortBufMgGlobalXonLimitSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortBufMgGlobalXonLimitGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortBufMgGlobalXoffLimitSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortBufMgGlobalXoffLimitGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortBufMgRxProfileSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortBufMgRxProfileGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortBufMgProfileXonLimitSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortBufMgProfileXonLimitGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortBufMgProfileXoffLimitSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortBufMgProfileXoffLimitGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortBufMgProfileRxBufLimitSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortBufMgProfileRxBufLimitGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortBufMgGlobalRxBufNumberGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortBufMgRxBufNumberGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortBufMgGlobalPacketNumberGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortBufMgRxMcCntrGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortBufMgTxDmaBurstLimitEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortBufMgTxDmaBurstLimitEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortBufMgTxDmaBurstLimitThresholdsSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortBufMgTxDmaBurstLimitThresholdsGet)
UTF_SUIT_END_TESTS_MAC(cpssPxPortBufMg)


