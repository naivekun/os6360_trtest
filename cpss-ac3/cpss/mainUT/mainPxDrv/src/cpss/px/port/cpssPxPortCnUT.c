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
* @file cpssPxPortCnUT.c
*
* @brief Unit tests for cpssPxPortCn, that provides
* CPSS implementation for Port Congestion Notification API.
*
* @version   1
********************************************************************************
*/
/* the define of UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC must come from C files that
   already fixed the types of ports from GT_U8 !

   NOTE: must come before ANY include to H files !!!!
*/
#define UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC

#include <cpss/px/config/private/prvCpssPxInfo.h>
#include <cpss/px/port/cpssPxPortCn.h>
#include <cpss/px/port/private/prvCpssPxPort.h>
#include <cpss/px/cos/private/prvCpssPxCoS.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <common/tgfCommon.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>


/* defines */

/* Default valid value for port id */
#define PORT_CN_VALID_PHY_PORT_CNS  0

/* Default valid value for dev id */
#define PORT_CN_VALID_DEV_CNS       0

/* Invalid timer value */
#define PORT_CN_INVALID_TIMER_CNS   (0xFFFF+1)

/* Invalid portSpeedIndex */
#define PORT_CN_INVALID_PORTSPEEDINDEX_CNS 8

#define QCN_SAMPLE_INTERVAL_MAX_CNS     0xFFFFF
/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssPxPortCnFcEnableSet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (Pipe)
    1.1.1. Call with state [GT_FALSE and GT_TRUE].
    Expected: GT_OK.
    1.1.2. Call cpssPxPortCnFcEnableGet.
    Expected: GT_OK and the same enable.
*/

    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM  port   = PORT_CN_VALID_PHY_PORT_CNS;
    GT_BOOL     enable = GT_FALSE;
    GT_BOOL     state  = GT_FALSE;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            /*
               1.1.1. Call with state [GT_FALSE and GT_TRUE].
               Expected: GT_OK.
            */

            /* Call function with enable = GT_FALSE] */
            enable = GT_FALSE;

            st = cpssPxPortCnFcEnableSet(dev, port, enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);

            /*
               1.1.2. Call cpssPxPortCnFcEnableGet.
               Expected: GT_OK and the same enable.
            */
            st = cpssPxPortCnFcEnableGet(dev, port, &state);

            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                            "[cpssPxPortCnFcEnableGet]: %d, %d", dev, port);
            UTF_VERIFY_EQUAL2_STRING_MAC(enable, state,
                            "get another enable value than was set: %d, %d", dev, port);

            /*
               1.1.1. Call with state [GT_FALSE and GT_TRUE].
               Expected: GT_OK.
            */

            /* Call function with enable = GT_TRUE] */
            enable = GT_TRUE;

            st = cpssPxPortCnFcEnableSet(dev, port, enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);

            /*
               1.1.2. Call cpssPxPortCnFcEnableGet.
               Expected: GT_OK and the same enable.
            */
            st = cpssPxPortCnFcEnableGet(dev, port, &state);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                          "[cpssPxPortCnFcEnableGet]: %d, %d", dev, port);
            UTF_VERIFY_EQUAL2_STRING_MAC(enable, state,
                          "get another enable value than was set: %d, %d", dev, port);
        }

        /*
           1.2. For all active devices go over all non available
           physical ports.
        */
        enable = GT_TRUE;

        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            /* enable == GT_TRUE    */
            st = cpssPxPortCnFcEnableSet(dev, port, enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        /* enable == GT_TRUE  */
        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssPxPortCnFcEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                     */
        /* enable == GT_TRUE  */
        port = PRV_CPSS_PX_CPU_DMA_NUM_CNS;

        st = cpssPxPortCnFcEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    enable = GT_TRUE;
    port = PORT_CN_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortCnFcEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    /* port == 0, enable == GT_TRUE */

    st = cpssPxPortCnFcEnableSet(dev, port, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortCnFcEnableGet
(
    IN   GT_U8     devNum,
    IN   GT_U8     portNum,
    OUT  GT_BOOL   *statePtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPortCnFcEnableGet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (Pipe)
    1.1.1. Call with non-null statePtr.
    Expected: GT_OK.
    1.1.2. Call with statePtr [NULL].
    Expected: GT_BAD_PTR
*/

    GT_STATUS   st    = GT_OK;
    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM  port  = PORT_CN_VALID_PHY_PORT_CNS;
    GT_BOOL     state = GT_FALSE;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            /* 1.1.1. Call with non-null statePtr.
               Expected: GT_OK.
            */
            st = cpssPxPortCnFcEnableGet(dev, port, &state);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* 1.1.2. Call with statePtr [NULL].
               Expected: GT_BAD_PTR
            */
            st = cpssPxPortCnFcEnableGet(dev, port, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, NULL", dev, port);
        }

        /* 1.2. For all active devices go over all non available
           physical ports.
        */
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_FALSE))
        {
            st = cpssPxPortCnFcEnableGet(dev, port, &state);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssPxPortCnFcEnableGet(dev, port, &state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = PRV_CPSS_PX_CPU_DMA_NUM_CNS;

        st = cpssPxPortCnFcEnableGet(dev, port, &state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    port = PORT_CN_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortCnFcEnableGet(dev, port, &state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortCnFcEnableGet(dev, port, &state);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortCnFcTimerSet
(
    IN GT_U8      devNum,
    IN GT_U8      portNum,
    IN GT_U32     portSpeedIndex,
    IN GT_U32     timer
)
*/
UTF_TEST_CASE_MAC(cpssPxPortCnFcTimerSet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (Pipe)
    1.1.1. Call with portSpeedIndex [0 / 7], timer[0 / 0xFFFF].
    Expected: GT_OK.
    1.1.2. Call cpssPxPortCnFcTimerGet with the same params.
    Expected: GT_OK and the same enable.
    1.1.3. Call with wrong portSpeedIndex [PORT_CN_INVALID_PORTSPEEDINDEX_CNS]
    Expected: GT_BAD_PARAM.
    1.1.4. Call with wrong timer [PORT_CN_INVALID_TIMER_CNS]
    Expected: GT_OUT_OF_RANGE.
*/

    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM port   = PORT_CN_VALID_PHY_PORT_CNS;
    GT_U32      portSpeedIndex = 0;
    GT_U32      timer = 0;
    GT_U32      timerGet = 0;

    /* there is no PFC/QCN in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            /*
               1.1.1. Call with portSpeedIndex [0 / 7], timer[0 / 0xFFFF].
               Expected: GT_OK.
            */

            /*call with portSpeedIndex [0] and timer[0]*/
            portSpeedIndex = 0;
            timer = 0;

            st = cpssPxPortCnFcTimerSet(dev, port, portSpeedIndex, timer);
            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, port, portSpeedIndex, timer);

            /*
               1.1.2. Call cpssPxPortCnFcTimerGet with the same params.
               Expected: GT_OK and the same timer.
            */
            st = cpssPxPortCnFcTimerGet(dev, port, portSpeedIndex, &timerGet);

            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                            "[cpssPxPortCnFcTimerGet]: %d, %d", dev, port);
            UTF_VERIFY_EQUAL2_STRING_MAC(timer, timerGet,
                      "get another timer value than was set: %d, %d", dev, port);

            /*
               1.1.1. Call with portSpeedIndex [0 / 7], timer[0 / 0xFFFF].
               Expected: GT_OK.
            */

            /*call with portSpeedIndex [7] and timer[0xFFFF]*/
            portSpeedIndex = 7;
            timer = 0xFFFF;

            st = cpssPxPortCnFcTimerSet(dev, port, portSpeedIndex, timer);
            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, port, portSpeedIndex, timer);

            /*
               1.1.2. Call cpssPxPortCnFcTimerGet with the same params.
               Expected: GT_OK and the same timer.
            */
            st = cpssPxPortCnFcTimerGet(dev, port, portSpeedIndex, &timerGet);

            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                            "[cpssPxPortCnFcTimerGet]: %d, %d", dev, port);
            UTF_VERIFY_EQUAL2_STRING_MAC(timer, timerGet,
                      "get another timer value than was set: %d, %d", dev, port);

            /*
               1.1.3. Call with wrong portSpeedIndex [PORT_CN_INVALID_PORTSPEEDINDEX_CNS].
               Expected: GT_BAD_PARAM.
            */
            st = cpssPxPortCnFcTimerSet(dev, port, PORT_CN_INVALID_PORTSPEEDINDEX_CNS, timer);
            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_BAD_PARAM, st, dev, port, PORT_CN_INVALID_PORTSPEEDINDEX_CNS, timer);

            /*
               1.1.4. Call with wrong timer [PORT_CN_INVALID_TIMER_CNS]
               Expected: GT_OUT_OF_RANGE.
            */
            timer = PORT_CN_INVALID_TIMER_CNS;

            st = cpssPxPortCnFcTimerSet(dev, port, portSpeedIndex, timer);
            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OUT_OF_RANGE, st, dev, port, portSpeedIndex, timer);
            /* Restore valid timer value */
            timer = 0xFFFF;
        }

        /*
           1.2. For all active devices go over all non available
           physical ports.
        */

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssPxPortCnFcTimerSet(dev, port, portSpeedIndex, timer);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssPxPortCnFcTimerSet(dev, port, portSpeedIndex, timer);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                     */
        port = PRV_CPSS_PX_CPU_DMA_NUM_CNS;

        st = cpssPxPortCnFcTimerSet(dev, port, portSpeedIndex, timer);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_BAD_PARAM, st, dev, port, portSpeedIndex, timer);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortCnFcTimerSet(dev, port, portSpeedIndex, timer);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    /* port == 0, enable == GT_TRUE */

    st = cpssPxPortCnFcTimerSet(dev, port, portSpeedIndex, timer);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortCnFcTimerGet
