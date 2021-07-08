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
* @file cpssDxChPortCnUT.c
*
* @brief Unit tests for cpssDxChPortCn, that provides
* CPSS implementation for Port Congestion Notification API.
*
* @version   25
********************************************************************************
*/
/* the define of UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC must come from C files that
   already fixed the types of ports from GT_U8 !

   NOTE: must come before ANY include to H files !!!!
*/
#define UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC

#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortCn.h>
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPort.h>
#include <cpss/dxCh/dxChxGen/cos/private/prvCpssDxChCoS.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <common/tgfCommon.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>

/* defines */

/* Default valid value for port id */
#define PORT_CN_VALID_PHY_PORT_CNS  0

/* Default invalid value for port id */
#define PORT_CN_INVALID_PHY_PORT_CNS  64

/* Default valid value for dev id */
#define PORT_CN_VALID_DEV_CNS  0

/* Default invalid value for dev id */
#define PORT_CN_INVALID_DEV_CNS  32

/* Invalid etherType value */
#define PORT_CN_INVALID_ETHERTYPE_CNS  0xFFFF+1

/* Invalid timer value */
#define PORT_CN_INVALID_TIMER_CNS      0xFFFF+1

/* Invalid xOnLimit / xOffLimit  value */
#define PORT_CN_INVALID_XONXOFFLIMIT_CNS      0x1FFF+1

/* Invalid threshold  value */
#define PORT_CN_INVALID_THRESHOLD_CNS         0x1FFF+1

/* Invalid portSpeedIndex */
#define PORT_CN_INVALID_PORTSPEEDINDEX_CNS 8

/* Invalid profileIndex */
#define PORT_CN_INVALID_PROFILEINDEX_CNS   8

/* Invalid tcQueue */
#define PORT_CN_INVALID_TCQUEUE_CNS        8

/* Invalid tcQueue for Sip6 */
#define PORT_CN_INVALID_SIP6_TCQUEUE_CNS   16

/* Invalid index */
#define PORT_CN_INVALID_INDEX_CNS        32

/* Invalid trigger threshold value */
#define PORT_CN_INVALID_TRIGGER_THRESHOLD_CNS         0x3FFF+1

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortCnEtherTypeSet
(
    IN GT_U8    devNum,
    IN GT_U32   etherType
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortCnEtherTypeSet)
{
/*
    ITERATE_DEVICES (xCat and above)
    1.1. Call with etherType [0 / 0xF0F0 / 0xFFFF].
    Expected: GT_OK.
    1.2. Call cpssDxChPortCnEtherTypeGet with not-NULL etherTypePtr.
    Expected: GT_OK and the same etherType as was set.
    1.3. Call with wrong etherType [PORT_CN_INVALID_ETHERTYPE_CNS].
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;

    GT_U32   etherType = 0;
    GT_U32   etherTypeGet = 1;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with etherType [0 / 0xF0F0 / 0xFFFF].
            Expected: GT_OK.
        */
        etherType = 0;

        st = cpssDxChPortCnEtherTypeSet(dev, etherType);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call cpssDxChPortCnEtherTypeGet with not-NULL etherTypePtr.
            Expected: GT_OK and the same etherType as was set.
        */
        st = cpssDxChPortCnEtherTypeGet(dev, &etherTypeGet);

        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChPortCnEtherTypeGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(etherType, etherTypeGet,
                                     "got another etherType then was set: %d", dev);

        /*
            1.1. Call with etherType [0 / 0xF0F0 / 0xFFFF].
            Expected: GT_OK.
        */
        etherType = 0xF0F0;

        st = cpssDxChPortCnEtherTypeSet(dev, etherType);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call cpssDxChPortCnEtherTypeGet with not-NULL etherTypePtr.
            Expected: GT_OK and the same etherType as was set.
        */
        st = cpssDxChPortCnEtherTypeGet(dev, &etherTypeGet);

        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChPortCnEtherTypeGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(etherType, etherTypeGet,
                                     "got another etherType then was set: %d", dev);


        /*
            1.1. Call with etherType [0 / 0xF0F0 / 0xFFFF].
            Expected: GT_OK.
        */
        etherType = 0xFFFF;

        st = cpssDxChPortCnEtherTypeSet(dev, etherType);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call cpssDxChPortCnEtherTypeGet with not-NULL etherTypePtr.
            Expected: GT_OK and the same etherType as was set.
        */
        st = cpssDxChPortCnEtherTypeGet(dev, &etherTypeGet);

        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChPortCnEtherTypeGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(etherType, etherTypeGet,
                                     "got another etherType then was set: %d", dev);

        /*
            1.3. Call with wrong etherType [PORT_CN_INVALID_ETHERTYPE_CNS].
            Expected: GT_OUT_OF_RANGE.
        */
        etherType = PORT_CN_INVALID_ETHERTYPE_CNS;

        st = cpssDxChPortCnEtherTypeSet(dev, etherType);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, dev);

        etherType = 0;
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortCnEtherTypeSet(dev, etherType);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortCnEtherTypeSet(dev, etherType);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortCnEtherTypeGet
(
    IN  GT_U8           devNum,
    OUT GT_U32         *etherTypePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortCnEtherTypeGet)
{
/*
    ITERATE_DEVICES (xCat and above)
    1.1. Call with not-NULL etherTypePtr
    Expected: GT_OK.
    1.2. Call with wrong  etherTypePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;

    GT_U32      etherType;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with not-NULL etherType.
            Expected: GT_OK.
        */
        st = cpssDxChPortCnEtherTypeGet(dev, &etherType);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with etherType[NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChPortCnEtherTypeGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, etherType = NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortCnEtherTypeGet(dev, &etherType);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortCnEtherTypeGet(dev, &etherType);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortCnFcEnableSet
(
    IN  GT_U8     devNum,
    IN  GT_U8     portNum,
    IN  GT_BOOL   enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortCnFcEnableSet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (xCat,Lion,xCat2)
    1.1.1. Call with state [GT_FALSE and GT_TRUE].
    Expected: GT_OK.
    1.1.2. Call cpssDxChPortCnFcEnableGet.
    Expected: GT_OK and the same enable.
*/

    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM  port   = PORT_CN_VALID_PHY_PORT_CNS;
    GT_BOOL     enable = GT_FALSE;
    GT_BOOL     state  = GT_FALSE;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_FALCON_E);

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

            st = cpssDxChPortCnFcEnableSet(dev, port, enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);

            /*
               1.1.2. Call cpssDxChPortCnFcEnableGet.
               Expected: GT_OK and the same enable.
            */
            st = cpssDxChPortCnFcEnableGet(dev, port, &state);

            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                            "[cpssDxChPortCnFcEnableGet]: %d, %d", dev, port);
            UTF_VERIFY_EQUAL2_STRING_MAC(enable, state,
                            "get another enable value than was set: %d, %d", dev, port);

            /*
               1.1.1. Call with state [GT_FALSE and GT_TRUE].
               Expected: GT_OK.
            */

            /* Call function with enable = GT_TRUE] */
            enable = GT_TRUE;

            st = cpssDxChPortCnFcEnableSet(dev, port, enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);

            /*
               1.1.2. Call cpssDxChPortCnFcEnableGet.
               Expected: GT_OK and the same enable.
            */
            st = cpssDxChPortCnFcEnableGet(dev, port, &state);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                          "[cpssDxChPortCnFcEnableGet]: %d, %d", dev, port);
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
            st = cpssDxChPortCnFcEnableSet(dev, port, enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        /* enable == GT_TRUE  */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPortCnFcEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                     */
        /* enable == GT_TRUE  */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChPortCnFcEnableSet(dev, port, enable);
        if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev) == GT_TRUE)
        {
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);
        }
        else
        {
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, port, enable);
        }
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    enable = GT_TRUE;
    port = PORT_CN_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_FALCON_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortCnFcEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    /* port == 0, enable == GT_TRUE */

    st = cpssDxChPortCnFcEnableSet(dev, port, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortCnFcEnableGet
(
    IN   GT_U8     devNum,
    IN   GT_U8     portNum,
    OUT  GT_BOOL   *statePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortCnFcEnableGet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (xCat,Lion,xCat2)
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
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_FALCON_E);

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
            st = cpssDxChPortCnFcEnableGet(dev, port, &state);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* 1.1.2. Call with statePtr [NULL].
               Expected: GT_BAD_PTR
            */
            st = cpssDxChPortCnFcEnableGet(dev, port, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, NULL", dev, port);
        }

        /* 1.2. For all active devices go over all non available
           physical ports.
        */
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_FALSE))
        {
            st = cpssDxChPortCnFcEnableGet(dev, port, &state);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPortCnFcEnableGet(dev, port, &state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChPortCnFcEnableGet(dev, port, &state);
        if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev) == GT_TRUE)
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    port = PORT_CN_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_FALCON_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortCnFcEnableGet(dev, port, &state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortCnFcEnableGet(dev, port, &state);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortCnFcTimerSet
(
    IN GT_U8      devNum,
    IN GT_U8      portNum,
    IN GT_U32     portSpeedIndex,
    IN GT_U32     timer
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortCnFcTimerSet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (xCat and above)
    1.1.1. Call with portSpeedIndex [0 / 7], timer[0 / 0xFFFF].
    Expected: GT_OK.
    1.1.2. Call cpssDxChPortCnFcTimerGet with the same params.
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
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_FALCON_E);

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

            st = cpssDxChPortCnFcTimerSet(dev, port, portSpeedIndex, timer);
            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, port, portSpeedIndex, timer);

            /*
               1.1.2. Call cpssDxChPortCnFcTimerGet with the same params.
               Expected: GT_OK and the same timer.
            */
            st = cpssDxChPortCnFcTimerGet(dev, port, portSpeedIndex, &timerGet);

            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                            "[cpssDxChPortCnFcTimerGet]: %d, %d", dev, port);
#ifdef ASIC_SIMULATION
            if(prvCpssDxChPortRemotePortCheck(dev, port) == GT_FALSE)
#endif  /* ASIC_SIMULATION */
            {
                UTF_VERIFY_EQUAL2_STRING_MAC(timer, timerGet,
                          "get another timer value than was set: %d, %d", dev, port);
            }

            /*
               1.1.1. Call with portSpeedIndex [0 / 7], timer[0 / 0xFFFF].
               Expected: GT_OK.
            */

            /*call with portSpeedIndex [7] and timer[0xFFFF]*/
            portSpeedIndex = 7;
            if(prvCpssDxChPortRemotePortCheck(dev, port))
            {
                portSpeedIndex = 3;
            }
            timer = 0xFFFF;

            st = cpssDxChPortCnFcTimerSet(dev, port, portSpeedIndex, timer);
            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, port, portSpeedIndex, timer);

            /*
               1.1.2. Call cpssDxChPortCnFcTimerGet with the same params.
               Expected: GT_OK and the same timer.
            */
            st = cpssDxChPortCnFcTimerGet(dev, port, portSpeedIndex, &timerGet);

            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                            "[cpssDxChPortCnFcTimerGet]: %d, %d", dev, port);
            UTF_VERIFY_EQUAL2_STRING_MAC(timer, timerGet,
                      "get another timer value than was set: %d, %d", dev, port);

            /*
               1.1.3. Call with wrong portSpeedIndex [PORT_CN_INVALID_PORTSPEEDINDEX_CNS].
               Expected: GT_BAD_PARAM.
            */
            st = cpssDxChPortCnFcTimerSet(dev, port, PORT_CN_INVALID_PORTSPEEDINDEX_CNS, timer);
            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_BAD_PARAM, st, dev, port, PORT_CN_INVALID_PORTSPEEDINDEX_CNS, timer);

            /*
               1.1.4. Call with wrong timer [PORT_CN_INVALID_TIMER_CNS]
               Expected: GT_OUT_OF_RANGE.
            */
            timer = PORT_CN_INVALID_TIMER_CNS;

            st = cpssDxChPortCnFcTimerSet(dev, port, portSpeedIndex, timer);
            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OUT_OF_RANGE, st, dev, port, portSpeedIndex, timer);

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
            st = cpssDxChPortCnFcTimerSet(dev, port, portSpeedIndex, timer);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPortCnFcTimerSet(dev, port, portSpeedIndex, timer);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                     */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChPortCnFcTimerSet(dev, port, portSpeedIndex, timer);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_BAD_PARAM, st, dev, port, portSpeedIndex, timer);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_FALCON_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortCnFcTimerSet(dev, port, portSpeedIndex, timer);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    /* port == 0, enable == GT_TRUE */

    st = cpssDxChPortCnFcTimerSet(dev, port, portSpeedIndex, timer);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortCnFcTimerGet
(
    IN  GT_U8    devNum,
    IN  GT_U8    portNum,
    IN  GT_U32   portSpeedIndex,
    OUT GT_U32   *timerPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortCnFcTimerGet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (xCat and above)
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
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_FALCON_E);

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

            st = cpssDxChPortCnFcTimerGet(dev, port, portSpeedIndex, &timerGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*
               1.1.1. Call with portSpeedIndex [0 / 7] and  non-null timerPtr.
               Expected: GT_OK.
            */

            portSpeedIndex = 7;
            if(prvCpssDxChPortRemotePortCheck(dev, port))
            {
                portSpeedIndex = 3;
            }

            st = cpssDxChPortCnFcTimerGet(dev, port, portSpeedIndex, &timerGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*
                1.1.2. Call with wrong  portSpeedIndex [PORT_CN_INVALID_PORTSPEEDINDEX_CNS]
                       and  non-null timerPtr.
                Expected: GT_BAD_PARAM.
            */

            st = cpssDxChPortCnFcTimerGet(dev, port, PORT_CN_INVALID_PORTSPEEDINDEX_CNS, &timerGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

            /*
                1.1.3. Call with wrong timerPtr [NULL].
                Expected: GT_BAD_PTR
            */
            st = cpssDxChPortCnFcTimerGet(dev, port, portSpeedIndex,  NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, NULL", dev, port);
        }

        /* 1.2. For all active devices go over all non available
           physical ports.
        */
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            st = cpssDxChPortCnFcTimerGet(dev, port, portSpeedIndex, &timerGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPortCnFcTimerGet(dev, port, portSpeedIndex, &timerGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChPortCnFcTimerGet(dev, port, portSpeedIndex, &timerGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    port = PORT_CN_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_FALCON_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortCnFcTimerGet(dev, port, portSpeedIndex, &timerGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortCnFcTimerGet(dev, port, portSpeedIndex, &timerGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortCnFrameQueueSet
(
    IN GT_U8    devNum,
    IN GT_U8    tcQueue
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortCnFrameQueueSet)
{
/*
    ITERATE_DEVICES (xCat and above)
    1.1. Call with tcQueue [0 / 7].
    Expected: GT_OK.
    1.2. Call cpssDxChPortCnFrameQueueGet with not-NULL tcQueuePtr.
    Expected: GT_OK and the same tcQueue as was set.
    1.3. Call with wrong tcQueue [PORT_CN_INVALID_TCQUEUE_CNS].
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;

    GT_U8   tcQueue = 0;
    GT_U8   tcQueueGet = 1;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
        UTF_CPSS_PP_E_ARCH_CNS);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with tcQueue [0 / 7].
            Expected: GT_OK.
        */
        tcQueue = 0;

        st = cpssDxChPortCnFrameQueueSet(dev, tcQueue);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call cpssDxChPortCnFrameQueueGet with not-NULL tcQueuePtr.
            Expected: GT_OK and the same tcQueue as was set.
        */
        st = cpssDxChPortCnFrameQueueGet(dev, &tcQueueGet);

        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChPortCnFrameQueueGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(tcQueue, tcQueueGet,
                                     "got another tcQueue then was set: %d", dev);

        /*
            1.1. Call with tcQueue [0 / 7].
            Expected: GT_OK.
        */
        tcQueue = 7;

        st = cpssDxChPortCnFrameQueueSet(dev, tcQueue);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call cpssDxChPortCnFrameQueueGet with not-NULL tcQueuePtr.
            Expected: GT_OK and the same tcQueue as was set.
        */
        st = cpssDxChPortCnFrameQueueGet(dev, &tcQueueGet);

        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChPortCnFrameQueueGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(tcQueue, tcQueueGet,
                                     "got another tcQueue then was set: %d", dev);

        /*
            1.3. Call with wrong tcQueue [PORT_CN_INVALID_TCQUEUE_CNS].
            Expected: GT_BAD_PARAM.
        */
        tcQueue = PORT_CN_INVALID_TCQUEUE_CNS;

        st = cpssDxChPortCnFrameQueueSet(dev, tcQueue);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        tcQueue = 0;
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E|
        UTF_CPSS_PP_E_ARCH_CNS);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortCnFrameQueueSet(dev, tcQueue);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortCnFrameQueueSet(dev, tcQueue);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortCnFrameQueueGet
(
    IN  GT_U8           devNum,
    OUT GT_U8          *tcQueuePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortCnFrameQueueGet)
{
/*
    ITERATE_DEVICES (xCat and above)
    1.1. Call with not-NULL tcQueuePtr
    Expected: GT_OK.
    1.2. Call with wrong  tcQueuePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;

    GT_U8      tcQueue = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E|
        UTF_CPSS_PP_E_ARCH_CNS);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with not-NULL tcQueue.
            Expected: GT_OK.
        */
        st = cpssDxChPortCnFrameQueueGet(dev, &tcQueue);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with tcQueue[NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChPortCnFrameQueueGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, tcQueue = NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E|
        UTF_CPSS_PP_E_ARCH_CNS);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortCnFrameQueueGet(dev, &tcQueue);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortCnFrameQueueGet(dev, &tcQueue);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortCnModeEnableSet
(
    IN  GT_U8     devNum,
    IN  GT_BOOL   enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortCnModeEnableSet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (xCat and above)
    1.1. Call with state
            CPSS_DXCH_PORT_CN_MODE_DISABLE_E = GT_FALSE,
            CPSS_DXCH_PORT_CN_MODE_ENABLE_E  = GT_TRUE,
            Bobcat3 only: CPSS_DXCH_PORT_CN_MODE_REACTION_POINT_E = 2
    Expected: GT_OK.
    1.2. Call cpssDxChPortCnModeEnableGet.
    Expected: GT_OK and the same enable.
*/

    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    CPSS_DXCH_PORT_CN_MODE_ENT enable = GT_FALSE;
    CPSS_DXCH_PORT_CN_MODE_ENT state  = GT_FALSE;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
           1.1. Call with state CPSS_DXCH_PORT_CN_MODE_DISABLE_E.
           Expected: GT_OK.
        */

        enable = CPSS_DXCH_PORT_CN_MODE_DISABLE_E;

        st = cpssDxChPortCnModeEnableSet(dev, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, enable);

        /*
           1.2. Call cpssDxChPortCnModeEnableGet.
           Expected: GT_OK and the same enable.
        */
        st = cpssDxChPortCnModeEnableGet(dev, &state);

        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                        "[cpssDxChPortCnModeEnableGet]: %d, %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(enable, state,
                        "get another enable value than was set: %d, %d", dev);

        /*
           1.1. Call with state CPSS_DXCH_PORT_CN_MODE_ENABLE_E.
           Expected: GT_OK.
        */

        enable = CPSS_DXCH_PORT_CN_MODE_ENABLE_E;

        st = cpssDxChPortCnModeEnableSet(dev, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, enable);

        /*
           1.2. Call cpssDxChPortCnModeEnableGet.
           Expected: GT_OK and the same enable.
        */
        st = cpssDxChPortCnModeEnableGet(dev, &state);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                      "[cpssDxChPortCnModeEnableGet]: %d, %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(enable, state,
                      "get another enable value than was set: %d, %d", dev);

        /*
           1.1. Call with state CPSS_DXCH_PORT_CN_MODE_REACTION_POINT_E.
           Expected: GT_OK for bc3 and GT_BAD_PARAM in other case.
        */

        enable = CPSS_DXCH_PORT_CN_MODE_REACTION_POINT_E;

        st = cpssDxChPortCnModeEnableSet(dev, enable);

        if(PRV_CPSS_SIP_5_20_CHECK_MAC(dev))
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, enable);

            /*
               1.2. Call cpssDxChPortCnModeEnableGet.
               Expected: GT_OK and the same enable.
            */
            st = cpssDxChPortCnModeEnableGet(dev, &state);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                          "[cpssDxChPortCnModeEnableGet]: %d, %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(enable, state,
                          "get another enable value than was set: %d, %d", dev);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, enable);
        }
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    enable = CPSS_DXCH_PORT_CN_MODE_ENABLE_E;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortCnModeEnableSet(dev, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortCnModeEnableSet(dev, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortCnModeEnableGet
(
    IN   GT_U8     devNum,
    OUT  GT_BOOL   *statePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortCnModeEnableGet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (xCat and above)
    1.1. Call with non-null statePtr.
    Expected: GT_OK.
    1.2. Call with statePtr [NULL].
    Expected: GT_BAD_PTR
*/

    GT_STATUS   st    = GT_OK;

    GT_U8       dev;
    CPSS_DXCH_PORT_CN_MODE_ENT state = GT_FALSE;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Call with non-null statePtr.
           Expected: GT_OK.
        */
        st = cpssDxChPortCnModeEnableGet(dev, &state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* 1.2. Call with statePtr [NULL].
           Expected: GT_BAD_PTR
        */
        st = cpssDxChPortCnModeEnableGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, %d, NULL", dev);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortCnModeEnableGet(dev, &state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortCnModeEnableGet(dev, &state);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortCnPanicPauseThresholdsSet
(
    IN GT_U8    devNum,
    IN GT_BOOL  enable,
    IN GT_U32   xoffLimit,
    IN GT_U32   xonLimit
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortCnPanicPauseThresholdsSet)
{
/*
    ITERATE_DEVICES (xCat,xCat2)
    1.1. Call with enable [GT_FALSE and GT_TRUE], xoffLimit [0 / 0x1FFF],
                  xonLimit  [0 / 0x1FFF].
    Expected: GT_OK.
    1.2. Call cpssDxChPortCnPanicPauseThresholdsGet with the same params.
    Expected: GT_OK and the same values.
    1.3. Call with enable [GT_FALSE],
                  xoffLimit [PORT_CN_INVALID_XONXOFFLIMIT_CNS],
                  xonLimit  [0].
    Expected: GT_OK.
    1.4. Call with enable [GT_FALSE], xoffLimit [0],
                  xonLimit  [PORT_CN_INVALID_XONXOFFLIMIT_CNS].
    Expected: GT_OK.
    1.5. Call with enable [GT_TRUE],
                  xoffLimit [PORT_CN_INVALID_XONXOFFLIMIT_CNS],
                  xonLimit  [0].
    Expected: GT_OUT_OF_RANGE.
    1.6. Call with enable [GT_TRUE], xoffLimit [0],
                  xonLimit  [PORT_CN_INVALID_XONXOFFLIMIT_CNS].
    Expected: GT_OUT_OF_RANGE.
*/

    GT_STATUS   st     = GT_OK;

    GT_U8    dev;
    GT_BOOL  enable = GT_FALSE;
    GT_U32   xoffLimit = 0;
    GT_U32   xonLimit = 0;

    GT_BOOL  enableGet = GT_FALSE;
    GT_U32   xoffLimitGet = 0;
    GT_U32   xonLimitGet = 0;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_LION_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
           1.1. Call with enable [GT_FALSE and GT_TRUE], xoffLimit [0 /0x1FFF],
                          xonLimit  [0 /0x1FFF].
           Expected: GT_OK.
        */

        enable = GT_FALSE;
        xoffLimit = 0;
        xonLimit = 0;

        st = cpssDxChPortCnPanicPauseThresholdsSet(dev, enable, xoffLimit, xonLimit);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, enable);

        /* the functionality does not have "enable" bit.
           The "disable" is simulated by set limits to maximal value 0x1FFF. */
        xoffLimit = 0x1FFF;
        xonLimit = 0x1FFF;

        /*
            1.2. Call cpssDxChPortCnPanicPauseThresholdsGet with the same params.
            Expected: GT_OK and the same enable.
        */
        st = cpssDxChPortCnPanicPauseThresholdsGet(dev, &enableGet,
                                                   &xoffLimitGet, &xonLimitGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                        "[cpssDxChPortCnPanicPauseThresholdsGet]: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                        "get another enable value than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(xoffLimit, xoffLimitGet,
                        "get another xoffLimit value than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(xonLimit, xonLimitGet,
                        "get another xonLimit value than was set: %d", dev);

        /*
           1.1. Call with enable [GT_FALSE and GT_TRUE],
                          xoffLimit [0 /0x1FFF],
                          xonLimit  [0 /0x1FFF].
           Expected: GT_OK.
        */

        enable = GT_TRUE;
        xoffLimit = 0x1FFF;
        xonLimit = 0x1FFF;

        st = cpssDxChPortCnPanicPauseThresholdsSet(dev, enable, xoffLimit, xonLimit);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, enable);

        /* the functionality does not have "enable" bit.
           The "disable" is simulated by set limits to maximal value 0x1FFF.
           So Get function return enable = GT_FALSE */
        enable = GT_FALSE;

        /*
            1.2. Call cpssDxChPortCnPanicPauseThresholdsGet with the same params.
            Expected: GT_OK and the same enable.
        */
        st = cpssDxChPortCnPanicPauseThresholdsGet(dev, &enableGet,
                                                   &xoffLimitGet, &xonLimitGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                        "[cpssDxChPortCnPanicPauseThresholdsGet]: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                        "get another enable value than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(xoffLimit, xoffLimitGet,
                        "get another xoffLimit value than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(xonLimit, xonLimitGet,
                        "get another xonLimit value than was set: %d", dev);

        /*
            1.3. Call with enable [GT_FALSE],
                          xoffLimit [PORT_CN_INVALID_XONXOFFLIMIT_CNS],
                          xonLimit  [0].
            Expected: GT_OK.
        */

        enable = GT_FALSE;
        xoffLimit = PORT_CN_INVALID_XONXOFFLIMIT_CNS;
        xonLimit = 0;

        st = cpssDxChPortCnPanicPauseThresholdsSet(dev, enable, xoffLimit, xonLimit);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, enable);

        xoffLimit = 0;

        /*
            1.4. Call with enable [GT_FALSE],
                          xoffLimit [0],
                          xonLimit  [PORT_CN_INVALID_XONXOFFLIMIT_CNS].
            Expected: GT_OK.
        */

        enable = GT_FALSE;
        xoffLimit = 0;
        xonLimit = PORT_CN_INVALID_XONXOFFLIMIT_CNS;

        st = cpssDxChPortCnPanicPauseThresholdsSet(dev, enable, xoffLimit, xonLimit);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, enable);

        xonLimit = 0;

        /*
            1.5. Call with enable [GT_TRUE],
                          xoffLimit [PORT_CN_INVALID_XONXOFFLIMIT_CNS],
                          xonLimit  [0].
            Expected: GT_OUT_OF_RANGE.
        */

        enable = GT_TRUE;
        xoffLimit = PORT_CN_INVALID_XONXOFFLIMIT_CNS;
        xonLimit = 0;

        st = cpssDxChPortCnPanicPauseThresholdsSet(dev, enable, xoffLimit, xonLimit);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OUT_OF_RANGE, st, dev, enable);

        xoffLimit = 0;

        /*
            1.6. Call with enable [GT_TRUE], xoffLimit [0],
                          xonLimit  [PORT_CN_INVALID_XONXOFFLIMIT_CNS].
            Expected: GT_OUT_OF_RANGE.
        */

        enable = GT_TRUE;
        xoffLimit = 0;
        xonLimit = PORT_CN_INVALID_XONXOFFLIMIT_CNS;

        st = cpssDxChPortCnPanicPauseThresholdsSet(dev, enable, xoffLimit, xonLimit);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OUT_OF_RANGE, st, dev, enable);

        xonLimit = 0;
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    enable = GT_TRUE;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_LION_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortCnPanicPauseThresholdsSet(dev, enable, xoffLimit, xonLimit);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortCnPanicPauseThresholdsSet(dev, enable, xoffLimit, xonLimit);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortCnPanicPauseThresholdsGet
(
    IN  GT_U8    devNum,
    OUT GT_BOOL  *enablePtr,
    OUT GT_U32   *xoffLimitPtr,
    OUT GT_U32   *xonLimitPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortCnPanicPauseThresholdsGet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (xCat,xCat2)
    1.1. Call with non-null pointers.
    Expected: GT_OK.
    1.2. Call with enablePtr [NULL].
    Expected: GT_BAD_PTR
    1.3. Call with xoffLimitPtr [NULL].
    Expected: GT_BAD_PTR
    1.4. Call with xonLimitPtr [NULL].
    Expected: GT_BAD_PTR
*/

    GT_STATUS   st    = GT_OK;

    GT_U8    dev;
    GT_BOOL  enableGet;
    GT_U32   xoffLimitGet;
    GT_U32   xonLimitGet;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_LION_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
           1.1. Call with non-null statePtr.
           Expected: GT_OK.
        */
        st = cpssDxChPortCnPanicPauseThresholdsGet(dev, &enableGet,
                                              &xoffLimitGet, &xonLimitGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
           1.2. Call with enableGet [NULL].
           Expected: GT_BAD_PTR.
        */
        st = cpssDxChPortCnPanicPauseThresholdsGet(dev, NULL,
                                              &xoffLimitGet, &xonLimitGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, enable is NULL", dev);

        /*
           1.3. Call with xoffLimitGet [NULL].
           Expected: GT_BAD_PTR.
        */
        st = cpssDxChPortCnPanicPauseThresholdsGet(dev, &enableGet,
                                              NULL, &xonLimitGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, xoffLimitGet is NULL", dev);

        /*
           1.4. Call with xonLimitGet [NULL].
           Expected: GT_BAD_PTR.
        */
        st = cpssDxChPortCnPanicPauseThresholdsGet(dev, &enableGet,
                                              &xoffLimitGet, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, xonLimitGet is NULL", dev);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_LION_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortCnPanicPauseThresholdsGet(dev, &enableGet, &xoffLimitGet, &xonLimitGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortCnPanicPauseThresholdsGet(dev, &enableGet, &xoffLimitGet, &xonLimitGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortCnPauseTriggerEnableSet
(
    IN  GT_U8     devNum,
    IN  GT_U8     portNum,
    IN  GT_BOOL   enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortCnPauseTriggerEnableSet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (xCat,xCat2)
    1.1.1. Call with state [GT_FALSE and GT_TRUE].
    Expected: GT_OK.
    1.1.2. Call cpssDxChPortCnPauseTriggerEnableGet.
    Expected: GT_OK and the same enable.
*/

    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM  port   = PORT_CN_VALID_PHY_PORT_CNS;
    GT_BOOL     enable = GT_FALSE;
    GT_BOOL     state  = GT_FALSE;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_LION_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

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

            st = cpssDxChPortCnPauseTriggerEnableSet(dev, port, enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);

            /*
               1.1.2. Call cpssDxChPortCnPauseTriggerEnableGet.
               Expected: GT_OK and the same enable.
            */
            st = cpssDxChPortCnPauseTriggerEnableGet(dev, port, &state);

            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                            "[cpssDxChPortCnPauseTriggerEnableGet]: %d, %d", dev, port);
            UTF_VERIFY_EQUAL2_STRING_MAC(enable, state,
                            "get another enable value than was set: %d, %d", dev, port);

            /*
               1.1.1. Call with state [GT_FALSE and GT_TRUE].
               Expected: GT_OK.
            */

            /* Call function with enable = GT_TRUE] */
            enable = GT_TRUE;

            st = cpssDxChPortCnPauseTriggerEnableSet(dev, port, enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);

            /*
               1.1.2. Call cpssDxChPortCnPauseTriggerEnableGet.
               Expected: GT_OK and the same enable.
            */
            st = cpssDxChPortCnPauseTriggerEnableGet(dev, port, &state);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                          "[cpssDxChPortCnPauseTriggerEnableGet]: %d, %d", dev, port);
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
            st = cpssDxChPortCnPauseTriggerEnableSet(dev, port, enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        /* enable == GT_TRUE  */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPortCnPauseTriggerEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                     */
        /* enable == GT_TRUE  */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChPortCnPauseTriggerEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, port, enable);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    enable = GT_TRUE;
    port = PORT_CN_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_LION_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortCnPauseTriggerEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    /* port == 0, enable == GT_TRUE */

    st = cpssDxChPortCnPauseTriggerEnableSet(dev, port, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortCnPauseTriggerEnableGet
(
    IN   GT_U8     devNum,
    IN   GT_U8     portNum,
    OUT  GT_BOOL   *statePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortCnPauseTriggerEnableGet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (xCat,xCat2)
    1.1.1. Call with non-null statePtr.
    Expected: GT_OK.
    1.1.2. Call with statePtr [NULL].
    Expected: GT_BAD_PTR
*/

    GT_STATUS   st    = GT_OK;

    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM port  = PORT_CN_VALID_PHY_PORT_CNS;
    GT_BOOL     state = GT_FALSE;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_LION_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

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
            st = cpssDxChPortCnPauseTriggerEnableGet(dev, port, &state);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* 1.1.2. Call with statePtr [NULL].
               Expected: GT_BAD_PTR
            */
            st = cpssDxChPortCnPauseTriggerEnableGet(dev, port, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, NULL", dev, port);
        }

        /* 1.2. For all active devices go over all non available
           physical ports.
        */
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_FALSE))
        {
            st = cpssDxChPortCnPauseTriggerEnableGet(dev, port, &state);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPortCnPauseTriggerEnableGet(dev, port, &state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChPortCnPauseTriggerEnableGet(dev, port, &state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    port = PORT_CN_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_LION_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortCnPauseTriggerEnableGet(dev, port, &state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortCnPauseTriggerEnableGet(dev, port, &state);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortCnProfileSet
(
    IN GT_U8    devNum,
    IN GT_U8    portNum,
    IN GT_U32   profileIndex
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortCnProfileSet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (xCat,xCat2)
    1.1.1. Call with profileIndex [0 / 7].
    Expected: GT_OK.
    1.1.2. Call cpssDxChPortCnProfileGet with the same params.
    Expected: GT_OK and the same profileIndex.
    1.1.3. Call with profileIndex [PORT_CN_INVALID_PROFILEINDEX_CNS].
    Expected: GT_OUT_OF_RANGE.
*/

    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM  port   = PORT_CN_VALID_PHY_PORT_CNS;
    GT_U32      profileIndex = 0;
    GT_U32      profileIndexGet = 0;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_LION_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call with profileIndex [0 / 7].
                Expected: GT_OK.
            */

            profileIndex = 0;

            st = cpssDxChPortCnProfileSet(dev, port, profileIndex);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, profileIndex);

            /*
               1.1.2. Call cpssDxChPortCnProfileGet.
               Expected: GT_OK and the same profileIndex.
            */
            st = cpssDxChPortCnProfileGet(dev, port, &profileIndexGet);

            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                            "[cpssDxChPortCnProfileGet]: %d, %d", dev, port);
            UTF_VERIFY_EQUAL2_STRING_MAC(profileIndex, profileIndexGet,
                            "get another profileIndex value than was set: %d, %d", dev, port);

            /*
                1.1.1. Call with profileIndex [0 / 7].
                Expected: GT_OK.
            */

            /* Call function with profileIndex = GT_TRUE] */
            profileIndex = 7;

            st = cpssDxChPortCnProfileSet(dev, port, profileIndex);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, profileIndex);

            /*
               1.1.2. Call cpssDxChPortCnProfileGet.
               Expected: GT_OK and the same profileIndex.
            */
            st = cpssDxChPortCnProfileGet(dev, port, &profileIndexGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                          "[cpssDxChPortCnProfileGet]: %d, %d", dev, port);
            UTF_VERIFY_EQUAL2_STRING_MAC(profileIndex, profileIndexGet,
                          "get another profileIndex value than was set: %d, %d", dev, port);
        }

        /*
           1.2. For all active devices go over all non available
           physical ports.
        */

        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            /* profileIndex == GT_TRUE    */
            st = cpssDxChPortCnProfileSet(dev, port, profileIndex);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPortCnProfileSet(dev, port, profileIndex);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                     */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChPortCnProfileSet(dev, port, profileIndex);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, port, profileIndex);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    profileIndex = GT_TRUE;
    port = PORT_CN_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_LION_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortCnProfileSet(dev, port, profileIndex);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortCnProfileSet(dev, port, profileIndex);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortCnProfileGet
(
    IN  GT_U8   devNum,
    IN  GT_U8   portNum,
    OUT GT_U32  *profileIndexPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortCnProfileGet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (xCat,xCat2)
    1.1.1. Call with non-null profileIndexGetPtr.
    Expected: GT_OK.
    1.1.2. Call with profileIndexGetPtr [NULL].
    Expected: GT_BAD_PTR
*/

    GT_STATUS   st    = GT_OK;

    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM  port  = PORT_CN_VALID_PHY_PORT_CNS;
    GT_U32      profileIndexGet = 0;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_LION_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            /* 1.1.1. Call with non-null profileIndexGetPtr.
               Expected: GT_OK.
            */
            st = cpssDxChPortCnProfileGet(dev, port, &profileIndexGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* 1.1.2. Call with profileIndexGetPtr [NULL].
               Expected: GT_BAD_PTR
            */
            st = cpssDxChPortCnProfileGet(dev, port, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, NULL", dev, port);
        }

        /* 1.2. For all active devices go over all non available
           physical ports.
        */
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_FALSE))
        {
            st = cpssDxChPortCnProfileGet(dev, port, &profileIndexGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPortCnProfileGet(dev, port, &profileIndexGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChPortCnProfileGet(dev, port, &profileIndexGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    port = PORT_CN_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_LION_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortCnProfileGet(dev, port, &profileIndexGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortCnProfileGet(dev, port, &profileIndexGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortCnProfileQueueThresholdSet
(
    IN GT_U8    devNum,
    IN GT_U32   profileIndex,
    IN GT_U8    tcQueue,
    IN GT_U32   threshold
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortCnProfileQueueThresholdSet)
{
/*
    ITERATE_DEVICES (xCat,xCat2)
    1.1. Call with profileIndex [0 / 3 / 7], tcQueue [0 / 3 / 7]
         and threshold [0 / 0x1F00 / 0x1FFF].
    Expected: GT_OK.
    1.2. Call cpssDxChPortCnProfileQueueThresholdGet with not-NULL thresholdPtr.
    Expected: GT_OK and the same threshold as was set.
    1.3. Call with wrong profileIndex [PORT_CN_INVALID_PROFILEINDEX_CNS].
    Expected: GT_BAD_PARAM.
    1.4. Call with wrong tcQueue [PORT_CN_INVALID_TCQUEUE_CNS].
    Expected: GT_BAD_PARAM.
    1.5. Call with wrong threshold [PORT_CN_INVALID_THRESHOLD_CNS].
    Expected: GT_OUT_OF_RANGE.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;

    GT_U32   profileIndex = 0;
    GT_U8    tcQueue = 0;
    GT_U32   threshold = 0;
    GT_U32   thresholdGet = 1;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_LION_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with profileIndex [0 / 3 / 7], tcQueue [0 / 3 / 7]
                 and threshold [0 / 0x1F00 / 0x1FFF].
            Expected: GT_OK.
        */
        /*Call with profileIndex [0], tcQueue [0] threshold [0].*/
        profileIndex = 0;
        tcQueue = 0;
        threshold = 0;

        st = cpssDxChPortCnProfileQueueThresholdSet(dev, profileIndex, tcQueue, threshold);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call cpssDxChPortCnProfileQueueThresholdGet with not-NULL thresholdPtr.
            Expected: GT_OK and the same threshold as was set.
        */
        st = cpssDxChPortCnProfileQueueThresholdGet(dev, profileIndex, tcQueue, &thresholdGet);

        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                "cpssDxChPortCnProfileQueueThresholdGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(threshold, thresholdGet,
                                "got another threshold then was set: %d", dev);

        /*
            1.1. Call with profileIndex [0 / 3 / 7], tcQueue [0 / 3 / 7]
                 and threshold [0 / 0x1F00 / 0x1FFF].
            Expected: GT_OK.
        */
        /*Call with profileIndex [3], tcQueue [3] threshold [0x1F00].*/
        profileIndex = 3;
        tcQueue = 3;
        threshold = 0x1F00;

        st = cpssDxChPortCnProfileQueueThresholdSet(dev, profileIndex, tcQueue, threshold);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call cpssDxChPortCnProfileQueueThresholdGet with not-NULL thresholdPtr.
            Expected: GT_OK and the same threshold as was set.
        */
        st = cpssDxChPortCnProfileQueueThresholdGet(dev, profileIndex, tcQueue, &thresholdGet);

        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                "cpssDxChPortCnProfileQueueThresholdGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(threshold, thresholdGet,
                                "got another threshold then was set: %d", dev);

        /*
            1.1. Call with profileIndex [0 / 3 / 7], tcQueue [0 / 3 / 7]
                 and threshold [0 / 0x1F00 / 0x1FFF].
            Expected: GT_OK.
        */
        /*Call with profileIndex [7], tcQueue [7] threshold [0x1FFF].*/
        profileIndex = 7;
        tcQueue = 7;
        threshold = 0x1FFF;

        st = cpssDxChPortCnProfileQueueThresholdSet(dev, profileIndex, tcQueue, threshold);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call cpssDxChPortCnProfileQueueThresholdGet with not-NULL thresholdPtr.
            Expected: GT_OK and the same threshold as was set.
        */
        st = cpssDxChPortCnProfileQueueThresholdGet(dev, profileIndex, tcQueue, &thresholdGet);

        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                "cpssDxChPortCnProfileQueueThresholdGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(threshold, thresholdGet,
                                "got another threshold then was set: %d", dev);


        /*
            1.3. Call with wrong profileIndex [PORT_CN_INVALID_PROFILEINDEX_CNS].
            Expected: GT_BAD_PARAM.
        */
        profileIndex = PORT_CN_INVALID_PROFILEINDEX_CNS;

        st = cpssDxChPortCnProfileQueueThresholdSet(dev, profileIndex, tcQueue, threshold);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        profileIndex = 7;

        /*
            1.4. Call with wrong tcQueue [PORT_CN_INVALID_TCQUEUE_CNS].
            Expected: GT_BAD_PARAM.
        */
        tcQueue = PORT_CN_INVALID_TCQUEUE_CNS;

        st = cpssDxChPortCnProfileQueueThresholdSet(dev, profileIndex, tcQueue, threshold);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        tcQueue = 7;

        /*
            1.5. Call with wrong threshold [PORT_CN_INVALID_THRESHOLD_CNS].
            Expected: GT_OUT_OF_RANGE.
        */
        threshold = PORT_CN_INVALID_THRESHOLD_CNS;

        st = cpssDxChPortCnProfileQueueThresholdSet(dev, profileIndex, tcQueue, threshold);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, dev);

        threshold = 0x1FFF;
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_LION_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortCnProfileQueueThresholdSet(dev, profileIndex, tcQueue, threshold);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortCnProfileQueueThresholdSet(dev, profileIndex, tcQueue, threshold);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortCnProfileQueueThresholdGet
(
    IN  GT_U8    devNum,
    IN  GT_U32   profileIndex,
    IN  GT_U8    tcQueue,
    OUT GT_U32   *thresholdPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortCnProfileQueueThresholdGet)
{
/*
    ITERATE_DEVICES (xCat,xCat2)
    1.1. Call with profileIndex [0 / 7], tcQueue [0 / 7] and not null threshold.
    Expected: GT_OK.
    1.2. Call with wrong profileIndex [PORT_CN_INVALID_PROFILEINDEX_CNS].
    Expected: GT_BAD_PARAM.
    1.3. Call with wrong tcQueue [PORT_CN_INVALID_TCQUEUE_CNS].
    Expected: GT_BAD_PARAM.
    1.4. Call with wrong  thresholdPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;

    GT_U32      profileIndex = 0;
    GT_U8       tcQueue = 0;
    GT_U32      threshold;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_LION_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with profileIndex [0 / 7], tcQueue [0 / 7] and not null threshold.
            Expected: GT_OK.
        */
        profileIndex = 0;
        tcQueue = 0;

        st = cpssDxChPortCnProfileQueueThresholdGet(dev, profileIndex, tcQueue, &threshold);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.1. Call with profileIndex [0 / 7], tcQueue [0 / 7] and not null threshold.
            Expected: GT_OK.
        */
        profileIndex = 7;
        tcQueue = 7;

        st = cpssDxChPortCnProfileQueueThresholdGet(dev, profileIndex, tcQueue, &threshold);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with wrong profileIndex [PORT_CN_INVALID_PROFILEINDEX_CNS].
            Expected: GT_BAD_PARAM.
        */
        profileIndex = PORT_CN_INVALID_PROFILEINDEX_CNS;

        st = cpssDxChPortCnProfileQueueThresholdGet(dev, profileIndex, tcQueue, &threshold);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        profileIndex = 7;

        /*
            1.3. Call with wrong tcQueue [PORT_CN_INVALID_TCQUEUE_CNS].
            Expected: GT_BAD_PARAM.
        */
        tcQueue = PORT_CN_INVALID_TCQUEUE_CNS;

        st = cpssDxChPortCnProfileQueueThresholdGet(dev, profileIndex, tcQueue, &threshold);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        tcQueue = 7;

        /*
            1.4. Call with wrong  threshold pointer [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChPortCnProfileQueueThresholdGet(dev, profileIndex, tcQueue, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, threshold = NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_LION_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortCnProfileQueueThresholdGet(dev, profileIndex, tcQueue, &threshold);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortCnProfileQueueThresholdGet(dev, profileIndex, tcQueue, &threshold);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortCnProfileThresholdSet
(
    IN GT_U8    devNum,
    IN GT_U32   profileIndex,
    IN GT_U32   threshold
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortCnProfileThresholdSet)
{
/*
    ITERATE_DEVICES (xCat,xCat2)
    1.1. Call with profileIndex [0 / 3 / 7] and threshold [0 / 0x1F00 / 0x1FFF].
    Expected: GT_OK.
    1.2. Call cpssDxChPortCnProfileThresholdGet with not-NULL thresholdPtr.
    Expected: GT_OK and the same threshold as was set.
    1.3. Call with wrong profileIndex [PORT_CN_INVALID_PROFILEINDEX_CNS].
    Expected: GT_BAD_PARAM.
    1.4. Call with wrong threshold [PORT_CN_INVALID_THRESHOLD_CNS].
    Expected: GT_OUT_OF_RANGE.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;

    GT_U32   profileIndex = 0;
    GT_U32   threshold = 0;
    GT_U32   thresholdGet = 1;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_LION_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with profileIndex [0 / 3 / 7] and threshold [0 / 0x1F00 / 0x1FFF].
            Expected: GT_OK.
        */
        /*Call with profileIndex [0], threshold [0].*/
        profileIndex = 0;
        threshold = 0;

        st = cpssDxChPortCnProfileThresholdSet(dev, profileIndex, threshold);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call cpssDxChPortCnProfileThresholdGet with not-NULL thresholdPtr.
            Expected: GT_OK and the same threshold as was set.
        */
        st = cpssDxChPortCnProfileThresholdGet(dev, profileIndex, &thresholdGet);

        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                "cpssDxChPortCnProfileThresholdGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(threshold, thresholdGet,
                                "got another threshold then was set: %d", dev);

        /*
            1.1. Call with profileIndex [0 / 3 / 7] and threshold [0 / 0x1F00 / 0x1FFF].
            Expected: GT_OK.
        */
        /*Call with profileIndex [3] threshold [0x1F00].*/
        profileIndex = 3;
        threshold = 0x1F00;

        st = cpssDxChPortCnProfileThresholdSet(dev, profileIndex, threshold);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call cpssDxChPortCnProfileThresholdGet with not-NULL thresholdPtr.
            Expected: GT_OK and the same threshold as was set.
        */
        st = cpssDxChPortCnProfileThresholdGet(dev, profileIndex, &thresholdGet);

        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                "cpssDxChPortCnProfileThresholdGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(threshold, thresholdGet,
                                "got another threshold then was set: %d", dev);

        /*
            1.1. Call with profileIndex [0 / 7] and threshold [0 / 0x1FFF].
            Expected: GT_OK.
        */
        /*Call with profileIndex [7], threshold [0x1FFF].*/
        profileIndex = 7;
        threshold = 0x1FFF;

        st = cpssDxChPortCnProfileThresholdSet(dev, profileIndex, threshold);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call cpssDxChPortCnProfileThresholdGet with not-NULL thresholdPtr.
            Expected: GT_OK and the same threshold as was set.
        */
        st = cpssDxChPortCnProfileThresholdGet(dev, profileIndex, &thresholdGet);

        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                "cpssDxChPortCnProfileThresholdGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(threshold, thresholdGet,
                                "got another threshold then was set: %d", dev);

        /*
            1.3. Call with wrong profileIndex [PORT_CN_INVALID_PROFILEINDEX_CNS].
            Expected: GT_BAD_PARAM.
        */
        profileIndex = PORT_CN_INVALID_PROFILEINDEX_CNS;

        st = cpssDxChPortCnProfileThresholdSet(dev, profileIndex, threshold);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        profileIndex = 7;

        /*
            1.4. Call with wrong threshold [PORT_CN_INVALID_THRESHOLD_CNS].
            Expected: GT_OUT_OF_RANGE.
        */
        threshold = PORT_CN_INVALID_THRESHOLD_CNS;

        st = cpssDxChPortCnProfileThresholdSet(dev, profileIndex, threshold);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, dev);

        threshold = 0x1FFF;
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_LION_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortCnProfileThresholdSet(dev, profileIndex, threshold);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortCnProfileThresholdSet(dev, profileIndex, threshold);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortCnProfileThresholdGet
(
    IN  GT_U8    devNum,
    IN  GT_U32   profileIndex,
    OUT GT_U32   *thresholdPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortCnProfileThresholdGet)
{
/*
    ITERATE_DEVICES (xCat,xCat2)
    1.1. Call with profileIndex [0 / 7] and not null threshold.
    Expected: GT_OK.
    1.2. Call with wrong profileIndex [PORT_CN_INVALID_PROFILEINDEX_CNS].
    Expected: GT_BAD_PARAM.
    1.3. Call with wrong  thresholdPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;

    GT_U32      profileIndex = 0;
    GT_U32      threshold;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_LION_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with profileIndex [0 / 7] and not null threshold.
            Expected: GT_OK.
        */
        profileIndex = 0;

        st = cpssDxChPortCnProfileThresholdGet(dev, profileIndex, &threshold);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.1. Call with profileIndex [0 / 7] and not null threshold.
            Expected: GT_OK.
        */
        profileIndex = 7;

        st = cpssDxChPortCnProfileThresholdGet(dev, profileIndex, &threshold);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with wrong profileIndex [PORT_CN_INVALID_PROFILEINDEX_CNS].
            Expected: GT_BAD_PARAM.
        */
        profileIndex = PORT_CN_INVALID_PROFILEINDEX_CNS;

        st = cpssDxChPortCnProfileThresholdGet(dev, profileIndex, &threshold);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        profileIndex = 7;

        /*
            1.3. Call with wrong  threshold pointer [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChPortCnProfileThresholdGet(dev, profileIndex, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, threshold = NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_LION_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortCnProfileThresholdGet(dev, profileIndex, &threshold);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortCnProfileThresholdGet(dev, profileIndex, &threshold);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortCnQueueAwareEnableSet
(
    IN GT_U8    devNum,
    IN GT_U8    tcQueue,
    IN GT_BOOL  enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortCnQueueAwareEnableSet)
{
/*
    ITERATE_DEVICES (xCat,xCat2)
    1.1. Call with tcQueue [0 / 7] and enable [GT_FALSE / GT_TRUE].
    Expected: GT_OK.
    1.2. Call cpssDxChPortCnQueueAwareEnableGet with not-NULL enablePtr.
    Expected: GT_OK and the same enable as was set.
    1.3. Call with wrong tcQueue [PORT_CN_INVALID_TCQUEUE_CNS].
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;

    GT_U8   tcQueue = 0;
    GT_BOOL  enable = GT_TRUE;
    GT_BOOL  enableGet = 1;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_LION_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with tcQueue [0 / 7] and enable [GT_FALSE / GT_TRUE].
            Expected: GT_OK.
        */
        /*Call with tcQueue [0], enable [GT_TRUE].*/
        tcQueue = 0;
        enable = GT_TRUE;

        st = cpssDxChPortCnQueueAwareEnableSet(dev, tcQueue, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call cpssDxChPortCnQueueAwareEnableGet with not-NULL enablePtr.
            Expected: GT_OK and the same enable as was set.
        */
        st = cpssDxChPortCnQueueAwareEnableGet(dev, tcQueue, &enableGet);

        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                "cpssDxChPortCnQueueAwareEnableGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                                "got another enable then was set: %d", dev);

        /*
            1.1. Call with tcQueue [0 / 7] and enable [GT_FALSE / GT_TRUE].
            Expected: GT_OK.
        */
        /*Call with tcQueue [7] enable [GT_FALSE].*/
        tcQueue = 7;
        enable = GT_FALSE;

        st = cpssDxChPortCnQueueAwareEnableSet(dev, tcQueue, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call cpssDxChPortCnQueueAwareEnableGet with not-NULL enablePtr.
            Expected: GT_OK and the same enable as was set.
        */
        st = cpssDxChPortCnQueueAwareEnableGet(dev, tcQueue, &enableGet);

        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                "cpssDxChPortCnQueueAwareEnableGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                                "got another enable then was set: %d", dev);


        /*
            1.3. Call with wrong tcQueue [PORT_CN_INVALID_TCQUEUE_CNS].
            Expected: GT_BAD_PARAM.
        */
        tcQueue = PORT_CN_INVALID_TCQUEUE_CNS;

        st = cpssDxChPortCnQueueAwareEnableSet(dev, tcQueue, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        tcQueue = 7;

    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_LION_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortCnQueueAwareEnableSet(dev, tcQueue, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortCnQueueAwareEnableSet(dev, tcQueue, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortCnQueueAwareEnableGet
(
    IN  GT_U8    devNum,
    IN  GT_U32   tcQueue,
    OUT GT_U32   *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortCnQueueAwareEnableGet)
{
/*
    ITERATE_DEVICES (xCat and above)
    1.1. Call with tcQueue [0 / 7] and not null enable.
    Expected: GT_OK.
    1.2. Call with wrong tcQueue [PORT_CN_INVALID_TCQUEUE_CNS].
    Expected: GT_BAD_PARAM.
    1.3. Call with wrong  enablePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;

    GT_U8       tcQueue = 0;
    GT_BOOL     enable = GT_TRUE;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_LION_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with tcQueue [0 / 7] and not null enable.
            Expected: GT_OK.
        */
        tcQueue = 0;

        st = cpssDxChPortCnQueueAwareEnableGet(dev, tcQueue, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.1. Call with tcQueue [0 / 7] and not null enable.
            Expected: GT_OK.
        */
        tcQueue = 7;

        st = cpssDxChPortCnQueueAwareEnableGet(dev, tcQueue, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with wrong tcQueue [PORT_CN_INVALID_TCQUEUE_CNS].
            Expected: GT_BAD_PARAM.
        */
        tcQueue = PORT_CN_INVALID_TCQUEUE_CNS;

        st = cpssDxChPortCnQueueAwareEnableGet(dev, tcQueue, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        tcQueue = 7;

        /*
            1.3. Call with wrong  enable pointer [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChPortCnQueueAwareEnableGet(dev, tcQueue, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, enable = NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_LION_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortCnQueueAwareEnableGet(dev, tcQueue, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortCnQueueAwareEnableGet(dev, tcQueue, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortCnSpeedIndexSet
(
    IN GT_U8    devNum,
    IN GT_U8    portNum,
    IN GT_U32   portSpeedIndex
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortCnSpeedIndexSet)
{
/*
    ITERATE_DEVICES_PHY_PORTS
    1.1.1. Call with portSpeedIndex [0 / 7].
    Expected: GT_OK.
    1.1.2. Call cpssDxChPortCnSpeedIndexGet with the same params.
    Expected: GT_OK and the same portSpeedIndex.
    1.1.3. Call with portSpeedIndex [PORT_CN_INVALID_PORTSPEEDINDEX_CNS].
    Expected: GT_OUT_OF_RANGE.
*/

    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM  port   = PORT_CN_VALID_PHY_PORT_CNS;
    GT_U32      portSpeedIndex = 0;
    GT_U32      portSpeedIndexGet = 0;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_LION_E | UTF_LION2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextTxqPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextTxqPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call with portSpeedIndex [0 / 7].
                Expected: GT_OK.
            */
            portSpeedIndex = 0;

            st = cpssDxChPortCnSpeedIndexSet(dev, port, portSpeedIndex);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, portSpeedIndex);

            /*
               1.1.2. Call cpssDxChPortCnSpeedIndexGet.
               Expected: GT_OK and the same portSpeedIndex.
            */
            st = cpssDxChPortCnSpeedIndexGet(dev, port, &portSpeedIndexGet);

            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                            "[cpssDxChPortCnSpeedIndexGet]: %d, %d", dev, port);
            UTF_VERIFY_EQUAL2_STRING_MAC(portSpeedIndex, portSpeedIndexGet,
                      "get another portSpeedIndex value than was set: %d, %d", dev, port);

            /*
                1.1.1. Call with portSpeedIndex [0 / 7].
                Expected: GT_OK.
            */
            /* Call function with portSpeedIndex = GT_TRUE] */
            portSpeedIndex = 7;

            st = cpssDxChPortCnSpeedIndexSet(dev, port, portSpeedIndex);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, portSpeedIndex);

            /*
               1.1.2. Call cpssDxChPortCnSpeedIndexGet.
               Expected: GT_OK and the same portSpeedIndex.
            */
            st = cpssDxChPortCnSpeedIndexGet(dev, port, &portSpeedIndexGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                          "[cpssDxChPortCnSpeedIndexGet]: %d, %d", dev, port);
            UTF_VERIFY_EQUAL2_STRING_MAC(portSpeedIndex, portSpeedIndexGet,
                    "get another portSpeedIndex value than was set: %d, %d", dev, port);

            /*
                1.1.3. Call with portSpeedIndex [PORT_CN_INVALID_PORTSPEEDINDEX_CNS].
                Expected: GT_OUT_OF_RANGE.
            */
            portSpeedIndex = PORT_CN_INVALID_PORTSPEEDINDEX_CNS;

            st = cpssDxChPortCnSpeedIndexSet(dev, port, portSpeedIndex);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OUT_OF_RANGE, st, dev, port, portSpeedIndex);

            portSpeedIndex = 7;
        }

        /*
           1.2. For all active devices go over all non available
           physical ports.
        */

        st = prvUtfNextTxqPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while(GT_OK == prvUtfNextTxqPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChPortCnSpeedIndexSet(dev, port, portSpeedIndex);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPortCnSpeedIndexSet(dev, port, portSpeedIndex);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                     */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChPortCnSpeedIndexSet(dev, port, portSpeedIndex);
        if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev) == GT_TRUE)
        {
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, portSpeedIndex);
        }
        else
        {
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, port, portSpeedIndex);
        }
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    port = PORT_CN_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_LION_E | UTF_LION2_E | UTF_FALCON_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortCnSpeedIndexSet(dev, port, portSpeedIndex);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortCnSpeedIndexSet(dev, port, portSpeedIndex);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortCnSpeedIndexGet
(
    IN   GT_U8     devNum,
    IN   GT_U8     portNum,
    OUT  GT_U32   *portSpeedIndexPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortCnSpeedIndexGet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (xCat,xCat2)
    1.1.1. Call with non-null portSpeedIndexGetPtr.
    Expected: GT_OK.
    1.1.2. Call with portSpeedIndexGetPtr [NULL].
    Expected: GT_BAD_PTR
*/

    GT_STATUS   st    = GT_OK;

    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM   port  = PORT_CN_VALID_PHY_PORT_CNS;
    GT_U32      portSpeedIndexGet = 0;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_LION_E | UTF_LION2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextTxqPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextTxqPortGet(&port, GT_TRUE))
        {
            /* 1.1.1. Call with non-null portSpeedIndexGetPtr.
               Expected: GT_OK.
            */
            st = cpssDxChPortCnSpeedIndexGet(dev, port, &portSpeedIndexGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* 1.1.2. Call with portSpeedIndexGetPtr [NULL].
               Expected: GT_BAD_PTR
            */
            st = cpssDxChPortCnSpeedIndexGet(dev, port, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, NULL", dev, port);
        }

        /* 1.2. For all active devices go over all non available
           physical ports.
        */
        st = prvUtfNextTxqPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while(GT_OK == prvUtfNextTxqPortGet(&port, GT_FALSE))
        {
            st = cpssDxChPortCnSpeedIndexGet(dev, port, &portSpeedIndexGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPortCnSpeedIndexGet(dev, port, &portSpeedIndexGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChPortCnSpeedIndexGet(dev, port, &portSpeedIndexGet);
        if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev) == GT_TRUE)
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    port = PORT_CN_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_LION_E | UTF_LION2_E | UTF_FALCON_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortCnSpeedIndexGet(dev, port, &portSpeedIndexGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortCnSpeedIndexGet(dev, port, &portSpeedIndexGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortCnFbCalcConfigSet
(
    IN GT_U8    devNum,
    IN CPSS_PORT_CN_FB_CALCULATION_CONFIG_STC *fbCalcCfgPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortCnFbCalcConfigSet)
{
/*
    ITERATE_DEVICES (Lion and above)
    1.1. Call with all correct values
                                   fbCalcCfg.wExp[0 / 5];
                                   fbCalcCfg.fbLsb[0 / 0x1A];
                                   fbCalcCfg.deltaEnable[GT_TRUE / GT_FALSE];
                                   fbCalcCfg.fbMin[0 / 0xFFFFA];
                                   fbCalcCfg.fbMax[0 / 0xFFFFA];
    Expected: GT_OK.
    1.2. Call cpssDxChPortCnFbCalcConfigGet with not-NULL fbCalcCfgPtr.
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

    CPSS_PORT_CN_FB_CALCULATION_CONFIG_STC fbCalcCfg;
    CPSS_PORT_CN_FB_CALCULATION_CONFIG_STC fbCalcCfgGet;

    GT_U32 fbMinMaxNumOfBits; /* number of bits in fbMin and fbMax fields */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E);

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

        st = cpssDxChPortCnFbCalcConfigSet(dev, &fbCalcCfg);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        if(GT_OK == st)
        {
            /*
                1.2. Call cpssDxChPortCnFbCalcConfigGet with not-NULL fbCalcCfgPtr.
                Expected: GT_OK and the same fbCalcCfg values.
            */
            st = cpssDxChPortCnFbCalcConfigGet(dev, &fbCalcCfgGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChPortCnFbCalcConfigGet: %d", dev);

            UTF_VERIFY_EQUAL1_STRING_MAC(fbCalcCfg.fbLsb, fbCalcCfgGet.fbLsb,
                             "got another fbCalcCfg.fbLsb then was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(fbCalcCfg.fbMin, fbCalcCfgGet.fbMin,
                             "got another fbCalcCfg.fbMin then was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(fbCalcCfg.fbMax, fbCalcCfgGet.fbMax,
                             "got another fbCalcCfg.fbMax then was set: %d", dev);
            if(!PRV_CPSS_SIP_6_CHECK_MAC(dev))
            {
                UTF_VERIFY_EQUAL1_STRING_MAC(fbCalcCfg.wExp, fbCalcCfgGet.wExp,
                                 "got another fbCalcCfg.wExp then was set: %d", dev);
                UTF_VERIFY_EQUAL1_STRING_MAC(fbCalcCfg.deltaEnable, fbCalcCfgGet.deltaEnable,
                                 "got another fbCalcCfg.deltaEnable then was set: %d", dev);
            }
        }

        if(GT_TRUE == PRV_CPSS_DXCH_BOBCAT3_CHECK_MAC(dev))
        {
            fbMinMaxNumOfBits = 24;
        }
        else
        {
            fbMinMaxNumOfBits = 20;
        }

        fbCalcCfg.fbMin = BIT_MASK_MAC(fbMinMaxNumOfBits);
        fbCalcCfg.fbMax = BIT_MASK_MAC(fbMinMaxNumOfBits);

        st = cpssDxChPortCnFbCalcConfigSet(dev, &fbCalcCfg);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        if(GT_OK == st)
        {
            /*
                1.2. Call cpssDxChPortCnFbCalcConfigGet with not-NULL fbCalcCfgPtr.
                Expected: GT_OK and the same fbCalcCfg values.
            */
            st = cpssDxChPortCnFbCalcConfigGet(dev, &fbCalcCfgGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChPortCnFbCalcConfigGet: %d", dev);

            UTF_VERIFY_EQUAL1_STRING_MAC(fbCalcCfg.fbLsb, fbCalcCfgGet.fbLsb,
                             "got another fbCalcCfg.fbLsb then was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(fbCalcCfg.fbMin, fbCalcCfgGet.fbMin,
                             "got another fbCalcCfg.fbMin then was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(fbCalcCfg.fbMax, fbCalcCfgGet.fbMax,
                             "got another fbCalcCfg.fbMax then was set: %d", dev);
            if(!PRV_CPSS_SIP_6_CHECK_MAC(dev))
            {
                UTF_VERIFY_EQUAL1_STRING_MAC(fbCalcCfg.wExp, fbCalcCfgGet.wExp,
                                 "got another fbCalcCfg.wExp then was set: %d", dev);
                UTF_VERIFY_EQUAL1_STRING_MAC(fbCalcCfg.deltaEnable, fbCalcCfgGet.deltaEnable,
                                 "got another fbCalcCfg.deltaEnable then was set: %d", dev);
            }
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
        fbCalcCfg.fbLsb = 0x0E;
        fbCalcCfg.deltaEnable = GT_FALSE;
        fbCalcCfg.fbMin = 0xFFFFA;
        fbCalcCfg.fbMax = 0xFFFFA;

        st = cpssDxChPortCnFbCalcConfigSet(dev, &fbCalcCfg);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call cpssDxChPortCnFbCalcConfigGet with not-NULL fbCalcCfgPtr.
            Expected: GT_OK and the same fbCalcCfg values.
        */
        st = cpssDxChPortCnFbCalcConfigGet(dev, &fbCalcCfgGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChPortCnFbCalcConfigGet: %d", dev);

        UTF_VERIFY_EQUAL1_STRING_MAC(fbCalcCfg.fbLsb, fbCalcCfgGet.fbLsb,
                         "got another fbCalcCfg.fbLsb then was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(fbCalcCfg.fbMin, fbCalcCfgGet.fbMin,
                         "got another fbCalcCfg.fbMin then was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(fbCalcCfg.fbMax, fbCalcCfgGet.fbMax,
                         "got another fbCalcCfg.fbMax then was set: %d", dev);
        if(!PRV_CPSS_SIP_6_CHECK_MAC(dev))
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(fbCalcCfg.wExp, fbCalcCfgGet.wExp,
                      "got another fbCalcCfg.wExp then was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(fbCalcCfg.deltaEnable, fbCalcCfgGet.deltaEnable,
                      "got another fbCalcCfg.deltaEnable then was set: %d", dev);
        }

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
        if(!PRV_CPSS_SIP_6_CHECK_MAC(dev))
        {
            st = cpssDxChPortCnFbCalcConfigSet(dev, &fbCalcCfg);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, dev);
        }

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

        st = cpssDxChPortCnFbCalcConfigSet(dev, &fbCalcCfg);
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


        st = cpssDxChPortCnFbCalcConfigSet(dev, &fbCalcCfg);
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

        st = cpssDxChPortCnFbCalcConfigSet(dev, &fbCalcCfg);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, dev);

        fbCalcCfg.fbMax = 0;

        /*
            1.7. Call with wrong fbCalcCfgPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChPortCnFbCalcConfigSet(dev, NULL);
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
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortCnFbCalcConfigSet(dev, &fbCalcCfg);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortCnFbCalcConfigSet(dev, &fbCalcCfg);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortCnFbCalcConfigGet
(
    IN  GT_U8    devNum,
    OUT CPSS_PORT_CN_FB_CALCULATION_CONFIG_STC *fbCalcCfgPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortCnFbCalcConfigGet)
{
/*
    ITERATE_DEVICES (Lion and above)
    1.1. Call with not null fbCalcCfgPtr.
    Expected: GT_OK.
    1.2. Call with wrong fbCalcCfgPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;

    CPSS_PORT_CN_FB_CALCULATION_CONFIG_STC fbCalcCfg;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with not null fbCalcCfgPtr.
            Expected: GT_OK.
        */
        st = cpssDxChPortCnFbCalcConfigGet(dev, &fbCalcCfg);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with wrong fbCalcCfgPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChPortCnFbCalcConfigGet(dev, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortCnFbCalcConfigGet(dev, &fbCalcCfg);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortCnFbCalcConfigGet(dev, &fbCalcCfg);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortCnMessageGenerationConfigSet
(
    IN GT_U8    devNum,
    IN CPSS_PORT_CNM_GENERATION_CONFIG_STC *cnmGenerationCfgPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortCnMessageGenerationConfigSet)
{
/*
    ITERATE_DEVICES (Lion and above)
    1.1. Call with all correct values
                       cnmGenerationCfg.qosProfileId [0 / 0x7F]
                       cnmGenerationCfg.isRouted [0 / 1]
                       cnmGenerationCfg.overrideUp [GT_TRUE / GT_FALSE]
                       cnmGenerationCfg.cnmUp [0 / 10]
                       cnmGenerationCfg.defaultVlanId [0 / 0xFFF]
                       cnmGenerationCfg.scaleFactor [256 / 512]
                       cnmGenerationCfg.version [0 / 0xF]
                       cnmGenerationCfg.cpidMsb [0 / 0x123456789ABCD0]
                       cnmGenerationCfg.cnUntaggedEnable [GT_TRUE / GT_FALSE]
                       cnmGenerationCfg.forceCnTag [GT_TRUE / GT_FALSE]
                       cnmGenerationCfg.flowIdTag [0 / 0xFFFF]
                       cnmGenerationCfg.appendPacket [GT_TRUE / GT_FALSE]
                       cnmGenerationCfg.keepQcnSrcInfo [GT_TRUE / GT_FALSE]

    Expected: GT_OK.
    1.2. Call cpssDxChPortCnMessageGenerationConfigGet with
         not-NULL cnmGenerationCfgPtr.
    Expected: GT_OK and the same identEnaalcCfg as was set.
    1.3. Call with wrong cnmGenerationCfgPtr [NULL].
    Expected: GT_BAD_PTR.
    1.4. Call with wrong cnmGenerationCfg.qosProfileId [0x80]
    Expected: NOT GT_OK.
    1.5. Call with wrong cnmGenerationCfg.isRouted [2]
    Expected: NOT GT_OK.
    1.6. Call with wrong cnmGenerationCfg.defaultVlanId [2]
    Expected: NOT GT_OK.
    1.7. Call with wrong cnmGenerationCfg.scaleFactor [128]
    Expected: NOT GT_OK.
    1.8. Call with wrong cnmGenerationCfg.version [0x10]
    Expected: NOT GT_OK.
    1.9. Call with wrong cnmGenerationCfg.flowIdTag [0x10000]
    Expected: NOT GT_OK.
*/

    GT_STATUS   st  = GT_OK;
    GT_U8       dev;
    GT_BOOL     isEqual = GT_FALSE;

    CPSS_PORT_CNM_GENERATION_CONFIG_STC cnmGenerationCfg;
    CPSS_PORT_CNM_GENERATION_CONFIG_STC cnmGenerationCfgGet;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E);

    cpssOsBzero((GT_VOID*) &cnmGenerationCfg, sizeof(cnmGenerationCfg));
    cpssOsBzero((GT_VOID*) &cnmGenerationCfgGet, sizeof(cnmGenerationCfgGet));

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with all correct values
                               cnmGenerationCfg.qosProfileId [0]
                               cnmGenerationCfg.isRouted [0]
                               cnmGenerationCfg.overrideUp [GT_TRUE]
                               cnmGenerationCfg.cnmUp [0]
                               cnmGenerationCfg.defaultVlanId [0]
                               cnmGenerationCfg.scaleFactor [256]
                               cnmGenerationCfg.version [0]
                               cnmGenerationCfg.cpidMsb [0]
                               cnmGenerationCfg.cnUntaggedEnable [GT_TRUE]
                               cnmGenerationCfg.forceCnTag [GT_TRUE]
                               cnmGenerationCfg.flowIdTag [0]
                               cnmGenerationCfg.appendPacket [GT_TRUE]
                               cnmGenerationCfg.keepQcnSrcInfo [GT_TRUE]
            Expected: GT_OK.
        */
        cnmGenerationCfg.qosProfileId = 0;
        cnmGenerationCfg.isRouted = 0;
        cnmGenerationCfg.overrideUp = GT_TRUE;
        cnmGenerationCfg.cnmUp = 0;
        cnmGenerationCfg.defaultVlanId = 0;
        cnmGenerationCfg.scaleFactor = 256;
        cnmGenerationCfg.version = 0;
        cnmGenerationCfg.cpidMsb[0] = 0;
        cnmGenerationCfg.cpidMsb[1] = 0;
        cnmGenerationCfg.cpidMsb[2] = 0;
        cnmGenerationCfg.cpidMsb[3] = 0;
        cnmGenerationCfg.cpidMsb[4] = 0;
        cnmGenerationCfg.cpidMsb[5] = 0;
        cnmGenerationCfg.cpidMsb[6] = 0; /* only some bits are relevant */
        cnmGenerationCfg.cnUntaggedEnable = GT_TRUE;
        cnmGenerationCfg.forceCnTag = GT_TRUE;
        cnmGenerationCfg.flowIdTag = 0;
        cnmGenerationCfg.appendPacket = GT_TRUE;

        /* Aldrin2 allows to keep Src Info in descriptor unchanged */
        if(PRV_CPSS_SIP_5_25_CHECK_MAC(dev))
        {
            cnmGenerationCfg.keepQcnSrcInfo = GT_TRUE;
        }

        st = cpssDxChPortCnMessageGenerationConfigSet(dev, &cnmGenerationCfg);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call cpssDxChPortCnMessageGenerationConfigGet with not-NULL etherTypePtr.
            Expected: GT_OK and the same etherType and identEnable as was set.
        */
        st = cpssDxChPortCnMessageGenerationConfigGet(dev, &cnmGenerationCfgGet);

        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChPortCnMessageGenerationConfigGet: %d", dev);

        UTF_VERIFY_EQUAL1_STRING_MAC(cnmGenerationCfg.qosProfileId, cnmGenerationCfgGet.qosProfileId,
                         "got another cnmGenerationCfg.qosProfileId then was set: %d", dev);

        if ((PRV_CPSS_SIP_5_CHECK_MAC(dev)))
        {
            /* Not supported fields -> compare with 0 */

            UTF_VERIFY_EQUAL1_STRING_MAC(0, cnmGenerationCfgGet.cnmUp,
                             "got another cnmGenerationCfg.cnmUp then was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(0, cnmGenerationCfgGet.flowIdTag,
                             "got another cnmGenerationCfg.flowIdTag then was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(0, cnmGenerationCfgGet.defaultVlanId,
                             "got another cnmGenerationCfg.defaultVlanId then was set: %d", dev);
            /* Aldrin2 has support for fields appendPacket and keepQcnSrcInfo
             */
            if ((PRV_CPSS_SIP_6_CHECK_MAC(dev)))
            {
                UTF_VERIFY_EQUAL1_STRING_MAC(cnmGenerationCfg.appendPacket, cnmGenerationCfgGet.appendPacket,
                                 "got another cnmGenerationCfg.appendPacket then was set: %d", dev);
                UTF_VERIFY_EQUAL1_STRING_MAC(0, cnmGenerationCfgGet.keepQcnSrcInfo,
                                 "got another cnmGenerationCfg.keepQcnSrcInfo then was set: %d", dev);
            }
            else if ((PRV_CPSS_SIP_5_25_CHECK_MAC(dev)))
            {
                UTF_VERIFY_EQUAL1_STRING_MAC(cnmGenerationCfg.appendPacket, cnmGenerationCfgGet.appendPacket,
                                 "got another cnmGenerationCfg.appendPacket then was set: %d", dev);
                UTF_VERIFY_EQUAL1_STRING_MAC(cnmGenerationCfg.keepQcnSrcInfo, cnmGenerationCfgGet.keepQcnSrcInfo,
                                 "got another cnmGenerationCfg.keepQcnSrcInfo then was set: %d", dev);
            }
            else
            {
                UTF_VERIFY_EQUAL1_STRING_MAC(0, cnmGenerationCfgGet.appendPacket,
                                 "got another cnmGenerationCfg.appendPacket then was set: %d", dev);
                UTF_VERIFY_EQUAL1_STRING_MAC(0, cnmGenerationCfgGet.keepQcnSrcInfo,
                                 "got another cnmGenerationCfg.keepQcnSrcInfo then was set: %d", dev);
            }
        }
        else
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(cnmGenerationCfg.defaultVlanId,
                                         cnmGenerationCfgGet.defaultVlanId,
                             "got another cnmGenerationCfg.defaultVlanId then was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(cnmGenerationCfg.appendPacket,
                                         cnmGenerationCfgGet.appendPacket,
                             "got another cnmGenerationCfg.appendPacket then was set: %d", dev);
        }

        if (PRV_CPSS_PP_MAC(dev)->devFamily != CPSS_PP_FAMILY_DXCH_XCAT2_E)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(cnmGenerationCfg.isRouted, cnmGenerationCfgGet.isRouted,
                             "got another cnmGenerationCfg.isRouted then was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(cnmGenerationCfg.overrideUp, cnmGenerationCfgGet.overrideUp,
                             "got another cnmGenerationCfg.overrideUp then was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(cnmGenerationCfg.scaleFactor, cnmGenerationCfgGet.scaleFactor,
                             "got another cnmGenerationCfg.scaleFactor then was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(cnmGenerationCfg.version, cnmGenerationCfgGet.version,
                             "got another cnmGenerationCfg.version then was set: %d", dev);
            /* verifying values */
            isEqual = (0 == cpssOsMemCmp((GT_VOID*) cnmGenerationCfg.cpidMsb, (GT_VOID*) cnmGenerationCfgGet.cpidMsb,
                                     sizeof(cnmGenerationCfg.cpidMsb) - 1 )) ? GT_TRUE : GT_FALSE;
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual, "got another cnmGenerationCfg.cpidMsb then was set: %d", dev);

            if ((!PRV_CPSS_SIP_6_CHECK_MAC(dev)))
            {
                UTF_VERIFY_EQUAL1_STRING_MAC(cnmGenerationCfg.cnUntaggedEnable,
                                             cnmGenerationCfgGet.cnUntaggedEnable,
                                 "got another cnmGenerationCfg.cnUntaggedEnable then was set: %d", dev);
            }
            UTF_VERIFY_EQUAL1_STRING_MAC(cnmGenerationCfg.forceCnTag, cnmGenerationCfgGet.forceCnTag,
                             "got another cnmGenerationCfg.forceCnTag then was set: %d", dev);
        }


        /*
            1.1. Call with all correct values
                               cnmGenerationCfg.qosProfileId [0x7F]
                               cnmGenerationCfg.isRouted [1]
                               cnmGenerationCfg.overrideUp [GT_FALSE]
                               cnmGenerationCfg.cnmUp [10]
                               cnmGenerationCfg.defaultVlanId [0xFFF]
                               cnmGenerationCfg.scaleFactor [512]
                               cnmGenerationCfg.version [0xF]
                               cnmGenerationCfg.cpidMsb [10]
                               cnmGenerationCfg.cnUntaggedEnable [GT_FALSE]
                               cnmGenerationCfg.forceCnTag [GT_FALSE]
                               cnmGenerationCfg.flowIdTag [0xFFFF]
                               cnmGenerationCfg.appendPacket [GT_FALSE]
                               cnmGenerationCfg.keepQcnSrcInfo [GT_FALSE]
            Expected: GT_OK.
        */
        cnmGenerationCfg.qosProfileId = 0x7F;
        cnmGenerationCfg.isRouted = 1;
        cnmGenerationCfg.overrideUp = GT_FALSE;
        cnmGenerationCfg.cnmUp = 10;
        cnmGenerationCfg.defaultVlanId = 0xFFF;
        cnmGenerationCfg.scaleFactor = 512;
        cnmGenerationCfg.version = 0xF;
        cnmGenerationCfg.cpidMsb[0] = 0x12;
        cnmGenerationCfg.cpidMsb[1] = 0x34;
        cnmGenerationCfg.cpidMsb[2] = 0x56;
        cnmGenerationCfg.cpidMsb[3] = 0x78;
        cnmGenerationCfg.cpidMsb[4] = 0x9A;
        cnmGenerationCfg.cpidMsb[5] = 0xBC;
        cnmGenerationCfg.cpidMsb[6] = 0xDD; /* only some bits are relevant */
        cnmGenerationCfg.cnUntaggedEnable = GT_FALSE;
        cnmGenerationCfg.forceCnTag = GT_FALSE;
        cnmGenerationCfg.flowIdTag = 0xFFFF;
        cnmGenerationCfg.appendPacket = GT_FALSE;

        /* Aldrin2 allows to keep Src Info in descriptor unchanged */
        if(PRV_CPSS_SIP_5_25_CHECK_MAC(dev))
        {
            cnmGenerationCfg.keepQcnSrcInfo = GT_FALSE;
        }

        st = cpssDxChPortCnMessageGenerationConfigSet(dev, &cnmGenerationCfg);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call cpssDxChPortCnMessageGenerationConfigGet with not-NULL etherTypePtr.
            Expected: GT_OK and the same etherType and identEnable as was set.
        */
        st = cpssDxChPortCnMessageGenerationConfigGet(dev, &cnmGenerationCfgGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChPortCnMessageGenerationConfigGet: %d", dev);

        UTF_VERIFY_EQUAL1_STRING_MAC(cnmGenerationCfg.qosProfileId, cnmGenerationCfgGet.qosProfileId,
                         "got another cnmGenerationCfg.qosProfileId then was set: %d", dev);
        if ((PRV_CPSS_SIP_5_CHECK_MAC(dev)))
        {
            /* Not supported fields -> compare with 0 */
            /* Aldrin2 has support for fields appendPacket and keepQcnSrcInfo
             */

            UTF_VERIFY_EQUAL1_STRING_MAC(0, cnmGenerationCfgGet.cnmUp,
                             "got another cnmGenerationCfg.cnmUp then was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(0, cnmGenerationCfgGet.flowIdTag,
                             "got another cnmGenerationCfg.flowIdTag then was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(0, cnmGenerationCfgGet.defaultVlanId,
                             "got another cnmGenerationCfg.defaultVlanId then was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(0, cnmGenerationCfgGet.appendPacket,
                             "got another cnmGenerationCfg.appendPacket then was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(0, cnmGenerationCfgGet.keepQcnSrcInfo,
                             "got another cnmGenerationCfg.keepQcnSrcInfo then was set: %d", dev);
        }
        else
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(cnmGenerationCfg.defaultVlanId,
                                         cnmGenerationCfgGet.defaultVlanId,
                             "got another cnmGenerationCfg.defaultVlanId then was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(cnmGenerationCfg.appendPacket,
                                         cnmGenerationCfgGet.appendPacket,
                             "got another cnmGenerationCfg.appendPacket then was set: %d", dev);
        }

        if (PRV_CPSS_PP_MAC(dev)->devFamily != CPSS_PP_FAMILY_DXCH_XCAT2_E)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(cnmGenerationCfg.isRouted, cnmGenerationCfgGet.isRouted,
                             "got another cnmGenerationCfg.isRouted then was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(cnmGenerationCfg.overrideUp, cnmGenerationCfgGet.overrideUp,
                             "got another cnmGenerationCfg.overrideUp then was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(cnmGenerationCfg.scaleFactor, cnmGenerationCfgGet.scaleFactor,
                             "got another cnmGenerationCfg.scaleFactor then was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(cnmGenerationCfg.version, cnmGenerationCfgGet.version,
                             "got another cnmGenerationCfg.version then was set: %d", dev);
            /* verifying values */
            isEqual = (0 == cpssOsMemCmp((GT_VOID*) cnmGenerationCfg.cpidMsb, (GT_VOID*) cnmGenerationCfgGet.cpidMsb,
                                     sizeof(cnmGenerationCfg.cpidMsb) - 1 )) ? GT_TRUE : GT_FALSE;
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual, "got another cnmGenerationCfg.cpidMsb then was set: %d", dev);

            UTF_VERIFY_EQUAL1_STRING_MAC(cnmGenerationCfg.cnUntaggedEnable,
                                         cnmGenerationCfgGet.cnUntaggedEnable,
                             "got another cnmGenerationCfg.cnUntaggedEnable then was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(cnmGenerationCfg.forceCnTag, cnmGenerationCfgGet.forceCnTag,
                             "got another cnmGenerationCfg.forceCnTag then was set: %d", dev);
        }

        /*
            1.3. Call with wrong cnmGenerationCfgPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChPortCnMessageGenerationConfigSet(dev, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /*
            1.4. Call with wrong cnmGenerationCfg.qosProfileId [0x80]
            Expected: NOT GT_OK.
        */
        cnmGenerationCfg.qosProfileId = 0x8000;

        st = cpssDxChPortCnMessageGenerationConfigSet(dev, &cnmGenerationCfg);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        cnmGenerationCfg.qosProfileId = 0;

        /*
            1.5. Call with wrong cnmGenerationCfg.isRouted [2]
            Expected: NOT GT_OK.
        */
        cnmGenerationCfg.isRouted = 2;

        st = cpssDxChPortCnMessageGenerationConfigSet(dev, &cnmGenerationCfg);
        if (PRV_CPSS_PP_MAC(dev)->devFamily != CPSS_PP_FAMILY_DXCH_XCAT2_E)
        {
            UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        }
        else
        {
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        }

        cnmGenerationCfg.isRouted = 0;

        /*
            1.6. Call with wrong cnmGenerationCfg.defaultVlanId [0x1000]
            Expected: NOT GT_OK.
        */
        cnmGenerationCfg.defaultVlanId = 0x1000;

        st = cpssDxChPortCnMessageGenerationConfigSet(dev, &cnmGenerationCfg);
        if ((PRV_CPSS_SIP_5_CHECK_MAC(dev)))
        {
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        }
        else
        {
            UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        }

        cnmGenerationCfg.defaultVlanId = 0;

        /*
            1.7. Call with wrong cnmGenerationCfg.scaleFactor [128]
            Expected: NOT GT_OK.
        */
        cnmGenerationCfg.scaleFactor = 128;
        st = cpssDxChPortCnMessageGenerationConfigSet(dev, &cnmGenerationCfg);
        if (PRV_CPSS_PP_MAC(dev)->devFamily != CPSS_PP_FAMILY_DXCH_XCAT2_E)
        {
            UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        }
        else
        {
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        }

        cnmGenerationCfg.scaleFactor = 256;

        /*
            1.8. Call with wrong cnmGenerationCfg.version [0x10]
            Expected: NOT GT_OK.
        */
        cnmGenerationCfg.version = 0x10;

        st = cpssDxChPortCnMessageGenerationConfigSet(dev, &cnmGenerationCfg);
        if (PRV_CPSS_PP_MAC(dev)->devFamily != CPSS_PP_FAMILY_DXCH_XCAT2_E)
        {
            UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        }
        else
        {
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        }

        cnmGenerationCfg.version = 0;

        /*
            1.9. Call with wrong cnmGenerationCfg.flowIdTag [0x10000]
            Expected: NOT GT_OK.
        */
        cnmGenerationCfg.flowIdTag = 0x10000;

        st = cpssDxChPortCnMessageGenerationConfigSet(dev, &cnmGenerationCfg);
        if ((PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_LION_E)||
            (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E))
        {
            UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        }
        else
        {
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        }

        cnmGenerationCfg.flowIdTag = 0;
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortCnMessageGenerationConfigSet(dev, &cnmGenerationCfg);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortCnMessageGenerationConfigSet(dev, &cnmGenerationCfg);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortCnMessageGenerationConfigGet
(
    IN  GT_U8    devNum,
    OUT CPSS_PORT_CNM_GENERATION_CONFIG_STC *cnmGenerationCfgPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortCnMessageGenerationConfigGet)
{
/*
    ITERATE_DEVICES (Lion and above)
    1.1. Call with not null cnmGenerationCfgPtr.
    Expected: GT_OK.
    1.2. Call with wrong cnmGenerationCfgPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;

    CPSS_PORT_CNM_GENERATION_CONFIG_STC cnmGenerationCfg;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with not null cnmGenerationCfgPtr.
            Expected: GT_OK.
        */
        st = cpssDxChPortCnMessageGenerationConfigGet(dev, &cnmGenerationCfg);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with wrong cnmGenerationCfgPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChPortCnMessageGenerationConfigGet(dev, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortCnMessageGenerationConfigGet(dev, &cnmGenerationCfg);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortCnMessageGenerationConfigGet(dev, &cnmGenerationCfg);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortCnPacketLengthSet
(
    IN GT_U8    devNum,
    IN CPSS_DXCH_PORT_CN_PACKET_LENGTH_ENT packetLength
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortCnPacketLengthSet)
{
/*
    ITERATE_DEVICES (Lion and above)
    1.1. Call with all correct packetLength
                                [CPSS_DXCH_PORT_CN_LENGTH_ORIG_PACKET_E /
                                 CPSS_DXCH_PORT_CN_LENGTH_1_5_KB_E /
                                 CPSS_DXCH_PORT_CN_LENGTH_2_KB_E /
                                 CPSS_DXCH_PORT_CN_LENGTH_10_KB_E].
    Expected: GT_OK.
    1.2. Call cpssDxChPortCnPacketLengthGet with not-NULL packetLengthPtr.
    Expected: GT_OK and the same packetLength as was set.
    1.3. Call with wrong enum values packetLength.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;

    CPSS_DXCH_PORT_CN_PACKET_LENGTH_ENT  packetLength = CPSS_DXCH_PORT_CN_LENGTH_ORIG_PACKET_E;
    CPSS_DXCH_PORT_CN_PACKET_LENGTH_ENT  packetLengthGet;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with all correct packetLength
                                        [CPSS_DXCH_PORT_CN_LENGTH_ORIG_PACKET_E /
                                         CPSS_DXCH_PORT_CN_LENGTH_1_5_KB_E /
                                         CPSS_DXCH_PORT_CN_LENGTH_2_KB_E /
                                         CPSS_DXCH_PORT_CN_LENGTH_10_KB_E].
            Expected: GT_OK.
        */
        packetLength = CPSS_DXCH_PORT_CN_LENGTH_ORIG_PACKET_E;

        st = cpssDxChPortCnPacketLengthSet(dev, packetLength);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call cpssDxChPortCnPacketLengthGet with not-NULL packetLengthPtr.
            Expected: GT_OK and the same packetLength as was set.
        */
        st = cpssDxChPortCnPacketLengthGet(dev, &packetLengthGet);

        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                         "cpssDxChPortCnPacketLengthGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(packetLength, packetLengthGet,
                         "got another packetLength then was set: %d", dev);

        /*
            1.1. Call with all correct packetLength
                                        [CPSS_DXCH_PORT_CN_LENGTH_ORIG_PACKET_E /
                                         CPSS_DXCH_PORT_CN_LENGTH_1_5_KB_E /
                                         CPSS_DXCH_PORT_CN_LENGTH_2_KB_E /
                                         CPSS_DXCH_PORT_CN_LENGTH_10_KB_E].
            Expected: GT_OK.
        */
        packetLength = CPSS_DXCH_PORT_CN_LENGTH_1_5_KB_E;

        st = cpssDxChPortCnPacketLengthSet(dev, packetLength);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call cpssDxChPortCnPacketLengthGet with not-NULL packetLengthPtr.
            Expected: GT_OK and the same packetLength as was set.
        */
        st = cpssDxChPortCnPacketLengthGet(dev, &packetLengthGet);

        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                         "cpssDxChPortCnPacketLengthGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(packetLength, packetLengthGet,
                         "got another packetLength then was set: %d", dev);

        /*
            1.1. Call with all correct packetLength
                                        [CPSS_DXCH_PORT_CN_LENGTH_ORIG_PACKET_E /
                                         CPSS_DXCH_PORT_CN_LENGTH_1_5_KB_E /
                                         CPSS_DXCH_PORT_CN_LENGTH_2_KB_E /
                                         CPSS_DXCH_PORT_CN_LENGTH_10_KB_E].
            Expected: GT_OK.
        */
        packetLength = CPSS_DXCH_PORT_CN_LENGTH_2_KB_E;

        st = cpssDxChPortCnPacketLengthSet(dev, packetLength);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call cpssDxChPortCnPacketLengthGet with not-NULL packetLengthPtr.
            Expected: GT_OK and the same packetLength as was set.
        */
        st = cpssDxChPortCnPacketLengthGet(dev, &packetLengthGet);

        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                         "cpssDxChPortCnPacketLengthGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(packetLength, packetLengthGet,
                         "got another packetLength then was set: %d", dev);

        /*
            1.1. Call with all correct packetLength
                                        [CPSS_DXCH_PORT_CN_LENGTH_ORIG_PACKET_E /
                                         CPSS_DXCH_PORT_CN_LENGTH_1_5_KB_E /
                                         CPSS_DXCH_PORT_CN_LENGTH_2_KB_E /
                                         CPSS_DXCH_PORT_CN_LENGTH_10_KB_E].
            Expected: GT_OK.
        */
        packetLength = CPSS_DXCH_PORT_CN_LENGTH_10_KB_E;

        st = cpssDxChPortCnPacketLengthSet(dev, packetLength);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call cpssDxChPortCnPacketLengthGet with not-NULL packetLengthPtr.
            Expected: GT_OK and the same packetLength as was set.
        */
        st = cpssDxChPortCnPacketLengthGet(dev, &packetLengthGet);

        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                         "cpssDxChPortCnPacketLengthGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(packetLength, packetLengthGet,
                         "got another packetLength then was set: %d", dev);

        /*
            1.3. Call with wrong enum values packetLength.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChPortCnPacketLengthSet
                            (dev, packetLength),
                            packetLength);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E | UTF_FALCON_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortCnPacketLengthSet(dev, packetLength);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortCnPacketLengthSet(dev, packetLength);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortCnPacketLengthGet
(
    IN  GT_U8    devNum,
    OUT CPSS_DXCH_PORT_CN_PACKET_LENGTH_ENT  *packetLengthPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortCnPacketLengthGet)
{
/*
    ITERATE_DEVICES (Lion and above)
    1.1. Call with not null packetLengthPtr.
    Expected: GT_OK.
    1.2. Call with wrong packetLengthPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;

    CPSS_DXCH_PORT_CN_PACKET_LENGTH_ENT  packetLength;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with not null packetLengthPtr.
            Expected: GT_OK.
        */
        st = cpssDxChPortCnPacketLengthGet(dev, &packetLength);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with wrong packetLengthPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChPortCnPacketLengthGet(dev, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E | UTF_FALCON_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortCnPacketLengthGet(dev, &packetLength);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortCnPacketLengthGet(dev, &packetLength);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortCnPrioritySpeedLocationSet
(
    IN GT_U8    devNum,
    IN CPSS_DXCH_PORT_CN_PRIORITY_SPEED_LOCATION_ENT prioritySpeedLocation
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortCnPrioritySpeedLocationSet)
{
/*
    ITERATE_DEVICES (Lion and above)
    1.1. Call with all correct prioritySpeedLocation
                                [CPSS_DXCH_PORT_CN_PRIORITY_SPEED_CPID_E /
                                 CPSS_DXCH_PORT_CN_SDU_UP_E /
                                 CPSS_DXCH_PORT_CN_SPEED_E]
    Expected: GT_OK.
    1.2. Call cpssDxChPortCnPrioritySpeedLocationGet
         with not-NULL prioritySpeedLocationPtr.
    Expected: GT_OK and the same prioritySpeedLocation as was set.
    1.3. Call with wrong enum values prioritySpeedLocation.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;

    CPSS_DXCH_PORT_CN_PRIORITY_SPEED_LOCATION_ENT  prioritySpeedLocation = 0;
    CPSS_DXCH_PORT_CN_PRIORITY_SPEED_LOCATION_ENT  prioritySpeedLocationGet;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with all correct prioritySpeedLocation
                                        [CPSS_DXCH_PORT_CN_PRIORITY_SPEED_CPID_E /
                                         CPSS_DXCH_PORT_CN_SDU_UP_E /
                                         CPSS_DXCH_PORT_CN_SPEED_E]
            Expected: GT_OK.
        */
        prioritySpeedLocation = CPSS_DXCH_PORT_CN_PRIORITY_SPEED_CPID_E;

        st = cpssDxChPortCnPrioritySpeedLocationSet(dev, prioritySpeedLocation);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call cpssDxChPortCnPrioritySpeedLocationGet
                 with not-NULL prioritySpeedLocationPtr.
            Expected: GT_OK and the same prioritySpeedLocation as was set.
        */
        st = cpssDxChPortCnPrioritySpeedLocationGet(dev, &prioritySpeedLocationGet);

        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                         "cpssDxChPortCnPrioritySpeedLocationGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(prioritySpeedLocation, prioritySpeedLocationGet,
                         "got another prioritySpeedLocation then was set: %d", dev);

        /*
            1.1. Call with all correct prioritySpeedLocation
                                        [CPSS_DXCH_PORT_CN_PRIORITY_SPEED_CPID_E /
                                         CPSS_DXCH_PORT_CN_SDU_UP_E /
                                         CPSS_DXCH_PORT_CN_SPEED_E]
            Expected: GT_OK.
        */
        prioritySpeedLocation = CPSS_DXCH_PORT_CN_SDU_UP_E;

        st = cpssDxChPortCnPrioritySpeedLocationSet(dev, prioritySpeedLocation);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call cpssDxChPortCnPrioritySpeedLocationGet
                 with not-NULL prioritySpeedLocationPtr.
            Expected: GT_OK and the same prioritySpeedLocation as was set.
        */
        st = cpssDxChPortCnPrioritySpeedLocationGet(dev, &prioritySpeedLocationGet);

        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                         "cpssDxChPortCnPrioritySpeedLocationGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(prioritySpeedLocation, prioritySpeedLocationGet,
                         "got another prioritySpeedLocation then was set: %d", dev);

        /*
            1.1. Call with all correct prioritySpeedLocation
                                        [CPSS_DXCH_PORT_CN_PRIORITY_SPEED_CPID_E /
                                         CPSS_DXCH_PORT_CN_SDU_UP_E /
                                         CPSS_DXCH_PORT_CN_SPEED_E]
            Expected: GT_OK.
        */
        prioritySpeedLocation = CPSS_DXCH_PORT_CN_SPEED_E;

        st = cpssDxChPortCnPrioritySpeedLocationSet(dev, prioritySpeedLocation);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call cpssDxChPortCnPrioritySpeedLocationGet
                 with not-NULL prioritySpeedLocationPtr.
            Expected: GT_OK and the same prioritySpeedLocation as was set.
        */
        st = cpssDxChPortCnPrioritySpeedLocationGet(dev, &prioritySpeedLocationGet);

        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                         "cpssDxChPortCnPrioritySpeedLocationGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(prioritySpeedLocation, prioritySpeedLocationGet,
                         "got another prioritySpeedLocation then was set: %d", dev);

        /*
            1.3. Call with wrong enum values prioritySpeedLocation.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChPortCnPrioritySpeedLocationSet
                            (dev, prioritySpeedLocation),
                            prioritySpeedLocation);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortCnPrioritySpeedLocationSet(dev, prioritySpeedLocation);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortCnPrioritySpeedLocationSet(dev, prioritySpeedLocation);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortCnPrioritySpeedLocationGet
(
    IN  GT_U8    devNum,
    OUT CPSS_DXCH_PORT_CN_PRIORITY_SPEED_LOCATION_ENT  *prioritySpeedLocationPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortCnPrioritySpeedLocationGet)
{
/*
    ITERATE_DEVICES (Lion and above)
    1.1. Call with not null prioritySpeedLocationPtr.
    Expected: GT_OK.
    1.2. Call with wrong prioritySpeedLocationPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;

    CPSS_DXCH_PORT_CN_PRIORITY_SPEED_LOCATION_ENT  prioritySpeedLocation;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with not null prioritySpeedLocationPtr.
            Expected: GT_OK.
        */
        st = cpssDxChPortCnPrioritySpeedLocationGet(dev, &prioritySpeedLocation);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with wrong prioritySpeedLocationPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChPortCnPrioritySpeedLocationGet(dev, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortCnPrioritySpeedLocationGet(dev, &prioritySpeedLocation);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortCnPrioritySpeedLocationGet(dev, &prioritySpeedLocation);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortCnProfileQueueConfigSet
(
    IN GT_U8                               devNum,
    IN CPSS_PORT_TX_DROP_PROFILE_SET_ENT   profileSet,
    IN GT_U8                               tcQueue,
    IN CPSS_PORT_CN_PROFILE_CONFIG_STC     *cnProfileCfgPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortCnProfileQueueConfigSet)
{
/*
    ITERATE_DEVICES (Lion and above)
    1.1. Call with all correct parameters
                                profileSet [CPSS_PORT_TX_DROP_PROFILE_1_E /
                                            CPSS_PORT_TX_DROP_PROFILE_4_E],
                                tcQueue [1 / 5],
                                cnProfileCfg.cnAware [GT_TRUE / GT_FALSE],
                                cnProfileCfg.threshold [1 / 10].
    Expected: GT_OK.
    1.2. Call cpssDxChPortCnProfileQueueConfigGet
         with not-NULL cnProfileCfgPtr.
    Expected: GT_OK and the same values as was set.
    1.3. Call with wrong enum values profileSet.
    Expected: GT_BAD_PARAM.
    1.4. Call with wrong tcQueue [PORT_CN_INVALID_TCQUEUE_CNS].
    Expected: NOT GT_OK.
    1.5. Call with wrong cnProfileCfgPtr [NULL].
    Expected: GT_BAD_PTR.
    1.6. Call with wrong cnProfileCfg.threshold [out of range].
    Expected: NOT GT_OK.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;

    CPSS_PORT_TX_DROP_PROFILE_SET_ENT   profileSet = 0;
    GT_U8                               tcQueue = 0;
    CPSS_PORT_CN_PROFILE_CONFIG_STC     cnProfileCfg;
    CPSS_PORT_CN_PROFILE_CONFIG_STC     cnProfileCfgGet;
    GT_U32 thresholdNumOfBits;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with all correct parameters
                                        profileSet [CPSS_PORT_TX_DROP_PROFILE_1_E],
                                        tcQueue [1],
                                        cnProfileCfg.threshold [1].
            Expected: GT_OK.
        */
        profileSet = CPSS_PORT_TX_DROP_PROFILE_1_E;
        tcQueue = 1;
        cnProfileCfg.cnAware = GT_TRUE;
        cnProfileCfg.threshold = 1;

        /* Aldrin2 has introduced alpha param as part of this structure */
        if (PRV_CPSS_SIP_5_25_CHECK_MAC(dev))
        {
            cnProfileCfg.alpha = CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_25_E;
        }


        st = cpssDxChPortCnProfileQueueConfigSet(dev, profileSet, tcQueue, &cnProfileCfg);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call cpssDxChPortCnProfileQueueConfigGet
                 with not-NULL cnProfileCfgPtr.
            Expected: GT_OK and the same values as was set.
        */
        st = cpssDxChPortCnProfileQueueConfigGet(dev, profileSet, tcQueue, &cnProfileCfgGet);

        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
            "cpssDxChPortCnProfileQueueConfigGet: %d", dev);

        UTF_VERIFY_EQUAL1_STRING_MAC(cnProfileCfg.cnAware,
                                     cnProfileCfgGet.cnAware,
            "got another cnProfileCfg.cnAware then was set: %d", dev);

        UTF_VERIFY_EQUAL1_STRING_MAC(cnProfileCfg.threshold,
                                     cnProfileCfgGet.threshold,
            "got another cnProfileCfg.threshold then was set: %d", dev);

        /* Aldrin2 has introduced alpha param as part of this structure */
        if(PRV_CPSS_SIP_5_25_CHECK_MAC(dev))
        {
             UTF_VERIFY_EQUAL3_STRING_MAC(cnProfileCfg.alpha,
                                          cnProfileCfgGet.alpha,
                 "got another cnProfileCfg.alpha then was set: %d %d %d", dev, cnProfileCfg.alpha, cnProfileCfgGet.alpha);
        }

        /*
            1.1. Call with all correct parameters
                                        profileSet [CPSS_PORT_TX_DROP_PROFILE_4_E],
                                        tcQueue [5],
                                        cnProfileCfg.threshold [10].
            Expected: GT_OK.
        */
        profileSet = CPSS_PORT_TX_DROP_PROFILE_4_E;
        tcQueue = 5;
        cnProfileCfg.cnAware = GT_FALSE;
        cnProfileCfg.threshold = 10;

        /* Aldrin2 has introduced alpha param as part of this structure */
        if (PRV_CPSS_SIP_5_25_CHECK_MAC(dev))
        {
            cnProfileCfg.alpha = CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_4_E;
        }

        st = cpssDxChPortCnProfileQueueConfigSet(dev, profileSet, tcQueue, &cnProfileCfg);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        if(GT_OK == st)
        {
            /*
                1.2. Call cpssDxChPortCnProfileQueueConfigGet
                     with not-NULL cnProfileCfgPtr.
                Expected: GT_OK and the same values as was set.
            */
            st = cpssDxChPortCnProfileQueueConfigGet(dev, profileSet, tcQueue, &cnProfileCfgGet);

            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChPortCnProfileQueueConfigGet: %d %d %d", dev, cnProfileCfg.alpha, cnProfileCfgGet.alpha);

            UTF_VERIFY_EQUAL1_STRING_MAC(cnProfileCfg.cnAware, cnProfileCfgGet.cnAware,
                "got another cnProfileCfg.cnAware then was set: %d", dev);

            UTF_VERIFY_EQUAL1_STRING_MAC(cnProfileCfg.threshold, cnProfileCfgGet.threshold,
                "got another cnProfileCfg.threshold then was set: %d", dev);

            /* Aldrin2 has introduced alpha param as part of this structure */
            if(PRV_CPSS_SIP_5_25_CHECK_MAC(dev))
            {
                UTF_VERIFY_EQUAL4_STRING_MAC(cnProfileCfg.alpha,
                    cnProfileCfgGet.alpha,
                    "got another cnProfileCfg.alpha then was set: %d %d %d %d", dev, cnProfileCfg.alpha, cnProfileCfgGet.alpha, cnProfileCfgGet.threshold);
            }
        }


        /* call with max threshold value */
        if(PRV_CPSS_SIP_5_20_CHECK_MAC(dev) == GT_TRUE)
        {
            thresholdNumOfBits = 20;
        }
        else if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev) == GT_TRUE)
        {
            thresholdNumOfBits = 16;
        }
        else
        {
            thresholdNumOfBits = 13;
        }

        cnProfileCfg.threshold = BIT_MASK_MAC(thresholdNumOfBits);

        st = cpssDxChPortCnProfileQueueConfigSet(dev, profileSet, tcQueue, &cnProfileCfg);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        if(GT_OK == st)
        {
            /*
                1.2. Call cpssDxChPortCnProfileQueueConfigGet
                     with not-NULL cnProfileCfgPtr.
                Expected: GT_OK and the same values as was set.
            */
            st = cpssDxChPortCnProfileQueueConfigGet(dev, profileSet, tcQueue, &cnProfileCfgGet);

            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChPortCnProfileQueueConfigGet: %d", dev);

            UTF_VERIFY_EQUAL1_STRING_MAC(cnProfileCfg.cnAware, cnProfileCfgGet.cnAware,
                "got another cnProfileCfg.cnAware then was set: %d", dev);

            UTF_VERIFY_EQUAL1_STRING_MAC(cnProfileCfg.threshold, cnProfileCfgGet.threshold,
                "got another cnProfileCfg.threshold then was set: %d", dev);
            /* Aldrin2 has introduced alpha param as part of this structure */
            if(PRV_CPSS_SIP_5_25_CHECK_MAC(dev))
            {
                UTF_VERIFY_EQUAL1_STRING_MAC(cnProfileCfg.alpha,
                    cnProfileCfgGet.alpha,
                    "got another cnProfileCfg.alpha then was set: %d", dev);
            }
        }

        /*
            1.3. Call with wrong enum values profileSet.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChPortCnProfileQueueConfigSet
                            (dev, profileSet, tcQueue, &cnProfileCfg),
                            profileSet);

        /*
            1.4. Call with wrong tcQueue [PORT_CN_INVALID_TCQUEUE_CNS].
            Expected: NOT GT_OK.
        */
        tcQueue = PRV_CPSS_SIP_6_CHECK_MAC(dev) ? PORT_CN_INVALID_SIP6_TCQUEUE_CNS : PORT_CN_INVALID_TCQUEUE_CNS;

        st = cpssDxChPortCnProfileQueueConfigSet(dev, profileSet, tcQueue, &cnProfileCfg);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        tcQueue = 5;

        /*
            1.5. Call with wrong cnProfileCfgPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChPortCnProfileQueueConfigSet(dev, profileSet, tcQueue, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);


        /*
            1.6. Call with wrong cnProfileCfg.threshold [out of range].
            Expected: GT_BAD_PARAM.
        */
        cnProfileCfg.threshold = BIT_MASK_MAC(thresholdNumOfBits) + 1;

        st = cpssDxChPortCnProfileQueueConfigSet(dev, profileSet, tcQueue, &cnProfileCfg);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        cnProfileCfg.threshold = 0;

        /* Aldrin2 has introduced alpha param for DBA */
        if(PRV_CPSS_SIP_5_25_CHECK_MAC(dev))
        {
            /*
               1.7. Call with wrong cnProfileCfg.alpha [out of range].
               Expected: GT_BAD_PARAM.
            */
            cnProfileCfg.alpha = PRV_CPSS_SIP_6_CHECK_MAC(dev) ? 0x0c /* 0xb is the max allowed alpha enum for falcon */
                                 : CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_8_E;

            st = cpssDxChPortCnProfileQueueConfigSet(dev, profileSet, tcQueue, &cnProfileCfg);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

            cnProfileCfg.alpha = CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_E;
        }
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortCnProfileQueueConfigSet(dev, profileSet, tcQueue, &cnProfileCfg);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortCnProfileQueueConfigSet(dev, profileSet, tcQueue, &cnProfileCfg);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortCnProfileQueueConfigGet
(
    IN  GT_U8                               devNum,
    IN  CPSS_PORT_TX_DROP_PROFILE_SET_ENT   profileSet,
    IN  GT_U8                               tcQueue,
    OUT CPSS_PORT_CN_PROFILE_CONFIG_STC     *cnProfileCfgPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortCnProfileQueueConfigGet)
{
/*
    ITERATE_DEVICES (Lion and above)
    1.1. Call with all correct parameters
                                profileSet [CPSS_PORT_TX_DROP_PROFILE_1_E /
                                            CPSS_PORT_TX_DROP_PROFILE_4_E],
                                tcQueue [1 / 5],
                   and not NULL cnProfileCfgPtr.
    Expected: GT_OK.
    1.2. Call with wrong enum values profileSet.
    Expected: GT_BAD_PARAM.
    1.3. Call with wrong tcQueue [PORT_CN_INVALID_TCQUEUE_CNS].
    Expected: NOT GT_OK.
    1.4. Call with wrong cnProfileCfgPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;

    CPSS_PORT_TX_DROP_PROFILE_SET_ENT   profileSet = 0;
    GT_U8                               tcQueue = 0;
    CPSS_PORT_CN_PROFILE_CONFIG_STC     cnProfileCfg;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with all correct parameters
                                        profileSet [CPSS_PORT_TX_DROP_PROFILE_1_E]
                                        tcQueue [1],
                           and not NULL cnProfileCfgPtr.
            Expected: GT_OK.
        */
        profileSet = CPSS_PORT_TX_DROP_PROFILE_1_E;
        tcQueue = 1;

        st = cpssDxChPortCnProfileQueueConfigGet(dev, profileSet, tcQueue, &cnProfileCfg);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.1. Call with all correct parameters
                                        profileSet [CPSS_PORT_TX_DROP_PROFILE_4_E],
                                        tcQueue [5],
                           and not NULL cnProfileCfgPtr.
            Expected: GT_OK.
        */
        profileSet = CPSS_PORT_TX_DROP_PROFILE_4_E;
        tcQueue = 5;

        st = cpssDxChPortCnProfileQueueConfigGet(dev, profileSet, tcQueue, &cnProfileCfg);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);


        /*
            1.2. Call with wrong enum values profileSet.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChPortCnProfileQueueConfigGet
                            (dev, profileSet, tcQueue, &cnProfileCfg),
                            profileSet);

        /*
            1.4. Call with wrong tcQueue [PORT_CN_INVALID_TCQUEUE_CNS].
            Expected: NOT GT_OK.
        */
        tcQueue = PRV_CPSS_SIP_6_CHECK_MAC(dev) ? PORT_CN_INVALID_SIP6_TCQUEUE_CNS : PORT_CN_INVALID_TCQUEUE_CNS;

        st = cpssDxChPortCnProfileQueueConfigGet(dev, profileSet, tcQueue, &cnProfileCfg);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        tcQueue = 5;

        /*
            1.5. Call with wrong cnProfileCfgPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChPortCnProfileQueueConfigGet(dev, profileSet, tcQueue, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortCnProfileQueueConfigGet(dev, profileSet, tcQueue, &cnProfileCfg);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortCnProfileQueueConfigGet(dev, profileSet, tcQueue, &cnProfileCfg);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortCnQueueStatusModeEnableSet
(
    IN GT_U8    devNum,
    IN GT_BOOL  enable,
    IN GT_U8    targetDev,
    IN GT_U8    targetPort
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortCnQueueStatusModeEnableSet)
{
/*
    ITERATE_DEVICES (Lion and above)
    1.1. Call with all correct parameters
                                        enable [GT_TRUE / GT_FALSE],
                                        targetDev [0 - 31],
                                        targetPort [0 - 61].
    Expected: GT_OK.
    1.2. Call cpssDxChPortCnQueueStatusModeEnableGet with not-NULL pointers
    Expected: GT_OK and the same values as was set.
    1.3. Call with enable [GT_FALSE]
         and wrong targetDev [PORT_CN_INVALID_DEV_CNS] (not relevant).
    Expected: GT_OK.
    1.4. Call withenable [GT_FALSE]
         and wrong targetPort [PORT_CN_INVALID_PHY_PORT_CNS] (not relevant).
    Expected: GT_OK.
    1.5. Call with enable [GT_TRUE] and wrong targetDev [PORT_CN_INVALID_DEV_CNS].
    Expected: NOT GT_OK.
    1.6. Call with enable [GT_TRUE] and wrong targetPort [PORT_CN_INVALID_PHY_PORT_CNS].
    Expected: NOT GT_OK.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;

    GT_BOOL  enable = GT_TRUE;
    GT_HW_DEV_NUM    targetDev = PORT_CN_VALID_DEV_CNS;
    GT_PORT_NUM    targetPort = PORT_CN_VALID_PHY_PORT_CNS;
    GT_HW_DEV_NUM    maxTargetDevNum;
    GT_PORT_NUM    maxTargetPort;

    GT_BOOL  enableGet = GT_FALSE;
    GT_HW_DEV_NUM    targetDevGet = 0;
    GT_PORT_NUM    targetPortGet = 0;
    GT_U32 step;



    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E);;

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {

        if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev) == GT_TRUE)
        {
            maxTargetDevNum = PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_HW_DEV_NUM_MAC(dev) + 1;
            maxTargetPort = PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_E_PORT_MAC(dev) + 1;
            step = 64;
        }
        else
        {
            maxTargetDevNum = PORT_CN_INVALID_DEV_CNS;
            maxTargetPort = PORT_CN_INVALID_PHY_PORT_CNS;
            step = 1;
        }

        /*
            1.1. Call with all correct parameters enable [GT_TRUE / GT_FALSE],
                                                  targetDev [0 - 31], targetPort [0 - 61].
            Expected: GT_OK.
        */
        for(enable = GT_FALSE; enable <= GT_TRUE; enable++)
            for(targetDev = 0; targetDev < maxTargetDevNum; targetDev+=step)
                for(targetPort = 0; targetPort < maxTargetPort; targetPort+=step)
                {
                    st = cpssDxChPortCnQueueStatusModeEnableSet(dev, enable, targetDev, targetPort);
                    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st,
                     "cpssDxChPortCnQueueStatusModeEnableSet: dev=%d, enable=%d, targetDev=%d, targetPort=%d",
                                                 dev, enable, targetDev, targetPort);

                    if(GT_OK == st)
                    {
                        /*
                            1.2. Call cpssDxChPortCnQueueStatusModeEnableGet
                                 with not-NULL cnProfileCfgPtr.
                            Expected: GT_OK and the same values as was set.
                        */
                        st = cpssDxChPortCnQueueStatusModeEnableGet(dev,
                                            &enableGet, &targetDevGet, &targetPortGet);
                        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                            "cpssDxChPortCnQueueStatusModeEnableGet: %d", dev);

                        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                                        "got another enable then was set: %d", dev);

                        if(GT_TRUE == enable)
                        {
                            UTF_VERIFY_EQUAL1_STRING_MAC(targetDev, targetDevGet,
                                            "got another targetDev then was set: %d", dev);
                            UTF_VERIFY_EQUAL1_STRING_MAC(targetPort, targetPortGet,
                                            "got another targetPort then was set: %d", dev);
                        }
                    }
                }

        /*
            1.3. Call with enable [GT_FALSE]
                 and wrong targetDev [PORT_CN_INVALID_DEV_CNS] (not relevant).
            Expected: GT_OK.
        */
        enable = GT_FALSE;
        targetDev = maxTargetDevNum;
        targetPort = 0;

        st = cpssDxChPortCnQueueStatusModeEnableSet(dev, enable, targetDev, targetPort);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st,
         "cpssDxChPortCnQueueStatusModeEnableSet: dev=%d, enable=%d, targetDev=%d, targetPort=%d",
                                     dev, enable, targetDev, targetPort);
        targetDev = 0;

        /*
            1.4. Call with enable [GT_FALSE]
                 and wrong targetPort [PORT_CN_INVALID_PHY_PORT_CNS] (not relevant).
            Expected: GT_OK.
        */
        enable = GT_FALSE;
        targetDev = 0;
        targetPort = maxTargetPort ;

        st = cpssDxChPortCnQueueStatusModeEnableSet(dev, enable, targetDev, targetPort);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st,
         "cpssDxChPortCnQueueStatusModeEnableSet: dev=%d, enable=%d, targetDev=%d, targetPort=%d",
                                     dev, enable, targetDev, targetPort);
        targetPort = 0;

        /*
            1.5. Call with enable [GT_TRUE]
                 and wrong targetDev [PORT_CN_INVALID_DEV_CNS].
            Expected: NOT GT_OK.
        */
        enable = GT_TRUE;
        targetDev = maxTargetDevNum;
        targetPort = 0;

        st = cpssDxChPortCnQueueStatusModeEnableSet(dev, enable, targetDev, targetPort);
        UTF_VERIFY_NOT_EQUAL4_STRING_MAC(GT_OK, st,
         "cpssDxChPortCnQueueStatusModeEnableSet: dev=%d, enable=%d, targetDev=%d, targetPort=%d",
                                     dev, enable, targetDev, targetPort);
        targetDev = 0;

        /*
            1.6. Call with enable [GT_TRUE]
                 and wrong targetPort [PORT_CN_INVALID_PHY_PORT_CNS].
            Expected: NOT GT_OK.
        */
        enable = GT_TRUE;
        targetDev = 0;
        targetPort = maxTargetPort ;

        st = cpssDxChPortCnQueueStatusModeEnableSet(dev, enable, targetDev, targetPort);
        UTF_VERIFY_NOT_EQUAL4_STRING_MAC(GT_OK, st,
         "cpssDxChPortCnQueueStatusModeEnableSet: dev=%d, enable=%d, targetDev=%d, targetPort=%d",
                                     dev, enable, targetDev, targetPort);
        targetPort = 0;
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortCnQueueStatusModeEnableSet(dev, enable, targetDev, targetPort);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortCnQueueStatusModeEnableSet(dev, enable, targetDev, targetPort);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortCnQueueStatusModeEnableGet
(
    IN  GT_U8    devNum,
    OUT GT_BOOL  *enablePtr,
    OUT GT_U8    *targetDevPtr,
    OUT GT_U8    *targetPortPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortCnQueueStatusModeEnableGet)
{
/*
    ITERATE_DEVICES (Lion and above)
    1.1. Call with not null pointers.
    Expected: GT_OK.
    1.2. Call with wrong enablePtr [NULL].
    Expected: GT_BAD_PTR.
    1.2. Call with wrong targetDevPtr [NULL].
    Expected: GT_BAD_PTR.
    1.2. Call with wrong terargetPortPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;

    GT_BOOL  enable;
    GT_HW_DEV_NUM    targetDev;
    GT_PORT_NUM    targetPort;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with not null enablePtr.
            Expected: GT_OK.
        */
        st = cpssDxChPortCnQueueStatusModeEnableGet(dev, &enable, &targetDev, &targetPort);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with wrong enablePtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChPortCnQueueStatusModeEnableGet(dev, NULL, &targetDev, &targetPort);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /*
            1.3. Call with wrong targetDev [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChPortCnQueueStatusModeEnableGet(dev, &enable, NULL, &targetPort);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /*
            1.4. Call with wrong targetPort [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChPortCnQueueStatusModeEnableGet(dev, &enable, &targetDev, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortCnQueueStatusModeEnableGet(dev, &enable, &targetDev, &targetPort);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortCnQueueStatusModeEnableGet(dev, &enable, &targetDev, &targetPort);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortCnSampleEntrySet
(
    IN GT_U8    devNum,
    IN GT_U8    entryIndex,
    IN CPSS_PORT_CN_SAMPLE_INTERVAL_ENTRY_STC *entryPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortCnSampleEntrySet)
{
/*
    ITERATE_DEVICES (Lion and above)
    1.1. Call with all correct parameters
                            entryIndex[0 - 7]
                            entry.interval[0 - 0xFFFF]
                            entry.randBitmap[0 - 0xFFFF]
    Expected: GT_OK.
    1.2. Call cpssDxChPortCnSampleEntryGet with not-NULL entryPtr.
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

    GT_U8                                  entryIndex = 0;
    CPSS_PORT_CN_SAMPLE_INTERVAL_ENTRY_STC entry;
    CPSS_PORT_CN_SAMPLE_INTERVAL_ENTRY_STC entryGet;
    GT_U32      counter = 0;/* loop counter */
    GT_BOOL     reduceIterations = (prvUtfIsGmCompilation() || prvUtfReduceLogSizeFlagGet());

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E | UTF_FALCON_E);

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
                    st = cpssDxChPortCnSampleEntrySet(dev, entryIndex, &entry);

                    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st,
                    "cpssDxChPortCnSampleEntrySet: dev=%d, entryIndex=%d, entry.interval=%d, entry.randBitmap=%d",
                               dev, entryIndex, entry.interval, entry.randBitmap);

                    if(GT_OK == st)
                    {
                        /*
                            1.2. Call cpssDxChPortCnSampleEntryGet with not-NULL entryPtr.
                            Expected: GT_OK and the same values as was set.
                        */
                        st = cpssDxChPortCnSampleEntryGet(dev, entryIndex, &entryGet);

                        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                         "cpssDxChPortCnSampleEntryGet: %d", dev);
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

        st = cpssDxChPortCnSampleEntrySet(dev, entryIndex, &entry);

        UTF_VERIFY_NOT_EQUAL4_STRING_MAC(GT_OK, st,
        "cpssDxChPortCnSampleEntrySet: dev=%d, entryIndex=%d, entry.interval=%d, entry.randBitmap=%d",
                   dev, entryIndex, entry.interval, entry.randBitmap);

        entryIndex = 0;

        /*
            1.4. Call with wrong entry.interval [0x10000].
            Expected: NOT GT_OK.
        */
        entry.interval = 0x10000;

        st = cpssDxChPortCnSampleEntrySet(dev, entryIndex, &entry);

        UTF_VERIFY_NOT_EQUAL4_STRING_MAC(GT_OK, st,
        "cpssDxChPortCnSampleEntrySet: dev=%d, entryIndex=%d, entry.interval=%d, entry.randBitmap=%d",
                   dev, entryIndex, entry.interval, entry.randBitmap);

        entry.interval = 0;

        /*
            1.5. Call with wrong entry.randBitmap [0x10000].
            Expected: NOT GT_OK.
        */
        entry.randBitmap = 0x10000;

        st = cpssDxChPortCnSampleEntrySet(dev, entryIndex, &entry);

        UTF_VERIFY_NOT_EQUAL4_STRING_MAC(GT_OK, st,
        "cpssDxChPortCnSampleEntrySet: dev=%d, entryIndex=%d, entry.randBitmap=%d, entry.randBitmap=%d",
                   dev, entryIndex, entry.randBitmap, entry.randBitmap);

        entry.randBitmap = 0;

        /*
            1.6. Call with wrong entryPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChPortCnSampleEntrySet(dev, entryIndex, NULL);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, st);

    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E | UTF_FALCON_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortCnSampleEntrySet(dev, entryIndex, &entry);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortCnSampleEntrySet(dev, entryIndex, &entry);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortCnSampleEntryGet
(
    IN GT_U8    devNum,
    IN GT_U8    entryIndex,
    OUT CPSS_PORT_CN_SAMPLE_INTERVAL_ENTRY_STC *entryPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortCnSampleEntryGet)
{
/*
    ITERATE_DEVICES (Lion and above)
    1.1. Call with all correct parameters entryIndex[0 - 7] and not null entryPtr.
    Expected: GT_OK.
    1.2. Call with wrong entryIndex [8].
    Expected: NOT GT_OK.
    1.3. Call with wrong entryPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;

    GT_U8                                  entryIndex = 0;
    CPSS_PORT_CN_SAMPLE_INTERVAL_ENTRY_STC entry;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with all correct parameters entryIndex[0 - 7] and not null entryPtr.
            Expected: GT_OK.
        */
        for(entryIndex = 0; entryIndex < 8; entryIndex++)
        {
            st = cpssDxChPortCnSampleEntryGet(dev, entryIndex, &entry);

            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                "cpssDxChPortCnSampleEntryGet: dev=%d, entryIndex=", dev, entryIndex);
        }
        /*
            1.3. Call with wrong entryIndex [8].
            Expected: NOT GT_OK.
        */
        entryIndex = 8;

        st = cpssDxChPortCnSampleEntryGet(dev, entryIndex, &entry);

        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st,
                "cpssDxChPortCnSampleEntryGet: dev=%d, entryIndex=%d", dev, entryIndex);
        entryIndex = 0;

        /*
            1.3. Call with wrong entryPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChPortCnSampleEntryGet(dev, entryIndex, NULL);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, st);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E | UTF_FALCON_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortCnSampleEntryGet(dev, entryIndex, &entry);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortCnSampleEntryGet(dev, entryIndex, &entry);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortCnTerminationEnableSet
(
    IN  GT_U8     devNum,
    IN  GT_U8     portNum,
    IN  GT_BOOL   enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortCnTerminationEnableSet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (Lion and above)
    1.1.1. Call with state [GT_FALSE and GT_TRUE].
    Expected: GT_OK.
    1.1.2. Call cpssDxChPortCnTerminationEnableGet.
    Expected: GT_OK and the same enable.
*/
    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM port   = PORT_CN_VALID_PHY_PORT_CNS;
    GT_BOOL     enable = GT_FALSE;
    GT_BOOL     state  = GT_FALSE;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E);

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

            st = cpssDxChPortCnTerminationEnableSet(dev, port, enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);

            /*
               1.1.2. Call cpssDxChPortCnTerminationEnableGet.
               Expected: GT_OK and the same enable.
            */
            st = cpssDxChPortCnTerminationEnableGet(dev, port, &state);

            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                            "cpssDxChPortCnTerminationEnableGet: %d, %d", dev, port);
            UTF_VERIFY_EQUAL2_STRING_MAC(enable, state,
                            "get another enable value than was set: %d, %d", dev, port);

            /*
               1.1.1. Call with state [GT_FALSE and GT_TRUE].
               Expected: GT_OK.
            */

            /* Call function with enable = GT_TRUE] */
            enable = GT_TRUE;

            st = cpssDxChPortCnTerminationEnableSet(dev, port, enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);

            /*
               1.1.2. Call cpssDxChPortCnTerminationEnableGet.
               Expected: GT_OK and the same enable.
            */
            st = cpssDxChPortCnTerminationEnableGet(dev, port, &state);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                          "[cpssDxChPortCnTerminationEnableGet]: %d, %d", dev, port);
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
            st = cpssDxChPortCnTerminationEnableSet(dev, port, enable);
            if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
            {
                if(port >= UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev))
                {
                    UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                }
                else
                {
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                }
            }
            else
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        /* enable == GT_TRUE  */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPortCnTerminationEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                     */
        /* enable == GT_TRUE  */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChPortCnTerminationEnableSet(dev, port, enable);
        if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
        }
        else
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, port, enable);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    enable = GT_TRUE;
    port = PORT_CN_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortCnTerminationEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    /* port == 0, enable == GT_TRUE */

    st = cpssDxChPortCnTerminationEnableSet(dev, port, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortCnTerminationEnableGet
(
    IN   GT_U8     devNum,
    IN   GT_U8     portNum,
    OUT  GT_BOOL   *statePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortCnTerminationEnableGet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (Lion and above)
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
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E);

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
            st = cpssDxChPortCnTerminationEnableGet(dev, port, &state);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* 1.1.2. Call with statePtr [NULL].
               Expected: GT_BAD_PTR
            */
            st = cpssDxChPortCnTerminationEnableGet(dev, port, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, NULL", dev, port);
        }

        /* 1.2. For all active devices go over all non available
           physical ports.
        */
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_FALSE))
        {
            st = cpssDxChPortCnTerminationEnableGet(dev, port, &state);
            if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
            {
                if(port >= UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev))
                {
                    UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                }
                else
                {
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                }
            }
            else
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPortCnTerminationEnableGet(dev, port, &state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChPortCnTerminationEnableGet(dev, port, &state);
        if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
        }
        else
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    port = PORT_CN_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortCnTerminationEnableGet(dev, port, &state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortCnTerminationEnableGet(dev, port, &state);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortCnPauseTimerMapTableEntrySet
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  index,
    IN  GT_U32                  pauseTimer
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortCnPauseTimerMapTableEntrySet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (xCat and above)
    1.1.1. Call with index [0 / 63], timer[0 / 0xFFFF].
    Expected: GT_OK.
    1.1.2. Call cpssDxChPortCnPauseTimerMapTableEntryGet with the same params.
    Expected: GT_OK and the same timerGet.
    1.1.3. Call with wrong Index [64]
    Expected: GT_BAD_PARAM.
    1.1.4. Call with wrong timer [0x10000]
    Expected: GT_OUT_OF_RANGE.
*/

    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_U32      index = 0;
    GT_U32      maxIndex = 64;
    GT_U32      timer = 0;
    GT_U32      timerGet = 0;

    GT_U32 notApplicableDevices =
        (UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E  |
        UTF_XCAT_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_XCAT3_E |
        UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_FALCON_E);

    /* prepare iterator for go over all active devices */

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notApplicableDevices);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* write, read, compare */
        for (index = 0; (index < maxIndex); index++)
        {
            timer = index * 7;
            st = cpssDxChPortCnPauseTimerMapTableEntrySet(dev, index, timer);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, index, timer);
        }
        for (index = 0; (index < maxIndex); index++)
        {
            timer = index * 7; /* same as written upper */
            st = cpssDxChPortCnPauseTimerMapTableEntryGet(dev, index, &timerGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, index);
            UTF_VERIFY_EQUAL2_PARAM_MAC(timer, timerGet, dev, index);
        }
        /*
           Call with wrong index [maxIndex].
           Expected: GT_BAD_PARAM.
        */
        timer = 0;
        index = maxIndex;
        st = cpssDxChPortCnPauseTimerMapTableEntrySet(dev, index, timer);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, index , timer);

        /*
           Call with wrong timer [0x10000]
           Expected: GT_OUT_OF_RANGE.
        */
        index = 0;
        timer = 0x10000;
        st = cpssDxChPortCnPauseTimerMapTableEntrySet(dev, index, timer);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OUT_OF_RANGE, st, dev, index , timer);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notApplicableDevices);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        index = 0;
        timer = 0;
        st = cpssDxChPortCnPauseTimerMapTableEntrySet(dev, index, timer);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, index , timer);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    index = 0;
    timer = 0;
    st = cpssDxChPortCnPauseTimerMapTableEntrySet(dev, index, timer);
    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, index , timer);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortCnPauseTimerMapTableEntryGet
(
    IN   GT_U8                   devNum,
    IN   GT_U32                  index,
    OUT  GT_U32                  *pauseTimerPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortCnPauseTimerMapTableEntryGet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (xCat and above)
    1.1.1. Call with index [0 / 63] and  non-null timerPtr.
    Expected: GT_OK.
    1.1.2. Call with wrong  index [64]
           and  non-null timerPtr.
    Expected: GT_BAD_PARAM.
    1.1.3. Call with wrong timerPtr [NULL].
    Expected: GT_BAD_PTR.
*/

    GT_STATUS   st    = GT_OK;
    GT_U8       dev;
    GT_U32      index = 0;
    GT_U32      timerGet = 0;
    GT_U32      maxIndex = 64;

    GT_U32 notApplicableDevices =
        (UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E  |
        UTF_XCAT_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_XCAT3_E |
        UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_FALCON_E);

    /* prepare iterator for go over all active devices */

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notApplicableDevices);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for (index = 0; (index < maxIndex); index++)
        {
            st = cpssDxChPortCnPauseTimerMapTableEntryGet(dev, index, &timerGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, index);
        }
        /*
           Call with wrong index [maxIndex].
           Expected: GT_BAD_PARAM.
        */
        index = maxIndex;
        st = cpssDxChPortCnPauseTimerMapTableEntryGet(dev, index, &timerGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, index);

        /*
           Call with NULL timerPtr
           Expected: GT_BAD_PTR.
        */
        index = 0;
        st = cpssDxChPortCnPauseTimerMapTableEntryGet(dev, index, NULL);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, index);
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notApplicableDevices);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        index = 0;
        st = cpssDxChPortCnPauseTimerMapTableEntryGet(dev, index, &timerGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, index);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    index = 0;
    st = cpssDxChPortCnPauseTimerMapTableEntryGet(dev, index, &timerGet);
    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, index);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortCnDbaAvailableBuffSet
(
    IN   GT_U8                   devNum,
    IN   GT_U32                  availableBuff
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortCnDbaAvailableBuffSet)
{
/*
    1.1. Call with availableBuff [0x9100 / 0x9800].
    Expected: GT_OK.
    1.2. Call with out of range availableBuff [0x100000]
    Expected: GT_OUT_OF_RANGE.
*/

    GT_STATUS   st    = GT_OK;
    GT_U8       dev;
    GT_U32      availableBuff = 0;
    GT_U32      availableBuffGet = 0;

    GT_U32 notApplicableDevices =
        (UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E  |
        UTF_XCAT_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_XCAT3_E |
        UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notApplicableDevices);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
            /* 1.1. Call with availableBuff [0x9100].
             * Expected: GT_OK.
             */
            availableBuff = 0x9100;
            st = cpssDxChPortCnDbaAvailableBuffSet(dev, availableBuff);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            st = cpssDxChPortCnDbaAvailableBuffGet(dev, &availableBuffGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChPortCnAvailableBuffSet: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(availableBuff, availableBuffGet,
                                         "got another availableBuff then was set: %d", dev);

            /* 1.1. Call with availableBuff [0x9800].
             * Expected: GT_OK.
             */
            availableBuff = 0x9800;
            st = cpssDxChPortCnDbaAvailableBuffSet(dev, availableBuff);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            st = cpssDxChPortCnDbaAvailableBuffGet(dev, &availableBuffGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChPortCnAvailableBuffSet: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(availableBuff, availableBuffGet,
                                         "got another availableBuff then was set: %d", dev);

            /* 1.2. Call with availableBuff out of range [0x10000].
             * Expected: GT_OK.
             */
            availableBuff = 0x100000;
            st = cpssDxChPortCnDbaAvailableBuffSet(dev, availableBuff);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, dev);

            availableBuff = 0x9060;
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notApplicableDevices);

    /* 2. Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortCnDbaAvailableBuffSet(dev, availableBuff);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    availableBuff = 0x9060;
    st = cpssDxChPortCnDbaAvailableBuffSet(dev, availableBuff);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortCnDbaAvailableBuffGet
(
    IN   GT_U8                   devNum,
    OUT  GT_U32                  *availableBuffPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortCnDbaAvailableBuffGet)
{
/*
    1.1. Call with non-null availableBuffPtr.
    Expected: GT_OK.
    1.2. Call with wrong availableBuffPtr [NULL].
    Expected: GT_BAD_PTR.
*/

    GT_STATUS   st    = GT_OK;
    GT_U8       dev;
    GT_U32      availableBuffGet;

    GT_U32 notApplicableDevices =
        (UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E  |
        UTF_XCAT_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_XCAT3_E |
        UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notApplicableDevices);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
           1.1. Call with non-null availableBuffPtr.
           Expected: GT_OK.
        */
        st = cpssDxChPortCnDbaAvailableBuffGet(dev, &availableBuffGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
           1.2. Call with NULL availableBuffPtr.
           Expected: GT_BAD_PTR.
        */
        st = cpssDxChPortCnDbaAvailableBuffGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, availableBuffGet = NULL", dev);
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notApplicableDevices);

    /* 2. Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortCnDbaAvailableBuffGet(dev, &availableBuffGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortCnDbaAvailableBuffGet(dev, &availableBuffGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortCnDbaModeEnableSet
(
    IN   GT_U8                   devNum,
    IN   GT_BOOL                 enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortCnDbaModeEnableSet)
{
/*
    1.1. Call with enable [GT_TRUE/GT_FALSE].
    Expected: GT_OK.
*/

    GT_STATUS   st    = GT_OK;
    GT_U8       dev;
    GT_BOOL     enable = GT_FALSE;
    GT_BOOL     enableGet = GT_FALSE;

    GT_U32 notApplicableDevices =
        (UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E  |
        UTF_XCAT_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_XCAT3_E |
        UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_FALCON_E);

    /* prepare iterator for go over all active devices */

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notApplicableDevices);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
           1.1. Call with enable [GT_TRUE].
           Expected: GT_OK.
        */
        enable = GT_TRUE;
        st = cpssDxChPortCnDbaModeEnableSet(dev, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssDxChPortCnDbaModeEnableGet(dev, &enableGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChPortCnDbaModeEnableGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                                     "got another enable then was set: %d", dev);

        /*
         * 1.1. Call with enable [GT_FALSE].
         * Expected: GT_OK.
         */
        enable = GT_FALSE;
        st = cpssDxChPortCnDbaModeEnableSet(dev, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssDxChPortCnDbaModeEnableGet(dev, &enableGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChPortCnDbaModeEnableGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                                     "got another enable then was set: %d", dev);
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notApplicableDevices);

    /* 2. Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortCnDbaModeEnableSet(dev, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    enable = GT_TRUE;
    st = cpssDxChPortCnDbaModeEnableSet(dev, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortCnDbaModeEnableGet
(
    IN   GT_U8                   devNum,
    OUT  GT_BOOL                 *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortCnDbaModeEnableGet)
{
/*
    1.1. Call with non-null enablePtr.
    Expected: GT_OK.
    1.2. Call with wrong enablePtr [NULL].
    Expected: GT_BAD_PTR.
*/

    GT_STATUS   st    = GT_OK;
    GT_U8       dev;
    GT_BOOL     enablePtr;

    GT_U32 notApplicableDevices =
        (UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E  |
        UTF_XCAT_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_XCAT3_E |
        UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_FALCON_E);

    /* prepare iterator for go over all active devices */

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notApplicableDevices);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
           1.1. Call with non-null enablePtr.
           Expected: GT_OK.
        */
        st = cpssDxChPortCnDbaModeEnableGet(dev, &enablePtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
           1.2. Call with NULL enablePtr
           Expected: GT_BAD_PTR.
        */
        st = cpssDxChPortCnDbaModeEnableGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, enablePtr = NULL", dev);
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notApplicableDevices);

    /* 2. Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortCnDbaModeEnableGet(dev, &enablePtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortCnDbaModeEnableGet(dev, &enablePtr);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortCnDbaPoolAvailableBuffSet
(
    IN   GT_U8                   devNum,
    IN   GT_U32                  poolNum,
    IN   GT_U32                  poolAvailableBuff
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortCnDbaPoolAvailableBuffSet)
{
/*
    1.1. Call with availableBuff [0x91000 / 0x98000] and poolNum [0 / 1].
    Expected: GT_OK.
    1.2. Call with out of range availableBuff [0x100000]
    Expected: GT_OUT_OF_RANGE.
*/

    GT_STATUS   st    = GT_OK;
    GT_U8       dev;
    GT_U32      availableBuff = 0;
    GT_U32      availableBuffGet = 0;
    GT_U32      poolNum = 0;

    GT_U32 notApplicableDevices =
        (UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E  |
        UTF_XCAT_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_XCAT3_E |
        UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notApplicableDevices);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
            /* 1.1. Call with availableBuff [0x91000] and poolNum 0.
             * Expected: GT_OK.
             */
            availableBuff = 0x91000;
            st = cpssDxChPortCnDbaPoolAvailableBuffSet(dev, 0, availableBuff);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            st = cpssDxChPortCnDbaPoolAvailableBuffGet(dev, 0, &availableBuffGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChPortCnDbaPoolAvailableBuffSet: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(availableBuff, availableBuffGet,
                                         "got another availableBuff then was set: %d", dev);

            /* 1.2. Call with availableBuff [0x98000] and poolNum 1.
             * Expected: GT_OK.
             */
            availableBuff = 0x98000;
            st = cpssDxChPortCnDbaPoolAvailableBuffSet(dev, 1, availableBuff);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            st = cpssDxChPortCnDbaPoolAvailableBuffGet(dev, 1, &availableBuffGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChPortCnDbaPoolAvailableBuffSet: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(availableBuff, availableBuffGet,
                                         "got another availableBuff then was set: %d", dev);

            /* 1.3. Call with poolNum out of range [2].
             * Expected: GT_OK.
             */
            poolNum = 2;
            st = cpssDxChPortCnDbaPoolAvailableBuffSet(dev, poolNum, availableBuff);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

            /* 1.3. Call with availableBuff out of range [0x100000].
             * Expected: GT_OK.
             */
            availableBuff = 0x100000;
            st = cpssDxChPortCnDbaPoolAvailableBuffSet(dev, 0, availableBuff);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, dev);

            availableBuff = 0x90000;
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notApplicableDevices);

    /* 2. Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortCnDbaPoolAvailableBuffSet(dev, 0, availableBuff);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    availableBuff = 0x90000;
    st = cpssDxChPortCnDbaPoolAvailableBuffSet(dev, 0, availableBuff);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortCnDbaPoolAvailableBuffGet
(
    IN   GT_U8                   devNum,
    IN   GT_U32                  poolNum,
    OUT  GT_U32                  *availableBuffPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortCnDbaPoolAvailableBuffGet)
{
/*
    1.1. Call with non-null availableBuffPtr.
    Expected: GT_OK.
    1.2. Call with wrong availableBuffPtr [NULL].
    Expected: GT_BAD_PTR.
*/

    GT_STATUS   st    = GT_OK;
    GT_U8       dev;
    GT_U32      availableBuffGet;
    GT_U32      poolNum = 0;

    GT_U32 notApplicableDevices =
        (UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E  |
        UTF_XCAT_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_XCAT3_E |
        UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notApplicableDevices);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
           1.1. Call with non-null availableBuffPtr.
           Expected: GT_OK.
        */
        st = cpssDxChPortCnDbaPoolAvailableBuffGet(dev, poolNum, &availableBuffGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
           1.2. Call with NULL availableBuffPtr.
           Expected: GT_BAD_PTR.
        */
        st = cpssDxChPortCnDbaPoolAvailableBuffGet(dev, poolNum, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, availableBuffGet = NULL", dev);

        /* 2. Call function with out of bound value for poolNum */
        poolNum = 2;
        st = cpssDxChPortCnDbaPoolAvailableBuffGet(dev, poolNum, &availableBuffGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
        poolNum = 0;
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notApplicableDevices);

    /* 3. Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortCnDbaPoolAvailableBuffGet(dev, poolNum, &availableBuffGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 4. Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortCnDbaPoolAvailableBuffGet(dev, poolNum, &availableBuffGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortCnProfilePortConfigSet
(
    IN GT_U8                               devNum,
    IN CPSS_PORT_TX_DROP_PROFILE_SET_ENT   profileSet,
    IN CPSS_PORT_CN_PROFILE_CONFIG_STC     *cnProfileCfgPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortCnProfilePortConfigSet)
{
/*
    ITERATE_DEVICES (Lion and above)
    1.1. Call with all correct parameters
                                profileSet [CPSS_PORT_TX_DROP_PROFILE_1_E /
                                            CPSS_PORT_TX_DROP_PROFILE_4_E],
                                cnProfileCfg.cnAware [GT_TRUE / GT_FALSE],
                                cnProfileCfg.threshold [1 / 10].
    Expected: GT_OK.
    1.2. Call cpssDxChPortCnProfilePortConfigGet
         with not-NULL cnProfileCfgPtr.
    Expected: GT_OK and the same values as was set.
    1.3. Call with wrong enum values profileSet.
    Expected: GT_BAD_PARAM.
    1.5. Call with wrong cnProfileCfgPtr [NULL].
    Expected: GT_BAD_PTR.
    1.6. Call with wrong cnProfileCfg.threshold [out of range].
    Expected: NOT GT_OK.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;

    CPSS_PORT_TX_DROP_PROFILE_SET_ENT   profileSet = 0;
    CPSS_PORT_CN_PROFILE_CONFIG_STC     cnProfileCfg;
    CPSS_PORT_CN_PROFILE_CONFIG_STC     cnProfileCfgGet;
    GT_U32 thresholdNumOfBits;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
    UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E);

    cpssOsBzero((GT_VOID*) &cnProfileCfg, sizeof(cnProfileCfg));
    cpssOsBzero((GT_VOID*) &cnProfileCfgGet, sizeof(cnProfileCfgGet));
    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with all correct parameters
                                        profileSet [CPSS_PORT_TX_DROP_PROFILE_1_E],
                                        cnProfileCfg.threshold [1].
            Expected: GT_OK.
        */
        profileSet = CPSS_PORT_TX_DROP_PROFILE_1_E;
        cnProfileCfg.cnAware = GT_TRUE;
        cnProfileCfg.threshold = 1;
        cnProfileCfg.alpha = CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_125_E;


        st = cpssDxChPortCnProfilePortConfigSet(dev, profileSet, &cnProfileCfg);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call cpssDxChPortCnProfilePortConfigGet
                 with not-NULL cnProfileCfgPtr.
            Expected: GT_OK and the same values as was set.
        */
        st = cpssDxChPortCnProfilePortConfigGet(dev, profileSet, &cnProfileCfgGet);

        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
            "cpssDxChPortCnProfilePortConfigGet: %d", dev);

        UTF_VERIFY_EQUAL1_STRING_MAC(cnProfileCfg.cnAware,
                                     cnProfileCfgGet.cnAware,
            "got another cnProfileCfg.cnAware then was set: %d", dev);

        UTF_VERIFY_EQUAL1_STRING_MAC(cnProfileCfg.threshold,
                                     cnProfileCfgGet.threshold,
            "got another cnProfileCfg.threshold then was set: %d", dev);

        UTF_VERIFY_EQUAL3_STRING_MAC(cnProfileCfg.alpha,
                                     cnProfileCfgGet.alpha,
            "got another cnProfileCfg.alpha then was set: %d %d %d", dev, cnProfileCfg.alpha, cnProfileCfgGet.alpha);

        /*
            1.1. Call with all correct parameters
                                        profileSet [CPSS_PORT_TX_DROP_PROFILE_4_E],
                                        cnProfileCfg.threshold [10].
            Expected: GT_OK.
        */
        profileSet = CPSS_PORT_TX_DROP_PROFILE_4_E;
        cnProfileCfg.cnAware = GT_FALSE;
        cnProfileCfg.threshold = 10;
        cnProfileCfg.alpha = CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_16_E;

        st = cpssDxChPortCnProfilePortConfigSet(dev, profileSet, &cnProfileCfg);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        if(GT_OK == st)
        {
            /*
                1.2. Call cpssDxChPortCnProfilePortConfigGet
                     with not-NULL cnProfileCfgPtr.
                Expected: GT_OK and the same values as was set.
            */
            st = cpssDxChPortCnProfilePortConfigGet(dev, profileSet, &cnProfileCfgGet);

            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChPortCnProfilePortConfigGet: %d %d %d", dev, cnProfileCfg.alpha, cnProfileCfgGet.alpha);

            UTF_VERIFY_EQUAL1_STRING_MAC(cnProfileCfg.cnAware, cnProfileCfgGet.cnAware,
                "got another cnProfileCfg.cnAware then was set: %d", dev);

            UTF_VERIFY_EQUAL1_STRING_MAC(cnProfileCfg.threshold, cnProfileCfgGet.threshold,
                "got another cnProfileCfg.threshold then was set: %d", dev);

            UTF_VERIFY_EQUAL4_STRING_MAC(cnProfileCfg.alpha,
                                         cnProfileCfgGet.alpha,
                "got another cnProfileCfg.alpha then was set: %d %d %d %d", dev, cnProfileCfg.alpha, cnProfileCfgGet.alpha, cnProfileCfgGet.threshold);
        }

        thresholdNumOfBits = 20;
        cnProfileCfg.threshold = BIT_MASK_MAC(thresholdNumOfBits);

        st = cpssDxChPortCnProfilePortConfigSet(dev, profileSet, &cnProfileCfg);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        if(GT_OK == st)
        {
            /*
                1.2. Call cpssDxChPortCnProfilePortConfigGet
                     with not-NULL cnProfileCfgPtr.
                Expected: GT_OK and the same values as was set.
            */
            st = cpssDxChPortCnProfilePortConfigGet(dev, profileSet, &cnProfileCfgGet);

            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChPortCnProfilePortConfigGet: %d", dev);

            UTF_VERIFY_EQUAL1_STRING_MAC(cnProfileCfg.cnAware, cnProfileCfgGet.cnAware,
                "got another cnProfileCfg.cnAware then was set: %d", dev);

            UTF_VERIFY_EQUAL1_STRING_MAC(cnProfileCfg.threshold, cnProfileCfgGet.threshold,
                "got another cnProfileCfg.threshold then was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(cnProfileCfg.alpha,
                                         cnProfileCfgGet.alpha,
                "got another cnProfileCfg.alpha then was set: %d", dev);
        }

        /*
            1.3. Call with wrong enum values profileSet.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChPortCnProfilePortConfigSet
                            (dev, profileSet, &cnProfileCfg),
                            profileSet);

        /*
            1.5. Call with wrong cnProfileCfgPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChPortCnProfilePortConfigSet(dev, profileSet, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);


        /*
            1.6. Call with wrong cnProfileCfg.threshold [out of range].
            Expected: GT_BAD_PARAM.
        */
        cnProfileCfg.threshold = BIT_MASK_MAC(thresholdNumOfBits) + 1;

        st = cpssDxChPortCnProfilePortConfigSet(dev, profileSet, &cnProfileCfg);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        cnProfileCfg.threshold = 0;

        /*
           1.7. Call with wrong cnProfileCfg.alpha [out of range].
           Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChPortCnProfilePortConfigSet
                            (dev, profileSet, &cnProfileCfg),
                            cnProfileCfg.alpha);

        cnProfileCfg.alpha = CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_E;
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
    UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortCnProfilePortConfigSet(dev, profileSet, &cnProfileCfg);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortCnProfilePortConfigSet(dev, profileSet, &cnProfileCfg);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortCnProfilePortConfigGet
(
    IN  GT_U8                               devNum,
    IN  CPSS_PORT_TX_DROP_PROFILE_SET_ENT   profileSet,
    OUT CPSS_PORT_CN_PROFILE_CONFIG_STC     *cnProfileCfgPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortCnProfilePortConfigGet)
{
/*
    ITERATE_DEVICES (Lion and above)
    1.1. Call with all correct parameters
                                profileSet [CPSS_PORT_TX_DROP_PROFILE_1_E /
                                            CPSS_PORT_TX_DROP_PROFILE_4_E],
                   and not NULL cnProfileCfgPtr.
    Expected: GT_OK.
    1.2. Call with wrong enum values profileSet.
    Expected: GT_BAD_PARAM.
    1.4. Call with wrong cnProfileCfgPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;

    CPSS_PORT_TX_DROP_PROFILE_SET_ENT   profileSet = 0;
    CPSS_PORT_CN_PROFILE_CONFIG_STC     cnProfileCfg;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
    UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with all correct parameters
                                        profileSet [CPSS_PORT_TX_DROP_PROFILE_1_E]
                           and not NULL cnProfileCfgPtr.
            Expected: GT_OK.
        */
        profileSet = CPSS_PORT_TX_DROP_PROFILE_1_E;

        st = cpssDxChPortCnProfilePortConfigGet(dev, profileSet, &cnProfileCfg);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.1. Call with all correct parameters
                                        profileSet [CPSS_PORT_TX_DROP_PROFILE_4_E],
                           and not NULL cnProfileCfgPtr.
            Expected: GT_OK.
        */
        profileSet = CPSS_PORT_TX_DROP_PROFILE_4_E;

        st = cpssDxChPortCnProfilePortConfigGet(dev, profileSet, &cnProfileCfg);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);


        /*
            1.2. Call with wrong enum values profileSet.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChPortCnProfilePortConfigGet
                            (dev, profileSet, &cnProfileCfg),
                            profileSet);

        /*
            1.5. Call with wrong cnProfileCfgPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChPortCnProfilePortConfigGet(dev, profileSet, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
    UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortCnProfilePortConfigGet(dev, profileSet, &cnProfileCfg);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortCnProfilePortConfigGet(dev, profileSet, &cnProfileCfg);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortCnQcnTriggerSet
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              index,
    IN CPSS_DXCH_PORT_CN_TRIGGER_CONFIG_STC *qcnTriggerPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortCnQcnTriggerSet)
{
/*
    1.1. Call with index [0 / 31], and qcnTriggerCfg.qcnTriggerTh0/
    qcnTriggerCfg.qcnTriggerTh1 [0x1FFF / 0x3FFF]
    Expected: GT_OK.
    1.2. Call cpssDxChPortCnQcnTriggerGet with not-NULL qcnTriggerGetPtr.
    Expected: GT_OK and the same threshold as was set.
    1.3. Call with wrong index [PORT_CN_INVALID_INDEX_CNS].
    Expected: GT_BAD_PARAM.
    1.4. Call with wrong threshold [PORT_CN_INVALID_TRIGGER_THRESHOLD_CNS].
    Expected: GT_OUT_OF_RANGE.
    1.5. Call with wrong qcnTriggerCfgPtr [NULL].
    Expected: GT_BAD_PTR
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;
    GT_U32      index;

    CPSS_DXCH_PORT_CN_TRIGGER_CONFIG_STC qcnTriggerCfg;
    CPSS_DXCH_PORT_CN_TRIGGER_CONFIG_STC qcnTriggerCfgGet;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_LION_E | UTF_LION2_E | UTF_XCAT3_E | 
                                     UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E);

    cpssOsBzero((GT_VOID*) &qcnTriggerCfg, sizeof(qcnTriggerCfg));
    cpssOsBzero((GT_VOID*) &qcnTriggerCfgGet, sizeof(qcnTriggerCfgGet));

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with index [0 / 31] and qcnTriggerCfg.qcnTriggerTh0/
            qcnTriggerCfg.qcnTriggerTh1 [0x1FFF / 0x3FFF]
            Expected: GT_OK.
        */
        index = 0;
        qcnTriggerCfg.qcnTriggerTh0 = 0x1FFF;
        qcnTriggerCfg.qcnTriggerTh1 = 0x3FFF;

        st = cpssDxChPortCnQcnTriggerSet(dev, index, &qcnTriggerCfg);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call cpssDxChPortCnQcnTriggerGet with not-NULL qcnTriggerCfgGetPtr.
            Expected: GT_OK and the same threshold as was set.
        */
        st = cpssDxChPortCnQcnTriggerGet(dev, index, &qcnTriggerCfgGet);

        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                "cpssDxChPortCnQcnTriggerGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(qcnTriggerCfg.qcnTriggerTh0, qcnTriggerCfgGet.qcnTriggerTh0,
                                "got another threshold 0 then was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(qcnTriggerCfg.qcnTriggerTh1, qcnTriggerCfgGet.qcnTriggerTh1,
                                "got another threshold 1 then was set: %d", dev);

        index = 31;
        qcnTriggerCfg.qcnTriggerTh0 = 0x3FFF;
        qcnTriggerCfg.qcnTriggerTh1 = 0x1FFF;

        st = cpssDxChPortCnQcnTriggerSet(dev, index, &qcnTriggerCfg);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssDxChPortCnQcnTriggerGet(dev, index, &qcnTriggerCfgGet);

        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                "cpssDxChPortCnQcnTriggerGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(qcnTriggerCfg.qcnTriggerTh0, qcnTriggerCfgGet.qcnTriggerTh0,
                                "got another threshold 0 then was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(qcnTriggerCfg.qcnTriggerTh1, qcnTriggerCfgGet.qcnTriggerTh1,
                                "got another threshold 1 then was set: %d", dev);

        /*
            1.3. Call with wrong index [PORT_CN_INVALID_INDEX_CNS].
            Expected: GT_BAD_PARAM.
        */
        index = PORT_CN_INVALID_INDEX_CNS;

        st = cpssDxChPortCnQcnTriggerSet(dev, index, &qcnTriggerCfg);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        index = 0;

        /*
            1.4. Call with wrong threshold [PORT_CN_INVALID_TRIGGER_THRESHOLD_CNS].
            Expected: GT_OUT_OF_RANGE.
        */
        qcnTriggerCfg.qcnTriggerTh0 = PORT_CN_INVALID_TRIGGER_THRESHOLD_CNS;
        st = cpssDxChPortCnQcnTriggerSet(dev, index, &qcnTriggerCfg);

        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, dev);

        qcnTriggerCfg.qcnTriggerTh0 = 0x1FFF;
        /*
            1.5. Call with wrong qcnTriggerCfgPtr [NULL].
            Expected: GT_OK.
        */
        st = cpssDxChPortCnQcnTriggerSet(dev, index, NULL);

        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_LION_E | UTF_LION2_E | UTF_XCAT3_E | 
                                     UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortCnQcnTriggerSet(dev, index, &qcnTriggerCfg);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortCnQcnTriggerSet(dev, index, &qcnTriggerCfg);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortCnQcnTriggerGet
(
    IN  GT_U8                                devNum,
    IN  GT_U32                               index,
    OUT CPSS_DXCH_PORT_CN_TRIGGER_CONFIG_STC *qcnTriggerPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortCnQcnTriggerGet)
{
/*
    1.1. Call with not null qcnTriggerCfgPtr.
    Expected: GT_OK
    1.2. Call with wrong qcnTriggerCfgPtr [NULL].
    Expected: GT_BAD_PTR
*/
    GT_STATUS   st  = GT_OK;
    GT_U8  dev;
    GT_U32 index = 0;

    CPSS_DXCH_PORT_CN_TRIGGER_CONFIG_STC qcnTriggerCfg;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_LION_E | UTF_LION2_E | UTF_XCAT3_E | 
                                     UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with not null qcnTriggerCfgPtr.
            Expected: GT_OK.
        */
        st = cpssDxChPortCnQcnTriggerGet(dev, index, &qcnTriggerCfg);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with wrong qcnTriggerCfgPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChPortCnQcnTriggerGet(dev, index, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_LION_E | UTF_LION2_E | UTF_XCAT3_E |
                                     UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortCnQcnTriggerGet(dev, index, &qcnTriggerCfg);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortCnQcnTriggerGet(dev, index, &qcnTriggerCfg);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortCnCNMEnableSet
(
    IN GT_U8   devNum,
    IN GT_BOOL enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortCnCNMEnableSet)
{
/*
    1.1. Call with enable [GT_TRUE/GT_FALSE].
    Expected: GT_OK.
*/

    GT_STATUS   st    = GT_OK;
    GT_U8       dev;
    GT_BOOL     enable = GT_FALSE;
    GT_BOOL     enableGet = GT_FALSE;

    GT_U32 notApplicableDevices =
        (UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E  |
        UTF_XCAT_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_XCAT3_E |
        UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E);

    /* prepare iterator for go over all active devices */

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notApplicableDevices);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
           1.1. Call with enable [GT_TRUE].
           Expected: GT_OK.
        */
        enable = GT_TRUE;
        st = cpssDxChPortCnCNMEnableSet(dev, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssDxChPortCnCNMEnableGet(dev, &enableGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChPortCnCNMEnableGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                                     "got another enable then was set: %d", dev);

        /*
         * 1.1. Call with enable [GT_FALSE].
         * Expected: GT_OK.
         */
        enable = GT_FALSE;
        st = cpssDxChPortCnCNMEnableSet(dev, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssDxChPortCnCNMEnableGet(dev, &enableGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChPortCnCNMEnableGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                                     "got another enable then was set: %d", dev);
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notApplicableDevices);

    /* 2. Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortCnCNMEnableSet(dev, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    enable = GT_TRUE;
    st = cpssDxChPortCnCNMEnableSet(dev, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortCnCNMEnableGet
(
    IN  GT_U8   devNum,
    OUT GT_BOOL *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortCnCNMEnableGet)
{
/*
    1.1. Call with non-null enablePtr.
    Expected: GT_OK.
    1.2. Call with wrong enablePtr [NULL].
    Expected: GT_BAD_PTR.
*/

    GT_STATUS   st    = GT_OK;
    GT_U8       dev;
    GT_BOOL     enablePtr;

    GT_U32 notApplicableDevices =
        (UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E  |
        UTF_XCAT_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_XCAT3_E |
        UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E);

    /* prepare iterator for go over all active devices */

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notApplicableDevices);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
           1.1. Call with non-null enablePtr.
           Expected: GT_OK.
        */
        st = cpssDxChPortCnCNMEnableGet(dev, &enablePtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
           1.2. Call with NULL enablePtr
           Expected: GT_BAD_PTR.
        */
        st = cpssDxChPortCnCNMEnableGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, enablePtr = NULL", dev);
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notApplicableDevices);

    /* 2. Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortCnCNMEnableGet(dev, &enablePtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortCnCNMEnableGet(dev, &enablePtr);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortCnCongestedQPriorityLocationSet
(
    IN GT_U8    devNum,
    IN CPSS_DXCH_PORT_CN_CONGESTED_Q_PRIORITY_LOCATION_ENT congestedQPriorityLocation
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortCnCongestedQPriorityLocationSet)
{
/*
    ITERATE_DEVICES (Lion and above)
    1.1. Call with all correct congestedQPriorityLocation
                                [CPSS_DXCH_PORT_CN_CONG_Q_NUM_E /
                                 CPSS_DXCH_PORT_CN_QCN_ORIG_PRIO_E]
    Expected: GT_OK.
    1.2. Call cpssDxChPortCnCongestedQPriorityLocationGet
         with not-NULL congestedQPriorityLocationPtr.
    Expected: GT_OK and the same congestedQPriorityLocation as was set.
    1.3. Call with wrong enum values congestedQPriorityLocation.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;

    CPSS_DXCH_PORT_CN_CONGESTED_Q_PRIORITY_LOCATION_ENT  congestedQPriorityLocation = 0;
    CPSS_DXCH_PORT_CN_CONGESTED_Q_PRIORITY_LOCATION_ENT  congestedQPriorityLocationGet;

    GT_U32 notApplicableDevices =
        (UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E  |
        UTF_XCAT_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_XCAT3_E |
        UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notApplicableDevices);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with correct prioritySpeedLocation
                                        [CPSS_DXCH_PORT_CN_CONG_Q_NUM_E]
            Expected: GT_OK.
        */
        congestedQPriorityLocation = CPSS_DXCH_PORT_CN_CONG_Q_NUM_E;

        st = cpssDxChPortCnCongestedQPriorityLocationSet(dev, congestedQPriorityLocation);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call cpssDxChPortCnCongestedQPriorityLocationGet
                 with not-NULL congestedQPriorityLocationGetPtr.
            Expected: GT_OK and the same prioritySpeedLocation as was set.
        */
        st = cpssDxChPortCnCongestedQPriorityLocationGet(dev, &congestedQPriorityLocationGet);

        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                         "cpssDxChPortCnCongestedQPriorityLocationGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(congestedQPriorityLocation, congestedQPriorityLocationGet,
                         "got another congestedQPriorityLocation then was set: %d", dev);

        /*
            1.3. Call with all correct congestedQPriorityLocation
                                        [CPSS_DXCH_PORT_CN_QCN_ORIG_PRIO_E]
            Expected: GT_OK.
        */
        congestedQPriorityLocation = CPSS_DXCH_PORT_CN_QCN_ORIG_PRIO_E;

        st = cpssDxChPortCnCongestedQPriorityLocationSet(dev, congestedQPriorityLocation);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.4. Call cpssDxChPortCnCongestedQPriorityLocationGet
                 with not-NULL congestedQPriorityLocationGetPtr.
            Expected: GT_OK and the same prioritySpeedLocation as was set.
        */
        st = cpssDxChPortCnCongestedQPriorityLocationGet(dev, &congestedQPriorityLocationGet);

        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                         "cpssDxChPortCnCongestedQPriorityLocationGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(congestedQPriorityLocation, congestedQPriorityLocationGet,
                         "got another congestedQPriorityLocationGet then was set: %d", dev);

        /*
            1.5. Call with wrong enum values congestedQPriorityLocation.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChPortCnCongestedQPriorityLocationSet
                            (dev, congestedQPriorityLocation),
                            congestedQPriorityLocation);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notApplicableDevices);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortCnCongestedQPriorityLocationSet(dev, congestedQPriorityLocation);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortCnCongestedQPriorityLocationSet(dev, congestedQPriorityLocation);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortCnCongestedQPriorityLocationGet
(
    IN  GT_U8    devNum,
    OUT CPSS_DXCH_PORT_CN_CONGESTED_Q_PRIORITY_LOCATION_ENT  *congestedQPriorityLocationPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortCnCongestedQPriorityLocationGet)
{
/*
    ITERATE_DEVICES (Lion and above)
    1.1. Call with not null congestedQPriorityLocationPtr.
    Expected: GT_OK.
    1.2. Call with wrong congestedQPriorityLocationPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;

    CPSS_DXCH_PORT_CN_CONGESTED_Q_PRIORITY_LOCATION_ENT  congestedQPriorityLocation;

    GT_U32 notApplicableDevices =
        (UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E  |
        UTF_XCAT_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_XCAT3_E |
        UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notApplicableDevices);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with not null congestedQPriorityLocationPtr.
            Expected: GT_OK.
        */
        st = cpssDxChPortCnCongestedQPriorityLocationGet(dev, &congestedQPriorityLocation);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with wrong congestedQPriorityLocationPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChPortCnCongestedQPriorityLocationGet(dev, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notApplicableDevices);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortCnCongestedQPriorityLocationGet(dev, &congestedQPriorityLocation);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortCnCongestedQPriorityLocationGet(dev, &congestedQPriorityLocation);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
 * Configuration of cpssDxChPortCn suit
 */