(
    IN  GT_U8    devNum,
    IN  GT_U8    portNum,
    IN  GT_U32   portSpeedIndex,
    OUT GT_U32   *timerPtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPortCnFcTimerGet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (Pipe)
    1.1.1. Call with portSpeedIndex [0 / 7] and  non-null timerPtr.
    Expected: GT_OK.
    1.1.2. Call with wrong  portSpeedIndex [PORT_CN_INVALID_PORTSPEEDINDEX_CNS]
           and  non-null timerPtr.
    Expected: GT_BAD_PARAM.
    1.1.3. Call with wrong timerPtr [NULL].
    Expected: GT_BAD_PTR.
*/

    GT_STATUS   st    = GT_OK;

    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM  port  = PORT_CN_VALID_PHY_PORT_CNS;
    GT_U32      portSpeedIndex = 0;
    GT_U32      timerGet = 0;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            /*
               1.1.1. Call with portSpeedIndex [0 / 7] and  non-null timerPtr.
               Expected: GT_OK.
            */

            portSpeedIndex = 0;

            st = cpssPxPortCnFcTimerGet(dev, port, portSpeedIndex, &timerGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*
               1.1.1. Call with portSpeedIndex [0 / 7] and  non-null timerPtr.
               Expected: GT_OK.
            */

            portSpeedIndex = 7;
            st = cpssPxPortCnFcTimerGet(dev, port, portSpeedIndex, &timerGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*
                1.1.2. Call with wrong  portSpeedIndex [PORT_CN_INVALID_PORTSPEEDINDEX_CNS]
                       and  non-null timerPtr.
                Expected: GT_BAD_PARAM.
            */
            st = cpssPxPortCnFcTimerGet(dev, port, PORT_CN_INVALID_PORTSPEEDINDEX_CNS, &timerGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

            /*
                1.1.3. Call with wrong timerPtr [NULL].
                Expected: GT_BAD_PTR
            */
            st = cpssPxPortCnFcTimerGet(dev, port, portSpeedIndex,  NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, NULL", dev, port);
        }

        /* 1.2. For all active devices go over all non available
           physical ports.
        */
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            st = cpssPxPortCnFcTimerGet(dev, port, portSpeedIndex, &timerGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssPxPortCnFcTimerGet(dev, port, portSpeedIndex, &timerGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = PRV_CPSS_PX_CPU_DMA_NUM_CNS;

        st = cpssPxPortCnFcTimerGet(dev, port, portSpeedIndex, &timerGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = PORT_CN_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortCnFcTimerGet(dev, port, portSpeedIndex, &timerGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortCnFcTimerGet(dev, port, portSpeedIndex, &timerGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortCnModeEnableSet
(
    IN  GT_U8     devNum,
    IN  GT_BOOL   enable
)
*/
UTF_TEST_CASE_MAC(cpssPxPortCnModeEnableSet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (Pipe)
    1.1. Call with state
            GT_FALSE, GT_TRUE
    Expected: GT_OK.
    1.2. Call cpssPxPortCnModeEnableGet.
    Expected: GT_OK and the same enable.
*/

    GT_STATUS   st     = GT_OK;
    GT_U8       dev;
    GT_BOOL enable = GT_FALSE;
    GT_BOOL state  = GT_FALSE;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
           1.1. Call with state GT_FALSE.
           Expected: GT_OK.
        */

        enable = GT_FALSE;

        st = cpssPxPortCnModeEnableSet(dev, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, enable);

        /*
           1.2. Call cpssPxPortCnModeEnableGet.
           Expected: GT_OK and the same enable.
        */
        st = cpssPxPortCnModeEnableGet(dev, &state);

        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                        "[cpssPxPortCnModeEnableGet]: %d, %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(enable, state,
                        "get another enable value than was set: %d, %d", dev);

        /*
           1.1. Call with state GT_TRUE.
           Expected: GT_OK.
        */

        enable = GT_TRUE;

        st = cpssPxPortCnModeEnableSet(dev, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, enable);

        /*
           1.2. Call cpssPxPortCnModeEnableGet.
           Expected: GT_OK and the same enable.
        */
        st = cpssPxPortCnModeEnableGet(dev, &state);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                      "[cpssPxPortCnModeEnableGet]: %d, %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(enable, state,
                      "get another enable value than was set: %d, %d", dev);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    enable = GT_TRUE;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortCnModeEnableSet(dev, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortCnModeEnableSet(dev, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortCnModeEnableGet
(
    IN   GT_U8     devNum,
    OUT  GT_BOOL   *statePtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPortCnModeEnableGet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (Pipe)
    1.1. Call with non-null statePtr.
    Expected: GT_OK.
    1.2. Call with statePtr [NULL].
    Expected: GT_BAD_PTR
*/

    GT_STATUS   st    = GT_OK;

    GT_U8       dev;
    GT_BOOL state = GT_FALSE;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Call with non-null statePtr.
           Expected: GT_OK.
        */
        st = cpssPxPortCnModeEnableGet(dev, &state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* 1.2. Call with statePtr [NULL].
           Expected: GT_BAD_PTR
        */
        st = cpssPxPortCnModeEnableGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, %d, NULL", dev);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortCnModeEnableGet(dev, &state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortCnModeEnableGet(dev, &state);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortCnFbCalcConfigSet
(
    IN GT_U8    devNum,
    IN CPSS_PORT_CN_FB_CALCULATION_CONFIG_STC *fbCalcCfgPtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPortCnFbCalcConfigSet)
{
/*
    ITERATE_DEVICES (Pipe)
    1.1. Call with all correct values
                                   fbCalcCfg.wExp[0 / 5];
                                   fbCalcCfg.fbLsb[0 / 0x1A];
                                   fbCalcCfg.deltaEnable[GT_TRUE / GT_FALSE];
                                   fbCalcCfg.fbMin[0 / 0xFFFFA];
                                   fbCalcCfg.fbMax[0 / 0xFFFFA];
    Expected: GT_OK.
    1.2. Call cpssPxPortCnFbCalcConfigGet with not-NULL fbCalcCfgPtr.
    Expected: GT_OK and the same fbCalcCfgPtr  as was set.
    1.3. Call with wrong wExp (out of range):
                                   fbCalcCfg.wExp[8];
                                   fbCalcCfg.fbLsb[0];
                                   fbCalcCfg.deltaEnable[GT_TRUE];
                                   fbCalcCfg.fbMin[0];
                                   fbCalcCfg.fbMax[0];
    Expected: GT_OUT_OF_RANGE.
    1.4. Call with wrong fbLsb (out of range):
                                   fbCalcCfg.wExp[0];
                                   fbCalcCfg.fbLsb[0x20];
                                   fbCalcCfg.deltaEnable[GT_TRUE];
                                   fbCalcCfg.fbMin[0];
                                   fbCalcCfg.fbMax[0];
    Expected: GT_OUT_OF_RANGE.
    1.5. Call with wrong fbMin (out of range):
                                   fbCalcCfg.wExp[0];
                                   fbCalcCfg.fbLsb[0];
                                   fbCalcCfg.deltaEnable[GT_TRUE];
                                   fbCalcCfg.fbMin[0x100000];
                                   fbCalcCfg.fbMax[0];
    Expected: GT_OUT_OF_RANGE.
    1.6. Call with wrong fbMax (out of range):
                                   fbCalcCfg.wExp[0];
                                   fbCalcCfg.fbLsb[0];
                                   fbCalcCfg.deltaEnable[GT_TRUE];
                                   fbCalcCfg.fbMin[0];
                                   fbCalcCfg.fbMax[0x100000];
    Expected: GT_OUT_OF_RANGE.
    1.7. Call with wrong fbCalcCfgPtr [NULL].
    Expected: GT_BAD_PTR.
*/

    GT_STATUS   st  = GT_OK;
    GT_U8       dev;

    CPSS_PX_PORT_CN_FB_CALCULATION_CONFIG_STC fbCalcCfg;
    CPSS_PX_PORT_CN_FB_CALCULATION_CONFIG_STC fbCalcCfgGet;

    GT_U32 fbMinMaxNumOfBits; /* number of bits in fbMin and fbMax fields */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with all correct values
                                           fbCalcCfg.wExp[0];
                                           fbCalcCfg.fbLsb[0];
                                           fbCalcCfg.deltaEnable[GT_TRUE];
                                           fbCalcCfg.fbMin[0];
                                           fbCalcCfg.fbMax[0];
            Expected: GT_OK.
        */
        fbCalcCfg.wExp = 0;
        fbCalcCfg.fbLsb = 0;
        fbCalcCfg.deltaEnable = GT_TRUE;
        fbCalcCfg.fbMin = 0;
        fbCalcCfg.fbMax = 0;

        st = cpssPxPortCnFbCalcConfigSet(dev, &fbCalcCfg);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        if(GT_OK == st)
        {
            /*
                1.2. Call cpssPxPortCnFbCalcConfigGet with not-NULL fbCalcCfgPtr.
                Expected: GT_OK and the same fbCalcCfg values.
            */
            st = cpssPxPortCnFbCalcConfigGet(dev, &fbCalcCfgGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssPxPortCnFbCalcConfigGet: %d", dev);

            UTF_VERIFY_EQUAL1_STRING_MAC(fbCalcCfg.wExp, fbCalcCfgGet.wExp,
                             "got another fbCalcCfg.wExp then was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(fbCalcCfg.fbLsb, fbCalcCfgGet.fbLsb,
                             "got another fbCalcCfg.fbLsb then was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(fbCalcCfg.deltaEnable, fbCalcCfgGet.deltaEnable,
                             "got another fbCalcCfg.deltaEnable then was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(fbCalcCfg.fbMin, fbCalcCfgGet.fbMin,
                             "got another fbCalcCfg.fbMin then was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(fbCalcCfg.fbMax, fbCalcCfgGet.fbMax,
                             "got another fbCalcCfg.fbMax then was set: %d", dev);
        }

        fbMinMaxNumOfBits = 24;

        fbCalcCfg.fbMin = BIT_MASK_MAC(fbMinMaxNumOfBits);
        fbCalcCfg.fbMax = BIT_MASK_MAC(fbMinMaxNumOfBits);

        st = cpssPxPortCnFbCalcConfigSet(dev, &fbCalcCfg);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        if(GT_OK == st)
        {
            /*
                1.2. Call cpssPxPortCnFbCalcConfigGet with not-NULL fbCalcCfgPtr.
                Expected: GT_OK and the same fbCalcCfg values.
            */
            st = cpssPxPortCnFbCalcConfigGet(dev, &fbCalcCfgGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssPxPortCnFbCalcConfigGet: %d", dev);

            UTF_VERIFY_EQUAL1_STRING_MAC(fbCalcCfg.wExp, fbCalcCfgGet.wExp,
                             "got another fbCalcCfg.wExp then was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(fbCalcCfg.fbLsb, fbCalcCfgGet.fbLsb,
                             "got another fbCalcCfg.fbLsb then was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(fbCalcCfg.deltaEnable, fbCalcCfgGet.deltaEnable,
                             "got another fbCalcCfg.deltaEnable then was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(fbCalcCfg.fbMin, fbCalcCfgGet.fbMin,
                             "got another fbCalcCfg.fbMin then was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(fbCalcCfg.fbMax, fbCalcCfgGet.fbMax,
                             "got another fbCalcCfg.fbMax then was set: %d", dev);
        }

        /*
            1.1. Call with all correct values
                                           fbCalcCfg.wExp[5];
                                           fbCalcCfg.fbLsb[0x1A];
                                           fbCalcCfg.deltaEnable[GT_FALSE];
                                           fbCalcCfg.fbMin[0xFFFFA];
                                           fbCalcCfg.fbMax[0xFFFFA];
            Expected: GT_OK.
        */
        fbCalcCfg.wExp = 5;
        fbCalcCfg.fbLsb = 0x1A;
        fbCalcCfg.deltaEnable = GT_FALSE;
        fbCalcCfg.fbMin = 0xFFFFA;
        fbCalcCfg.fbMax = 0xFFFFA;

        st = cpssPxPortCnFbCalcConfigSet(dev, &fbCalcCfg);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call cpssPxPortCnFbCalcConfigGet with not-NULL fbCalcCfgPtr.
            Expected: GT_OK and the same fbCalcCfg values.
        */
        st = cpssPxPortCnFbCalcConfigGet(dev, &fbCalcCfgGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssPxPortCnFbCalcConfigGet: %d", dev);

        UTF_VERIFY_EQUAL1_STRING_MAC(fbCalcCfg.wExp, fbCalcCfgGet.wExp,
                         "got another fbCalcCfg.wExp then was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(fbCalcCfg.fbLsb, fbCalcCfgGet.fbLsb,
                         "got another fbCalcCfg.fbLsb then was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(fbCalcCfg.deltaEnable, fbCalcCfgGet.deltaEnable,
                         "got another fbCalcCfg.deltaEnable then was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(fbCalcCfg.fbMin, fbCalcCfgGet.fbMin,
                         "got another fbCalcCfg.fbMin then was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(fbCalcCfg.fbMax, fbCalcCfgGet.fbMax,
                         "got another fbCalcCfg.fbMax then was set: %d", dev);

        /*
            1.3. Call with wrong wExp (out of range):
                                           fbCalcCfg.wExp[8];
                                           fbCalcCfg.fbLsb[0];
                                           fbCalcCfg.deltaEnable[GT_TRUE];
                                           fbCalcCfg.fbMin[0];
                                           fbCalcCfg.fbMax[0];
            Expected: GT_OUT_OF_RANGE.
        */
        fbCalcCfg.wExp = 8;
        fbCalcCfg.fbLsb = 0;
        fbCalcCfg.deltaEnable = GT_TRUE;
        fbCalcCfg.fbMin = 0;
        fbCalcCfg.fbMax = 0;

        st = cpssPxPortCnFbCalcConfigSet(dev, &fbCalcCfg);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, dev);

        /*
            1.4. Call with wrong fbLsb (out of range):
                                               fbCalcCfg.wExp[0];
                                               fbCalcCfg.fbLsb[0x20];
                                               fbCalcCfg.deltaEnable[GT_TRUE];
                                               fbCalcCfg.fbMin[0];
                                               fbCalcCfg.fbMax[0];
            Expected: GT_OUT_OF_RANGE.
        */
        fbCalcCfg.wExp = 0;
        fbCalcCfg.fbLsb = 0x20;
        fbCalcCfg.deltaEnable = GT_TRUE;
        fbCalcCfg.fbMin = 0;
        fbCalcCfg.fbMax = 0;

        st = cpssPxPortCnFbCalcConfigSet(dev, &fbCalcCfg);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, dev);

        /*
            1.5. Call with wrong fbMin (out of range):
                                           fbCalcCfg.wExp[0];
                                           fbCalcCfg.fbLsb[0];
                                           fbCalcCfg.deltaEnable[GT_TRUE];
                                           fbCalcCfg.fbMin[out of range];
                                           fbCalcCfg.fbMax[0];
            Expected: GT_OUT_OF_RANGE.
        */
        fbCalcCfg.wExp = 0;
        fbCalcCfg.fbLsb = 0;
        fbCalcCfg.deltaEnable = GT_TRUE;
        fbCalcCfg.fbMin = BIT_MASK_MAC(fbMinMaxNumOfBits) + 1;
        fbCalcCfg.fbMax = 0;


        st = cpssPxPortCnFbCalcConfigSet(dev, &fbCalcCfg);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, dev);

        fbCalcCfg.fbMin = 0;

        /*
            1.6. Call with wrong fbMax (out of range):
                                           fbCalcCfg.wExp[0];
                                           fbCalcCfg.fbLsb[0];
                                           fbCalcCfg.deltaEnable[GT_TRUE];
                                           fbCalcCfg.fbMin[0];
                                           fbCalcCfg.fbMax[out of range];
            Expected: GT_OUT_OF_RANGE.
        */
        fbCalcCfg.wExp = 0;
        fbCalcCfg.fbLsb = 0;
        fbCalcCfg.deltaEnable = GT_TRUE;
        fbCalcCfg.fbMin = 0;
        fbCalcCfg.fbMax = BIT_MASK_MAC(fbMinMaxNumOfBits) + 1;

        st = cpssPxPortCnFbCalcConfigSet(dev, &fbCalcCfg);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, dev);

        fbCalcCfg.fbMax = 0;

        /*
            1.7. Call with wrong fbCalcCfgPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssPxPortCnFbCalcConfigSet(dev, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    fbCalcCfg.wExp = 0;
    fbCalcCfg.fbLsb = 0;
    fbCalcCfg.deltaEnable = GT_TRUE;
    fbCalcCfg.fbMin = 0;
    fbCalcCfg.fbMax = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortCnFbCalcConfigSet(dev, &fbCalcCfg);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortCnFbCalcConfigSet(dev, &fbCalcCfg);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortCnFbCalcConfigGet
(
    IN  GT_U8    devNum,
    OUT CPSS_PX_PORT_CN_FB_CALCULATION_CONFIG_STC *fbCalcCfgPtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPortCnFbCalcConfigGet)
{
/*
    ITERATE_DEVICES (Pipe)
    1.1. Call with not null fbCalcCfgPtr.
    Expected: GT_OK.
    1.2. Call with wrong fbCalcCfgPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;

    CPSS_PX_PORT_CN_FB_CALCULATION_CONFIG_STC fbCalcCfg;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with not null fbCalcCfgPtr.
            Expected: GT_OK.
        */
        st = cpssPxPortCnFbCalcConfigGet(dev, &fbCalcCfg);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with wrong fbCalcCfgPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssPxPortCnFbCalcConfigGet(dev, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortCnFbCalcConfigGet(dev, &fbCalcCfg);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortCnFbCalcConfigGet(dev, &fbCalcCfg);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortCnMessageGenerationConfigSet
(
    IN GT_U8    devNum,
    IN CPSS_PX_PORT_CNM_GENERATION_CONFIG_STC *cnmGenerationCfgPtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPortCnMessageGenerationConfigSet)
{
/*
    ITERATE_DEVICES (Pipe)
    1.1. Call with all correct values
                       cnmGenerationCfg.packetType [0 / 31]
                       cnmGenerationCfg.priority [0 / 7]
                       cnmGenerationCfg.dropPrecedence [CPSS_DP_GREEN_E / CPSS_DP_YELLOW_E / CPSS_DP_RED_E]
                       cnmGenerationCfg.tc4pfc [0 / 7]
                       cnmGenerationCfg.localGeneratedPacketTypeAssignmentEnable [GT_FALSE / GT_TRUE]
                       cnmGenerationCfg.localGeneratedPacketType [0 / 31]
    Expected: GT_OK.
    1.2. Call cpssPxPortCnMessageGenerationConfigGet with
         not-NULL cnmGenerationCfgPtr.
    Expected: GT_OK and the same identEnaalcCfg as was set.
    1.3. Call with wrong cnmGenerationCfgPtr [NULL].
    Expected: GT_BAD_PTR.
    1.4. Call with wrong cnmGenerationCfg.packetType [32]
    Expected: NOT GT_OK.
    1.5. Call with wrong cnmGenerationCfg.priority [8]
    Expected: NOT GT_OK.
    1.6. Call with wrong cnmGenerationCfg.dropPrecedence [CPSS_DP_LAST_E]
    Expected: NOT GT_OK.
    1.7. Call with wrong cnmGenerationCfg.tc4pfc[8]
    Expected: NOT GT_OK.
    1.8. Call with wrong cnmGenerationCfg.localGeneratedPacketType  [32]
    Expected: NOT GT_OK.
*/

    GT_STATUS   st  = GT_OK;
    GT_U8       dev;

    CPSS_PX_PORT_CNM_GENERATION_CONFIG_STC cnmGenerationCfg;
    CPSS_PX_PORT_CNM_GENERATION_CONFIG_STC cnmGenerationCfgGet;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    cpssOsBzero((GT_VOID*) &cnmGenerationCfg, sizeof(cnmGenerationCfg));
    cpssOsBzero((GT_VOID*) &cnmGenerationCfgGet, sizeof(cnmGenerationCfgGet));

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
    1.1. Call with all correct values
                       cnmGenerationCfg.packetType [0 / 31]
                       cnmGenerationCfg.priority [0 / 7]
                       cnmGenerationCfg.dropPrecedence [CPSS_DP_GREEN_E / CPSS_DP_YELLOW_E / CPSS_DP_RED_E]
                       cnmGenerationCfg.tc4pfc [0 / 7]
                       cnmGenerationCfg.localGeneratedPacketTypeAssignmentEnable [GT_FALSE / GT_TRUE]
                       cnmGenerationCfg.localGeneratedPacketType [0 / 31]
            Expected: GT_OK.
        */
        cnmGenerationCfg.packetType = 0;
        cnmGenerationCfg.priority = 0;
        cnmGenerationCfg.dropPrecedence = CPSS_DP_GREEN_E;
        cnmGenerationCfg.tc4pfc = 0;
        cnmGenerationCfg.localGeneratedPacketTypeAssignmentEnable = GT_TRUE;
        cnmGenerationCfg.localGeneratedPacketType = 0;

        st = cpssPxPortCnMessageGenerationConfigSet(dev, &cnmGenerationCfg);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call cpssPxPortCnMessageGenerationConfigGet with not-NULL cnmGenerationCfgPtr.
            Expected: GT_OK and the same fields values as were set.
        */
        st = cpssPxPortCnMessageGenerationConfigGet(dev, &cnmGenerationCfgGet);

        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssPxPortCnMessageGenerationConfigGet: %d", dev);

        UTF_VERIFY_EQUAL1_STRING_MAC(cnmGenerationCfg.packetType, cnmGenerationCfgGet.packetType,
                         "got another cnmGenerationCfg.packetType then was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(cnmGenerationCfg.priority, cnmGenerationCfgGet.priority,
                         "got another cnmGenerationCfg.priority then was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(cnmGenerationCfg.dropPrecedence, cnmGenerationCfgGet.dropPrecedence,
                         "got another cnmGenerationCfg.dropPrecedence then was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(cnmGenerationCfg.tc4pfc, cnmGenerationCfgGet.tc4pfc,
                         "got another cnmGenerationCfg.tc4pfc then was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(cnmGenerationCfg.localGeneratedPacketTypeAssignmentEnable, 
                                     cnmGenerationCfgGet.localGeneratedPacketTypeAssignmentEnable,
                         "got another cnmGenerationCfg.localGeneratedPacketTypeAssignmentEnable then was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(cnmGenerationCfg.localGeneratedPacketType, cnmGenerationCfgGet.localGeneratedPacketType,
                         "got another cnmGenerationCfg.localGeneratedPacketType then was set: %d", dev);


        /*
            1.1. Call with all correct values
                       cnmGenerationCfg.packetType [31]
                       cnmGenerationCfg.priority [7]
                       cnmGenerationCfg.dropPrecedence [CPSS_DP_YELLOW_E]
                       cnmGenerationCfg.tc4pfc [7]
                       cnmGenerationCfg.localGeneratedPacketTypeAssignmentEnable [GT_FALSE]
                       cnmGenerationCfg.localGeneratedPacketType [31]
            Expected: GT_OK.
        */
        cnmGenerationCfg.packetType = 31;
        cnmGenerationCfg.priority = 7;
        cnmGenerationCfg.dropPrecedence = CPSS_DP_YELLOW_E;
        cnmGenerationCfg.tc4pfc = 7;
        cnmGenerationCfg.localGeneratedPacketTypeAssignmentEnable = GT_FALSE;
        cnmGenerationCfg.localGeneratedPacketType = 31;

        st = cpssPxPortCnMessageGenerationConfigSet(dev, &cnmGenerationCfg);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call cpssPxPortCnMessageGenerationConfigGet with not-NULL cnmGenerationCfgGet.
            Expected: GT_OK and the same fields values as were set.
        */
        st = cpssPxPortCnMessageGenerationConfigGet(dev, &cnmGenerationCfgGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssPxPortCnMessageGenerationConfigGet: %d", dev);

        UTF_VERIFY_EQUAL1_STRING_MAC(cnmGenerationCfg.packetType, cnmGenerationCfgGet.packetType,
                         "got another cnmGenerationCfg.packetType then was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(cnmGenerationCfg.priority, cnmGenerationCfgGet.priority,
                         "got another cnmGenerationCfg.priority then was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(cnmGenerationCfg.dropPrecedence, cnmGenerationCfgGet.dropPrecedence,
                         "got another cnmGenerationCfg.dropPrecedence then was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(cnmGenerationCfg.tc4pfc, cnmGenerationCfgGet.tc4pfc,
                         "got another cnmGenerationCfg.tc4pfc then was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(cnmGenerationCfg.localGeneratedPacketTypeAssignmentEnable, 
                                     cnmGenerationCfgGet.localGeneratedPacketTypeAssignmentEnable,
                         "got another cnmGenerationCfg.localGeneratedPacketTypeAssignmentEnable then was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(cnmGenerationCfg.localGeneratedPacketType, cnmGenerationCfgGet.localGeneratedPacketType,
                         "got another cnmGenerationCfg.localGeneratedPacketType then was set: %d", dev);

        /*
            1.3. Call with wrong cnmGenerationCfgPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssPxPortCnMessageGenerationConfigSet(dev, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /*
            1.4. Call with wrong cnmGenerationCfg.packetType [32]
            Expected: NOT GT_OK.
        */
        cnmGenerationCfg.packetType = 32;

        st = cpssPxPortCnMessageGenerationConfigSet(dev, &cnmGenerationCfg);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        cnmGenerationCfg.packetType = 0;

        /*
            1.5. Call with wrong cnmGenerationCfg.priority [8]
            Expected: NOT GT_OK.
        */
        cnmGenerationCfg.priority = 8;

        st = cpssPxPortCnMessageGenerationConfigSet(dev, &cnmGenerationCfg);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        cnmGenerationCfg.priority = 0;

        /*
            1.6. Call with wrong enum cnmGenerationCfg.dropPrecedence
            Expected: NOT GT_OK.
        */
        cnmGenerationCfg.dropPrecedence = CPSS_DP_LAST_E;

        st = cpssPxPortCnMessageGenerationConfigSet(dev, &cnmGenerationCfg);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        cnmGenerationCfg.dropPrecedence = CPSS_DP_GREEN_E;

        /*
            1.7. Call with wrong cnmGenerationCfg.tc4pfc[8]
            Expected: NOT GT_OK.
        */
        cnmGenerationCfg.tc4pfc = 8;
        st = cpssPxPortCnMessageGenerationConfigSet(dev, &cnmGenerationCfg);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        cnmGenerationCfg.tc4pfc = 0;

        /*
            1.8. Call with wrong cnmGenerationCfg.localGeneratedPacketType [32]
            Expected: NOT GT_OK.
        */
        cnmGenerationCfg.localGeneratedPacketType = 32;

        st = cpssPxPortCnMessageGenerationConfigSet(dev, &cnmGenerationCfg);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        cnmGenerationCfg.localGeneratedPacketType = 0;
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortCnMessageGenerationConfigSet(dev, &cnmGenerationCfg);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortCnMessageGenerationConfigSet(dev, &cnmGenerationCfg);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortCnMessageGenerationConfigGet
(
    IN  GT_U8    devNum,
    OUT CPSS_PX_PORT_CNM_GENERATION_CONFIG_STC *cnmGenerationCfgPtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPortCnMessageGenerationConfigGet)
{
/*
    ITERATE_DEVICES (Pipe)
    1.1. Call with not null cnmGenerationCfgPtr.
    Expected: GT_OK.
    1.2. Call with wrong cnmGenerationCfgPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;

    CPSS_PX_PORT_CNM_GENERATION_CONFIG_STC cnmGenerationCfg;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with not null cnmGenerationCfgPtr.
            Expected: GT_OK.
        */
        st = cpssPxPortCnMessageGenerationConfigGet(dev, &cnmGenerationCfg);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with wrong cnmGenerationCfgPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssPxPortCnMessageGenerationConfigGet(dev, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortCnMessageGenerationConfigGet(dev, &cnmGenerationCfg);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortCnMessageGenerationConfigGet(dev, &cnmGenerationCfg);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortCnPacketLengthSet
(
    IN GT_U8    devNum,
    IN CPSS_PX_PORT_CN_PACKET_LENGTH_ENT packetLength
)
*/
UTF_TEST_CASE_MAC(cpssPxPortCnPacketLengthSet)
{
/*
    ITERATE_DEVICES (Pipe)
    1.1. Call with all correct packetLength
                                [CPSS_PX_PORT_CN_LENGTH_ORIG_PACKET_E /
                                 CPSS_PX_PORT_CN_LENGTH_1_5_KB_E /
                                 CPSS_PX_PORT_CN_LENGTH_2_KB_E /
                                 CPSS_PX_PORT_CN_LENGTH_10_KB_E].
    Expected: GT_OK.
    1.2. Call cpssPxPortCnPacketLengthGet with not-NULL packetLengthPtr.
    Expected: GT_OK and the same packetLength as was set.
    1.3. Call with wrong enum values packetLength.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;

    CPSS_PX_PORT_CN_PACKET_LENGTH_ENT  packetLength = CPSS_PX_PORT_CN_LENGTH_ORIG_PACKET_E;
    CPSS_PX_PORT_CN_PACKET_LENGTH_ENT  packetLengthGet;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with all correct packetLength
                                        [CPSS_PX_PORT_CN_LENGTH_ORIG_PACKET_E /
                                         CPSS_PX_PORT_CN_LENGTH_1_5_KB_E /
                                         CPSS_PX_PORT_CN_LENGTH_2_KB_E /
                                         CPSS_PX_PORT_CN_LENGTH_10_KB_E].
            Expected: GT_OK.
        */
        packetLength = CPSS_PX_PORT_CN_LENGTH_ORIG_PACKET_E;

        st = cpssPxPortCnPacketLengthSet(dev, packetLength);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call cpssPxPortCnPacketLengthGet with not-NULL packetLengthPtr.
            Expected: GT_OK and the same packetLength as was set.
        */
        st = cpssPxPortCnPacketLengthGet(dev, &packetLengthGet);

        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                         "cpssPxPortCnPacketLengthGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(packetLength, packetLengthGet,
                         "got another packetLength then was set: %d", dev);

        /*
            1.1. Call with all correct packetLength
                                        [CPSS_PX_PORT_CN_LENGTH_ORIG_PACKET_E /
                                         CPSS_PX_PORT_CN_LENGTH_1_5_KB_E /
                                         CPSS_PX_PORT_CN_LENGTH_2_KB_E /
                                         CPSS_PX_PORT_CN_LENGTH_10_KB_E].
            Expected: GT_OK.
        */
        packetLength = CPSS_PX_PORT_CN_LENGTH_1_5_KB_E;

        st = cpssPxPortCnPacketLengthSet(dev, packetLength);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call cpssPxPortCnPacketLengthGet with not-NULL packetLengthPtr.
            Expected: GT_OK and the same packetLength as was set.
        */
        st = cpssPxPortCnPacketLengthGet(dev, &packetLengthGet);

        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                         "cpssPxPortCnPacketLengthGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(packetLength, packetLengthGet,
                         "got another packetLength then was set: %d", dev);

        /*
            1.1. Call with all correct packetLength
                                        [CPSS_PX_PORT_CN_LENGTH_ORIG_PACKET_E /
                                         CPSS_PX_PORT_CN_LENGTH_1_5_KB_E /
                                         CPSS_PX_PORT_CN_LENGTH_2_KB_E /
                                         CPSS_PX_PORT_CN_LENGTH_10_KB_E].
            Expected: GT_OK.
        */
        packetLength = CPSS_PX_PORT_CN_LENGTH_2_KB_E;

        st = cpssPxPortCnPacketLengthSet(dev, packetLength);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call cpssPxPortCnPacketLengthGet with not-NULL packetLengthPtr.
            Expected: GT_OK and the same packetLength as was set.
        */
        st = cpssPxPortCnPacketLengthGet(dev, &packetLengthGet);

        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                         "cpssPxPortCnPacketLengthGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(packetLength, packetLengthGet,
                         "got another packetLength then was set: %d", dev);

        /*
            1.1. Call with all correct packetLength
                                        [CPSS_PX_PORT_CN_LENGTH_ORIG_PACKET_E /
                                         CPSS_PX_PORT_CN_LENGTH_1_5_KB_E /
                                         CPSS_PX_PORT_CN_LENGTH_2_KB_E /
                                         CPSS_PX_PORT_CN_LENGTH_10_KB_E].
            Expected: GT_OK.
        */
        packetLength = CPSS_PX_PORT_CN_LENGTH_10_KB_E;

        st = cpssPxPortCnPacketLengthSet(dev, packetLength);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call cpssPxPortCnPacketLengthGet with not-NULL packetLengthPtr.
            Expected: GT_OK and the same packetLength as was set.
        */
        st = cpssPxPortCnPacketLengthGet(dev, &packetLengthGet);

        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                         "cpssPxPortCnPacketLengthGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(packetLength, packetLengthGet,
                         "got another packetLength then was set: %d", dev);

        /*
            1.3. Call with wrong enum values packetLength.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssPxPortCnPacketLengthSet
                            (dev, packetLength),
                            packetLength);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortCnPacketLengthSet(dev, packetLength);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortCnPacketLengthSet(dev, packetLength);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortCnPacketLengthGet
(
    IN  GT_U8    devNum,
    OUT CPSS_PX_PORT_CN_PACKET_LENGTH_ENT  *packetLengthPtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPortCnPacketLengthGet)
{
/*
    ITERATE_DEVICES (Pipe)
    1.1. Call with not null packetLengthPtr.
    Expected: GT_OK.
    1.2. Call with wrong packetLengthPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;

    CPSS_PX_PORT_CN_PACKET_LENGTH_ENT  packetLength;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with not null packetLengthPtr.
            Expected: GT_OK.
        */
        st = cpssPxPortCnPacketLengthGet(dev, &packetLength);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with wrong packetLengthPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssPxPortCnPacketLengthGet(dev, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortCnPacketLengthGet(dev, &packetLength);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortCnPacketLengthGet(dev, &packetLength);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortCnQueueStatusModeEnableSet
(
    IN GT_U8    devNum,
    IN GT_BOOL  enable,
    IN GT_U8    targetPort
)
*/
UTF_TEST_CASE_MAC(cpssPxPortCnQueueStatusModeEnableSet)
{
/*
    ITERATE_DEVICES (Pipe)
    1.1. Call with all correct parameters
                                        enable [GT_TRUE / GT_FALSE],
                                        targetPort [0 - 16].
    Expected: GT_OK.
    1.2. Call cpssPxPortCnQueueStatusModeEnableGet with not-NULL pointers
    Expected: GT_OK and the same values as was set.
    1.3. Call with enable [GT_FALSE].
    Expected: GT_OK.
    1.4. Call withenable [GT_FALSE].
    Expected: GT_OK.
    1.5. Call with enable [GT_TRUE].
    Expected: NOT GT_OK.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;

    GT_BOOL  enable = GT_TRUE;
    GT_PORT_NUM    targetPort = PORT_CN_VALID_PHY_PORT_CNS;
    GT_PORT_NUM    maxTargetPort;

    GT_BOOL  enableGet = GT_FALSE;
    GT_PORT_NUM    targetPortGet = 0;



    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {

        maxTargetPort = 17;

        /*
            1.1. Call with all correct parameters enable [GT_TRUE / GT_FALSE], targetPort [0 - 16].
            Expected: GT_OK.
        */
        for(enable = GT_FALSE; enable <= GT_TRUE; enable++)
            for(targetPort = 0; targetPort < maxTargetPort; targetPort++)
            {
                st = cpssPxPortCnQueueStatusModeEnableSet(dev, enable, targetPort);
                UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,
                 "cpssPxPortCnQueueStatusModeEnableSet: dev=%d, enable=%d, targetPort=%d",
                                             dev, enable, targetPort);

                if(GT_OK == st)
                {
                    /*
                        1.2. Call cpssPxPortCnQueueStatusModeEnableGet
                             with not-NULL cnProfileCfgPtr.
                        Expected: GT_OK and the same values as was set.
                    */
                    st = cpssPxPortCnQueueStatusModeEnableGet(dev,
                                        &enableGet, &targetPortGet);
                    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                        "cpssPxPortCnQueueStatusModeEnableGet: %d", dev);

                    UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                                    "got another enable then was set: %d", dev);

                    if(GT_TRUE == enable)
                    {
                        UTF_VERIFY_EQUAL1_STRING_MAC(targetPort, targetPortGet,
                                        "got another targetPort then was set: %d", dev);
                    }
                }
            }

        /*
            1.3. Call with enable [GT_FALSE].
            Expected: GT_OK.
        */
        enable = GT_FALSE;
        targetPort = 0;

        st = cpssPxPortCnQueueStatusModeEnableSet(dev, enable, targetPort);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,
         "cpssPxPortCnQueueStatusModeEnableSet: dev=%d, enable=%d, targetPort=%d",
                                     dev, enable, targetPort);
        /*
            1.4. Call with enable [GT_FALSE].
            Expected: GT_OK.
        */
        enable = GT_FALSE;
        targetPort = maxTargetPort;

        st = cpssPxPortCnQueueStatusModeEnableSet(dev, enable, targetPort);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,
         "cpssPxPortCnQueueStatusModeEnableSet: dev=%d, enable=%d, targetPort=%d",
                                     dev, enable, targetPort);
        /*
            1.5. Call with enable [GT_TRUE].
            Expected: NOT GT_OK.
        */
        enable = GT_TRUE;

        st = cpssPxPortCnQueueStatusModeEnableSet(dev, enable, targetPort);
        UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st,
         "cpssPxPortCnQueueStatusModeEnableSet: dev=%d, enable=%d, targetPort=%d",
                                     dev, enable, targetPort);
        targetPort = 0;
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortCnQueueStatusModeEnableSet(dev, enable, targetPort);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortCnQueueStatusModeEnableSet(dev, enable, targetPort);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortCnQueueStatusModeEnableGet
(
    IN  GT_U8    devNum,
    OUT GT_BOOL  *enablePtr,
    OUT GT_U8    *targetPortPtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPortCnQueueStatusModeEnableGet)
{
/*
    ITERATE_DEVICES (Pipe)
    1.1. Call with not null pointers.
    Expected: GT_OK.
    1.2. Call with wrong enablePtr [NULL].
    Expected: GT_BAD_PTR.
    1.3. Call with wrong targetPortPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;

    GT_BOOL     enable;
    GT_PORT_NUM targetPort;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with not null enablePtr.
            Expected: GT_OK.
        */
        st = cpssPxPortCnQueueStatusModeEnableGet(dev, &enable, &targetPort);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with wrong enablePtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssPxPortCnQueueStatusModeEnableGet(dev, NULL, &targetPort);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /*
            1.3. Call with wrong targetPort [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssPxPortCnQueueStatusModeEnableGet(dev, &enable, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortCnQueueStatusModeEnableGet(dev, &enable, &targetPort);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortCnQueueStatusModeEnableGet(dev, &enable, &targetPort);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortCnSampleEntrySet
(
    IN GT_U8    devNum,
    IN GT_U8    entryIndex,
    IN CPSS_PX_PORT_CN_SAMPLE_INTERVAL_ENTRY_STC *entryPtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPortCnSampleEntrySet)
{
/*
    ITERATE_DEVICES (Pipe)
    1.1. Call with all correct parameters
                            entryIndex[0 - 7]
                            entry.interval[0 - 0xFFFF]
                            entry.randBitmap[0 - 0xFFFF]
    Expected: GT_OK.
    1.2. Call cpssPxPortCnSampleEntryGet with not-NULL entryPtr.
    Expected: GT_OK and the same values as was set.
    1.3. Call with wrong entryIndex [8].
    Expected: NOT GT_OK.
    1.4. Call with wrong entry.interval [0x10000].
    Expected: NOT GT_OK.
    1.5. Call with wrong entry.randBitmap [0x10000].
    Expected: NOT GT_OK.
    1.6. Call with wrong entryPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;

    GT_U8                                       entryIndex = 0;
    CPSS_PX_PORT_CN_SAMPLE_INTERVAL_ENTRY_STC   entry;
    CPSS_PX_PORT_CN_SAMPLE_INTERVAL_ENTRY_STC   entryGet;
    GT_U32      counter = 0;/* loop counter */
    GT_BOOL     reduceIterations = (prvUtfIsGmCompilation() || prvUtfReduceLogSizeFlagGet());

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for(entryIndex = 0; entryIndex < 8; entryIndex++)
        {
            for(entry.interval = 0; entry.interval < 0xFFFF; entry.interval += 256)
            {
                for(entry.randBitmap = 0; entry.randBitmap < 0xFFFF; entry.randBitmap += 384)
                {
                    counter++;
                    /*
                        1.1. Call with all correct parameters
                                                entryIndex[0 - 7]
                                                entry.interval[0 - 0xFFFF]
                                                entry.randBitmap[0 - 0xFFFF]
                        Expected: GT_OK.
                    */
                    st = cpssPxPortCnSampleEntrySet(dev, entryIndex, &entry);

                    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st,
                    "cpssPxPortCnSampleEntrySet: dev=%d, entryIndex=%d, entry.interval=%d, entry.randBitmap=%d",
                               dev, entryIndex, entry.interval, entry.randBitmap);

                    if(GT_OK == st)
                    {
                        /*
                            1.2. Call cpssPxPortCnSampleEntryGet with not-NULL entryPtr.
                            Expected: GT_OK and the same values as was set.
                        */
                        st = cpssPxPortCnSampleEntryGet(dev, entryIndex, &entryGet);

                        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                         "cpssPxPortCnSampleEntryGet: %d", dev);
                        UTF_VERIFY_EQUAL1_STRING_MAC(entry.interval, entryGet.interval,
                                         "got another entry.interval then was set: %d", dev);
                        UTF_VERIFY_EQUAL1_STRING_MAC(entry.randBitmap, entryGet.randBitmap,
                                         "got another entry.randBitmap then was set: %d", dev);
                    }

                    if((counter % 100000) == 0)
                    {
                        utfPrintKeepAlive();
                    }

                    if(reduceIterations)
                    {
                        /* reduce iterations number */
                        entry.randBitmap += (384 * 11);
                    }
                }

                if(reduceIterations)
                {
                    /* reduce iterations number */
                    entry.interval += (256*11);
                }
            }

            if(reduceIterations)
            {
                /* reduce iterations number */
                entryIndex += 5;
            }
        }
        /*
            1.3. Call with wrong entryIndex [8].
            Expected: NOT GT_OK.
        */
        entryIndex = 8;
        entry.interval = 0;
        entry.randBitmap = 0;

        st = cpssPxPortCnSampleEntrySet(dev, entryIndex, &entry);

        UTF_VERIFY_NOT_EQUAL4_STRING_MAC(GT_OK, st,
        "cpssPxPortCnSampleEntrySet: dev=%d, entryIndex=%d, entry.interval=%d, entry.randBitmap=%d",
                   dev, entryIndex, entry.interval, entry.randBitmap);

        entryIndex = 0;

        /*
            1.4. Call with wrong entry.interval [0x10000].
            Expected: NOT GT_OK.
        */
        entry.interval = 0x10000;

        st = cpssPxPortCnSampleEntrySet(dev, entryIndex, &entry);

        UTF_VERIFY_NOT_EQUAL4_STRING_MAC(GT_OK, st,
        "cpssPxPortCnSampleEntrySet: dev=%d, entryIndex=%d, entry.interval=%d, entry.randBitmap=%d",
                   dev, entryIndex, entry.interval, entry.randBitmap);

        entry.interval = 0;

        /*
            1.5. Call with wrong entry.randBitmap [0x10000].
            Expected: NOT GT_OK.
        */
        entry.randBitmap = 0x10000;

        st = cpssPxPortCnSampleEntrySet(dev, entryIndex, &entry);

        UTF_VERIFY_NOT_EQUAL4_STRING_MAC(GT_OK, st,
        "cpssPxPortCnSampleEntrySet: dev=%d, entryIndex=%d, entry.randBitmap=%d, entry.randBitmap=%d",
                   dev, entryIndex, entry.randBitmap, entry.randBitmap);

        entry.randBitmap = 0;

        /*
            1.6. Call with wrong entryPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssPxPortCnSampleEntrySet(dev, entryIndex, NULL);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, st);

    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortCnSampleEntrySet(dev, entryIndex, &entry);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortCnSampleEntrySet(dev, entryIndex, &entry);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortCnSampleEntryGet
(
    IN GT_U8    devNum,
    IN GT_U8    entryIndex,
    OUT CPSS_PX_PORT_CN_SAMPLE_INTERVAL_ENTRY_STC  *entryPtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPortCnSampleEntryGet)
{
/*
    ITERATE_DEVICES (Pipe)
    1.1. Call with all correct parameters entryIndex[0 - 7] and not null entryPtr.
    Expected: GT_OK.
    1.2. Call with wrong entryIndex [8].
    Expected: NOT GT_OK.
    1.3. Call with wrong entryPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;

    GT_U8                                       entryIndex = 0;
    CPSS_PX_PORT_CN_SAMPLE_INTERVAL_ENTRY_STC   entry;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with all correct parameters entryIndex[0 - 7] and not null entryPtr.
            Expected: GT_OK.
        */
        for(entryIndex = 0; entryIndex < 8; entryIndex++)
        {
            st = cpssPxPortCnSampleEntryGet(dev, entryIndex, &entry);

            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                "cpssPxPortCnSampleEntryGet: dev=%d, entryIndex=", dev, entryIndex);
        }
        /*
            1.3. Call with wrong entryIndex [8].
            Expected: NOT GT_OK.
        */
        entryIndex = 8;

        st = cpssPxPortCnSampleEntryGet(dev, entryIndex, &entry);

        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st,
                "cpssPxPortCnSampleEntryGet: dev=%d, entryIndex=%d", dev, entryIndex);
        entryIndex = 0;

        /*
            1.3. Call with wrong entryPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssPxPortCnSampleEntryGet(dev, entryIndex, NULL);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, st);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortCnSampleEntryGet(dev, entryIndex, &entry);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortCnSampleEntryGet(dev, entryIndex, &entry);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortCnTerminationEnableSet
(
    IN  GT_U8     devNum,
    IN  GT_U8     portNum,
    IN  GT_BOOL   enable
)
*/
UTF_TEST_CASE_MAC(cpssPxPortCnTerminationEnableSet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (Pipe)
    1.1.1. Call with state [GT_FALSE and GT_TRUE].
    Expected: GT_OK.
    1.1.2. Call cpssPxPortCnTerminationEnableGet.
    Expected: GT_OK and the same enable.
*/
    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM port   = PORT_CN_VALID_PHY_PORT_CNS;
    GT_BOOL     enable = GT_FALSE;
    GT_BOOL     state  = GT_FALSE;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            /*
               1.1.1. Call with state [GT_FALSE and GT_TRUE].
               Expected: GT_OK.
            */

            /* Call function with enable = GT_FALSE] */
            enable = GT_FALSE;

            st = cpssPxPortCnTerminationEnableSet(dev, port, enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);

            /*
               1.1.2. Call cpssPxPortCnTerminationEnableGet.
               Expected: GT_OK and the same enable.
            */
            st = cpssPxPortCnTerminationEnableGet(dev, port, &state);

            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                            "cpssPxPortCnTerminationEnableGet: %d, %d", dev, port);
            UTF_VERIFY_EQUAL2_STRING_MAC(enable, state,
                            "get another enable value than was set: %d, %d", dev, port);

            /*
               1.1.1. Call with state [GT_FALSE and GT_TRUE].
               Expected: GT_OK.
            */

            /* Call function with enable = GT_TRUE] */
            enable = GT_TRUE;

            st = cpssPxPortCnTerminationEnableSet(dev, port, enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);

            /*
               1.1.2. Call cpssPxPortCnTerminationEnableGet.
               Expected: GT_OK and the same enable.
            */
            st = cpssPxPortCnTerminationEnableGet(dev, port, &state);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                          "[cpssPxPortCnTerminationEnableGet]: %d, %d", dev, port);
            UTF_VERIFY_EQUAL2_STRING_MAC(enable, state,
                          "get another enable value than was set: %d, %d", dev, port);
        }

        /*
           1.2. For all active devices go over all non available
           physical ports.
        */
        enable = GT_TRUE;

        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            /* enable == GT_TRUE    */
            st = cpssPxPortCnTerminationEnableSet(dev, port, enable);
            if(port >= UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev))
            {
                UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            }
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        /* enable == GT_TRUE  */
        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssPxPortCnTerminationEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                     */
        /* enable == GT_TRUE  */
        port = PRV_CPSS_PX_CPU_DMA_NUM_CNS;

        st = cpssPxPortCnTerminationEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    enable = GT_TRUE;
    port = PORT_CN_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortCnTerminationEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    /* port == 0, enable == GT_TRUE */

    st = cpssPxPortCnTerminationEnableSet(dev, port, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortCnTerminationEnableGet
(
    IN   GT_U8     devNum,
    IN   GT_U8     portNum,
    OUT  GT_BOOL   *statePtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPortCnTerminationEnableGet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (Pipe)
    1.1.1. Call with non-null statePtr.
    Expected: GT_OK.
    1.1.2. Call with statePtr [NULL].
    Expected: GT_BAD_PTR
*/
    GT_STATUS   st    = GT_OK;

    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM  port  = PORT_CN_VALID_PHY_PORT_CNS;
    GT_BOOL     state = GT_FALSE;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            /* 1.1.1. Call with non-null statePtr.
               Expected: GT_OK.
            */
            st = cpssPxPortCnTerminationEnableGet(dev, port, &state);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* 1.1.2. Call with statePtr [NULL].
               Expected: GT_BAD_PTR
            */
            st = cpssPxPortCnTerminationEnableGet(dev, port, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, NULL", dev, port);
        }

        /* 1.2. For all active devices go over all non available
           physical ports.
        */
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_FALSE))
        {
            st = cpssPxPortCnTerminationEnableGet(dev, port, &state);
            if(port >= UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev))
            {
                UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            }
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssPxPortCnTerminationEnableGet(dev, port, &state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = PRV_CPSS_PX_CPU_DMA_NUM_CNS;

        st = cpssPxPortCnTerminationEnableGet(dev, port, &state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    port = PORT_CN_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortCnTerminationEnableGet(dev, port, &state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortCnTerminationEnableGet(dev, port, &state);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortCnMessagePortMapEntrySet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  CPSS_PX_PORTS_BMP               portsBmp
);
*/
UTF_TEST_CASE_MAC(cpssPxPortCnMessagePortMapEntrySet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (Pipe)
    1.1. Call with portNum[0..maxVal] for all valid ports.
    Expected: GT_OK.
    1.1.1 Call with out of range port[maxVal+1].
    Expected: GT_OUT_OF_RANGE.
    1.1.2  Call with not valid port bitmap.
    Expected: GT_BAD_PARAM.
    1.2. For all active devices go over all non available
    physical ports.
    Expected: not OK
    1.2.1 Call function for each non-active port
    Expected: not OK
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM  port  = PORT_CN_VALID_PHY_PORT_CNS;

    CPSS_PX_PORTS_BMP portBitmap = 0;
    CPSS_PX_PORTS_BMP portBitmapGet;

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            st = cpssPxPortCnMessagePortMapEntrySet(dev, port, portBitmap);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, port);

            st = cpssPxPortCnMessagePortMapEntryGet(dev, port, &portBitmapGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, port);

            /* Verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(portBitmap, portBitmapGet,
                       "get another portBitmapGet than was set: %d", port);

            /* Max 17 bits */
            portBitmap = (portBitmap + 1) & 0x1FFFF;
        }
        /*  1.1.1  Call with out of range port[maxVal+1].
            Expected: GT_OUT_OF_RANGE. */
        st = cpssPxPortCnMessagePortMapEntrySet(dev, port, portBitmap);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, port);

        /* Reset port to be valid */
        port  = PORT_CN_VALID_PHY_PORT_CNS;
        /*  1.1.2  Call with not valid port bitmap.
            Expected: GT_BAD_PARAM. */
        st = cpssPxPortCnMessagePortMapEntrySet(dev, port, 0xffffff);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, port);

        /*
           1.2. For all active devices go over all non available
           physical ports.
        */
        portBitmap = 0xF;

        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_FALSE))
        {
            /* 
               1.2.1 Call function for each non-active port 
            */
            st = cpssPxPortCnMessagePortMapEntrySet(dev, port, portBitmap);
            if(port >= UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev))
            {
                UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            }
        }

        /* 1.2.2 For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        /* enable == GT_TRUE  */
        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssPxPortCnMessagePortMapEntrySet(dev, port, portBitmap);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortCnMessagePortMapEntrySet(dev, port, portBitmap);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, port);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortCnMessagePortMapEntrySet(dev, port, portBitmap);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, port);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortCnMessagePortMapEntryGet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    OUT CPSS_PX_PORTS_BMP               *portsBmpPtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPortCnMessagePortMapEntryGet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (Pipe)
    1.1. Call with portNum[0..maxVal] for all valid ports.
    Expected: GT_OK.
    1.1.1 Call with out of range portNum[maxVal+1].
    Expected: GT_OUT_OF_RANGE.
    1.1.2 Call with portsBmpPtr == NULL.
    Expected: GT_BAD_PTR.
    1.2. For all active devices go over all non available
    physical ports.
    Expected: not OK
    1.2.1 Call function for each non-active port enable == GT_TRUE
    Expected: not OK