UTF_SUIT_BEGIN_TESTS_MAC(cpssDxChPortCn)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortCnEtherTypeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortCnEtherTypeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortCnFcEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortCnFcEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortCnFcTimerSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortCnFcTimerGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortCnFrameQueueSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortCnFrameQueueGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortCnModeEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortCnModeEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortCnPanicPauseThresholdsSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortCnPanicPauseThresholdsGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortCnPauseTriggerEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortCnPauseTriggerEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortCnProfileSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortCnProfileGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortCnProfileQueueThresholdSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortCnProfileQueueThresholdGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortCnProfileThresholdSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortCnProfileThresholdGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortCnQueueAwareEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortCnQueueAwareEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortCnSpeedIndexSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortCnSpeedIndexGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortCnFbCalcConfigSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortCnFbCalcConfigGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortCnMessageGenerationConfigSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortCnMessageGenerationConfigGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortCnPacketLengthSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortCnPacketLengthGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortCnPrioritySpeedLocationSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortCnPrioritySpeedLocationGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortCnProfileQueueConfigSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortCnProfileQueueConfigGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortCnQueueStatusModeEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortCnQueueStatusModeEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortCnSampleEntrySet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortCnSampleEntryGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortCnTerminationEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortCnTerminationEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortCnPauseTimerMapTableEntrySet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortCnPauseTimerMapTableEntryGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortCnDbaAvailableBuffSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortCnDbaAvailableBuffGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortCnDbaModeEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortCnDbaModeEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortCnDbaPoolAvailableBuffSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortCnDbaPoolAvailableBuffGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortCnQcnTriggerSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortCnQcnTriggerGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortCnCNMEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortCnCNMEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortCnProfilePortConfigSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortCnProfilePortConfigGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortCnCongestedQPriorityLocationSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortCnCongestedQPriorityLocationGet)
UTF_SUIT_END_TESTS_MAC(cpssDxChPortCn)