*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM  port  = PORT_CN_VALID_PHY_PORT_CNS;

    CPSS_PX_PORTS_BMP portBitmapGet;

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            st = cpssPxPortCnMessagePortMapEntryGet(dev, port, &portBitmapGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, port);
        }
        /*  1.1.1  Call with out of range port[maxVal+1].
            Expected: GT_OUT_OF_RANGE. */
        st = cpssPxPortCnMessagePortMapEntryGet(dev, port, &portBitmapGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, port);

        /* Reset port to be valid */
        port = PORT_CN_VALID_PHY_PORT_CNS;

        /*  1.1.2  Call with portsBmpPtr == NULL.
            Expected: GT_BAD_PTR. */
        st = cpssPxPortCnMessagePortMapEntryGet(dev, port, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, port);

        /* 1.2. For all active devices go over all non available
           physical ports.
        */
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_FALSE))
        {
            /*
                1.2.1 Call function for each non-active port enable == GT_TRUE
                Expected: not OK
            */
            st = cpssPxPortCnMessagePortMapEntryGet(dev, port, &portBitmapGet);
            if(port >= UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev))
            {
                UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            }
        }
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortCnMessagePortMapEntryGet(dev, port, &portBitmapGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, port);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortCnMessagePortMapEntryGet(dev, port, &portBitmapGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, port);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortCnMessageTriggeringStateSet
(
    IN GT_SW_DEV_NUM            devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN GT_U8                    tcQueue,
    IN GT_U32                   qcnSampleInterval,
    IN GT_U32                   qlenOld
);
*/
UTF_TEST_CASE_MAC(cpssPxPortCnMessageTriggeringStateSet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (Pipe)
    1.1. Call with portNum[0..maxVal] for all valid ports.
    Expected: GT_OK.
    1.1.1 Call with out of range port[maxVal+1].
    Expected: GT_OUT_OF_RANGE.
    1.1.2  Call with not valid qcnSampleInterval, qlenOld.
    Expected: GT_OUT_OF_RANGE.
    1.2. For all active devices go over all non available
    physical ports.
    Expected: not OK
    1.2.1 Call function for each non-active port
    Expected: not OK
    1.2.2 For active device check that function returns GT_BAD_PARAM
    for out of bound value for port number.                         
    enable == GT_TRUE
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM  port  = PORT_CN_VALID_PHY_PORT_CNS;

    GT_U8       tcQueue = 0;
    GT_U32      qcnSampleInterval = 0;
    GT_U32      qlenOld = 0;

    GT_U32      qcnSampleIntervalGet;
    GT_U32      qlenOldGet;

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            for (tcQueue = 0; tcQueue < 8; tcQueue++)
            {
                st = cpssPxPortCnMessageTriggeringStateSet(dev, port, tcQueue, qcnSampleInterval, qlenOld);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, port);

                st = cpssPxPortCnMessageTriggeringStateGet(dev, port, tcQueue, &qcnSampleIntervalGet, &qlenOldGet);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, port);

                /* Verifying values */
                UTF_VERIFY_EQUAL1_STRING_MAC(qcnSampleInterval, qcnSampleIntervalGet,
                           "get another qcnSampleIntervalGet than was set: %d", port);
                UTF_VERIFY_EQUAL1_STRING_MAC(qlenOld, qlenOldGet,
                           "get another qlenOldGet than was set: %d", port);

                /* Generate random values in valid range */
                qcnSampleInterval = cpssOsRand() % QCN_SAMPLE_INTERVAL_MAX_CNS;
                qlenOld = cpssOsRand() % QCN_SAMPLE_INTERVAL_MAX_CNS;
            }
        }

        /* 
            1.1.1  Call with out of range port[maxVal+1].
            Expected: GT_BAD_PARAM.
        */
        st = cpssPxPortCnMessageTriggeringStateSet(dev, port, tcQueue, qcnSampleInterval, qlenOld);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, port);

        /* Reset port and tc queue to be valid */
        port  = PORT_CN_VALID_PHY_PORT_CNS;
        tcQueue = 2;
        /* 
            1.1.2  Call with not valid qcnSampleInterval, qlenOld.
            Expected: GT_OUT_OF_RANGE.
        */
        st = cpssPxPortCnMessageTriggeringStateSet(dev, port, tcQueue, QCN_SAMPLE_INTERVAL_MAX_CNS+1, 0);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, port);

        st = cpssPxPortCnMessageTriggeringStateSet(dev, port, tcQueue, 0, QCN_SAMPLE_INTERVAL_MAX_CNS+1);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, port);
        /*
           1.2. For all active devices go over all non available
           physical ports.
        */
        qcnSampleInterval = 0;
        qlenOld = 0;

        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_FALSE))
        {
            /* 
               1.2.1 Call function for each non-active port 
            */
            st = cpssPxPortCnMessageTriggeringStateSet(dev, port, tcQueue, qcnSampleInterval, qlenOld);
            if(port >= UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev))
            {
                UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            }
        }

        /* 
           1.2.2 For active device check that function returns GT_BAD_PARAM
           for out of bound value for port number.                         
           enable == GT_TRUE
        */
        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssPxPortCnMessageTriggeringStateSet(dev, port, tcQueue, qcnSampleInterval, qlenOld);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortCnMessageTriggeringStateSet(dev, port, tcQueue, qcnSampleInterval, qlenOld);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, port);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortCnMessageTriggeringStateSet(dev, port, tcQueue, qcnSampleInterval, qlenOld);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, port);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortCnMessageTriggeringStateGet
(
    IN GT_SW_DEV_NUM            devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U8                   tcQueue,
    OUT GT_U32                  *qcnSampleIntervalPtr,
    OUT GT_U32                  *qlenOldPtr
);
*/
UTF_TEST_CASE_MAC(cpssPxPortCnMessageTriggeringStateGet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (Pipe)
    1.1. Call with portNum[0..maxVal] for all valid ports.
    Expected: GT_OK.
    1.1.1 Call with out of range portNum[maxVal+1].
    Expected: GT_OUT_OF_RANGE.
    1.1.2  Call with qcnSampleInterval == NULL.
    Expected: GT_BAD_PTR.
    1.1.3  Call with qlenOld == NULL.
    Expected: GT_BAD_PTR.
    1.2. For all active devices go over all non available
    physical ports.
    Expected: not OK
    1.2.1 Call function for each non-active port enable == GT_TRUE
    Expected: not OK

*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM  port  = PORT_CN_VALID_PHY_PORT_CNS;
    GT_U8       tcQueue = 0;

    GT_U32      qcnSampleIntervalGet;
    GT_U32      qlenOldGet;

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            st = cpssPxPortCnMessageTriggeringStateGet(dev, port, tcQueue, &qcnSampleIntervalGet, &qlenOldGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, port);
        }
        /* 
            1.1.1  Call with out of range port[maxVal+1].
            Expected: GT_OUT_OF_RANGE.
        */
        st = cpssPxPortCnMessageTriggeringStateGet(dev, port, tcQueue, &qcnSampleIntervalGet, &qlenOldGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, port);

        /* Reset port to be valid */
        port = PORT_CN_VALID_PHY_PORT_CNS;
        tcQueue = 2;

        /* 
            1.1.2  Call with qcnSampleInterval == NULL.
            Expected: GT_BAD_PTR.
        */
        st = cpssPxPortCnMessageTriggeringStateGet(dev, port, tcQueue, NULL, &qlenOldGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, port);

        /* 
            1.1.3  Call with qlenOld == NULL.
            Expected: GT_BAD_PTR.
        */
        st = cpssPxPortCnMessageTriggeringStateGet(dev, port, tcQueue, &qcnSampleIntervalGet, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, port);

        /* 
           1.2. For all active devices go over all non available
           physical ports.
        */
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_FALSE))
        {
            /*
                1.2.1 Call function for each non-active port enable == GT_TRUE
                Expected: not OK
            */
            st = cpssPxPortCnMessageTriggeringStateGet(dev, port, tcQueue, &qcnSampleIntervalGet, &qlenOldGet);
            if(port >= UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev))
            {
                UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            }
        }
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortCnMessageTriggeringStateGet(dev, port, tcQueue, &qcnSampleIntervalGet, &qlenOldGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, port);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortCnMessageTriggeringStateGet(dev, port, tcQueue, &qcnSampleIntervalGet, &qlenOldGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, port);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortCnProfileQueueConfigSet
(
    IN GT_SW_DEV_NUM                                devNum,
    IN CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT    profileSet,
    IN GT_U8                                        tcQueue,
    IN CPSS_PX_PORT_CN_PROFILE_CONFIG_STC           *cnProfileCfgPtr
) 
*/ 
UTF_TEST_CASE_MAC(cpssPxPortCnProfileQueueConfigSet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (Pipe)
    1.1. Call with all correct parameters.
    Expected: GT_OK.
    1.2. Call cpssPxPortCnProfileQueueConfigGet
         with not-NULL cnProfileCfgPtr.
    Expected: GT_OK and the same values as was set.
    1.3. Call with wrong enum values profileSet.
    Expected: GT_BAD_PARAM.
    1.4. Call with wrong tcQueue [CPSS_TC_RANGE_CNS].
    Expected: NOT GT_OK.
    1.5. Call with wrong cnProfileCfgPtr [NULL].
    Expected: GT_BAD_PTR.
    1.6. Call with wrong cnProfileCfg.threshold [out of range].
    Expected: NOT GT_OK.
    1.7. Call with wrong cnProfileCfg.alpha [out of range].
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;

    CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT   profileSet = CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_0_E;
    GT_U32                                      tcQueue = 0;
    CPSS_PX_PORT_CN_PROFILE_CONFIG_STC          cnProfileCfg;
    CPSS_PX_PORT_CN_PROFILE_CONFIG_STC          cnProfileCfgGet;
    GT_U32 thresholdNumOfBits;

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with all correct parameters.
            Expected: GT_OK.
        */
        cnProfileCfg.cnAware = GT_TRUE;
        cnProfileCfg.threshold = 0;
        cnProfileCfg.alpha = CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_1_E;
        for (profileSet = CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_0_E; profileSet <= CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_15_E;  profileSet++)
        {
            for (tcQueue = 0; tcQueue < CPSS_TC_RANGE_CNS; tcQueue++)
            {
                st = cpssPxPortCnProfileQueueConfigSet(dev, profileSet, tcQueue, &cnProfileCfg);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
                /*
                    1.2. Call cpssPxPortCnProfileQueueConfigGet
                         with not-NULL cnProfileCfgPtr.
                    Expected: GT_OK and the same values as was set.
                */
                st = cpssPxPortCnProfileQueueConfigGet(dev, profileSet, tcQueue, &cnProfileCfgGet);

                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                    "cpssPxPortCnProfileQueueConfigGet: %d", dev);

                UTF_VERIFY_EQUAL1_STRING_MAC(cnProfileCfg.cnAware,
                                             cnProfileCfgGet.cnAware,
                    "got another cnProfileCfg.cnAware then was set: %d", dev);

                UTF_VERIFY_EQUAL1_STRING_MAC(cnProfileCfg.threshold,
                                             cnProfileCfgGet.threshold,
                    "got another cnProfileCfg.threshold then was set: %d", dev);

                UTF_VERIFY_EQUAL1_STRING_MAC(cnProfileCfg.alpha,
                                             cnProfileCfgGet.alpha,
                    "got another cnProfileCfg.alpha then was set: %d", dev);

                /* Revert cnAware value */
                cnProfileCfg.cnAware = !cnProfileCfg.cnAware;

                /* Generate random threshold value in valid range */
                cnProfileCfg.threshold = cpssOsRand() % BIT_20;

                /* Generate random alpha coefficient */
                cnProfileCfg.alpha = cpssOsRand() %
                    (CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_4_E + 1);
            }
        }


        /* Restore valid values */
        profileSet = CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_0_E;
        tcQueue = 0;
        /* call with max threshold value */
        thresholdNumOfBits = 20;
        cnProfileCfg.threshold = BIT_MASK_MAC(thresholdNumOfBits);
        cnProfileCfg.alpha = CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_2_E;

        st = cpssPxPortCnProfileQueueConfigSet(dev, profileSet, tcQueue, &cnProfileCfg);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        if(GT_OK == st)
        {
            /*
                1.2. Call cpssPxPortCnProfileQueueConfigGet
                     with not-NULL cnProfileCfgPtr.
                Expected: GT_OK and the same values as was set.
            */
            st = cpssPxPortCnProfileQueueConfigGet(dev, profileSet, tcQueue, &cnProfileCfgGet);

            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssPxPortCnProfileQueueConfigGet: %d", dev);

            UTF_VERIFY_EQUAL1_STRING_MAC(cnProfileCfg.cnAware, cnProfileCfgGet.cnAware,
                "got another cnProfileCfg.cnAware then was set: %d", dev);

            UTF_VERIFY_EQUAL1_STRING_MAC(cnProfileCfg.threshold, cnProfileCfgGet.threshold,
                "got another cnProfileCfg.threshold then was set: %d", dev);

            UTF_VERIFY_EQUAL1_STRING_MAC(cnProfileCfg.alpha, cnProfileCfgGet.alpha,
                "got another cnProfileCfg.alpha then was set: %d", dev);
        }

        /*
            1.3. Call with wrong enum values profileSet.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssPxPortCnProfileQueueConfigSet
                            (dev, profileSet, tcQueue, &cnProfileCfg),
                            profileSet);

        /*
            1.4. Call with wrong tcQueue [CPSS_TC_RANGE_CNS].
            Expected: NOT GT_OK.
        */
        tcQueue = CPSS_TC_RANGE_CNS;

        st = cpssPxPortCnProfileQueueConfigSet(dev, profileSet, tcQueue, &cnProfileCfg);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        tcQueue = 5;

        /*
            1.5. Call with wrong cnProfileCfgPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssPxPortCnProfileQueueConfigSet(dev, profileSet, tcQueue, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);


        /*
            1.6. Call with wrong cnProfileCfg.threshold [out of range].
            Expected: GT_BAD_PARAM.
        */
        cnProfileCfg.threshold = BIT_MASK_MAC(thresholdNumOfBits) + 1;

        st = cpssPxPortCnProfileQueueConfigSet(dev, profileSet, tcQueue, &cnProfileCfg);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        cnProfileCfg.threshold = 0;

        /*
            1.7. Call with wrong cnProfileCfg.alpha [out of range].
            Expected: GT_BAD_PARAM.
        */
        cnProfileCfg.alpha = CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_4_E + 1;

        st = cpssPxPortCnProfileQueueConfigSet(dev, profileSet, tcQueue, &cnProfileCfg);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        cnProfileCfg.alpha = CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_1_E;
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortCnProfileQueueConfigSet(dev, profileSet, tcQueue, &cnProfileCfg);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortCnProfileQueueConfigSet(dev, profileSet, tcQueue, &cnProfileCfg);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortCnProfileQueueConfigGet
(
    IN  GT_SW_DEV_NUM                               devNum,
    IN  CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT   profileSet,
    IN  GT_U8                                       tcQueue,
    OUT CPSS_PX_PORT_CN_PROFILE_CONFIG_STC          *cnProfileCfgPtr
) 
*/ 
UTF_TEST_CASE_MAC(cpssPxPortCnProfileQueueConfigGet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (Pipe)
    1.1. Call with all correct parameters.
    Expected: GT_OK.
    1.2. Call with wrong enum values profileSet.
    Expected: GT_BAD_PARAM.
    1.3. Call with wrong tcQueue [CPSS_TC_RANGE_CNS].
    Expected: NOT GT_OK.
    1.4. Call with wrong cnProfileCfgPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;

    CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT   profileSet = CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_0_E;
    GT_U32                                      tcQueue = 0;
    CPSS_PX_PORT_CN_PROFILE_CONFIG_STC          cnProfileCfg;

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with all correct parameters.
            Expected: GT_OK.
        */
        for (profileSet = CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_0_E; profileSet <= CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_15_E;  profileSet++)
        {
            for (tcQueue = 0; tcQueue < CPSS_TC_RANGE_CNS; tcQueue++)
            {
                /*
                    1.1. Call with all correct parameters.
                    Expected: GT_OK.
                */
                st = cpssPxPortCnProfileQueueConfigGet(dev, profileSet, tcQueue, &cnProfileCfg);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
            }
        }

        /* Restore valid values */
        profileSet = CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_0_E;
        tcQueue = 0;

        /*
            1.2. Call with wrong enum values profileSet.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssPxPortCnProfileQueueConfigGet
                            (dev, profileSet, tcQueue, &cnProfileCfg),
                            profileSet);

        /*
            1.4. Call with wrong tcQueue [CPSS_TC_RANGE_CNS].
            Expected: NOT GT_OK.
        */
        tcQueue = CPSS_TC_RANGE_CNS;

        st = cpssPxPortCnProfileQueueConfigGet(dev, profileSet, tcQueue, &cnProfileCfg);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        tcQueue = 5;

        /*
            1.5. Call with wrong cnProfileCfgPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssPxPortCnProfileQueueConfigGet(dev, profileSet, tcQueue, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortCnProfileQueueConfigGet(dev, profileSet, tcQueue, &cnProfileCfg);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortCnProfileQueueConfigGet(dev, profileSet, tcQueue, &cnProfileCfg);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortCnAvailableBuffSet
(
    IN  GT_SW_DEV_NUM   dev,
    IN  GT_U32          availableBuff
)
*/
UTF_TEST_CASE_MAC(cpssPxPortCnAvailableBuffSet)
{
/*
    1. Go over all active devices.
    1.1. Call cpssPxPortCnAvailableBuffSet with valid parameters.
         Expected: GT_OK.
    1.2. Call cpssPxPortCnAvailableBuffGet.
         Expected: GT_OK and the same values as was set.
    1.3. Call with out of range value of availableBuff[0x100000].
         Expected: GT_OUT_OF_RANGE.
    2. Go over all not-active devices and devices from non-applicable family.
       Expected: GT_NOT_APPLICABLE_DEVICE.
    3. Call function with out of bound value of device id.
       Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st;
    GT_U8       dev;
    GT_U32      availableBuff;
    GT_U32      availableBuffGet;

    /* valid values of availableBuff */
    const GT_U32 validBuffCount[] = {
        0, 0x0F, 0xFF, 0x0FFF, 0xFFFF, 0x0FFFFF
    };
    GT_U32 countOfValidBuffCount =
                        sizeof(validBuffCount) / sizeof(validBuffCount[0]);
    GT_U32 i;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for (i = 0; i < countOfValidBuffCount; i++)
        {
            availableBuff = validBuffCount[i];

            /*
                1.1. Call cpssPxPortTxTailDropDbaAvailableBuffSet with
                     valid parameters.
                Expected: GT_OK.
            */
            st = cpssPxPortCnAvailableBuffSet(dev,
                    availableBuff);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
                1.2. Call cpssPxPortCnAvailableBuffGet with
                     valid pointer to availableBuff.
                Expected: GT_OK and the same values than was set.
            */
            st = cpssPxPortCnAvailableBuffGet(dev,
                    &availableBuffGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                    "cpssPxPortTxTailDropDbaAvailableBuffGet: %d", dev);

            /* verify values */
            UTF_VERIFY_EQUAL1_STRING_MAC(availableBuff, availableBuffGet,
                    "got another availableBuff then was set: %d", dev);
        }

        /*
            1.3. Call with out of range value of availableBuff[64].
            Expected: GT_OUT_OF_RANGE.
        */
        availableBuff = 0x100000;

        st = cpssPxPortCnAvailableBuffSet(dev, availableBuff);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, dev);
    }

    /* restore correct value */
    availableBuff = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /*    check that function returns GT_NOT_APPLICABLE_DEVICE.         */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortCnAvailableBuffSet(dev, availableBuff);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortCnAvailableBuffSet(dev, availableBuff);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortCnAvailableBuffGet
(
    IN  GT_SW_DEV_NUM   dev,
    OUT GT_U32          *availableBuffPtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPortCnAvailableBuffGet)
{
/*
    1. Go over all active devices.
    1.1. Call function with valid pointer to availableBuff.
         Expected: GT_OK.
    1.2. Call function with wrong pointer to availableBuff[NULL].
         Expected: GT_BAD_PTR.
    2. Go over all not-active devices and devices from non-applicable family.
       Expected: GT_NOT_APPLICABLE_DEVICE.
    3. Call function with out of bound value of device id.
       Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st;
    GT_U8       dev;
    GT_U32      buffCount;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with valid pointer to availableBuff.
            Expected: GT_OK.
        */
        st = cpssPxPortCnAvailableBuffGet(dev, &buffCount);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call function with wrong pointer to availableBuff[NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssPxPortCnAvailableBuffGet(dev, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /*    check that function returns GT_NOT_APPLICABLE_DEVICE.         */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortCnAvailableBuffGet(dev, &buffCount);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortCnAvailableBuffGet(dev, &buffCount);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortCnMessageTypeSet
(
    IN  GT_SW_DEV_NUM                       devNum,
    IN  CPSS_PX_PORT_CN_MESSAGE_TYPE_ENT    mType
)
*/
UTF_TEST_CASE_MAC(cpssPxPortCnMessageTypeSet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (Pipe)
    1.1. Call with message type
            CPSS_PX_PORT_CN_MESSAGE_TYPE_QCN_E,
            CPSS_PX_PORT_CN_MESSAGE_TYPE_CCFC_E
    Expected: GT_OK.
    1.2. Call cpssPxPortCnMessageTypeGet.
    Expected: GT_OK and the same enable.
*/

    GT_STATUS   st     = GT_OK;
    GT_U8       dev;
    CPSS_PX_PORT_CN_MESSAGE_TYPE_ENT    mType = CPSS_PX_PORT_CN_MESSAGE_TYPE_QCN_E;
    CPSS_PX_PORT_CN_MESSAGE_TYPE_ENT    mTypeGet;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
           1.1. Call with message type CPSS_PX_PORT_CN_MESSAGE_TYPE_QCN_E.
           Expected: GT_OK.
        */
        st = cpssPxPortCnMessageTypeSet(dev, mType);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, mType);

        /*
           1.2. Call cpssPxPortCnMessageTypeGet.
           Expected: GT_OK and the same message type.
        */
        st = cpssPxPortCnMessageTypeGet(dev, &mTypeGet);

        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                        "[cpssPxPortCnMessageTypeGet]: %d, %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(mType, mTypeGet,
                        "get another message type than was set: %d, %d", dev);

        /*
           1.1. Call with message type CPSS_PX_PORT_CN_MESSAGE_TYPE_CCFC_E;
           Expected: GT_OK.
        */

        mType = CPSS_PX_PORT_CN_MESSAGE_TYPE_CCFC_E;

        st = cpssPxPortCnMessageTypeSet(dev, mType);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, mType);

        /*
           1.2. Call cpssPxPortCnModeEnableGet.
           Expected: GT_OK and the same message type.
        */
        st = cpssPxPortCnMessageTypeGet(dev, &mTypeGet);

        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                        "[cpssPxPortCnMessageTypeGet]: %d, %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(mType, mTypeGet,
                        "get another message type than was set: %d, %d", dev);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    mType = CPSS_PX_PORT_CN_MESSAGE_TYPE_QCN_E;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortCnMessageTypeSet(dev, mType);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortCnMessageTypeSet(dev, mType);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortCnMessageTypeGet
(
    IN  GT_SW_DEV_NUM                       devNum,
    OUT CPSS_PX_PORT_CN_MESSAGE_TYPE_ENT    *mTypePtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPortCnMessageTypeGet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (Pipe)
    1.1. Call with non-null mTypePtr.
    Expected: GT_OK.
    1.2. Call with mTypePtr [NULL].
    Expected: GT_BAD_PTR
*/

    GT_STATUS   st    = GT_OK;

    GT_U8       dev;
    CPSS_PX_PORT_CN_MESSAGE_TYPE_ENT    mType = CPSS_PX_PORT_CN_MESSAGE_TYPE_QCN_E;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Call with non-null message type.
           Expected: GT_OK.
        */
        st = cpssPxPortCnMessageTypeGet(dev, &mType);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* 1.2. Call with mType[NULL].
           Expected: GT_BAD_PTR
        */
        st = cpssPxPortCnMessageTypeGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, %d, NULL", dev);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortCnMessageTypeGet(dev, &mType);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortCnMessageTypeGet(dev, &mType);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
 * Configuration of cpssPxPortCn suit
 */

UTF_SUIT_BEGIN_TESTS_MAC(cpssPxPortCn)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortCnFcEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortCnFcEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortCnFcTimerSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortCnFcTimerGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortCnModeEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortCnModeEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortCnFbCalcConfigSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortCnFbCalcConfigGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortCnMessageGenerationConfigSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortCnMessageGenerationConfigGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortCnPacketLengthSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortCnPacketLengthGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortCnQueueStatusModeEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortCnQueueStatusModeEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortCnSampleEntrySet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortCnSampleEntryGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortCnTerminationEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortCnTerminationEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortCnMessagePortMapEntrySet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortCnMessagePortMapEntryGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortCnMessageTriggeringStateSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortCnMessageTriggeringStateGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortCnProfileQueueConfigSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortCnProfileQueueConfigGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortCnAvailableBuffSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortCnAvailableBuffGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortCnMessageTypeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortCnMessageTypeGet)
UTF_SUIT_END_TESTS_MAC(cpssPxPortCn)

